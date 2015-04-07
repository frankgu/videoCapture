#pragma once
#ifndef VIDEO_CAPTURE_PROCESS_H
#define VIDEO_CAPTURE_PROCESS_H
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <exception>
#include <deque>
#include <thread>
#include <mutex>
#include <chrono>
#include "GBHDescriptor.h"

struct Image
{
	cv::Mat image;
	long long timeStamp;
};

class VideoCaptureProcess
{
private:
	enum{ START, STOP };
	cv::VideoCapture cap;
	std::deque<Image> images;
	std::deque<Image> resizedImages;
	std::deque<cv::Mat> GBHImages;
	std::mutex mutex;
	std::mutex GBHThreadmutex;
	std::thread captureThread;
	std::thread GBHDescriptorThread;
	int ratio;
	bool terminateRequest;
	bool terminate;
	int state;
	int numberofResizedFrames;
	void loop();
	void GBHDescriptorloop();
	std::chrono::system_clock::time_point start_time;

	GBHDescriptor desc;

public:
	VideoCaptureProcess(const char* source, int numberOfFrames, int _numberofResizedFrames = 2000, int ratio = 4); //constractor source = input source, numberOfFrames = number of cached frames
	VideoCaptureProcess(int source, int numberOfFrames, int numberofResizedFrames = 1000, int ratio = 4);
	static void captureLoop(VideoCaptureProcess* obj); //Static method called by thread
	static void GBHDescriptorGenerator(VideoCaptureProcess* obj); //Static method called by threads
	void start(); //start capturing thread
	void stop(); //stop capturing thread
	void grabFrame(cv::Mat& dest, int lag = 0); //grabFrame gets captured frames from capturing thread. dest = destination frame, lag = delayed frame by lag
	void grabGBHFrame(cv::Mat& dest, int lag = 0); // grab the GBH result image
	void grabFrameWithTime(cv::Mat& dest, long long& time, int lag = 0);
	void removeMResizedFrame(int m);
	void grabResizedFrameWithTime(cv::Mat& dest, long long& time, int lag = 0);
	std::deque<Image> getFixedImageVideo();
	std::deque<Image> grabNResizedFrame(int N); //grabFrame gets captured frames from capturing thread. dest = destination frame, lag = delayed frame by lag
	int getFPS();
	cv::Size getFrameSize();

	~VideoCaptureProcess(void);
};

#endif