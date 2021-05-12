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

#include "Metric.hpp"
#include <opencv2/highgui/highgui_c.h>

/******** SI FILTER ********/

/* filter mask generator */
void Metric::getFilterMask(int len, float *filter){
	
/*
	Returns a horizontal bandpass filter
	len >=3 && len % 2 != 0
	vertical filter is the transpose of horizontal filter
*/

	int m = (len-1)/2;  //half filter width
	float c = 2.0*m/6;  
	for(int x=-m; x<=m; x++){
		filter[x+m] = (x/c)*exp(-(1/2)*(x/c)*(x/c)); //calculate		
	}	
	//normalize filter
	float co = 0;
	for(int x=0; x<len; x++){
		co += len * fabs(filter[x]);
	}
	co /= 8; 
	for(int x=0; x<len; x++){
		filter[x] /= co; 
	} 
}

void Metric::si_filter(cv::Mat& src, cv::Mat& si_filtered, cv::Mat& hfiltered,  cv::Mat& vfiltered, int len){
	
	float filterMask[len];
	float filterA [len][len];

	getFilterMask(len, filterMask);
	for(int i=0; i<len; i++){
		for(int j=0; j<len; j++){
			filterA[i][j] = filterMask[i];
		}
	}
	cv::Mat kernel = cv::Mat(len, len, CV_32F, &filterA);
	cv::Mat tmp;

	cv::filter2D(src, hfiltered, -1, kernel);
	cv::filter2D(src, vfiltered, -1, kernel.t());

	cv::Mat h2 = hfiltered.mul(hfiltered);
	cv::Mat v2 = vfiltered.mul(vfiltered);

	cv::add(h2,v2, tmp); 
	cv::sqrt(tmp, si_filtered);	 
}

/******** HV & HVBAR FILTER ********/
void Metric::hv_bar_filter(cv::Mat& si, cv::Mat& hSiFiltered, cv::Mat& vSiFiltered, cv::Mat& hvbar, cv::Mat& hv){
	/*  
		Originally the computation involves atan2 (slow): compute ratio between h & v, putting the smaller value on top and the larger value on the bottom.  Ignore divide by zero, because later code checking against rmin will catch that.  Essentially, fold angle into pi/4.
	*/

	double theta = .225;
	double ratio_treshold = tan(theta);
	double rmin = 20;

	hSiFiltered = cv::abs(hSiFiltered);
	vSiFiltered = cv::abs(vSiFiltered);
	
	cv::Mat ratio;
	cv::divide(min(vSiFiltered, hSiFiltered) , max(vSiFiltered, hSiFiltered), ratio);

	cv::Mat rat_t, base;

	/*
		// Threshold to Zero
		if src(x,y) > thresh
		  dst(x,y) = src(x,y)
		else
		  dst(x,y) = 0
		threshold(src, dst, tresh, -1, THRESH_TOZERO); //maxValue ignored

		// Inverse Binary Threshold
		if src(x,y) > thresh
		  dst(x,y) = maxValue
		else
		  dst(x,y) = 0
		threshold(src,dst, thresh, maxValue, THRESH_BINARY_INV);

	*/

	// apply perceptability threshold
	cv::threshold(si, base, rmin, -1, CV_THRESH_TOZERO);	
	// hv and hv bar criteria
	cv::threshold(ratio, rat_t, ratio_treshold, 1, CV_THRESH_BINARY);

	/* rat_t now contains 0 is the threshold is not exceeded, 1 otherwise... */
	hv = base.mul(rat_t);
	cv::subtract(base, hv, hvbar);
	
}

void Metric::filter_si_hv_bar(cv::Mat& src, cv::Mat& si, cv::Mat& hvbar, cv::Mat& hv, int len){
	cv::Mat hfil, vfil;
	si_filter(src, si, hfil, vfil ,len);
	hv_bar_filter(si, hfil, vfil, hvbar, hv);
}

void Metric::v(std::string msg, int level, int target_level){
	if(level > target_level)
		std::cout << msg << std::endl;
}

void Metric::v(int msg, int level, int target_level){
	if(level > target_level)
		std::cout << msg << std::endl;
}
 

void Metric::v(double msg, int level, int target_level){
	if(level > target_level)
		std::cout << msg << std::endl;
}

