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

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "xi_init.hpp"
#include "xi_common_utils.hpp"
#include "xi_dnn_conv_utils.h"
#include "xi_dnn_conv_utils.hpp"
#include "xi_dnn_fc_utils.hpp"
#include "xi_dnn_deconv_utils.hpp"
#include "xi_dnn_norm_utils.hpp"
#include "xi_dnn_nms_utils.hpp"

typedef struct file_paths_
{
	char *wgt_path;
	char *bias_path;

	char *mean_path;
	char *gamma_path;
	char *beta_path;
	char *variance_path;
}file_paths;

void printShape(vector<int> shape)
{
#if EN_DEBUG_INIT_PRINT
	for(int i = 0; i < shape.size(); i++)
	{
		cout << "shape[i] : " << shape[i] << endl;
	}
#endif
}


void convInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer, int layer_id, file_paths *const_data_path)
{

#if	EN_DEBUG_INIT_PRINT
	cout<<"convInit Start: "<<endl;
#endif

	ConvolutionParameter *conv_param = xlayer_seq->hw_ops->conv_params;

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottom.at(0).blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->top.at(0).blob->shape;

	int *scalar_conv_args = (int *)currentLayer->params;

	scalar_conv_args[0]  = bottomShape.at(2);  //input_height
	scalar_conv_args[1]  = bottomShape.at(3);  //input_width
	scalar_conv_args[2]  = topShape.at(2);     //output_height
	scalar_conv_args[3]  = topShape.at(3);     //output_width
	scalar_conv_args[4]  = conv_param->M;      //output planes
	scalar_conv_args[5]  = conv_param->N;      //input planes
	scalar_conv_args[6]  = conv_param->stride_h;      //stride_h
	scalar_conv_args[7]  = conv_param->filter_h;      //filter_h
	scalar_conv_args[8]  = conv_param->filter_w;      //filter_w
	scalar_conv_args[9]  = conv_param->pad_h;         //pad_h
	scalar_conv_args[10] = conv_param->reluflag;      //relu
	scalar_conv_args[11] = conv_param->group-1;       //default group is 1
	scalar_conv_args[12] = layer_id;                  //layer_id
	scalar_conv_args[29] = conv_param->dilation;      //dilation
	scalar_conv_args[34] = xlayer_seq->opcode;        //Opcode

	if( (scalar_conv_args[34] == OPCODE_BN) || (scalar_conv_args[34] == OPCODE_ELTWISE) )
	{
		scalar_conv_args[10] = conv_param->extra_reluflag;
		scalar_conv_args[6] = 1;
		scalar_conv_args[7] = 1;
		scalar_conv_args[8] = 1;
	}


	stgRowCount(scalar_conv_args);

	straddleFactorCount(scalar_conv_args);

	nkpfCount(scalar_conv_args);


	int group_flag = scalar_conv_args[11];

	int io_precision = currentLayer->qf_format.ip_fbits+currentLayer->qf_format.wt_fbits-currentLayer->qf_format.op_fbits;

	scalar_conv_args[20] = io_precision;
	scalar_conv_args[21] = 0;

	//# Mean Values
	scalar_conv_args[22] = 0;
	scalar_conv_args[23] = 0;
	scalar_conv_args[24] = 0;
	scalar_conv_args[25] = 0;


	scalar_conv_args[18] = 0;              //out_offset
	scalar_conv_args[26] = 0;              //mean_sub_flag
	scalar_conv_args[27] = 0;              //bias_offset
	scalar_conv_args[28] = 0;              //weight_offset
	scalar_conv_args[30] = group_flag;     //group_flag for the next group iteration
	//scalar_conv_args[31] = 0;            //0

	//LRN parameters
	scalar_conv_args[31] = 1;              //read_from_ddr_en
	scalar_conv_args[32] = 1;              //write_from_ddr_en


	float float_val = 1.0f;
	short ival = (short)(float_val);
	int fbits = 2;
	short fxval = ConvertToFP(float_val, ival, fbits);
	scalar_conv_args[33] = fxval;              //norm_k = 1.0f in Q14.2 format


	//In LRN_SoS Mul_in = alpha/localSize, In LRN_PnS Mul_in = 1.0f in Q14.2
	float alpha   = xlayer_seq->hw_ops->conv_params->lrn_alpha;
	int localSize = xlayer_seq->hw_ops->conv_params->lrn_lsize;
	int mul_in = 0;

	if(scalar_conv_args[34] == 6)  //Sos
	{
		mul_in = alpha/localSize;
	}
	if(scalar_conv_args[34] == 9)  //LRN_PnS
	{
		mul_in = fxval;            //1.0f in Q14.2 format
	}
	scalar_conv_args[35] = mul_in;

	scalar_conv_args[36] = currentLayer->qf_format.ip_fbits;//6;//2;  //Norm_prec is assume to be 2
	//Norm_prec2 = 14 + Fbits_in + Fbits_out
	scalar_conv_args[37] = 14+currentLayer->qf_format.ip_fbits+currentLayer->qf_format.op_fbits;

#if EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
		fprintf(stderr,"\t%d",(int)scalar_conv_args[i]);
	//	fprintf(stderr,"\n--------------in_height:plane = %d:%d \tout_height:plane = %d:%d\t in_stgcnt = %d \tout_stgcnt = %d ",l_input_height_ffa0 ,l_input_planes_ffa0, l_output_height_ffa0, l_output_planes_ffa0,l_istg_row_cnt_ffa0, l_ostg_row_cnt_ffa0);
#endif

#if 0
	fprintf(stderr, " l_input_height_ffa0		 = %d\n",scalar_conv_args[0]);
	fprintf(stderr, " l_input_Width_ffa0		 = %d\n",scalar_conv_args[1]);
	fprintf(stderr, " l_output_height_ffa0		 = %d\n",scalar_conv_args[2]);
	fprintf(stderr, " l_output_width_ffa0		 = %d\n",scalar_conv_args[3]);
	fprintf(stderr, " l_output_planes_ffa0		 = %d\n",scalar_conv_args[4]);
	fprintf(stderr, " l_input_planes_ffa0		 = %d\n",scalar_conv_args[5]);
	fprintf(stderr, " l_conv_stride_ffa0		 = %d\n",scalar_conv_args[6]);
	fprintf(stderr, " l_filter_height_ffa0		 = %d\n",scalar_conv_args[7]);
	fprintf(stderr, " l_filter_width_ffa0		 = %d\n",scalar_conv_args[8]);
	fprintf(stderr, " l_pad_ffa0				 = %d\n",scalar_conv_args[9]);
	fprintf(stderr, " l_relu_ffa0				 = %d\n",scalar_conv_args[10]);
	fprintf(stderr, " l_group_ffa0				 = %d\n",scalar_conv_args[11]);
	fprintf(stderr, " l_layer_ffa0				 = %d\n",scalar_conv_args[12]);
	fprintf(stderr, " l_nkpf_ffa0				 = %d\n",scalar_conv_args[13]);
	fprintf(stderr, " l_istg_row_cnt_ffa0        = %d\n",scalar_conv_args[14]);
	fprintf(stderr, " l_ostg_row_cnt_ffa0		 = %d\n",scalar_conv_args[15]);
	fprintf(stderr, " l_compute_planes_ffa0		 = %d\n",scalar_conv_args[16]);
	fprintf(stderr, " l_straddle_factor_ffa0	 = %d\n",scalar_conv_args[17]);
	fprintf(stderr, " l_out_offset_ffa0			 = %d\n",scalar_conv_args[18]);
	fprintf(stderr, " l_single_load_kernel_ffa0  = %d\n",scalar_conv_args[19]);
	fprintf(stderr, " l_inout_precision_ffa0	 = %d\n",scalar_conv_args[20]);
	fprintf(stderr, " l_buffer_split_ffa0 		 = %d\n",scalar_conv_args[21]);
	fprintf(stderr, " l_mean_value0_ffa0		 = %d\n",scalar_conv_args[22]);
	fprintf(stderr, " l_mean_value1_ffa0		 = %d\n",scalar_conv_args[23]);
	fprintf(stderr, " l_mean_value2_ffa0		 = %d\n",scalar_conv_args[24]);
	fprintf(stderr, " l_mean_value3_ffa0		 = %d\n",scalar_conv_args[25]);
	fprintf(stderr, " l_mean_sub_flag_ffa0       = %d\n",scalar_conv_args[26]);
	fprintf(stderr, " l_bias_offset_ffa0         = %d\n",scalar_conv_args[27]);
	fprintf(stderr, " l_weight_offset_ffa0       = %d\n",scalar_conv_args[28]);
	fprintf(stderr, " l_dilation_factor_ffa0     = %d\n",scalar_conv_args[29]);
	fprintf(stderr, " l_group_flag_ffa0          = %d\n",scalar_conv_args[30]);

	fprintf(stderr, " l_read_from_ddr_en_ffa0  = %d\n",scalar_conv_args[31]);
	fprintf(stderr, " l_write_to_ddr_en_ffa0   = %d\n",scalar_conv_args[32]);
#endif

	currentLayer->kernType = CONV;

	char *base_path = currentLayer->base_path;

	if( (scalar_conv_args[34] == OPCODE_CONV) || (scalar_conv_args[34] == OPCODE_FUSE_BN_CONV) )
	{
		//Loading Weights
		char *wt_ptr0 = (char *)currentLayer->wts_ptrs[0];
		char *wt_ptr1 = (char *)currentLayer->wts_ptrs[1];

		int wt_bw = xlayer_seq->hw_ops->wt_bw;
		int wt_fl = xlayer_seq->hw_ops->wt_fl;

		loadConvWgtsTXT(wt_ptr0, wt_ptr1, const_data_path->wgt_path, scalar_conv_args, currentLayer->qf_format.wt_fbits, wt_bw, wt_fl);

		//Loading Bias
		short *bias_ptr = (short *)currentLayer->bias_ptr;
		
		int bs_bw, bs_fl;

		if(xlayer_seq->hw_ops->conv_params->has_software_fuse)
		{
			bs_bw = xlayer_seq->hw_ops->op_bw;
			bs_fl = xlayer_seq->hw_ops->op_fl;
		}
		else
		{
			bs_bw = xlayer_seq->hw_ops->wt_bw;
			bs_fl = xlayer_seq->hw_ops->wt_fl;
		}

		loadConvBiasTXT(bias_ptr, const_data_path->bias_path, conv_param->M, layer_id, currentLayer->qf_format.bs_fbits, bs_bw, bs_fl);

	}

	if( (scalar_conv_args[34] == OPCODE_BN) || (scalar_conv_args[34] == OPCODE_FUSE_BN_CONV) )
	{
		scalar_conv_args[35] = 1<<currentLayer->qf_format.ip_fbits;

		short *bias_ptr = (short *)currentLayer->xtra_ptrs[0];//in_ptrs[3];//bias_ptr;
		currentLayer->in_ptrs[4] = bias_ptr;;
		float epsilon =  xlayer_seq->hw_ops->conv_params->batchnorm_eps;
		int nElems  = scalar_conv_args[5];   //inp planes
		int ip_fbits = currentLayer->qf_format.ip_fbits;
		int op_fbits = currentLayer->qf_format.op_fbits;

		int mean_fbits = xlayer_seq->hw_ops->bn_mean_fl;
		int variance_fbits = xlayer_seq->hw_ops->bn_variance_fl;

		int gamma_fbits = xlayer_seq->hw_ops->scale_gamma_fl;
		int beta_fbits = xlayer_seq->hw_ops->scale_beta_fl;
		int gamma_by_std_fbits = xlayer_seq->hw_ops->scale_gamma_by_std_fl;

		int norm_prec = ip_fbits+gamma_by_std_fbits-op_fbits;
		int norm_prec_3 = ip_fbits+gamma_by_std_fbits-beta_fbits;
		scalar_conv_args[36] = norm_prec;
		scalar_conv_args[37] = 0;
		scalar_conv_args[38] = norm_prec_3;

		mean_fbits = ip_fbits;

		load_mean_gamma_beta(bias_ptr, const_data_path->mean_path, const_data_path->variance_path, const_data_path->gamma_path, const_data_path->beta_path, nElems, epsilon,
				mean_fbits, gamma_by_std_fbits, beta_fbits);

#if 0
		fprintf(stderr,"%s\n", const_data_path->wgt_path);
		fprintf(stderr,"%s\n", const_data_path->bias_path);

		fprintf(stderr,"%s\n", const_data_path->mean_path);
		fprintf(stderr,"%s\n", const_data_path->variance_path);
		fprintf(stderr,"%s\n", const_data_path->gamma_path);
		fprintf(stderr,"%s\n", const_data_path->beta_path);
#endif

	}

	if(scalar_conv_args[34] == OPCODE_ELTWISE)
	{

		scalar_conv_args[35] = 0; //mul_in;
		scalar_conv_args[36] = 0; //norm_prec
		scalar_conv_args[37] = 0; //norm_prec_2
		scalar_conv_args[38] = 0; //norm_prec_3
	}

	int inDepth    = scalar_conv_args[5];
	int outDepth   = scalar_conv_args[4];
	if((group_flag) && (inDepth > 4))
	{
		scalar_conv_args[5] = scalar_conv_args[5]/2;
	}

	if((group_flag) && (outDepth > 8))
	{
		scalar_conv_args[4] = scalar_conv_args[4]/2;
	}

	scalar_conv_args[11] = 0;              //making group as zero for the first group

	for(int i=39;i<MAX_PARAM_SIZE;i++)
	{
		scalar_conv_args[i] = 0;
	}

#if	EN_DEBUG_INIT_PRINT
	cout<<"convInit End: "<<endl;
#endif
}

void poolInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer, int layer_id)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"poolInit Start : "<<endl;
#endif

	PoolingParameter *pool_param = xlayer_seq->hw_ops->pool_params;

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottom.at(0).blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->top.at(0).blob->shape;

	int *scalar_pool_args = (int *)currentLayer->params;

	int pooltype = (int)xlayer_seq->hw_ops->pool_params->PoolType;

	scalar_pool_args[0] = bottomShape.at(2);  //input_height
	scalar_pool_args[1] = bottomShape.at(3);  //input_width
	scalar_pool_args[2] = topShape.at(2);     //output_height
	scalar_pool_args[3] = topShape.at(3);     //output_width
	scalar_pool_args[4] = bottomShape.at(1)/2;    //out_depth/2 ??
	scalar_pool_args[5] = pool_param->stride_h;      //stride_h
	scalar_pool_args[6] = pool_param->stride_w;      //stride_v
	scalar_pool_args[7] = pool_param->kernel_h;      //kernel_h
	scalar_pool_args[8] = pool_param->kernel_w;      //kernel_w
	scalar_pool_args[9] = pooltype;                  //mode
	scalar_pool_args[10] = pool_param->pad_h;        //pad_h	  pad_w??

	if(scalar_pool_args[9] == 1)//Checking for MAX_POOL or AVG_POOL
	{
		//scalar_pool_args[12] = 5;
		float f_val = (float)1/(scalar_pool_args[0]*scalar_pool_args[1]);
		char ival = (char)(f_val);
		char fxval = ConvertToFP( f_val, ival, 8);
		scalar_pool_args[11] = fxval;
	}
	else
		scalar_pool_args[11] = 1;//poolLayer_precision[layer_id].divisor;

	for(int i=12;i<MAX_PARAM_SIZE;i++)
	{
		scalar_pool_args[i] = 0;
	}

