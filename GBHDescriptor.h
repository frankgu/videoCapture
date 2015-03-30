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

	bool computeIntegVideo(const std::deque<cv::Mat> &ofQue);
	void computeMaxColorDxDy(const cv::Mat& src, cv::Mat& dx, cv::Mat& dy);
	void integralHist(const cv::Mat& dx, const cv::Mat& dy, cv::Mat& hist) const;

private:

	std::vector<cv::Mat> _iv[2];	//integral video for 3D derivatives(dx, dy, dt)
};

