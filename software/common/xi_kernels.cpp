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

#include "../common/kernelinfo_class.h"
#include "xi_kernels.h"
#include "../scheduler/xi_utils.hpp"

#ifdef __SDSOC
#include <sds_lib.h>
#endif

#include <ap_int.h>

#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)

#if PORT_BITWIDTH_64BIT
#define GMEM_INPUTTYPE       unsigned long long int
#define GMEM_INTYPE_OTHER    ap_uint<64>
#define GMEM_WEIGHTTYPE      ap_uint<64>
#define GMEM_OUTTYPE         ap_uint<64>
#define GMEM_MAXPOOLTYPE     ap_uint<128>
#define GMEM_INTYPE_FC       ap_uint<128>
#define GMEM_BIASTYPE        unsigned long long int
#else
#define GMEM_INPUTTYPE       unsigned long long int
#define GMEM_INTYPE_OTHER    ap_uint<128>
#define GMEM_WEIGHTTYPE      ap_uint<128>
#define GMEM_OUTTYPE         ap_uint<128>
#define GMEM_MAXPOOLTYPE     ap_uint<128>
#define GMEM_INTYPE_FC       ap_uint<128>
#define GMEM_BIASTYPE        unsigned long long int
#endif  //#if PORT_BITWIDTH_64BIT

int ConvolutionWrapper(GMEM_WEIGHTTYPE *weights1,GMEM_WEIGHTTYPE *weight2,
#if (KER_PROC==16 || (PORT_BITWIDTH_64BIT==1 && KER_PROC==8))
		GMEM_WEIGHTTYPE *weights3,GMEM_WEIGHTTYPE *weight4,
#endif
		GMEM_OUTTYPE *output1,
#if !SINGLE_IO_PORT
		GMEM_OUTTYPE *output2,
#endif
		GMEM_INTYPE_OTHER *input_other1,
#if !SINGLE_IO_PORT
		GMEM_INTYPE_OTHER *input_other2,
#endif
		GMEM_INPUTTYPE *input_1st, GMEM_BIASTYPE *bias,
#if !DISABLE_BN
		GMEM_INPUTTYPE *input_norm2, GMEM_INPUTTYPE *input_norm3,
#endif
		GMEM_INPUTTYPE *istg_out1,
#if !SINGLE_IO_PORT
		GMEM_INPUTTYPE *istg_out2,
#endif
		int *scalar_conv_args);

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
		int *scalar_conv_args )
{

	//Valid for FC Layers using conv kernel
	if(scalar_conv_args[34] == OPCODE_FC2CONV)
	{
		int reH = scalar_conv_args[107];
		int reW = scalar_conv_args[108];
		int reD = scalar_conv_args[109];
		int isize = reH * reW * reD;

		if(scalar_conv_args[125] == 0)  //fc_id
		{

			//# Re-arrange conv output data to fc input data
			fc_inputdatawrite((IO_DATA_TYPE *)weights1, (IO_DATA_TYPE *)weights2, reH, reW, reD, 0, (IO_DATA_TYPE *)input_other1, NULL);

			//# Convolution out to FC in re-arrange
			convOut_to_fcInAsWts_rearrange((int8_t *)input_other1, (int8_t *)weights1, isize);

		}
		else
		{
			//# Input size of current layer
			//# conv_inp_height * conv_inp_width
			isize = scalar_conv_args[5];
			//fprintf(stderr, "isize : %d\n", isize);
			//# FC out to FC in re-arrange
			fcOut_to_fcIn_rearrange((int16_t *)weights1, (int8_t *)input_other1, isize);
			weights1 = (CHAR_TYPE *)input_other1;
		}
	}

#if 1
	int pool_split_cnt = scalar_conv_args[110];
	//pool_split_cnt = 1;

	for(int loop_iter=0;loop_iter<pool_split_cnt;loop_iter++)
	{

		ConvolutionWrapper((GMEM_WEIGHTTYPE *)weights1,(GMEM_WEIGHTTYPE *)weights2,
#if (KER_PROC==16 || (PORT_BITWIDTH_64BIT==1 && KER_PROC==8))
				(GMEM_WEIGHTTYPE *)weights3,(GMEM_WEIGHTTYPE *)weights4,
#endif
				(GMEM_OUTTYPE *)output1,
#if !SINGLE_IO_PORT
				(GMEM_OUTTYPE *)output2,
#endif
				(GMEM_INTYPE_OTHER *)input_other1,
#if !SINGLE_IO_PORT
				(GMEM_INTYPE_OTHER *)input_other2,
#endif
				(GMEM_INPUTTYPE *)input_1st, (GMEM_BIASTYPE *)bias,
#if !DISABLE_BN
				(GMEM_INPUTTYPE *)input_norm2, (GMEM_INPUTTYPE *)input_norm3,
#endif
				(GMEM_INPUTTYPE *)istg_out1,
#if !SINGLE_IO_PORT
				(GMEM_INPUTTYPE *)istg_out2,
#endif
				scalar_conv_args);

		if( (scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV) )
		{
			input_other1 = input_other1 + scalar_conv_args[111];
			istg_out1    = istg_out1 + scalar_conv_args[112];
#if !SINGLE_IO_PORT
			input_other2 = input_other2 + scalar_conv_args[111];
			istg_out2    = istg_out2+ scalar_conv_args[112];
#endif
			/*
			if(loop_iter==(pool_split_cnt-2))
			{
				scalar_conv_args[4] = scalar_conv_args[113];
				scalar_conv_args[5] = scalar_conv_args[114];
				scalar_conv_args[50] = scalar_conv_args[113];
				scalar_conv_args[51] = scalar_conv_args[114];

				scalar_conv_args[56] = scalar_conv_args[115];
				scalar_conv_args[58] = scalar_conv_args[116];
				scalar_conv_args[14] = scalar_conv_args[117];
				scalar_conv_args[15] = scalar_conv_args[118];
			}*/
/*
			if(loop_iter==(pool_split_cnt-1))
			{
				scalar_conv_args[4] = scalar_conv_args[109];
			}
			*/
		}
		
#ifdef __SDSOC
		if((pool_split_cnt > 1) && (loop_iter < (pool_split_cnt-1)))
			sds_wait(1);
#endif		


	}
#endif
}

