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

VQM::VQM(std::string logfile_path, int log_level, int verbose_level){
	this->logfile_path = logfile_path;
	this->log_level = log_level;
	this->verbose_level = verbose_level;
};

VQM::~VQM(){
	
}

/* 
 * computes parts of VQM metrics for a .2 second slice 
 */	
double VQM::compute(cv::Mat ref[][3], cv::Mat proc[][3], int nFrames){
//TODO: debug; einzelwerte sollten passen, beim zusammenführen les ich wsl irgendwo noch zu viel aus denk ich... mal im debug modus starten... eeeerm und ct_ati_gain liefert immer 0 zurück -> nachschaun, was da verbockt is
	std::ofstream logfile;
	logfile.open ((logfile_path).c_str(), std::ios::out | std::ios::app );

	/* input: original frames */ 

	/* 
	 *
	 * Input parameters 
	 * 		proc .. a slice of the processed image stream
	 * 		ref ... a slice of the reference picture stream
	 *
	 * ref[0], proc[0]: luminance planes for calculation of
	 * 		si_loss
	 * 		si gain
	 * 		hv loss
	 * 		hv gain
	 *		ct ati gain
	 *	
	 * ref[1] ref[2] proc[1] proc[2] YCr and YCb plane for calculation of 
	 * 		chroma spread 
	 *		chroma extreme
  	 */



	int h = ref[0][0].rows;
	int w = ref[0][0].cols;

	double avg;
	int cnt;
	int ctr;
	cv::Mat tmp;

	/* PREPROCESSING: SI, HV, HVBAR, ATI fitlering */
	cv::Mat si_reference[nFrames];
	cv::Mat si_processed[nFrames];
	cv::Mat hv_reference[nFrames];
	cv::Mat hv_processed[nFrames];
	cv::Mat hv_bar_reference[nFrames];
	cv::Mat hv_bar_processed[nFrames];

	cv::Mat ati_processed[nFrames-1];
	cv::Mat ati_reference[nFrames-1];

	v("[debug] starting filtering", this->verbose_level, VERBOSE_MINIMAL );

	filter_si_hv_bar(ref[0][Y], si_reference[0], hv_bar_reference[0], hv_reference[0], 13);
	filter_si_hv_bar(proc[0][Y], si_processed[0], hv_bar_processed[0], hv_processed[0],13);

	for(int i=1; i<nFrames; i++){	
		/* apply SI, HV and HVBAR filters to each luminance plane */
		filter_si_hv_bar(ref[i][Y], si_reference[i], hv_bar_reference[i], hv_reference[i], 13);
		filter_si_hv_bar(proc[i][Y], si_processed[i], hv_bar_processed[i], hv_processed[i], 13);	
		/*	[ct_ati_gain (1)] 	
			Apply the “absolute value of temporal information” (ATI) 
			motion detection filter to each luminance plane. 
			ATI is the absolute value of a pixel-by-pixel 
			difference between the current and previous video frame. 
		*/
		absdiff(proc[i][Y], proc[i-1][Y], ati_processed[i-1]);
		absdiff(ref[i][Y],  ref[i-1][Y],  ati_reference[i-1]);

	}
	v("[debug] calculating chroma_spread and chroma_extreme", this->verbose_level, VERBOSE_MINIMAL );



	float cr_mean_p, cb_mean_p;
	float cr_mean_r, cb_mean_r;
	float tx, ex, ex2;
	int w_size_x = 8;	
	int w_size_y = 8;
	/* (chroma spread 1) Divide the CB and CR color planes 
		into separate 8 pixel x 8 line x 1 frame S-T regions. */
	for(int i=0; i< nFrames; i++){
		tx = 0;
		ex = 0;
		ex2 = 0;
		chroma_extreme_t.clear();
		for(int x = 0; x < w - w_size_x; x+=w_size_x){	
			cr_mean_p = cb_mean_p = 0;
			cr_mean_r = cb_mean_r = 0;
			for(int y = 0; y < h - w_size_y; y+=w_size_y){	

				/* (chroma spread 2) Compute the mean of each S-T region. 
					Multiply the CR means by 1.5 to increase the perceptual
					weighting of the red color component in the next step. 
				*/
				tmp = cv::Mat( ref[CB][i],  cv::Rect(x, y, w_size_x, w_size_y));	
				cb_mean_r = ( cv::mean(tmp)[0] );
				tmp = cv::Mat( ref[CR][i],  cv::Rect(x, y, w_size_x, w_size_y));
				cr_mean_r = 1.5 * ( cv::mean(tmp)[0] );
				tmp = cv::Mat(proc[CB][i],  cv::Rect(x, y, w_size_x, w_size_y));	
				cb_mean_p = ( cv::mean(tmp)[0] );
				tmp = cv::Mat(proc[CR][i],  cv::Rect(x, y, w_size_x, w_size_y));	
				cr_mean_p = 1.5 * ( cv::mean(tmp)[0] );

				/* (chroma spread 3) Compare original and processed feature streams 
					CB and CR using Euclidean distance (see equation 2). */
				tx = euclideansq(cb_mean_r, cr_mean_r, cb_mean_p, cr_mean_p);
				// as we get the squared distance...
				ex2 += tx;
				tx = sqrt(tx);
				ex += tx;
				chroma_extreme_t.push_back(tx);				
			}			
		}

		/* (chroma_spread 4) Spatially collapse by computing the standard deviation of blocks for each 1-frame slice of time. */
		ex  /= w/8*h/8;
		ex2 /= w/8*h/8;
		chroma_spread.push_back(sqrt(ex2-ex*ex));

		if(this->log_level > LOG_MINIMAL){
			logfile << "chroma_spread: " << sqrt(ex2-ex*ex) << std::endl;
		}
		/* 
	(chroma_extreme 2) Spatially collapse by computing for each slice of time the average of the worst 1% of blocks (i.e., rank-sorted values from the 99% level to the 100% level), and subtract from that result the 99% level. This identifies very bad distortions that impact a small portion of the image.
		*/
		std::sort(chroma_extreme_t.begin(), chroma_extreme_t.end() );
		uint p = chroma_extreme_t.size() * 0.99;
		avg = 0;
		cnt = 0;
		for(uint pos = p ; pos < chroma_extreme_t.size(); pos++){
			avg += chroma_extreme_t[pos];
			cnt++;
		}
		chroma_extreme.push_back( avg/cnt -  chroma_extreme_t[p] );
		if(this->log_level > LOG_MINIMAL){
			logfile << "chroma_extreme: " << avg/cnt -  chroma_extreme_t[p]  << std::endl;
		}
	}
	 
	v("[debug] calculating si/hv gain/loss", this->verbose_level, VERBOSE_MINIMAL );




	/* [si_loss(2), hv_loss (2)] Divide each of the SI, HV and HVBAR video sequences into 8 pixel x 8 line x 0.2 second S-T regions. */	

	float si_p_ex = 0;
	float si_p_ex2 = 0;
	float si_r_ex = 0;
	float si_r_ex2 = 0;
	float hv_p_ex = 0;
	float hv_r_ex = 0;
	float hv_bar_p_ex = 0;
	float hv_bar_r_ex = 0;

	float hv_ratio_p = 0;
	float hv_ratio_r = 0;

	w_size_x = 8;
	w_size_y = 8;

	std::vector<float> si_loss_t(w/w_size_x*h/w_size_y);
	float si_gain_t = 0;
	std::vector<float> hv_loss_t(w/w_size_x*h/w_size_y);
	std::vector<float> hv_gain_t(w/w_size_x*h/w_size_y);



	for(int x = 0; x < w-w_size_x; x+=w_size_x){
		for(int y = 0; y < h-w_size_y; y+=w_size_y){	
			si_p_ex = 0;
			si_p_ex2 = 0;
			si_r_ex = 0;
			si_r_ex2 = 0;
			hv_p_ex = 0;
			hv_r_ex = 0;
			hv_bar_p_ex = 0;
			hv_bar_r_ex = 0;
			for(int i=0; i< nFrames; i++){
				tmp = cv::Mat(si_reference[i],  cv::Rect(x, y, w_size_x, w_size_y));	
				si_r_ex  += ( cv::mean(tmp)[0] );
				si_r_ex2 += ( cv::mean(tmp.mul(tmp))[0] );

				tmp = cv::Mat(si_processed[i],  cv::Rect(x, y, w_size_x, w_size_y));	
				si_p_ex  += ( cv::mean(tmp)[0] );
				si_p_ex2 += ( cv::mean(tmp.mul(tmp))[0] );

				tmp = cv::Mat(hv_reference[i],  cv::Rect(x, y, w_size_x, w_size_y));	
				hv_r_ex  += ( cv::mean(tmp)[0] );

				tmp = cv::Mat(hv_processed[i],  cv::Rect(x, y, w_size_x, w_size_y));	
				hv_p_ex  += ( cv::mean(tmp)[0] );

				tmp = cv::Mat(hv_bar_reference[i],  cv::Rect(x, y, w_size_x, w_size_y));	
				hv_bar_r_ex  += ( cv::mean(tmp)[0] );

				tmp = cv::Mat(hv_bar_processed[i],  cv::Rect(x, y, w_size_x, w_size_y));	
				hv_bar_p_ex  += ( cv::mean(tmp)[0] );
			}
			si_p_ex 	/= nFrames;
			si_p_ex2	/= nFrames;
			si_r_ex		/= nFrames;
			si_r_ex2	/= nFrames;
			hv_p_ex		/= nFrames;			
			hv_r_ex 	/= nFrames;
			hv_bar_p_ex /= nFrames;
			hv_bar_r_ex /= nFrames;
 
			
			/* [si:loss (3) ] Compute the standard deviation of each S-T region */
			/* [si_loss (4) ] apply a perceptability threshold, 
				replacing values less than 12 with 12*/
			/* [si_loss (5)] compare original and processed feature streams using ratio comparison function followed by loss function */	

			// 12x12 because we are calculating with variances in the inner braces
			si_loss_t.push_back(
					loss(
						ratioComp(
							perc_thresh(sqrt( si_r_ex2 - si_r_ex * si_r_ex ) , 12), 
							perc_thresh(sqrt( si_p_ex2 - si_p_ex * si_p_ex ) , 12) 
						)
				)
			);
			/* [si_gain (2) ] apply a perceptability threshold, 
				replacing values less than 8 with 8 */			
			/* [si_gain (3) ] compare original and processed feature streams using log comparison function followed by gain function*/ 	
			// 8x8 because we are calculating with variances in the inner braces
			si_gain_t += sqrt(
				gain(
					logComp(
						perc_thresh( sqrt(si_r_ex2 - si_r_ex * si_r_ex) , 8), 
						perc_thresh( sqrt(si_p_ex2 - si_p_ex * si_p_ex) , 8)
					)
				)
			);

			/* [hv_loss (3)] Compute the mean of each S-T region. */
			/* [hv_loss (4)] Apply a perceptibility threshold, replacing values less than 3 with 3. */				
			/* [hv_loss (5)] Compute the ratio (HV / HVBAR). */
			/* [hv_loss(6)] Compare original and processed feature streams (each computed using steps 1 through 5) using the ratio comparison function (see equation 3) followed by the loss function. */		
			hv_ratio_r = perc_thresh(hv_bar_r_ex	, 3) / perc_thresh(hv_r_ex	, 3);
			hv_ratio_p = perc_thresh(hv_bar_p_ex	, 3) / perc_thresh(hv_p_ex	, 3);
			
			hv_loss_t.push_back(
				loss(
					ratioComp(
						hv_ratio_r, hv_ratio_p
					)					
				)
			);

			/* [hv_gain (2)] Compare original and processed feature streams using the log comparison function (see equation 4) followed by the gain function.*/
			hv_gain_t.push_back(
				gain(
					logComp(
						hv_ratio_r, hv_ratio_p
					)					
				)
			);
		}
	}
  

	/* [si_gain (4) ] spatially collapse by computing the average of all blocks */
	si_gain_t /= nFrames * w/w_size_x * h/w_size_y;
	
	si_gain.push_back(si_gain_t); 

	if(this->log_level > LOG_MINIMAL){
		logfile << "si_gain: "<< si_gain_t << std::endl; 		
	}



	/* [hv_gain (3)] Spatially collapse by computing the average of the worst 5% of blocks for each 0.2 second slice of time. */
	
	
	std::sort(hv_gain_t.begin(), hv_gain_t.end() );
	avg = 0;
	cnt = hv_gain_t.size() * 0.05;

	for(uint i= hv_gain_t.size()-cnt; i <  hv_gain_t.size(); i++){
		avg += hv_gain_t[i];
	}
	avg /=  cnt;

	hv_gain.push_back(avg);

	if(this->log_level > LOG_MINIMAL){
		logfile << "hv_gain: "<< avg << std::endl; 
	}


	/* hv_loss
			(7) Spatially collapse by computing the average of the worst 5% of blocks for each 0.2 second slice of time.*/
	
	std::sort(hv_loss_t.begin(), hv_loss_t.end() );
	avg = 0;
	cnt =  hv_loss_t.size() * 0.05;
	for(int i=0; i <= cnt; i++){
		avg += hv_loss_t[i];
	}
	avg /= cnt;

	hv_loss.push_back(avg);

	if(this->log_level > LOG_MINIMAL){
		logfile << "hv_loss: "<< avg << std::endl; 
	}



	
	/* [si_loss (6)] spatially collapse by computing the average of the worst (i.e. most impaired) 5% of S-T blocks for each 0.2 second slice of time */	
	
	std::sort(si_loss_t.begin(), si_loss_t.end() );
	avg = 0;
	cnt = si_loss_t.size() * 0.05;
	for(int i=0; i < cnt; i++){
		avg += si_loss_t[i];

	}
	avg /= cnt;

	si_loss.push_back(avg);
	if(this->log_level > LOG_MINIMAL){
		logfile << "si_loss: "<< avg << std::endl; 
	}

	v("[debug] calculating ct_ati_gain ",
		 this->verbose_level, VERBOSE_MINIMAL );

	float p_ex;
	float p_ex2;
	float r_ex;
	float r_ex2;
	float c_p_ex; 
	float c_p_ex2;
	float c_r_ex; 
	float c_r_ex2;

	double fsp, fsr; 	
	avg = 0;
	ctr = 0;

	w_size_x = 40;
	w_size_y = 40;

	for(int x = 0; x < w - w_size_x; x+=w_size_x){
		for(int y = 0; y < h - w_size_y; y+=w_size_y){	
			p_ex 	= 0;
			p_ex2	= 0;
			r_ex	= 0;
			r_ex2	= 0;
			c_p_ex	= 0;
			c_p_ex2	= 0;
			c_r_ex	= 0;
			c_r_ex2	= 0;

			/*	
				[ct_ati_gain (2)] 
				Divide each video sequence into 4 pixel x 4 line x 0.2 second S-T regions. 
			*/		
				
			for(int i=0; i< nFrames-1; i++){
			
				/*	[ct_ati_gain (3)] Compute the standard deviation of each S-T region. */
				/*	[ct_ati_gain (5)] Repeat steps 2 through 4 on the Y luminance 
					video sequence (without perceptual filtering) to form 
					“contrast” feature streams. 
				*/	
				tmp = cv::Mat(ati_processed[i],  cv::Rect(x, y, w_size_x, w_size_y));	
				p_ex  += ( cv::mean(tmp)[0] );
				p_ex2 += ( cv::mean(tmp.mul(tmp))[0] );




				tmp = cv::Mat(ati_reference[i],  cv::Rect(x, y, w_size_x, w_size_y));	
				r_ex  += ( cv::mean(tmp)[0] );
				r_ex2 += ( cv::mean(tmp.mul(tmp))[0] );

				tmp = cv::Mat(proc[i][Y],  cv::Rect(x, y, w_size_x, w_size_y));
				c_p_ex  += ( cv::mean(tmp)[0] );
				c_p_ex2 += ( cv::mean(tmp.mul(tmp))[0] );

				tmp = cv::Mat(ref[i][Y],  cv::Rect(x, y, w_size_x, w_size_y));
				c_r_ex  += ( cv::mean(tmp)[0] );
				c_r_ex2 += ( cv::mean(tmp.mul(tmp))[0] );
			}	

			/*
				VAR(X) = E(X)^2 - E(X^2)
			 */	

		/*	[ct_ati_gain (6)] Multiply the contrast and ATI feature streams. */

			fsp = sqrt(
					perc_thresh( p_ex*p_ex - p_ex2, 9) 
					* perc_thresh( c_p_ex*c_p_ex - c_p_ex2, 9)  
			);
			fsr = sqrt(
					perc_thresh( r_ex*r_ex - r_ex2, 9) 
					* perc_thresh( c_r_ex*c_r_ex - c_r_ex2, 9)  
			);

			/*	[ct_ati_gain (7)] Compare original and processed feature streams (each computed using steps 1 through 6) using the ratio comparison function (see equation 3) followed by the gain function. */
			avg += gain(ratioComp(fsr, fsp));	

			ctr++;
		}
	}

	/*	[ct_ati_gain (8)] Spatially collapse by computing the mean of each 0.2 second slice of time. */
	avg /= ctr;
	ct_ati_gain.push_back(avg);

	if(this->log_level > LOG_MINIMAL){
		logfile << "ct_ati_gain: "<< avg << std::endl; 
		logfile << "processed frames: " << nFrames << std::endl;
	}

	this->n_frames.push_back(nFrames);
	
	logfile.close();
	
	return 0;
}

