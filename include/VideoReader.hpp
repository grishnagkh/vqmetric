#ifndef VideoReader_hpp
#define VideoReader_hpp

#include <string> 
#include "opencv2/opencv.hpp"

using namespace std;

class VideoReader{

	public: 
		virtual bool nextFrame(cv::Mat&) = 0;
		virtual int getFps() = 0;
		virtual int getVideoWidth() = 0;
		virtual int getVideoHeight() = 0;
		virtual int getNFrames() = 0;

		std::string getVideoFilePath();
		VideoReader(string);

	protected:
		string videoFilePath;
};

#endif
