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

class VideoCaptureProcess
{
private:
	enum{ START, STOP };
	cv::VideoCapture cap;
	std::deque<cv::Mat> images;
	std::deque<cv::Mat> resizedImages;
	std::deque<long long> timeStamps;
	std::mutex mutex;
	std::thread captureThread;
	int ratio;
	bool terminateRequest;
	bool terminate;
	std::chrono::system_clock::time_point start_time;
	int state;
	int numberofResizedFrames;
	void loop();
public:
	static void captureLoop(VideoCaptureProcess* obj); //Static method called by thread
	VideoCaptureProcess(const char* source, int numberOfFrames, int _numberofResizedFrames = 2000, int ratio = 4); //constractor source = input source, numberOfFrames = number of cached frames
	VideoCaptureProcess(int source, int numberOfFrames, int numberofResizedFrames = 1000, int ratio = 4);
	void start(); //start capturing thread
	void stop(); //stop capturing thread
	void grabFrame(cv::Mat& dest, int lag = 0); //grabFrame gets captured frames from capturing thread. dest = destination frame, lag = delayed frame by lag
	void grabFrameWithTime(cv::Mat& dest, long long& time, int lag = 0);
	std::deque<cv::Mat> grabNResizedFrame(int N); //grabFrame gets captured frames from capturing thread. dest = destination frame, lag = delayed frame by lag
	void removeMResizedFrame(int m);
	void grabResizedFrameWithTime(cv::Mat& dest, long long& time, int lag = 0);
	~VideoCaptureProcess(void);
};

#endif