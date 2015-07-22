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

VQM::VQM(int nSlices, int totalFrames){
	this->actSlice = 0;
	this->nSlices = nSlices;
	this->si_loss = new double[nSlices]();
	this->si_gain = new double[nSlices]();
	this->hv_loss = new double[nSlices]();
	this->hv_gain = new double[nSlices]();
	this->chroma_spread = new double[totalFrames]();
	this->ct_ati_gain = new double[nSlices]();
	this->chroma_extreme = new double[nSlices]();
};

/* 
 * computes parts of VQM metrics for a .2 second slice 
 */	
double VQM::compute(cv::Mat orig[][3], cv::Mat processed[][3], int nFrames){
	/* input: original frames */ 
std::cout << "starting vqm calculation" << std::endl;

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

	int h = orig[0][0].rows;
	int w = orig[0][0].cols;


/*
TODO
chroma_spread
			(1) Divide the CB and CR color planes into separate 8 pixel x 8 line x 1 frame S-T regions.
			(2) Compute the mean of each S-T region. Multiple the CR means by 1.5 to increase the perceptual weighting of the red color component in the next step.
			(3) Compare original and processed feature streams CB and CR using Euclidean distance (see equation 2).
			(4) Spatially collapse by computing the standard deviation of blocks for each 1-frame slice of time.
			(5) Temporally collapse by sorting the values in time and selecting the 10% level, and then clip at a minimum value of 0.6. Since all values are positive, this represents a bestcase 	processing temporally. Thus, chroma_spread measures color impairments that are nearly always present
*/

//orig, processed are the original image sequences



	/* kernel to calculate mean of a stdimx \times stdimy region*/
	int stdimx = 8; /* kernel size x */
	int stdimy = 8; /* kernel size y */
	cv::Point anchor = cv::Point(0,0); 		/* anchor @ top left corner */
	/* the actual kernel*/
	cv::Mat mean_kernel = cv::Mat::ones(stdimx, stdimy, CV_32F) /  (float)(stdimx*stdimy);
	cv::Mat exo;
	cv::Mat exp;
	
	

cv::Mat ochannels[3];
cv::Mat pchannels[3];


	float chromafeature[nFrames*w/stdimx*h/stdimy];
int countaeeeer = 0;
	for(int i=0; i<nFrames; i++){


			/*	(1) Divide the CB and CR color planes into separate 8 pixel x 8 line x 1 frame S-T regions. */	
  
lumO[i] = orig[i][0];
lumP[i] = processed[i][0];

			/* (2) Compute the mean of each S-T region. Multiple the CR means by 1.5 to increase the perceptual weighting of the red color component in the next step. */
		cv::filter2D(orig[i][0], exo, -1 , mean_kernel, anchor );
		cv::filter2D(processed[i][0], exp, -1 , mean_kernel, anchor );
	
cv::split( exo, ochannels ); 
cv::split( exp, pchannels ); 
ochannels[1].mul(1.5);
pchannels[1].mul(1.5);

			/*	(3) Compare original and processed feature streams CB and CR using Euclidean distance (see equation 2). */
		
float cro, crp, cbo, cbp;
int xpos, ypos;
for(int y=0; y<h/stdimy; y++){
	for(int x=0; x<w/stdimx; x++){					
		xpos = stdimx*x;
		ypos = stdimy*y;
		cro = ochannels[1].at<float>(xpos, ypos);
		cbo = ochannels[2].at<float>(xpos, ypos);
		crp = pchannels[1].at<float>(xpos, ypos);
		cbp = pchannels[2].at<float>(xpos, ypos);
		chromafeature[countaeeeer++] = sqrt(euclideansq(cro, cbo, crp, cbp)); //we want the sqrt here i think..  and i hope that's right :D :D :D just check it in this way...
	}
}
	}

	
/* (4) Spatially collapse by computing the standard deviation of blocks for each 1-frame slice of time. 
	we temporally collapse here for the actual sclice
*/
	
	
	float *fptr = chromafeature;

	int bpf = w/stdimx*h/stdimy; //blocks per frame
	
double ex, ex2;
	for(int  i=0; i<nFrames;i++){
		ex = calc_mean(fptr, bpf);
		ex2 = calc_mean_squared(fptr, bpf);
		chroma_spread[nFrames*actSlice+i] = ex2 - ex*ex;

		fptr += bpf;
	}

//todo.. iterative update of 

//temporal collapse in getMetricValue...


// var(X) = E(X^2) - [E(X)]^2


/*

chroma_extreme

	(1) Perform steps 1 through 3 from chroma_spread.
	(2) Spatially collapse by computing for each slice of time the average of the worst 1% of blocks (i.e., rank-sorted values from the 99% level to the 100% level), and subtract from that result the 99% level. This identifies very bad distortions that impact a small portion of the image.
	(3) Temporally collapse by computing standard deviation of the results from step 2 //this is meant over the whole video, so we must think of something ...

	//these averages of variances are tackled with

	// Xi ... data available for slice i, i.e. a single call of compute
	// X = X1 + X2 + ... + Xi + ... + Xn		// + = union
	// EXi = mean(Xi)
	// E2Xi = mean(Xi*Xi)
	// save, [EXi, E2Xi, length(Xi)]

	// final calculation: 
	// var(X) = sum i=1..n  length(Xi)/length(X) * E2Xi-(EXi*EXi)
		

*/

//chroma extreme... 
/* (1) Perform steps 1 through 3 from chroma_spread. */ 
	// stored in chromafeature
/* (2) Spatially collapse by computing for each slice of time the average of the worst 1% of blocks (i.e., rank-sorted values from the 99% level to the 100% level), and subtract from that result the 99% level. This identifies very bad distortions that impact a small portion of the image. */
	//we only have this slice of time here, temporal collapse done later....
	std::sort(chromafeature, chromafeature+sizeof(chromafeature)/sizeof(chromafeature[0]));
	int onePerc = 0.01 * nFrames*w/stdimx*h/stdimy; // 1 % of elements
	int tmpAvg = 0;
	for(int i = 0; i<onePerc; i++){
		tmpAvg += chromafeature[(nFrames*w/stdimx*h/stdimy-1)-i]; //last 1%
	}
	tmpAvg /= onePerc;
	
chroma_extreme[actSlice] = tmpAvg;




std::cout << "[debug] transforming to cv32_f and calculating si, hv, and hvbar filters" << std::endl;
	cv::Mat tmp;
	for(int i=0; i<nFrames; i++){	
	 	/* [si_loss (1)] apply SI13 filter to each luminance plane */
		/* [hv_loss (1)] apply the HV and HVBAR perceptual filters to each luminance plane. */
		filter_si_hv_bar(lumO[i], siO[i], hvBarO[i], hvO[i], 13);
		filter_si_hv_bar(lumP[i], siP[i], hvBarP[i], hvP[i], 13);	
	}
std::cout << "[debug] transformation to cv32_f and calculating si, hv, and hvbar filters ended" << std::endl;


	/* split S-T regions, assuming that we are given a single time slice with nFrames */
	int rW = 8;
	int rH = 8;
	int nRegions = w / rW * h / rH;
	int elemPerRegion = rW*rH*nFrames;

	int region;
	int idx;
	
	float** siRegO = new float*[nRegions];
	float** siRegP = new float*[nRegions];
	float** hvRegO = new float*[nRegions];
	float** hvRegP = new float*[nRegions];
	float** hvBarRegO = new float*[nRegions];
	float** hvBarRegP = new float*[nRegions];

	for(int i=0; i<nRegions; i++){
		siRegO[i] = new float[elemPerRegion];
		siRegP[i] = new float[elemPerRegion];
		hvRegO[i] = new float[elemPerRegion];
		hvRegP[i] = new float[elemPerRegion];
		hvBarRegO[i] = new float[elemPerRegion];
		hvBarRegP[i] = new float[elemPerRegion];
	}


	region = 0;
	int idxX, idxY;

std::cout << "[debug] splitting into " << nRegions << " s-t regions with " << elemPerRegion << " elements per region" << std::endl;

	/* [si_loss(2), hv_loss (2)] Divide each of the SI, HV and HVBAR video sequences into 8 pixel x 8 line x 0.2 second S-T regions. */	

	for(int y=0; y<h/rH; y++){
		for(int x=0; x<w/rW; x++){
			// each region
			idx = 0;
			for(int i=0; i<nFrames; i++){
	
				//each input frame 
				for(int dy=0; dy<rH; dy++){
					for(int dx=0; dx<rW; dx++){
						//8x8 window
						idxX = rW*x+dx;
						idxY = rH*y+dy;
						siRegO[region][idx] = siO[i].at<float>(idxX, idxY);
						siRegP[region][idx] = siP[i].at<float>(idxX, idxY);
						hvRegO[region][idx] = hvO[i].at<float>(idxX, idxY);
						hvRegP[region][idx] = hvP[i].at<float>(idxX, idxY);
						hvBarRegO[region][idx] = hvBarO[i].at<float>(idxX, idxY);
						hvBarRegP[region][idx] = hvBarP[i].at<float>(idxX, idxY);

						idx++;
					}	
				}
			}
			region++;
		}
	}

	/* feature streams */
	double fs_si_loss[nRegions]; 
	double fs_si_gain[nRegions];
	double fs_hv_loss[nRegions]; 
	double fs_hv_gain[nRegions];	

	double sdsiO, sdsiP;
	double hvratioO, hvratioP;

	for(int reg=0; reg < nRegions; reg++){		
		/* [si:loss (3) ] Compute the standard deviation of each S-T region */

		sdsiO = calc_sd(siRegO[reg],elemPerRegion);
		sdsiP = calc_sd(siRegP[reg],elemPerRegion);

		/* [si_loss (4)] apply a perceptability threshold, replacing values less than 12 with 12*/
		/* [si_loss (5)] compare original and processed feature streams using ratio comparison function followed by loss function */
		fs_si_loss[reg] = loss(ratioComp(perc_thresh(sdsiO, 12), perc_thresh(sdsiP, 12)));
 
		/* [si_gain (2) ] apply a perceptability threshold, replacing values less than 8 with 8 */				
		/* [si_gain (3) ] compare original and processed feature streams using log comparison function followed by gain function*/ 
		fs_si_gain[reg] = gain(logComp(perc_thresh(sdsiO, 8), perc_thresh(sdsiP, 8)));

		/* [hv_loss (3)] Compute the mean of each S-T region. */
		/* [hv_loss (4)] Apply a perceptibility threshold, replacing values less than 3 with 3. */				
		/* [hv_loss (5)] Compute the ratio (HV / HVBAR). */
		hvratioO = perc_thresh(calc_mean(hvRegO[reg], elemPerRegion), 3) / perc_thresh(calc_mean(hvBarRegO[reg], elemPerRegion), 3);
		hvratioP = perc_thresh(calc_mean(hvRegP[reg], elemPerRegion), 3) / perc_thresh(calc_mean(hvBarRegP[reg], elemPerRegion), 3);
		/* [hv_loss(6)] Compare original and processed feature streams (each computed using steps 1 through 5) using the ratio comparison function (see equation 3) followed by the loss function. */		
		fs_hv_loss[reg] = loss(ratioComp(hvratioO, hvratioP));
		/* [hv_gain (2)] Compare original and processed feature streams using the log comparison function (see equation 4) followed by the gain function.*/
		fs_hv_gain[reg] = gain(logComp(hvratioO, hvratioP)); 
	}	
	
	/* [hv_gain (3)] Spatially collapse by computing the average of the worst 5% of blocks for each 0.2 second slice of time. */
	/* [hv_gain (4)] Temporally collapse by taking the mean over all time slices. 	TODO: timecollapse*/ 
	
	std::sort(fs_hv_gain, fs_hv_gain+sizeof(fs_hv_gain)/sizeof(fs_hv_gain[0]));
	int until = nRegions * .05;
	double avg = 0;
	for(int i=0; i<until; i++){ 
		 avg += fs_hv_gain[i];
	}
	avg /= until;	
	hv_gain[actSlice] = avg;
std::cout<<"[debug] hv_gain for slice " << actSlice <<" : "<< hv_gain[actSlice] << std::endl; 



	/* hv_loss
			(7) Spatially collapse by computing the average of the worst 5% of blocks for each 0.2 second slice of time.*/
	std::sort(fs_hv_loss, fs_hv_loss+sizeof(fs_hv_loss)/sizeof(fs_hv_loss[0]));
	until = nRegions * .05;
	avg = 0;
	for(int i=0; i<until; i++){ 
		 avg += fs_hv_loss[i];
	}
	avg /= until;	
	hv_loss[actSlice] = avg;

std::cout<<"[debug] hv_loss for slice " << actSlice <<" : "<< hv_loss[actSlice] << std::endl; 
/* hv_loss
				(8) Temporally collapse by taking the mean over all time slices. //TODO: timecollapse / getMetricValue
				(9) Square the parameter (i.e., non-linear scaling), and clip at a minimum value of 0.06 (see equation 5).  //TODO: timecollapse / getMetricValue
	*/


	
	/* [si_loss (6)] spatially collapse by computing the average of the worst (i.e. most impaired) 5% of S-T blocks for each 0.2 second slice of time */	
	std::sort(fs_si_loss, fs_si_loss+sizeof(fs_si_loss)/sizeof(fs_si_loss[0]));
	until = nRegions * .05;
	avg = 0;
	for(int i=0; i<until; i++){ 
		avg += fs_si_loss[i];
	}

	avg /= until;	
	si_loss[actSlice] = avg;

std::cout<<"[debug] si_loss for slice " << actSlice <<" : "<< si_loss[actSlice] << std::endl; 

	/* 
	 * [si_gain (4) ] spatially and temporally collapse by computing the average of all block and the clip at a minimum value of 0.004
	 * [si_gain (5) ] set all values greater than 0.14 to 0.14
	 */

	avg = 0;
	for(int i=0; i<nRegions; i++){ 
		avg += fs_si_gain[i];
	}

	avg /= nRegions;	
	si_gain[actSlice] = avg; 
	/* rest of 4 and 5 tbd in temporal collapse, maybe we should store here how much frames went into the calculation?= TODO*/

std::cout<<"[debug] si_gain for slice " << actSlice <<" : "<< si_gain[actSlice] << std::endl; 
	
	/*	[ct_ati_gain (2)] Divide each video sequence into 4 pixel x 4 line x 0.2 second S-T regions. */

	rW = 4;
	rH = 4;
	nRegions = w / rW * h / rH;
	elemPerRegion = rW*rH*(nFrames-1);

std::cout << "[debug] splitting into " << nRegions << " s-t regions with " << elemPerRegion << " elements per region" << std::endl;

	double fs_ati_gain[nRegions];

	region = 0;
	idx = 0;

	float** atiRegO = new float*[nRegions];
	float** atiRegP = new float*[nRegions];
	float** lumRegO = new float*[nRegions];
	float** lumRegP = new float*[nRegions];

	for(int i=0; i<nRegions; i++){
		atiRegO[i] = new float[elemPerRegion];
		atiRegP[i] = new float[elemPerRegion];
		lumRegO[i] = new float[elemPerRegion];
		lumRegP[i] = new float[elemPerRegion];
	}

	for(int y=0; y<h/rH; y++){
		for(int x=0; x<w/rW; x++){
			// each region
			idx = 0;
			for(int i=0; i<(nFrames-1); i++){ //we have a picture less here
				//each input frame 
				for(int dy=0; dy<rH; dy++){
					for(int dx=0; dx<rW; dx++){						
						idxX = rW*x+dx;
						idxY = rH*y+dy;
						/*	[ct_ati_gain (1)] Apply the “absolute value of temporal information” (ATI) motion detection filter to each luminance plane. 
						ATI is the absolute value of a pixel-by-pixel difference between
						the current and previous video frame. */
						atiRegO[region][idx] =  abs(lumO[i].at<float>(idxX, idxY) - lumO[i+1].at<float>(idxX, idxY));
						atiRegP[region][idx] =  abs(lumP[i].at<float>(idxX, idxY) - lumP[i+1].at<float>(idxX, idxY));
						lumRegO[region][idx] =  lumO[i+1].at<float>(idxX, idxY);
						lumRegP[region][idx] =  lumP[i+1].at<float>(idxX, idxY);				

						idx++;
					}	
				}
			}
			region++;
		}
	}	

	/*	[ct_ati_gain (3)] Compute the standard deviation of each S-T region. */
	double sdatiRegO, sdatiRegP, sdlumRegO, sdlumRegP;

	for(int reg=0; reg < nRegions; reg++){
		/*	[ct_ati_gain (4)] Apply a perceptibility threshold, replacing values less than 3 with 3. */
		sdatiRegO = perc_thresh(calc_sd(atiRegO[reg], elemPerRegion), 3);
		sdatiRegP = perc_thresh(calc_sd(atiRegP[reg], elemPerRegion), 3);
		sdlumRegO = perc_thresh(calc_sd(lumRegO[reg], elemPerRegion), 3);
		sdlumRegP = perc_thresh(calc_sd(lumRegP[reg], elemPerRegion), 3);

		/*	[ct_ati_gain (5)] Repeat steps 2 through 4 on the Y luminance video sequence (without perceptual filtering) to form “contrast” feature streams. */	
		/*	[ct_ati_gain (6)] Multiply the contrast and ATI feature streams. */
		/*	[ct_ati_gain (7)] Compare original and processed feature streams (each computed using steps 1 through 6) using the ratio comparison function (see equation 3) followed by the gain function. */
		fs_ati_gain[reg] = gain(ratioComp(sdatiRegO * sdlumRegO, sdatiRegP * sdlumRegP));
	}


/*	[ct_ati_gain (8)] Spatially collapse by computing the mean of each 0.2 second slice of time. */
	avg = 0;
	for(int i=0; i<nRegions; i++){ 
		avg += fs_ati_gain[i];
	}
	avg /= nRegions;
	ct_ati_gain[actSlice] = avg; 

/*	[ct_ati_gain (9)] Temporally collapse by sorting values in time and selecting the 10% level. The parameter values are all positive, so this temporal collapsing function is a form of best-case processing, detecting impairments that are nearly always present. */

std::cout<<"[debug] ct_ati_gain for slice " << actSlice <<" : "<< ct_ati_gain[actSlice] << std::endl; 


	actSlice++;	

	return -1;
}

double VQM::loss(double d){
	if(d>0) 
		return 0;
	return d;
}
double VQM::gain(double d){
	if(d<0) 
		return 0;
	return d;
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
double VQM::calc_mean(float* arr, int len){
	double mean = 0;
	for(int i=0;i<len;i++){
		mean += arr[i];
	}
	return mean/len;
}
double VQM::calc_mean_squared(float* arr, int len){
	double mean = 0;
	for(int i=0;i<len;i++){
		mean += arr[i]*arr[i];
	}
	return mean/len;
}
double VQM::calc_sd(float* arr, int len){
	double mean = calc_mean(arr,len);
	double sd = 0;
	for(int i=0;i<len;i++){
		sd += (mean-arr[i])*(mean-arr[i]);
	}
	return sqrt(sd/(len-1));
}
double VQM::perc_thresh(double d, double t){
	return (d < t ? t : d);
}
