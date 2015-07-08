#include "VideoCaptureReader.hpp"

bool VideoCaptureReader::nextFrame(cv::Mat& theFrame){
	return this->cap.read(theFrame);	
}
int VideoCaptureReader::getFps(){
	return this->cap.get(CV_CAP_PROP_FPS);
}
int VideoCaptureReader::getVideoWidth(){
	return this->cap.get(CV_CAP_PROP_FRAME_WIDTH);
}
int VideoCaptureReader::getVideoHeight(){
	return this->cap.get(CV_CAP_PROP_FRAME_HEIGHT);
}
int VideoCaptureReader::getNFrames(){
	return this->cap.get(CV_CAP_PROP_FRAME_COUNT);
}
std::string VideoCaptureReader::getVideoFilePath(){
	return VideoReader::getVideoFilePath();
}