#if 0
	for(int i=0;i<MAX_PARAM_SIZE;i++)
	{
		fprintf(stderr,"\t%d", scalar_pool_args[i]);
	}
#endif

	currentLayer->kernType = POOL;

#if	EN_DEBUG_INIT_PRINT
	cout<<"poolInit End : "<<endl;
#endif
}


void fcInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer, file_paths *const_data_path)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"fcInit Start: "<<endl;
#endif

	FCLayerParameter *fc_params = xlayer_seq->hw_ops->fc_params;

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottom.at(0).blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->top.at(0).blob->shape;

	int *scalar_fc_args = (int *)currentLayer->params;


	scalar_fc_args[1]  = fc_params->N;             //input planes
	scalar_fc_args[0]  = fc_params->M;             //output planes
	scalar_fc_args[2]  = fc_params->reluflag;      //relu
	scalar_fc_args[3]  = currentLayer->qf_format.wt_fbits;
	scalar_fc_args[4]  = currentLayer->qf_format.ip_fbits;
	scalar_fc_args[5]  = currentLayer->qf_format.bs_fbits;
	scalar_fc_args[6]  = currentLayer->qf_format.op_fbits;

	for(int i=7;i<MAX_PARAM_SIZE;i++)
	{
		scalar_fc_args[i] = 0;
	}

#if EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
	{
		fprintf(stderr,"\t%d", scalar_fc_args[i]);
	}
#endif

	currentLayer->kernType =FC_LAYER;

	char *base_path = currentLayer->base_path;

	//Loading Weights
	char *wt_ptr0 = (char *)currentLayer->wts_ptrs[0];
	char *wt_ptr1 = (char *)currentLayer->wts_ptrs[1];

	int wt_bw = xlayer_seq->hw_ops->wt_bw;
	int wt_fl = xlayer_seq->hw_ops->wt_fl;

	loadfcWgtsTXT(wt_ptr0, wt_ptr1, const_data_path->wgt_path, scalar_fc_args[0], scalar_fc_args[1], scalar_fc_args[3], wt_bw, wt_fl);

	//Loading Bias
	short *bias_ptr = (short *)currentLayer->bias_ptr;

	//int bs_bw = xlayer_seq->hw_ops->op_bw;
	//int bs_fl = xlayer_seq->hw_ops->op_fl;

	int bs_bw = xlayer_seq->hw_ops->wt_bw;
	int bs_fl = xlayer_seq->hw_ops->wt_fl;

	loadfcBiasTXT(bias_ptr, const_data_path->bias_path, scalar_fc_args[0], scalar_fc_args[5], bs_bw, bs_fl);

#if	EN_DEBUG_INIT_PRINT
	cout<<"fcInit End: "<<endl;
#endif
}


void softmaxInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer, int prev_layer_type)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"softmaxInit Start : "<<endl;
#endif

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottom.at(0).blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->top.at(0).blob->shape;

	printShape(bottomShape);

	int *scalar_softmax_args = (int *)currentLayer->params;

	scalar_softmax_args[0] = xlayer_seq->hw_ops->softmax_params->nclasses;//bottomShape.at(1);   //out_depth
	scalar_softmax_args[1] = xlayer_seq->hw_ops->softmax_params->nboxes;//bottomShape.at(2);   //out_depth
	scalar_softmax_args[2] = prev_layer_type;

	for(int i=3;i<MAX_PARAM_SIZE;i++)
	{
		scalar_softmax_args[i] = 0;
	}

#if 0
	for(int i=0;i<MAX_PARAM_SIZE;i++)
	{
		fprintf(stderr,"\t%d", scalar_softmax_args[i]);
	}
#endif

	currentLayer->kernType = SOFTMAX;

#if	EN_DEBUG_INIT_PRINT
	cout<<"softmaxInit End : "<<endl;
#endif
}

void deconvInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer, file_paths *const_data_path)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"deconvInit Start : "<<endl;
#endif

	DeconvolutionParameter *deconv_params = xlayer_seq->hw_ops->deconv_params;

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottom.at(0).blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->top.at(0).blob->shape;

	int *scalar_deconv_args = (int *)currentLayer->params;

	printShape(bottomShape);
	printShape(topShape);

	scalar_deconv_args[0]  = deconv_params->N;   //N
	scalar_deconv_args[1]  = bottomShape.at(3);  //input_width
	scalar_deconv_args[2]  = bottomShape.at(2);  //input_height
	scalar_deconv_args[3]  = topShape.at(3);     //output_width
	scalar_deconv_args[4]  = topShape.at(2);     //output_height
	scalar_deconv_args[5]  = deconv_params->filter_h;   //filter_h
	scalar_deconv_args[6]  = deconv_params->stride_h;   //stride_h

#if EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
	{
		fprintf(stderr,"\t%d", scalar_deconv_args[i]);
	}
