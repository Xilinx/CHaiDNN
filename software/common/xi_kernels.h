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
		SHORT_TYPE *output1, SHORT_TYPE *output2,
		SHORT_TYPE *input_other1, SHORT_TYPE *input_other2,
		CHAR_TYPE *input_1st, SHORT_TYPE *bias, CHAR_TYPE *input_norm2, CHAR_TYPE *input_norm3,
		CHAR_TYPE *istg_out1, CHAR_TYPE *istg_out2,
		int *scalar_conv_args );



void PoolForward(
		            SHORT_TYPE *pool_in, SHORT_TYPE *pool_out,
		            SHORT_TYPE *pool_in1, SHORT_TYPE *pool_out1,
				    int *scalar_pool_args
				);


							
void FcForward(
		        CHAR_TYPE *A1,
		        CHAR_TYPE *A2,
				SHORT_TYPE *x,
				SHORT_TYPE *y_in,
				SHORT_TYPE *y_out,
				INT_TYPE *scalar_fc_args);

void SwSoftmaxForward(float *softmax_in, float *sumBuffer, float *softmax_out, int *scalar_softmax_args);


void DeconvForward(
					short* deconvIN, short* deconvWT,
					short* deconvBias, unsigned long long int* deconvIDout,
					int *scalar_deconv_args
					);



void NormalizationForward(short *input, short *input1, short *gamma, short *output, short *output1, int *sumBuffer, int *scalar_norm_args);




void PermuteForward(short *input1, short *input2, short *sds_input1, short *sds_input2, float *output, float *output_unpack, int *scalar_permute_args);



void NMSForward(int* nms_finalboxcount, int *nms_id, int *nms_label, float *nms_box, float *nms_score,
		        float* conf, float* pbox, float* loc, float* var, float score_threshold, float nms_overlap, int *scalar_nms_args );

				
void cropLayer(int* inarray, int* outarray, int in_height, int in_width, int out_height, int out_width, int offset, int channels);

#endif//_XI_KERNELS_H_
