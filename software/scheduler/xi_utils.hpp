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

#ifndef _XI_UTILS_HPP_
#define _XI_UTILS_HPP_

#include "../include/hw_settings.h"

#define XI_UTILS_CONSOLE_TEXT	0
typedef  signed char int8_t;
typedef  short int int16_t;


//# Re-arranges data for HW FC
void fc_inputdatawrite(IO_DATA_TYPE *conv_out, IO_DATA_TYPE *conv_out1, int height, int width, int indepth, bool relu,
		IO_DATA_TYPE *kernel_output, char *output_file_path);

void fc_inputdatawrite_float(IO_DATA_TYPE *conv_out, IO_DATA_TYPE *conv_out1, int height, int width, int indepth, bool relu,
		SW_FC_DATA_TYPE *kernel_output, float sf_in);

void fc_inputdatawrite_float_dynamicfixed(IO_DATA_TYPE *conv_out, IO_DATA_TYPE *conv_out1, int height, int width, int indepth, bool relu,
		SW_FC_DATA_TYPE *kernel_output, int in_fbits);

//# Re-arranges data for HW deconv
void DeconvInReArrange(IO_DATA_TYPE *input1, IO_DATA_TYPE *input2, short *output, int height, int width, int inp_planes);

//# Convolution out to FC in re-arrange
int convOut_to_fcInAsWts_rearrange(int8_t *convOut, int8_t *fc_in_asWts, int isize);

//# FC out to FC in re-arrange
int fcOut_to_fcIn_rearrange(int16_t *fcOut, int8_t *fc_in_asWts, int isize);

//# FC out to SoftMax in
int fcOut_to_smaxIn(int16_t *fcOut, int8_t *smaxIn, int isize);

//int swfcOut_to_smaxIn(float *fcOut, int8_t *smaxIn, int isize, float sf_in);
int swfcOut_to_smaxIn(float *fcOut, float *smaxIn, int isize);

#endif//_XI_UTILS_HPP_
