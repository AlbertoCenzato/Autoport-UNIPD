/*
 * IPPAnalysis.cpp
 *
 *  Created on: Oct 12, 2016
 *      Author: alberto
 */

#include "IPPAnalysis.hpp"
#include <chrono>

extern Status status;


IPPAnalysis::IPPAnalysis(ImgLoader* loader) {
	this->loader = loader;
	Settings& settings =  Settings::getInstance();
	imageAnalyzer 	  = ImgAnalysis();
	patternAnalyzer   = PatternAnalysis();
	positionEstimator = PositionEstimation();

	ROITol     = settings.ROITolerance;
	sizeInfTol = 4;							//FIXME: add this to Settings
	sizeSupTol = settings.sizeSupTolerance;
	sizeTol    = settings.sizeTolerance;
	colorTol   = settings.colorTolerance;
}

IPPAnalysis::~IPPAnalysis() {}


Result IPPAnalysis::evaluate(Mat& extrinsicFactors) {

	cout << "\n-----------------------------------------\n" << endl;

	// retieve image from camera or video
	Mat image;
	bool retrieved = loader->getNextFrame(image);
	if(!retrieved) {
		cerr << "Couldn't retrieve frame!" << endl;
		return Result::END;
	}

	Mat resampleMat;
	Point2f t;
	loader->getResampleMat(resampleMat);
	loader->getCropVector (t);

	vector<LedDescriptor> points(10);

	auto begin = chrono::high_resolution_clock::now();
	bool success = imageAnalyzer.evaluate(image, points, 1);
	auto end = chrono::high_resolution_clock::now();
	cout << "\nImage analysis: " << chrono::duration_cast<chrono::milliseconds>(end-begin).count() << "ms" << endl;

	// find leds
	if(!success) {
		cerr << "ImageAnalysis failed!" << endl;
		return Result::FAILURE;
	}

	// register leds to match pattern
	begin = chrono::high_resolution_clock::now();
	success = patternAnalyzer.evaluate(points);
	end = chrono::high_resolution_clock::now();
	cout << "\nPattern analysis: " << chrono::duration_cast<chrono::milliseconds>(end-begin).count() << "ms" << endl;

	if(!success) {
		if(status == Status::FIRST_LANDING_PHASE || status == Status::SECOND_LANDING_PHASE) {
			cout << "Target lost!" << endl;
			status = Status::LOOKING_FOR_TARGET;
		}
		cerr << "PatternAnalysis failed!" << endl;
		return Result::FAILURE;
	}
	status = Status::FIRST_LANDING_PHASE;
	cout << "PatternAnalysis succeded!" << endl;

	Scalar blue(255,0,0);
	for(uint i = 0; i < points.size(); ++i) {
		string number = to_string(i);
		if(!points[i].isEmpty()) {
			circle(image, points[i].position, 30, blue, 10);
			putText(image, number, points[i].position, HersheyFonts::FONT_HERSHEY_PLAIN,
					2,blue,10,8);
		}
	}

	if(success) {
		updateROI(points);
		updateImgRes(points);

		//TODO: find an adequate tolerance for each color channel
		//updateColor(points);
	}

	convertPointsToCamera(points, t, resampleMat);

	//estimate position
	begin = chrono::high_resolution_clock::now();
	success = positionEstimator.evaluate(points, extrinsicFactors);
	end = chrono::high_resolution_clock::now();
	cout << "\nPosition estimation: " << chrono::duration_cast<chrono::milliseconds>(end-begin).count() << "ms" << endl;

	if(!success) return Result::FAILURE;

	namedWindow("Original image", WINDOW_NORMAL);
	imshow("Original image", image);
	waitKey(1);

	return Result::SUCCESS;
}

bool IPPAnalysis::reset() {
	return resetROIandRes() && resetColor();
}
