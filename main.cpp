#include "opencv2/opencv.hpp"
#include <unistd.h>
#include <fstream>
#include <math.h>

using namespace cv;
using namespace std;

/******** CLIPPING *************/
 
double clip(double f, double threshold){
	if(f >= threshold)
		return f;
	return threshold;
}


/******** FEATURE COMPARISON *************/

double euclideansq(double fo1, double fo2, double fp1, double fp2){
	/* returns the square of the euclidean distance */
	double t1 = (fo1 - fp1) ;
	double t2 = (fo2 - fp2) ;
	return t1*t1 + t2*t2;
}
double ratioComp(double orig, double processed){
	return (processed-orig)/orig;
}
double logComp(double orig, double processed){
	return log10(processed/orig);
}
/*****************************************/

void applyGaussianBlur(Mat src, Mat dst, int ksize, double sigma){
	int invalid = (ksize-1)/2;
	cv::Mat tmp(src.rows, src.cols, CV_32F);
	cv::GaussianBlur(src, tmp, cv::Size(ksize,ksize), sigma);
	tmp(cv::Range(invalid, tmp.rows-invalid), cv::Range(invalid, tmp.cols-invalid)).copyTo(dst);
}


/******** SI FILTER ********/

/* filter mask generator */
void getFilterMask(int len, float *filter){
	
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
void si_filter(const Mat& src, Mat& si_filtered, Mat& hfiltered,  Mat& vfiltered, int len){
	
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
void hv_bar_filter(Mat& si, Mat& hSiFiltered, Mat& vSiFiltered, Mat& hvbar, Mat& hv){
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

void filter_si_hv_bar(Mat& src, Mat& si, Mat& hvbar, Mat& hv, int len){
	Mat hfil, vfil;
	si_filter(src, si, hfil, vfil ,len);
	hv_bar_filter(si, hfil, vfil, hvbar, hv);
}


/************ PSNR ************/

double getPSNR(const Mat& I1, const Mat& I2){
    Mat s1;
    subtract(I1, I2, s1);    
    s1 = s1.mul(s1);        
   
	double mse = mean(s1).val[0];
	if(mse <= 1e-10){
		mse = 1e-15; // => psnr ~200 if frames identical
	}
	double psnr = 10.0 * log10((255 * 255) / mse);
	return psnr;
    
}

/************ SSIM ************/

double getMSSIM( const Mat& I1, const Mat& I2){
	/* 
		Code from 
		http://docs.opencv.org/doc/tutorials/highgui/video-input-psnr-ssim/video-input-psnr-ssim.html
	*/

    const double C1 = 6.5025, C2 = 58.5225;
    /***************************** INITS **********************************/

    Mat I2_2   = I2.mul(I2);        // I2^2
    Mat I1_2   = I1.mul(I1);        // I1^2
    Mat I1_I2  = I1.mul(I2);        // I1 * I2

    /*************************** END INITS **********************************/

    Mat mu1, mu2;                   // PRELIMINARY COMPUTING
    GaussianBlur(I1, mu1, Size(11, 11), 1.5);
    GaussianBlur(I2, mu2, Size(11, 11), 1.5);

    Mat mu1_2   =   mu1.mul(mu1);
    Mat mu2_2   =   mu2.mul(mu2);
    Mat mu1_mu2 =   mu1.mul(mu2);

    Mat sigma1_2, sigma2_2, sigma12;

    GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
    sigma1_2 -= mu1_2;

    GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
    sigma2_2 -= mu2_2;

    GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;

    ///////////////////////////////// FORMULA ////////////////////////////////
    Mat t1, t2, t3;

    t1 = 2 * mu1_mu2 + C1;
    t2 = 2 * sigma12 + C2;
    t3 = t1.mul(t2);                 // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))

    t1 = mu1_2 + mu2_2 + C1;
    t2 = sigma1_2 + sigma2_2 + C2;
    t1 = t1.mul(t2);                 // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))

    Mat ssim_map;
    divide(t3, t1, ssim_map);        // ssim_map =  t3./t1;

    Scalar mssim = mean(ssim_map);   // mssim = average of ssim map
	return mssim.val[0];
}
float calc_psnr(Mat orig, Mat processed, Mat tmp){	
	subtract(orig, processed, tmp);
    multiply(tmp, tmp, tmp);		
	float mse = mean(tmp).val[0];
	if(mse == 0){			
		mse = 1e-20 ;//some small difference to bypass inf values, results in psnr value 248.131 if frames are identical
	}
	return float(20*log10(255) - 10*log10(mse));		
}

int main(int ac, const char** av){


	Mat src1 = imread("img.png", CV_LOAD_IMAGE_COLOR);
	Mat src, si, hvbar,hv;
	src1.convertTo(src, CV_32F);

/*
	imshow("orig before call",src);
	waitKey(1000);
	Mat dest;
	si_filter(src, dest, 13);
	imshow("orig after call",src);
	waitKey(2000);
*/

/*	test();	*/

imshow("orig before call",src);
	filter_si_hv_bar( src, si, hvbar, hv, 13);

	imshow("orig after call",src);
//	waitKey(2000);
imshow("si",si);
	//waitKey(2000);
imshow("hvbar",hvbar);
//	waitKey(5000);
imshow("hv",hv);
	waitKey(15000);

	return 0;	
}

int psnr_ssim(){


	//no params for now...	
//	VideoCapture in("out1.mp4");
//	VideoCapture out("out2.mp4");
	
VideoCapture in("1000.mp4");
VideoCapture out("500.mp4");
	

	if (!in.isOpened() || !out.isOpened()) {
        cerr << "Failed to open the video file!\n" << endl;
        return 1;
    }
	
	int cnt = 0;
	Mat src, proc;	
	int nframes = in.get(CV_CAP_PROP_FRAME_COUNT);	
	int w = in.get(CV_CAP_PROP_FRAME_WIDTH);
	int h = in.get(CV_CAP_PROP_FRAME_HEIGHT);
	int fps = in.get(CV_CAP_PROP_FPS);
	
	double fourcc = in.get(CV_CAP_PROP_FOURCC);
	double format = in.get(CV_CAP_PROP_FORMAT);

	cout << "processing " << nframes  << " frames of an " << w << "x" << h << " video with " << fps << " fps" << "| format: " <<  format<< "| fourcc: " << fourcc << endl;

	double psnr[nframes];
	double ssim[nframes];

	Mat tmp(w,h,CV_32F);
	Mat original, processed, blub;


	while(cnt < nframes && in.read(src) &&  out.read(proc)){

		src.convertTo(original, CV_32F);
		proc.convertTo(processed, CV_32F);

		psnr[cnt] = getPSNR(original, processed);// calc_psnr(original, processed, tmp);
     	ssim[cnt] = getMSSIM(original, processed); //calc_ssim(original, processed);

		cnt++;
	}


	stringstream s;
	s << "frame;psnr;ssim\n";	

	double  sumpsnr=0, sumssim=0;
	for(int i=0; i<nframes; i++){
		s << i+1 << ";"<< psnr[i] << ";" << ssim[i] <<"\n";
		sumpsnr += psnr[i];
		sumssim += ssim[i];
	}
	s << "average; " <<sumpsnr / nframes <<";" << sumssim / nframes <<"\n";
	
	std::ofstream file("out.txt");	
	file << s.str();

	cout << s.str();
	cout << "processed " << nframes << " frames \n";
	cout << "psnr average: " << sumpsnr / nframes << "\n";
	cout << "ssim average: " << sumssim / nframes << "\n";
	
	return 0;
}

