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

#ifndef VideoCaptureReader_hpp
#define VideoCaptureReader_hpp

#include "VideoReader.hpp"

using namespace std;

/* 
 * implementation of VideoReader using VideoCapture of opencv
 * 
 * for the ommited documentation see VideoReader
 */ 

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
		/* returns true when the file has been successfully opened, false otherwise */
		bool isOpened(){
			return this->cap.isOpened();
		}
};

#endif
