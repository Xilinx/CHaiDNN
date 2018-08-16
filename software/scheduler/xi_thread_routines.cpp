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
#include "../include/xchange_structs.hpp"
//#include "../imageRead/xiInputImage.hpp"
#include <limits>
#include "xi_thread_routines.hpp"
//# Declarations of all kernel functions
#include "../common/xi_kernels.h"

#define PTHREAD_DEBUG_TEXT	0

#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)

#define ConvertToFP(fVal, iPart, fbits)	((int)((iPart<<fbits) + ((fVal-(float)iPart))*(1<<fbits)))

#if ENABLE_IMAGE_READ_THREAD
//# Image Read Thread
void * imageReadRoutine(void *arguments)
{
#if PTHREAD_DEBUG_TEXT
	std::cout << "\n** Image Read Thread " << std::endl;
#endif

	//# Get arg struct
	ImgReadThreadArgs * imreadArgs = (ImgReadThreadArgs *)arguments;
	//# Call Image Read
	inputImageRead(&(imreadArgs->fp), imreadArgs->firstLayer);
	//# Update done flag
	*(imreadArgs->ImageReadDone) = 1;

#if PTHREAD
	pthread_exit(NULL);
#endif
	return NULL;
}
#endif
#define IO_FLOAT_DATA_TYPE float//char
#define IO_DATA_TYPE1 float //char
//#define IO_DATA_TYPE float //char
#define IO_FLOAT_DATA_TYPE1 float

//# Softmax Thread Routine
void * softmaxRoutine(void *arguments)
{
	//	std::cout << "\n** SoftMax Thread " << std::endl;
	//uint8_t * softMaxdone = (uint8_t*)arguments;
	//*softMaxdone = 1;
	xChangeLayer *layerArgs = (xChangeLayer *)arguments;
	layerArgs->layer_done[0] = 0;

	int *scalar_softmax_args = (int *)layerArgs->params;

#if 0
	IO_DATA_TYPE1 *softmax_in;
	if(scalar_softmax_args[2]!=PERMUTE)
	{
		softmax_in = (float *)layerArgs->in_ptrs[2];
		IO_DATA_TYPE *in_ptr = (IO_DATA_TYPE *)layerArgs->in_ptrs[0];

		for(int i = 0; i < scalar_softmax_args[0]*scalar_softmax_args[3]; i++)
		{
			IO_DATA_TYPE in_val = in_ptr[i];
			float temp_val = ((float)in_val)/(1 << layerArgs->qf_format.ip_fbits);//3);
			//float temp_val = ((float)in_val)/(1 << 1);//3);
			//fprintf(stderr, "%f\n", temp_val );
			softmax_in[i] = temp_val;
		}
	}
	else
	{
		softmax_in = (float *)layerArgs->in_ptrs[0];

	}
#endif

	IO_DATA_TYPE *softmax_in = (IO_DATA_TYPE *)layerArgs->in_ptrs[0];
	IO_DATA_TYPE1 *softmax_in1 = (IO_DATA_TYPE1 *)layerArgs->in_ptrs[2];

	SwSoftmaxForward(softmax_in,
			softmax_in1,
			(IO_FLOAT_DATA_TYPE *)layerArgs->xtra_ptrs[0],
			(IO_FLOAT_DATA_TYPE *)layerArgs->out_ptrs[0],
			(int*)layerArgs->params);


	layerArgs->layer_done[0] = 1;
#if PTHREAD
	pthread_exit(NULL);
#endif
	return NULL;
}

//# NMS Thread Routine
void * nmsRoutine(void *arguments)
{
	//	std::cout << "\n** NMS Thread " << std::endl;
	xChangeLayer *layerArgs = (xChangeLayer *)arguments;
	layerArgs->layer_done[0] = 0;

	//# Call NMS wrapper
	NMSForward(
			(float*)layerArgs->out_ptrs[0],   //nms score
			(int*)layerArgs->out_ptrs[1],
			(int*)layerArgs->xtra_ptrs[0],
			(float*)layerArgs->xtra_ptrs[1],
			(int*)layerArgs->xtra_ptrs[2],
			(float*)layerArgs->in_ptrs[2],   //conf conv perm soft
			(float*)layerArgs->wts_ptrs[0],
			(IO_DATA_TYPE*)layerArgs->in_ptrs[0],   //loc conv perm - 2nd set of input blob
			(float*)layerArgs->wts_ptrs[1],
			(int*)layerArgs->params);

	layerArgs->layer_done[0] = 1;
#if PTHREAD
	pthread_exit(NULL);
#endif
	return NULL;
}

