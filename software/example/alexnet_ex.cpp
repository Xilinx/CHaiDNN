/*----------------------------------------------------
Copyright 2017 Xilinx, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
----------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../interface/xi_interface.hpp"

#if 1//def __SDSCC__
#undef __ARM_NEON__
#undef __ARM_NEON
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#define __ARM_NEON__
#define __ARM_NEON
#else
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#endif

#include <iostream>

#ifdef __SDSOC
#include "sds_lib.h"
#endif

//# Performance check
#ifdef __SDSOC
long long int clock_start, clock_end, frequency;
#define TIME_STAMP_INIT  clock_start = sds_clock_counter();
#define TIME_STAMP  { \
		clock_end = sds_clock_counter(); \
		frequency = sds_clock_frequency(); \
}
#endif

using namespace std;
using namespace cv;

int loadMeanSubtractedDatatoBuffptr(const char *img_path, short *buff_ptr, int height, int width, int img_channel, int resize_h, int resize_w, short *mean_ptr, int fbits_input)
{

	cv::Mat frame;

	frame = imread(img_path, 1);
	if(!frame.data)
	{
		std :: cout << "[ERROR] Image read failed - " << img_path << std :: endl;
		return -1;
	}
	else
	{
		std :: cout << "[IMRDx] Image read : " << img_path << std :: endl;
	}

	int h_off = 0;
	int w_off = 0;

	cv::Mat cv_img[1], cv_cropped_img[1];

	if((resize_h != frame.rows) || (resize_w != frame.cols))
	{
		cv::resize(frame, cv_img[0], cv::Size(resize_h, resize_w));

		if(resize_h > height)
		{
			h_off = (resize_h - height) / 2;
			w_off = (resize_w - width) / 2;
			cv::Rect roi(w_off, h_off, height, width);
			cv_cropped_img[0] = cv_img[0](roi);
		}
		else
			cv_cropped_img[0] = cv_img[0];
	}
	else
	{
		frame.copyTo(cv_cropped_img[0]);
	}


	//float pixel;
	uchar pixel;
	int input_index=0;
	float float_val;
	short fxval;
	int mean_idx = 0;

	for (int h = 0; h < height; ++h)
	{
		const uchar* ptr = cv_cropped_img[0].ptr<uchar>(h);//data;
		int img_index = 0;
		for (int w = 0; w < width; ++w)
		{
			for (int ch = 0; ch < img_channel; ++ch)
			{
				//pixel = static_cast<float>(ptr[img_index++]);
				pixel = ptr[img_index++];

				short pixel1 = (short)pixel << fbits_input;

				mean_idx = (ch * resize_h + h + h_off) * resize_w + w + w_off;
				short mean_fxval = mean_ptr[mean_idx];

				//short ival = (short)float_mean;
				//short mean_fxval = ConvertToFP(float_mean, ival, fbits_input);
				fxval = pixel1 - mean_fxval;

				buff_ptr[input_index] = fxval;

				input_index++;
			}// Channels
		}// Image width

	}// Image Height


}

int main(int argc, char **argv)
{
	//# Create vector for buffer pointers
	bufPtrs ptrsList;
	
	//# Create Job-Queue
	std::vector<xChangeLayer> jobQueue[NUM_IMG];

	//# Create input/output Buffers
	void *input = malloc(227*227*3*sizeof(short));
	//# Create memory with max size of output layer
	void *output = malloc(1024*sizeof(float));
	
	//# Get Path
	char *dirpath    = "/mnt/models/AlexNet"; 		/* Path to the model directory */
	char *prototxt   = "deploy.prototxt";			/* Prototxt file name residing in model directory */
	char *caffemodel = "AlexNet.caffemodel";		/* caffemodel file name residing in model directory */
	char *mean_file	 = "mean.binaryproto";				/* mean file name residing in model directory */
	int  mean_type	 = 1;							/* 0 for .txt, 1 for .binaryproto */

	char *img_path  = "/mnt/models/AlexNet/input/camel.jpg";	/* Correct the path accordingly */
	
	//# Call Initialization: One time initalization of network params
	xiInit(dirpath, prototxt, caffemodel, mean_file, mean_type, jobQueue, ptrsList, 256, 256);

	//# The Following function "loadImagetoBuffptr" is used only for example purpose. It takes image path as input.
	//# If application have video input, please copy frame buffer into input1 & input2 buffers and use them in "xiExec" 
	int inp_fbits   = jobQueue[0][0].qf_format.ip_fbits;
	short *mean_ptr = (short *)jobQueue[0][0].mean;

	loadMeanSubtractedDatatoBuffptr(img_path, (short *)input, 227, 227, 3, 256, 256, mean_ptr, inp_fbits);
	
	int inBytes = 2;
	//# Execute the network for input image
	TIME_STAMP_INIT
	xiExec(jobQueue, input, output, inBytes);	/* Call this in a loop for running multiple times with new input/output buffers */
	TIME_STAMP
	
	//# Total time for the API in Images/Second
	double tot_time = (((double)(clock_end-clock_start)/(double)frequency)*1000)/(double)NUM_IMG;
	fprintf(stderr, "\n[PERFM] Performance : %lf Images/second\n", (double)(1000)/tot_time);
	fprintf(stderr, "\n\n");
	
	//# Write file for checking error
	FILE *out = fopen("/mnt/models/AlexNet/out/out.txt", "w");
	
	//# Write output to a file
	for(int i = 0; i < 1000; ++i)
	{
		fprintf(out, "%f\n", ((float*)output)[i]);
	}
	fclose(out);
	
	//# Call Release
	xiRelease(ptrsList); /* Release before exiting application */
	fprintf(stderr, "[INFOx] Memory Released\n"); 
	return 0;
}