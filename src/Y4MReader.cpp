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


Y4MReader::Y4MReader(string x):VideoReader(x){		
	pixfmt = YUV420; /* default value */
	
	this->fileInput.open (this->videoFilePath.c_str(), 
						std::ifstream::in |std::ifstream::binary);

	char *tokstart, *tokend, *header_end;

	char header[MAX_YUV4_HEADER]; //reserve bytes for the header

	//consume header + a little extra
	int i;
	for (i = 0; i < MAX_YUV4_HEADER; i++) {
		header[i] = this->fileInput.get();	
		if (header[i] == '\n') {
			header[i] = 0x20;  // Add a space after last option.
								   // Makes parsing "444" vs "444alpha" easier.
			header[i + 1] = 0;
			break;
		}				
	}
				
//debug print;
	
//	printf("YUV header %s\n", &header[0]);

//	cout << endl;
	
//debug print end				

			/** parse the header **/	
	if (i == MAX_YUV4_HEADER)
		return;

    if (strncmp(header, "YUV4MPEG2", 9))
  		return;

    header_end = &header[i + 1]; // Include space
    for (tokstart = &header[10];
         tokstart < header_end; tokstart++) {
        if (*tokstart == 0x20)
            continue;
        switch (*tokstart++) {
        case 'W': // Width. Required.
            w = strtol(tokstart, &tokend, 10);
            tokstart = tokend;
            break;
        case 'H': // Height. Required.
            h =  strtol(tokstart, &tokend, 10);
            tokstart = tokend;
            break;
        case 'C': // Color space, TODO see https://github.com/FFmpeg/FFmpeg/blob/master/libavformat/yuv4mpegdec.c
            if (strncmp("420jpeg", tokstart, 7) == 0) {
				pixfmt = YUV420;
            } else if (strncmp("420mpeg2", tokstart, 8) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("420paldv", tokstart, 8) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("420p16", tokstart, 6) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("422p16", tokstart, 6) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("444p16", tokstart, 6) == 0) {
  				std::cout << "pixel format not supported, trying 420p " << std::endl;
            } else if (strncmp("420p14", tokstart, 6) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("422p14", tokstart, 6) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("444p14", tokstart, 6) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("420p12", tokstart, 6) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("422p12", tokstart, 6) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;
            } else if (strncmp("444p12", tokstart, 6) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;
            } else if (strncmp("420p10", tokstart, 6) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;
            } else if (strncmp("422p10", tokstart, 6) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("444p10", tokstart, 6) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("420p9", tokstart, 5) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("422p9", tokstart, 5) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("444p9", tokstart, 5) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("420", tokstart, 3) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("411", tokstart, 3) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("422", tokstart, 3) == 0) {
				pixfmt = YUV422;
            } else if (strncmp("444alpha", tokstart, 8) == 0 ) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("444", tokstart, 3) == 0) {
				pixfmt = YUV444;
            } else if (strncmp("mono16", tokstart, 6) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else if (strncmp("mono", tokstart, 4) == 0) {
				std::cout << "pixel format not supported, trying 420p " << std::endl;         
            } else {
                std::cout << "YUV4MPEG stream contains an unknown pixel format " << std::endl;              
            }
            while (tokstart < header_end && *tokstart != 0x20)
                tokstart++;
            break;
        case 'I': // Interlace type //TODO: ignored for now
            switch (*tokstart++){
            case '?':
              
                break;
            case 'p':
              
                break;
            case 't':
          
                break;
            case 'b':
               
                break;
            case 'm':
				break;             
            default:
				 std::cout << "YUV4MPEG stream contains invalid interlace type " << std::endl;
            }
            break;
        case 'F': // Frame rate
            sscanf(tokstart, "%d:%d", &(this->raten), &(this->rated)); // 0:0 if unknown
            while (tokstart < header_end && *tokstart != 0x20)
                tokstart++;
            break;
        case 'A': // Pixel aspect
            sscanf(tokstart, "%d:%d", &aspectn, &aspectd); // 0:0 if unknown
            while (tokstart < header_end && *tokstart != 0x20)
                tokstart++;
            break;
        case 'X': // Vendor extensions, ignore here
                tokstart++;
            break;
        }
    }
 
	/* header parsing end */

	frame_w[Y] = w;	
	frame_h[Y] = h;	
	
	/* calculate frame length without header */
	if(pixfmt ==  YUV420){
		frame_w[V] = frame_w[U] = w >> 1;
		frame_h[V] = frame_h[U] = h >> 1;
	}else if(pixfmt ==  YUV422){
		/* to be tested */
		frame_h[V] = frame_h[U] = h;
		frame_w[V] = frame_w[U] = w >> 1;
	}else if(pixfmt ==  YUV444){
		/* to be tested */
		frame_h[V] = frame_h[U] = h;
		frame_w[V] = frame_w[U] = w;
	}else{
		std::cerr << "unsupported input format" << std::endl; 
		exit(-1);
	}
}

Y4MReader::~Y4MReader(){
	this->fileInput.close();
}

bool Y4MReader::nextFrame(cv::Mat& theFrame){

	char f_header[MAX_FRAME_HEADER]; /* reserve bytes for the header */
			
	/* consume header + a little extra */
	int i;
	for (i = 0; i < MAX_FRAME_HEADER; i++) {
		f_header[i] = this->fileInput.get();	
		if (f_header[i] == '\n') {
			f_header[i] = 0x20;  // Add a space after last option.							
			f_header[i + 1] = 0;
			break;
		}				
	}

	if(	!(strncmp("FRAME",f_header, 5) == 0 ))
		return false;

	uchar bufferY[frame_w[Y] * frame_h[Y]]; 
	uchar bufferU[frame_w[U] * frame_h[U]]; 
	uchar bufferV[frame_w[V] * frame_h[V]]; 

	cv::Mat planes[3];
	cv::Mat tmp;
	this->fileInput.read((char*)(&bufferY), sizeof(char) * frame_w[Y] * frame_h[Y]);
	
	planes[Y] = cv::Mat(frame_h[Y], frame_w[Y], CV_8UC1, bufferY) ;	

	this->fileInput.read((char*)(&bufferU), sizeof(char) * frame_w[U] * frame_h[U]) ;
	tmp = cv::Mat(frame_h[U], frame_w[U], CV_8UC1, bufferU);	
	cv::resize(tmp, planes[U], cv::Size(), frame_w[Y] / frame_w[U], frame_h[Y] / frame_h[U], CV_INTER_CUBIC);

	this->fileInput.read((char*)(&bufferV), sizeof(char) * frame_w[V] * frame_h[V]) ;
	tmp =  cv::Mat(frame_h[V], frame_w[V], CV_8UC1, bufferV);	
	cv::resize(tmp, planes[V], cv::Size(), frame_w[Y] / frame_w[V], frame_h[Y] / frame_h[V], CV_INTER_CUBIC);

	
	tmp = cv::Mat(h, w, CV_8UC3);
	merge(planes, 3, tmp);

	tmp.convertTo(theFrame, CV_32F);

	return true;
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
	return -1; /* we do not know how many frames the video has */
}
std::string Y4MReader::getVideoFilePath(){
	return this->videoFilePath;
}
/* returns true when the file has been successfully opened, false otherwise */
bool Y4MReader::isOpened(){
	return this->fileInput.is_open();
}
