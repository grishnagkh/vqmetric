#include "Util.hpp"

using namespace std; 

double Util::euclideansq(double fo1, double fo2, double fp1, double fp2){
	/* returns the square of the euclidean distance */
	double t1 = (fo1 - fp1) ;
	double t2 = (fo2 - fp2) ;
	return t1*t1 + t2*t2;
}
double Util::ratioComp(double orig, double processed){
	return (processed-orig)/orig;
}
double Util::logComp(double orig, double processed){
	return log10(processed/orig);
}
 
double Util::clip(double f, double threshold){
	if(f >= threshold)
		return f;
	return threshold;
} 
