# vqmetric 

a tool for calculating full-reference objective video quality metrics

#### Compilation

Compilation tested with Ubuntu 14.0.4 and OpenCV 2.4.8.0 

### Usage

#### Options

-r, --reference

	path to reference video file

-p, --processed

	path to processed video file

--psnr

	calculate peak signal to noise ratio

--ssim

	calculate structured similarity

-h, --help

	print this message

-v <level>, --verbose <level>

	0- silent

	1- verbose

	2- verbose extended (same as verbose, for now) 

	3- verbose debug

sample call

	./vqtool -p <processed video> -r <reference video> --psnr --ssim out



#### Features

 * PSNR
 * SSIM
 
#### Supported Input Formats

Tested input formats are 
 * mp4
 * y4m (opencv version, seems to interpret header as frames)
  
##### Work in Progress

 * Features
   * MSSSIM
   * VQM
 * Support for different resolutions
  
 
