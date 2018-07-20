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

#ifndef _XI_FORMAT_CONVERTER_HPP__
#define _XI_FORMAT_CONVERTER_HPP__
#include <limits>
#include "../include/hw_settings.h"
#include "../include/xchange_structs.hpp"

using namespace std;
#include <stdint.h>
void * xpackRoutine(void *arguments);
struct _xPackThreadArgs
{
	xChangeLayer *Layer;
	uint8_t * xpackThreadDone;
};
typedef _xPackThreadArgs xPackThreadArgs;
template<class T>
void PackDataToHardware(float *software_output,int height,int width,int indepth,int fbits,T* conv_out, T *conv_out1,int split);
template<class T>
void UnpackDataToSoftware(T* conv_out, T *conv_out1, int height,int width,int indepth,int fbits,float *kernel_output);
template<class T>
void UnpackDataToSoftware(T* conv_out, T *conv_out1, int height,int width,int indepth,int fbits,float &sf,float &th,string &quant,float *kernel_output);
template<class T> 
void PackDataToHardware(float *software_output,int height,int width,int indepth,int fbits,float &sf,float &th,string &quant,T* conv_out, T *conv_out1,int split);
template<class T>
void packDataToSoftmax(float *software_output,int width,int indepth,int fbits,float &sf,float &th,string &quant,T* conv_out);
template<class T>
void unPackPermutetocustom(T* conv_out,int height,int width,int indepth,int fbits,float &sf,float &th,string &quant,float *software_output);
template<class T>
void unpackDataToSoftmax(T* conv_out,int width,int indepth,int fbits,float &sf,float &th,string &quant,float *software_output);
#endif//_xTHREAD_ROUTINES_HPP_