double VQM::timeCollapse(int nSlices){

	si_gain_collapsed.clear();
	si_loss_collapsed.clear();
	hv_gain_collapsed.clear();
	hv_loss_collapsed.clear();
	ct_ati_gain_collapsed.clear();
	chroma_spread_collapsed.clear();
	chroma_extreme_collapsed.clear();
	
	
	v("[debug] time collapse", this->verbose_level, VERBOSE_DEFAULT );
	v("\t[debug] number of slices per time unit: ", this->verbose_level, VERBOSE_DEFAULT );
	v(nSlices, this->verbose_level, VERBOSE_DEFAULT );
	v("\t[debug] number of calculated slices: ", this->verbose_level, VERBOSE_DEFAULT );
	v(int(si_gain.size()), this->verbose_level, VERBOSE_DEFAULT );
	v("\t[debug] number of calculated slices: ", this->verbose_level, VERBOSE_DEFAULT );
	v(int(si_loss.size()), this->verbose_level, VERBOSE_DEFAULT );
	v("\t[debug] number of calculated slices: ", this->verbose_level, VERBOSE_DEFAULT );
	v(int(hv_gain.size()), this->verbose_level, VERBOSE_DEFAULT );
	v("\t[debug] number of calculated slices: ", this->verbose_level, VERBOSE_DEFAULT );
	v(int(hv_loss.size()), this->verbose_level, VERBOSE_DEFAULT );
	v("\t[debug] number of calculated slices: ", this->verbose_level, VERBOSE_DEFAULT );
	v(int(chroma_spread.size()), this->verbose_level, VERBOSE_DEFAULT );
	v("\t[debug] number of calculated slices: " , this->verbose_level, VERBOSE_DEFAULT );
	v(int(chroma_extreme.size()), this->verbose_level, VERBOSE_DEFAULT );
	v("\t[debug] number of calculated slices: " , this->verbose_level, VERBOSE_DEFAULT );
	v(int(ct_ati_gain.size()), this->verbose_level, VERBOSE_DEFAULT );
	

	int avg;
	int at;
	int len;

	v("\t[debug] time collapse hv gain", this->verbose_level, VERBOSE_MINIMAL );

	/* [hv_gain (4)] Temporally collapse by taking the mean over all time slices. */ 
	avg = 0;
	for(uint segmentStart = 0; segmentStart < hv_gain.size(); segmentStart += nSlices){
		for(uint i = segmentStart; i < segmentStart + nSlices && i < hv_gain.size(); i++){
			avg += hv_gain[i];
		}
		hv_gain_collapsed.push_back( avg / nSlices );
	}



	v("\t[debug] time collapse hv loss", this->verbose_level, VERBOSE_MINIMAL );

	/*	( hv_loss 8) Temporally collapse by taking the mean over all time slices. */
	avg = 0;
	for(uint segmentStart = 0; segmentStart < hv_loss.size(); segmentStart += nSlices){
		for(uint i = segmentStart; i < segmentStart + nSlices && i < hv_loss.size(); i++){
			avg += hv_loss[i];
		}
		hv_loss_collapsed.push_back( avg / nSlices );
	}

	
	v("\t[debug] time collapse si gain", this->verbose_level, VERBOSE_MINIMAL );

	/* [si_gain (4) ] temporally collapse by computing the average of all blocks */
	avg = 0;
	for(uint segmentStart = 0; segmentStart < si_gain.size(); segmentStart += nSlices){
		for(uint i = segmentStart; i < segmentStart + nSlices && i < si_gain.size(); i++){
			avg += si_gain[i];
		}
		si_gain_collapsed.push_back( avg / nSlices );
	}

	

	v("\t[debug] time collapse ct ati gain", this->verbose_level, VERBOSE_MINIMAL );

	/* [ct_ati_gain (9)] Temporally collapse by sorting values in time 
		and selecting the 10% level.  */

	std::vector<double> cloned_ct_ati_gain(ct_ati_gain);

	for(uint segmentStart = 0; 
			segmentStart < ct_ati_gain.size(); 
			segmentStart += nSlices){

		len = uint(nSlices) > ct_ati_gain.size() - segmentStart ?
					ct_ati_gain.size() - segmentStart : nSlices;

		at = len * n_frames[segmentStart/nSlices] * 0.1;

		std::sort (	cloned_ct_ati_gain.begin() + segmentStart, 
					cloned_ct_ati_gain.begin() + segmentStart + len);


		ct_ati_gain_collapsed.push_back( ct_ati_gain[segmentStart + at] );
	}

	/* (si_loss 7) Temporally collapse by sorting the values in time
		 and selecting the 10% level*/ 

	v("\t[debug] time si loss", this->verbose_level, VERBOSE_MINIMAL );

	std::vector<double> cloned_si_loss(si_loss);

	for(uint segmentStart = 0; segmentStart < si_loss.size(); segmentStart += nSlices){
		

		len = uint(nSlices) > si_loss.size() - segmentStart ? 
				si_loss.size() - segmentStart : nSlices;

		at = len * 0.1;	

		std::sort (	cloned_si_loss.begin() + segmentStart, 
					cloned_si_loss.begin() + segmentStart + len);
		
		si_loss_collapsed.push_back( cloned_si_loss[segmentStart + at] );
	}
	v("\t[debug] time collapse chroma extreme", this->verbose_level, VERBOSE_MINIMAL );

	double mean, meansq;
	/* (chroma_extreme 3) Temporally collapse by computing standard deviation of the results */
	for(uint segmentStart = 0; 
			segmentStart < chroma_extreme.size(); 
			segmentStart += nSlices * n_frames[segmentStart/nSlices]){

		meansq = mean = 0;
		
		for(uint pos = segmentStart;
				pos < segmentStart + nSlices * n_frames[segmentStart/nSlices] 
					&& pos < chroma_extreme.size(); 
				pos++){

			meansq += chroma_extreme[pos] * chroma_extreme[pos];
			mean   += chroma_extreme[pos];
		}
		meansq /= nSlices * n_frames[segmentStart/nSlices];
		mean   /= nSlices * n_frames[segmentStart/nSlices];
		
		chroma_extreme_collapsed.push_back( meansq - mean * mean);
	}

/* double free or corruption down here*/ 

	v("\t[debug] time collapse chroma spread", this->verbose_level, VERBOSE_MINIMAL );

	/* (chroma_spread 5) Temporally collapse by sorting the values in time
		 and selecting the 10% level*/ 

	std::vector<double> cloned_chroma_spread(chroma_spread);
	
	int slice = 0;
	for(uint frame = 0; 
			frame < chroma_spread.size(); 
			frame += n_frames[slice++]){	

		std::sort (	cloned_chroma_spread.begin() + frame, 
					cloned_chroma_spread.begin() + frame + n_frames[slice] );
		
		chroma_spread_collapsed.push_back( cloned_chroma_spread[frame + n_frames[slice]*0.1] );
	} 


	return 0;
}

