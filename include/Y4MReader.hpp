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


	int pixfmt;
	int w, h;
	int raten; // nominator of frame rate
	int rated; // denominator of frame rate
	int aspectn; // nominator of pixel aspect
	int aspectd; // denominator of pixel aspect
	int pos;  //actual pos in the file
	int frame_len; //frame length without header
	public:		
		bool nextFrame(cv::Mat&);

		int getFps();
		int getVideoWidth();
		int getVideoHeight();
		int getNFrames();
		
		std::string getVideoFilePath();

		Y4MReader(string x):VideoReader(x)
			{		
				pixfmt = YUV420;
					
				fileInput.open (this->videoFilePath.c_str(), 
									std::ifstream::in |std::ifstream::binary);

				char header[MAX_YUV4_HEADER]; //reserve bytes for the header
				char *tokstart, *tokend, *header_end;
				//consume header + a little extra
				int i;
				for (i = 0; i < MAX_YUV4_HEADER; i++) {
					header[i] = fileInput.get();	
					if (header[i] == '\n') {
						header[i] = 0x20;  // Add a space after last option.
											   // Makes parsing "444" vs "444alpha" easier.
						header[i + 1] = 0;
						break;
					}				
				}
				
				fileInput.close(); //open when necessary

//debug print;
				printf("YUV header %s\n", &header);

				cout << endl;
	
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
               
            } else if (strncmp("420mpeg2", tokstart, 8) == 0) {
               
            } else if (strncmp("420paldv", tokstart, 8) == 0) {
               
            } else if (strncmp("420p16", tokstart, 6) == 0) {
                
            } else if (strncmp("422p16", tokstart, 6) == 0) {
                
            } else if (strncmp("444p16", tokstart, 6) == 0) {
                
            } else if (strncmp("420p14", tokstart, 6) == 0) {
                
            } else if (strncmp("422p14", tokstart, 6) == 0) {
                
            } else if (strncmp("444p14", tokstart, 6) == 0) {
                
            } else if (strncmp("420p12", tokstart, 6) == 0) {
               
            } else if (strncmp("422p12", tokstart, 6) == 0) {
               
            } else if (strncmp("444p12", tokstart, 6) == 0) {
               
            } else if (strncmp("420p10", tokstart, 6) == 0) {
               
            } else if (strncmp("422p10", tokstart, 6) == 0) {

            } else if (strncmp("444p10", tokstart, 6) == 0) {

            } else if (strncmp("420p9", tokstart, 5) == 0) {

            } else if (strncmp("422p9", tokstart, 5) == 0) {

            } else if (strncmp("444p9", tokstart, 5) == 0) {

            } else if (strncmp("420", tokstart, 3) == 0) {

            } else if (strncmp("411", tokstart, 3) == 0) {

            } else if (strncmp("422", tokstart, 3) == 0) {

            } else if (strncmp("444alpha", tokstart, 8) == 0 ) {

            } else if (strncmp("444", tokstart, 3) == 0) {

            } else if (strncmp("mono16", tokstart, 6) == 0) {

            } else if (strncmp("mono", tokstart, 4) == 0) {

            } else {
                std::cout << "YUV4MPEG stream contains an unknown pixel format " << std::endl;              
            }
            while (tokstart < header_end && *tokstart != 0x20)
                tokstart++;
            break;
        case 'I': // Interlace type //TODO
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
	
	this->pos = strlen(header) * sizeof(char);	
	/*header parsing end */
			
 /* calculate frame length without header */
	if(pixfmt ==  YUV420)
		frame_len = w * h * 3 / 2 ;
	if(pixfmt ==  YUV422)
		frame_len = w * h * 2 ;
	if(pixfmt ==  YUV444)
		frame_len = w * h * 3 ;
	
			};
		/* returns true when the file has been successfully opened, false otherwise */
		bool isOpened(){
			return fileInput.is_open();
		}
};

#endif
