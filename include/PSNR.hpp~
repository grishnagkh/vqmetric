#ifndef psnr_hpp
#define psnr_hpp

#include "Metric.hpp"  

class PSNR: public Metric{
	public:
		double compute(cv::Mat[], cv::Mat[], int);
	private:
		double computeSingleFrame(cv::Mat&, cv::Mat&);
};

#endif
