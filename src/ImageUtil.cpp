#include "ImageUtil.hpp"

using namespace cv;

/*****************************************/

void ImageUtil::gaussianBlur(Mat& src, Mat& dst, int ksize, double sigma){
	int invalid = (ksize-1)/2;
	cv::Mat tmp(src.rows, src.cols, CV_32F);
	cv::GaussianBlur(src, tmp, cv::Size(ksize,ksize), sigma);
	tmp(cv::Range(invalid, tmp.rows-invalid), cv::Range(invalid, tmp.cols-invalid)).copyTo(dst);
}

/******** SI FILTER ********/

/* filter mask generator */
void ImageUtil::getFilterMask(int len, float *filter){
	
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
//	cout << "filter: ";
	for(int x=0; x<len; x++){
		filter[x] /= co;
		//cout << filter[x]<<"\t";
	}
	//cout << endl;
}

void ImageUtil::si_filter(Mat& src, Mat& si_filtered, Mat& hfiltered,  Mat& vfiltered, int len){
	
	float filterMask[len];
	float filterA [len][len];

	getFilterMask(len, filterMask);
	for(int i=0; i<len; i++){
		for(int j=0; j<len; j++){
			filterA[i][j] = filterMask[i];
		}
	}
	Mat kernel = Mat(len, len, CV_32F, &filterA);
	Mat tmp;

	filter2D(src, hfiltered, -1, kernel);
	filter2D(src, vfiltered, -1, kernel.t());

	Mat h2 = hfiltered.mul(hfiltered);
	Mat v2 = vfiltered.mul(vfiltered);

	add(h2,v2, tmp); 
	sqrt(tmp, si_filtered);	 

}

/******** HV & HVBAR FILTER ********/
void ImageUtil::hv_bar_filter(Mat& si, Mat& hSiFiltered, Mat& vSiFiltered, Mat& hvbar, Mat& hv){
	/*  
		Originally the computation involves atan2 (slow): compute ratio between h & v, putting the smaller value on top and the larger value on the bottom.  Ignore divide by zero, because later code checking against rmin will catch that.  Essentially, fold angle into pi/4.
	*/

	double theta = .225;
	double ratio_treshold = tan(theta);
	double rmin = 20;

	hSiFiltered = abs(hSiFiltered);
	vSiFiltered = abs(vSiFiltered);
	
	Mat ratio;
	divide(min(vSiFiltered, hSiFiltered) , max(vSiFiltered, hSiFiltered), ratio);

	Mat rat_t, base;

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
	threshold(si, base, rmin, -1, THRESH_TOZERO);	
	// hv and hv bar criteria
	threshold(ratio, rat_t, ratio_treshold, 1, THRESH_BINARY);

	/* rat_t now contains 0 is the threshold is not exceeded, 1 otherwise... */
	hv = base.mul(rat_t);
	subtract(base, hv, hvbar);
	
}

void ImageUtil::filter_si_hv_bar(Mat& src, Mat& si, Mat& hvbar, Mat& hv, int len){
	Mat hfil, vfil;
	si_filter(src, si, hfil, vfil ,len);
	hv_bar_filter(si, hfil, vfil, hvbar, hv);
}


/**************************/

/* the filter */
/* 
void si_filter_old(const Mat& src, Mat& dest, int len){
	Mat hfil, vfil;
	si_filter(src, dest, hfil, vfil ,len);

	float filterMask[len];
	float filterA [len][len];

	getFilterMask(len, filterMask);
	for(int i=0; i<len; i++){
		for(int j=0; j<len; j++){
			filterA[i][j] = filterMask[i];
		}
	}
	Mat kernel = Mat(len, len, CV_32F, &filterA);
	Mat hfiltered, vfiltered, tmp;

	filter2D(src, hfiltered, -1, kernel);
	filter2D(src, vfiltered, -1, kernel.t());

	Mat h2 = hfiltered.mul(hfiltered);
	Mat v2 = vfiltered.mul(vfiltered);

	add(h2,v2, tmp); 
	sqrt(tmp, dest);	 

}
*/

