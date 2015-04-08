#pragma once

#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <vector>
#include <deque>

class GBHDescriptor
{
public:
	
	GBHDescriptor();
	~GBHDescriptor();

	void computeIntegVideo(const std::deque<cv::Mat> &ofQue, std::vector<cv::Mat> &_iv);
	
private:

	void computeMaxColorDxDy(const cv::Mat& src, cv::Mat& dx, cv::Mat& dy);
};

