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
	this->actSlice = 0;
	this->nSlices = nSlices;
	this->si_loss = new double[nSlices]();
	this->si_gain = new double[nSlices]();
	this->hv_loss = new double[nSlices]();
	this->hv_gain = new double[nSlices]();
	this->chroma_spread = new double[nSlices]();
	this->ct_ati_gain = new double[nSlices]();
	this->chroma_extreme = new double[nSlices]();
};

/* 
 * computes parts of VQM metrics for a .2 second slice 
 */

//loss function: replace positive values with zero
		//gain function: replace negative values with zero
			

double VQM::compute(cv::Mat orig[], cv::Mat processed[], int nFrames){
	/* input: original frames */ 
	
	/* 
	 * luminance planes for 
	 * 	si_loss, hv loss,hv gain, si gain, ct ati gain
	 *
	 * chroma planes for
	 * 	chroma spread and chroma extreme
  	 */
	cv::Mat lumO[nFrames], lumP[nFrames];
	cv::Mat siO[nFrames], siP[nFrames];
	cv::Mat hvO[nFrames], hvP[nFrames];
	cv::Mat hvBarO[nFrames], hvBarP[nFrames];

	cv::Mat siSdO[nFrames], siSdP[nFrames];

	int h = orig[0].rows;
	int w = orig[0].cols;
	
	/* kernel to calculate mean of a stdimx \times stdimy region*/
	int stdimx = 8; /* kernel size x */
	int stdimy = 8; /* kernel size y */
	cv::Point anchor = cv::Point(-stdimx/2,-stdimy/2); 		/* anchor @ top left corner */
 	/* the actual kernel*/
	cv::Mat mean_kernel = cv::Mat::ones(stdimx, stdimy, CV_32F) /  (float)(stdimx*stdimy);
	
	/* temporary images */
	cv::Mat t1, t2; 
	/* a stream of compared images */
	cv::Mat comparedFeatureStream[nFrames];
	cv::Mat comparedFeatureStreamLoss[nFrames];
//	cv::Mat comparedFeatureStreamGain[nFrames];

	/* array for extracted 8x8 grid values */
	int slen = nFrames*(w/stdimx)*(h/stdimy);
	double streamSILoss[slen];
	
	int cnt=0;

	for(int i=0; i<nFrames; i++){
		orig[i].convertTo(lumO[i], CV_32F);
		processed[i].convertTo(lumP[i], CV_32F);
	 	/*  apply SI13 filter to each luminance plane */
		filter_si_hv_bar(lumO[i], siO[i], hvBarO[i], hvO[i], 13);
		filter_si_hv_bar(lumP[i], siP[i], hvBarP[i], hvP[i], 13);
		
		/* [si:loss (3) ] Compute the standard deviation of each S-T region */
	
		filter2D(siO[i], t1, -1 , mean_kernel, anchor ); //E(X)
		filter2D(siO[i].mul(siO[i]), t2, -1 , mean_kernel, anchor ); // E(X^2)
		cv::sqrt(t2 - t1.mul(t1), siSdO[i]); // var(X) = E(X^2) - [E(X)]^2

		filter2D(siP[i], t1, -1 , mean_kernel, anchor ); //E(X)
		filter2D(siP[i].mul(siP[i]), t2, -1 , mean_kernel, anchor ); // E(X^2)
		cv::sqrt(t2 - t1.mul(t1), siSdP[i]); // var(X) = E(X^2) - [E(X)]^2
		
		/*  [si_loss (4)] apply a perceptability threshold, replacing values less than 12 with 12*/	
		cv::threshold(siSdP[i], siSdP[i], 12, -1, CV_THRESH_BINARY);
		cv::threshold(siSdO[i], siSdO[i], 12, -1, CV_THRESH_BINARY);

		/* [si_loss (5)] compare original and processed feature streams using ratio comparison function followed by loss function */	
		cv::divide(siSdP[i] - siSdO[i], siSdO[i], comparedFeatureStream[i]); 	//ratio compare
	
		/* loss function */ 
		cv::threshold(comparedFeatureStreamLoss[i], comparedFeatureStream[i], 0, -1, CV_THRESH_TOZERO_INV);		

		/* extract 8x8 grid */
		for(int y=0; y<h; y+=stdimy){
			for(int x=0; x<w; x+=stdimx){
				streamSILoss[cnt++] = comparedFeatureStreamLoss[i].at<float>(x,y);
			}
		}
	}

	/* [si_loss (6)] spatially collapse by computing the average of the worst (i.e. most impaired) 5% of S-T blocks for each 0.2 second slice of time */
	
	std::sort(streamSILoss, streamSILoss+sizeof(streamSILoss)/sizeof(streamSILoss[0]));
	int until = slen*.05;
	double si_avg = 0;
	for(int i=0; i<until; i++){ 
		si_avg = streamSILoss[i];
	}
	si_avg /= until;	

	si_loss[actSlice] = si_avg;

	actSlice++;	


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
