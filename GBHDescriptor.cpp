#include "GBHDescriptor.h"


GBHDescriptor::GBHDescriptor()
{
}


GBHDescriptor::~GBHDescriptor()
{
}

void GBHDescriptor::computeIntegVideo(const std::deque<cv::Mat> &ofQue, std::vector<cv::Mat> &_iv)
{
	cv::Mat im, derXbuf[2], derYbuf[2];
	im = ofQue[0];

	cv::GaussianBlur(im, im, cv::Size(3, 3), 0, 0, 4);
	computeMaxColorDxDy(im, derXbuf[0], derYbuf[0]);

	int ivCount = 0;
	std::vector<cv::Mat> oFlows(2);
	for (size_t id = 1; id < ofQue.size(); id++)
	{
		//processing image with grey gradient
		im = ofQue[id];

		//apply a gaussianblur to the image to reduce the noise
		cv::GaussianBlur(im, im, cv::Size(3, 3), 0, 0, 4);

		//generate the deriatives in x and y directions
		computeMaxColorDxDy(im, derXbuf[1], derYbuf[1]);

		//appliy a [-1,1] temporal filter over two consecutive gradient images.
		cv::Mat tmp, tmp0;
		subtract(derXbuf[1], derXbuf[0], oFlows[0], cv::noArray(), CV_16S);
		subtract(derYbuf[1], derYbuf[0], oFlows[1], cv::noArray(), CV_16S);

		cv::Mat grad;
		cv::convertScaleAbs(oFlows[0], tmp);
		cv::convertScaleAbs(oFlows[1], tmp0);
		cv::addWeighted(tmp, 0.5, tmp0, 0.5, 0, grad);
		_iv.push_back(grad);

		derXbuf[0] = derXbuf[1];
		derXbuf[1] = cv::Mat();

		derYbuf[0] = derYbuf[1];
		derYbuf[1] = cv::Mat();
	}
}

void GBHDescriptor::computeMaxColorDxDy(const cv::Mat& src, cv::Mat& dx, cv::Mat& dy)
{
	//convert the imgae to grey
	cv::Mat grey;
	if (src.channels() > 1)
		cv::cvtColor(src, grey, CV_RGB2GRAY);
	else
		grey = src;

	//do the sobel computation
	Sobel(grey, dx, CV_16S, 1, 0);
	Sobel(grey, dy, CV_16S, 0, 1);

}