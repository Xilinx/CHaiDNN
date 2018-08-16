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
#include "ap_int.h"

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
	char *out_path;
	char *wgt_path;
	char *bias_path;

	char *mean_path;
	char *gamma_path;
	char *beta_path;
	char *variance_path;
}file_paths;

//# Layers supported
enum _kernel_mode{
	OFFLINE,
	DYNAMIC_FIXED
};

void printShape(vector<int> shape)
{
#if EN_DEBUG_INIT_PRINT
	for(int i = 0; i < shape.size(); i++)
	{
		cout << "shape[i] : " << shape[i] << endl;
	}
#endif
}

void convInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer, int layer_id, file_paths *const_data_path, int &io_8bit_flag, int fc_id)
{

#if	EN_DEBUG_INIT_PRINT
	cout<<"convInit Start: "<<endl;
#endif

	ConvolutionParameter *conv_params = xlayer_seq->hw_ops->conv_params;
	int *scalar_conv_args = (int *)currentLayer->params;

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottomShape.at(0);//.blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->topShape.at(0);//.blob->shape;

	int conv_inp_height, conv_inp_width;
	int conv_out_height, conv_out_width;
	int conv_inp_planes, conv_out_planes;
	int conv_filter_height, conv_filter_width;
	int conv_stride, conv_dilation, conv_pad, conv_group;
	int th_in, th_out;

	scalar_conv_args[34] = xlayer_seq->opcode;        //Opcode

	string quant_scheme = xlayer_seq->hw_ops->quantization_scheme;

	//cout << "Quant scheme : " << xlayer_seq->hw_ops->quantization_scheme << endl;

	//# Loading first layer threshold param
	//# This is used for in offline quant mode for input quantization
	float th_layer_in = 0;
	float th_layer_out = 0;
	float sf_in = 0;
	float sf_out = 0;

	scalar_conv_args[127] = 0;
	//if((layer_id == 0) && (quant_scheme.compare("Xilinx") == 0))
	if(quant_scheme.compare("Xilinx") == 0)
	{
		th_layer_in  = xlayer_seq->hw_ops->th_layer_in;
		th_layer_out = xlayer_seq->hw_ops->th_layer_out;
		scalar_conv_args[127] = 1;  //offline mode

		//if((scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV) || (scalar_conv_args[34] == OPCODE_FC2CONV))//Checking for MAX_POOL or AVG_POOL
		if((scalar_conv_args[34] == OPCODE_3D_CONV) || (scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV) || (scalar_conv_args[34] == OPCODE_POOL_CONV2CONV) || (scalar_conv_args[34] == OPCODE_FC2CONV))
		{
			th_layer_in  = conv_params->th_layer_in_3d;//39.5895589117;
			th_layer_out = conv_params->th_layer_out_3d;//39.5895589117;
		}

		int ip_bw =  currentLayer->qf_format.ip_bw;
		sf_in = th_layer_in / (pow(2, ip_bw - 1) - 1);

		int op_bw =  currentLayer->qf_format.op_bw;
		sf_out = th_layer_out / (pow(2, op_bw - 1) - 1);

	}

	currentLayer->float_params.push_back(th_layer_in);
	currentLayer->float_params.push_back(th_layer_out);
	currentLayer->float_params.push_back(sf_in);
	currentLayer->float_params.push_back(sf_out);


	if(scalar_conv_args[34] == OPCODE_FC2CONV)
	{
		conv_inp_height = conv_params->M_3d;       //FC output size
		conv_inp_width  = 1;
		conv_out_height = conv_params->M_3d;       //FC output size
		conv_out_width  = 1;
		conv_inp_planes = conv_params->N_3d;		//FC input size
		conv_out_planes = 16;
		conv_filter_height = 1;
		conv_filter_width  = 1;
		conv_dilation = 1;
		conv_pad = 0;
		conv_group = 0;
		conv_stride = 1;
	}
	else if ( (scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV))
	{
		conv_inp_height  = bottomShape.at(2);  //input_height
		conv_inp_width   = bottomShape.at(3);   //input_width
		conv_out_height  = topShape.at(2);     //output_height
		conv_out_width   = topShape.at(3);      //output_width
		conv_out_planes  = conv_params->M_3d;     //output planes
		conv_inp_planes  = conv_params->N_3d;     //input planes
		conv_stride      = conv_params->stride_h_3d;  //stride_h
		conv_filter_height = conv_params->filter_h_3d;   //filter_h
		conv_filter_width  = conv_params->filter_w_3d;  //filter_w
		conv_pad         = conv_params->pad_h_3d;       //pad_h
		conv_group       = 0;
		conv_dilation    = 1;
	}
	else
	{
		conv_inp_height  = bottomShape.at(2);  //input_height
		conv_inp_width   = bottomShape.at(3);   //input_width
		conv_out_height  = topShape.at(2);     //output_height
		conv_out_width   = topShape.at(3);      //output_width
		conv_out_planes  = conv_params->M;     //output planes
		conv_inp_planes  = conv_params->N;     //input planes
		conv_stride      = conv_params->stride_h;  //stride_h
		conv_filter_height = conv_params->filter_h;   //filter_h
		conv_filter_width  = conv_params->filter_w;  //filter_w
		conv_pad         = conv_params->pad_h;       //pad_h
		conv_group       = conv_params->group-1;     //default group is 1
		conv_dilation    = conv_params->dilation;    //dilation
	}

	scalar_conv_args[0]  = conv_inp_height;
	scalar_conv_args[1]  = conv_inp_width;
	scalar_conv_args[2]  = conv_out_height;
	scalar_conv_args[3]  = conv_out_width;
	scalar_conv_args[4]  = conv_out_planes;
	scalar_conv_args[5]  = conv_inp_planes;
	scalar_conv_args[6]  = conv_stride;
	scalar_conv_args[7]  = conv_filter_height;
	scalar_conv_args[8]  = conv_filter_width;
	scalar_conv_args[9]  = conv_pad;         //pad_h
	scalar_conv_args[11] = conv_group;
	scalar_conv_args[29] = conv_dilation;

	scalar_conv_args[10] = conv_params->reluflag;     //relu
	scalar_conv_args[12] = layer_id;                  //layer_id

	//cout << "relu start : " << scalar_conv_args[10] << endl;

#if 0
	if(scalar_conv_args[34] == OPCODE_3D_CONV)
	{
		scalar_conv_args[34] == OPCODE_CONV;
	}
#endif

	if(scalar_conv_args[34] == OPCODE_BN)
	{
		scalar_conv_args[10] = conv_params->extra_reluflag;
	}

	if( (scalar_conv_args[34] == OPCODE_BN) || (scalar_conv_args[34] == OPCODE_ELTWISE) )
	{
		//scalar_conv_args[10] = conv_params->extra_reluflag;
		scalar_conv_args[6] = 1;
		scalar_conv_args[7] = 1;
		scalar_conv_args[8] = 1;
	}

	if((scalar_conv_args[34] == OPCODE_3D_CONV) || (scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV))
	{
		scalar_conv_args[43] = scalar_conv_args[0];  //input height
		scalar_conv_args[44] = scalar_conv_args[1];  //input width
	}

	if((scalar_conv_args[34] == OPCODE_3D_CONV))
	{
		vector<int> intermediateShape =  xlayer_seq->hw_ops->intermediateShape.at(0);//.blob->shape;
		scalar_conv_args[0]  = intermediateShape.at(2);  //intermediate input_height
		scalar_conv_args[1]  = intermediateShape.at(3);  //intermediate input_width

		scalar_conv_args[45] = scalar_conv_args[0];  //output height
		scalar_conv_args[46] = scalar_conv_args[1];  //output width
	}

	if((scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV))
	{
		scalar_conv_args[45] = scalar_conv_args[2];  //output height
		scalar_conv_args[46] = scalar_conv_args[3];  //output width
	}

	if((scalar_conv_args[34] == OPCODE_3D_CONV) || (scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV) || (scalar_conv_args[34] == OPCODE_POOL_CONV2CONV))
	{
		scalar_conv_args[47] = conv_params->filter_h_3d;
		scalar_conv_args[48] = conv_params->filter_w_3d;
		scalar_conv_args[49] = conv_params->filter_h_3d*conv_params->filter_w_3d;//-fsz2;   //???
		scalar_conv_args[50] = conv_params->N_3d;    //input plane
		scalar_conv_args[51] = conv_params->M_3d;    //output plane
		scalar_conv_args[52] = conv_params->pad_h_3d;
		scalar_conv_args[53] = conv_params->stride_h_3d;
		scalar_conv_args[54] = conv_params->reluflag_3d;
		/*
		scalar_conv_args[55] = 0;                    //input offset
		//scalar_conv_args[55] = ((conv_params->pad_h_3d/conv_params->stride_h_3d)*scalar_conv_args[44]) + 1;  //((3d_pad/3d_stride)*3d_ip_w) + 1
		int conv_3d_req_rows = (conv_params->filter_h_3d-conv_params->stride_h_3d-conv_params->pad_h_3d);
		if(conv_3d_req_rows < 0)
		{
			conv_3d_req_rows = 0;
		}
		scalar_conv_args[55] = (conv_3d_req_rows*scalar_conv_args[44]) + conv_params->pad_h_3d;  //((3d_pad/3d_stride)*3d_ip_w) + 1
		*/
		
		scalar_conv_args[55] = ((scalar_conv_args[52] + scalar_conv_args[53] -1)/scalar_conv_args[53])*(scalar_conv_args[44] + 1);

		//scalar_conv_args[56] = 0;                    //istg_row_cnt

		//int io_precision = currentLayer->qf_format.ip_fbits+currentLayer->qf_format.wt_fbits-currentLayer->qf_format.op_fbits;

		int multiplier_fl;

		if( (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV))
		{
			multiplier_fl = 8;
		}
		else
			multiplier_fl = conv_params->wt_fl_3d;

		int io_precision = conv_params->ip_fl_3d+multiplier_fl-conv_params->op_fl_3d;

		//fprintf(stderr, "io_precision : %d multiplier_fl : %d\n", io_precision, multiplier_fl);

		// TODO :: Anusha
		if(quant_scheme.compare("Xilinx") == 0)
		{
			// TODO: Hardcoded for googlenet, has to be modified
			scalar_conv_args[57] = 16;//15;//io_precision;
		}
		else
			scalar_conv_args[57] = io_precision;

		scalar_conv_args[42] = 1<< (io_precision - 1);   //rounding

		scalar_conv_args[59] = scalar_conv_args[43]*scalar_conv_args[44];                    //inp_h*inp_w
	}
	else
	{
		//Default 3D conv params
		scalar_conv_args[43] = scalar_conv_args[0];  	//input height
		scalar_conv_args[44] = scalar_conv_args[1];  	//input width
		scalar_conv_args[45] = scalar_conv_args[2];  	//output height
		scalar_conv_args[46] = scalar_conv_args[3];  	//output width
		//scalar_conv_args[47] = 1;  						//filter height
		scalar_conv_args[47] = 1+conv_pad;//conv_params->pad_h;
		scalar_conv_args[48] = 1;					    //filter width
		scalar_conv_args[49] = 1;                       //fsz2
		scalar_conv_args[50] = conv_inp_planes;//conv_params->N;    		//input plane
		scalar_conv_args[51] = conv_out_planes;//conv_params->M;    		//output plane

		scalar_conv_args[52] = conv_pad;//conv_params->pad_h;
		scalar_conv_args[53] = 1;                       //stride_h_3d

		//# For normal Conv - Loading the extra out ptrs with dummy values
		//# This is to avoid HW exception
		//currentLayer->out_ptrs[2] = currentLayer->out_ptrs[0];
		//currentLayer->out_ptrs[3] = currentLayer->out_ptrs[1];


		//scalar_conv_args[56] = scalar_conv_args[14];    //conv3d_stg_row_cnt
		scalar_conv_args[59] = scalar_conv_args[0]*scalar_conv_args[1];                    //inp_h*inp_w
	}

	if((scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV))
	{
		scalar_conv_args[5]  = scalar_conv_args[50];
		scalar_conv_args[6]  = scalar_conv_args[53];
	}

	if(scalar_conv_args[34] == OPCODE_AVRPOOL2CONV)
	{
			//printf("stop");
	}

	if( (scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV) )
	{
		int inp_planes = conv_inp_planes;
		//if(inp_planes > 400)
			//printf("stop");

		int out_planes = conv_out_planes;

		int op_planes_split_cnt = 1;
		int output_planes_split = 1;

		do{
			stgRowCount(scalar_conv_args);  //update 56-conv3d_stg_row_cnt, 58-conv3d_ostg_row_cnt, 14-istg_row_cnt, 15-ostgRowCount params

			if(scalar_conv_args[15] <= 0)  //ostgrowcnt
			{
				inp_planes = inp_planes/2;
				out_planes = out_planes/2;
				inp_planes = AlignSize(inp_planes, 32);
				out_planes = AlignSize(out_planes, 32);
				scalar_conv_args[4] = inp_planes;
				scalar_conv_args[5] = out_planes;
				scalar_conv_args[50] = inp_planes;    //input plane
				scalar_conv_args[51] = out_planes;    //output plane
				op_planes_split_cnt++;
			}
			else
				output_planes_split = 0;
		}while(output_planes_split==1);

		if(op_planes_split_cnt>0)
		{
			scalar_conv_args[113] = AlignSize((conv_inp_planes-(inp_planes*(op_planes_split_cnt-1))),32);
			scalar_conv_args[114] = AlignSize((conv_out_planes-(out_planes*(op_planes_split_cnt-1))),32);
#if !SINGLE_IO_PORT
			inp_planes = inp_planes/2;
			out_planes = out_planes/2;
#endif
			scalar_conv_args[110] = pow(2,op_planes_split_cnt-1);
			scalar_conv_args[111] = inp_planes * conv_inp_width * conv_inp_height * XBATCH_SIZE;
			scalar_conv_args[112] = out_planes * conv_out_width * conv_out_height * XBATCH_SIZE;

			stgRowCount_poolsplit(scalar_conv_args);  //update 115-conv3d_stg_row_cnt, 116-conv3d_ostg_row_cnt, 117-istg_row_cnt, 118-ostgRowCount params
			scalar_conv_args[109] = conv_out_planes;
#if 0
	for(int i=110;i<119;i++)
		fprintf(stderr,"\t%d",(int)scalar_conv_args[i]);
#endif
		}
	}
	else
	{
		stgRowCount(scalar_conv_args);  //update 56-conv3d_stg_row_cnt, 58-conv3d_ostg_row_cnt, 14-istg_row_cnt, 15-ostgRowCount params
		scalar_conv_args[110] = 1;
	}

	if(scalar_conv_args[34] != OPCODE_3D_CONV)
	{
		scalar_conv_args[56] = scalar_conv_args[14];    //conv3d_stg_row_cnt
	}

	straddleFactorCount(scalar_conv_args);  //update 16-compute_planes, 17-straddle_factor

	nkpfCount(scalar_conv_args);  //13-nkpf, 19-slk

	int group_flag = scalar_conv_args[11];

	int io_precision = currentLayer->qf_format.ip_fbits+currentLayer->qf_format.wt_fbits-currentLayer->qf_format.op_fbits;

	//# Fixed the shift for offline quant mode
	if(quant_scheme.compare("Xilinx") == 0)
	{
		io_precision = QUANT_PREC_SHIFT;
		scalar_conv_args[20] = QUANT_PREC_SHIFT;
	}
	else
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

	//fprintf(stderr, "seq ip_bw=%d\n", xlayer_seq[0].hw_ops->ip_bw);
	//fprintf(stderr, "seq ip_3d_bw=%d\n", conv_params->ip_bw_3d);

	//# int6 input flag
	//# TODO: Fix this
	if(currentLayer->qf_format.ip_bw == 6)
		scalar_conv_args[39] = 1;
	else
		scalar_conv_args[39] = 0;

	//# int6 output flag
	//# TODO: Fix this
	if(currentLayer->qf_format.ip_bw == 6)
		scalar_conv_args[40] = 1;
	else
		scalar_conv_args[40] = 0;


	ap_uint<24> overflow_pattern;
	//# overflowPattern
	//overflowPattern(io_precision, scalar_conv_args[40], overflow_pattern);
	//scalar_conv_args[41] = overflow_pattern;

	scalar_conv_args[41] = 1 << (io_precision - 1);   // conv rounding //TODO CHECKING Functionality with ROUND: ANITHA
	//scalar_conv_args[43] = scalar_conv_args[41] - 1;
	//scalar_conv_args[44] = l_rounding_bn - 1;


#if 0//EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
		fprintf(stderr,"\t%d",(int)scalar_conv_args[i]);
	//	fprintf(stderr,"\n--------------in_height:plane = %d:%d \tout_height:plane = %d:%d\t in_stgcnt = %d \tout_stgcnt = %d ",l_input_height_ffa0 ,l_input_planes_ffa0, l_output_height_ffa0, l_output_planes_ffa0,l_istg_row_cnt_ffa0, l_ostg_row_cnt_ffa0);
#endif

	/* Loading Constant Data */

	char *base_path = currentLayer->base_path;

	if( (scalar_conv_args[34] == OPCODE_CONV) || (scalar_conv_args[34] == OPCODE_FUSE_BN_CONV) || (scalar_conv_args[34] == OPCODE_CRELU) || (scalar_conv_args[34] == OPCODE_3D_CONV))
	{
		//Loading Weights
		int wt_bw = xlayer_seq->hw_ops->wt_bw;
		int wt_fl = xlayer_seq->hw_ops->wt_fl;

		const float *weights_vec = (const float *)&(conv_params->weights[0][0]);
		const float *weights_th_vec;


		if(quant_scheme.compare("Xilinx") == 0)
			weights_th_vec = (const float *)&(xlayer_seq->hw_ops->th_params[0]);

		loadConvWgtsTXT(currentLayer, weights_vec, weights_th_vec, scalar_conv_args, wt_bw, wt_fl, layer_id, const_data_path->wgt_path, xlayer_seq->hw_ops->quantization_scheme);

		//Loading Bias
		int bs_bw;
		int bs_fl;
		HCONV_BIAS_TYPE *bias_ptr = (HCONV_BIAS_TYPE *)currentLayer->bias_ptr;
		if(quant_scheme.compare("Xilinx") == 0)
		{
			bs_bw = xlayer_seq->hw_ops->op_bw;
			bs_fl = xlayer_seq->hw_ops->op_fl;
		}
		else
		{
			bs_bw = 16;
			bs_fl = xlayer_seq->hw_ops->ip_fl+xlayer_seq->hw_ops->wt_fl;
		}

		int in_bw = xlayer_seq->hw_ops->ip_bw;
		int op_bw = xlayer_seq->hw_ops->op_bw;
		int op_fl = xlayer_seq->hw_ops->op_fl;

		const float *bias_vec = (const float *)&(conv_params->bias[0][0]);
		float th_out = 0;
		float th_in = 0;

		if(quant_scheme.compare("Xilinx") == 0)
		{
			th_out = xlayer_seq->hw_ops->th_layer_out;
			th_in = xlayer_seq->hw_ops->th_layer_in;
			//loadScaleFactor(bias_ptr, weights_th_vec, th_out, th_in);
		}

		loadConvBiasTXT(bias_ptr, bias_vec, weights_th_vec, th_out, th_in, conv_out_planes, layer_id, currentLayer->qf_format.bs_fbits, in_bw, op_bw, op_fl,
				wt_bw, bs_bw, bs_fl, const_data_path->bias_path, quant_scheme);

	}

	if( (scalar_conv_args[34] == OPCODE_BN) || (scalar_conv_args[34] == OPCODE_FUSE_BN_CONV) )
	{
#if 0
		if(scalar_conv_args[34] == OPCODE_FUSE_BN_CONV)
		{
			scalar_conv_args[34] = OPCODE_BN;
			scalar_conv_args[10] = 0;
		}
#endif

		scalar_conv_args[35] = 1<<currentLayer->qf_format.ip_fbits;

		HMEAN_TYPE *bias_ptr = (HMEAN_TYPE *)currentLayer->xtra_ptrs[0];//in_ptrs[3];//bias_ptr;
		currentLayer->in_ptrs[4] = bias_ptr;
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

		//# overflowPattern for batch norm
		overflowPattern(norm_prec, scalar_conv_args[40], overflow_pattern);
		scalar_conv_args[42] = overflow_pattern;    //TODO:FOR LRN we need to update



		mean_fbits = ip_fbits;


		gamma_fbits = xlayer_seq->hw_ops->scale_gamma_fl + 8;
		beta_fbits = xlayer_seq->hw_ops->scale_beta_fl+8;
		gamma_by_std_fbits = xlayer_seq->hw_ops->scale_gamma_by_std_fl+8;

		norm_prec = ip_fbits+8+gamma_by_std_fbits-op_fbits;
		norm_prec_3 = ip_fbits+8+gamma_by_std_fbits-beta_fbits;
		scalar_conv_args[36] = norm_prec;
		scalar_conv_args[37] = 0;
		scalar_conv_args[38] = norm_prec_3;

		//# overflowPattern for batch norm
		//overflowPattern(norm_prec, scalar_conv_args[40], overflow_pattern);
		//scalar_conv_args[42] = overflow_pattern;    //TODO:FOR LRN we need to update

		mean_fbits = ip_fbits + 8;

		const float *mean_vec     = (const float *)&(conv_params->batchnorm_mean[0]);
		const float *variance_vec = (const float *)&(conv_params->batchnorm_variance[0]);
		const float *gamma_vec    = (const float *)&(conv_params->scale_gamma[0]);
		const float *beta_vec     = (const float *)&(conv_params->scale_beta[0]);

		load_mean_gamma_beta(bias_ptr, mean_vec, variance_vec, gamma_vec, beta_vec,
				nElems, epsilon, mean_fbits, gamma_by_std_fbits, beta_fbits, const_data_path->wgt_path);

	}

	if(scalar_conv_args[34] == OPCODE_FC2CONV)
	{
		//Loading Weights
		HFC_WGT_TYPE *wt_ptr0 = (HFC_WGT_TYPE *)currentLayer->in_ptrs[0];
		HFC_WGT_TYPE *wt_ptr1 = (HFC_WGT_TYPE *)currentLayer->in_ptrs[1];

		int wt_bw = conv_params->wt_bw_3d;;//xlayer_seq->hw_ops->wt_bw;
		int wt_fl = conv_params->wt_fl_3d;//xlayer_seq->hw_ops->wt_fl;

		conv_inp_height = conv_params->M_3d;       //FC output size
		conv_inp_planes = conv_params->N_3d;		//FC input size

		const float *weights_vec = (const float *)&(conv_params->weights_3d[0][0]);
		const float *weights_th_vec;

		string quant_scheme = xlayer_seq->hw_ops->quantization_scheme;

		if(quant_scheme.compare("Xilinx") == 0)
			weights_th_vec = (const float *)&(xlayer_seq->hw_ops->th_params[0]);

		loadfcconvWgts<HFC_WGT_TYPE>(wt_ptr0, wt_ptr1, weights_vec, weights_th_vec, conv_inp_height, conv_inp_planes, wt_bw, wt_fl, quant_scheme);

		//Loading Bias
		HFC_BIAS_TYPE *bias_ptr = (HFC_BIAS_TYPE *)currentLayer->bias_ptr;

		int bs_bw = xlayer_seq->hw_ops->op_bw;
		int bs_fl = xlayer_seq->hw_ops->op_fl;

		int ip_bw = xlayer_seq->hw_ops->ip_bw;
		int op_bw = xlayer_seq->hw_ops->op_bw;

		float th_out = xlayer_seq->hw_ops->th_layer_out;
		float th_in = xlayer_seq->hw_ops->th_layer_in;

		const float *bias_vec = (const float *)&(conv_params->bias_3d[0][0]);

		loadfcconvBiasTXT<HFC_BIAS_TYPE>(bias_ptr, bias_vec, weights_th_vec, th_out, th_in, conv_inp_height, ip_bw, op_bw, wt_bw, bs_bw, bs_fl, quant_scheme);
	}

	//if((quant_scheme.compare("Xilinx") == 0) && (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV))
	if(scalar_conv_args[34] == OPCODE_AVRPOOL2CONV)
	{
		//Loading Weights
		short *out_ptr0 = (short *)currentLayer->xtra_ptrs[0];
		short *out_ptr1 = (short *)currentLayer->xtra_ptrs[1];

		//# Used by kernel
		currentLayer->in_ptrs[2] = currentLayer->xtra_ptrs[0];
		currentLayer->bias_ptr = currentLayer->xtra_ptrs[1];

		float th_out = xlayer_seq->hw_ops->th_layer_out;
		float th_in = xlayer_seq->hw_ops->th_layer_in;
		int offline_quant_mode = 1;

		if(quant_scheme.compare("DynamicFixed") == 0)
		{
			th_out = 1;
			th_in  = 1;
			offline_quant_mode = 0;
		}
		//int ip_bw = xlayer_seq->hw_ops->ip_bw;
		//int op_bw = xlayer_seq->hw_ops->op_bw;

		int ip_bw = conv_params->ip_bw_3d;
		int op_bw = conv_params->op_bw_3d;

		loadsf_avg_pool(out_ptr0, out_ptr1, th_out, th_in, ip_bw, op_bw, scalar_conv_args[51], scalar_conv_args[7], offline_quant_mode);
	}

	if(scalar_conv_args[34] == OPCODE_ELTWISE)
	{
		scalar_conv_args[35] = 0; //mul_in;
		scalar_conv_args[36] = 8; //norm_prec
		scalar_conv_args[37] = 8; //norm_prec_2
		scalar_conv_args[38] = 0; //norm_prec_3
		scalar_conv_args[42] = 0;
	}

	if(scalar_conv_args[34] == OPCODE_3D_CONV)
	{
		//Loading Weights
		char *wt_ptr0 = (char *)currentLayer->xtra_ptrs[0];
		char *wt_ptr1 = (char *)currentLayer->xtra_ptrs[1];
		currentLayer->in_ptrs[3] = wt_ptr0;
		currentLayer->in_ptrs[4] = wt_ptr1;

		const float *weight_vec     = (const float *)&(xlayer_seq->hw_ops->conv_params->weights_3d[0][0]);
		//loadSeperableConvWgtsTXT(wt_ptr0, weight_vec, scalar_conv_args, currentLayer->qf_format.wt_fbits);
		load3dSeperableConvWgts(wt_ptr0, wt_ptr1, weight_vec, scalar_conv_args, conv_params->wt_bw_3d, conv_params->wt_fl_3d, const_data_path->wgt_path);

		//Loading Bias
		//# 3D-sep Conv bias
		IO_DATA_TYPE *bs_ptr0 = (IO_DATA_TYPE *)currentLayer->xtra_ptrs[2];
		IO_DATA_TYPE *bs_ptr1 = (IO_DATA_TYPE *)currentLayer->xtra_ptrs[3];
		currentLayer->out_ptrs[2] = bs_ptr0;
		currentLayer->out_ptrs[3] = bs_ptr1;

		const float *bias_vec     = (const float *)&(xlayer_seq->hw_ops->conv_params->bias_3d[0][0]);
		//loadSeperableConvWgtsTXT(wt_ptr0, weight_vec, scalar_conv_args, currentLayer->qf_format.wt_fbits);
		load3dSeperableConvBias(bs_ptr0, bs_ptr1, bias_vec, scalar_conv_args[51], layer_id, conv_params->op_bw_3d, conv_params->op_fl_3d, const_data_path->bias_path);
	}

	/* group care for input and output planes */
	int inDepth    = scalar_conv_args[5];
	int outDepth   = scalar_conv_args[4];
	if((group_flag) && (inDepth > 4))
	{
		scalar_conv_args[5] = scalar_conv_args[5]/2;
		scalar_conv_args[50] = scalar_conv_args[50]/2;
	}

	if((group_flag) && (outDepth > 8))
	{
		scalar_conv_args[4] = scalar_conv_args[4]/2;
		scalar_conv_args[51] = scalar_conv_args[51]/2;
	}

	//scalar_conv_args[11] = 0;              //making group as zero for the first group


	//scalar_conv_args[58] = currentLayer->en_batch_size_one; //BATCH_SIZE_ONE_ENABLE


	//TODO:Extra Relu flag

#if 1
	///*************************************Scalar arguments from 60 to 94 for DSP optimization *************************************

	int pixProc;
	int input_height = scalar_conv_args[0];  	//input height
	int input_width  = scalar_conv_args[1];  	//input width
	int output_height = scalar_conv_args[2];  	//output height
	int output_width  = scalar_conv_args[3];  	//output width
	int filter_height = scalar_conv_args[7];//conv_params->filter_h;  //filter_h
	int filter_width  = scalar_conv_args[8];//conv_params->filter_w;  //filter_w
	int dilation_factor = scalar_conv_args[29]; //dilation
	int stride        = scalar_conv_args[6];    //stride_h

	if((scalar_conv_args[39] == 1))// || (scalar_conv_args[58]==1))
		pixProc = XI_PIX_PROC;
	else
		pixProc = XI_PIX_PROC/2;


	scalar_conv_args[60] = AlignSize(scalar_conv_args[5], 4);  // input_planes_align4
	scalar_conv_args[61] = AlignSize(scalar_conv_args[16], 4); // compute_planes_align4
	scalar_conv_args[62] = AlignSize(scalar_conv_args[4], 16); // output_planes_align16
	scalar_conv_args[63] = AlignSize(scalar_conv_args[5], 16); // input_planes_align16
	scalar_conv_args[64] = input_height * input_width;
	scalar_conv_args[65] = output_height * output_width;
	scalar_conv_args[66] = filter_height * filter_width;
	scalar_conv_args[67] = (dilation_factor * (filter_height - 1)) + 1;//filter height for dilated conv
	scalar_conv_args[68] = (dilation_factor * (filter_width - 1)) + 1; //filter width for dilated conv

	scalar_conv_args[70] = scalar_conv_args[15] * scalar_conv_args[3]; // ostg_row_count x output_width
	scalar_conv_args[71] = scalar_conv_args[68] / stride;              //dilated_filter_width / stride
	scalar_conv_args[72] = scalar_conv_args[68] % stride;              //dilated_filter_width % stride
	scalar_conv_args[73] = pixProc / output_width;
	scalar_conv_args[74] = pixProc % output_width;
	scalar_conv_args[75] = (pixProc/2) / output_width;
	scalar_conv_args[76] = (pixProc/2) % output_width;
	scalar_conv_args[77] = filter_height * filter_width * (scalar_conv_args[61]/4); //compute_loop_count



	int input_group_offset_1st, input_group_offset_other, weights_group_offset, output_group_offset, bias_group_offset;
	bool group_enable = scalar_conv_args[11];

	mem_offset_group(input_height, input_width, scalar_conv_args[63], output_height, output_width, scalar_conv_args[62],
			filter_height, filter_width, group_enable,
			&input_group_offset_1st, &input_group_offset_other, &weights_group_offset, &output_group_offset, &bias_group_offset);

	scalar_conv_args[78] = input_group_offset_1st;
	scalar_conv_args[79] = input_group_offset_other;
	scalar_conv_args[80] = weights_group_offset;
	scalar_conv_args[81] = output_group_offset;
	scalar_conv_args[82] = bias_group_offset;

	int alignInputPlane;
	if(XBATCH_SIZE==1)   //batch size 1
	{
		alignInputPlane  = AlignSize(scalar_conv_args[5], 32);
	}
	else
	{
		alignInputPlane  = AlignSize(scalar_conv_args[5], 16);
	}

	int alignOutputPlane;
	if(XBATCH_SIZE==1)   //batch size 1
	{
		alignOutputPlane  = AlignSize(scalar_conv_args[4], 32);
	}
	else
	{
		alignOutputPlane  = AlignSize(scalar_conv_args[4], 16);
	}

	if(scalar_conv_args[34] == OPCODE_3D_CONV)
	{
		scalar_conv_args[69] = scalar_conv_args[56] * scalar_conv_args[44]; // 3d_istg_row_count x input_width
		scalar_conv_args[83] = scalar_conv_args[14]*scalar_conv_args[53]; //3d_ostage_row_count * 3d_stride
		if(XBATCH_SIZE==1)   //batch size 1
		{
			alignInputPlane  = AlignSize(scalar_conv_args[50], 32);
		}
		else
		{
			alignInputPlane  = AlignSize(scalar_conv_args[50], 16);
		}
		scalar_conv_args[84] = scalar_conv_args[43]*scalar_conv_args[44]*(alignInputPlane/16); //3d_input_height * 3d_input_width * 3d_input_plane_align16

	}
	else
	{
		scalar_conv_args[69] = scalar_conv_args[14] * scalar_conv_args[1]; // istg_row_count x input_width
		scalar_conv_args[83] = scalar_conv_args[15]*stride; //ostage_row_count * stride
		scalar_conv_args[84] = scalar_conv_args[64]*(alignInputPlane/16); //input_height * input_width * input_plane_align16
	}

	scalar_conv_args[85] = scalar_conv_args[65]*(alignOutputPlane/16); //output_height * output_width * output_plane_align16

	//scalar_conv_args[86] = (XI_ROW8*8 + XI_ROW4*4 + XI_ROW2*2)*input_width;               //Layer1: input_rows_per_bram * input_width
	//scalar_conv_args[86] = (2)*input_width;               //Layer1: input_rows_per_bram * input_width

#if XI_ROW2==1
	int inputRows_inBRAM = 2;
#elif XI_ROW4==1
	int inputRows_inBRAM = 4;
#elif XI_ROW8==1
	int inputRows_inBRAM = 8;
#endif
	scalar_conv_args[86] = inputRows_inBRAM*input_width;

	//scalar_conv_args[87] = output_height * output_width * (scalar_conv_args[62]/16);  //crelu_out_offset
	scalar_conv_args[87]  = scalar_conv_args[45] * scalar_conv_args[46];//3d_conv_output_height * 3d_conv_output_width   //output size


	if(output_height % scalar_conv_args[15] == 0)
		scalar_conv_args[88] = scalar_conv_args[15];//ostageRow_cnt last iteration
	else
		scalar_conv_args[88] = (output_height%scalar_conv_args[15]);//ostageRow_cnt last iteration
	scalar_conv_args[89] = scalar_conv_args[88]*scalar_conv_args[3]; //ostgRow_last_iteration x width

	int total_pixel_fc0 = scalar_conv_args[15]*output_width;
	int pix_per_kp = pixProc;
	int pcmf_off1_fc0 = (total_pixel_fc0/pix_per_kp) +1;
	int pcmf_m1=(scalar_conv_args[17]-1);
	int pcmf_off_fc0 = pcmf_off1_fc0*pcmf_m1;
	int pc_loop_mul_fc0=pix_per_kp * pcmf_off_fc0;
	int pc_loop_bound = total_pixel_fc0 + pc_loop_mul_fc0;
	int Ltotal_pixel_fc0 = scalar_conv_args[88]*output_width;
	int Lpix_per_kp;
	int Lpcmf_off1_fc0 = (Ltotal_pixel_fc0/pix_per_kp) +1;
	int Lpcmf_m1=(scalar_conv_args[17]-1);
	int Lpcmf_off_fc0 = Lpcmf_off1_fc0*Lpcmf_m1;
	int Lpc_loop_mul_fc0=pix_per_kp * Lpcmf_off_fc0;
	int Last_itr_pc_loop_bound = Ltotal_pixel_fc0 + Lpc_loop_mul_fc0;

	if((scalar_conv_args[15] * output_width) <= pix_per_kp){
		pc_loop_bound = scalar_conv_args[17]*pix_per_kp;
		Last_itr_pc_loop_bound = scalar_conv_args[17]*pix_per_kp;
	}

	scalar_conv_args[90] = pc_loop_bound;
	scalar_conv_args[91] = Last_itr_pc_loop_bound;
	scalar_conv_args[92] =  scalar_conv_args[13] * filter_height * filter_width * (scalar_conv_args[61]/4);//wts_loop_count          = nkpf x f_h x f_w x compute plane / 4
	scalar_conv_args[93] =  filter_height * filter_width * (scalar_conv_args[62]/XI_KER_PROC);                   //wts_offset_scale          = fsz^2 x outplane / 16
	scalar_conv_args[94] =  scalar_conv_args[71] * stride;
	scalar_conv_args[95] =  scalar_conv_args[75] * output_width;;

	int outplanes_align16 = scalar_conv_args[62]/32;
	int rem_planes = scalar_conv_args[62] - outplanes_align16*32;
	int crelu_planes0 =  (scalar_conv_args[62]/32) + 1;
	int crelu_planes1 =  (scalar_conv_args[62]/32) + 1;
	if(rem_planes == 16)
		crelu_planes1 = crelu_planes1 - 1;

	if(rem_planes == 0)
	{
		crelu_planes0 = crelu_planes0 - 1;
		crelu_planes1 = crelu_planes1 - 1;
	}

	scalar_conv_args[96] = output_height * output_width * crelu_planes0;  //crelu_out_offset0
	scalar_conv_args[97] = output_height * output_width * crelu_planes1;  //crelu out_offset1

	if(XBATCH_SIZE==2)   //checking for multiples of 16 for batch 2
	{
		scalar_conv_args[96] = output_height * output_width * (scalar_conv_args[62]/16);  //crelu_out_offset0
		scalar_conv_args[97] = output_height * output_width * (scalar_conv_args[62]/16);  //crelu out_offset1
	}

	scalar_conv_args[11] = 0;              //making group as zero for the first group

	/* when group=2 */
	scalar_conv_args[78] = 0;
	scalar_conv_args[79] = 0;
	scalar_conv_args[80] = 0;
	scalar_conv_args[81] = 0;
	scalar_conv_args[82] = 0;

	/* args for dsp-opt code changes */
	int lrn_inter_sos_enable;
	if (scalar_conv_args[34] == 7)  //OPCODE_LRN_INTER_SOS = 7
		lrn_inter_sos_enable = 1;
	else
		lrn_inter_sos_enable = 0;

	int filter_size_dilated;
	int filter_stride;
	int pad_num;
	if (lrn_inter_sos_enable == 1)
	{
		filter_size_dilated = 1;
		filter_stride = 1;
		pad_num = 0;
	}
	else
	{
		filter_size_dilated = scalar_conv_args[67];//l_filter_height_dil_ffa0;
		filter_stride = scalar_conv_args[6];//(ap_uint<4> ) (l_conv_stride_ffa0);
		pad_num = scalar_conv_args[9];//(ap_uint<8> ) l_pad_ffa0;
	}

	scalar_conv_args[98] = -scalar_conv_args[52];  //-conv_params->pad_h_3d;
	//int endrow_fb0 = startrow_inc_fb0 + (weight_desc.filter_size_dilated - weight_desc.filter_stride - conv_desc.pad_num -1)*(conv_desc.conv3d_stride)+(conv_desc.conv3d_ft_h - conv_desc.conv3d_pad - 1);
	//int endrow_fb0 = scalar_conv_args[83] + (filter_size_dilated - filter_stride - pad_num -1) * (scalar_conv_args[53])+(scalar_conv_args[47] - scalar_conv_args[52] - 1);
        int endrow_fb0;
	if(scalar_conv_args[34] != 20)
	endrow_fb0 = (scalar_conv_args[15]*stride + filter_size_dilated - filter_stride - pad_num -1) * (scalar_conv_args[53])+(scalar_conv_args[47] - scalar_conv_args[52] - 1);
	else
	endrow_fb0 = ((scalar_conv_args[58] -1)*scalar_conv_args[53]) + scalar_conv_args[47] - scalar_conv_args[52] -1 ;
	//endrow = (pool_ostg_row_cnt -1) * (pool_stride) + pool_ft_h - pool_pad - 1


	scalar_conv_args[99] = endrow_fb0;

	//# Added for avg pool
	//# Pool Pad : scalar_conv_args[53]

	input_width = scalar_conv_args[44];
	output_width = scalar_conv_args[46];

	short eff_w;
	short op_width = ((input_width +2*conv_pad- filter_width)/scalar_conv_args[53])+1;

	if(op_width < output_width)
		eff_w = input_width + (output_width - op_width);
	else
		eff_w = input_width;

	scalar_conv_args[101] = eff_w;


	/* Average pool param */
	float f_val = (float)1/(conv_filter_height * conv_filter_width);
	ival = (char)(f_val);
	fxval = ConvertToFP( f_val, ival, 8);

	if(quant_scheme.compare("Xilinx") == 0)
	{
		scalar_conv_args[100] = 1;
	}
	else
		scalar_conv_args[100] = fxval;

	int offset_size;
	int align_factor = ALIGN_FACTOR(HCONV_BIAS_PORT_WIDTH, HCONV_BIAS_DATA_WIDTH);
	int t_group_size, sf_grp_size, sf_align_size;

	if(quant_scheme.compare("Xilinx") == 0)
	{
		scalar_conv_args[103] = 1;

		if(scalar_conv_args[34] == OPCODE_FC2CONV)
		{
			offset_size = scalar_conv_args[2];
		}
		else
			offset_size = scalar_conv_args[4];

		if(scalar_conv_args[30])
		{
			t_group_size = 2*offset_size;
		}
		else
			t_group_size = offset_size;

		int t_size = AlignSize(t_group_size, align_factor);
		scalar_conv_args[102] = t_size/align_factor;
	}
	else
	{
		scalar_conv_args[103] = 0;
		scalar_conv_args[102] = 0;
	}

	//scalar_conv_args[102] = AlignSize(scalar_conv_args[4], ALIGN_FACTOR(HCONV_BIAS_PORT_WIDTH, HCONV_BIAS_DATA_WIDTH));


	scalar_conv_args[120] = input_group_offset_1st;
	scalar_conv_args[121] = input_group_offset_other;
	scalar_conv_args[122] = weights_group_offset;
	scalar_conv_args[123] = output_group_offset;
	scalar_conv_args[124] = bias_group_offset;
	//# Scale factor offset for group
	int t_size = AlignSize(offset_size, align_factor);
	scalar_conv_args[125] = (t_size/align_factor) + (t_size/2);

	scalar_conv_args[126] = currentLayer->en_batch_size_one;

	//# Used for reset in grouping case
	scalar_conv_args[119] = scalar_conv_args[102];


	//	cout << "relu end : " << scalar_conv_args[10] << endl;
	if(scalar_conv_args[34] == OPCODE_FC2CONV)
	{
		scalar_conv_args[125] = fc_id;
	}

	currentLayer->kernType = CONV;
	if(quant_scheme.compare("Xilinx") == 0)
	{
		currentLayer->kernMode = OFFLINE;
	}
	else
	{
		currentLayer->kernMode = DYNAMIC_FIXED;
	}


#endif

#if	EN_DEBUG_INIT_PRINT
	cout<<"convInit End: "<<endl;
#endif
}

void poolInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer, int layer_id, file_paths *const_data_path)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"poolInit Start : "<<endl;
#endif

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottomShape.at(0);//.blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->topShape.at(0);//.blob->shape;

	int *scalar_pool_args = (int *)currentLayer->params;

	if(xlayer_seq->opcode == OPCODE_SEPARABLE_CONV)
	{
		ConvolutionParameter *conv_params = xlayer_seq->hw_ops->conv_params;
		scalar_pool_args[5] = conv_params->stride_h;      //stride_h
		scalar_pool_args[6] = conv_params->stride_w;      //stride_v
		scalar_pool_args[7] = conv_params->filter_h;//->kernel_h;      //kernel_h
		scalar_pool_args[8] = conv_params->filter_w;//->kernel_w;      //kernel_w
		scalar_pool_args[10] = conv_params->pad_h;        //pad_h	  pad_w??
		scalar_pool_args[9] = 1;                         //mode  //setting average pool

		scalar_pool_args[12] = 1;                        //separable conv opcode
		scalar_pool_args[13] = conv_params->reluflag;     //relu

		int io_precision = currentLayer->qf_format.ip_fbits+currentLayer->qf_format.wt_fbits-currentLayer->qf_format.op_fbits;
		scalar_pool_args[14] = io_precision;
	}
	else
	{
		int pooltype = (int)xlayer_seq->hw_ops->pool_params->PoolType;
		PoolingParameter *pool_param = xlayer_seq->hw_ops->pool_params;
		scalar_pool_args[5] = pool_param->stride_h;      //stride_h
		scalar_pool_args[6] = pool_param->stride_w;      //stride_v
		scalar_pool_args[7] = pool_param->kernel_h;      //kernel_h
		scalar_pool_args[8] = pool_param->kernel_w;      //kernel_w
		scalar_pool_args[10] = pool_param->pad_h;        //pad_h	  pad_w??

		scalar_pool_args[9] = pooltype;                 //mode
		scalar_pool_args[12] = 0;                        //separable conv opcode
		scalar_pool_args[13] = 0;                        //relu

	}

	scalar_pool_args[0] = bottomShape.at(2);  //input_height
	scalar_pool_args[1] = bottomShape.at(3);  //input_width
	scalar_pool_args[2] = topShape.at(2);     //output_height
	scalar_pool_args[3] = topShape.at(3);     //output_width

