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

#ifndef _XI_DNN_LIB_COMMON_H
#define _XI_DNN_LIB_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdio.h"
#include "stdlib.h"


#define GMEM_INPUTTYPE       char
#define GMEM_INTYPE_OTHER    short
#define GMEM_WEIGHTTYPE      char
#define GMEM_OUTTYPE         short
#define GMEM_MAXPOOLTYPE     short
#define GMEM_BIASTYPE        short

#define GMEM_DECONVOUTTYPE   short
#define GMEM_DECONVINTYPE    short

#define SOFT_GMEM_TYPE       unsigned int

#define GMEM_FC_INPUTTYPE    short//unsigned long long int
#define GMEM_FC_OUTTYPE      short
#define GMEM_FC_WEIGHTTYPE   char

#if DECONV_FLOAT
#define GMEM_DECONVINPTYPE   float
#else
#define GMEM_DECONVINPTYPE   short
#endif


#define GMEM_RGBTYPE       unsigned long long int
#define GMEM_DECONVTYPE    unsigned long long int
#define GMEM_DISPLAYTYPE   unsigned long long int
#define CONV_WT_TYPE       char

#define __MOST_NEG_VAL 4286578687
#define SOFTMAX_SIZE    5

typedef struct {
	unsigned char *input;
	short *input_mean;
	char *in_img;

	CONV_WT_TYPE **wgtsRearr;
	CONV_WT_TYPE **wgtsRearr1;
	short **convBias;
	short **inout_buf;
	short **inout_buf1;
	int *scalar_conv_args;

	short **pool_out;
	short **pool_out1;
	short **pool_in;
	short **pool_in1;
	int *scalar_pool_args;

	CONV_WT_TYPE **ince_wgts;
	CONV_WT_TYPE **ince_wgts1;
	short **ince_bias;
	short **ince_tensor0;
	short **ince_tensor1;
	short **ince_conc_out0;
	short **ince_conc_out1;
	int *scalar_ince_args;

	CONV_WT_TYPE **fcWgts1;
	CONV_WT_TYPE **fcWgts2;
	short **fcBias;
	short **fc_out;
	int *scalar_fc_args;

	float **softmax_inp;
	float *softmax_data_out;
	int   *softmax_data_out_index;
	int   *scalar_softmax_args;

	short *skipLayer16xInp;
	short *deconv_weights;
	short *deconv_bias;
	int **deconv_out;
	int *scalar_deconv_args;

	unsigned short *display_output;
	int *scalar_display_args;

	int **scalar_host_args;
	short *deconv_host_inp;
} dnnlib_config;

void dnnModel(dnnlib_config dnn_config);

#ifdef __cplusplus
}
#endif

#endif /* _XI_DNN_LIB_COMMON_H */