#endif

	currentLayer->kernType = DECONV;

	//Loading Weights
	short *wt_ptr0 = (short *)currentLayer->wts_ptrs[0];

	int out_depth =  deconv_params->M;       //output planes
	int in_depth  = scalar_deconv_args[0];   //input planes
	int filter_h  = scalar_deconv_args[5];
	int filter_w  = scalar_deconv_args[5];
	int nElems =in_depth * out_depth * filter_h * filter_w;

	int wt_bw = xlayer_seq->hw_ops->wt_bw;
	int wt_fl = xlayer_seq->hw_ops->wt_fl;

	loadDeconvData(const_data_path->wgt_path, nElems, wt_ptr0, wt_bw, wt_fl);

	//Loading Bias
	short *bias_ptr = (short *)currentLayer->bias_ptr;
	nElems = out_depth;

	int bs_bw = xlayer_seq->hw_ops->op_bw;
	int bs_fl = xlayer_seq->hw_ops->op_fl;

	loadDeconvData(const_data_path->bias_path, nElems, bias_ptr, bs_bw, bs_fl);

#if	EN_DEBUG_INIT_PRINT
	cout<<"deconvInit End : "<<endl;
#endif
}


void normInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer, file_paths *const_data_path)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"normInit Start : "<<endl;
#endif

	int across_spatial = xlayer_seq->hw_ops->l2norm_params->across_spatial;
	int channel_shared = xlayer_seq->hw_ops->l2norm_params->channel_shared;
	float eps = xlayer_seq->hw_ops->l2norm_params->eps;


	vector<int> bottomShape =  xlayer_seq->hw_ops->bottom.at(0).blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->top.at(0).blob->shape;

	int *scalar_norm_args = (int *)currentLayer->params;

	printShape(bottomShape);

	scalar_norm_args[0] = bottomShape.at(1);   //in_depth
	scalar_norm_args[1] = bottomShape.at(2);   //input_h
	scalar_norm_args[2] = bottomShape.at(3);   //input_w

	int fbits_gamma = 10;   //fixing to 10 bits

	scalar_norm_args[3] = currentLayer->qf_format.ip_fbits;
	scalar_norm_args[4] = fbits_gamma;
	scalar_norm_args[5] = currentLayer->qf_format.op_fbits;
	scalar_norm_args[6] = across_spatial;

	for(int i=7;i<MAX_PARAM_SIZE;i++)
	{
		scalar_norm_args[i] = 0;
	}

#if EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
	{
		fprintf(stderr,"\t%d", scalar_norm_args[i]);
	}
#endif

	currentLayer->kernType = NORM;

	//Loading Weights
	short *wt_ptr0 = (short *)currentLayer->wts_ptrs[0];

	loadNormGamma(const_data_path->wgt_path, scalar_norm_args[0], wt_ptr0, fbits_gamma);

#if	EN_DEBUG_INIT_PRINT
	cout<<"normInit End : "<<endl;
#endif
}


void permuteInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"permuteInit Start : "<<endl;
#endif

	vector<int>order = xlayer_seq->hw_ops->permute_params->order;

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottom.at(0).blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->top.at(0).blob->shape;

	int *scalar_permute_args = (int *)currentLayer->params;

	printShape(bottomShape);

	scalar_permute_args[0] = bottomShape.at(1);   //in_depth
	scalar_permute_args[1] = bottomShape.at(2);   //input_h
	scalar_permute_args[2] = bottomShape.at(3);   //input_w
	scalar_permute_args[3] = currentLayer->qf_format.ip_fbits;   //input_fbits


	for(int i=4;i<MAX_PARAM_SIZE;i++)
	{
		scalar_permute_args[i] = 0;
	}

#if EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
	{
		fprintf(stderr,"\t%d", scalar_permute_args[i]);
	}
#endif


	currentLayer->kernType = PERMUTE;

#if	EN_DEBUG_INIT_PRINT
	cout<<"permuteInit End : "<<endl;
#endif
}


void nmsInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer, file_paths *const_data_path)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"nmsInit Start : "<<endl;
#endif


	int *nms_params = (int *)xlayer_seq->hw_ops->nms_params;

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottom.at(0).blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->top.at(0).blob->shape;

	int *scalar_nms_args = (int *)currentLayer->params;


	scalar_nms_args[1] = xlayer_seq->hw_ops->nms_params->num_classes;
	scalar_nms_args[2] = xlayer_seq->hw_ops->nms_params->nms_top_k;
	scalar_nms_args[3] = xlayer_seq->hw_ops->nms_params->keep_top_k;
	scalar_nms_args[4] = xlayer_seq->hw_ops->nms_params->confidence_threshold;
	scalar_nms_args[5] = xlayer_seq->hw_ops->nms_params->nms_threshold;


	float confidence_threshold = xlayer_seq->hw_ops->nms_params->confidence_threshold;
	float nms_threshold        = xlayer_seq->hw_ops->nms_params->nms_threshold;

	currentLayer->float_params.push_back(confidence_threshold);
	currentLayer->float_params.push_back(nms_threshold);

	//Weights path
	vector<int> pboxShape =  xlayer_seq[0].hw_ops->nms_params->pboxShape;
	int pbox_size = pboxShape.at(2);

	scalar_nms_args[0] = pbox_size/4;//nms_nboxes;

	printShape(pboxShape);

	//Loading Weights
	float *pbox_ptr0 = (float *)currentLayer->wts_ptrs[0];

	nms_layerinputdatawrite(pbox_ptr0, pbox_size, const_data_path->wgt_path);

	vector<int> varianceShape =  xlayer_seq[0].hw_ops->nms_params->varShape;

	printShape(varianceShape);

	int variance_size = varianceShape.at(2);

	//Loading Weights
	float *variance_ptr0 = (float *)currentLayer->wts_ptrs[1];

	nms_layerinputdatawrite(variance_ptr0, variance_size, const_data_path->bias_path);

	for(int i=6;i<MAX_PARAM_SIZE;i++)
	{
		scalar_nms_args[i] = 0;
	}

#if EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
	{
		fprintf(stderr,"\t%d", scalar_nms_args[i]);
	}
#endif

	currentLayer->kernType = NMS;

#if	EN_DEBUG_INIT_PRINT
	cout<<"nmsInit End : "<<endl;
#endif
}

void cropInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"cropInit Start : "<<endl;
#endif

	int axis = xlayer_seq->hw_ops->crop_params->axis;
	int offset = xlayer_seq->hw_ops->crop_params->offset;

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottom.at(0).blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->top.at(0).blob->shape;

	int *scalar_crop_args = (int *)currentLayer->params;

	printShape(bottomShape);


	scalar_crop_args[0] = bottomShape.at(1);   //in_depth
	scalar_crop_args[1] = bottomShape.at(2);   //input_h
	scalar_crop_args[2] = bottomShape.at(3);   //input_w

	printShape(topShape);

	scalar_crop_args[3] = topShape.at(1);   //out_depth
	scalar_crop_args[4] = topShape.at(2);   //output_h
	scalar_crop_args[5] = topShape.at(3);   //output_w

	scalar_crop_args[6] = axis;
	scalar_crop_args[7] = offset;

	for(int i=8;i<MAX_PARAM_SIZE;i++)
	{
		scalar_crop_args[i] = 0;
	}

#if EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
	{
		fprintf(stderr,"\t%d", scalar_crop_args[i]);
	}
#endif


	currentLayer->kernType = CROP;

#if	EN_DEBUG_INIT_PRINT
	cout<<"cropInit End : "<<endl;