#if (IO_TYPE==16)
	scalar_pool_args[4] = (bottomShape.at(1))/2;    //out_depth/2 ??  //TODO  :ANITHA
#else
	if(XBATCH_SIZE==1)
		scalar_pool_args[4] = (bottomShape.at(1))/2;    //out_depth/2 ??  //TODO  :ANITHA
	else
		scalar_pool_args[4] = (bottomShape.at(1));    //out_depth/2 ??  //TODO  :ANITHA
#endif

	if(scalar_pool_args[9] == 1)//Checking for MAX_POOL or AVG_POOL
	{
		//scalar_pool_args[12] = 5;
		float f_val = (float)1/(scalar_pool_args[7]*scalar_pool_args[8]);
		char ival = (char)(f_val);
		char fxval = ConvertToFP( f_val, ival, 8);
		scalar_pool_args[11] = fxval;
		int io_precision = currentLayer->qf_format.ip_fbits+8-currentLayer->qf_format.op_fbits;
		scalar_pool_args[14] = io_precision; 
	}
	else
		scalar_pool_args[11] = 1;//poolLayer_precision[layer_id].divisor;

	if(xlayer_seq->opcode == OPCODE_SEPARABLE_CONV)
	{
		//Loading Weights
		char *wt_ptr0 = (char *)currentLayer->wts_ptrs[0];

		const float *weight_vec     = (const float *)&(xlayer_seq->hw_ops->conv_params->weights[0][0]);
		//loadSeperableConvWgtsTXT(wt_ptr0, weight_vec, scalar_pool_args, currentLayer->qf_format.wt_fbits);
	}

	string quant_scheme = xlayer_seq->hw_ops->quantization_scheme;

	//	cout << "Quant scheme : " << xlayer_seq->hw_ops->quantization_scheme << endl;

	//# Loading first layer threshold param
	//# This is used for in offline quant mode for input quantization
	float th_layer_in = 0;
	float th_layer_out = 0;

	if((quant_scheme.compare("Xilinx") == 0))
	{
		th_layer_in = xlayer_seq->hw_ops->th_layer_in;
		th_layer_out = xlayer_seq->hw_ops->th_layer_out;
	}

	currentLayer->float_params.push_back(th_layer_in);
	currentLayer->float_params.push_back(th_layer_out);

	int ip_bw =  currentLayer->qf_format.ip_bw;
	float sf_in = th_layer_in / (pow(2, ip_bw - 1) - 1);
	currentLayer->float_params.push_back(sf_in);

	int op_bw =  currentLayer->qf_format.op_bw;
	float sf_out = th_layer_out / (pow(2, op_bw - 1) - 1);
	currentLayer->float_params.push_back(sf_out);

	scalar_pool_args[20] = currentLayer->en_batch_size_one; //BATCH_SIZE_ONE_ENABLE

	if(quant_scheme.compare("Xilinx") == 0)
	{
		scalar_pool_args[103] = 1;
	}

	currentLayer->kernType = POOL;
	if(quant_scheme.compare("Xilinx") == 0)
	{
		currentLayer->kernMode = OFFLINE;
	}
	else
	{
		currentLayer->kernMode = DYNAMIC_FIXED;
	}

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

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottomShape.at(0);//.blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->topShape.at(0);//.blob->shape;

	int *scalar_fc_args = (int *)currentLayer->params;


	scalar_fc_args[1]  = fc_params->N;             //input planes
	scalar_fc_args[0]  = AlignSize(fc_params->M, 32);             //output planes
	scalar_fc_args[2]  = fc_params->reluflag;      //relu
	scalar_fc_args[3]  = currentLayer->qf_format.wt_fbits;
	scalar_fc_args[4]  = currentLayer->qf_format.ip_fbits;
	scalar_fc_args[5]  = currentLayer->qf_format.bs_fbits;
	scalar_fc_args[6]  = currentLayer->qf_format.op_fbits;

	if(xlayer_seq[0].hw_ops->ip_bw == 6)
		scalar_fc_args[7]  = 1;
	else
		scalar_fc_args[7]  = 0;

	if(xlayer_seq[0].hw_ops->op_bw == 6)
		scalar_fc_args[8]  = 1;
	else
		scalar_fc_args[8]  = 0;

	//	ap_uint<24> overflow_pattern;
	int io_precision = currentLayer->qf_format.ip_fbits+currentLayer->qf_format.wt_fbits-currentLayer->qf_format.op_fbits;

	//# overflowPattern
	//	overflowPattern(io_precision, scalar_fc_args[8], overflow_pattern);
	scalar_fc_args[9]  = currentLayer->en_batch_size_one; //BATCH_SIZE_ONE_ENABLE;

	//# real output planes
	scalar_fc_args[16]  = fc_params->M;             //Needed for Error checking

	scalar_fc_args[10] = 1 << (io_precision - 1);   //TODO CHECKING Functionality with ROUND: ANITHA

	currentLayer->kernType = FC_LAYER;

	char *base_path = currentLayer->base_path;

	//Loading Weights
	HFC_WGT_TYPE *wt_ptr0 = (HFC_WGT_TYPE *)currentLayer->wts_ptrs[0];
	HFC_WGT_TYPE *wt_ptr1 = (HFC_WGT_TYPE *)currentLayer->wts_ptrs[1];

	int wt_bw = xlayer_seq->hw_ops->wt_bw;
	int wt_fl = xlayer_seq->hw_ops->wt_fl;

	const float *weights_vec = (const float *)&(fc_params->weights[0][0]);
	loadfcWgtsTXT(wt_ptr0, wt_ptr1, weights_vec, scalar_fc_args[16], scalar_fc_args[1], wt_bw, wt_fl);

	//Loading Bias
	HFC_BIAS_TYPE *bias_ptr = (HFC_BIAS_TYPE *)currentLayer->bias_ptr;
	int bs_bw = xlayer_seq->hw_ops->op_bw;
	int bs_fl = xlayer_seq->hw_ops->op_fl;

	//int bs_bw = xlayer_seq->hw_ops->wt_bw;
	//int bs_fl = xlayer_seq->hw_ops->wt_fl;

	const float *bias_vec = (const float *)&(fc_params->bias[0][0]);

	loadfcBiasTXT(bias_ptr, bias_vec, scalar_fc_args[0], bs_bw, bs_fl);

