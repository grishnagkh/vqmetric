
#include "opencv2/opencv.hpp"
#include "VideoCaptureReader.hpp"
#include "PSNR.hpp"
#include "SSIM.hpp"

using namespace std;

int main(int ac, const char** av){
 
	VideoCaptureReader orig("out1.mp4");
	VideoCaptureReader processed("out2.mp4");

	int n=60;		

	cv::Mat in[n];
	cv::Mat out[n];

	cv::Mat tmp;	 

	for(int i=0; i<n; i++){
		orig.nextFrame(tmp);
		tmp.convertTo(in[i], CV_32F);
		processed.nextFrame(tmp);
		tmp.convertTo(out[i], CV_32F);		
	}

	PSNR psnr;
	SSIM ssim;

	std::cout<< "PSNR of first " <<  n << " frames: " << psnr.compute(in, out, n) <<endl;
	std::cout<< "SSIM of first " <<  n << " frames: " << ssim.compute(in, out, n) <<endl;
 
	
	return 0;	
}
