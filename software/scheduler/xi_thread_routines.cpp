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
#include "../imageread/xi_input_image.hpp"

#include "xi_thread_routines.hpp"
//# Declarations of all kernel functions
#include "../common/xi_kernels.h"

#define PTHREAD_DEBUG_TEXT	0


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

//# Softmax Thread Routine
void * softmaxRoutine(void *arguments)
{
//	std::cout << "\n** SoftMax Thread " << std::endl;
	//uint8_t * softMaxdone = (uint8_t*)arguments;
	//*softMaxdone = 1;
	xChangeLayer *layerArgs = (xChangeLayer *)arguments;
	layerArgs->layer_done[0] = 0;

	int *scalar_softmax_args = (int *)layerArgs->params;


	float *softmax_in;
	if(scalar_softmax_args[2]!=PERMUTE)
	{
		softmax_in = (float *)layerArgs->in_ptrs[2];
		short *in_ptr = (short *)layerArgs->in_ptrs[0];

		for(int i = 0; i < scalar_softmax_args[0]; i++)
		{
			short in_val = in_ptr[i];
			float temp_val = ((float)in_val)/(1 << layerArgs->qf_format.ip_fbits);//3);
			//fprintf(stderr, "%f\n", temp_val );
			softmax_in[i] = temp_val;
		}
	}
	else
	{
		softmax_in = (float *)layerArgs->in_ptrs[0];
	}

	SwSoftmaxForward(softmax_in,
			(float*)layerArgs->xtra_ptrs[0],
			(float *)layerArgs->out_ptrs[0],
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
			(int*)layerArgs->out_ptrs[0],
			(int*)layerArgs->out_ptrs[1],
			(int*)layerArgs->xtra_ptrs[0],
			(float*)layerArgs->xtra_ptrs[1],
			(float*)layerArgs->xtra_ptrs[2],
			(float*)layerArgs->in_ptrs[2],   //conf conv perm soft
			(float*)layerArgs->wts_ptrs[0],
			(float*)layerArgs->in_ptrs[0],   //loc conv perm - 2nd set of input blob
			(float*)layerArgs->wts_ptrs[1],
			(float)layerArgs->float_params[0],
			(float)layerArgs->float_params[1],
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
			(short*)layerArgs->in_ptrs[0],
			(short*)layerArgs->in_ptrs[1],
			(short*)layerArgs->wts_ptrs[0],
			(short*)layerArgs->out_ptrs[0],
			(short*)layerArgs->out_ptrs[1],
			(int*)layerArgs->xtra_ptrs[0],
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
			(short*)layerArgs->xtra_ptrs[0],
			(short*)layerArgs->xtra_ptrs[1],
			(short*)layerArgs->in_ptrs[0],
			(short*)layerArgs->in_ptrs[1],
			(float*)layerArgs->out_ptrs[0],  //output final
			(float*)layerArgs->out_ptrs[1],  //unpack
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

	//# Crop sw-kernel
	cropLayer(input, output, params[1], params[2], params[4],  params[5], params[7], 1);

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
