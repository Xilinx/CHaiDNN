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

#ifndef _XI_KERNELS_H_
#define _XI_KERNELS_H_

#define CHAR_TYPE     char
#define SHORT_TYPE    short
#define INT_TYPE      int

void ConvolutionForward(
		CHAR_TYPE *weights1, CHAR_TYPE *weights2,
#if (KER_PROC==16 || (PORT_BITWIDTH_64BIT==1 && KER_PROC==8))
		CHAR_TYPE *weights3, CHAR_TYPE *weights4,
#endif
		CHAR_TYPE *output1,
#if !SINGLE_IO_PORT
		CHAR_TYPE *output2,
#endif
		CHAR_TYPE *input_other1,
#if !SINGLE_IO_PORT
		CHAR_TYPE *input_other2,
#endif
		CHAR_TYPE *input_1st, SHORT_TYPE *bias,
#if !DISABLE_BN
		CHAR_TYPE *input_norm2, CHAR_TYPE *input_norm3,
#endif
		CHAR_TYPE *istg_out1,
#if !SINGLE_IO_PORT
		CHAR_TYPE *istg_out2,
#endif
		int *scalar_conv_args );



void PoolForward(
		            SHORT_TYPE *pool_in, SHORT_TYPE *pool_out,
		            SHORT_TYPE *pool_in1, SHORT_TYPE *pool_out1,
					CHAR_TYPE *wts,
				    int *scalar_pool_args
				);

void FcForward(
		CHAR_TYPE *A1, CHAR_TYPE *A2,
		SHORT_TYPE *in1, SHORT_TYPE *in2, SHORT_TYPE *in3,
		SHORT_TYPE *y_in, SHORT_TYPE *y_out,
		int *scalar_fc_args
		);


void SwFcForward(IO_DATA_TYPE *inp1, IO_DATA_TYPE *inp2, SW_FC_DATA_TYPE *inp3, SW_FC_DATA_TYPE *fc_wgts, SW_FC_DATA_TYPE *fc_bias, SW_FC_DATA_TYPE *fc_out,
		         int *scalar_fc_args);

void SwSoftmaxForward(IO_DATA_TYPE *softmax_in, IO_DATA_TYPE1 *softmax_in1, IO_DATA_TYPE1 *sumBuffer, IO_DATA_TYPE1 *softmax_out, int *scalar_softmax_args);

/*
void DeconvForward(
					short* deconvIN, short* deconvWT,
					short* deconvBias, unsigned long long int* deconvIDout,
					int *scalar_deconv_args
					);
*/

void DeconvForward(
		short* deconvIN1, short* deconvIN2, short* deconvIN3,
		short* deconvWT,
		short* deconvBias, int* deconvIDout,
		int *scalar_deconv_args
);


void NormalizationForward(IO_DATA_TYPE *input,
#if !SINGLE_IO_PORT
		IO_DATA_TYPE *input1,
#endif
		IO_DATA_TYPE *gamma, IO_DATA_TYPE *output,
#if !SINGLE_IO_PORT
		IO_DATA_TYPE *output1,
#endif
		int *sumBuffer, float sf_in, float sf_out, int *scalar_norm_args);

void PermuteForward(IO_DATA_TYPE *input1,
#if !SINGLE_IO_PORT
		IO_DATA_TYPE *input2,
#endif
		IO_DATA_TYPE *sds_input1,
#if !SINGLE_IO_PORT
		IO_DATA_TYPE *sds_input2,
#endif
		IO_DATA_TYPE *output, int *scalar_permute_args);

void NMSForward(float *nms_score, int *nms_id, int *nms_label, float *nms_box, int* nms_finalboxcount,
		        float* conf, float* pbox, IO_DATA_TYPE* loc, float* var,
				int *scalar_nms_args );

void CropForward(int *input, int *output, int *scalars);

void XpackForward(void *input1, void *input2, void *output1, void *output2, string quant, int *params, float *float_params);


#endif//_XI_KERNELS_H_
