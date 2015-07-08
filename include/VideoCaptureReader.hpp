#ifndef VideoCaptureReader_hpp
#define VideoCaptureReader_hpp

#include "VideoReader.hpp"

using namespace std;
 
class VideoCaptureReader: public VideoReader{
	cv::VideoCapture cap;
	public:  
		bool nextFrame(cv::Mat&);
		int getFps();
		int getVideoWidth();
		int getVideoHeight();
		int getNFrames();
		std::string getVideoFilePath();
		VideoCaptureReader(string x):VideoReader(x)
			{
				cv::VideoCapture cap();
				this->cap.open(this->videoFilePath);
			};


		bool isOpened(){
			return this->cap.isOpened();
		}
};

#endif
