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

#ifndef Main_hpp
#define Main_hpp

#include "opencv2/opencv.hpp"
#include "VideoCaptureReader.hpp"
#include "VideoReader.hpp"
#include "PSNR.hpp"
#include "SSIM.hpp"
#include "VQM.hpp"

#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>


int main(int, char**);
void printUsage();

const int VERBOSE_SILENT 	= 0;
const int VERBOSE 			= 1;
const int VERBOSE_EXTENDED 	= 2;
const int VERBOSE_DEBUG 	= 3;

const int LOG_NONE 			= 0;
const int LOG_RESULTS 		= 1;
const int LOG_DEBUG 		= 2;


#endif
