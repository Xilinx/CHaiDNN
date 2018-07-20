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
#include <iterator>
#include <numeric>
#include <iostream>

#include "../interface/xi_interface.hpp"
#include "../interface/xi_readwrite_util.hpp"

#undef __ARM_NEON__
#undef __ARM_NEON
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#define __ARM_NEON__
#define __ARM_NEON

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

typedef signed char int8_t;

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
	if(argc < 5) {
		std::cout << "Usage : ./sw_layer_plugin.elf <quant_mode> <bit_width> <img1_path> <img2_path>" << std::endl;
		std::cout << "quant_mode : DynamicFixed" << std::endl;		
		std::cout << "bit_width : 6" << std::endl;
		std::cout << "img1_path : Path to the first image" << std::endl;
		std::cout << "img2_path : Path to the second image" << std::endl;
		return -1;			
	}

	string quant_mode_arg(argv[1]);
	int bit_width_arg   = atoi(argv[2]);
	char *img_path1 	= argv[3];
	char *img_path2 	= argv[4];
	
	char *dirpath; 
	char *caffemodel;
	char *prototxt;
	
	int resize_h = 224;
	int resize_w = 224;
	bool inp_mode;
	
	// Xilinx quantmode 6 bit
	if(quant_mode_arg == "DynamicFixed" && bit_width_arg == 6) {
		dirpath    = "models/GoogleNetWithoutLRN/6Bit";
		caffemodel = "GoogleNetNoLRN_6Bit_CHaiDNN.caffemodel";
		prototxt   = "GoogleNetNoLRN_6Bit_CHaiDNN_Customlayer.prototxt";
        inp_mode   = 1;
	}
	else {
		std::cout << "Usage : ./custom_ex.elf <quant_mode> <bit_width> <img1_path> <img2_path>" << std::endl;
		std::cout << "quant_mode : DynamicFixed" << std::endl;		
		std::cout << "bit_width : 6" << std::endl;
		std::cout << "img1_path : Path to the first image" << std::endl;
		std::cout << "img2_path : Path to the second image" << std::endl;
		return -1;
	}
	
	std :: cout << "[INFOx] Network Path : " << dirpath << std :: endl;

	//# start layer in the graph
	string start_layer = "";

	//# end layer in the graph
	string end_layer   = "";

	//# Flag indicate layer1 or not
	//# 1 - if image data is the for first layer input otherwise 0
	bool is_first_layer = 1;

	//# Number of images to process
	//# Supported batch size is 2
	int numImg_to_process = 2;

	//# Struct which holds the input/output layer info
	io_layer_info io_layer_info_ptr;

	//####### Init call
	void *chaihandle = xiInit(dirpath, prototxt, caffemodel, &io_layer_info_ptr,
			numImg_to_process, is_first_layer, start_layer, end_layer);

			
	float *mean_ptr = (float*)malloc(3*sizeof(float));
	if(mean_ptr == NULL)
	{
		fprintf(stderr, "Failed to create mean_ptr memory\n");
		return -1;
	}
		
	float *var_ptr = (float*)malloc(3*sizeof(float));
	if(var_ptr == NULL)
	{
		fprintf(stderr, "Failed to create var_ptr memory\n");
		return -1;
	}
	
	//# With input Normalization 
	if(inp_mode == 1)
	{
		mean_ptr[0] = 0.485;
		mean_ptr[1] = 0.456;
		mean_ptr[2] = 0.406;
		var_ptr[0] = 0.229;
		var_ptr[1] = 0.224;
		var_ptr[2] = 0.225;
	}
	else
	{
		mean_ptr[0] = 104.0;
		mean_ptr[1] = 117.0;
		mean_ptr[2] = 123.0;
	}
	
	//# Create buffer to load normalized input
	vector<void *> normalizeInput;
	for(int batch_id = 0; batch_id < numImg_to_process; batch_id++)
	{
		void *ptr = (int8_t *)malloc(resize_h*resize_w*3*sizeof(int8_t));
		if(ptr == NULL)
		{
			fprintf(stderr, "[ERROR] Failed to create input memory\n");
			return -1;
		}

		normalizeInput.push_back(ptr);
	}
	
	int status = inputNormalization(normalizeInput, resize_h, resize_w, img_path1, img_path2,
			inp_mode, mean_ptr, var_ptr, numImg_to_process, io_layer_info_ptr);

	if(status == -1)
	{
		fprintf(stderr,"[ERROR] Image Read fail\n");
		return -1;
	}
	else
	{
		fprintf(stderr,"[INFOx] Image Read Done\n");
	}

	int in_size = io_layer_info_ptr.inlayer_sizebytes;

	//# Create input/output Buffers
	vector<void *> input;

	void *ptr;
	for(int i = 0; i < io_layer_info_ptr.num_in_bufs; i++)
	{
		if(io_layer_info_ptr.inlayer_exectype.compare("hardware") == 0)
		{
#ifdef __SDSOC
			ptr = sds_alloc_non_cacheable(in_size);
#else
			ptr = malloc(in_size);
#endif
		}
		else
			ptr = malloc(in_size);

		if(ptr == NULL)
		{
			fprintf(stderr, "Failed to create input memory\n");
			return -1;
		}
		input.push_back(ptr);
	}

	int out_size = io_layer_info_ptr.outlayer_sizebytes;
	vector<void *> output;

	for(int i = 0; i < io_layer_info_ptr.num_out_bufs; i++)
	{
		if(io_layer_info_ptr.outlayer_exectype.compare("hardware") == 0)
		{
#ifdef __SDSOC
			ptr = sds_alloc_non_cacheable(out_size);
#else
			ptr = malloc(out_size);
#endif
		}
		else
			ptr = malloc(out_size);

		if(ptr == NULL)
		{
			fprintf(stderr, "Failed to create input memory\n");
			return -1;
		}
		output.push_back(ptr);
	}

	fprintf(stderr,"[INFOx] Input Read Done\n");

	xiInputRead(normalizeInput, input, numImg_to_process, io_layer_info_ptr);

	int loop_iter = 100;
	