#if 1//0//(IO_TYPE==16)
int PoolWrapper(GMEM_MAXPOOLTYPE *poolin, GMEM_MAXPOOLTYPE *poolout, GMEM_MAXPOOLTYPE *poolin1, GMEM_MAXPOOLTYPE *poolout1, GMEM_MAXPOOLTYPE *wts, int *scalar_pool_args);
#else
int PoolWrapper(GMEM_MAXPOOLTYPE *poolin, GMEM_MAXPOOLTYPE *poolout, GMEM_MAXPOOLTYPE *poolin1, GMEM_MAXPOOLTYPE *poolout1, int *scalar_pool_args);
#endif

void PoolForward(
		SHORT_TYPE *pool_in, SHORT_TYPE *pool_out,
		SHORT_TYPE *pool_in1, SHORT_TYPE *pool_out1,
		CHAR_TYPE * wts,
		int *scalar_pool_args
)
{
#if __POOL_ENABLE__
	PoolWrapper((GMEM_MAXPOOLTYPE *)pool_in, (GMEM_MAXPOOLTYPE *)pool_in1,
			(GMEM_MAXPOOLTYPE *)pool_out, (GMEM_MAXPOOLTYPE *)pool_out1,
#if 1//0//(IO_TYPE==16)
			(GMEM_MAXPOOLTYPE*) wts,
#endif
			scalar_pool_args);
#endif
}

int SgemvWrapper(GMEM_INTYPE_FC *A1, GMEM_INTYPE_FC *A2,
		GMEM_INTYPE_FC *x,
		GMEM_INTYPE_FC *y_in, GMEM_INTYPE_FC *y_out,
		int *scalar_gemv_args );

