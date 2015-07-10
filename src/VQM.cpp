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

#include "VQM.hpp" 

VQM::VQM(int nSlices){
	this->nSlices = nSlices;
	this->si_loss = new double[nSlices]();
	this->si_gain = new double[nSlices]();
	this->hv_loss = new double[nSlices]();
	this->hv_gain = new double[nSlices]();
	this->chroma_spread = new double[nSlices]();
	this->ct_ati_gain = new double[nSlices]();
	this->chroma_extreme = new double[nSlices]();
};



double VQM::compute(cv::Mat orig[], cv::Mat processed[], int nFrames){
/***** TODO *****/	
	return -1;
}

double VQM::timeCollapse(){
/***** TODO *****/
	return -1;
}
double VQM::getMetricValue(){
/***** TODO *****/
	return -1;
}

double VQM::euclideansq(double fo1, double fo2, double fp1, double fp2){
	/* returns the square of the euclidean distance */
	double t1 = (fo1 - fp1) ;
	double t2 = (fo2 - fp2) ;
	return t1*t1 + t2*t2;
}
double VQM::ratioComp(double orig, double processed){
	return (processed-orig)/orig;
}
double VQM::logComp(double orig, double processed){
	return log10(processed/orig);
}
 
double VQM::clip(double f, double threshold){
	if(f >= threshold)
		return f;
	return threshold;
} 
