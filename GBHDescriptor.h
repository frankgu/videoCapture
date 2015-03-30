#pragma once

#include <cv.h>
#include <iostream>
#include <vector>
#include <deque>

class GBHDescriptor
{
public:
	
	GBHDescriptor();
	~GBHDescriptor();

	void computeIntegVideo(const std::vector<cv::Mat> &ofQue, std::vector<cv::Mat> &_iv);
	
private:

	void computeMaxColorDxDy(const cv::Mat& src, cv::Mat& dx, cv::Mat& dy);
	void integralHist(const cv::Mat& dx, const cv::Mat& dy, cv::Mat& hist) const;

	int _nbins;
};