#endif
}

void eltwiseaddInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"eltwiseInit Start : "<<endl;
#endif

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottom.at(0).blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->top.at(0).blob->shape;

	int *scalar_eltwise_args = (int *)currentLayer->params;

	printShape(bottomShape);


	scalar_eltwise_args[0] = bottomShape.at(1);   //in_depth
	scalar_eltwise_args[1] = bottomShape.at(2);   //input_h
	scalar_eltwise_args[2] = bottomShape.at(3);   //input_w

	printShape(topShape);

	scalar_eltwise_args[3] = topShape.at(1);   //out_depth
	scalar_eltwise_args[4] = topShape.at(2);   //output_h
	scalar_eltwise_args[5] = topShape.at(3);   //output_w


	for(int i=8;i<MAX_PARAM_SIZE;i++)
	{
		scalar_eltwise_args[i] = 0;
	}

#if EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
	{
		fprintf(stderr,"\t%d", scalar_eltwiseadd_args[i]);
	}
#endif


	//currentLayer->kernType = ELTWISE;

#if	EN_DEBUG_INIT_PRINT
	cout<<"eltwiseInit End : "<<endl;
#endif
}



void loadMean(XlayerData *xlayer_seq, xChangeLayer *currentLayer)//(char *mean_path, xChangeLayer *xChangeHostInpLayer)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<" copy mean into Layer 1 mean data : "<<endl;
#endif

	short *mean_ptr0 = (short *)currentLayer->mean;

	vector<int> meanShape =  xlayer_seq->meanShape;

	printShape(meanShape);

	int in_w = meanShape.at(3);
	int in_h = meanShape.at(2);
	int in_d = meanShape.at(1);
	int mean_size = in_d * in_h * in_w;

	currentLayer->resize_h = xlayer_seq->resize_height;
	currentLayer->resize_w = xlayer_seq->resize_width;

	char *base_path;
	base_path = currentLayer->base_path;

	///////////// Mean Read	////////////
	string mn_path = xlayer_seq[0].meanFile;//hw_ops->Path[0];
	const char *mean_path = mn_path.c_str();

	///////////// Mean Read	////////////
	int fbits_input = currentLayer->qf_format.ip_fbits;
	float float_val;
	short fxval;
	FILE* fp = fopen(mean_path, "rb");

	if(fp == NULL)
	{
		fprintf(stderr, "\n** Cannot open mean file (or) No mean file : %s **\n", mean_path);
		//return NULL;
	}


	for(int i = 0; i < mean_size; i++)
	{
		//fscanf(fp, "%f ", &float_val);
		fread(&float_val, sizeof(float)*1, 1, fp);

		short ival = (short)float_val;
		fxval = ConvertToFP(float_val, ival, fbits_input);
		mean_ptr0[i] = fxval;
	}

	fclose(fp);

	int *scalar_conv_args = (int *)currentLayer->params;

	if(mean_size < 4)
	{
		scalar_conv_args[22] = mean_ptr0[0];
		scalar_conv_args[23] = mean_ptr0[1];
		scalar_conv_args[24] = mean_ptr0[2];
		scalar_conv_args[25] = 0;
		scalar_conv_args[26] = 1;
	}

#if	EN_DEBUG_INIT_PRINT
	cout<<" Layer 1 mean data done : "<<endl;
#endif
}

void initPrint(vector<XlayerData> xlayer_seq, vector<xChangeLayer> lay_obj[NUM_IMG])
{
	int seq_len = xlayer_seq.size();

	for(int img_id = 0; img_id < NUM_IMG; img_id++)
	{
		vector<xChangeLayer> cur_obj;
		cur_obj = lay_obj[img_id];

		cout << "*** image : " << img_id << endl;
		xChangeLayer curLayer;

		for(int iter = 0; iter < seq_len; iter++)
		{
			curLayer = cur_obj[iter];

			cout << "*** Layer : " <<  xlayer_seq[iter].hw_ops -> name << endl;
			cout << "params : " << curLayer.params << endl;
			int *params = (int *)curLayer.params;

			//# input pointers
			for(int i = 0; i < MAX_PARAM_SIZE; i++)
			{
				cout << "params[" << i << "] : " << params[i] << endl;
			}

			//# input pointers
			for(int i = 0; i < INPUT_PORTS; i++)
			{
				cout << "ip_ptrs[" << i << "] : " << curLayer.in_ptrs[i] << endl;
			}

			//# output pointers
			for(int i = 0; i < OUTPUT_PORTS; i++)
			{
				cout << "out_ptrs[" << i << "] : " << curLayer.out_ptrs[i] << endl;
			}

			if( 	(xlayer_seq[iter].hw_ops -> type == "Convolution")  ||
					(xlayer_seq[iter].hw_ops -> type == "InnerProduct") ||
					(xlayer_seq[iter].hw_ops -> type == "Deconvolution")
			)
			{
				//# weight pointers
				for(int i = 0; i < WEIGHT_PORTS; i++)
				{
					cout << "wgt_ptrs[" << i << "] : " << curLayer.wts_ptrs[i] << endl;
				}

				//# bias pointer
				cout << "bias_ptrs : " << curLayer.bias_ptr << endl;
			}


			int p_size = curLayer.previous.size();
			for(int it=0;it<p_size;it++)
			{
				cout << "previous["<<it<<"].pos : " << curLayer.previous[it].pos << endl;
				cout << "previous["<<it<<"].seqidx : " << curLayer.previous[it].seqidx << endl;
			}

			p_size = curLayer.next.size();
			for(int it=0;it<p_size;it++)
			{
				cout << "next["<<it<<"].pos : " << curLayer.next[it].pos << endl;
				cout << "next["<<it<<"].seqidx : " << curLayer.next[it].seqidx << endl;
			}

			cout << "kernType : " <<  curLayer.kernType << endl;

		}
	}
}