//# Normalization Thread Routine
void * normRoutine(void *arguments)
{
	//	std::cout << "\n** Normalization Thread " << std::endl;

	//uint8_t * normdone = (uint8_t*)arguments;
	xChangeLayer *layerArgs = (xChangeLayer *)arguments;
	layerArgs->layer_done[0] = 0;

	//# Call Normalization wrapper
	NormalizationForward(
			(IO_DATA_TYPE*)layerArgs->in_ptrs[0],
#if !SINGLE_IO_PORT
			(IO_DATA_TYPE*)layerArgs->in_ptrs[1],
#endif
			(IO_DATA_TYPE*)layerArgs->wts_ptrs[0],
			(IO_DATA_TYPE*)layerArgs->out_ptrs[0],
#if !SINGLE_IO_PORT
			(IO_DATA_TYPE*)layerArgs->out_ptrs[1],
#endif
			(int*)layerArgs->xtra_ptrs[0],
			(float)layerArgs->float_params[2],
			(float)layerArgs->float_params[3],
			(int*)layerArgs->params);

	layerArgs->layer_done[0] = 1;

#if PTHREAD
	pthread_exit(NULL);
#endif
	return NULL;
}

//# Permute Thread Routine
void * permuteRoutine(void *arguments)
{
	//	std::cout << "\n** Permute Thread " << std::endl;
	//uint8_t * permutedone = (uint8_t*)arguments;
	//*permutedone = 1;

	xChangeLayer *layerArgs = (xChangeLayer *)arguments;
	layerArgs->layer_done[0] = 0;

	//# Call Permute wrapper
	PermuteForward(
			(IO_DATA_TYPE*)layerArgs->xtra_ptrs[0],
#if !SINGLE_IO_PORT
			(IO_DATA_TYPE*)layerArgs->xtra_ptrs[1],
#endif
			(IO_DATA_TYPE*)layerArgs->in_ptrs[0],
#if !SINGLE_IO_PORT
			(IO_DATA_TYPE*)layerArgs->in_ptrs[1],
#endif
			(IO_DATA_TYPE*)layerArgs->out_ptrs[0],  //output final
			//(float*)layerArgs->out_ptrs[1],  //unpack
			(int*)layerArgs->params);

	layerArgs->layer_done[0] = 1;
#if PTHREAD
	pthread_exit(NULL);
#endif
	return NULL;
}

//# Crop Thread Routine
void * cropRoutine(void *arguments)
{
#if PTHREAD_DEBUG_TEXT
	std::cout << "\[DEBUG] Crop Thread : p0" << std::endl;
#endif

	//# Get arg struct
	CropThreadArgs * cropArgs = (CropThreadArgs*)arguments;

#if PTHREAD_DEBUG_TEXT
	std::cout << "\[DEBUG] Crop Thread : p1" << std::endl;
#endif

	int * params = (int*)cropArgs->Layer->params;

#if PTHREAD_DEBUG_TEXT
	for(int idx = 0; idx < 16; ++idx)
	{
		std::cout << params[idx] << "  ";
	}
	std::cout << std::endl;
#endif

	int *input, *output;
	input  = (int*)cropArgs->Layer->in_ptrs[0];
	output = (int*)cropArgs->Layer->out_ptrs[0];

	int n_width = AlignSize(params[2], 2);

	//for(int batch_id = 0; batch_id < XBATCH_SIZE; ++batch_id)
	{
		//int *crop_in = (int *)(input+(batch_id*params[1]*n_width*1));
		//int *crop_out = (int *)(output+(batch_id*params[4]*params[5]*1));

		int *crop_in = (int *)input;
		int *crop_out = (int *)output;

#if PTHREAD_DEBUG_TEXT
		fprintf(stderr, "cropLayer Start : \n");
#endif
		//# Crop sw-kernel
		CropForward(crop_in, crop_out, params);

#if PTHREAD_DEBUG_TEXT
		fprintf(stderr, "cropLayer End : \n");
#endif
	}

#if PTHREAD_DEBUG_TEXT
	std::cout << "\[DEBUG] Crop Thread : p3" << std::endl;
#endif	

	//# Update done flag
	*(cropArgs->cropThreadDone) = 1;

#if PTHREAD_DEBUG_TEXT
	std::cout << "\[DEBUG] Crop Thread : p4" << std::endl;
#endif

#if PTHREAD
	pthread_exit(NULL);
#endif
	return NULL;
}

