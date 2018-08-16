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

#ifndef _xTHREAD_ROUTINES_HPP__
#define _xTHREAD_ROUTINES_HPP__
#include "../custom/custom_class.hpp"

#define PTHREAD 1

#include <stdint.h>

#if ENABLE_IMAGE_READ_THREAD
struct _ImgReadThreadArgs
{
	FILE 	* fp;
	xChangeLayer *firstLayer;
	uint8_t * ImageReadDone;
};
typedef _ImgReadThreadArgs ImgReadThreadArgs;

//# Image Read Thread
void * imageReadRoutine(void *arguments);

#endif

struct _CropThreadArgs
{
	xChangeLayer *Layer;
	uint8_t * cropThreadDone;
};
typedef _CropThreadArgs CropThreadArgs;

struct _xCustomThreadArgs
{
	xChangeLayer *Layer;
	uint8_t * xcustomThreadDone;
	Custom Xcustom_obj;
};
typedef _xCustomThreadArgs xCustomThreadArgs;

struct _xPackThreadArgs
{
	xChangeLayer *Layer;
	uint8_t * xpackThreadDone;
};
typedef _xPackThreadArgs xPackThreadArgs;

//# Softmax Thread Routine
void * softmaxRoutine(void *arguments);

//# NMS Thread Routine
void * nmsRoutine(void *arguments);

//# Normalization Thread Routine
void * normRoutine(void *arguments);

//# Permute Thread Routine
void * permuteRoutine(void *arguments);

//# Crop Thread Routine
void * cropRoutine(void *arguments);
void * xpackRoutine(void *arguments);
void * xcustomRoutine(void *arguments);

//# EltwiseAdd Thread Routine
void * eltwaddRoutine(void *arguments);

template<class T>
void PackDataToHardware(float *software_output,int height,int width,int indepth,int fbits,T* conv_out, T *conv_out1,int split);
template<class T>
void UnpackDataToSoftware(T* conv_out, T *conv_out1, int height,int width,int indepth,int fbits,float *kernel_output);
#endif//_xTHREAD_ROUTINES_HPP_
