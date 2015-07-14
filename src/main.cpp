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
#include "PSNR.hpp"
#include "SSIM.hpp"
#include "VQM.hpp"

int main(int ac, const char *av[]){

	if(ac != 3){
		std::cout << "Wrong unmber of parameters: expected <input filname> <output filename>";
		return -1;
	}
 
	VideoCaptureReader orig(av[1]);
	VideoCaptureReader processed(av[2]);

	if(orig.getFps() != processed.getFps()){
		std::cout << "original and processed video have different frame rates, we abort here";
		return -1;
	}
	if(orig.getVideoWidth() != processed.getVideoWidth()){
		std::cout << "original and processed video have different widths, we abort here";
		return -1;
	}
	if(orig.getVideoHeight() != processed.getVideoHeight()){
		std::cout << "original and processed video have different heights, we abort here";
		return -1;
	}
	if(orig.getNFrames() != processed.getNFrames()){
		std::cout << "original and processed video have different number of frames, we abort here";
		return -1;
	}
	

	// 2 seconds slices
	int framesPerSlice = orig.getFps() * 0.2;

//	int nSlices = orig.getNFrames() / framesPerSlice; //testing
	int nSlices = 0;

	if(orig.getNFrames() % framesPerSlice != 0){
		nSlices++; 
	}	

	std::cout << "Splitting calculation in " << nSlices << " 0.2second slices with " << framesPerSlice << " frames" << std::endl;

	PSNR psnr(nSlices);
	SSIM ssim(nSlices);

	VQM vqm(nSlices);
	

	cv::Mat inLum[framesPerSlice];
	cv::Mat outLum[framesPerSlice];
	cv::Mat in[framesPerSlice];
	cv::Mat out[framesPerSlice];
	
	cv::Mat tmp;	

	for(int slice=0; slice<nSlices; slice++){

		std::cout << "Decompressing and converting videos of slice " << slice << endl;

		clock_t before, after;

		before = clock();

		for(int i=0; i<framesPerSlice; i++){
			orig.nextFrame(in[i]);
			in[i].convertTo(inLum[i], CV_32F);
			processed.nextFrame(out[i]);
			out[i].convertTo(outLum[i], CV_32F);		
		}

	
		after = clock();
		std::cout << "decompression and conversion finished in " << double(after-before) / CLOCKS_PER_SEC << "s" <<endl;	

// 		before = clock();
//		std::cout << "calculating PSNR for slice " << slice << endl;
		std::cout<< "PSNR for slice " << slice << ": " << psnr.compute(inLum, outLum, framesPerSlice) <<endl;
//		after = clock();
//		std::cout << "calculation of PSNR for slice " << slice << " finished in " << double(after-before) / CLOCKS_PER_SEC << "s" <<endl;	

//		before = clock();
//		std::cout << "calculating SSIM for slice " << slice << endl;
		std::cout<< "SSIM for slice " << slice << ": " << ssim.compute(inLum, outLum, framesPerSlice) <<endl;
//		after = clock();		
//		std::cout << "calculation of SSIM for slice " << slice << " finished in " << double(after-before) / CLOCKS_PER_SEC << "s" <<endl;	

		vqm.compute(in, out, framesPerSlice);
	}
	std::cout << "..............." << std::endl;

	std::cout << "PSNR for all slices " << psnr.getMetricValue() << std::endl;
	std::cout << "SSIM for all slices " << ssim.getMetricValue() << std::endl;

	return 0;	
}
