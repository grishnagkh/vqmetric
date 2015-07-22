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
#include <vector>

PSNR::PSNR(std::string logfile_path, int loglevel){
	this->logfile_path = logfile_path;
	this->loglevel = loglevel;	
}

double PSNR::compute(cv::Mat orig[][3], cv::Mat processed[][3], int nFrames){
	std::ofstream logfile;
	logfile.open ((logfile_path + ".csv").c_str(), std::ios::out | std::ios::app ); //open in append mode

	double sum = 0;
	double tmp = 0;
	for(int i=0; i<nFrames;i++){
		tmp = computeSingleFrame(orig[i][0], processed[i][0]);		 //[0].. only on luma channel
		sum += tmp;
		if(loglevel == 1){
			logfile << tmp << std::endl;
		}
	}
	logfile.close();

	double psnr = sum/nFrames;	
	addCalculation(psnr, nFrames);	
	return psnr; 
}

double PSNR::computeSingleFrame(cv::Mat& reference, cv::Mat& processed){
	cv::Mat tmp;
	cv::subtract(reference, processed, tmp);
	tmp = tmp.mul(tmp);
	double mse = cv::mean(tmp).val[0];	
	if(mse <= 1e-10){
		mse = 1e-10; // => psnr clip to ~148
	}	

	return 10.0 * log10((255 * 255) / mse);
}
void PSNR::addCalculation(double val, int nFrames){
	values.push_back(val);
	nProcessed.push_back(nFrames);
}

/*
 * Collapse metric value in time, luckily we just have to calculate weighted average...
 */	
double PSNR::getMetricValue(){
	double sum = 0;
	int n = 0;
	
	for(std::vector<int>::size_type i = 0; i != values.size(); i++) {
		sum += values[i]*nProcessed[i];		
		n += nProcessed[i];
	}
	return sum / n;
}
