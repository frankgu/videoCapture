#include "VideoCaptureProcess.h"
#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>

int main()
{
	try
	{
		//const char* addr = "rtsp://192.168.0.100/axis-media/media.amp";
		const char* addr = "http://root:viva2014@64.61.112.18:8890/mjpg/video.mjpg";

		VideoCaptureProcess cap(0, 50, 50);
		cap.start();
		cv::Mat image;
		cv::Mat fixImage;
		long long time;
		cv::namedWindow("Original");
		while (true)
		{
			/** grab image from capturing thread
			Input Parameters:
			cv::Mat image = destination image
			long long time = time between starting capture thread and capturing image
			int lag = default 0, grab cached images
			*/

			//grap the original frame
			cap.grabFrameWithTime(image, time, 0);

			GBHDescriptor desc;
			std::vector<cv::Mat> output;
			std::deque<cv::Mat> input = cap.grabFixedVideo();
			if (!input[input.size() - 1].empty())
			{

				desc.computeIntegVideo(input, output);
				//cv::imshow("GBH", output[0]);

			}
				
			if (!image.empty())
			{
				char txt[100];
				sprintf(txt, "TimeStamp = %d %i", time, cap.getFPS());
				cv::putText(image, txt, cv::Point(50, 50), 0, 1.0, cv::Scalar(0, 0, 255));
				imshow("Original", image);
			}

			char c = cv::waitKey(30);
			if (c == 'c')
				break;
			else if (c == 's')
				cap.stop();//stop capturing thread
			else if (c == 'p')
				cap.start();//start capturing thread
		}
	}
	catch (std::exception& e)
	{
		std::cout << e.what();
	}
	return 0;
}
