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


#include "main.hpp"


/* 
 * Sample call 
 *
 * 	./main -p 500.mp4 -P mp4 -r 1000.mp4 -R mp4 --psnr --ssim out
 *
 */

int main(int argc, char **argv){	

	if(argc == 0){
		printUsage();
		exit(0);
	}

	/* begin option parsing */
	int c;
	/* flags */
	int psnr_flag = 0;
	int ssim_flag = 0;
	int vqm_flag = 0;

	int verbose = 0;

	string processed, reference, procfmt, reffmt, outputmode, scaling, out_prefix;
	
	while (1){
		static struct option long_options[] = {	
			  {"psnr", no_argument, &psnr_flag, 1},
			  {"ssim", no_argument, &ssim_flag, 1},
			  {"vqm",  no_argument, &vqm_flag , 1},

 			 /* These options don’t set a flag.
				 We distinguish them by their indices. */
			  {"help",  			no_argument, 		0, 'h'},
			  {"verbose",  			required_argument, 	0, 'v'},
			  {"processed",  		required_argument,	0, 'p'}, // path to source file
			  {"reference",  		required_argument, 	0, 'r'}, // path to reference file
			  {"processed-format",  required_argument, 	0, 'P'}, // try to estimate by file ending
			  {"reference-format",  required_argument,  0, 'R'}, // try to estimate by file ending
			  {"output",  			required_argument, 	0, 'o'}, // standard, extended, pedantic
			  {"scaling",  			required_argument, 	0, 'c'}, // lanczos, bicubic scaling
			  {0, 0, 0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "p:r:R:P:o:c:vh",
				       long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1) break;

		switch (c){
			case 0:
				/* If this option set a flag, do nothing else now. */				
				break;
			case 'p':
				processed = optarg;
				break;
			case 'r':
				reference = optarg;
				break;
			case 'P':
				procfmt = optarg;
				break;
			case 'R':
				reffmt = optarg;
				break;
			case 'o':
				outputmode = optarg;
				break;		
			case 'c':
				std::cout << "scaling mode: " << optarg  << std::endl;
				scaling = optarg;
				break;
			case 'v':
				verbose = optarg[0] - '0';
				if(verbose < VERBOSE_SILENT || verbose > VERBOSE_DEBUG){
					std::cout << "illegal verbose level: " << verbose << std::endl;
					printUsage();
					exit(-1);
				}
				break;
			case 'h':
				printUsage();
				exit(0);
				break;		 
		}
	}
	if (optind < argc){
		out_prefix = argv[optind];
	}else{
		out_prefix = "out";
	}

	if(verbose > VERBOSE_SILENT){
		std::cout << "calculating" 	<< std::endl;	
		std::cout << "\tpsnr: " << psnr_flag << std::endl;
		std::cout << "\tssim: " << ssim_flag << std::endl;
		std::cout << "\tvqm:  " << vqm_flag  << std::endl;
	}

/* option parsing end */

	VideoReader *rR, *pR;
	
	 
	VideoCaptureReader o(reference);
	rR = &o;
	 
	VideoCaptureReader p(processed);
	pR = &p;
	 

	if(rR->getFps() != pR->getFps()){
		std::cout << "original and processed video have different frame rates, we abort here";
		return -1;
	}
	if(rR->getNFrames() != pR->getNFrames()){
		if(verbose > VERBOSE_SILENT)
			std::cout << "original and processed video have different number of frames" << std::endl;
		//return -1;
	}
	if(rR->getVideoWidth() != pR->getVideoWidth()){
		std::cout << "original and processed video have different widths, we abort here";
		std::cout << " (scaling not implemented yet)";
		return -1;
	}
	if(rR->getVideoHeight() != pR->getVideoHeight()){
		std::cout << "original and processed video have different heights, we abort here";
		std::cout << " (scaling not implemented yet)";
		return -1;
	}
	
	// process .2 seconds slices 
	int framesPerSlice = rR->getFps() * 0.2; 
	if(verbose == VERBOSE_DEBUG)
		std::cout << framesPerSlice << " frames per slice" << std::endl;

	remove( (out_prefix + "_psnr.csv").c_str() );
	remove( (out_prefix + "_ssim.csv").c_str() );
	remove( (out_prefix + "_vqm.csv").c_str() );


//reserving for 10000 slices,.,, later we will rertie to vectors 
	PSNR psnr(out_prefix + "_psnr", LOG_RESULTS);
	SSIM ssim(out_prefix + "_ssim", LOG_RESULTS);

	VQM vqm(out_prefix + "_vqm", LOG_DEBUG, VERBOSE_DEBUG); //just until we have vectors, todo rewrite to vectors

	cv::Mat read1[framesPerSlice];
	cv::Mat read2[framesPerSlice];
	cv::Mat read11[framesPerSlice];
	cv::Mat read21[framesPerSlice];

	cv::Mat ref[framesPerSlice][3];
	cv::Mat proc[framesPerSlice][3];
	
	cv::Mat tmp;

	bool frame_avail = true;
	int i; //frames grabbed

	while(frame_avail){ 
		i = 0;
		while(i < framesPerSlice){
			frame_avail = rR->nextFrame(tmp);
			tmp.convertTo(read1[i], CV_32F);
			
			frame_avail = frame_avail && pR->nextFrame(tmp);
			tmp.convertTo(read2[i], CV_32F);

			if(!frame_avail) break;

			cv::cvtColor(read1[i], read11[i], CV_BGR2YCrCb);		
			cv::cvtColor(read2[i], read21[i], CV_BGR2YCrCb);	

			cv::split( read11[i], ref[i] ); 
			cv::split( read21[i], proc[i] ); 

			i++;
		}	
		if(!i) break; //no frames in slice
		
		if(verbose == VERBOSE_DEBUG)
			std::cout << "[debug] calculating over " << i << " frames: " ;
	
		if(psnr_flag){
			if(verbose == VERBOSE_DEBUG)
				std::cout << "[debug] psnr: " << psnr.compute(ref, proc, i) << std::endl;
			else
				psnr.compute(ref, proc, i);
		}

		if(ssim_flag){
			if(verbose == VERBOSE_DEBUG)
				std::cout << "[debug] ssim: " << ssim.compute(ref, proc, i) << std::endl;
			else
				ssim.compute(ref, proc, i);
		}

		if(vqm_flag){
			vqm.compute(ref, proc, i);
		}
	
	}
	
	if(verbose > VERBOSE_SILENT){
		std::cout << "cumulated PSNR over video " << psnr.getMetricValue() << std::endl;
		std::cout << "cumulated SSIM over video " << ssim.getMetricValue() << std::endl;
	}
	
	return 0;	
}

void printUsage(){
 std::cout << "-r, --reference\n\tpath to reference video file" << std::endl;
 std::cout << "-p, --processed\n\tpath to processed video file" << std::endl;
 std::cout << "--psnr\n\tcalculate peak signal to noise ratio" << std::endl;
 std::cout << "--ssim\n\tcalculate structured similarity" << std::endl;
 std::cout << "-h, --help\n\tprint this message" << std::endl;
 std::cout << "-v <level>, --verbose <level>" << std::endl;
	std::cout << "\t" << VERBOSE_SILENT << "- silent" << std::endl;
	std::cout << "\t" << VERBOSE << "- verbose" << std::endl;
	std::cout << "\t" << VERBOSE_EXTENDED << "- verbose extended (same as verbose, for now) " << std::endl;
	std::cout << "\t" << VERBOSE_DEBUG << "- verbose debug" << std::endl;
	

 std::cout << "sample call" << std::endl;
 std::cout << "\t./vqtool -p <processed video> -r <reference video> --psnr --ssim out" << std::endl;

}
