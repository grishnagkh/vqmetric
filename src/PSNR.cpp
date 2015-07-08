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

#include "PSNR.hpp" 

double PSNR::compute(cv::Mat orig[], cv::Mat processed[], int nFrames){
	double sum = 0;
	for(int i=0; i<nFrames;i++){
		sum += computeSingleFrame(orig[i], processed[i]);
	}
	return sum/nFrames; 
}

double PSNR::computeSingleFrame(cv::Mat& orig, cv::Mat& processed){
	cv::Mat tmp; 
	cv::subtract(orig, processed, tmp);
	tmp = tmp.mul(tmp);
	double mse = cv::mean(tmp).val[0];	
	if(mse <= 1e-10){
		mse = 1e-15; // => psnr ~200 if frames identical to avoid division by zero
	}
	return 10.0 * log10((255 * 255) / mse);
}
