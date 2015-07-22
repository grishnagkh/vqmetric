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

#include <ctime>
#include "opencv2/opencv.hpp"
#include "VideoCaptureReader.hpp"
#include "VideoReader.hpp"
#include "PSNR.hpp"
#include "SSIM.hpp"
#include "VQM.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

/* 
 * Sample call 
 *
 * 	./main -p 500.mp4 -P mp4 -r 1000.mp4 -R mp4 --psnr --ssim out
 *
 */

int main(int argc, char **argv){
	
	/* begin option parsing */
	int c;
	/* flags */
	int psnr_flag = 0;
	int ssim_flag = 0;
	int vqm_flag = 0;

	string processed, reference, procfmt, reffmt, outputmode, scaling, out_prefix;
	
	//TODO: segment length parameter
	while (1){
		static struct option long_options[] = {	
			  {"psnr", no_argument, &psnr_flag, 1},
			  {"ssim", no_argument, &ssim_flag, 1},
			  {"vqm",  no_argument, &vqm_flag , 1},
 			 /* These options don’t set a flag.
				 We distinguish them by their indices. */
			  {"processed",  		required_argument,	0, 'p'}, // path to source file
			  {"reference",  		required_argument, 	0, 'r'}, // path to reference file
			  {"processed-format",  required_argument, 	0, 'P'}, // try to estimate by file ending
			  {"reference-format",  required_argument,  0, 'R'}, // try to estimate by file ending
			  {"output",  			required_argument, 	0, 'o'}, // standard, extended, pedantic
			  {"scaling",  			required_argument, 	0, 'c'}, // lanczos, bicubic scaling
			  {0, 0, 0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "p:r:R:P:o:c:",
				       long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1) break;

		switch (c){
			case 0:
				/* If this option set a flag, do nothing else now. */				
				break;
			case 'p':
				processed = optarg;
				break;
			case 'r':
				reference = optarg;
				break;
			case 'P':
				procfmt = optarg;
				break;
			case 'R':
				reffmt = optarg;
				break;
			case 'o':
				outputmode = optarg;
				break;		
			case 'c':
				std::cout << "scaling mode: " << optarg  << std::endl;
				scaling = optarg;
				break;		 
		}
	}
	if (optind < argc){
		out_prefix = argv[optind];
	}else{
		out_prefix = "out";
	}
	std::cout << "reffmt: " << reffmt << strcmp(reffmt.c_str(), "mp4") << std::endl;

	std::cout << "flags" 			   << std::endl;	
	std::cout << "\tpsnr: " << psnr_flag << std::endl;
	std::cout << "\tssim: " << ssim_flag << std::endl;
	std::cout << "\tvqm:  " << vqm_flag  << std::endl;

/* option parsing end */

	VideoReader *rR, *pR;
	
	 
	VideoCaptureReader o(reference);
	rR = &o;
	 
	VideoCaptureReader p(processed);
	pR = &p;
	 

	if(rR->getFps() != pR->getFps()){
		std::cout << "original and processed video have different frame rates, we abort here";
		return -1;
	}
	if(rR->getNFrames() != pR->getNFrames()){
		std::cout << "original and processed video have different number of frames, we abort here";
		return -1;
	}
	if(rR->getVideoWidth() != pR->getVideoWidth()){
		std::cout << "original and processed video have different widths, we abort here";
		std::cout << " (scaling not implemented yet)";
		return -1;
	}
	if(rR->getVideoHeight() != pR->getVideoHeight()){
		std::cout << "original and processed video have different heights, we abort here";
		std::cout << " (scaling not implemented yet)";
		return -1;
	}
	
	// process .2 seconds slices 
	int framesPerSlice = rR->getFps() * 0.2; 
std::cout << framesPerSlice << " frames per slice" << std::endl;

	remove( (out_prefix + "_psnr.csv").c_str() );
	remove( (out_prefix + "_ssim.csv").c_str() );
	remove( (out_prefix + "_vqm.csv").c_str() );


//reserving for 10000 slices,.,, later we will rertie to vectors 
	PSNR psnr(out_prefix + "_psnr", 1);
	SSIM ssim(out_prefix + "_ssim", 1);

	VQM vqm(10000, 12000); //just until we have vectors, todo rewrite to vectors

	cv::Mat read1[framesPerSlice];
	cv::Mat read2[framesPerSlice];
	cv::Mat read11[framesPerSlice];
	cv::Mat read21[framesPerSlice];

	cv::Mat ref[framesPerSlice][3];
	cv::Mat proc[framesPerSlice][3];
	
	cv::Mat tmp;

	bool frame_avail = true;
	int i; //frames grabbed

std::cout << rR->getVideoFilePath() << ";" << pR->getVideoFilePath() << std::endl;

	while(frame_avail){ 
		i = 0;
		while(i < framesPerSlice){
			frame_avail = rR->nextFrame(tmp);
			tmp.convertTo(read1[i], CV_32F);
			if(!frame_avail) break;
			pR->nextFrame(tmp);
			tmp.convertTo(read2[i], CV_32F);

			cv::cvtColor(read1[i], read11[i], CV_BGR2YCrCb);		
			cv::cvtColor(read2[i], read21[i], CV_BGR2YCrCb);	

			cv::split( read11[i], ref[i] ); 
			cv::split( read21[i], proc[i] ); 

			i++;
		}	
		if(!i) break; //no frames in slice

std::cout << "calculating over " << i << " frames: " ;
	
		if(psnr_flag)
std::cout	<<			psnr.compute(ref, proc, i);
		if(ssim_flag)
std::cout<<";"<<		ssim.compute(ref, proc, i);
		//if(vqm_flag) 
			//vqm.compute(in, out, framesGrabbed);

std::cout << std::endl;
	}

	std::cout << "cumulated PSNR over video " << psnr.getMetricValue() << std::endl;
	std::cout << "cumulated SSIM over video " << ssim.getMetricValue() << std::endl;

	return 0;	
}