#ifdef __SDSOC
		TIME_STAMP_INIT
#endif

	//# Run the loop for loop_iter times
	for(int i = 0; i < loop_iter; i++)
	{
		//# Execute the network for input image
		xiExec(chaihandle, input, output);
	}
	
#ifdef __SDSOC
		TIME_STAMP
#endif

#ifdef __SDSOC
		//# Total time for the API in Images/Second
	double tot_time = (((double)(clock_end-clock_start)/(double)frequency*1000));
	fprintf(stderr, "\n[PERFM] Performance with Batching : %lf Images/second\n", ((double)(1000)/tot_time)*XBATCH_SIZE*(loop_iter));
	fprintf(stderr, "\n\n");
#endif
	
	//# Display latency of custom and Pack/Unpack layers
	getPerfInfo(chaihandle);

	int unpack_out_size = io_layer_info_ptr.outlayer_sizebytes;

	//# Create memory for unpack output data
	vector<void *> unpack_output;
	for(int batch_id = 0; batch_id < numImg_to_process; batch_id++)
	{
		void *ptr = malloc(unpack_out_size);
		unpack_output.push_back(ptr);
	}

	//# Loading required params for unpack function
	kernel_type_e out_kerType = io_layer_info_ptr.out_kerType;
	int out_layer_size = io_layer_info_ptr.out_size;

	cout << "[INFOx] Unapack output data" << endl;
	//# unpacks the output data
	xiUnpackOutput(output, unpack_output, out_kerType, out_layer_size, numImg_to_process);

	//# Write the output data to txt file
	outputWrite(dirpath, img_path1, unpack_output, numImg_to_process, io_layer_info_ptr, 0);

	//# Call Release
	xiRelease(chaihandle); /* Release before exiting application */

	/* Release input/output */
#ifdef __SDSOC
	for(int i = 0; i < input.size(); i++)
		sds_free(input[i]);

	for(int i = 0; i < output.size(); i++)
		sds_free(output[i]);

#else
	for(int i = 0; i < input.size(); i++)
		free(input[i]);

	for(int i = 0; i < output.size(); i++)
		free(output[i]);
#endif

	for(int batch_id = 0; batch_id < unpack_output.size(); batch_id++)
	{
		free(unpack_output[batch_id]);
	}

	fprintf(stderr, "[INFOx] Memory Released\n");

	return 0;
}
