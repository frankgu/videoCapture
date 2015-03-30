#include "GBHDescriptor.h"


GBHDescriptor::GBHDescriptor()
{
	//initialization
	_nbins = 8;
}


GBHDescriptor::~GBHDescriptor()
{
}

void GBHDescriptor::computeIntegVideo(const std::deque<cv::Mat> &ofQue, std::vector<cv::Mat> &_iv)
{
	cv::Mat im, derXbuf[2], derYbuf[2];
	im = ofQue[0];

	cv::GaussianBlur(im, im, cv::Size(9, 9), 2);

	//initial the first line of itegral value
	int imageHeight = im.rows + 1;
	int imageWidth = im.cols + 1;
	//_iv.push_back(cv::Mat(imageHeight, imageWidth * _nbins, CV_32FC1, cv::Scalar(0)));
	
	computeMaxColorDxDy(im, derXbuf[0], derYbuf[0]);

	int ivCount = 0;
	std::vector<cv::Mat> oFlows(2);
	for (size_t id = 1; id < ofQue.size(); id++)
	{
		//processing image with grey gradient
		im = ofQue[id];

		//apply a gaussianblur to the image to reduce the noise
		cv::GaussianBlur(im, im, cv::Size(9, 9), 2);

		//generate the deriatives in x and y directions
		computeMaxColorDxDy(im, derXbuf[1], derYbuf[1]);
		
		//appliy a [-1,1] temporal filter over two consecutive gradient images.
		cv::Mat tmp, tmp0;
		subtract(derXbuf[1], derXbuf[0], oFlows[0], cv::noArray(), CV_32F);
		subtract(derYbuf[1], derYbuf[0], oFlows[1], cv::noArray(), CV_32F);

		cv::Mat grad;
		cv::convertScaleAbs(oFlows[0], tmp);
		cv::convertScaleAbs(oFlows[1], tmp0);
		cv::addWeighted(tmp, 0.5, tmp0, 0.5, 0, grad);
		cv::imshow("GBH", grad);
		
/*		integralHist(oFlows[0], oFlows[1], tmp);
		tmp0 = cv::Mat(imageHeight, imageWidth*_nbins, CV_32FC1);
		add(tmp, _iv[ivCount], tmp0);
		_iv.push_back(tmp0);

		ivCount++;
		*/
		derXbuf[0] = derXbuf[1];
		derXbuf[1] = cv::Mat();

		derYbuf[0] = derYbuf[1];
		derYbuf[1] = cv::Mat();
	}
}

void GBHDescriptor::integralHist(const cv::Mat& dx, const cv::Mat& dy, cv::Mat& hist) const
{
	cv::Mat  magnitude, phase;

	//magnitude(dx, dy, magnitude);
	//phase(dx, dy, phase, true);
	cartToPolar(dx, dy, magnitude, phase, true);

	int cols = dx.cols;
	int rows = dx.rows;

	int iCols = (cols + 1)*_nbins;
	hist = cv::Mat(rows + 1, iCols, CV_32FC1, cv::Scalar(0));

	for (int iy = 0; iy < rows; iy++)
	{
		const float *pMag = magnitude.ptr<float>(iy);
		const float *pPhase = phase.ptr<float>(iy);
		const float *pHist0 = hist.ptr<float>(iy);//for integral image, first rows and first cols are zero
		float *pHist = hist.ptr<float>(iy + 1); //for integral image, first rows and first cols are zero
		std::vector<float> histSum(_nbins);
		for (int i = 0; i < _nbins; i++) histSum[i] = 0.f;
		for (int ix = 0; ix < cols; ix++)
		{
			float bin, weight0, weight1, magnitude0, magnitude1, angle;
			angle = pPhase[ix] >= 360 ? pPhase[ix] - 360 : pPhase[ix];
			int bin0, bin1;
			bin = angle / (float)(360/_nbins);

			bin0 = (int)floorf(bin);
			bin1 = (bin0 + 1) % _nbins;

			//split the magnitude into two adjacent bins
			weight1 = (bin - bin0);
			weight0 = 1 - weight1;
			magnitude0 = pMag[ix] * weight0;
			magnitude1 = pMag[ix] * weight1;
			histSum[bin0] += magnitude0;
			histSum[bin1] += magnitude1;
			for (int n = 0; n < _nbins; n++)
			{
				pHist[(ix + 1)*_nbins + n] = pHist0[(ix + 1)*_nbins + n] + histSum[n];
			}
		}
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
	Sobel(grey, dx, CV_32F, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);
	Sobel(grey, dy, CV_32F, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT);

}