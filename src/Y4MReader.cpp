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

#include "Y4MReader.hpp"
#include <math.h>

bool Y4MReader::nextFrame(cv::Mat& theFrame){
	/* read frame header */
	
	return 0;	
}
int Y4MReader::getFps(){
	return round(this->raten / this->rated);
}
int Y4MReader::getVideoWidth(){
	return this->w;	
}
int Y4MReader::getVideoHeight(){
	return this->h;	
}
int Y4MReader::getNFrames(){
	return -1;
}
std::string Y4MReader::getVideoFilePath(){
	return this->videoFilePath;
}