void * xcustomRoutine(void *arguments)
{
	static int lc=0;
#if PTHREAD_DEBUG_TEXT
	std::cout << "\[DEBUG] xcustomRoutine : p0" << std::endl;
#endif
	//# Get arg struct
	xCustomThreadArgs * xcustomArgs = (xCustomThreadArgs*)arguments;
	Custom Xcustom_obj = xcustomArgs->Xcustom_obj;

#if PTHREAD_DEBUG_TEXT
	std::cout << "\[DEBUG] xcustomRoutine : p1" << std::endl;
#endif

	int * params = (int*)xcustomArgs->Layer->params;

#if PTHREAD_DEBUG_TEXT
	for(int idx = 0; idx < 16; ++idx)
	{
		std::cout << params[idx] << "  ";
	}
	std::cout << std::endl;
#endif


	std::string custom_layer_type = (std::string)xcustomArgs->Layer->custom_layer_type;

	const xChangeLayer *Layer = xcustomArgs->Layer;


	//	cerr <<  "Just before custom call : " << xcustomArgs->Layer->input_dims.size() << " -- " << xcustomArgs->Layer << endl;
	Xcustom_obj.custom(custom_layer_type, xcustomArgs->Layer);
	float * out_ptr = (float *)xcustomArgs->Layer->out_ptrs[0];
	int out_size=1;
	for(int iter_od=0;iter_od<xcustomArgs->Layer->output_dim.size();iter_od++){
		out_size = out_size*xcustomArgs->Layer->output_dim[iter_od];
	}
	if(xcustomArgs->Layer->custom_reluflag){
		// call software relu subroutine

		for(int iter_os=0;iter_os<out_size*XBATCH_SIZE;iter_os++){

			float tmp = out_ptr[iter_os];
			if(tmp<0){
				out_ptr[iter_os]=0.0f;
			}
		}

	}
#if FILE_WRITE
	FILE *fin_cust=fopen("/tmp/cust_out.txt", "w");

	for(int i=0;i<out_size;i++){
		fprintf(fin_cust, "%f\n", out_ptr[i]);
	}
	fclose(fin_cust);
	cerr <<  "Just after custom call : " << xcustomArgs->Layer->input_dims.size() << " -- " << xcustomArgs->Layer << endl;
#endif

#if PTHREAD_DEBUG_TEXT
	std::cout << "\[DEBUG] Crop Thread : p3" << std::endl;
#endif

	//# Update done flag
	*(xcustomArgs->xcustomThreadDone) = 1;

#if PTHREAD_DEBUG_TEXT
	std::cout << "\[DEBUG] Crop Thread : p4" << std::endl;
#endif

#if PTHREAD
	pthread_exit(NULL);
#endif
	return NULL;
}


