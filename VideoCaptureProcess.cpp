#include "VideoCaptureProcess.h"
#include <iostream>


VideoCaptureProcess::VideoCaptureProcess(const char* source, int numberOfFrames, int numberOfFixedImages, int _numberofResizedFrames, int _ratio)
{
	state = STOP;
	cap.open(source); //opens capture stream
	start_time = std::chrono::high_resolution_clock::now();
	if (!cap.isOpened())
		throw std::exception("Cannot open source"); //throw exception if it is not able to open the stream
	images = std::deque<cv::Mat>(numberOfFrames); //initialize quoue of captured images
	fixedImages = std::deque<cv::Mat>(numberOfFixedImages);
	numberofResizedFrames = _numberofResizedFrames;
	timeStamps = std::deque<long long>(numberofResizedFrames);
	ratio = _ratio;
}

VideoCaptureProcess::VideoCaptureProcess(int source, int numberOfFrames, int numberOfFixedImages, int _numberofResizedFrames, int _ratio)
{
	state = STOP;
	cap.open(source); //opens capture stream
	start_time = std::chrono::high_resolution_clock::now();
	if (!cap.isOpened())
		throw std::exception("Cannot open source"); //throw exception if it is not able to open the stream
	images = std::deque<cv::Mat>(numberOfFrames); //initialize quoue of captured images
	fixedImages = std::deque<cv::Mat>(numberOfFixedImages);
	numberofResizedFrames = _numberofResizedFrames;
	timeStamps = std::deque<long long>(numberofResizedFrames);
	ratio = _ratio;
}

void VideoCaptureProcess::start()
{

	if (state == START)
		return;
	terminate = false;
	terminateRequest = false;
	captureThread = std::thread(VideoCaptureProcess::captureLoop, this); // starts capturing thread
	captureThread2 = std::thread(VideoCaptureProcess::captureFixedImages, this); // starts another capturing
	state = START;

}

void VideoCaptureProcess::captureLoop(VideoCaptureProcess* obj) //static function
{
	obj->loop();
}

void VideoCaptureProcess::captureFixedImages(VideoCaptureProcess* obj) // static function
{
	obj->loop2();
}

void VideoCaptureProcess::loop2()//another capture loop
{
	while (!terminate){

		mutex.lock();
		for (int i = 0; i < fixedImages.size(); i++)
		{
			fixedImages.push_back(images.at(images.size() - 1 - i).clone());

//			fixedImages.push_back(images[199].clone());
			fixedImages.pop_front();
		}
		mutex.unlock();
	}
}

void VideoCaptureProcess::loop()//capturing loop
{
	cv::Mat frame;
	while (!terminate)
	{
		if (cap.grab())
			cap.retrieve(frame);
		mutex.lock();
		if (terminateRequest)// checks for termination Request
			terminate = true;
		mutex.unlock();
		cv::Mat img = frame.clone();
		if (!img.empty())
		{
			cv::Mat res;
			cv::cvtColor(img, res, CV_BGR2GRAY);
			cv::resize(res, res, cv::Size(), 1.0 / ratio, 1.0 / ratio);
			mutex.lock();
			images.push_back(img);
			images.pop_front();
			if (numberofResizedFrames > resizedImages.size())
				resizedImages.push_back(res);
			auto end_time = std::chrono::high_resolution_clock::now();
			timeStamps.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());
			timeStamps.pop_front();
			mutex.unlock();

		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
void VideoCaptureProcess::grabFrame(cv::Mat& ret, int i)
{
	int sz;
	mutex.lock();
	sz = images.size();
	mutex.unlock();
	if (i >= sz)
		throw std::exception("Out of Range");

	mutex.lock();
	ret = images[sz - i - 1].clone();
	mutex.unlock();

}

void VideoCaptureProcess::grabFixedImageFrame(cv::Mat& ret)
{
	int sz, ts;
	mutex.lock();
	sz = fixedImages.size();
	ret = fixedImages[sz - 1].clone();
	mutex.unlock();
}

void VideoCaptureProcess::grabFrameWithTime(cv::Mat& ret, long long& time, int i)
{
	int sz, ts;
	mutex.lock();
	sz = images.size();
	ts = timeStamps.size();
	mutex.unlock();
	if (i >= sz)
		throw std::exception("Out of Range");

	mutex.lock();
	ret = images[sz - i - 1].clone();
	time = timeStamps[ts - i - 1];
	mutex.unlock();
}
std::deque<cv::Mat> VideoCaptureProcess::grabNResizedFrame(int N)
{
	int sz;
	mutex.lock();
	sz = resizedImages.size();
	mutex.unlock();
	if (N >= sz)
		return std::deque<cv::Mat>();
	std::deque<cv::Mat> out(N);
	mutex.lock();
	out = std::deque<cv::Mat>(resizedImages.begin(), resizedImages.begin() + N);
	mutex.unlock();
	return out;
}
void VideoCaptureProcess::removeMResizedFrame(int m)
{
	int sz;
	mutex.lock();
	sz = resizedImages.size();
	mutex.unlock();
	if (m >= sz)
		return;
	mutex.lock();
	for (int i = 0; i < m; i++)
		resizedImages.pop_front();
	mutex.unlock();
}
VideoCaptureProcess::~VideoCaptureProcess(void)
{

	stop();
	cap.release();

}
void VideoCaptureProcess::stop(void)
{
	if (state == STOP)
		return;
	mutex.lock();
	terminateRequest = true;
	mutex.unlock();
	captureThread.join();
	captureThread2.join();
	state = STOP;
}