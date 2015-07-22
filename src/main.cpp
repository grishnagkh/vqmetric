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
#include "Y4MReader.hpp"
#include "PSNR.hpp"
#include "SSIM.hpp"
#include "VQM.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>


int printUsage(){
	
}

/* 
 * Sample call 
 *
 * 	./main -p 500.mp4 -r 1000.mp4 --psnr --ssim out
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
//	std::cout << "output file prefix: " << out_prefix << std::endl;

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
	/* delete output files if they already exist */
	remove( (out_prefix + "_psnr").c_str() ); 
	remove( (out_prefix + "_ssim").c_str() ); 
	remove( (out_prefix + "_vqm").c_str() ); 

//reserving for 10000 slices,.,, later we will rertie to vectors 
	PSNR psnr(out_prefix + "_psnr", 1);
	SSIM ssim(out_prefix + "_ssim", 1);

	VQM vqm(10000, 12000); //just until we have vectors

	cv::Mat inLum[framesPerSlice];
	cv::Mat outLum[framesPerSlice];
	cv::Mat in[framesPerSlice];
	cv::Mat out[framesPerSlice];
	
	cv::Mat tmp;

	bool frame_avail = 1;
	int framesGrabbed;

rR->nextFrame(tmp);
pR->nextFrame(tmp);

	while(frame_avail){ 
		framesGrabbed = 0;
		while(framesGrabbed < framesPerSlice){
			frame_avail = rR->nextFrame(in[framesGrabbed]);
			in[framesGrabbed].convertTo(inLum[framesGrabbed], CV_32F);

			pR->nextFrame(out[framesGrabbed]);
			out[framesGrabbed].convertTo(outLum[framesGrabbed], CV_32F);

			if(!frame_avail) break;
			framesGrabbed++;
		}	
		if(!framesGrabbed) break;

		if(psnr_flag)
			psnr.compute(inLum, outLum, framesGrabbed);
		if(ssim_flag)
			ssim.compute(inLum, outLum, framesGrabbed);
		if(vqm_flag) 
			vqm.compute(in, out, framesGrabbed);
	}

	std::cout << "cumulated PSNR over video " << psnr.getMetricValue() << std::endl;
	std::cout << "cumulated SSIM over video " << ssim.getMetricValue() << std::endl;

	return 0;	
}
