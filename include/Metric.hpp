#ifndef metric_hpp
#define metric_hpp

#include "opencv2/opencv.hpp" 

class Metric{
	public:
		virtual double compute(cv::Mat[], cv::Mat[], int) = 0;
	protected:		
		void filter_si_hv_bar(cv::Mat&, cv::Mat&, cv::Mat&, cv::Mat&, int);
		void hv_bar_filter(cv::Mat&, cv::Mat&, cv::Mat&, cv::Mat&, cv::Mat&);
		void si_filter(cv::Mat&, cv::Mat&, cv::Mat&, cv::Mat&, int);
		void getFilterMask(int, float*);
};

#endif