void FcForward(
		CHAR_TYPE *A1, CHAR_TYPE *A2,
		SHORT_TYPE *in1, SHORT_TYPE *in2, SHORT_TYPE *in3,
		SHORT_TYPE *y_in, SHORT_TYPE *y_out,
		int *scalar_fc_args
)
{

	int reH = scalar_fc_args[6];
	int reW = scalar_fc_args[7];
	int reD = scalar_fc_args[8];

	int isize = reH * reW * reD;
	int prev_layer_type = scalar_fc_args[5];
	short *fc_in;

	if(prev_layer_type != 2)//FC_LAYER)
	{
		//# Re-arrange input data
		fc_inputdatawrite((IO_DATA_TYPE *)in1, (IO_DATA_TYPE *)in2, reH, reW, reD, 0, (IO_DATA_TYPE *)in3, NULL);
		fc_in = in3;
#if ENABLE_CONSOLE_TEXT
		std::cout << "\n[DEBUG] Re-arrange: done !" << std::endl;
#endif
	}
	else
	{
		fc_in = in1;
	}


#if 0
	SgemvWrapper((GMEM_INTYPE_FC*)A1, (GMEM_INTYPE_FC*)A2, (GMEM_INTYPE_FC*)fc_in,
			(GMEM_INTYPE_FC*)y_in, (GMEM_INTYPE_FC*)y_out, scalar_fc_args);
#endif

}


void SwFcWrapper(SW_FC_DATA_TYPE *fc_in, SW_FC_DATA_TYPE *fc_wgts, SW_FC_DATA_TYPE *fc_bias, SW_FC_DATA_TYPE *fc_out, int *scalar_fc_args);
void reluWrapper(float* in, float* out, int *scalar_fc_args);

void SwFcForward(IO_DATA_TYPE *inp1, IO_DATA_TYPE *inp2, SW_FC_DATA_TYPE *inp3, SW_FC_DATA_TYPE *fc_wgts, SW_FC_DATA_TYPE *fc_bias, SW_FC_DATA_TYPE *fc_out, int *scalar_fc_args)
{

	int reH = scalar_fc_args[6];
	int reW = scalar_fc_args[7];
	int reD = scalar_fc_args[8];

	int isize = reH * reW * reD;
	int prev_layer_type = scalar_fc_args[5];
	int quant_scheme_flag = scalar_fc_args[10];
#if 0
	if(prev_layer_type != 2)//FC_LAYER)
	{
		if(quant_scheme_flag == 1)
		{
			float *th_in_ptr 	  = (float*)(&scalar_fc_args[9]);
			float th_in = th_in_ptr[0];
			//# Re-arrange conv output data to fc input data
			fc_inputdatawrite_float((IO_DATA_TYPE *)inp1, (IO_DATA_TYPE *)inp2, reH, reW, reD, 0, (SW_FC_DATA_TYPE *)inp3, th_in);//scalar_fc_args[1]);
		}
		else
		{
			int inp_fbits = scalar_fc_args[11];
			fc_inputdatawrite_float_dynamicfixed((IO_DATA_TYPE *)inp1, (IO_DATA_TYPE *)inp2, reH, reW, reD, 0, (SW_FC_DATA_TYPE *)inp3, inp_fbits);//scalar_fc_args[1]);
		}
		inp1  = (IO_DATA_TYPE *)inp3;
	}
#endif

	SwFcWrapper((SW_FC_DATA_TYPE *)inp1, fc_wgts, fc_bias, fc_out, scalar_fc_args);

	if(scalar_fc_args[4] == 1)  //check for relu flag
	{
		reluWrapper(fc_out, fc_out, scalar_fc_args);
	}
}

void SwSoftmaxWrapper(IO_DATA_TYPE1 *softmax_in, IO_DATA_TYPE1 *sumBuffer, IO_DATA_TYPE1 *softmax_out, int *scalar_softmax_args);

