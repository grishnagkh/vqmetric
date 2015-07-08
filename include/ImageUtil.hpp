#ifndef ImageUtil_hpp
#define ImageUtil_hpp

#include "opencv2/opencv.hpp"

using namespace cv;

class ImageUtil{
	public:  
		void gaussianBlur(Mat&, Mat&, int, double);	
		void filter_si_hv_bar(Mat&, Mat&, Mat&, Mat&, int);
	private:
		void hv_bar_filter(Mat&, Mat&, Mat&, Mat&, Mat&);
		void si_filter(Mat&, Mat&, Mat&, Mat&, int);
		void getFilterMask(int, float*);
};

#endif
