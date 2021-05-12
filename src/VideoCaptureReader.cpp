/*
 *
 * Copyright (c) 2015, Stefan Petscharnig. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include "VideoCaptureReader.hpp"
#include <opencv2/highgui/highgui_c.h>

VideoCaptureReader::VideoCaptureReader(string x):VideoReader(x){
	cv::VideoCapture cap();
	this->cap.open(this->videoFilePath);
};
VideoCaptureReader::~VideoCaptureReader(){
	this->cap.release();
}
bool VideoCaptureReader::isOpened(){
	return this->cap.isOpened();
}
bool VideoCaptureReader::nextFrame(cv::Mat& theFrame){

	cv::Mat tmp;

	bool success = this->cap.read(theFrame);	
	if(!success)
		return success; //do not convert unsuccessful grabbed frames^^
	theFrame.convertTo(tmp, CV_32F);	
	cv::cvtColor(tmp, theFrame, CV_BGR2YCrCb);	

	return success;	
}
int VideoCaptureReader::getFps(){
	return this->cap.get(cv::CAP_PROP_FPS);
}
int VideoCaptureReader::getVideoWidth(){
	return this->cap.get(cv::CAP_PROP_FRAME_WIDTH);
}
int VideoCaptureReader::getVideoHeight(){
	return this->cap.get(cv::CAP_PROP_FRAME_HEIGHT);
}
int VideoCaptureReader::getNFrames(){
	return this->cap.get(cv::CAP_PROP_FRAME_COUNT);
}
std::string VideoCaptureReader::getVideoFilePath(){
	return VideoReader::getVideoFilePath();
}
