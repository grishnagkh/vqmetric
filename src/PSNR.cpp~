#include "PSNR.hpp" 

double PSNR::compute(cv::Mat orig[], cv::Mat processed[], int nFrames){
	double sum = 0;
	for(int i=0; i<nFrames;i++){
		sum += computeSingleFrame(orig[i], processed[i]);
	}
	return sum/nFrames; 
}

double PSNR::computeSingleFrame(cv::Mat& orig, cv::Mat& processed){
	cv::Mat tmp; 
	cv::subtract(orig, processed, tmp);
	tmp = tmp.mul(tmp);
	double mse = cv::mean(tmp).val[0];	
	if(mse <= 1e-10){
		mse = 1e-15; // => psnr ~200 if frames identical to avoid division by zero
	}
	return 10.0 * log10((255 * 255) / mse);
}
