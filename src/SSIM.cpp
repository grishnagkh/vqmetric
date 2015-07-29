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

#include "SSIM.hpp"



SSIM::SSIM(std::string logfile_path, int loglevel){
	this->logfile_path = logfile_path;
	this->loglevel = loglevel;
}

/* number of frames to cumulate */
double SSIM::timeCollapse(int nSlices){
	ssim_cumulated.clear();

	double sum;
	int n;

	for(std::vector<int>::size_type begin = 0; begin < values.size(); begin+=nSlices) {
		sum = 0;
		n = 0;
		for(int i = 0; i < nSlices && i + begin < values.size(); i++){
			sum += values[begin + i]*nProcessed[begin + i];		
			n += nProcessed[begin + i];
		}
		ssim_cumulated.push_back(sum/n);	
	}
	return 0;
}


/*
 * Collapse metric value in time, luckily we just have to calculate weighted average...
 */	
double SSIM::getMetricValue(std::vector<double> *results){
	results->clear();
	for(std::vector<int>::size_type i = 0; i < ssim_cumulated.size(); i+=1) {
		results->push_back(ssim_cumulated[i]);
	}	

	return 0;

} 

double SSIM::compute(cv::Mat orig[][3], cv::Mat processed[][3], int nFrames){
	std::ofstream logfile;
	logfile.open ((logfile_path).c_str(), std::ios::out | std::ios::app ); //open in append mode

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
	double ssim = sum/nFrames;
	addCalculation(ssim, nFrames);	
	return ssim; 
}

void SSIM::addCalculation(double val, int nFrames){
	values.push_back(val);
	nProcessed.push_back(nFrames);
}
double SSIM::computeSingleFrame( const cv::Mat& I1, const cv::Mat& I2){

    const double C1 = 6.5025, C2 = 58.5225;
    /***************************** INITS **********************************/

    cv::Mat I2_2   = I2.mul(I2);        // I2^2
   	cv::Mat I1_2   = I1.mul(I1);        // I1^2
    cv::Mat I1_I2  = I1.mul(I2);        // I1 * I2

    /*************************** END INITS **********************************/

    cv::Mat mu1, mu2;                   // PRELIMINARY COMPUTING
    cv::GaussianBlur(I1, mu1, cv::Size(11, 11), 1.5);
    cv::GaussianBlur(I2, mu2, cv::Size(11, 11), 1.5);

    cv::Mat mu1_2   =   mu1.mul(mu1);
    cv::Mat mu2_2   =   mu2.mul(mu2);
    cv::Mat mu1_mu2 =   mu1.mul(mu2);

   	cv::Mat sigma1_2, sigma2_2, sigma12;

    cv::GaussianBlur(I1_2, sigma1_2, cv::Size(11, 11), 1.5);
    sigma1_2 -= mu1_2;

    cv::GaussianBlur(I2_2, sigma2_2, cv::Size(11, 11), 1.5);
    sigma2_2 -= mu2_2;

    cv::GaussianBlur(I1_I2, sigma12, cv::Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;

    ///////////////////////////////// FORMULA ////////////////////////////////
    cv::Mat t1, t2, t3;

    t1 = 2.0 * mu1_mu2 + C1;
    t2 = 2.0 * sigma12 + C2;
    t3 = t1.mul(t2);                 // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))

    t1 = mu1_2 + mu2_2 + C1;
    t2 = sigma1_2 + sigma2_2 + C2;
    t1 = t1.mul(t2);                 // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))

    cv::Mat ssim_map;
    cv::divide(t3, t1, ssim_map);        // ssim_map =  t3./t1;

	cv::Scalar mssim = cv::mean(ssim_map);   // mssim = average of ssim map

	return mssim.val[0];
}
