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
 
double PSNR::timeCollapse(int nSlices){
	psnr_cumulated.clear();

	double sum;
	int n;

	for(std::vector<int>::size_type begin = 0; begin < values.size(); begin+=nSlices) {
		sum = 0;
		n = 0;
		for(int i = 0; i < nSlices && i + begin < values.size(); i++){
			sum += values[begin + i]*nProcessed[begin + i];		
			n += nProcessed[begin + i];
		}
		psnr_cumulated.push_back(sum/n);	
	}
	return 0;
}

double PSNR::getMetricValue(std::vector<double> *results){
	for(std::vector<int>::size_type i = 0; i < psnr_cumulated.size(); i+=1) {
		results->push_back(psnr_cumulated[i]);
	}	

	return 0;

}

double PSNR::compute(cv::Mat orig[][3], cv::Mat processed[][3], int nFrames){
	std::ofstream logfile;
	logfile.open ((logfile_path).c_str(), std::ios::out | std::ios::app ); //open in append mode

	double sum = 0;
	double tmp = 0;
	for(int i=0; i<nFrames;i++){
		tmp = computeSingleFrame(orig[i][0], processed[i][0]);		 //[0].. only on luma channel
		sum += tmp;
		if(loglevel > 0){ //log by frame
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
	// Attention. assuming 8bit samples for reference
	// this means, if processing 16 bit values, we are ~48dB off
	// 		20*(log(2^8 -1)-log(2^16-1))/log(10) = 48.19866
	// general formula for offset calculation with samples of B bits: 
	// 		20*(log(2^8-1))-log(2^B-1))/log(10)
	return 10.0 * log10((255 * 255) / mse);
}
void PSNR::addCalculation(double val, int nFrames){
	values.push_back(val);
	nProcessed.push_back(nFrames);
}



