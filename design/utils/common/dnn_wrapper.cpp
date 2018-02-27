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

#include "ap_int.h"

#include "../../src/conv/example/xi_conv_config.h"
#include "../../include/xi_dnnlib_common.h"

#define KER_GMEM_INPUTTYPE		 unsigned long long int
#define KER_GMEM_INTYPE_OTHER    ap_uint<128>
#define KER_GMEM_WEIGHTTYPE      ap_uint<128>
#define KER_GMEM_BIASTYPE        unsigned long long int
#define KER_GMEM_OUTTYPE         ap_uint<128>
#define KER_GMEM_DECONVOUTTYPE   unsigned long long int
#define KER_SOFT_GMEM_TYPE       unsigned int
#define KER_GMEM_FC_INPUTTYPE	 ap_uint<128>
#define KER_GMEM_FC_OUTTYPE      ap_uint<128>

#define KER_GMEM_DECONVOUTTYPE   unsigned long long int

#if DECONV_FLOAT
#define KER_GMEM_DECONVINPTYPE   float
#else
#define KER_GMEM_DECONVINPTYPE   short
#endif

#define KER_GMEM_MAXPOOLTYPE   ap_uint<128>

#define GMEM_RGBTYPE       unsigned long long int
#define GMEM_DECONVTYPE    unsigned long long int
#define GMEM_DISPLAYTYPE   unsigned long long int


void XiConvolutionTop(KER_GMEM_WEIGHTTYPE *weights1,KER_GMEM_WEIGHTTYPE *weights2, KER_GMEM_OUTTYPE *output1, KER_GMEM_OUTTYPE *output2, KER_GMEM_INTYPE_OTHER *input_other1, KER_GMEM_INTYPE_OTHER *input_other2,
		KER_GMEM_INPUTTYPE *input_1st, KER_GMEM_BIASTYPE *bias,KER_GMEM_INPUTTYPE *inp_norm2,KER_GMEM_INPUTTYPE *inp_norm3,KER_GMEM_INPUTTYPE *istg_out1,KER_GMEM_INPUTTYPE *istg_out2, int *scalar_conv_args);
void PoolTop(KER_GMEM_MAXPOOLTYPE *in1,KER_GMEM_MAXPOOLTYPE *in2, KER_GMEM_MAXPOOLTYPE *out1,KER_GMEM_MAXPOOLTYPE *out2, int *scalar_pool_args);
void xiSgemvTop(KER_GMEM_FC_INPUTTYPE *A1, KER_GMEM_FC_INPUTTYPE *A2, KER_GMEM_FC_INPUTTYPE *x, KER_GMEM_FC_OUTTYPE *y_in, KER_GMEM_FC_OUTTYPE *y_out, int *scalar_gemv_args);
void XiDeconvTop(KER_GMEM_DECONVINPTYPE* deconvIN, KER_GMEM_DECONVINPTYPE* deconvWT, KER_GMEM_DECONVINPTYPE* deconvBias, KER_GMEM_DECONVOUTTYPE* deconvIDout, int *scalar_deconv_args);


int ConvolutionWrapper(KER_GMEM_WEIGHTTYPE *weights1, KER_GMEM_WEIGHTTYPE *weights2, KER_GMEM_OUTTYPE *output1, KER_GMEM_OUTTYPE *output2, KER_GMEM_INTYPE_OTHER *input_other1, KER_GMEM_INTYPE_OTHER *input_other2, KER_GMEM_INPUTTYPE *input_1st, KER_GMEM_BIASTYPE *bias,KER_GMEM_INPUTTYPE *inp_norm2,KER_GMEM_INPUTTYPE *inp_norm3,KER_GMEM_INPUTTYPE *istg_out1,KER_GMEM_INPUTTYPE *istg_out2,int *scalar_conv_args)
{
#if __SDSOC
	#pragma SDS async(1)
#endif
	XiConvolutionTop(weights1,weights2, output1, output2, input_other1, input_other2, input_1st, bias, inp_norm2,inp_norm3,istg_out1,istg_out2, scalar_conv_args);
}

int PoolWrapper(KER_GMEM_MAXPOOLTYPE *in1,KER_GMEM_MAXPOOLTYPE *in2, KER_GMEM_MAXPOOLTYPE *out1,KER_GMEM_MAXPOOLTYPE *out2, int *scalar_pool_args)
{
#if __SDSOC
	#pragma SDS async(2)
#endif
   PoolTop(in1,in2,out1,out2, scalar_pool_args);
}

int SgemvWrapper(KER_GMEM_FC_INPUTTYPE *A1,KER_GMEM_FC_INPUTTYPE *A2, KER_GMEM_FC_INPUTTYPE *x, KER_GMEM_FC_OUTTYPE *y_in, KER_GMEM_FC_OUTTYPE *y_out, int *scalar_gemv_args )
{
#if __SDSOC
	#pragma SDS async(3)
#endif
	xiSgemvTop(A1,A2, x, y_in, y_out, scalar_gemv_args);
}

int DeconvWrapper(KER_GMEM_DECONVINPTYPE* deconvIN, KER_GMEM_DECONVINPTYPE* deconvWT, KER_GMEM_DECONVINPTYPE* deconvBias, KER_GMEM_DECONVOUTTYPE* deconvIDout, int *scalar_deconv_args)
{
#if __SDSOC
	#pragma SDS async(5)
#endif
	XiDeconvTop(deconvIN, deconvWT, deconvBias, deconvIDout, scalar_deconv_args);
}

