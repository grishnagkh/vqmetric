# vqmetric 

a tool for calculating full-reference objective video quality metrics

#### Compilation

Compilation tested with Ubuntu 14.0.4 and OpenCV 2.4.8.0 

### Usage

#### Options
-r, --reference

	path to reference video file

-R, --reference-format <format>

	mp4
	y4m	 - yuv420 supported

-p, --processed

	path to processed video file

-P, --processed-format <format>

	mp4
	y4m	 - yuv420 supported

--psnr

	calculate peak signal to noise ratio

--ssim

	calculate structured similarity

--vqm 

	calculate video quality metric

-t, --time <t> 
	
	temporally collapse in time
	t is a comma separated list with segment length in seconds, e.g. 1,2,4 
	default value: 2


-h, --help

	print this message

-v <level>, --verbose <level>

	0- silent

	1- verbose

	2- verbose extended (same as verbose, for now) 

	3- verbose debug

-l <level>, --log <level>

	0- log silent

	1- log psnr and ssim per frame

	3- additionally log vqm 

sample call

	./vqtool -p <processed video> -r <reference video> --psnr --ssim out


#### Features

 * PSNR
	As PSNR depends on the input sample depth, we just assume input samples of 8bit. This results when processing 16bit data in an offset of ~48dB, the formula forcalculation is 20*(log(2^8-1))-log(2^B-1))/log(10), with B as the number of bits per sample

 * SSIM
 * VQM
 
#### Supported Input Formats

Supported & tested input formats are 
 * mp4 (8bit samples)
 * y4m (YUV420, 8bit samples)
  
##### Work in Progress

 * Features
   * MSSSIM
 * Support for different resolutions
  
 
