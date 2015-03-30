#include "VideoCaptureProcess.h"
#include <iostream>


VideoCaptureProcess::VideoCaptureProcess(const char* source, int numberOfFrames, int numberOfFixedImages, int _numberofResizedFrames, int _ratio)
{
	state = STOP;
	cap.open(source); //opens capture stream
	start_time = std::chrono::high_resolution_clock::now();
	if (!cap.isOpened())
		throw std::exception("Cannot open source"); //throw exception if it is not able to open the stream
	images = std::deque<Image>(numberOfFrames); //initialize quoue of captured images
	fixedImages = std::deque<Image>(numberOfFixedImages);
	numberofResizedFrames = _numberofResizedFrames;
	ratio = _ratio;
}

VideoCaptureProcess::VideoCaptureProcess(int source, int numberOfFrames, int numberOfFixedImages, int _numberofResizedFrames, int _ratio)
{
	state = STOP;
	cap.open(source); //opens capture stream
	start_time = std::chrono::high_resolution_clock::now();
	if (!cap.isOpened())
		throw std::exception("Cannot open source"); //throw exception if it is not able to open the stream
	images = std::deque<Image>(numberOfFrames); //initialize quoue of captured images
	fixedImages = std::deque<Image>(numberOfFixedImages);
	numberofResizedFrames = _numberofResizedFrames;
	ratio = _ratio;
}

void VideoCaptureProcess::start()
{

	if (state == START)
		return;
	terminate = false;
	terminateRequest = false;
	captureThread = std::thread(VideoCaptureProcess::captureLoop, this); // starts capturing thread
	captureThread2 = std::thread(VideoCaptureProcess::captureFixedImages, this); // starts another capturing thread and do the action detection
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

		Image image;
		mutex.lock();
		for (int i = 0; i < fixedImages.size(); i++)
		{
			image.image = images.at(fixedImages.size() - 1 - i).image.clone();
			image.timeStamp = images.at(fixedImages.size() - 1 - i).timeStamp;
			fixedImages.push_back(image);
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
			Image image, resImage;
			cv::Mat res;
			cv::cvtColor(img, res, CV_BGR2GRAY);
			cv::resize(res, res, cv::Size(), 1.0 / ratio, 1.0 / ratio);
			mutex.lock();
			auto end_time = std::chrono::high_resolution_clock::now();
			long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
			image.image = img;
			resImage.image = img;
			image.timeStamp = timestamp;
			resImage.timeStamp = timestamp;

			// insert the image capture from the sequence and the timestamp to the images deque
			images.push_back(image);
			images.pop_front();

			// insert the resize image to the resizedImages deque
			if (numberofResizedFrames > resizedImages.size())
				resizedImages.push_back(resImage);
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
	ret = images[sz - i - 1].image.clone();
	mutex.unlock();

}

void VideoCaptureProcess::grabFixedImageFrame(cv::Mat& ret)
{
	int sz, ts;
	mutex.lock();
	sz = fixedImages.size();
	ret = fixedImages[sz - 1].image.clone();
	mutex.unlock();
}

void VideoCaptureProcess::grabFrameWithTime(cv::Mat& ret, long long& time, int i)
{
	int sz;
	mutex.lock();
	sz = images.size();
	mutex.unlock();
	if (i >= sz)
		throw std::exception("Out of Range");

	mutex.lock();
	ret = images[sz - i - 1].image.clone();
	time = images[sz - i - 1].timeStamp;
	mutex.unlock();
}

int VideoCaptureProcess::getFPS()
{
	return cap.get(CV_CAP_PROP_FPS);
}

std::deque<Image> VideoCaptureProcess::grabNResizedFrame(int N)
{
	int sz;
	mutex.lock();
	sz = resizedImages.size();
	mutex.unlock();
	if (N >= sz)
		return std::deque<Image>();
	std::deque<Image> out(N);
	mutex.lock();

	out = std::deque<Image>(resizedImages.begin(), resizedImages.begin() + N);
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