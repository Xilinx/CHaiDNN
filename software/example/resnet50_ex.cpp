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

//# Structure to be passed to exec thread routine
typedef struct execStruct_ {
	void *chaihandle;
	std::vector<void* > input;
	std::vector<void* > output;
} execStruct;

//# Thread routine wrapper for exec()
void* execRoutine(void* args_) {
	execStruct* args = (execStruct*) args_;
	xiExec(args->chaihandle, args->input, args->output);
	return NULL;
}

int main(int argc, char **argv)
{
	if(argc < 5) {
		std::cout << "Usage : ./resnet_ex.elf <quant_mode> <bit_width> <img1_path> <img2_path>" << std::endl;
		std::cout << "quant_mode : Xilinx" << std::endl;		
		std::cout << "bit_width : 8 / 6" << std::endl;
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
	if(quant_mode_arg == "Xilinx" && bit_width_arg == 6) {
		dirpath    = "models/ResNet-50-Legacy_NoBN/6Bit";
		caffemodel = "ResNet-50_NoBN_6Bit_CHaiDNN.caffemodel";
		prototxt   = "ResNet-50_NoBN_6Bit_deploy_CHaiDNN.prototxt";
        inp_mode   = 0;
	}
	// Xilinx quantmode 8 bit
	else if(quant_mode_arg == "Xilinx" && bit_width_arg == 8) {
		dirpath    = "models/ResNet-50-Legacy_NoBN/8Bit";
		caffemodel = "ResNet-50_NoBN_8Bit_CHaiDNN.caffemodel";
		prototxt   = "ResNet-50_NoBN_8Bit_deploy_CHaiDNN.prototxt";		
        inp_mode   = 0;
	}
	else {
		std::cout << "quant_mode argument should be Xilinx" << std::endl;
		std::cout << "Usage : ./resnet_ex.elf <quant_mode> <bit_width> <img1_path> <img2_path>" << std::endl;
		std::cout << "quant_mode : Xilinx" << std::endl;		
		std::cout << "bit_width : 8 / 6" << std::endl;
		std::cout << "img1_path : Path to the first image" << std::endl;
		std::cout << "img2_path : Path to the second image" << std::endl;
		return -1;	
	}

	std :: cout << "[INFOx] Network Path : " << dirpath << std :: endl;

	//# start/end layer in the graph1
	string start_layer_graph1 = "";
	string end_layer_graph1   = "pool5";

	//# start/end layer in the graph2
	string start_layer_graph2 = "fc1000";
	string end_layer_graph2   = "";

	//# Flag indicate layer1 or not
	//# 1 - if image data is the for first layer input otherwise 0
	bool is_first_layer_g1 = 1;
	bool is_first_layer_g2 = 0;

	//# Number of images to process
	//# Supported batch size is 2
	int numImg_to_process = 2;

	//# Struct which holds the input/output layer info
	io_layer_info io_layer_info_ptr1;
	io_layer_info io_layer_info_ptr2;

	//####### Init call for graph1
	void *chaihandle1 = xiInit(dirpath, prototxt, caffemodel, &io_layer_info_ptr1,
			numImg_to_process, is_first_layer_g1, start_layer_graph1, end_layer_graph1);


	//####### Init call for graph2
	void *chaihandle2 = xiInit(dirpath, prototxt, caffemodel, &io_layer_info_ptr2,
			numImg_to_process, is_first_layer_g2, start_layer_graph2, end_layer_graph2);

			
	//# Mean and Variance values
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
			inp_mode, mean_ptr, var_ptr, numImg_to_process, io_layer_info_ptr1);
			
	if(status == -1)
	{
		fprintf(stderr,"[ERROR] Image Read fail\n");
		return -1;
	}
	else
	{
		fprintf(stderr,"[INFOx] Image Read Done\n");
	}

	int size = io_layer_info_ptr1.inlayer_sizebytes;

	//# Create input/output Buffers
	vector<void *> input;

	void *ptr;
	for(int i = 0; i < io_layer_info_ptr1.num_in_bufs; i++)
	{
		if(io_layer_info_ptr1.inlayer_exectype.compare("hardware") == 0)
		{
#ifdef __SDSOC
			ptr = sds_alloc_non_cacheable(size);
#else
			ptr = malloc(size);
#endif
		}
		else
			ptr = malloc(size);

		if(ptr == NULL)
		{
			fprintf(stderr, "Failed to create input memory\n");
			return -1;
		}
		input.push_back(ptr);
	}

	xiInputRead(normalizeInput, input, numImg_to_process, io_layer_info_ptr1);
	fprintf(stderr,"[INFOx] Input Read Done\n");

	// Allocate output Buffers for pingpong for graph1
	vector<void *> tmp_out1;
	vector<void *> tmp_out2;

	//# Memory size required for output buffers
	int graph1_out_size = io_layer_info_ptr1.outlayer_sizebytes;

	void *ptr1, *ptr2;
	for(int i = 0; i < io_layer_info_ptr1.num_out_bufs; i++)
	{
		if(io_layer_info_ptr1.outlayer_exectype.compare("hardware") == 0)
		{
#ifdef __SDSOC
			ptr1 = sds_alloc_non_cacheable(size);
			ptr2 = sds_alloc_non_cacheable(size);
#else
			ptr1 = malloc(size);
			ptr2 = malloc(size);
#endif
		}
		else
		{
			ptr1 = malloc(size);
			ptr2 = malloc(size);
		}

		if((ptr1 == NULL) || (ptr2 == NULL))
		{
			fprintf(stderr, "Failed to create input memory\n");
			return -1;
		}
		tmp_out1.push_back(ptr1);
		tmp_out2.push_back(ptr2);
	}

	// Allocate output Buffers for pingpong for graph2
	vector<void *> output1;
	vector<void *> output2;

	//# Memory size required for output buffers
	int graph2_out_size = io_layer_info_ptr2.outlayer_sizebytes;

	for(int i = 0; i < io_layer_info_ptr2.num_out_bufs; i++)
	{
		if(io_layer_info_ptr2.outlayer_exectype.compare("hardware") == 0)
		{
#ifdef __SDSOC
			ptr1 = sds_alloc_non_cacheable(size);
			ptr2 = sds_alloc_non_cacheable(size);
#else
			ptr1 = malloc(size);
			ptr2 = malloc(size);
#endif
		}
		else
		{
			ptr1 = malloc(size);
			ptr2 = malloc(size);
		}

		if((ptr1 == NULL) || (ptr2 == NULL))
		{
			fprintf(stderr, "Failed to create input memory\n");
			return -1;
		}
		output1.push_back(ptr1);
		output2.push_back(ptr2);
	}

	//# Setup the arguments to pass to thread routines.
	// image 1 flow
	execStruct queue1_args_ping = {chaihandle1, input, tmp_out1};
	execStruct queue2_args_ping = {chaihandle2, tmp_out1, output1};
	// image 2 flow
	execStruct queue1_args_pong = {chaihandle1, input, tmp_out2};
	execStruct queue2_args_pong = {chaihandle2, tmp_out2, output2};

	execStruct queue1_Args[2] = {queue1_args_ping, queue1_args_pong};
	execStruct queue2_Args[2] = {queue2_args_ping, queue2_args_pong};

	
	//# EXEC STARTS
	int loop_iter = 100;
	int pingpong = 0;
	pthread_t queue2_thread;

#ifdef __SDSOC
	TIME_STAMP_INIT
#endif

	///////////////////////////////////////////////////////////////////////////

	//# Run the loop for loop_iter times, but average time is computed over last 'loop_iter' run
	// to avoid the effect of first warm-up run
	execRoutine((void*)(&queue1_Args[pingpong]));
	for(int i = 0; i < loop_iter; i++)
	{
		pthread_create(&queue2_thread, NULL, execRoutine, (void *)(&queue2_Args[pingpong]));
		pingpong = 1 - pingpong;
		execRoutine((void*)(&queue1_Args[pingpong]));
		pthread_join(queue2_thread, NULL);
	}
	execRoutine((void*)(&queue2_Args[pingpong]));


#ifdef __SDSOC
	TIME_STAMP
#endif

#ifdef __SDSOC
	//# Total time for the API in Images/Second
	double tot_time = (((double)(clock_end-clock_start)/(double)frequency*1000));
	fprintf(stderr, "\n[PERFM] Performance with Batching : %lf Images/second\n", ((double)(1000)/tot_time)*XBATCH_SIZE*(loop_iter));
	fprintf(stderr, "\n\n");
#endif

	int unpack_out_size = io_layer_info_ptr2.outlayer_sizebytes;

	//# Create memory for unpack output data
	vector<void *> unpack_output1;
	vector<void *> unpack_output2;

	for(int batch_id = 0; batch_id < numImg_to_process; batch_id++)
	{
		void *ptr1 = malloc(unpack_out_size);
		void *ptr2 = malloc(unpack_out_size);

		unpack_output1.push_back(ptr1);
		unpack_output2.push_back(ptr2);
	}

	//# Loading required params for unpack function
	kernel_type_e out_kerType = io_layer_info_ptr2.out_kerType;
	int out_layer_size = io_layer_info_ptr2.out_size;

	cout << "[INFOx] Unapack output data" << endl;
	//# unpacks the output data for ping and pong buffers
	xiUnpackOutput(output1, unpack_output1, out_kerType, out_layer_size, numImg_to_process);
	xiUnpackOutput(output2, unpack_output2, out_kerType, out_layer_size, numImg_to_process);

	//# Write the output data to txt file
	outputWrite(dirpath, img_path1, unpack_output1, numImg_to_process, io_layer_info_ptr2, 0);
	outputWrite(dirpath, img_path1, unpack_output2, numImg_to_process, io_layer_info_ptr2, 1);

	//# Call Release
	xiRelease(chaihandle1); /* Release before exiting application */
	xiRelease(chaihandle2); /* Release before exiting application */

	/* Release input/output */
#ifdef __SDSOC
	for(int i = 0; i < input.size(); i++)
		sds_free(input[i]);
	for(int i = 0; i < tmp_out1.size(); i++)
		sds_free(tmp_out1[i]);
	for(int i = 0; i < tmp_out2.size(); i++)
		sds_free(tmp_out2[i]);
	for(int i = 0; i < output1.size(); i++)
		sds_free(output1[i]);
	for(int i = 0; i < output2.size(); i++)
		sds_free(output2[i]);
#else
	for(int i = 0; i < input.size(); i++)
		free(input[i]);
	for(int i = 0; i < tmp_out1.size(); i++)
		free(tmp_out1[i]);
	for(int i = 0; i < tmp_out2.size(); i++)
		free(tmp_out2[i]);
	for(int i = 0; i < output1.size(); i++)
		free(output1[i]);
	for(int i = 0; i < output2.size(); i++)
		free(output2[i]);
#endif

	for(int batch_id = 0; batch_id < unpack_output1.size(); batch_id++)
	{
		free(unpack_output1[batch_id]);
	}

	for(int batch_id = 0; batch_id < unpack_output2.size(); batch_id++)
	{
		free(unpack_output2[batch_id]);
	}

	fprintf(stderr, "[INFOx] Memory Released\n");

	return 0;
}
