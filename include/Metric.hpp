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


#ifndef metric_hpp
#define metric_hpp

#include <iostream>
#include <fstream>

#include "opencv2/opencv.hpp" 

/*
 * This is the base Metric class for Full reference Metrics. It provides utilities 
 * to calculate objective image metrics.
 * Each Metric deriving from this class has to implement
 * the compute function, which performs the actual 
 * computation of the single metric
 */

class Metric{
	public:
	   /* 
	 	* the computation of a full reference metric takes place within this method 
		* param1: A sequence of images (reference sequence)
		* param2: A sequence of images (video sequence)
		* param3: Number of frames to process
		*/
		virtual double compute(cv::Mat[], cv::Mat[], int) = 0;
		/* 
		 * returns the metric value until now, 
		 * this is especially helpful when dealing with big files
		 */
		virtual double getMetricValue() = 0;
	 
	protected:		
		/******************************/
		/****** UTILITY functions *****/
		/******************************/

		/*
		 * Structural Information, HV, and HVBar filter 
		 *
		 * param1: Original image
		 * param2: SI filtered image
		 * param3: HVBar image
		 * param4: HV image
		 * param5: filter length for SI filter (will be expanded to len x len)
		 * 
		 */
		void filter_si_hv_bar(cv::Mat&, cv::Mat&, cv::Mat&, cv::Mat&, int);
		/* 
		 * Calculates HV Bar and hv filtered image from 
		 * a given SI filtered image and its horizontal 
		 * and vertical components
		 * 
		 * param1: (full) SI filtered image 
		 * param2: (horizontally) SI filtered image 
		 * param3: (vertically) SI filtered image 
		 * param4: hvbar image
		 * param5: hv image
		 */
		void hv_bar_filter(cv::Mat&, cv::Mat&, cv::Mat&, cv::Mat&, cv::Mat&);
		/*
		 * calculate a SI filtered image 
		 * param1: source image
		 * param2: SI filtered image
		 * param3: horizontal component SI filtered image 
		 * param4: vertical component SI filtered image 
		 * param5: filter length
		 */
		void si_filter(cv::Mat&, cv::Mat&, cv::Mat&, cv::Mat&, int);
		/*
		 * Calculates a bandbass filter (assuming len >=3 && len % 2 != 0)
		 *
		 * param1: filter lengh
		 * param2: filter
		 */
		void getFilterMask(int, float*); 
};

#endif
