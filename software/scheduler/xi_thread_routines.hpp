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

#ifndef _xTHREAD_ROUTINES_HPP_
#define _xTHREAD_ROUTINES_HPP_

#define PTHREAD 1

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

#endif//_xTHREAD_ROUTINES_HPP_