#if	EN_DEBUG_INIT_PRINT
	cout<<"fcInit End: "<<endl;
#endif
}

// A function to read a txt file to a vector<float>
vector<float> readtxt2(const string& filename)
																														{
	std::ifstream input_file(filename.c_str());
	if(!input_file)
	{
		cerr << "[ERROR] Couldn't open" << filename << endl;
		exit(-1);
	}

	std::vector<float> v;
	std::copy(istream_iterator<float>(input_file), istream_iterator<float>(), back_inserter(v));
	input_file.close();
	return v;
																														}

void swfcInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer, file_paths *const_data_path, int fc_id)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"fcInit Start: "<<endl;
#endif

	FCLayerParameter *fc_params = xlayer_seq->hw_ops->fc_params;

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottomShape.at(0);//.blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->topShape.at(0);//.blob->shape;

	string quant_scheme = xlayer_seq->hw_ops->quantization_scheme;

	//if(quant_scheme.compare("Xilinx") == 0)
	//weights_th_vec = (const float *)&(xlayer_seq->hw_ops->th_params[0]);

	int *scalar_fc_args = (int *)currentLayer->params;

	scalar_fc_args[0]  = 1;             //1
	scalar_fc_args[1]  = fc_params->N;  //input planes
	scalar_fc_args[2]  = fc_params->M;  //output planes
	scalar_fc_args[3]  = XBATCH_SIZE;  //output planes
	scalar_fc_args[4]  = fc_params->reluflag;      //relu
	
	scalar_fc_args[5]  = currentLayer->en_batch_size_one;

	currentLayer->kernType = FC_LAYER;

	char *base_path = currentLayer->base_path;

	//Loading Weights
	SW_FC_DATA_TYPE *wt_ptr0 = (SW_FC_DATA_TYPE *)currentLayer->wts_ptrs[0];

	const float *weights_vec = (const float *)&(fc_params->weights[0][0]);
	loadswfcWghtsBias(wt_ptr0, weights_vec, scalar_fc_args[1], scalar_fc_args[2]);

