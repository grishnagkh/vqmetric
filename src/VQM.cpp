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
	
/* TODO: 
	(1) extract 
		gain, 
		loss 
		perceptability threshold 
		variance of s-region
		mean of s-region
		[ for readability and less error prone coding :) ]
	(2) test si_loss, si_gain
	(3) implement rest
	(4) maybe only spatial collapse and we save each feature stream?

	//meet br: i think the variance has to be calculated wrt all frames in 0.2s segment...
*/
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
	cv::Mat siSdOLoss[nFrames], siSdPLoss[nFrames];
	cv::Mat siSdOGain[nFrames], siSdPGain[nFrames];

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
	cv::Mat comparedFeatureStreamGain[nFrames];

	/* array for extracted 8x8 grid values */
	int slen = nFrames*(w/stdimx)*(h/stdimy);
	double streamSILoss[slen];
	double streamSIGain[slen];
	
	int cnt=0;
	double thresh;

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
		
		thresh = 12;
		cv::threshold(siSdO[i], t1, thresh, 1, CV_THRESH_BINARY_INV); // 1 if v < 12, 0 otherwise
		cv::threshold(siSdO[i], siSdOLoss[i], thresh, -1, CV_THRESH_TOZERO); //clear if  v < 12
		siSdOLoss[i] = siSdOLoss[i] + thresh*t1; // add 12 if v<12

		cv::threshold(siSdP[i], t1, thresh, 1, CV_THRESH_BINARY_INV); // 1 if v < 12
		cv::threshold(siSdP[i], siSdPLoss[i], thresh, -1, CV_THRESH_TOZERO); //clear if  v < 12
		siSdPLoss[i] = siSdPLoss[i] + thresh*t1; // add 12 if v<12

		/* [si_gain (2) ] apply a perceptability threshold, replacing values less than 8 with 8 */
		thresh = 8;

		cv::threshold(siSdP[i], t1, thresh, 1, CV_THRESH_BINARY_INV); // 1 if v < 8
		cv::threshold(siSdP[i], siSdPGain[i], thresh, -1, CV_THRESH_TOZERO); //clear if  v < 8
		siSdPGain[i] = siSdPGain[i] + thresh*t1; // add 8 if v<8	

		cv::threshold(siSdO[i], t1, thresh, 1, CV_THRESH_BINARY_INV); // 1 if v < 8
		cv::threshold(siSdO[i], siSdOGain[i], thresh, -1, CV_THRESH_TOZERO); //clear if  v < 8
		siSdOGain[i] = siSdOGain[i] + thresh*t1; // add 8 if v<8	


		/* [si_loss (5)] compare original and processed feature streams using ratio comparison function followed by loss function */	
		cv::divide(siSdPLoss[i] - siSdOLoss[i], siSdOLoss[i], comparedFeatureStream[i]); 	//ratio compare
	
		/* loss function */ 
		cv::threshold(comparedFeatureStreamLoss[i], comparedFeatureStreamLoss[i], 0, -1, CV_THRESH_TOZERO_INV);		
		
		/* [si_gain (3) ] compare original and processed feature streams using log comparison function followed by gain function*/ 
		

		/* log comparison: log10(processed/orig) */
		// = ln(processed/orig)/ln(10) ; ln(10) ~ 2.30258509299
		cv::log(siSdOGain[i],t1); //ln(o)
		cv::log(siSdPGain[i],t2); //ln(p)
		cv::subtract(t2, t1, comparedFeatureStreamGain[i]); //ln(p/o) = ln(p) - ln(o)
		comparedFeatureStreamGain[i] /= 2.30258509299; //log10(p/o)

		/* gain function */
		cv::threshold(comparedFeatureStreamGain[i], comparedFeatureStreamGain[i], 0, -1, CV_THRESH_TOZERO);


		/* extract 8x8 grid for si_loss*/
		for(int y=0; y<h; y+=stdimy){
			for(int x=0; x<w; x+=stdimx){
				streamSIGain[cnt] = comparedFeatureStreamGain[i].at<float>(x,y);				
				streamSILoss[cnt++] = comparedFeatureStreamLoss[i].at<float>(x,y);
			}
		}

		
	}

	/* [si_loss (6)] spatially collapse by computing the average of the worst (i.e. most impaired) 5% of S-T blocks for each 0.2 second slice of time */	

	std::sort(streamSILoss, streamSILoss+sizeof(streamSILoss)/sizeof(streamSILoss[0]));
	int until = slen*.05;
	double si_avg = 0;
	for(int i=0; i<until; i++){ 
		si_avg += streamSILoss[i];
	}
	si_avg /= until;	

	si_loss[actSlice] = si_avg;

		
	/* 
	 * [si_gain (4) ] spatially and temporally collapse by computing the average of all block and the clip at a minimum value of 0.004
	 * [si_gain (5) ] set all values greater than 0.14 to 0.14
	 */

		//discussion: 4/5 are shifted partially into timecollapse or getmetricvalue
		//for now we only spatially collapse for a 0.2 second block
	double si_gain_avg = 0;
	for(int i=0; i<slen; i++){ 
		si_gain_avg += streamSIGain[i];
	}
	si_gain_avg /= slen;		
	si_gain[actSlice] = si_gain_avg; 
	/* si_gain now contains the spatially collapsed 0.2second block collapsed time slice */

	actSlice++;	

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
