#ifndef ssim_hpp
#define ssim_hpp

#include "Metric.hpp" 

class SSIM: public Metric{
	public:
		double compute(cv::Mat[], cv::Mat[], int);
	private:
		double computeSingleFrame(const cv::Mat&, const cv::Mat&);
};

#endif