void SwSoftmaxForward(IO_DATA_TYPE *softmax_in, IO_DATA_TYPE1 *softmax_in1, IO_DATA_TYPE1 *sumBuffer, IO_DATA_TYPE1 *softmax_out, int *scalar_softmax_args)
{

#if 0
	if(scalar_softmax_args[6] == OPCODE_FC2CONV)
	{
		int isize = scalar_softmax_args[0] * scalar_softmax_args[1];
		fcOut_to_smaxIn((int16_t *)softmax_in, (int8_t *)softmax_in1, isize);
		softmax_in = (int8_t*)softmax_in1;
	}
#endif

#if 1
	int prev_layer_type = scalar_softmax_args[2];
	if(prev_layer_type != 6)//PERMUTE)
	{
		int isize = scalar_softmax_args[0] * scalar_softmax_args[1];
		//# Read scale factor for input
		//float *th_in_ptr 	  = (float*)(&scalar_softmax_args[8]);
		//float th_in = th_in_ptr[0];
		//swfcOut_to_smaxIn1((IO_DATA_TYPE1 *)softmax_in, (int8_t *)softmax_in1, isize, th_in);
		swfcOut_to_smaxIn((IO_DATA_TYPE1 *)softmax_in, (IO_DATA_TYPE1 *)softmax_in1, isize);
		//IO_DATA_TYPE1 softmax_in = (IO_DATA_TYPE1*)softmax_in1;
	}
	else
	{
		softmax_in1 = (IO_DATA_TYPE1*)softmax_in;
	}
#endif

	SwSoftmaxWrapper(softmax_in1, sumBuffer, softmax_out, scalar_softmax_args);
}

void DeconvWrapper(short* deconvIN, short* deconvWT, short* deconvBias, unsigned long long int* deconvIDout, int *scalar_deconv_args);
//void DeconvForwardWrapper(short* in1, short* in2, short* in3, short* weights, short* bias, int * out, int *scalars)
void DeconvForward(
		short* deconvIN1, short* deconvIN2, short* deconvIN3,
		short* deconvWT,
		short* deconvBias, int* deconvIDout,
		int *scalar_deconv_args
)
{
	//# Re-arrage previous layer output for Deconv input
	DeconvInReArrange((IO_DATA_TYPE *)deconvIN1, (IO_DATA_TYPE *)deconvIN2, deconvIN3, scalar_deconv_args[1], scalar_deconv_args[2], scalar_deconv_args[0]);

	/*FILE *fp = fopen("deconv_in.txt", "w");
	if(fp == NULL)
	{
		fprintf(stderr, "Failed to create file\n");
	}
	for(int i = 0; i < 24*16*16; i++)
	{
		fprintf(fp, "%hd\n", deconvIN3[i]);
	}
	fclose(fp);*/

	/*
#ifdef __SDSOC
	FILE *fp = fopen("/mnt/models/AlexNetFCN/caffe_ref/score_fr_out.txt", "r");
#else
	FILE *fp = fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/AlexNetFCN-8Bit/caffe_ref/score_fr_out.txt", "r");
#endif
	if(fp == NULL)
	{
		fprintf(stderr, "cant read file\n");
		return;
	}

	for(int i = 0; i < 21*16*16; i++)
	{
		float val;
		fscanf(fp, "%f", &val);
		short ival = (short)val;
		short hls_val = (short)val;//ConvertToFP(val, ival, 3);
		for(int j = 0; j < XBATCH_SIZE; j++)
		{
			int idx = j*21*16*16;
			deconvIN3[i+idx] = hls_val;
		}
	}

	//for(int i = 0; i < 10; i++)
		//	fprintf(stderr, "in[%d] = %hd\n", i, deconv_in[i]);
	 */

	int o_width = AlignSize(scalar_deconv_args[3], 2);
	int n_planes = AlignSize(scalar_deconv_args[0], 32);
	for(int batch_id = 0; batch_id < XBATCH_SIZE; ++batch_id)
	{
		//fprintf(stderr, "[INFOx] Batch : %d\n", batch_id);
		short* deconv_in = (short *)(deconvIN3+(batch_id*scalar_deconv_args[1]*scalar_deconv_args[2]*n_planes));

		//# Hardcoded output depth as 1
		unsigned long long int *deconv_out = (unsigned long long int *)(deconvIDout+(batch_id*scalar_deconv_args[4]*o_width*1));
		//fprintf(stderr, "Deconv Start : \n");
#if __DECONV_ENABLE__
		DeconvWrapper(deconv_in, deconvWT, deconvBias, (unsigned long long int*)deconv_out, scalar_deconv_args);
#endif

#ifdef __SDSOC
		if(batch_id < (XBATCH_SIZE-1))
			sds_wait(3);
#endif
		//fprintf(stderr, "Deconv End : \n");
	}

	//DeconvWrapper(deconvIN, deconvWT, deconvBias, deconvIDout, scalar_deconv_args);
}

