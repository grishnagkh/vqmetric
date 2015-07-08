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

#ifndef psnr_hpp
#define psnr_hpp

#include "Metric.hpp"  

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
	public:
		/* 
		 * Computes PSNR metrics value for a video sequence 
 		 * according to the mean of the PSNR value for each
         * frame. 
		 */
		double compute(cv::Mat[], cv::Mat[], int);
	private:
		
};

#endif
