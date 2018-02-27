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

#ifndef _XI_KERNELS_HPP_
#define _XI_KERNELS_HPP_

#include "xi_kernels.h"

#include <ap_int.h>

#define GMEM_INPUTTYPE       unsigned long long int
#define GMEM_INTYPE_OTHER    ap_uint<128>
#define GMEM_WEIGHTTYPE      ap_uint<128>
#define GMEM_OUTTYPE         ap_uint<128>
#define GMEM_MAXPOOLTYPE     ap_uint<128>
#define GMEM_BIASTYPE        unsigned long long int

int ConvolutionWrapper(GMEM_WEIGHTTYPE *weights1,GMEM_WEIGHTTYPE *weight2,
		               GMEM_OUTTYPE *output1, GMEM_OUTTYPE *output2,
					   GMEM_INTYPE_OTHER *input_other1, GMEM_INTYPE_OTHER *input_other2,
					   GMEM_INPUTTYPE *input_1st, GMEM_BIASTYPE *bias, 
					   GMEM_INPUTTYPE *input_norm2, GMEM_INPUTTYPE *input_norm3, 
					   GMEM_INPUTTYPE *istg_out1, GMEM_INPUTTYPE *istg_out2,
                                           int *scalar_conv_args);

void ConvolutionForward(
		CHAR_TYPE *weights1, CHAR_TYPE *weights2,
		SHORT_TYPE *output1, SHORT_TYPE *output2,
		SHORT_TYPE *input_other1, SHORT_TYPE *input_other2,
		CHAR_TYPE *input_1st, SHORT_TYPE *bias, CHAR_TYPE *input_norm2, CHAR_TYPE *input_norm3,
		CHAR_TYPE *istg_out1, CHAR_TYPE *istg_out2,
		int *scalar_conv_args )
{

	ConvolutionWrapper((GMEM_WEIGHTTYPE *)weights1,(GMEM_WEIGHTTYPE *)weights2,
			           (GMEM_OUTTYPE *)output1, (GMEM_OUTTYPE *)output2,
			           (GMEM_INTYPE_OTHER *)input_other1, (GMEM_INTYPE_OTHER *)input_other2,
				   (GMEM_INPUTTYPE *)input_1st, (GMEM_BIASTYPE *)bias,
				   (GMEM_INPUTTYPE *)input_norm2, (GMEM_INPUTTYPE *)input_norm3, 
				   (GMEM_INPUTTYPE *)istg_out1, (GMEM_INPUTTYPE *)istg_out2,
                                   scalar_conv_args);
}

int PoolWrapper(GMEM_MAXPOOLTYPE *poolin, GMEM_MAXPOOLTYPE *poolout, GMEM_MAXPOOLTYPE *poolin1, GMEM_MAXPOOLTYPE *poolout1, int *scalar_pool_args);

void PoolForward(
		            SHORT_TYPE *pool_in, SHORT_TYPE *pool_out,
		            SHORT_TYPE *pool_in1, SHORT_TYPE *pool_out1,
				    int *scalar_pool_args
				)
{

	PoolWrapper((GMEM_MAXPOOLTYPE *)pool_in, (GMEM_MAXPOOLTYPE *)pool_in1,
			    (GMEM_MAXPOOLTYPE *)pool_out, (GMEM_MAXPOOLTYPE *)pool_out1,
				scalar_pool_args);

}

int SgemvWrapper(GMEM_INTYPE_OTHER *A1, GMEM_INTYPE_OTHER *A2,
		         GMEM_INTYPE_OTHER *x,
				 GMEM_INTYPE_OTHER *y_in, GMEM_INTYPE_OTHER *y_out,
				 int *scalar_gemv_args );
								
void FcForward(
		        CHAR_TYPE *A1,
		        CHAR_TYPE *A2,
				SHORT_TYPE *x,
				SHORT_TYPE *y_in,
				SHORT_TYPE *y_out,
				INT_TYPE *scalar_fc_args)
{

    SgemvWrapper((GMEM_INTYPE_OTHER*)A1, (GMEM_INTYPE_OTHER*)A2, (GMEM_INTYPE_OTHER*)x,
    		     (GMEM_INTYPE_OTHER*)y_in, (GMEM_INTYPE_OTHER*)y_out, scalar_fc_args);

}

void SwSoftmaxWrapper(float *softmax_in, float *sumBuffer, float *softmax_out, int *scalar_softmax_args);

void SwSoftmaxForward(float *softmax_in, float *sumBuffer, float *softmax_out, int *scalar_softmax_args)
{
	SwSoftmaxWrapper(softmax_in, sumBuffer, softmax_out, scalar_softmax_args);
}

void DeconvWrapper(short* deconvIN, short* deconvWT, short* deconvBias, unsigned long long int* deconvIDout, int *scalar_deconv_args);

void DeconvForward(
					short* deconvIN, short* deconvWT,
					short* deconvBias, unsigned long long int* deconvIDout,
					int *scalar_deconv_args
					)
{
	DeconvWrapper(deconvIN, deconvWT, deconvBias, deconvIDout, scalar_deconv_args);
}

void NormalizationWrapper(short *inArray, short *inArray1, short* gamma,
		                  short *outArray, short *outArray1, int *sumBuffer, int *scalar_norm_args);

void NormalizationForward(short *input, short *input1, short *gamma, short *output, short *output1, int *sumBuffer, int *scalar_norm_args)
{

	NormalizationWrapper(input, input1, gamma, output, output1, sumBuffer, scalar_norm_args);

}

void PermuteWrapper(short *input1, short *input2, short *sds_input1, short *sds_input2, float *output, float *output_unpack, int *scalar_permute_args);

void PermuteForward(short *input1, short *input2, short *sds_input1, short *sds_input2, float *output, float *output_unpack, int *scalar_permute_args)
{
	PermuteWrapper(input1, input2, sds_input1, sds_input2, output, output_unpack, scalar_permute_args);
}

void NMSWrapper(int* nms_finalcount, int *nms_id, int *nms_label, float *nms_box, float *nms_score, float* conf, float* pbox, float* loc, float* var, float score_threshold, float nms_overlap, int *scalar_nms_args );


void NMSForward(int* nms_finalboxcount, int *nms_id, int *nms_label, float *nms_box, float *nms_score,
		        float* conf, float* pbox, float* loc, float* var, float score_threshold, float nms_overlap, int *scalar_nms_args )
{
	NMSWrapper(nms_finalboxcount, nms_id, nms_label, nms_box, nms_score, conf, pbox, loc, var, score_threshold, nms_overlap, scalar_nms_args);
}

#endif//_XI_KERNELS_HPP_