#if !SINGLE_IO_PORT

void NormalizationWrapper(IO_DATA_TYPE *inArray, IO_DATA_TYPE *inArray1, IO_DATA_TYPE *gamma,
		IO_DATA_TYPE *outArray, IO_DATA_TYPE *outArray1, int *sumBuffer, int *scalar_norm_args);

void NormalizationWrapper_float(IO_DATA_TYPE *inArray, IO_DATA_TYPE *inArray1, IO_DATA_TYPE1 *gamma,
		IO_DATA_TYPE *outArray, IO_DATA_TYPE *outArray1, IO_DATA_TYPE1 *sumBuffer1, float sf_in, float sf_out, int *scalar_norm_args);

void NormalizationWrapper_fixfloat(IO_DATA_TYPE *inArray, IO_DATA_TYPE *inArray1, IO_DATA_TYPE1 *gamma,
		IO_DATA_TYPE *outArray, IO_DATA_TYPE *outArray1, IO_DATA_TYPE1 *sumBuffer1, float sf_in, float sf_out, int *scalar_norm_args);


void NormalizationForward(IO_DATA_TYPE *input, IO_DATA_TYPE *input1, IO_DATA_TYPE *gamma, IO_DATA_TYPE *output, IO_DATA_TYPE *output1, int *sumBuffer, float sf_in, float sf_out, int *scalar_norm_args)
{

	if(scalar_norm_args[9] == 1)  //offline mode
	{
		NormalizationWrapper_float(input, input1, (IO_DATA_TYPE1 *)gamma, output, output1, (IO_DATA_TYPE1 *)sumBuffer, sf_in, sf_out, scalar_norm_args);
		//NormalizationWrapper_fixfloat(input, input1, (IO_DATA_TYPE1 *)gamma, output, output1, (IO_DATA_TYPE1 *)sumBuffer, sf_in, sf_out, scalar_norm_args);
	}
	else
	{
		NormalizationWrapper(input, input1, gamma, output, output1, sumBuffer, scalar_norm_args);
	}
}
#else
void NormalizationWrapper(IO_DATA_TYPE *inArray, IO_DATA_TYPE *gamma,
		IO_DATA_TYPE *outArray, int *sumBuffer, int *scalar_norm_args);

void NormalizationWrapper_float(IO_DATA_TYPE *inArray, IO_DATA_TYPE1 *gamma,
		IO_DATA_TYPE *outArray, IO_DATA_TYPE1 *sumBuffer1, float sf_in, float sf_out, int *scalar_norm_args);

void NormalizationForward(IO_DATA_TYPE *input, IO_DATA_TYPE *gamma, IO_DATA_TYPE *output, int *sumBuffer, float sf_in, float sf_out, int *scalar_norm_args)
{

	if(scalar_norm_args[9] == 1)  //offline mode
	{
		NormalizationWrapper_float(input, (IO_DATA_TYPE1 *)gamma, output, (IO_DATA_TYPE1 *)sumBuffer, sf_in, sf_out, scalar_norm_args);
	}
	else
	{
		NormalizationWrapper(input, gamma, output, sumBuffer, scalar_norm_args);
	}
}
#endif  //#if !SINGLE_IO_PORT

