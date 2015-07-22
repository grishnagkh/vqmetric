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

#ifndef ssim_hpp
#define ssim_hpp

#include "Metric.hpp" 

/*
 * Structured Similarity (SSIM) Metric 
 */

class SSIM: public Metric{

	int* nProcessed;
	double* values; 
	int actSlice;
	int nSlices;
	std::string logfile_path;
	int loglevel;

	public:
		/* 
		 * Computes SSIM metrics value for a video sequence 
 		 * according to the mean of the SSIM value for each
         * frame. 
		 */
		double compute(cv::Mat[], cv::Mat[], int);
		/* 
		 * returns the metric value
		 */
		double getMetricValue();

		/*
		 * Constructor, param: number of slices
		 */
		SSIM(int, std::string, int);
	private:
		/* 
		 * Computes SSIM Value for a pair of images. 
 		 */
		double computeSingleFrame(const cv::Mat&, const cv::Mat&);
		/*
		 * add calculation results of a slice...
		 */
		void addCalculation(double, int);
};

#endif
