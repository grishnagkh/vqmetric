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

#include "opencv2/opencv.hpp"
#include "VideoCaptureReader.hpp"
#include "PSNR.hpp"
#include "SSIM.hpp"

using namespace std;

int main(int ac, const char** av){
 
	VideoCaptureReader orig("out1.mp4");
	VideoCaptureReader processed("out2.mp4");

	int n=60;		

	cv::Mat in[n];
	cv::Mat out[n];

	cv::Mat tmp;	 

	for(int i=0; i<n; i++){
		orig.nextFrame(tmp);
		tmp.convertTo(in[i], CV_32F);
		processed.nextFrame(tmp);
		tmp.convertTo(out[i], CV_32F);		
	}

	PSNR psnr;
	SSIM ssim;

	std::cout<< "PSNR of first " <<  n << " frames: " << psnr.compute(in, out, n) <<endl;
	std::cout<< "SSIM of first " <<  n << " frames: " << ssim.compute(in, out, n) <<endl;
 
	
	return 0;	
}
