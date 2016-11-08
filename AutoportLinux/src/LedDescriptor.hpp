/*
 * LedDescriptor.h
 *
 *  Created on: Oct 29, 2016
 *      Author: alberto
 */

#ifndef LEDDESCRIPTOR_HPP_
#define LEDDESCRIPTOR_HPP_

#include "opencv2/opencv.hpp"
#include "GenPurpFunc.hpp"

using namespace std;
using namespace cv;

class LedDescriptor {
public:

	Point2f position;
	Scalar 	color;
	float	size;

	LedDescriptor();
	LedDescriptor(Point2f &position, Scalar &color, float size);
	LedDescriptor(float x, float y, float hue, float saturation, float value, float size);
	virtual ~LedDescriptor();

	float L2Dist  (const LedDescriptor &ledDesc) const;
	float cartDist(const LedDescriptor &ledDesc) const;

	bool isEmpty();

	static Point2f centroid(const vector<LedDescriptor> &descriptors);

};

#endif /* LEDDESCRIPTOR_HPP_ */
