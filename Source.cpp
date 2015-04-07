#include "VideoCaptureProcess.h"
#include <time.h>
#include <limits.h>
#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>

int main(){

	try{
		//const char* addr = "rtsp://192.168.0.100/axis-media/media.amp";
		//const char* addr = "http://root:viva2014@64.61.112.18:8890/mjpg/video.mjpg";
		const char* addr = "C:\\Users\\Dongfeng\\Desktop\\fight.mp4";

		// video capture process
		VideoCaptureProcess cap(addr, 50);
		cap.start();

		// video writer
		cv::VideoWriter outputVideo;
		outputVideo.open("C:/Users/Dongfeng/Desktop/output.avi", -1, 60, cap.getFrameSize());

		cv::Mat image, GBHImage;
		long long timestamp;
		cv::namedWindow("Original");
		cv::namedWindow("GBH");

		//fps counter begin
		time_t start, end;
		double sec;
		double fps;
		int counter = 0;
		//fps counter end

		int c;
		while (true){

			//fps counter begin
			if (counter == 0){
				time(&start);
			}
			//fps counter end

			//grap the original frame and the GBH result frame
			cap.grabFrameWithTime(image, timestamp, 0);
			cap.grabGBHFrame(GBHImage, 0);

			if (!GBHImage.empty()){
				cv::imshow("GBH", GBHImage);
				outputVideo << GBHImage;
			}

			if (!image.empty()){
				char txt[100];
				sprintf(txt, "TimeStamp = %d", timestamp);
				cv::putText(image, txt, cv::Point(50, 50), 0, 1.0, cv::Scalar(0, 0, 255));
				cv::imshow("Original", image);
			}

			// fps counter begin
			time(&end);
			counter++;
			sec = difftime(end, start);
			fps = counter / sec;
			if (counter > 30)
				std::cout << fps << std::endl;
			std::cout.flush();
			// overflow protection
			if (counter == (INT_MAX - 1000))
				counter = 0;
			// fps counter end

			c = cv::waitKey(33);
			if (c == 'c')
				break;
			else if (c == 's')
				cap.stop();//stop capturing thread
			else if (c == 'p')
				cap.start();//start capturing thread

		}
		outputVideo.release();
	}
	catch (std::exception& e){
		std::cout << e.what();
	}
	return 0;
}