#if 0
	if(fc_id == 1)
	{

		const string yfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/AlexNet-8bit/weightsTrimmed/fc7_wts.txt";
		vector<float> vY = readtxt2(yfile);

		//for(int i=0; i<scalar_fc_args[1] * scalar_fc_args[2]; i++)
		for(int i=0; i<20; i++)
		{
			//X[i] = vX[i];
			if(weights_vec[i] != vY[i])
				fprintf(stderr, "i=%d vY[i]=%f WT[i]=%f \n", i, vY[i], weights_vec[i]);
		}
	}
#endif
	//Loading Bias
	SW_FC_DATA_TYPE *bias_ptr = (SW_FC_DATA_TYPE *)currentLayer->bias_ptr;

	const float *bias_vec = (const float *)&(fc_params->bias[0][0]);

	loadswfcWghtsBias(bias_ptr, bias_vec, scalar_fc_args[2], 1);  //Mx1

#if	EN_DEBUG_INIT_PRINT
	cout<<"fcInit End: "<<endl;
#endif
}

void softmaxInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"softmaxInit Start : "<<endl;
#endif

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottomShape.at(0);//.blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->topShape.at(0);//.blob->shape;

	printShape(bottomShape);

	int *scalar_softmax_args = (int *)currentLayer->params;

	scalar_softmax_args[0] = xlayer_seq->hw_ops->softmax_params->nclasses;//bottomShape.at(1);   //out_depth
	scalar_softmax_args[1] = xlayer_seq->hw_ops->softmax_params->nboxes;//bottomShape.at(2);   //out_depth
	scalar_softmax_args[2] = 0;//prev_layer_type;
	scalar_softmax_args[3] = XBATCH_SIZE;
	scalar_softmax_args[4] = currentLayer->qf_format.ip_fbits;
	scalar_softmax_args[5] = currentLayer->en_batch_size_one; //BATCH_SIZE_ONE_ENABLE



	int ip_bw = xlayer_seq->hw_ops->ip_bw;
	float th_in = 0;

	//# Flag for quantization scheme
	//# Set to 1 for Offline quant mode otherwise zero
	scalar_softmax_args[7] = 0;
	if(xlayer_seq->hw_ops->quantization_scheme.compare("Xilinx") == 0)
	{
		th_in = xlayer_seq->hw_ops->th_layer_in;
		scalar_softmax_args[7] = 1;
	}

	float sf_in = th_in / (pow(2, ip_bw - 1) - 1);

	scalar_softmax_args[8] = *((int *)(&sf_in));
	scalar_softmax_args[9] = *((int *)(&th_in));

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

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottomShape.at(0);//.blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->topShape.at(0);//.blob->shape;

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

	currentLayer->kernType = DECONV;

	//Loading Weights
	short *wt_ptr0 = (short *)currentLayer->wts_ptrs[0];

	int out_depth =  deconv_params->M;       //output planes
	int in_depth  = scalar_deconv_args[0];   //input planes
	int filter_h  = scalar_deconv_args[5];
	int filter_w  = scalar_deconv_args[5];
	int nElems =in_depth * out_depth * filter_h * filter_w;
	const float *weights_vec = (const float *)&(deconv_params->weights[0][0]);

	int quant_scheme = 0;
	const float *weights_th_vec;

	if(xlayer_seq->hw_ops->quantization_scheme.compare("Xilinx") == 0)
	{
		quant_scheme = 1;
		weights_th_vec = (const float *)&(xlayer_seq->hw_ops->th_params[0]);
	}
	int wt_bw = xlayer_seq->hw_ops->wt_bw;
	loadDeconvData(weights_vec, out_depth, in_depth, filter_h, filter_w, wt_ptr0, weights_th_vec, wt_bw, quant_scheme);

	//Loading Bias
	short *bias_ptr = (short *)currentLayer->bias_ptr;
	nElems = out_depth;
	const float *bias_vec = (const float *)&(deconv_params->bias[0][0]);
	float th_out = xlayer_seq->hw_ops->th_layer_out;
	int bias_bw = xlayer_seq->hw_ops->op_bw;
	loadDeconvBiasData(bias_vec, out_depth, bias_ptr, th_out, bias_bw, quant_scheme);


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


	vector<int> bottomShape =  xlayer_seq->hw_ops->bottomShape.at(0);//.blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->topShape.at(0);//.blob->shape;

	int *scalar_norm_args = (int *)currentLayer->params;

	printShape(bottomShape);

	scalar_norm_args[0] = bottomShape.at(1);   //in_depth
	scalar_norm_args[1] = bottomShape.at(2);   //input_h
	scalar_norm_args[2] = bottomShape.at(3);   //input_w

	int fbits_gamma = currentLayer->qf_format.op_fbits;   //fixing to 10 bits
	//int fbits_gamma = 10;   //fixing to 10 bits

	scalar_norm_args[3] = currentLayer->qf_format.ip_fbits;
	scalar_norm_args[4] = fbits_gamma;
	scalar_norm_args[5] = currentLayer->qf_format.op_fbits;
	scalar_norm_args[6] = across_spatial;
	scalar_norm_args[7] = XBATCH_SIZE;
	scalar_norm_args[8] = currentLayer->en_batch_size_one; //BATCH_SIZE_ONE_ENABLE

#if EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
	{
		fprintf(stderr,"\t%d", scalar_norm_args[i]);
	}
