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

#ifndef Y4MReader_hpp
#define Y4MReader_hpp

#include "VideoReader.hpp"


#include <iostream>
#include <fstream>

using namespace std;

/*
 * Class reading .y4m video files, for now we assume pixel format 420
 */

class Y4MReader: public VideoReader{
	ifstream fileInput;
 	const static int MAX_YUV4_HEADER 		= 80;
 	const static int MAX_FRAME_HEADER 		= 80;
	const static int Y4M_FRAME_MAGIC_LEN	= 6;

 	const static int YUV420 = 1;
 	const static int YUV422 = 2;
 	const static int YUV444 = 3;

 	const static int Y = 0;
 	const static int V = 1; // Cb
 	const static int U = 2; // Cr

	int pixfmt;
	int w, h;
	int raten; // nominator of frame rate
	int rated; // denominator of frame rate
	int aspectn; // nominator of pixel aspect
	int aspectd; // denominator of pixel aspect
	int pos;  //actual pos in the file
	int frame_w[3]; //frame width per channel
	int frame_h[3]; //frame height per channel
	public:	
		~Y4MReader();
		Y4MReader(string x);
	
		bool nextFrame(cv::Mat&);
		bool isOpened();
		int getFps();
		int getVideoWidth();
		int getVideoHeight();
		int getNFrames();
		
		std::string getVideoFilePath();
};
		
#endif