double VQM::getMetricValue(std::vector<double> *results){

	results->clear();
	
	/* calculate vqm per segment */ 
	int nSegments = si_gain_collapsed.size();
	
	double vqm_val;

	for(int i = 0; i < nSegments; i++){
		
		v("\t\t[debug] SEGMENT ", this->verbose_level, VERBOSE_DEFAULT );
		v(i, this->verbose_level, VERBOSE_DEFAULT );

		/* (chroma_spread 5) clip at a minimum value of 0.6. */
		clip(chroma_spread_collapsed[i], 0.6);

		/* [si_gain (5) ] clip at a minimum value of 0.004  */
		clip(si_gain_collapsed[i], 0.004);

		/* [si_gain (5) ] set all values greater than 0.14 to 0.14  */	
		if(si_gain_collapsed[i] > 0.14)			
			si_gain_collapsed[i] = 0.14;

		/* ( hv_loss 9) Square the parameter (i.e., non-linear scaling), 
			and clip at a minimum value of 0.06 */
		hv_loss_collapsed[i] *= hv_loss_collapsed[i];
		clip(hv_loss_collapsed[i], 0.06);


		v("[debug] si_loss_collapsed (after clip) : ", this->verbose_level, VERBOSE_DEFAULT );
		v(si_loss_collapsed[i], this->verbose_level, VERBOSE_DEFAULT );
		v("[debug] si_gain_collapsed: ", this->verbose_level, VERBOSE_DEFAULT );
		v(si_gain_collapsed[i], this->verbose_level, VERBOSE_DEFAULT );
		v("[debug] hv_loss_collapsed: ", this->verbose_level, VERBOSE_DEFAULT );
		v(hv_loss_collapsed[i], this->verbose_level, VERBOSE_DEFAULT );
		v("[debug] hv_gain_collapsed: " , this->verbose_level, VERBOSE_DEFAULT );
		v(hv_gain_collapsed[i], this->verbose_level, VERBOSE_DEFAULT );
		v("[debug] ct_ati_gain_collapsed: ", this->verbose_level, VERBOSE_DEFAULT );
		v(ct_ati_gain_collapsed[i], this->verbose_level, VERBOSE_DEFAULT );
		v("[debug] chroma_spread_collapsed: ", this->verbose_level, VERBOSE_DEFAULT );
		v(chroma_spread_collapsed[i], this->verbose_level, VERBOSE_DEFAULT );
		v("[debug] chroma_extreme_collapsed: ", this->verbose_level, VERBOSE_DEFAULT );
		v(chroma_extreme_collapsed[i], this->verbose_level, VERBOSE_DEFAULT );
		v("[debug] vqm value: : " , this->verbose_level, VERBOSE_DEFAULT );
		

		vqm_val =  
			si_loss_collapsed[i] * FACTOR_SI_LOSS +
			si_gain_collapsed[i] * FACTOR_SI_GAIN +
			hv_loss_collapsed[i] * FACTOR_HV_LOSS +
			hv_gain_collapsed[i] * FACTOR_HV_GAIN +
			ct_ati_gain_collapsed[i] * FACTOR_CT_ATI_GAIN +
			chroma_spread_collapsed[i] * FACTOR_CHROMA_SPREAD +
			chroma_extreme_collapsed[i] * FACTOR_CHROMA_EXTREME;

		//clip to a lower threshold of 0
		vqm_val = clip(vqm_val, 0);		
		//crushing function
		if(vqm_val > 1){
			vqm_val = (1.5) * vqm_val / (0.5+vqm_val);
		}
		//inverse
		vqm_val = 1-vqm_val;

		//clip again, for avoiding negative values
		vqm_val = clip(vqm_val, 0);		

		v(vqm_val, this->verbose_level, VERBOSE_DEFAULT );

		results->push_back(vqm_val);
	}

	return 0;
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

double VQM::euclideansq(double fo1, double fo2, double fp1, double fp2){
	/* returns the square of the euclidean distance */
	double t1 = (fo1 - fp1) ;
	double t2 = (fo2 - fp2) ;
	return t1*t1 + t2*t2;
}
double VQM::ratioComp(double reference, double processed){
	return (processed-reference)/reference;
}
double VQM::logComp(double reference, double processed){
	return log10(processed/reference);
}
 
double VQM::clip(double p, double t){
	if(p <= t)
		return 0;
	return p-t;
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