#endif



	//Loading Weights
	int offline_quant_mode = 0;
	string quant_scheme = xlayer_seq->hw_ops->quantization_scheme;
	float th_layer_in = 0;
	float th_layer_out = 0;

	const float *gamma_vec = (const float *)&(xlayer_seq->hw_ops->l2norm_params->gamma[0]);
	if(quant_scheme.compare("Xilinx") == 0)
	{
		offline_quant_mode = 1;
		th_layer_in = xlayer_seq->hw_ops->th_layer_in;
		th_layer_out = xlayer_seq->hw_ops->th_layer_out;
		//th_layer_in = 278.548614502;
		//th_layer_out = 7.81050395966;

		currentLayer->float_params.push_back(th_layer_in);
		currentLayer->float_params.push_back(th_layer_out);

		int ip_bw =  currentLayer->qf_format.ip_bw;
		float sf_in = th_layer_in / (pow(2, ip_bw - 1) - 1);
		currentLayer->float_params.push_back(sf_in);

		int op_bw =  currentLayer->qf_format.op_bw;
		float sf_out = th_layer_out / (pow(2, op_bw - 1) - 1);
		currentLayer->float_params.push_back(sf_out);

		IO_DATA_TYPE1 *wt_ptr1 = (IO_DATA_TYPE1 *)currentLayer->wts_ptrs[0];
		loadNormGamma_float(gamma_vec, scalar_norm_args[0], wt_ptr1, sf_out);
	}
	else
	{
		IO_DATA_TYPE *wt_ptr0 = (IO_DATA_TYPE *)currentLayer->wts_ptrs[0];
		loadNormGamma(gamma_vec, scalar_norm_args[0], wt_ptr0, fbits_gamma);
	}

	scalar_norm_args[9] = offline_quant_mode;
	scalar_norm_args[10] = pow(2,currentLayer->qf_format.op_bw-1)-1;

	currentLayer->kernType = NORM;
	if(quant_scheme.compare("Xilinx") == 0)
	{
		currentLayer->kernMode = OFFLINE;
	}
	else
	{
		currentLayer->kernMode = DYNAMIC_FIXED;
	}

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

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottomShape.at(0);//.blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->topShape.at(0);//.blob->shape;

	int *scalar_permute_args = (int *)currentLayer->params;

	printShape(bottomShape);

	scalar_permute_args[0] = bottomShape.at(1);   //in_depth
	scalar_permute_args[1] = bottomShape.at(2);   //input_h
	scalar_permute_args[2] = bottomShape.at(3);   //input_w
	scalar_permute_args[3] = currentLayer->qf_format.ip_fbits;   //input_fbits
	scalar_permute_args[4] = XBATCH_SIZE;
	scalar_permute_args[5] = currentLayer->en_batch_size_one; //BATCH_SIZE_ONE_ENABLE

	int offline_quant_mode = 0;

	if(xlayer_seq->hw_ops->quantization_scheme.compare("Xilinx") == 0)
	{
		offline_quant_mode = 1;
	}

	scalar_permute_args[6] = offline_quant_mode;

	string quant_scheme = xlayer_seq->hw_ops->quantization_scheme;

	//# Loading first layer threshold param
	//# This is used for in offline quant mode for input quantization
	float th_layer_in = 0;
	float th_layer_out = 0;

	if((quant_scheme.compare("Xilinx") == 0))
	{
		th_layer_in = xlayer_seq->hw_ops->th_layer_in;
		th_layer_out = xlayer_seq->hw_ops->th_layer_out;
	}

	currentLayer->float_params.push_back(th_layer_in);
	currentLayer->float_params.push_back(th_layer_out);

	int ip_bw =  currentLayer->qf_format.ip_bw;
	float sf_in = th_layer_in / (pow(2, ip_bw - 1) - 1);
	currentLayer->float_params.push_back(sf_in);

	int op_bw =  currentLayer->qf_format.op_bw;
	float sf_out = th_layer_out / (pow(2, op_bw - 1) - 1);
	currentLayer->float_params.push_back(sf_out);


#if EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
	{
		fprintf(stderr,"\t%d", scalar_permute_args[i]);
	}
#endif


	currentLayer->kernType = PERMUTE;

	if(quant_scheme.compare("Xilinx") == 0)
	{
		currentLayer->kernMode = OFFLINE;
	}
	else
	{
		currentLayer->kernMode = DYNAMIC_FIXED;
	}

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

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottomShape.at(0);//.blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->topShape.at(0);//.blob->shape;

	int *scalar_nms_args = (int *)currentLayer->params;


	scalar_nms_args[1] = xlayer_seq->hw_ops->nms_params->num_classes;
	scalar_nms_args[2] = xlayer_seq->hw_ops->nms_params->nms_top_k;
	scalar_nms_args[3] = xlayer_seq->hw_ops->nms_params->keep_top_k;
	scalar_nms_args[4] = xlayer_seq->hw_ops->nms_params->confidence_threshold;
	scalar_nms_args[5] = xlayer_seq->hw_ops->nms_params->nms_threshold;
	scalar_nms_args[6] = XBATCH_SIZE;
	scalar_nms_args[7] = currentLayer->qf_format.ip_fbits;
	scalar_nms_args[8] = currentLayer->en_batch_size_one; //BATCH_SIZE_ONE_ENABLE

	int ip_bw = xlayer_seq->hw_ops->ip_bw;
	float th_in = 0;

	//# Flag for quantization scheme
	//# Set to 1 for Offline quant mode otherwise zero
	scalar_nms_args[10] = 0;///0;//TODO:debug NMS offline:ANITHA

#if 0
	//# Check for offline quant mode and load input threshold value
	if(xlayer_seq->hw_ops->quantization_scheme.compare("Xilinx") == 0)
	{
		th_in = xlayer_seq->hw_ops->th_layer_in;
		scalar_nms_args[10] = 1;
	}
#if 0
	float sf_in = th_in / (pow(2, ip_bw - 1) - 1);

	scalar_nms_args[9] = *((int *)(&sf_in));
#endif
#endif

	float confidence_threshold = xlayer_seq->hw_ops->nms_params->confidence_threshold;
	float nms_threshold        = xlayer_seq->hw_ops->nms_params->nms_threshold;

	currentLayer->float_params.push_back(confidence_threshold);
	currentLayer->float_params.push_back(nms_threshold);

	scalar_nms_args[11] = *((int *)(&confidence_threshold));
	scalar_nms_args[12] = *((int *)(&nms_threshold));


#if 0
	string quant_scheme = xlayer_seq->hw_ops->quantization_scheme;

	//# Loading first layer threshold param
	//# This is used for in offline quant mode for input quantization
	float th_layer_in = 0;
	float th_layer_out = 0;

	if((quant_scheme.compare("Xilinx") == 0))
	{
		th_layer_in = xlayer_seq->hw_ops->th_layer_in;
		th_layer_out = xlayer_seq->hw_ops->th_layer_out;
	}

	currentLayer->float_params.push_back(th_layer_in);
	currentLayer->float_params.push_back(th_layer_out);

	ip_bw =  currentLayer->qf_format.ip_bw;
	sf_in = th_layer_in / (pow(2, ip_bw - 1) - 1);
	currentLayer->float_params.push_back(sf_in);

	int op_bw =  currentLayer->qf_format.op_bw;
	float sf_out = th_layer_out / (pow(2, op_bw - 1) - 1);
	currentLayer->float_params.push_back(sf_out);
#endif

	//Weights path
	vector<int> pboxShape =  xlayer_seq[0].hw_ops->nms_params->pboxShape;
	int pbox_size = pboxShape.at(2);

	scalar_nms_args[0] = pbox_size/4;//nms_nboxes;

	printShape(pboxShape);

	//Loading Weights
	float *pbox_ptr0 = (float *)currentLayer->wts_ptrs[0];

	nms_layerinputdatawrite(pbox_ptr0, pbox_size, xlayer_seq->hw_ops->nms_params->pbox);

	vector<int> varianceShape =  xlayer_seq[0].hw_ops->nms_params->varShape;

	printShape(varianceShape);

	int variance_size = varianceShape.at(2);

	//Loading Weights
	float *variance_ptr0 = (float *)currentLayer->wts_ptrs[1];

	nms_layerinputdatawrite(variance_ptr0, variance_size, xlayer_seq->hw_ops->nms_params->var);

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

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottomShape.at(0);//.blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->topShape.at(0);//.blob->shape;

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
	cout<<"eltwiseaddInit Start : "<<endl;
#endif

	vector<int> bottomShape =  xlayer_seq->hw_ops->bottomShape.at(0);//.blob->shape;
	vector<int> topShape    =  xlayer_seq->hw_ops->topShape.at(0);//.blob->shape;

	int *scalar_eltwiseadd_args = (int *)currentLayer->params;

	printShape(bottomShape);

	scalar_eltwiseadd_args[0] = bottomShape.at(1) * bottomShape.at(2) * bottomShape.at(3) * (XBATCH_SIZE/HCONV_OUT_PORTS);   //in_size

	if(xlayer_seq[0].hw_ops->ip_bw == 6)
		scalar_eltwiseadd_args[1]  = 1;
	else
		scalar_eltwiseadd_args[1]  = 0;

	scalar_eltwiseadd_args[2]  = xlayer_seq->hw_ops->eltwise_params->reluflag;

	scalar_eltwiseadd_args[3] = bottomShape.at(1);   //in_depth
	scalar_eltwiseadd_args[4] = bottomShape.at(2);   //input_h
	scalar_eltwiseadd_args[5] = bottomShape.at(3);   //input_w

	printShape(topShape);

	scalar_eltwiseadd_args[6] = topShape.at(1);   //out_depth
	scalar_eltwiseadd_args[7] = topShape.at(2);   //output_h
	scalar_eltwiseadd_args[8] = topShape.at(3);   //output_w

#if EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
	{
		fprintf(stderr,"\t%d", scalar_eltwiseadd_args[i]);
	}
#endif


	currentLayer->kernType = ELTWISEADD;

#if	EN_DEBUG_INIT_PRINT
	cout<<"eltwiseaddInit End : "<<endl;
#endif
}

void xcustomInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"xcustomInit Start : "<<endl;
#endif


	for(int iter_i=0;iter_i<xlayer_seq->hw_ops->xcustom_params->input_dims.size();iter_i++){

		currentLayer->input_dims.push_back(xlayer_seq->hw_ops->xcustom_params->input_dims[iter_i]);
	}
	for(int in_i=0;in_i<currentLayer->input_dims.size();in_i++){
		currentLayer->input_dims[in_i][0]=XBATCH_SIZE;
	}
	for(int iter_i=0;iter_i<xlayer_seq->hw_ops->xcustom_params->output_dims.size();iter_i++){

		currentLayer->output_dims.push_back(xlayer_seq->hw_ops->xcustom_params->output_dims[iter_i]);
	}
	for(int out_i=0;out_i<currentLayer->output_dims.size();out_i++){
		currentLayer->output_dims[out_i][0]=XBATCH_SIZE;
	}
	for(int iter_i=0;iter_i<xlayer_seq->hw_ops->xcustom_params->output_dim.size();iter_i++){

		currentLayer->output_dim.push_back(xlayer_seq->hw_ops->xcustom_params->output_dim[iter_i]);
	}

		currentLayer->output_dim[0]=XBATCH_SIZE;

	currentLayer->float_args  = xlayer_seq->hw_ops->xcustom_params->float_args;
	currentLayer->string_args = xlayer_seq->hw_ops->xcustom_params->string_args;
	currentLayer->custom_float_params = xlayer_seq->hw_ops->xcustom_params->params;
	currentLayer->params_dims = xlayer_seq->hw_ops->xcustom_params->params_dims;
	currentLayer->custom_layer_type = xlayer_seq->hw_ops->xcustom_params->type;
	currentLayer->custom_reluflag =  xlayer_seq->hw_ops->xcustom_params->reluflag;



#if 0
	int *scalar_xcustom_args = (int *)currentLayer->params;

	printShape(bottomShape);

	scalar_xcustom_args[0] = bottomShape.at(1);   //in_depth
	scalar_xcustom_args[1] = bottomShape.at(2);   //input_h
	scalar_xcustom_args[2] = bottomShape.at(3);   //input_w

	printShape(topShape);

	scalar_xcustom_args[3] = topShape.at(1);   //out_depth
	scalar_xcustom_args[4] = topShape.at(2);   //output_h
	scalar_xcustom_args[5] = topShape.at(3);   //output_w
#endif

#if 0//EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
	{
		fprintf(stderr,"\t%d", scalar_xcustom_args[i]);
	}
#endif


	currentLayer->kernType = XCUSTOM;

#if	EN_DEBUG_INIT_PRINT
	cout<<"xcustomInit End : "<<endl;
#endif
}

void xpackInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"xpackInit Start : "<<endl;
#endif

	vector<int> topShape    =  xlayer_seq->hw_ops->topShape[0];
	vector<int> bottomShape    =  xlayer_seq->hw_ops->bottomShape[0];
	int *scalar_pack_args = (int *)currentLayer->params;
	int s_cnt=0;
	for(int it=1;it<bottomShape.size();it++){
		scalar_pack_args[s_cnt]=bottomShape.at(it);
		s_cnt++;
	}
	for(int it=1;it<topShape.size();it++){
		scalar_pack_args[s_cnt]=topShape.at(it);
		s_cnt++;
	}

	scalar_pack_args[8] = xlayer_seq->hw_ops->xpack_params->pack;
	scalar_pack_args[9] = xlayer_seq->hw_ops->xpack_params->fbits;

#if (IO_TYPE==16)
	scalar_pack_args[9] = scalar_pack_args[9] - 1;
#endif

	float th_layer_in = xlayer_seq->hw_ops->th_layer_in;
	//th_layer_in =165.620995617;
	currentLayer->float_params.push_back(th_layer_in);
	currentLayer->string_args.push_back(xlayer_seq->hw_ops->quantization_scheme);

	int ip_bw =  xlayer_seq->hw_ops->ip_bw;
	float sf_in = th_layer_in / (pow(2, ip_bw - 1) - 1);
	currentLayer->float_params.push_back(sf_in);

	//	scalar_pack_args[0] = bottomShape.at(1);
	//	scalar_pack_args[1] = bottomShape.at(2);
	//	scalar_pack_args[2] = bottomShape.at(3);
	//	scalar_pack_args[3] = topShape.at(1);
	//	scalar_pack_args[4] = topShape.at(2);
	//	scalar_pack_args[5] = topShape.at(3);
	currentLayer->kernType = XPACK;

	string quant_scheme = xlayer_seq->hw_ops->quantization_scheme;
	if(quant_scheme.compare("Xilinx") == 0)
	{
		currentLayer->kernMode = OFFLINE;
	}
	else
	{
		currentLayer->kernMode = DYNAMIC_FIXED;
	}


#if	EN_DEBUG_INIT_PRINT
	cout<<"xpackInit End : "<<endl;
#endif
}

