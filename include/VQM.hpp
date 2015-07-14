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

#ifndef vqm_hpp
#define vqm_hpp

#include "Metric.hpp"  
#include <math.h>

/*
 * Video Quality Metric, implementation is based on
 *
 * Pinson, Margaret H., and Stephen Wolf. 
 * "A new standardized method for objectively measuring video quality." 
 * Broadcasting, IEEE Transactions on 50.3 (2004): 312-322
 *
 */

/***** TODO *****/

class VQM: public Metric{
	double* si_loss;
	double* si_gain;
	double* hv_loss;
	double* hv_gain;
	double* chroma_spread;
	double* ct_ati_gain;
	double* chroma_extreme;

	int nSlices;
	int actSlice; 

	static const double FACTOR_SI_LOSS = -0.2097;
	static const double FACTOR_SI_GAIN = -2.3416;
	static const double FACTOR_HV_LOSS = 0.5969;
	static const double FACTOR_HV_GAIN = 0.2483;
	static const double FACTOR_CHROMA_SPREAD = 0.0192;
	static const double FACTOR_CT_ATI_GAIN = 0.0431;
	static const double FACTOR_CHROMA_EXTREME = 0.0076;

	public:
		/* 
		 * Computes VQM values
		 * as the Video Quality Metric is time sensitive, 
		 * we need a final cumulation of the results and 
 		 * the return value only indicates success of the computation
		 *
		 * argument 1: reference image sequence
		 * argument 2: processed image sequence
		 * argument 3: number of fraems to process
		 * 
		 * the  image sequences are split into space regions, 
		 * the time cumulation is done in getMetricValue, 
		 * space cumulation is done here, therefore the length of 
		 * a time region is determined by the input to this function
		 * this enables that we do not need to keep the whole 
		 * video sequences in memory, but only a fraction with the duration 
		 * of the S-T region (recommended: 0.2s = fps*0.2 frames)
		 * 
		 * the result of a single computation round are stored 
		 * internally in the VQM object
		 */
		double compute(cv::Mat[], cv::Mat[], int);
		/* 
		 * returns the metric value until now, 
		 * this is especially helpful when dealing with big files
		 */
		double getMetricValue();
		/*
		 * cumulates the results
		 */
		double timeCollapse();

		/* 
		 * Constructor
		 * parameter1: number of time slices for the computation 
		 *
		 */
		VQM(int);
	private:		
		double perc_thresh(double, double);
		double calc_mean(float*, int);
		double calc_sd(float*, int);
		/* (squared) euclidean distance of two points */
		double euclideansq(double, double, double, double);
		/* ratio comparison */
		double ratioComp(double, double);
		/* logarithmic comparison */ 
		double logComp(double, double);
		/* clip a number to a threshold*/
		double clip(double, double); 
		
		double gain(double d);
		double loss(double d);
		
};

#endif