void initXChangeHost(char *file_path, vector < XlayerData > &xlayer_seq, vector<xChangeLayer> (&lay_obj)[NUM_IMG])
{

#if EN_CONSOLE_TXT_INIT
	cout<< "\ninitXChangeHost : " <<endl;
#endif

	int layer_cnt = xlayer_seq.size();

#if EN_CONSOLE_TXT_INIT
	cout<< "\nlayer_cnt : " << layer_cnt <<endl;
#endif

	int imgId = 0;
	int fcId = 0;
	int prev_layer_type = -1;

	char *weight_path   = (char *)malloc(500*sizeof(char));
	char *bias_path     = (char *)malloc(500*sizeof(char));
	char *mean_path     = (char *)malloc(500*sizeof(char));
	char *variance_path = (char *)malloc(500*sizeof(char));
	char *beta_path     = (char *)malloc(500*sizeof(char));
	char *gamma_path    = (char *)malloc(500*sizeof(char));

	//loading out_path, ref_path
	//char *ref_path = (char *)malloc(500*sizeof(char));
	//char *out_path = (char *)malloc(500*sizeof(char));

	for(int layerId=0;layerId<layer_cnt;layerId++)
	{

		XlayerData *current_seq_layer = &xlayer_seq[layerId];
		xChangeLayer *currentLayer = &lay_obj[imgId][layerId];

#if EN_CONSOLE_TXT_INIT
		cout<< "layer_id : " << layerId <<endl;

		cout << "*** Layer : " << xlayer_seq[layerId].hw_ops->name << endl;

		cout << "Layer Type : " << xlayer_seq[layerId].hw_ops->type << endl;
#endif

		if(!xlayer_seq[layerId].prev_layers_ID.empty()){

			int p_size = xlayer_seq[layerId].prev_layers_ID.size();
			for(int it=0;it<p_size;it++)
			{
				layerID previous;
				previous.pos    = xlayer_seq[layerId].prev_layers_ID[it].pos;
				previous.seqidx = xlayer_seq[layerId].prev_layers_ID[it].layerindex;
				currentLayer->previous.push_back(previous);
			}
		}
		else
		{
			layerID previous;
			previous.pos    = -1;
			previous.seqidx = -1;
			currentLayer->previous.push_back(previous);

		}
		if(!xlayer_seq[layerId].next_layer_ID.empty()){

			int p_size = xlayer_seq[layerId].next_layer_ID.size();

			for(int it=0;it<p_size;it++)
			{
				layerID next;
				next.pos    = xlayer_seq[layerId].next_layer_ID[it].pos;
				next.seqidx = xlayer_seq[layerId].next_layer_ID[it].layerindex;
				currentLayer->next.push_back(next);
			}
		}
		else
		{
			layerID next;
			next.pos    = -1;
			next.seqidx = -1;
			currentLayer->next.push_back(next);
		}

		currentLayer->layer_done.push_back(0);

		qformat qf_format;
		qf_format.ip_bw    = xlayer_seq[layerId].hw_ops->ip_bw;
		qf_format.ip_fbits = xlayer_seq[layerId].hw_ops->ip_fl;
		qf_format.op_bw    = xlayer_seq[layerId].hw_ops->op_bw;
		qf_format.op_fbits = xlayer_seq[layerId].hw_ops->op_fl;
		qf_format.wt_bw    = xlayer_seq[layerId].hw_ops->wt_bw;
		qf_format.wt_fbits = xlayer_seq[layerId].hw_ops->wt_fl;
		qf_format.bs_bw    = xlayer_seq[layerId].hw_ops->op_bw;
		qf_format.bs_fbits = xlayer_seq[layerId].hw_ops->op_fl;

#if EN_CONSOLE_TXT_INIT
		cout <<"wt_fbits:"<< qf_format.wt_fbits <<"  ip_fbits:"<< qf_format.ip_fbits <<"  op_fbits:"<< qf_format.op_fbits <<"  bs_fbits:"<< qf_format.bs_fbits <<endl;
#endif


#define SSD 0
#define GNET 0
#define XNETSSD 0
#define RESNET50 0

#if RESNET50
		qf_format.ip_fbits = 10;
		qf_format.op_fbits = 10;
		qf_format.bs_fbits = 10;
#endif  //#if GNET

#if GNET
		if(layerId == 0)
			qf_format.ip_fbits = 7;
		else
			qf_format.ip_fbits = 1;
		qf_format.op_fbits = 1;
		qf_format.bs_fbits = 1;
#endif  //#if GNET

#if SSD
		if(layerId == 0)
		{
			qf_format.ip_fbits = 7;
			qf_format.op_fbits = 1;
		}

		if((layerId > 0) && (layerId < 14))
		{
			qf_format.ip_fbits = 1;
			qf_format.op_fbits = 1;
		}

		if(layerId == 14)
		{
			qf_format.ip_fbits = 1;
			qf_format.op_fbits = 8;
		}

		if(layerId == 15)
		{
			qf_format.ip_fbits = 1;
			qf_format.op_fbits = 1;
		}

		if((layerId == 16)||(layerId == 17))  //D1 Conv
		{
			qf_format.ip_fbits = 8;
			qf_format.op_fbits = 8;
		}

		if(layerId == 18)
		{
			qf_format.ip_fbits = 1;
			qf_format.op_fbits = 1;
		}

		if((layerId == 19)||(layerId == 20))  //D1 permute
		{
			qf_format.ip_fbits = 8;
			qf_format.op_fbits = 8;
		}

		if(layerId == 21)                     //conv5_3
		{
			qf_format.ip_fbits = 1;
			qf_format.op_fbits = 7;
		}

		if(layerId == 22)                    //conv5_3  to pool
		{
			qf_format.ip_fbits = 7;
			qf_format.op_fbits = 7;
		}

		if(layerId == 23)                    //fc6 dilation
		{
			qf_format.ip_fbits = 7;
			qf_format.op_fbits = 8;
		}

		if(layerId > 23)
		{
			qf_format.ip_fbits = 8;
			qf_format.op_fbits = 8;
		}
#endif  //#if SSD

#if XNETSSD
#define BITS1 3
#define BITS2 8

		if(layerId == 0)
		{
			qf_format.ip_fbits = 7;
			qf_format.op_fbits = BITS1;
		}

		if((layerId > 0) && (layerId < 10))
		{
			qf_format.ip_fbits = BITS1;
			qf_format.op_fbits = BITS1;
		}

		if(layerId == 10)   //norm
		{
			qf_format.ip_fbits = BITS1;
			qf_format.op_fbits = BITS2;
		}

		if((layerId > 10) && (layerId < 12))
		{
			qf_format.ip_fbits = BITS1;
			qf_format.op_fbits = BITS1;
		}

		if(layerId == 12)   //norm_loc_perm
		{
			qf_format.ip_fbits = BITS2;
			qf_format.op_fbits = BITS2;
		}

		if(layerId == 13)   //norm_conf_perm
		{
			qf_format.ip_fbits = BITS2;
			qf_format.op_fbits = BITS2;
		}

		if((layerId > 13) && (layerId < 15))
		{
			qf_format.ip_fbits = BITS1;
			qf_format.op_fbits = BITS1;
		}

		if(layerId == 15)   //norm_loc_perm
		{
			qf_format.ip_fbits = BITS2;
			qf_format.op_fbits = BITS2;
		}

		if(layerId == 16)   //norm_conf_perm
		{
			qf_format.ip_fbits = BITS2;
			qf_format.op_fbits = BITS2;
		}

		if((layerId > 16) && (layerId < 20))
		{
			qf_format.ip_fbits = BITS1;
			qf_format.op_fbits = BITS1;
		}

		if(layerId == 20)   //conv4_6
		{
			qf_format.ip_fbits = BITS1;
			qf_format.op_fbits = BITS2;
		}

		if(layerId > 20)
		{
			qf_format.ip_fbits = BITS2;
			qf_format.op_fbits = BITS2;
		}
#endif  //#if SSD

		qf_format.bs_fbits = qf_format.op_fbits;

#if EN_CONSOLE_TXT_INIT
		cout <<"wt_fbits:"<< qf_format.wt_fbits <<"  ip_fbits:"<< qf_format.ip_fbits <<"  op_fbits:"<< qf_format.op_fbits <<"  bs_fbits:"<< qf_format.bs_fbits <<endl;
#endif

		currentLayer->qf_format = qf_format;

		currentLayer->base_path = file_path;

		file_paths const_data_path;
		char *base_path = currentLayer->base_path;

		string op_path = xlayer_seq[layerId].hw_ops->output_file;//-> name;
		replace(op_path.begin(), op_path.end(), '/', '_');  // Replace all / with _, otherwise fileWrite becomes an issue
		const char *path0 = op_path.c_str();

		//loading out_path, ref_path
		char *ref_path = (char *)malloc(500*sizeof(char));
		char *out_path = (char *)malloc(500*sizeof(char));

		sprintf(out_path, "%s/out/%s", base_path, path0);

#if	PACKED_INOUT
		sprintf(ref_path, "%s/hls_ref/%s", base_path, path0);
#else
		sprintf(ref_path, "%s/caffe_ref/%s", base_path, path0);
#endif

		currentLayer->ref_path = ref_path;
		currentLayer->out_path = out_path;
#if EN_CONSOLE_TXT_INIT
		cout <<"ref_path: "<< currentLayer->ref_path <<endl;
		cout <<"out_path: "<< currentLayer->out_path <<endl;
#endif

		if( xlayer_seq[layerId].hw_ops->type.compare("Convolution") == 0)
		{

			if((xlayer_seq[layerId].opcode == OPCODE_CONV) || (xlayer_seq[layerId].opcode == OPCODE_FUSE_BN_CONV))
			{
				//Weights path
				if(xlayer_seq[layerId].hw_ops->conv_params->weightsPath.empty())
				{
					cout << "weights file not found" << endl;
				}

				string wt_path = xlayer_seq[layerId].hw_ops->conv_params->weightsPath[0];
				//replace(wt_path.begin(), wt_path.end(), '/', '_');
				const char *path1 = wt_path.c_str();
				sprintf(weight_path, "%s", path1);

				//Bias path
				if(xlayer_seq[layerId].hw_ops->conv_params->biasPath.empty())
				{
					cout << "bias file not found" << endl;
				}
				string bs_path = xlayer_seq[layerId].hw_ops->conv_params->biasPath[0];
				//replace(bs_path.begin(), bs_path.end(), '/', '_');
				const char *path2 = bs_path.c_str();
				sprintf(bias_path, "%s", path2);

				const_data_path.wgt_path     = weight_path;
				const_data_path.bias_path    = bias_path;
			}

			if((xlayer_seq[layerId].opcode == OPCODE_BN) || (xlayer_seq[layerId].opcode == OPCODE_FUSE_BN_CONV))
			{
				//Gamma path
				if(xlayer_seq[layerId].hw_ops->conv_params->scale_gammaPath.empty())
				{
					cout << "Gamma file not found" << endl;
				}

				string wt_path = xlayer_seq[layerId].hw_ops->conv_params->scale_gammaPath[0];
				//replace(wt_path.begin(), wt_path.end(), '/', '_');
				const char *path1 = wt_path.c_str();
				sprintf(gamma_path, "%s", path1);

				//Beta path
				if(xlayer_seq[layerId].hw_ops->conv_params->scale_betaPath.empty())
				{
					cout << "beta file not found" << endl;
				}
				string bs_path = xlayer_seq[layerId].hw_ops->conv_params->scale_betaPath[0];
				//replace(bs_path.begin(), bs_path.end(), '/', '_');
				const char *path2 = bs_path.c_str();
				sprintf(beta_path, "%s", path2);

				//BN Mean path
				if(xlayer_seq[layerId].hw_ops->conv_params->batchnorm_meanPath.empty())
				{
					cout << "batchnorm mean file not found" << endl;
				}
				string mn_path = xlayer_seq[layerId].hw_ops->conv_params->batchnorm_meanPath[0];
				//replace(bs_path.begin(), bs_path.end(), '/', '_');
				const char *path3 = mn_path.c_str();
				sprintf(mean_path, "%s", path3);

				//BN Variance path
				if(xlayer_seq[layerId].hw_ops->conv_params->batchnorm_variancePath.empty())
				{
					cout << "batchnorm mean file not found" << endl;
				}
				string var_path = xlayer_seq[layerId].hw_ops->conv_params->batchnorm_variancePath[0];
				//replace(bs_path.begin(), bs_path.end(), '/', '_');
				const char *path4 = var_path.c_str();
				sprintf(variance_path, "%s", path4);

				const_data_path.gamma_path     = gamma_path;
				const_data_path.beta_path      = beta_path;
				const_data_path.mean_path      = mean_path;
				const_data_path.variance_path  = variance_path;
			}
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("InnerProduct") == 0)
		{
			//Weights path
			if(xlayer_seq[layerId].hw_ops->fc_params->weightsPath.empty())
			{
				cout << "weights file not found" << endl;
			}

			string wt_path = xlayer_seq[layerId].hw_ops->fc_params->weightsPath[0];
			//replace(wt_path.begin(), wt_path.end(), '/', '_');
			const char *path1 = wt_path.c_str();
			sprintf(weight_path, "%s", path1);

			//Bias path
			if(xlayer_seq[layerId].hw_ops->fc_params->biasPath.empty())
			{
				cout << "bias file not found" << endl;
			}
			string bs_path = xlayer_seq[layerId].hw_ops->fc_params->biasPath[0];
			//replace(bs_path.begin(), bs_path.end(), '/', '_');
			const char *path2 = bs_path.c_str();
			sprintf(bias_path, "%s", path2);

			const_data_path.wgt_path  = weight_path;
			const_data_path.bias_path = bias_path;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("Deconvolution") == 0)
		{
			//Weights path
			if(xlayer_seq[layerId].hw_ops->deconv_params->weightsPath.empty())
			{
				cout << "weights file not found" << endl;
			}

			string wt_path = xlayer_seq[layerId].hw_ops->deconv_params->weightsPath[0];
			//replace(wt_path.begin(), wt_path.end(), '/', '_');
			const char *path1 = wt_path.c_str();
			sprintf(weight_path, "%s", path1);

			//Bias path
			if(xlayer_seq[layerId].hw_ops->deconv_params->biasPath.empty())
			{
				cout << "bias file not found" << endl;
			}
			string bs_path = xlayer_seq[layerId].hw_ops->deconv_params->biasPath[0];
			//replace(bs_path.begin(), bs_path.end(), '/', '_');
			const char *path2 = bs_path.c_str();
			sprintf(bias_path, "%s", path2);

			const_data_path.wgt_path  = weight_path;
			const_data_path.bias_path = bias_path;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("L2Normalize") == 0)
		{
			if(xlayer_seq[layerId].hw_ops->l2norm_params->gammaFile.empty())
			{
				cout << "gamma file not found" << endl;
			}

			string wt_path = xlayer_seq[layerId].hw_ops->l2norm_params->gammaFile;//no ms_params->pboxFile;
			//replace(wt_path.begin(), wt_path.end(), '/', '_');
			const char *path1 = wt_path.c_str();
			sprintf(weight_path, "%s", path1);

			const_data_path.wgt_path  = weight_path;
			const_data_path.bias_path = bias_path;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("NMS") == 0)
		{
			if(xlayer_seq[layerId].hw_ops->nms_params->pboxFile.empty())
			{
				cout << "pbox file not found" << endl;
			}

			string wt_path = xlayer_seq[layerId].hw_ops->nms_params->pboxFile;
			//replace(wt_path.begin(), wt_path.end(), '/', '_');
			const char *path1 = wt_path.c_str();
			sprintf(weight_path, "%s", path1);

			if(xlayer_seq[layerId].hw_ops->nms_params->varFile.empty())
			{
				cout << "var file not found" << endl;
			}

			string bs_path = xlayer_seq[layerId].hw_ops->nms_params->varFile;
			//replace(bs_path.begin(), bs_path.end(), '/', '_');
			const char *path2 = bs_path.c_str();
			sprintf(bias_path, "%s", path2);

			const_data_path.wgt_path  = weight_path;
			const_data_path.bias_path = bias_path;
		}



#if EN_CONSOLE_TXT_INIT
		cout <<"weight_path: "<< weight_path <<endl;
		cout <<"bias_path: "<< bias_path <<endl;
#endif

		if( xlayer_seq[layerId].hw_ops->type.compare("Convolution") == 0)
		{
			convInit(current_seq_layer, currentLayer, layerId, &const_data_path);
			prev_layer_type = CONV;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("Pooling") == 0)
		{
			poolInit(current_seq_layer, currentLayer, layerId);
			prev_layer_type = POOL;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("InnerProduct") == 0)
		{
#if GNET
			if(fcId==0)
			{
				currentLayer->qf_format.ip_fbits = 1;
			}
			else
			{
				currentLayer->qf_format.ip_fbits = 3;
			}
			currentLayer->qf_format.op_fbits = 3;
			currentLayer->qf_format.bs_fbits = 3;
#endif
			fcInit(current_seq_layer, currentLayer, &const_data_path);
			fcId++;
			prev_layer_type = FC_LAYER;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("Softmax") == 0)
		{
#if GNET
			currentLayer->qf_format.ip_fbits = 3;
#endif
			softmaxInit(current_seq_layer, currentLayer, prev_layer_type);
			prev_layer_type = SOFTMAX;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("Deconvolution") == 0)
		{
			deconvInit(current_seq_layer, currentLayer, &const_data_path);
			prev_layer_type = DECONV;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("L2Normalize") == 0)
		{
			normInit(current_seq_layer, currentLayer, &const_data_path);
			prev_layer_type = NORM;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("Permute") == 0)
		{
			permuteInit(current_seq_layer, currentLayer);
			prev_layer_type = PERMUTE;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("NMS") == 0)
		{
			nmsInit(current_seq_layer, currentLayer, &const_data_path);
			prev_layer_type = NMS;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("Crop") == 0)
		{
			cropInit(current_seq_layer, currentLayer);
			prev_layer_type = CROP;
		}

		if(layerId==0)
		{
			loadMean(current_seq_layer, currentLayer);
		}


#if EN_CONSOLE_TXT_INIT
		qf_format = currentLayer->qf_format;
		cout <<"wt_fbits:"<< qf_format.wt_fbits <<"  ip_fbits:"<< qf_format.ip_fbits <<"  op_fbits:"<< qf_format.op_fbits <<"  bs_fbits:"<< qf_format.bs_fbits <<endl;
#endif

#if EN_CONSOLE_TXT_INIT
		int *scalar_args = (int *)lay_obj[imgId][layerId].params;
		fprintf(stderr,"\nlayerId=%d\n",layerId);
		for(int i=0;i<MAX_PARAM_SIZE;i++)
		{
			fprintf(stderr,"\t%d", scalar_args[i]);
		}
		fprintf(stderr,"\n");
#endif

#if EN_CONSOLE_TXT_INIT_DBG
		cout <<".";
#endif
	}

#if EN_CONSOLE_TXT_INIT_DBG
	cout << endl;
#endif

#if EN_CONSOLE_TXT_INIT
	cout << "Params updated for each Layer for image-0" << endl;
#endif

	//# Copying the pointers for other images
	for (int imgId = 1; imgId < NUM_IMG; imgId++)
	{
		xChangeLayer *currentLayer = &lay_obj[imgId][0];
		xChangeLayer *img0CurrentLayer = &lay_obj[0][0];

		//# Travel through all layers
		for(int layerId=0;layerId<layer_cnt;layerId++)
		{
			xChangeLayer *currentLayer = &lay_obj[imgId][layerId];
			xChangeLayer *img0CurrentLayer = &lay_obj[0][layerId];

			currentLayer->kernType  = img0CurrentLayer->kernType;
			currentLayer->qf_format = img0CurrentLayer->qf_format;

			currentLayer->out_path  = img0CurrentLayer->out_path;
			currentLayer->ref_path  = img0CurrentLayer->ref_path;
			currentLayer->resize_h  = img0CurrentLayer->resize_h;
			currentLayer->resize_w  = img0CurrentLayer->resize_w;

			currentLayer->layer_done.push_back(0);

			int *scalar_conv_args = (int *)currentLayer->params;

			if( (scalar_conv_args[34] == OPCODE_BN) || (scalar_conv_args[34] == OPCODE_FUSE_BN_CONV) )
			{

				short *bias_ptr = (short *)img0CurrentLayer->in_ptrs[4];
				currentLayer->in_ptrs[4] = bias_ptr;;
			}

			if( currentLayer->kernType == NMS)
			{
				float confidence_threshold = img0CurrentLayer->float_params[0];
				float nms_threshold        = img0CurrentLayer->float_params[1];

				currentLayer->float_params.push_back(confidence_threshold);
				currentLayer->float_params.push_back(nms_threshold);
			}

			int p_size = img0CurrentLayer->previous.size();

			for(int it=0;it<p_size;it++)
			{
				layerID previous;
				previous.pos = img0CurrentLayer->previous[it].pos;
				previous.seqidx = img0CurrentLayer->previous[it].seqidx;

				currentLayer->previous.push_back(previous);
			}

			p_size = img0CurrentLayer->next.size();

			for(int it=0;it<p_size;it++)
			{
				layerID next;
				next.pos = img0CurrentLayer->next[it].pos;
				next.seqidx = img0CurrentLayer->next[it].seqidx;

				currentLayer->next.push_back(next);
			}
		}
	}

	free(weight_path);
	free(bias_path);
	free(mean_path);
	free(variance_path);
	free(gamma_path);
	free(beta_path);

	//free(ref_path);
	//free(out_path);


#if EN_CONSOLE_TXT_INIT
	cout << "Params updated for each Layer for all images" << endl;

	cout<< "initXChangeHost : done" <<endl;
#endif

}