void xunpackInit(XlayerData *xlayer_seq, xChangeLayer *currentLayer)
{
#if	EN_DEBUG_INIT_PRINT
	cout<<"xunpackInit Start : "<<endl;
#endif

	vector<int> topShape    =  xlayer_seq->hw_ops->top.at(0).blob->shape;
	vector<int> bottomShape =  xlayer_seq->hw_ops->bottom.at(0).blob->shape;
	int *scalar_pack_args   = (int *)currentLayer->params;

	scalar_pack_args[0] = bottomShape.at(1);
	scalar_pack_args[1] = bottomShape.at(2);
	scalar_pack_args[2] = bottomShape.at(3);
	scalar_pack_args[3] = topShape.at(1);
	scalar_pack_args[4] = topShape.at(2);
	scalar_pack_args[5] = topShape.at(3);

	scalar_pack_args[8] = xlayer_seq->hw_ops->xpack_params->pack;
	scalar_pack_args[9] = xlayer_seq->hw_ops->xpack_params->fbits;

#if (IO_TYPE==16)
	scalar_pack_args[9] = scalar_pack_args[9] - 1;
#endif

	currentLayer->kernType = XUNPACK;


#if	EN_DEBUG_INIT_PRINT
	cout<<"xunpackInit End : "<<endl;
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

#if DATA_IN_BINARY
	FILE* fp = fopen(mean_path, "rb");
#else
	FILE* fp = fopen(mean_path, "r");
#endif

	if(fp == NULL)
	{
		fprintf(stderr, "\n** Cannot open mean file (or) No mean file : %s **\n", mean_path);
		//return NULL;
	}


	for(int i = 0; i < mean_size; i++)
	{

#if DATA_IN_BINARY
		fread(&float_val, sizeof(float)*1, 1, fp);
#else
		fscanf(fp, "%f ", &float_val);
#endif

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

void initXChangeHost(char *file_path, vector < XlayerData > &xlayer_seq, vector<xChangeLayer> (&lay_obj)[NUM_IMG], int en_batch_size_one)
{

#if EN_CONSOLE_TXT_INIT
	cout<< "\ninitXChangeHost : " <<endl;
#endif

	int layer_cnt = xlayer_seq.size();

#if EN_CONSOLE_TXT_INIT
	cout<< "\nlayer_cnt : " << layer_cnt <<endl;
#endif

	int imgId = 0;
	int prev_layer_type = -1;
	qformat prev_qf_format;
	int height, width, depth;
#if 0
	char *weight_path   = (char *)malloc(500*sizeof(char));
	char *bias_path     = (char *)malloc(500*sizeof(char));
	char *mean_path     = (char *)malloc(500*sizeof(char));
	char *variance_path = (char *)malloc(500*sizeof(char));
	char *beta_path     = (char *)malloc(500*sizeof(char));
	char *gamma_path    = (char *)malloc(500*sizeof(char));
#endif

	int io_8bit_flag = 0;
	int fc_id = 0;

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
		currentLayer->en_batch_size_one = en_batch_size_one;
		currentLayer->kernexecType = xlayer_seq[layerId].layer_type;
		currentLayer->layername = xlayer_seq[layerId].hw_ops->name;

		vector<int> topShape    =  xlayer_seq[layerId].hw_ops->top.at(0).blob->shape;
		vector<int> bottomShape    =  xlayer_seq[layerId].hw_ops->bottom.at(0).blob->shape;
		int size;
		int s_cnt=0;
		size = 1;
		int size_param;
		int bytes;

		_datatype_e xdata_type_flag=xlayer_seq[layerId].ip_blob[0].io_data_type;

		if(xdata_type_flag ==XI_INT8 || xdata_type_flag == XI_UINT8)
		{
			bytes = 1;
		}else if(xdata_type_flag ==XI_INT16 || xdata_type_flag == XI_UINT16)
		{
			bytes = 2;
		}else if(xdata_type_flag ==XI_INT32|| xdata_type_flag == XI_UINT32 || xdata_type_flag ==XI_FLOAT)
		{
			bytes = 4;
		}

		for(int it=1;it<bottomShape.size();it++)
		{
			size_param = bottomShape.at(it);
			if( ( xlayer_seq[layerId].hw_ops->type.compare("Convolution") == 0) && (layerId==0) && (it==1) )
			{
				size_param = AlignSize(size_param, 4);
			}

			size *= size_param;
			//printf("%d\n", size_param);
			s_cnt++;
		}

		currentLayer->input_sizebytes = size*bytes*XBATCH_SIZE;

		xdata_type_flag=xlayer_seq[layerId].op_blob[0].io_data_type;

		if(xdata_type_flag ==XI_INT8 || xdata_type_flag == XI_UINT8)
		{
			bytes = 1;
		}else if(xdata_type_flag ==XI_INT16 || xdata_type_flag == XI_UINT16)
		{
			bytes = 2;
		}else if(xdata_type_flag ==XI_INT32|| xdata_type_flag == XI_UINT32 || xdata_type_flag ==XI_FLOAT)
		{
			bytes = 4;
		}

		size = 1;
		for(int it=1;it<topShape.size();it++){

			size_param = topShape.at(it);
			size *= size_param;
			//printf("%d\n", size_param);
			s_cnt++;
		}
		currentLayer->output_sizebytes = size*bytes*XBATCH_SIZE;
		currentLayer->output_size = size;

		qformat qf_format;
		qf_format.ip_bw    = xlayer_seq[layerId].hw_ops->ip_bw;
		qf_format.ip_fbits = xlayer_seq[layerId].hw_ops->ip_fl;
		qf_format.op_bw    = xlayer_seq[layerId].hw_ops->op_bw;
		qf_format.op_fbits = xlayer_seq[layerId].hw_ops->op_fl;
		qf_format.wt_bw    = xlayer_seq[layerId].hw_ops->wt_bw;
		qf_format.wt_fbits = xlayer_seq[layerId].hw_ops->wt_fl;
		qf_format.bs_bw    = xlayer_seq[layerId].hw_ops->op_bw;
		qf_format.bs_fbits = xlayer_seq[layerId].hw_ops->op_fl;

		//if((xlayer_seq[layerId].hw_ops->opcode == OPCODE_POOL2CONV) || (xlayer_seq[layerId].hw_ops->opcode == OPCODE_AVRPOOL2CONV) || (xlayer_seq[layerId].hw_ops->opcode == OPCODE_FC2CONV))
		if((xlayer_seq[layerId].hw_ops->opcode == OPCODE_3D_CONV) || (xlayer_seq[layerId].hw_ops->opcode == OPCODE_POOL2CONV) || (xlayer_seq[layerId].hw_ops->opcode == OPCODE_AVRPOOL2CONV) || (xlayer_seq[layerId].hw_ops->opcode == OPCODE_POOL_CONV2CONV) || (xlayer_seq[layerId].hw_ops->opcode == OPCODE_FC2CONV))
		{
			qf_format.ip_bw    = xlayer_seq[layerId].hw_ops->conv_params->ip_bw_3d;
			qf_format.ip_fbits = xlayer_seq[layerId].hw_ops->conv_params->ip_fl_3d;
			qf_format.op_bw    = xlayer_seq[layerId].hw_ops->conv_params->op_bw_3d;
			qf_format.op_fbits = xlayer_seq[layerId].hw_ops->conv_params->op_fl_3d;
			qf_format.wt_bw    = xlayer_seq[layerId].hw_ops->conv_params->wt_bw_3d;
			qf_format.wt_fbits = xlayer_seq[layerId].hw_ops->conv_params->wt_fl_3d;
			qf_format.bs_bw    = xlayer_seq[layerId].hw_ops->conv_params->op_bw_3d;
			qf_format.bs_fbits = xlayer_seq[layerId].hw_ops->conv_params->op_fl_3d;
		}

#if (IO_TYPE==16)
		qf_format.ip_fbits = qf_format.ip_fbits - 1;
		qf_format.op_fbits = qf_format.op_fbits - 1;
		qf_format.bs_fbits = qf_format.bs_fbits - 1;
#endif

#if EN_CONSOLE_TXT_INIT
		cout <<"wt_fbits:"<< qf_format.wt_fbits <<"  ip_fbits:"<< qf_format.ip_fbits <<"  op_fbits:"<< qf_format.op_fbits <<"  bs_fbits:"<< qf_format.bs_fbits <<endl;
#endif

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
		//sprintf(ref_path, "%s/outputs/%s", base_path, path0);

		//sprintf(ref_path, "%s/outputs_BiasZero/%s", base_path, path0);
#endif

		currentLayer->ref_path = ref_path;
		currentLayer->out_path = out_path;
#if EN_CONSOLE_TXT_INIT
		cout <<"ref_path: "<< currentLayer->ref_path <<endl;
		cout <<"out_path: "<< currentLayer->out_path <<endl;
#endif


		//loading out_path, ref_path
		char *weight_path = (char *)malloc(500*sizeof(char));
		char *bias_path = (char *)malloc(500*sizeof(char));

		string layer_name = xlayer_seq[layerId].hw_ops->output_file;
		replace(layer_name.begin(), layer_name.end(), '/', '_');  // Replace all / with _, otherwise fileWrite becomes an issue
		const char *path2 = layer_name.c_str();
#if 0
		sprintf(bias_path, "%s", path2);
		char *path3 = strstr(bias_path, "_out.txt");
		//pch = strstr (str,"simple");
		strncpy (path3,"",0);
		puts (path3);
#endif

		sprintf(weight_path, "%s/data/%s", base_path, path2);
		sprintf(bias_path, "%s/data/%s", base_path, path2);

		const_data_path.wgt_path  = weight_path;
		const_data_path.bias_path = bias_path;

#if EN_CONSOLE_TXT_INIT
		cout <<"wght_path: "<< weight_path <<endl;
		cout <<"bias_path: "<< bias_path <<endl;
#endif



#if 0
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
				//replace(bs_path.begin(), bs_path.end(), '/', '_');conv_params->op_bw_3d;
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

		if( xlayer_seq[layerId].hw_ops->type.compare("Pooling") == 0)
		{
			if(xlayer_seq[layerId].opcode == OPCODE_SEPARABLE_CONV)
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
				std::cerr << "[OPC40] Path : " << weight_path << std::endl;

				const_data_path.wgt_path  = weight_path;
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

#endif

		int *scalar_args = (int *)currentLayer->params;
		int *prevparams;
		int *curparams = (int*)currentLayer->params;
		float *prevfloatparams;
		float *curfloatparams;
		xChangeLayer *prevLayer;
		int prev_layer_id;;
		int p_size;

		for(int i=0;i<MAX_PARAM_SIZE;i++)
		{
			scalar_args[i] = 0;
		}

		int offline_quant_mode = 0;
		if(xlayer_seq[layerId].hw_ops->quantization_scheme.compare("Xilinx") == 0)
		{
			offline_quant_mode = 1;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("Convolution") == 0)
		{
			convInit(current_seq_layer, currentLayer, layerId, &const_data_path, io_8bit_flag, fc_id);
#if 0
			if(xlayer_seq[layerId].hw_ops->opcode == OPCODE_FC2CONV)
			{
				fc_id++;
			}
			prev_layer_type = CONV;
			if(currentLayer->qf_format.op_fbits == 0)
			{
				currentLayer->qf_format.op_fbits = prev_qf_format.op_fbits;
			}

			int p_size = xlayer_seq[layerId].prev_layers_ID.size();
			int prev_layer_id;

			if(p_size > 0)
			{
				int prev_layer_id = xlayer_seq[layerId].prev_layers_ID[0].layerindex;
				prevLayer = &lay_obj[imgId][prev_layer_id];

				prevparams = (int*)prevLayer->params;

				//Following params used for Host
				curparams[107] = prevparams[2];  //output height
				curparams[108] = prevparams[3];  //output width
				curparams[109] = prevparams[4];  //output depth
			}
#endif

#if 0
			curfloatparams = &currentLayer->float_params[0];
			if(curparams[34] == OPCODE_AVRPOOL2CONV)//Checking for MAX_POOL or AVG_POOL
			{
				fprintf(stderr, "stop");
				fprintf(stderr, "%f\n", curfloatparams[0]);
			}
#endif
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("Pooling") == 0)
		{
#if 0
			if(currentLayer->qf_format.op_fbits == 0)  //average pool
			{
				currentLayer->qf_format = prev_qf_format;
			}
#endif
			poolInit(current_seq_layer, currentLayer, layerId, &const_data_path);
			//prev_layer_type = POOL;


#if 0
			curfloatparams = &currentLayer->float_params[0];
			if(curparams[9] == 1)//Checking for MAX_POOL or AVG_POOL
			{
				fprintf(stderr, "stop");
				fprintf(stderr, "%f\n", curfloatparams[0]);
			}
#endif
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("InnerProduct") == 0)
		{
			//fcInit(current_seq_layer, currentLayer, &const_data_path);
			swfcInit(current_seq_layer, currentLayer, &const_data_path, fc_id);
#if 0
			int p_size = xlayer_seq[layerId].prev_layers_ID.size();
			int prev_layer_id;
			if(p_size > 0)
			{
				int prev_layer_id = xlayer_seq[layerId].prev_layers_ID[0].layerindex;
				prevLayer = &lay_obj[imgId][prev_layer_id];

				prevfloatparams = &prevLayer->float_params[0];
				currentLayer->float_params.push_back(prevfloatparams[2]);
				currentLayer->float_params.push_back(prevfloatparams[3]);
			}
			prev_layer_type = FC_LAYER;

			fc_id++;
#endif

		}

		if( xlayer_seq[layerId].hw_ops->type.compare("Softmax") == 0)
		{
			//currentLayer->qf_format = prev_qf_format;
			softmaxInit(current_seq_layer, currentLayer);
#if 0
			if(prev_layer_type == PERMUTE)
			{
				int p_size = xlayer_seq[layerId].prev_layers_ID.size();
				int prev_layer_id;
				if(p_size > 0)
				{
					int prev_layer_id = xlayer_seq[layerId].prev_layers_ID[0].layerindex;
					prevLayer = &lay_obj[imgId][prev_layer_id];

					prevfloatparams = &prevLayer->float_params[0];
					currentLayer->float_params.push_back(prevfloatparams[2]);
					currentLayer->float_params.push_back(prevfloatparams[3]);

					float sf_in = prevfloatparams[3];//th_in / (pow(2, ip_bw - 1) - 1);

					curparams[8] = *((int *)(&sf_in));
				}
			}
			prev_layer_type = SOFTMAX;
#endif
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("Deconvolution") == 0)
		{
			deconvInit(current_seq_layer, currentLayer, &const_data_path);
			//prev_layer_type = DECONV;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("L2Normalize") == 0)
		{
			//currentLayer->qf_format = prev_qf_format;
			normInit(current_seq_layer, currentLayer, &const_data_path);
			//prev_layer_type = NORM;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("Permute") == 0)
		{
			//currentLayer->qf_format = prev_qf_format;
			permuteInit(current_seq_layer, currentLayer);
			//prev_layer_type = PERMUTE;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("NMS") == 0)
		{

			nmsInit(current_seq_layer, currentLayer, &const_data_path);
			currentLayer->output_sizebytes = (200+1)*7*4*XBATCH_SIZE;
#if 0
			currentLayer->qf_format = prev_qf_format;
			int p_size = xlayer_seq[layerId].prev_layers_ID.size();
			int prev_layer_id;
			if(p_size > 0)
			{
				int prev_layer_id = xlayer_seq[layerId].prev_layers_ID[0].layerindex;
				prevLayer = &lay_obj[imgId][prev_layer_id];

				prevfloatparams = &prevLayer->float_params[0];

				//float th_in = 6.71453475952;
				//float sf_in = th_in / (pow(2, 8 - 1) - 1);
				//prevfloatparams[2] = sf_in;//6.71453475952;   //TODO:ANITHA

				currentLayer->float_params.push_back(prevfloatparams[2]);
				currentLayer->float_params.push_back(prevfloatparams[3]);
			}
			prev_layer_type = NMS;
#endif
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("Crop") == 0)
		{
			cropInit(current_seq_layer, currentLayer);
			//prev_layer_type = CROP;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("XCustom") == 0)
		{
			xcustomInit(current_seq_layer, currentLayer);
			//prev_layer_type = XCUSTOM;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("XPack") == 0)
		{
			xpackInit(current_seq_layer, currentLayer);
			//currentLayer->qf_format = prev_qf_format;
			//prev_layer_type = XPACK;
		}

		if( xlayer_seq[layerId].hw_ops->type.compare("Eltwise") == 0)
		{
			eltwiseaddInit(current_seq_layer, currentLayer);
		}

		if(xlayer_seq[layerId].hw_ops->quantization_scheme.compare("SinglePrecision") == 0)
		{
			currentLayer->float_params.push_back(0);
			currentLayer->float_params.push_back(0);
			currentLayer->float_params.push_back(0);
			currentLayer->float_params.push_back(0);
		}

		p_size = xlayer_seq[layerId].prev_layers_ID.size();

		if(p_size > 0)
		{
			prev_layer_id = xlayer_seq[layerId].prev_layers_ID[0].layerindex;
			prevLayer = &lay_obj[imgId][prev_layer_id];

			if(currentLayer->qf_format.ip_fbits == 0)
			{
				currentLayer->qf_format = prevLayer->qf_format;
				currentLayer->qf_format.ip_bw = prevLayer->qf_format.op_bw;
				currentLayer->qf_format.ip_fbits = prevLayer->qf_format.op_fbits;
			}
			currentLayer->prev_kernType = prevLayer->kernType;
			currentLayer->prev_kernMode = prevLayer->kernMode;

			prevfloatparams = &prevLayer->float_params[0];
			currentLayer->float_params.push_back(prevfloatparams[0]);
			currentLayer->float_params.push_back(prevfloatparams[1]);
			currentLayer->float_params.push_back(prevfloatparams[2]);
			currentLayer->float_params.push_back(prevfloatparams[3]);

			if(currentLayer->kernType == SOFTMAX)
			{
				float sf_in = prevfloatparams[3];
				curparams[8] = *((int *)(&sf_in));
				curparams[2] = currentLayer->prev_kernType;
				curparams[4] = currentLayer->qf_format.ip_fbits;
				if(currentLayer->prev_kernType == CONV)
				{
					curparams[6] = OPCODE_FC2CONV;
				}
				else
					curparams[6] = 0;

				if(currentLayer->prev_kernType == PERMUTE)
				{
					curparams[6] = OPCODE_FC2CONV;
				}
				else
					curparams[6] = 0;

				if (currentLayer->prev_kernMode == OFFLINE)
				{
					curparams[7] = 1;
				}
				else
				{
					curparams[7] = 0;
				}
			}

			if(currentLayer->kernType == FC_LAYER)
			{
				prevparams = (int*)prevLayer->params;
				int reH, reW, reD;
#if 0
				reH = prevparams[2];
				reW = prevparams[3];
#if (IO_TYPE==8)
				reD = prevparams[4];
#else
				reD = prevparams[4] << 1;
#endif
#endif

				reH = prevparams[1];
				reW = prevparams[2];
#if (IO_TYPE==8)
				reD = prevparams[0];
#else
				reD = prevparams[0] << 1;
#endif

				curparams[5] = currentLayer->prev_kernType;
				curparams[6] = reH;
				curparams[7] = reW;
				curparams[8] = reD;
				//float sf_in = prevfloatparams[3];
				float sf_in = prevfloatparams[0];
				curparams[9] = *((int *)(&sf_in));
				if (currentLayer->prev_kernMode == OFFLINE)
				{
					curparams[10] = 1;  //quant mode
				}
				else
				{
					curparams[10] = 0;  //quant mode
				}
				curparams[11]  = currentLayer->qf_format.ip_fbits;
			}

			if(currentLayer->kernType == NMS)
			{
				float sf_in = prevfloatparams[3];
				curparams[13] = *((int *)(&sf_in));
				if (currentLayer->prev_kernMode == OFFLINE)
				{
					curparams[10] = 1;  //quant mode
				}
				curparams[7]  = currentLayer->qf_format.ip_fbits;
			}

#if 0
			if(currentLayer->kernType == XPACK)
			{
				curparams[9]  = currentLayer->qf_format.ip_fbits;
			}
#endif
		}

#if 0
		//saving fbits for the next layer, if fbits are not specified
		prev_qf_format.ip_bw    = currentLayer->qf_format.op_bw;
		prev_qf_format.ip_fbits = currentLayer->qf_format.op_fbits;
		prev_qf_format.op_bw    = currentLayer->qf_format.op_bw;
		prev_qf_format.op_fbits = currentLayer->qf_format.op_fbits;
		prev_qf_format.wt_bw    = currentLayer->qf_format.wt_bw;
		prev_qf_format.wt_fbits = currentLayer->qf_format.wt_fbits;
		prev_qf_format.bs_bw    = currentLayer->qf_format.op_bw;
		prev_qf_format.bs_fbits = currentLayer->qf_format.op_fbits;
#endif
		if(layerId==0)
		{
			//loadMean(current_seq_layer, currentLayer);
		}

#if EN_CONSOLE_TXT_INIT
		qf_format = currentLayer->qf_format;
		cout <<"wt_fbits:"<< qf_format.wt_fbits <<"  ip_fbits:"<< qf_format.ip_fbits <<"  op_fbits:"<< qf_format.op_fbits <<"  bs_fbits:"<< qf_format.bs_fbits <<endl;
#endif

#if EN_CONSOLE_TXT_INIT
		int *scalar_args1 = (int *)lay_obj[imgId][layerId].params;
		cout <<"layerId:"<< layerId;
		cout << endl;
		for(int i=0;i<MAX_PARAM_SIZE;i++)
		{
			cout << scalar_args1[i] << " ";
		}
		cout << endl;
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

#if 0
	free(weight_path);
	free(bias_path);
	free(mean_path);
	free(variance_path);
	free(gamma_path);
	free(beta_path);
#endif

#if EN_CONSOLE_TXT_INIT
	cout << "Params updated for each Layer for all images" << endl;

	cout<< "initXChangeHost : done" <<endl;
#endif

}