void * xpackRoutine(void *arguments)
{
#if PTHREAD_DEBUG_TEXT
	std::cout << "\[DEBUG] Xpack Thread : p0" << std::endl;
#endif

	//# Get arg struct
	xPackThreadArgs * xpackArgs = (xPackThreadArgs*)arguments;

#if PTHREAD_DEBUG_TEXT
	std::cout << "\[DEBUG] Xpack Thread : p1" << std::endl;
#endif



#if PTHREAD_DEBUG_TEXT
	for(int idx = 0; idx < 16; ++idx)
	{
		std::cout << params[idx] << "  ";
	}
	std::cout << std::endl;
#endif

	string quant = xpackArgs->Layer->string_args[0];
	void *input1 = xpackArgs->Layer->in_ptrs[0];
	void *input2 = xpackArgs->Layer->in_ptrs[1];
	void *output1 = xpackArgs->Layer->out_ptrs[0];
	void *output2 = xpackArgs->Layer->out_ptrs[1];
	float *float_params = (float *)&xpackArgs->Layer->float_params[0];
	int *params = (int *)xpackArgs->Layer->params;


	XpackForward(input1, input2, output1, output2, quant, params, float_params);

#if 0
	//int * params = (int*)xpackArgs->Layer->params;
	void *input, *output;
	float *software_output;
	IO_DATA_TYPE * conv_out;
	IO_DATA_TYPE * conv_out1;
	float *kernel_output;

	float th_layer_in = xpackArgs->Layer->float_params[0];
	float sf_in  = xpackArgs->Layer->float_params[1];

	int pack_flag = params[8];
	if(pack_flag==1){
		software_output = (float*)xpackArgs->Layer->in_ptrs[0];
		conv_out  		= (IO_DATA_TYPE*)xpackArgs->Layer->out_ptrs[0];
		conv_out1 		= (IO_DATA_TYPE*)xpackArgs->Layer->out_ptrs[1];
	}else if (pack_flag==0){

		conv_out  		= (IO_DATA_TYPE*)xpackArgs->Layer->in_ptrs[0];
		conv_out1 		= (IO_DATA_TYPE*)xpackArgs->Layer->in_ptrs[1];
		kernel_output 	= (float*)xpackArgs->Layer->out_ptrs[0];
	}else if (pack_flag==2){
		software_output = (float*)xpackArgs->Layer->in_ptrs[0];
		conv_out  		= (IO_DATA_TYPE*)xpackArgs->Layer->out_ptrs[0];
	}
	else{
		if(pack_flag==3){
			conv_out  		= (IO_DATA_TYPE*)xpackArgs->Layer->in_ptrs[0];
			software_output = (float*)xpackArgs->Layer->out_ptrs[0];
		}
	}




	int in_height;
	int in_width;
	int in_indepth;
	int out_height;
	int out_width;
	int out_indepth;
	if(pack_flag!=2){
		out_indepth= params[3];
		out_width= params[4];
		out_height=params[5];
		in_height=params[2];
		in_width= params[1];
		in_indepth= params[0];
	}else{
		out_indepth= params[1];
		in_indepth= params[0];
		in_width=params[2];
		out_width= params[3];
	}

	int split =1;
#if 0
	//# Crop sw-kernel
	if(pack_flag==1){

		//PackDataToHardware<IO_DATA_TYPE>(software_output,out_height, out_width, out_indepth,fbits,conv_out, conv_out1,split);
		PackDataToHardware<IO_DATA_TYPE>(software_output,out_height,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,conv_out,conv_out1,split);

	}else if(pack_flag==0){
		//UnpackDataToSoftware<IO_DATA_TYPE>(conv_out,conv_out1,out_height, out_width, out_indepth,fbits,kernel_output);
		UnpackDataToSoftware<IO_DATA_TYPE>(conv_out, conv_out1,  out_height,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,kernel_output);
	}else if(pack_flag==2){
		packDataToSoftmax<IO_DATA_TYPE>(software_output,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,conv_out);
		//packDataToSoftmax<IO_DATA_TYPE>(software_output,out_width,out_indepth,fbits,conv_out);
	}
	else{
		if(pack_flag==3){
			unPackPermutetocustom<IO_DATA_TYPE>(conv_out,out_height,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,software_output);
		}
	}
#endif

#endif

#if PTHREAD_DEBUG_TEXT
	std::cout << "\[DEBUG] Xpack Thread : p3" << std::endl;
#endif

	//# Update done flag
	*(xpackArgs->xpackThreadDone) = 1;

#if PTHREAD_DEBUG_TEXT
	std::cout << "\[DEBUG] Xpack Thread : p4" << std::endl;
#endif

#if PTHREAD
	pthread_exit(NULL);
#endif
	return NULL;
}

//# EltwiseAdd Thread Routine
void * eltwaddRoutine(void *arguments)
{
	//	std::cout << "\n** EltwiseAdd Thread " << std::endl;
	xChangeLayer *layerArgs = (xChangeLayer *)arguments;
	layerArgs->layer_done[0] = 0;

	//# Call Eltwiseadd wrapper
	EltwiseaddForward(
			(void*)layerArgs->in_ptrs[0],
			(void*)layerArgs->in_ptrs[1],
			(void*)layerArgs->in_ptrs[2],
			(void*)layerArgs->in_ptrs[3],
			(void*)layerArgs->out_ptrs[0],
			(void*)layerArgs->out_ptrs[1],
			(int*)layerArgs->params
			);

	layerArgs->layer_done[0] = 1;
#if PTHREAD
	pthread_exit(NULL);
#endif
	return NULL;
}