#if !SINGLE_IO_PORT
void PermuteWrapper(IO_DATA_TYPE *input1, IO_DATA_TYPE *input2, IO_DATA_TYPE *sds_input1, IO_DATA_TYPE *sds_input2, IO_DATA_TYPE *output, int *scalar_permute_args);

void PermuteForward(IO_DATA_TYPE *input1, IO_DATA_TYPE *input2, IO_DATA_TYPE *sds_input1, IO_DATA_TYPE *sds_input2, IO_DATA_TYPE *output, int *scalar_permute_args)
{
	PermuteWrapper((IO_DATA_TYPE *)input1, (IO_DATA_TYPE *)input2, (IO_DATA_TYPE *)sds_input1, (IO_DATA_TYPE *)sds_input2, (IO_DATA_TYPE *)output, scalar_permute_args);
}
#else
void PermuteWrapper(IO_DATA_TYPE *input1, IO_DATA_TYPE *sds_input1, IO_DATA_TYPE *output, int *scalar_permute_args);

void PermuteForward(IO_DATA_TYPE *input1, IO_DATA_TYPE *sds_input1, IO_DATA_TYPE *output, int *scalar_permute_args)
{
	PermuteWrapper((IO_DATA_TYPE *)input1, (IO_DATA_TYPE *)sds_input1, (IO_DATA_TYPE *)output, scalar_permute_args);
}
#endif  //#if !SINGLE_IO_PORT

void NMSWrapper(int* nms_finalcount, int *nms_id, int *nms_label, float *nms_box,
		float *nms_score, float* conf, float* pbox, IO_DATA_TYPE* loc, float* var,
		int *scalar_nms_args );


void NMSForward(float *nms_score, int *nms_id, int *nms_label, float *nms_box, int* nms_finalboxcount,
		float* conf, float* pbox, IO_DATA_TYPE* loc, float* var, int *scalar_nms_args )
{
	NMSWrapper(nms_finalboxcount, nms_id, nms_label, nms_box, nms_score, conf, pbox, loc, var, scalar_nms_args);
}

void CropWrapper(int* inarray, int* outarray, int in_height, int in_width, int out_height, int out_width, int offset, int channels);

void CropForward(int *input, int *output, int *scalars)
{

	int n_width = AlignSize(scalars[2], 2);
	//fprintf(stderr, "Start : CropForwardWrapper\n");
	for(int batch_id = 0; batch_id < XBATCH_SIZE; ++batch_id)
	{
		int *crop_in = (int *)(input+(batch_id*scalars[1]*n_width*1));
		int *crop_out = (int *)(output+(batch_id*scalars[4]*scalars[5]*1));
		//fprintf(stderr, "cropLayer Start : \n");
		//# Crop sw-kernel
		CropWrapper(crop_in, crop_out, scalars[1], scalars[2], scalars[4],  scalars[5], scalars[7], 1);
		//fprintf(stderr, "cropLayer End : \n");
	}
}

void XpackWrapper(void *input1, void *input2, void *output1, void *output2, string quant, int *params, float *float_params);

void XpackForward(void *input1, void *input2, void *output1, void *output2, string quant, int *params, float *float_params)
{
	XpackWrapper(input1, input2, output1, output2, quant, params, float_params);
}

void EltwiseaddWrapper(IO_DATA_TYPE *input1, IO_DATA_TYPE *input2, IO_DATA_TYPE *input3, IO_DATA_TYPE *input4, IO_DATA_TYPE *output1, IO_DATA_TYPE *output2, int *params);

void EltwiseaddForward(void *input1, void *input2, void *input3, void *input4, void *output1, void *output2, int *params)
{
	EltwiseaddWrapper((IO_DATA_TYPE *)input1, (IO_DATA_TYPE *)input2, (IO_DATA_TYPE *)input3, (IO_DATA_TYPE *)input4, (IO_DATA_TYPE *)output1, (IO_DATA_TYPE *)output2, params);
}


#endif//_XI_KERNELS_HPP_
