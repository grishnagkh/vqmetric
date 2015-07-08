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
