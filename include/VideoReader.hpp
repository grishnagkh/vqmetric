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

#ifndef VideoReader_hpp
#define VideoReader_hpp

#include <string> 
#include "opencv2/opencv.hpp"

using namespace std;

/*
 * Class responsible for grabbing video frames
 */

class VideoReader{

	public:
		/* 
		 * Reads the next Frame of the Video and stores it in a Mat  
		 * returns false if the video has no frames left to read, true otherwise.
         */ 
		virtual bool nextFrame(cv::Mat&) = 0;
		/*
		 * returns frames per second 
         */
		virtual int getFps() = 0;
		/*
		 * returns video width 
         */
		virtual int getVideoWidth() = 0;
		/*
		 * returns video height
         */
		virtual int getVideoHeight() = 0;
		/*
		 * returns number of frames
         */
		virtual int getNFrames() = 0;
		/*
		 * returns the video file path
         */
		std::string getVideoFilePath();
		/*
		 * constructor 
         */
		VideoReader(string);

	protected:
		/* path to the video file */
		string videoFilePath;
};

#endif
