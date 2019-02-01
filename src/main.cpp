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
 * 	./vqtool -p 500.mp4 -P mp4 -r 1000.mp4 -R mp4 --psnr --ssim out
 *  ./vqtool -p test.y4m -P y4m -r test.y4m -R y4m --psnr --verbose 3
 *  ./vqtool -p test.y4m -P y4m -r test.y4m -R y4m --ssim --verbose 3
 *	./vqtool -p 500.mp4 -P mp4 -r 1000.mp4 -R mp4 --ssim --psnr --vqm --verbose 1 --log 1 -t 1,2,4
 *	./vqtool -p processed.mp4 -P mp4 -r reference.mp4 -R mp4 --ssim --psnr --verbose 1 --log 1 -t 1,2,4,6,8
 *  ./vqtool -p processed.mp4 -P mp4 -r reference.mp4 -R mp4 --vqm --verbose 1 --log 3 -t 1,2,4,6,8
 *	./vqtool -p processed.mp4 -P mp4 -r reference.mp4 -R mp4 --ssim --psnr --verbose 1 --log 3 -t 1,2,4,6,8 --dashready
 */


int main(int argc, char **argv){	

	std::ostringstream d;

	if(argc == 1){
		printUsage();
		exit(0);
	}

	/* begin option parsing */
	int c;
	/* flags */
	int psnr_flag = 0;
	int ssim_flag = 0;
	int vqm_flag = 0;
	int dashready_flag = 0;
	bool console_output_flag = false;

	int verbose = 0;
	int loglvl = 0;

        bool ignore_framerate = false;

	string timesString="", processed, reference, procfmt, reffmt, outputmode, scaling="auto", out_prefix;
	
	while (1){
		static struct option long_options[] = {	
			  {"psnr", no_argument, &psnr_flag, 1},
			  {"ssim", no_argument, &ssim_flag, 1},
			  {"vqm",  no_argument, &vqm_flag , 1},

			  {"dashready",  no_argument, &dashready_flag , 1},

 			 /* These options don’t set a flag.
				 We distinguish them by their indices. */
			  {"help",  			no_argument, 		0, 'h'},
			  {"verbose",  			required_argument, 	0, 'v'},
			  {"log",  				required_argument, 	0, 'l'},
			  {"time",  			required_argument,	0, 't'}, // path to source file
			  {"processed",  		required_argument,	0, 'p'}, // path to source file
			  {"reference",  		required_argument, 	0, 'r'}, // path to reference file
			  {"processed-format",  required_argument, 	0, 'P'}, // try to estimate by file ending
			  {"reference-format",  required_argument,  0, 'R'}, // try to estimate by file ending
			  {"output",  			required_argument, 	0, 'o'}, // standard, extended, pedantic
			  {"scaling",  			required_argument, 	0, 'c'}, // lanczos, bicubic scaling
			  {"ignore-frame-rate", 	no_argument, 	0, 'F'},
                          {"console-output",            no_argument,    0, 'C'},
			  {0, 0, 0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "t:p:r:R:P:o:c:v:l:hFC",
				       long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1) break;

		switch (c){
			case 0:
				/* If this option set a flag, do nothing else now. */				
			break;
			case 't':
				timesString = optarg;
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
			case 'l':
				loglvl = optarg[0] - '0';				
				break;
			case 'h':
				printUsage();
				exit(0);
				break;
                        case 'F':
                                ignore_framerate = true;
                                break;
                        case 'C':
                                console_output_flag = true;
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

	VideoReader * rR;
	VideoReader * pR;

	if(	(strncmp("mp4",procfmt.c_str(), 3) == 0 )){
		pR = new VideoCaptureReader(processed);
		dbg("[debug] intialized VideoCaptureReader for processed video sequence\n" , verbose);
	}else if((strncmp("y4m",procfmt.c_str(), 3) == 0 )){
		pR = new Y4MReader(processed);
		dbg("[debug] intialized Y4MReader for processed video sequence\n", verbose);
	}else{
		if(verbose > VERBOSE_SILENT){
			std::cout << "you did not specify an input format for the processed video, we assume mp4" << std::endl;
		}
		pR = new VideoCaptureReader(processed);	
		dbg("[debug] intialized VideoCaptureReader for processed video sequence\n" , verbose);
	}

	if(	(strncmp("mp4",reffmt.c_str(), 3) == 0 )){
		rR = new VideoCaptureReader(reference);
		dbg("[debug] intialized VideoCaptureReader for reference video sequence\n", verbose);
	}else if(	(strncmp("y4m",reffmt.c_str(), 3) == 0 )){
		rR = new Y4MReader(reference);
		dbg("[debug] intialized Y4MReader for reference video sequence\n" , verbose);
	}else{
		if(verbose > VERBOSE_SILENT){
			std::cout << "you did not specify an input format for the reference video, we assume mp4" << std::endl;
		}
		rR = new VideoCaptureReader(reference);
		dbg("[debug] intialized VideoCaptureReader for reference video sequence\n", verbose);
	}

	dbg("[debug] requesting fps...\n", verbose);


	if(!ignore_framerate && rR->getFps() != pR->getFps()){
		std::cout << "original and processed video have different frame rates, we abort here";
		return -1;
	}
	dbg("[debug] requesting number of frames...\n", verbose);
	if(rR->getNFrames() != pR->getNFrames()){
		if(verbose > VERBOSE_SILENT)
			std::cout << "original and processed video have different number of frames" << std::endl;
		//return -1;
	}
	dbg("[debug] requesting video width...\n", verbose);
	if(rR->getVideoWidth() != pR->getVideoWidth() 
			|| rR->getVideoHeight() != pR->getVideoHeight()){
		std::cout << "original and processed video have different dimensions, we scale using method " << scaling << std::endl;
		std::cout << " (scaling not implemented yet)" << std::endl;
		return -1;
	}
	
	// process .2 seconds slices 
	int framesPerSlice = rR->getFps() * 0.2; 
	dbg("[debug] ", verbose);
	dbg(framesPerSlice, verbose);
	dbg(" frames per slice\n" , verbose);

	remove( (out_prefix + "_psnr.log").c_str() );
	remove( (out_prefix + "_ssim.log").c_str() );
	remove( (out_prefix + "_vqm.log").c_str() );

	PSNR psnr(out_prefix + "_psnr.log", loglvl);
	SSIM ssim(out_prefix + "_ssim.log", loglvl);
	VQM vqm(out_prefix + "_vqm.log", loglvl, verbose);


	cv::Mat read1[framesPerSlice];
	cv::Mat read2[framesPerSlice];

	cv::Mat ref[framesPerSlice][3];
	cv::Mat proc[framesPerSlice][3];
	
	cv::Mat tmp;

	bool frame_avail = true;
	int i; //frames grabbed

	int nSlices = 0;
	int nFrames = 0;

	if(verbose == VERBOSE){		
		std::cout << "read frames:            0";
	}

	while(frame_avail){

		i = 0;

		while(i < framesPerSlice){
			
			frame_avail = rR->nextFrame(read1[i]);
			frame_avail = frame_avail && pR->nextFrame(read2[i]);

			if(!frame_avail) 
				break;
	
			nFrames++;
			
			cv::split( read1[i], ref[i] ); 
			cv::split( read2[i], proc[i] ); 

			i++;
	
			if(verbose == VERBOSE){
				std::cout << string(floor(log(nFrames) / log(10))+1, '\b');	
				std::cout << nFrames;
				cout << flush;
			}	

		}	


		if(!i) break; //no frames in slice
		
		dbg("[debug]  calculating over ", verbose);
		dbg(i, verbose);
		dbg(" frames\n", verbose);
			
		if(psnr_flag){
			psnr.compute(ref, proc, i);
		}

		if(ssim_flag){
			ssim.compute(ref, proc, i);
		}

		if(vqm_flag){
			vqm.compute(ref, proc, i);
		}
		
		nSlices++;	
	
	}
	
	
	if(verbose == VERBOSE){
		std::cout << std::endl;
	}

	if(timesString.size() == 0){
		std::cerr << "no time argument calculating over video\n" ;
		timesString="2";
	}

	
	vector<string> time_vector = split(timesString, ',');
	
	//1 slice hat 0.2 sekunden
	
	std::ofstream resfile;
	std::ofstream mpd_file;

	std::vector<double> psnr_res;
	std::vector<double> ssim_res;
	std::vector<double> vqm_res;

	int time;
	int slices;
	std::string filename;
	std::string mpd_filename;

	int size;

	for(uint i=0; i<time_vector.size(); i++){
		
		time = atoi(time_vector[i].c_str());
		slices = time * rR->getFps()/ framesPerSlice;

		if(psnr_flag){
			psnr_res.clear();

			dbg("psnr time collapse", verbose);
			psnr.timeCollapse(slices);
			dbg("psnr get metric value", verbose);
			psnr.getMetricValue(&psnr_res);	
			dbg("psnr write results", verbose);		

			filename = out_prefix + "_psnr_" + time_vector[i] + "s.csv";
			remove( filename.c_str() ); //remove file if exists
			resfile.open (filename.c_str(), std::ios::out | std::ios::app); 
			for(std::vector<int>::size_type i = 0; i < psnr_res.size(); i+=1) {
				resfile << i << ";" << psnr_res[i] << std::endl;
				dbg(psnr_res[i], verbose);
			}	
			resfile.close();
			size = psnr_res.size();
		}
		if(ssim_flag){
			ssim_res.clear();

			dbg("ssim time collapse", verbose);
			ssim.timeCollapse(slices);
			dbg("ssim get metric value", verbose);
			ssim.getMetricValue(&ssim_res);
			dbg("ssim write results", verbose);	
	
			filename = out_prefix + "_ssim_" + time_vector[i] + "s.csv";
			remove( filename.c_str() ); //remove file if exists
			resfile.open (filename.c_str(), std::ios::out | std::ios::app);
			for(std::vector<int>::size_type i = 0; i < ssim_res.size(); i+=1) {
				resfile << i << ";" << ssim_res[i] << std::endl;
				dbg(ssim_res[i], verbose);
			}
			resfile.close();
			size = ssim_res.size();
		}
		if(vqm_flag){
			vqm_res.clear();

			dbg("vqm time collapse", verbose);
			vqm.timeCollapse(slices);
			dbg("vqm get metric value", verbose);
			vqm.getMetricValue(&vqm_res);
			dbg("vqm write results", verbose);	

			filename = out_prefix + "_vqm_" + time_vector[i] + "s.csv";
			remove( filename.c_str() ); //remove file if exists
			resfile.open (filename.c_str(), std::ios::out | std::ios::app); 

			for(std::vector<int>::size_type i = 0; i < vqm_res.size(); i+=1) {
				resfile << i << ";" << vqm_res[i] << std::endl;
				std::cout << vqm_res[i];
			}	
			resfile.close();
			size = vqm_res.size();	
		}

		if(dashready_flag){
			/* print mpd file extension to file */
			mpd_filename = out_prefix + "_mpdExtension_" + time_vector[i] + "s.csv";
			remove( mpd_filename.c_str() ); //remove file if exists
			mpd_file.open (mpd_filename.c_str(), std::ios::out | std::ios::app); 

			mpd_file << "<Quality>" << std::endl;
			for(int i = 0; i < size ; i+=1) {

				mpd_file << "\t<SegInfo SegNr=\"" << (i+1) << "\" QoS=\"";
				if(psnr_flag) 
					mpd_file << "(PSNR," << psnr_res[i] << ")";

				if(psnr_flag && (ssim_flag || vqm_flag)) 
					mpd_file << ";";

				if(ssim_flag) 
					mpd_file << "(SSIM," << ssim_res[i] << ")";

				if(ssim_flag && vqm_flag) 
					mpd_file << ";";
				if(vqm_flag)  
					mpd_file << "(VQM,"  << vqm_res[i] << ")";

				mpd_file << "\"";
				mpd_file << "/>";
				mpd_file << std::endl;
			}
			mpd_file << "</Quality>" << std::endl;
			mpd_file.close();
		}

                if (console_output_flag) {
                        if(psnr_flag)
                          std::cout << psnr_res[i] << " ";

                        if(ssim_flag)
                          std::cout << ssim_res[i] << " ";

                        std::cout << "\n";
                }
	}

	return 0;
}


std::vector<string> split(string str, char delimiter) {

  std::vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;
  
  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }
  
  return internal;
}

void dbg(string x, int verbose){
	if(verbose >= VERBOSE_DEBUG)
		std::cout << x;
}
void dbg(int x, int verbose){
	if(verbose >= VERBOSE_DEBUG)
		std::cout << x;
}
void dbg(double x, int verbose){
	if(verbose >= VERBOSE_DEBUG)
		std::cout << x;
}
void printUsage(){
 std::cout << "-r, --reference\n\tpath to reference video file" << std::endl;
 std::cout << "-R, --reference-format <format>" << std::endl;
 std::cout << "\tmp4" << std::endl;
 std::cout << "\ty4m\t - yuv420 supported" << std::endl;

 std::cout << "-p, --processed\n\tpath to processed video file" << std::endl;
 std::cout << "-P, --processed-format <format>" << std::endl;
 std::cout << "\tmp4" << std::endl;
 std::cout << "\ty4m\t - yuv420 supported" << std::endl;


 std::cout << "--psnr\n\tcalculate peak signal to noise ratio" << std::endl;
 std::cout << "--ssim\n\tcalculate structured similarity" << std::endl;
 std::cout << "--vqm\n\tcalculate vqm" << std::endl;

 std::cout << "--dashready\n\tproduce dashready output to insert into mpds" << std::endl;

 std::cout << "-t, --time <t> \n\ttemporally collapse in time" << std::endl;
 std::cout << "\tt is a comma separated list with segment length in seconds, e.g. 1,2,4 " << std::endl;
 std::cout << "\tdefault value: 2" << std::endl;


 std::cout << "-h, --help\n\tprint this message" << std::endl;
 std::cout << "-v <level>, --verbose <level>" << std::endl;
 std::cout << "\t" << VERBOSE_SILENT << "- silent" << std::endl;
 std::cout << "\t" << VERBOSE << "- verbose" << std::endl;
 std::cout << "\t" << VERBOSE_EXTENDED << "- verbose extended (same as verbose, for now) " << std::endl;
 std::cout << "\t" << VERBOSE_DEBUG << "- verbose debug" << std::endl;

 std::cout << "-F, --ignore-frame-rate\n";
 std::cout << "\tDo not check for equal frame-rates in input streams\n";
 std::cout << "-C, --console-output\n";
 std::cout << "\tWrite measurement results to console\n";

 std::cout << "sample call" << std::endl;
 std::cout << "\t./vqtool -p <processed video> -r <reference video> --psnr --ssim out" << std::endl;

}
