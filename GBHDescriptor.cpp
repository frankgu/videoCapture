#include "GBHDescriptor.h"


GBHDescriptor::GBHDescriptor()
{
}


GBHDescriptor::~GBHDescriptor()
{

	//clear the private variables
	_iv[0].clear();
	_iv[1].clear();

}

bool GBHDescriptor::computeIntegVideo(const std::deque<cv::Mat> &ofQue)
{
	cv::Mat im, derXbuf[3], derYbuf[3];
	im = ofQue[0];

	cv::GaussianBlur(im, im, cv::Size(9, 9), 2);

	//initial the first line of itegral value
	int imageHeight = im.rows + 1;
	int imageWidth = im.cols + 1;
	for (int i = 0; i < 2; i++)
	{
		_iv[i].push_back(cv::Mat(imageHeight, imageWidth * 8, CV_32FC1, cv::Scalar(0)));
	}

	computeMaxColorDxDy(im, derXbuf[0], derYbuf[0]);

	int ivCount = 0;
	std::vector<cv::Mat> oFlows(2), rootOFs(2);
	for (int id = 1; id < ofQue.size(); id++)
	{
		//processing image with grey gradient
		im = ofQue[id];

		cv::GaussianBlur(im, im, cv::Size(9, 9), 2);

		computeMaxColorDxDy(im, derXbuf[2], derYbuf[2]);
		
		cv::Mat tmpPs, tmp0;
		subtract(derXbuf[2], derXbuf[0], oFlows[0], cv::noArray(), CV_32F);
		subtract(derYbuf[2], derYbuf[0], oFlows[1], cv::noArray(), CV_32F);

		integralHist(oFlows[0], oFlows[1], tmpPs);
		tmp0 = cv::Mat(imageHeight, imageWidth*8, CV_32FC1);
		add(tmpPs, _iv[0][ivCount], tmp0);
		_iv[0].push_back(tmp0);

		ivCount++;

		derXbuf[0] = derXbuf[2];
		derXbuf[2] = cv::Mat();

		derYbuf[0] = derYbuf[2];
		derYbuf[2] = cv::Mat();
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

	int iCols = (cols + 1)*8;
	hist = cv::Mat(rows + 1, iCols, CV_32FC1, cv::Scalar(0));

	for (int iy = 0; iy < rows; iy++)
	{
		const float *pMag = magnitude.ptr<float>(iy);
		const float *pPhase = phase.ptr<float>(iy);
		const float *pHist0 = hist.ptr<float>(iy);//for integral image, first rows and first cols are zero
		float *pHist = hist.ptr<float>(iy + 1); //for integral image, first rows and first cols are zero
		std::vector<float> histSum(8);
		for (int i = 0; i < 8; i++) histSum[i] = 0.f;
		for (int ix = 0; ix < cols; ix++)
		{
			float bin, weight0, weight1, magnitude0, magnitude1, angle;
			angle = pPhase[ix] >= 360 ? pPhase[ix] - 360 : pPhase[ix];
			int bin0, bin1;
			bin = angle / (float)(360/8);

			bin0 = floorf(bin);
			bin1 = (bin0 + 1) % 8;

			//split the magnitude into two adjacent bins
			weight1 = (bin - bin0);
			weight0 = 1 - weight1;
			magnitude0 = pMag[ix] * weight0;
			magnitude1 = pMag[ix] * weight1;
			histSum[bin0] += magnitude0;
			histSum[bin1] += magnitude1;
			for (int n = 0; n < 8; n++)
			{
				pHist[(ix + 1)*8 + n] = pHist0[(ix + 1)*8 + n] + histSum[n];
			}
		}
	}
}


void GBHDescriptor::computeMaxColorDxDy(const cv::Mat& src, cv::Mat& dx, cv::Mat& dy)
{
	//convert the imgae to grey
	cv::Mat grey;
	cv::cvtColor(src, grey, CV_RGB2GRAY);

	//do the sobel computation
	Sobel(grey, dx, CV_32F, 1, 0, 1, 1, 0, IPL_BORDER_REPLICATE);
	Sobel(grey, dy, CV_32F, 0, 1, 1, 1, 0, IPL_BORDER_REPLICATE);

}