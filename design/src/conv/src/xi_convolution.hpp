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

#define DEBUG_COMPUTE 0
#if DEBUG_COMPUTE
FILE *fp_input=fopen("/proj/sdxapps/users/maheshm/debug_t/input_compute.txt","w");
FILE *fp_weight=fopen("/proj/sdxapps/users/maheshm/debug_t/weight_compute.txt","w");
FILE *fp_wr=fopen("/proj/sdxapps/users/maheshm/debug_t/write_debug.txt","w");
FILE *fptr= fopen("/proj/sdxapps/users/maheshm/debug_t/input_pix0.txt","w");

#endif

//FILE *f_ostg= fopen("/proj/sdxapps/users/maheshm/debug_t/ostg.txt","w");

void LoadDesc_ffa(int *scalar_conv_args,
		input_struct &input_desc,
		output_struct &output_desc,
		weight_struct &weight_desc,
		conv_struct &conv_desc)
{
#pragma HLS INLINE
	int scalar_args[39];

	Arg_Loop:
	for(ap_uint<8> arg=0;arg< 39;arg++)
	{
#pragma HLS PIPELINE
		scalar_args[arg] = scalar_conv_args[arg];		
	}

	int l_input_height_ffa0			= scalar_args[0];
	int l_input_Width_ffa0			= scalar_args[1];
	int l_output_height_ffa0		= scalar_args[2];
	int l_output_width_ffa0			= scalar_args[3];
	int l_output_planes_ffa0		= scalar_args[4];
	int l_input_planes_ffa0			= scalar_args[5];
	int l_conv_stride_ffa0			= scalar_args[6];
	int l_filter_height_ffa0		= scalar_args[7];
	int l_filter_width_ffa0			= scalar_args[8];
	int l_pad_ffa0					= scalar_args[9];
	int l_relu_ffa0					= scalar_args[10];
	int l_group_ffa0				= scalar_args[11];
	int l_layer_ffa0				= scalar_args[12];
	int l_nkpf_ffa0					= scalar_args[13];
	int l_istg_row_cnt_ffa0			= scalar_args[14];
	int l_ostg_row_cnt_ffa0			= scalar_args[15];
	int l_compute_planes_ffa0		= scalar_args[16];
	int l_straddle_factor_ffa0		= scalar_args[17];
	int l_out_offset_ffa0			= scalar_args[18];
	bool l_single_load_kernel_ffa0	= scalar_args[19];
	int l_inout_precision_ffa0		= scalar_args[20];
	int l_shift_precision2_ffa0 	= scalar_args[21];
	int l_mean_value0_ffa0			= scalar_args[22];
	int l_mean_value1_ffa0			= scalar_args[23];
	int l_mean_value2_ffa0			= scalar_args[24];
	int l_mean_value3_ffa0			= scalar_args[25];
	bool l_mean_sub_flag_ffa0       = scalar_args[26];
	int l_bias_offset_ffa0          = scalar_args[27];
	int l_weight_offset_ffa0        = scalar_args[28];
	int l_dilation_factor_ffa0      = scalar_args[29];
	int l_read_from_ddr 			= scalar_args[31];
	int l_write_to_ddr 				= scalar_args[32];
//	bool l_bn_en					= scalar_args[32];
	int l_norm_k					= scalar_args[33];
	//int l_mode						= scalar_args[34];
	//int l_elem_wise_offset			= scalar_args[35];
	//int l_bn_mean_offset			= scalar_args[36];
	int l_opcode					= scalar_args[34];
	int l_mul_in 					= scalar_args[35];
	int l_norm_prec 				= scalar_args[36];
	int l_norm_prec_2				= scalar_args[37];
	int l_norm_prec_3				= scalar_args[38];

#if !__SYNTHESIS
	if(l_layer_ffa0==26){
		int gg=0;
	}
//	conv_desc.layer_debug = l_layer_ffa0;
#endif

	/****opcode********/
	//**** 0 - Conv
	//**** 1 - Conv+relu
	//**** 2 - dilConv
	//**** 3 - dilConv+relu
	//**** 4 - BN intra SOS
	//**** 5 - BN intra mean
	//**** 6 - LRN intra SOS
	//**** 7 - LRN inter SOS
	//**** 8 - L2 norm
	//**** 9 - LRN_PnS
	//**** 10- BN_SnB + Relu//Batch norm scale and bias
	//**** 11- Elem-wise Addition
	//**** 12- BN_SnB + Relu + Conv // BN + Conv Fuse
	//**** 13- BN_SnB
	ap_uint<8> opcode = (ap_uint<8>)l_opcode;

	//************************** Opcode control signals ************************//
	//
	if(opcode == 4)
		conv_desc.bn_intra_sos_enable = 1;
	else
		conv_desc.bn_intra_sos_enable = 0;

	if(opcode == 5)
		conv_desc.bn_intra_mean_enable = 1;
	else
		conv_desc.bn_intra_mean_enable = 0;

	if(opcode == 6)
		conv_desc.lrn_intra_sos_enable = 1;
	else
		conv_desc.lrn_intra_sos_enable = 0;

	if(opcode == 7)
		conv_desc.lrn_inter_sos_enable = 1;
	else
		conv_desc.lrn_inter_sos_enable = 0;

	if(opcode == 8)
		conv_desc.l2_sos_enable = 1;
	else
		conv_desc.l2_sos_enable = 0;

	if((opcode == 4)  || (opcode == 6) || (opcode == 7) || (opcode == 8))//BN_intra_sos,  lrn inter and lrn inter, l2 norm
		conv_desc.sos_enable = 1;
	else
		conv_desc.sos_enable = 0;

	if(opcode == 9)
		conv_desc.lrn_pns_enable = 1;
	else
		conv_desc.lrn_pns_enable = 0;

	if(opcode == 10)
	{
		conv_desc.bn_snb_enable = 1;
		conv_desc.norm_flag_set[0]=3;
		conv_desc.norm_flag_set[1]=4;
		conv_desc.norm_flag_set[2]=0;
		conv_desc.norm_flag_set[3]=0;
		conv_desc.norm_loop_cnt = 1;

	}
	else
		conv_desc.bn_snb_enable = 0;

	if(opcode == 11)
	{
		conv_desc.elem_wise_add_en = 1;
		conv_desc.norm_flag_set[0]=5;
		conv_desc.norm_flag_set[1]=6;
		conv_desc.norm_flag_set[2]=0;
		conv_desc.norm_flag_set[3]=0;
		conv_desc.norm_loop_cnt = 2;
	}
	else
		conv_desc.elem_wise_add_en = 0;
	// BN SnB + Conv
	if(opcode == 12)
	{
		conv_desc.bn_conv_fuse_en = 1;
		conv_desc.norm_flag_set[0]=7;
		conv_desc.norm_flag_set[1]=0;
		conv_desc.norm_flag_set[2]=0;
		conv_desc.norm_flag_set[3]=0;
		conv_desc.norm_loop_cnt = 1;
	}
	else
		conv_desc.bn_conv_fuse_en = 0;

	//
	//******************************* End *************************************//

#if 0
	fprintf(stderr,"\n--------------");
	for(int i=0;i<38;i++)
		fprintf(stderr,"\t%d",(int)scalar_conv_args[i]);
	//	fprintf(stderr,"\n--------------in_height:plane = %d:%d \tout_height:plane = %d:%d\t in_stgcnt = %d \tout_stgcnt = %d ",l_input_height_ffa0 ,l_input_planes_ffa0, l_output_height_ffa0, l_output_planes_ffa0,l_istg_row_cnt_ffa0, l_ostg_row_cnt_ffa0);
#endif
	ap_uint<16> input_planes_ffa0 = (ap_uint<16>)(l_input_planes_ffa0);
	ap_uint<3> check_4ker_ffa0 = input_planes_ffa0.range(1,0);
	bool add_4ker_ffa0;
	if(check_4ker_ffa0 == 0)
		add_4ker_ffa0 = 0;
	else
		add_4ker_ffa0 = 1;
	ap_uint<16> input_planes_design_ffa0 = ((input_planes_ffa0/4) + add_4ker_ffa0)*4;

	ap_uint<16> input_compute_planes_ffa0 = (ap_uint<16>)(l_compute_planes_ffa0);
	ap_uint<3> check_compute_4ker_ffa0 = input_compute_planes_ffa0.range(1,0);
	bool add_compute_4ker_ffa0;
	if(check_compute_4ker_ffa0 == 0)
		add_compute_4ker_ffa0 = 0;
	else
		add_compute_4ker_ffa0 = 1;
	ap_uint<16> compute_planes_design_ffa0 = ((input_compute_planes_ffa0/4) + add_compute_4ker_ffa0)*4;

	ap_uint<16> output_planes_ffa0;
	ap_uint<3> check_8ker_ffa0;
	bool add_8ker_ffa0;
	ap_uint<16> output_planes_design_ffa0 ;
#if XI_KER_PROC==8
	output_planes_ffa0 = (ap_uint<16>)(l_output_planes_ffa0);
	check_8ker_ffa0 = output_planes_ffa0.range(2,0);
	if(check_8ker_ffa0 == 0)
		add_8ker_ffa0 = 0;
	else
		add_8ker_ffa0 = 1;
	output_planes_design_ffa0 = ((output_planes_ffa0/8) + add_8ker_ffa0)*8;
#elif XI_KER_PROC==4  || XI_KER_PROC==1 || XI_KER_PROC==2
	output_planes_ffa0 = (ap_uint<16>)(l_output_planes_ffa0);
	check_8ker_ffa0 = output_planes_ffa0.range(1,0);
	if(check_8ker_ffa0 == 0)
		add_8ker_ffa0 = 0;
	else
		add_8ker_ffa0 = 1;
	output_planes_design_ffa0 = ((output_planes_ffa0/4) + add_8ker_ffa0)*4;
#endif

	input_desc.width = (ap_uint<16>)(l_input_Width_ffa0);
	input_desc.height = (ap_uint<16>)(l_input_height_ffa0);
	input_desc.size = (ap_uint<32>)(l_input_height_ffa0 * l_input_Width_ffa0);
	input_desc.planes = (ap_uint<16>)(input_planes_design_ffa0);
	input_desc.compute_planes = (ap_uint<16>)compute_planes_design_ffa0;
	input_desc.mean_value[0] = (short)l_mean_value0_ffa0 ;
	input_desc.mean_value[1] = (short)l_mean_value1_ffa0 ;
	input_desc.mean_value[2] = (short)l_mean_value2_ffa0 ;
	input_desc.mean_value[3] = (short)l_mean_value3_ffa0 ;
	input_desc.mean_sub_flag = l_mean_sub_flag_ffa0;

	output_desc.width = (ap_uint<16>)l_output_width_ffa0;
	output_desc.height = (ap_uint<16>)l_output_height_ffa0;
	output_desc.size = (ap_uint<32>)(l_output_height_ffa0 * l_output_width_ffa0);
	output_desc.planes = (ap_uint<16>)(output_planes_design_ffa0);
	output_desc.out_offset = l_out_offset_ffa0;
	output_desc.bias_offset = l_bias_offset_ffa0;

	weight_desc.num_kernels = output_planes_design_ffa0;
	weight_desc.filter_size = (ap_uint<8>)(l_filter_height_ffa0);
	weight_desc.filter_size_square =  (ap_uint<8>)(l_filter_height_ffa0)*(ap_uint<8>)(l_filter_height_ffa0);
	if(conv_desc.lrn_inter_sos_enable == 1)
	{
		weight_desc.nkpf = 1;
		weight_desc.filter_stride = 1;
	}
	else
	{
		weight_desc.nkpf = (ap_uint<4>)(l_nkpf_ffa0);
		weight_desc.filter_stride = (ap_uint<4>)(l_conv_stride_ffa0);
	}

//	weight_desc.slk_en = l_single_load_kernel_ffa0;
	weight_desc.slk_counter = l_single_load_kernel_ffa0;
	weight_desc.weight_offset = l_weight_offset_ffa0;
	weight_desc.dilation_factor = (ap_uint<3>)l_dilation_factor_ffa0;

	if(conv_desc.lrn_inter_sos_enable == 1)
		weight_desc.filter_size_dilated = 1;
	else
		weight_desc.filter_size_dilated = (weight_desc.dilation_factor*(weight_desc.filter_size-1)) + 1;

	conv_desc.group_en = (bool)(l_group_ffa0);
	conv_desc.relu_en = (bool)l_relu_ffa0;
	if(l_layer_ffa0==0)
		conv_desc.layer_id = 0;
	else
		conv_desc.layer_id = 1;
	conv_desc.inout_precision = (ap_uint<5>)(l_inout_precision_ffa0);
	//conv_desc.shift_precision1 = (ap_uint<4>)(l_shift_precision1_ffa0);
	conv_desc.shift_precision2 = (ap_uint<4>)(l_shift_precision2_ffa0);
	conv_desc.pad_en = (bool)l_pad_ffa0;
	//	conv_desc.pad_num = (ap_uint<8>)l_pad_ffa0;

	if(conv_desc.lrn_inter_sos_enable == 1)
	{
		conv_desc.pad_num = 0;
	}
	else
	{
		conv_desc.pad_num = (ap_uint<8>)l_pad_ffa0;
	}
	conv_desc.istg_row_count = (ap_uint<16>)(l_istg_row_cnt_ffa0);
	conv_desc.ostg_row_count = (ap_uint<16>)(l_ostg_row_cnt_ffa0);
	conv_desc.straddle = l_straddle_factor_ffa0;
	//	conv_desc.compute_loop_count = weight_desc.filter_size_square*(input_desc.compute_planes.range(11,2));
	ap_uint<32> process_pixels_ffa0 = output_desc.width*(ap_uint<16>)(l_ostg_row_cnt_ffa0);
	//	conv_desc.buff_split =(bool)l_buffer_split_ffa0;
	conv_desc.fsz_by_stride = (ap_uint<8>)(weight_desc.filter_size_dilated/weight_desc.filter_stride);
	conv_desc.fsz_mod_stride = (ap_uint<8>)(weight_desc.filter_size_dilated%weight_desc.filter_stride);
	conv_desc.read_from_ddr = (bool)l_read_from_ddr;
	conv_desc.write_to_ddr = (bool)l_write_to_ddr;

//	conv_desc.bn_en = (bool)l_bn_en;
	conv_desc.norm_k = (ap_int<16>)l_norm_k;
	//conv_desc.mode = (ap_uint<4>)l_mode;
	//conv_desc.elem_wise_offset = (ap_uint<32>)l_elem_wise_offset;
	//conv_desc.bn_mean_offset = (ap_uint<32>)l_bn_mean_offset;

	conv_desc.mul_in = (ap_uint<18>) l_mul_in;
	conv_desc.norm_prec = (ap_uint<4>) l_norm_prec;
	/*
#if XI_OSTAGEBUFF_PIX==4
	if(process_pixels_ffa0.range(1,0)==0)
		conv_desc.out_pix=process_pixels_ffa0;
	else if(process_pixels_ffa0.range(1,0)==1)
		conv_desc.out_pix=process_pixels_ffa0+3;
	else if(process_pixels_ffa0.range(1,0)==2)
		conv_desc.out_pix=process_pixels_ffa0+2;
	else if(process_pixels_ffa0.range(1,0)==3)
		conv_desc.out_pix=process_pixels_ffa0+1;
#elif XI_OSTAGEBUFF_PIX==2
	if(process_pixels_ffa0[0]==0)
		conv_desc.out_pix=process_pixels_ffa0;
	else if(process_pixels_ffa0[0]==1)
		conv_desc.out_pix=process_pixels_ffa0+1;
#else
	conv_desc.out_pix=process_pixels_ffa0;
#endif// XI_OSTAGEBUFF_PIX
	 */
	conv_desc.out_pix=process_pixels_ffa0;

	if( (opcode == 4) || (opcode == 5) || (opcode == 6))//BN_intra sos, BN_intra mean, LRN intra sos
		conv_desc.mac_loop_count = 4;
	else// lrn inter , conv, l2 norm
		conv_desc.mac_loop_count = 1;


	conv_desc.opcode = opcode;
	conv_desc.norm_prec_2 = (ap_uint<4>)l_norm_prec_2;
	conv_desc.norm_prec_3 = (ap_uint<4>)l_norm_prec_3;

	if(opcode == 6 || opcode == 7 || opcode == 8)//LRN_intersos, LRN_intrasos, l2_sos
		conv_desc.pix_per_kp = 6;
	else if(opcode == 4 || opcode == 5)//BN man, BN sos
		conv_desc.pix_per_kp = 1;
	else //conv, convRLU, DilConv, DilConvRLU
		conv_desc.pix_per_kp = XI_PIX_PROC;

	if(opcode == 6)//LRN_intersos
		conv_desc.loop_bound_1x1_filter = 1;
	else
		conv_desc.loop_bound_1x1_filter = input_desc.compute_planes.range(15,3);


	if(conv_desc.lrn_inter_sos_enable)
	{
		conv_desc.feeding_buff_plane_loop_bound = 16;
		conv_desc.feeding_buff_row_loop_bound = 1;
	}
	else//conv, convRLU, DilConv, DilConvRLU
	{
		conv_desc.feeding_buff_plane_loop_bound = input_desc.compute_planes.range(15,2);
		conv_desc.feeding_buff_row_loop_bound = weight_desc.filter_size;
	}

	if(conv_desc.lrn_inter_sos_enable == 1 || conv_desc.lrn_intra_sos_enable == 1 )
	{
		conv_desc.straddle_in_plane_inc_by4 = XI_KER_PROC/4;
	}
	else//conv
	{
		conv_desc.straddle_in_plane_inc_by4 = input_desc.compute_planes.range(11,2);
	}

	conv_desc.pix_by_outwidth = conv_desc.pix_per_kp/output_desc.width;
	conv_desc.pix_mod_outwidth = conv_desc.pix_per_kp%output_desc.width;
	conv_desc.pix2_div_outwidth =(conv_desc.pix_per_kp/2)/output_desc.width;
	conv_desc.pix2_mod_outwidth =(conv_desc.pix_per_kp/2)%output_desc.width;

	if(conv_desc.lrn_inter_sos_enable == 1 )
		conv_desc.compute_loop_count = 4;//16 feature map to accumulate
	else if(conv_desc.lrn_intra_sos_enable == 1)
		conv_desc.compute_loop_count = weight_desc.filter_size_square;
	else
		conv_desc.compute_loop_count = weight_desc.filter_size_square*(input_desc.compute_planes.range(11,2));

	conv_desc.intra_en = (conv_desc.bn_intra_mean_enable || conv_desc.bn_intra_sos_enable || conv_desc.lrn_intra_sos_enable);

	if(conv_desc.pix_per_kp < XI_PIX_PROC){
		conv_desc.adder_loop_cnt = 1;
		conv_desc.ker_loop_cnt =  1;
	}else{
		conv_desc.adder_loop_cnt = XI_PIX_PROC;
		conv_desc.ker_loop_cnt =  weight_desc.num_kernels;
	}
	//****************************NOTE*******************
	//in BN intra assume, input height and width < 16 and filter size is same as input size
	//L2 norm, same as conv but we have to use 6 pix_proc instead of 52 pix_proc

}

void OffsetMemPointer_ffb(input_struct input_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		conv_struct conv_desc,
		ap_uint<32> &input_group_offset_1st_fa0,
		ap_uint<32> &input_group_offset_other_fa0,
		ap_uint<32> &weights_group_offset_fa0,
		ap_uint<32> &output_group_offset_fa0,
		ap_uint<32> &bias_group_offset_fa0)
{
#pragma HLS INLINE

	int I_hw_ffb0;
#pragma HLS RESOURCE variable=I_hw_ffb0 core=MulnS latency=2
	I_hw_ffb0 = input_desc.size;
	int I_hwp_ffb0;
#pragma HLS RESOURCE variable=I_hwp_ffb0 core=MulnS latency=2
	I_hwp_ffb0 = I_hw_ffb0*input_desc.planes;

	int K_nkp_ffb0;
#pragma HLS RESOURCE variable=K_nkp_ffb0 core=MulnS latency=2
	K_nkp_ffb0 = weight_desc.num_kernels*input_desc.planes;
	int K_nkpfsz2_ffb0;
#pragma HLS RESOURCE variable=K_nkpfsz2_ffb0 core=MulnS latency=2
#if XI_KER_PROC==8
	K_nkpfsz2_ffb0 = K_nkp_ffb0*(weight_desc.filter_size_square);
#elif XI_KER_PROC==4
#if XI_WEIGHTS_8B
	K_nkpfsz2_ffb0 = K_nkp_ffb0*(weight_desc.filter_size_square);
#else
	K_nkpfsz2_ffb0 = K_nkp_ffb0*(weight_desc.filter_size_square+1);
#endif
#elif XI_KER_PROC==1
	K_nkpfsz2_ffb0 = K_nkp_ffb0*(weight_desc.filter_size_square);
#endif

	int O_hw_ffb0;
#pragma HLS RESOURCE variable=O_hw_ffb0 core=MulnS latency=2
	O_hw_ffb0 = output_desc.height*output_desc.width;
	int O_hwp_ffb0;
#pragma HLS RESOURCE variable=O_hwp_ffb0 core=MulnS latency=2
	O_hwp_ffb0 = O_hw_ffb0*output_desc.planes;

	if(conv_desc.group_en == 0)
	{
		input_group_offset_1st_fa0 = 0;
		input_group_offset_other_fa0 = 0;
		weights_group_offset_fa0 = 0;
		output_group_offset_fa0 = 0;
		bias_group_offset_fa0 = 0;
	}
	else
	{
		input_group_offset_1st_fa0 = (I_hwp_ffb0)>>XI_INPUTPACKCOUNT_LOG2;
		/*if(conv_desc.buff_split==0)
		{
			input_group_offset_other_fa0 = (I_hwp_ffb0)>>XI_INPUTPACKCOUNT2_LOG2;
			output_group_offset_fa0 = (O_hwp_ffb0)>>XI_OUTPUTPACKCOUNT_LOG2;
		}
		else
		{
		}*/
		input_group_offset_other_fa0 = (I_hwp_ffb0)>>(XI_INPUTPACKCOUNT2_LOG2+1);
		output_group_offset_fa0 = (O_hwp_ffb0)>>(XI_OUTPUTPACKCOUNT_LOG2+1);
		weights_group_offset_fa0 = (K_nkpfsz2_ffb0)>>(XI_WEIGHTPACKCOUNT_LOG2+1);
		bias_group_offset_fa0 = (output_desc.planes)>>XI_BIASPACKCOUNT_LOG2;
	}

}


void CopyOstgToIstg(
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		short ostaging_buff0_fb0[4][8][XI_OSTAGEBUFF_DEPTH],
		conv_struct conv_desc,
		output_struct output_desc
)
{
#pragma HLS INLINE OFF


#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=2

#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=3

	//if(!(conv_desc.write_to_ddr))
	//{

	//int dim = conv_desc.read_from_ddr;
	//istaging_buff0_fb0[dim][dim][dim] = ostaging_buff0_fb0[dim][dim][dim][dim];
#if 0
	//debug
	fprintf(stderr,"\n");
	for(int dim1=0;dim1 <4;dim1++)
	{
		for(int dim2=0;dim2<8;dim2++)
		{
			int temp = (int)ostaging_buff0_fb0[dim1][dim2][0][0];
			fprintf(stderr,"ostg[%d][%d] = %d  ",dim1,dim2,temp);
		}
	}
	fprintf(stderr,"\n");

	//debug
#endif


	ap_uint<16> loop_bound = output_desc.size;
	copy_to_istg_loop1:
	for(ap_uint<16> planes =0;planes<output_desc.planes;planes+=32)
	{
		copy_to_istg_loop2:
		for(ap_uint<16> size = 0;size< loop_bound;size++)
		{

#pragma HLS PIPELINE
#pragma HLS loop_flatten

			ap_uint<16> bram_depth = size + (planes/32) *loop_bound;
			//ap_uint<16> ostg_bram_depth = size + (planes/32) *conv_desc.out_pix;

			copy_to_istg_loop3:
			for(int dim2=0;dim2<8;dim2++)
			{
#pragma HLS UNROLL
				for(int dim1=0;dim1<4;dim1++)
				{
#pragma HLS UNROLL

					ap_uint<16> temp = ostaging_buff0_fb0[dim1][dim2][bram_depth];
					if(temp[15] == 1)
						istaging_buff0_fb0[dim1][dim2][bram_depth] = 0;
					else
						istaging_buff0_fb0[dim1][dim2][bram_depth] = temp;

				}//dim2
			}//dim1


		} //copy_to_istg_loop
	}


#if 0
	//debug
	fprintf(stderr,"\n");
	for(int dim1=0;dim1 <4;dim1++)
	{
		for(int dim2=0;dim2<8;dim2++)
		{
			int temp = (int)istaging_buff0_fb0[dim1][dim2][0];
			fprintf(stderr,"istg[%d][%d] = %d  ",dim1,dim2,temp);
		}
	}
	fprintf(stderr,"\n");
	//debug
#endif

}//if


//}

template<int CFILTER_SIZE,int CCONV_STRIDE,int CNUM_KERNELS, int IINPUT_PLANES,int PNKPF>
void LoadKernels_fz(weight_struct weight_desc,
		weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],
		gmem_weighttype *gmem_weight1_fa0,
		gmem_weighttype *gmem_weight2_fa0,
		ap_uint<14> weight_loopCount_fd0)
{
#pragma HLS INLINE OFF
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=2

#if XI_KER_PROC==8


	Weight_Loop:
	for(ap_uint<14> ddrpntr_fz0=0;ddrpntr_fz0< weight_loopCount_fd0;ddrpntr_fz0++)
	{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=128 max=128

		ap_uint<256> weight_256bit_fz0 ;
		//fprintf(stderr,"\n");
		weight_256bit_fz0.range(127,0) = gmem_weight1_fa0[ddrpntr_fz0];
		weight_256bit_fz0.range(255,128) = gmem_weight2_fa0[ddrpntr_fz0];

		ap_uint<128> wt1 = gmem_weight1_fa0[ddrpntr_fz0];
		ap_uint<128> wt2 = gmem_weight2_fa0[ddrpntr_fz0];

		/*	for(int i=0,bit=0;i<16;i++,bit+=8)
		{
			ap_int<8> wta = wt1.range((bit+7),bit);
			fprintf(stderr,"wt[%d] = %d\t",(int)i,(int)wta);
		}
		for(int i=0,bit=0;i<16;i++,bit+=8)
			{
				ap_int<8> wta = wt2.range((bit+7),bit);
				fprintf(stderr,"wt[%d] = %d\t",(int)i,(int)wta);
			}
		 */
		Weighttype_Loop:
		for(ap_uint<10> word_cnt_fz0=0,bit=0 ;word_cnt_fz0<32;word_cnt_fz0++,bit+=8)
		{
#pragma HLS UNROLL
			//ap_int<8> wt = weight_256bit_fz0.range((bit+7),bit);
			//fprintf(stderr,"wt[%d] = %d\t",(int)word_cnt_fz0,(int)wt);
			weight_buff0_fd0[word_cnt_fz0.range(4,2)][word_cnt_fz0.range(1,0)][ddrpntr_fz0] = weight_256bit_fz0.range((bit+7),bit);
		}//Weighttype_Loop

		int tt=0;
	}//Weight_Loop


	/*	Weight_Loop:
	for(ap_uint<14> ddrpntr_fz0=0;ddrpntr_fz0< weight_loopCount_fd0;ddrpntr_fz0++)
	{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=128 max=128

		weighttype weight_ddr_64bit_fz0 ;

		weight_ddr_64bit_fz0 = gmem_weight_fa0[ddrpntr_fz0];
		Weighttype_Loop:
		for(ap_uint<10> word_cnt_fz0=0,bit=0 ;word_cnt_fz0<32;word_cnt_fz0++,bit+=8)
		{
#pragma HLS UNROLL
			weight_buff0_fd0[word_cnt_fz0.range(4,2)][word_cnt_fz0.range(1,0)][ddrpntr_fz0] = weight_ddr_64bit_fz0.range((bit+7),bit);
		}//Weighttype_Loop
	}//Weight_Loop
	 */
#elif XI_KER_PROC==4
#if XI_WEIGHTS_8B
	for(ap_uint<14> ddrpntr_fz5=0;ddrpntr_fz5< weight_loopCount_fd0;ddrpntr_fz5++)
	{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=61*8 max=61*8
		weighttype weight_ddr_128bit_fz0 ;
		weight_ddr_128bit_fz0 = gmem_weight_fa0[ddrpntr_fz5];
		for(ap_uint<10> word_cnt_fz5=0,bit=0;word_cnt_fz5<16;word_cnt_fz5++,bit+=8)
		{
#pragma HLS UNROLL

			weight_buff0_fd0[word_cnt_fz5.range(3,2)][word_cnt_fz5.range(1,0)][ddrpntr_fz5] = weight_ddr_128bit_fz0.range((bit+7),bit);


		}
	}

#else
	ap_uint<10> fsz2_p1_by2 = (weight_desc.filter_size_square+1)/2;
	Weight_Loop:
	for(ap_uint<14> ddrpntr_fz2=0;ddrpntr_fz2< weight_loopCount_fd0;ddrpntr_fz2++)
	{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=160 max=160

		ap_uint<14> pad_count;
#pragma HLS resource variable=pad_count core=DivnS
		pad_count = ddrpntr_fz2/fsz2_p1_by2;

		ap_uint<14> bram_offset = (2*ddrpntr_fz2) - pad_count;
		weighttype weight_ddr_64bit_fz2 ;
		weight_ddr_64bit_fz2 = gmem_weight_fa0[ddrpntr_fz2];

		Weighttype_Loop:
		for(ap_uint<10> word_cnt_fz2=0,bit=0, bit2=256;word_cnt_fz2<16;word_cnt_fz2++,bit+=16, bit2+=16)
		{
#pragma HLS UNROLL
			weight_buff0_fd0[word_cnt_fz2.range(3,2)][word_cnt_fz2.range(1,0)][bram_offset] = weight_ddr_64bit_fz2.range((bit+15),bit);
			weight_buff0_fd0[word_cnt_fz2.range(3,2)][word_cnt_fz2.range(1,0)][bram_offset + 1] = weight_ddr_64bit_fz2.range((bit2+15),bit2);
		}//Weighttype_Loop
	}//Weight_Loop
#endif
#elif XI_KER_PROC==1

	Weight_Loop:
	for(ap_uint<14> ddrpntr_fz3=0;ddrpntr_fz3< weight_loopCount_fd0;ddrpntr_fz3++)
	{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=61 max=61

		weighttype weight_ddr_64bit_fz3 ;
		weight_ddr_64bit_fz3 = gmem_weight_fa0[ddrpntr_fz3];
		Weighttype_Loop:
		for(ap_uint<10> word_cnt_fz3=0,bit=0;word_cnt_fz3<4;word_cnt_fz3++,bit+=8)
		{
#pragma HLS UNROLL
			weight_buff0_fd0[0][word_cnt_fz3][ddrpntr_fz3] = weight_ddr_64bit_fz3.range((bit+7),bit);

		}//Weighttype_Loop
	}//Weight_Loop

#elif XI_KER_PROC==2
	Weight_Loop:
	for(ap_uint<14> ddrpntr_fz4=0;ddrpntr_fz4< weight_loopCount_fd0;ddrpntr_fz4++)
	{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=61 max=61

		weighttype weight_ddr_64bit_fz4 ;
		weight_ddr_64bit_fz4 = gmem_weight_fa0[ddrpntr_fz4];

#if !XI_WEIGHTS_8B
		bool dim1 = ddrpntr_fz4%2;
		ap_uint<12> dim3 = ddrpntr_fz4/2;
		Weighttype_Loop:
		for(ap_uint<10> word_cnt_fz4=0,bit=0;word_cnt_fz4<4;word_cnt_fz4++,bit+=16)
		{
#pragma HLS UNROLL
			weight_buff0_fd0[dim1][word_cnt_fz4.range(1,0)][dim3] = weight_ddr_64bit_fz4.range((bit+15),bit);
		}//Weighttype_Loop

#else
		Weighttype_Loop:
		for(ap_uint<10> word_cnt_fz4=0,bit=0;word_cnt_fz4<8;word_cnt_fz4++,bit+=8)
		{
#pragma HLS UNROLL
			weight_buff0_fd0[word_cnt_fz4[2]][word_cnt_fz4.range(1,0)][ddrpntr_fz4] = weight_ddr_64bit_fz4.range((bit+7),bit);
		}//Weighttype_Loop
#endif//XI_WEIGHTS_8B
	}//Weight_Loop


#endif//XI_KER_PROC

}

template<int CFILTER_SIZE,int CCONV_STRIDE,int CNUM_KERNELS, int IINPUT_PLANES,int PNKPF>
void LoadKernelsEn_fz(weight_struct weight_desc,
		weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],
		gmem_weighttype *gmem_weight1_fa0,
		gmem_weighttype *gmem_weight2_fa0,
		ap_uint<14> weight_loopCount_fd0,
		bool lk_enable)
{
#pragma HLS INLINE OFF

	if(lk_enable == 1)
	{
		LoadKernels_fz<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>(weight_desc,weight_buff0_fd0,gmem_weight1_fa0 ,gmem_weight2_fa0 , weight_loopCount_fd0);
	}
}

void InputBuffCopy(
		short input_buf_copy_fy0[4][XI_PIX_PROC],
		short input_buf_copy_reg_fy0[4][XI_PIX_PROC]
)
{
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_reg_fy0 complete dim=0

#pragma HLS interface register port=input_buf_copy_reg_fy0
#pragma HLS INLINE off


	Pln_Loop1:
	for(ap_uint<3> plane_fy1=0; plane_fy1<4;plane_fy1++)
	{
#pragma HLS unroll
		Pix_Loop1:
		for(ap_uint<8> pixel_fy1=0; pixel_fy1<(XI_PIX_PROC);pixel_fy1++)
		{
#pragma HLS unroll
			input_buf_copy_reg_fy0[plane_fy1][pixel_fy1] = input_buf_copy_fy0[plane_fy1][pixel_fy1];

		}//Pix_Loop1
	}//Pln_Loop1
}

void InputBuffCopy_1(
		short input_buf_copy_fy0[4][XI_PIX_PROC],
		short input_buf_copy_reg_fy0[4][XI_PIX_PROC]
)
{
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_reg_fy0 complete dim=0

#pragma HLS interface register port=input_buf_copy_reg_fy0
#pragma HLS INLINE off


	Pln_Loop1:
	for(ap_uint<3> plane_fy1=0; plane_fy1<4;plane_fy1++)
	{
#pragma HLS unroll
		Pix_Loop1:
		for(ap_uint<8> pixel_fy1=0; pixel_fy1<(XI_PIX_PROC);pixel_fy1++)
		{
#pragma HLS unroll
			input_buf_copy_reg_fy0[plane_fy1][pixel_fy1] = input_buf_copy_fy0[plane_fy1][pixel_fy1];

		}//Pix_Loop1
	}//Pln_Loop1
}


void KBuffCopy(weight_width k_buf_copy_0_fy0[4],
		weight_width k_buf_copy_1_fy0[4],
		weight_width k_buf_copy_2_fy0[4],
		weight_width k_buf_copy_3_fy0[4],
		weight_width k_buf_copy_4_fy0[4],
		weight_width k_buf_copy_5_fy0[4],
		weight_width k_buf_copy_6_fy0[4],
		weight_width k_buf_copy_7_fy0[4],
		weight_width k_buf_copy_0_reg_fy0[4],
		weight_width k_buf_copy_1_reg_fy0[4],
		weight_width k_buf_copy_2_reg_fy0[4],
		weight_width k_buf_copy_3_reg_fy0[4],
		weight_width k_buf_copy_4_reg_fy0[4],
		weight_width k_buf_copy_5_reg_fy0[4],
		weight_width k_buf_copy_6_reg_fy0[4],
		weight_width k_buf_copy_7_reg_fy0[4])
{
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_0_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_1_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_2_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_3_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_4_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_5_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_6_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_7_reg_fy0 complete dim=0

#pragma HLS ARRAY_PARTITION variable=k_buf_copy_0_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_1_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_2_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_3_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_4_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_5_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_6_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_7_fy0 complete dim=0

#pragma HLS interface register port=k_buf_copy_0_reg_fy0
#pragma HLS interface register port=k_buf_copy_1_reg_fy0
#pragma HLS interface register port=k_buf_copy_2_reg_fy0
#pragma HLS interface register port=k_buf_copy_3_reg_fy0
#pragma HLS interface register port=k_buf_copy_4_reg_fy0
#pragma HLS interface register port=k_buf_copy_5_reg_fy0
#pragma HLS interface register port=k_buf_copy_6_reg_fy0
#pragma HLS interface register port=k_buf_copy_7_reg_fy0

#pragma HLS INLINE off

	for(ap_uint<3> i=0;i<4;i++)
	{
#pragma HLS UNROLL
		k_buf_copy_0_reg_fy0[i] = k_buf_copy_0_fy0[i];
		k_buf_copy_1_reg_fy0[i] = k_buf_copy_1_fy0[i];
		k_buf_copy_2_reg_fy0[i] = k_buf_copy_2_fy0[i];
		k_buf_copy_3_reg_fy0[i] = k_buf_copy_3_fy0[i];
		k_buf_copy_4_reg_fy0[i] = k_buf_copy_4_fy0[i];
		k_buf_copy_5_reg_fy0[i] = k_buf_copy_5_fy0[i];
		k_buf_copy_6_reg_fy0[i] = k_buf_copy_6_fy0[i];
		k_buf_copy_7_reg_fy0[i] = k_buf_copy_7_fy0[i];
	}



}
template<int CFILTER_SIZE,int CCONV_STRIDE,int CNUM_KERNELS,int IINPUT_PLANES,int PNKPF>
void Compute8Ker_fy(conv_struct conv_desc,
		weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024],
		ap_uint<4> nkpf_cnt_fe0,
		short result_0_ping_fe0[4][XI_PIX_PROC],
		short result_1_ping_fe0[4][XI_PIX_PROC],
		short result_2_ping_fe0[4][XI_PIX_PROC],
		short result_3_ping_fe0[4][XI_PIX_PROC],
		short result_4_ping_fe0[4][XI_PIX_PROC],
		short result_5_ping_fe0[4][XI_PIX_PROC],
		short result_6_ping_fe0[4][XI_PIX_PROC],
		short result_7_ping_fe0[4][XI_PIX_PROC])
{
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=2
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM

#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=2

#pragma HLS ARRAY_PARTITION variable=result_0_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_1_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_2_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_3_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_4_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_5_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_6_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_7_ping_fe0 complete dim=1
#pragma HLS resource variable=result_0_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_1_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_2_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_3_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_4_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_5_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_6_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_7_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS INLINE off


	ap_int<32> mac_res_temp_0_fy0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_0_fy0 complete dim=0
	ap_int<32> mac_res_temp_1_fy0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_1_fy0 complete dim=0
	ap_int<32> mac_res_temp_2_fy0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_2_fy0 complete dim=0
	ap_int<32> mac_res_temp_3_fy0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_3_fy0 complete dim=0
	ap_int<32> mac_res_temp_4_fy0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_4_fy0 complete dim=0
	ap_int<32> mac_res_temp_5_fy0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_5_fy0 complete dim=0
	ap_int<32> mac_res_temp_6_fy0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_6_fy0 complete dim=0
	ap_int<32> mac_res_temp_7_fy0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_7_fy0 complete dim=0

	ap_uint<12> k_off_cnt_fy0;
	k_off_cnt_fy0 = (conv_desc.compute_loop_count*nkpf_cnt_fe0);

	Pix_Loop:
	for(ap_uint<8> pixel_fy0=0;pixel_fy0<XI_PIX_PROC;pixel_fy0++)
	{
#pragma HLS unroll
		Pln_Loop:
		for(ap_uint<4> plane_fy0=0;plane_fy0<4;plane_fy0++)
		{
#pragma HLS unroll
			mac_res_temp_0_fy0[pixel_fy0][plane_fy0] = 0;
			mac_res_temp_1_fy0[pixel_fy0][plane_fy0] = 0;
			mac_res_temp_2_fy0[pixel_fy0][plane_fy0] = 0;
			mac_res_temp_3_fy0[pixel_fy0][plane_fy0] = 0;
			mac_res_temp_4_fy0[pixel_fy0][plane_fy0] = 0;
			mac_res_temp_5_fy0[pixel_fy0][plane_fy0] = 0;
			mac_res_temp_6_fy0[pixel_fy0][plane_fy0] = 0;
			mac_res_temp_7_fy0[pixel_fy0][plane_fy0] = 0;
		}//Pln_Loop
	}//Pix_Loop

#if 0//DEBUG_COMPUTE
	fprintf(fp_input,"\n Current Plane id:%d\n",(int)current_plane);
	fprintf(fp_weight,"\n Current Plane id:%d\n",(int)current_plane);
#endif

	Compute_Loop:
	for(ap_uint<12> inp_buff_addr1_fy0=0, inp_buff_addr2_fy0=512 ,offset_kbuf_fy0 = k_off_cnt_fy0;inp_buff_addr1_fy0<conv_desc.compute_loop_count;
			/*..continue..*/inp_buff_addr1_fy0++,inp_buff_addr2_fy0++,offset_kbuf_fy0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=16 max=16
#pragma HLS PIPELINE

		short input_buf_copy_fy0[4][XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_fy0 complete dim=0

		short input_buf_copy_reg_fy0[4][XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_reg_fy0 complete dim=0

		short input_buf_copy_reg_1_fy0[4][XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_reg_1_fy0 complete dim=0

		short input_buf_copy_reg_dup_fy0[4][XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_reg_dup_fy0 complete dim=0

		short input_buf_copy_reg_dup_1_fy0[4][XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_reg_dup_1_fy0 complete dim=0




		Pln_Loop1:
		for(ap_uint<3> plane_fy1=0; plane_fy1<4;plane_fy1++)
		{
#pragma HLS unroll
			Pix_Loop1:
			for(ap_uint<8> pixel_fy1=0, pix_split_fy0 =0; pixel_fy1<(XI_PIX_PROC/2);pixel_fy1++, pix_split_fy0++)
			{
#pragma HLS unroll
				input_buf_copy_fy0[plane_fy1][pix_split_fy0] = input_buff0_fc0[plane_fy1][pixel_fy1][inp_buff_addr1_fy0];
				input_buf_copy_fy0[plane_fy1][pix_split_fy0+(XI_PIX_PROC/2)] = input_buff0_fc0[plane_fy1][pixel_fy1][inp_buff_addr2_fy0];
			}//Pix_Loop1
		}//Pln_Loop1



		InputBuffCopy(input_buf_copy_fy0,input_buf_copy_reg_fy0);

		InputBuffCopy_1(input_buf_copy_fy0,input_buf_copy_reg_1_fy0);

		InputBuffCopy(input_buf_copy_reg_fy0,input_buf_copy_reg_dup_fy0);

		InputBuffCopy_1(input_buf_copy_reg_1_fy0,input_buf_copy_reg_dup_1_fy0);


		weight_width k_buf_copy_0_fy0[4];
		weight_width k_buf_copy_1_fy0[4];
		weight_width k_buf_copy_2_fy0[4];
		weight_width k_buf_copy_3_fy0[4];
		weight_width k_buf_copy_4_fy0[4];
		weight_width k_buf_copy_5_fy0[4];
		weight_width k_buf_copy_6_fy0[4];
		weight_width k_buf_copy_7_fy0[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_0_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_1_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_2_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_3_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_4_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_5_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_6_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_7_fy0 complete dim=0

		weight_width k_buf_copy_0_reg_fy0[4];
		weight_width k_buf_copy_1_reg_fy0[4];
		weight_width k_buf_copy_2_reg_fy0[4];
		weight_width k_buf_copy_3_reg_fy0[4];
		weight_width k_buf_copy_4_reg_fy0[4];
		weight_width k_buf_copy_5_reg_fy0[4];
		weight_width k_buf_copy_6_reg_fy0[4];
		weight_width k_buf_copy_7_reg_fy0[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_0_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_1_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_2_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_3_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_4_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_5_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_6_reg_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_7_reg_fy0 complete dim=0

		Pln_Loop2:
		for(ap_uint<3> plane_fy2=0; plane_fy2<4;plane_fy2++)
		{
#pragma HLS unroll
			k_buf_copy_0_fy0[plane_fy2] = weight_buff0_fd0[0][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_1_fy0[plane_fy2] = weight_buff0_fd0[1][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_2_fy0[plane_fy2] = weight_buff0_fd0[2][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_3_fy0[plane_fy2] = weight_buff0_fd0[3][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_4_fy0[plane_fy2] = weight_buff0_fd0[4][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_5_fy0[plane_fy2] = weight_buff0_fd0[5][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_6_fy0[plane_fy2] = weight_buff0_fd0[6][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_7_fy0[plane_fy2] = weight_buff0_fd0[7][plane_fy2][offset_kbuf_fy0];
		}//Pln_Loop2

		KBuffCopy(k_buf_copy_0_fy0,
				k_buf_copy_1_fy0,
				k_buf_copy_2_fy0,
				k_buf_copy_3_fy0,
				k_buf_copy_4_fy0,
				k_buf_copy_5_fy0,
				k_buf_copy_6_fy0,
				k_buf_copy_7_fy0,
				k_buf_copy_0_reg_fy0,
				k_buf_copy_1_reg_fy0,
				k_buf_copy_2_reg_fy0,
				k_buf_copy_3_reg_fy0,
				k_buf_copy_4_reg_fy0,
				k_buf_copy_5_reg_fy0,
				k_buf_copy_6_reg_fy0,
				k_buf_copy_7_reg_fy0);



#if 0// DEBUG_COMPUTE

		//************************************88
		short read = input_buf_copy_fy0[0][1];
		input_global[index_in]    = input_buf_copy_fy0[0][1];
		//	input_global[index_in+25] = input_buf_copy_fy0[1][0];
		//	input_global[index_in+50] = input_buf_copy_fy0[2][0];
		//	input_global[index_in+75] = input_buf_copy_fy0[3][0];

		index_in++;
		//	if(index_in%25 == 0)
		//		index_in=+75;
		//*************************************88

		//************************************88
		weights_global[index]    = k_buf_copy_0_fy0[0];
		weights_global[index+25] = k_buf_copy_1_fy0[0];
		weights_global[index+50] = k_buf_copy_2_fy0[0];
		weights_global[index+75] = k_buf_copy_3_fy0[0];
		index++;
		if(index%25 == 0)
			index+=75;
		//*************************************88
#endif

#if 0
		Pix_Loop2:
		for(int pixel_fy2=0; pixel_fy2<XI_PIX_PROC;pixel_fy2++)
		{
#pragma HLS unroll
			Pln_Loop3:
			for(int plane_fy3=0; plane_fy3<4;plane_fy3++)
			{
#pragma HLS unroll

				mac_res_temp_0_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_fy0[plane_fy3][pixel_fy2] * k_buf_copy_0_fy0[plane_fy3]);
				mac_res_temp_1_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_fy0[plane_fy3][pixel_fy2] * k_buf_copy_1_fy0[plane_fy3]);
				mac_res_temp_2_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_fy0[plane_fy3][pixel_fy2] * k_buf_copy_2_fy0[plane_fy3]);
				mac_res_temp_3_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_fy0[plane_fy3][pixel_fy2] * k_buf_copy_3_fy0[plane_fy3]);
				mac_res_temp_4_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_fy0[plane_fy3][pixel_fy2] * k_buf_copy_4_fy0[plane_fy3]);
				mac_res_temp_5_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_fy0[plane_fy3][pixel_fy2] * k_buf_copy_5_fy0[plane_fy3]);
				mac_res_temp_6_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_fy0[plane_fy3][pixel_fy2] * k_buf_copy_6_fy0[plane_fy3]);
				mac_res_temp_7_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_fy0[plane_fy3][pixel_fy2] * k_buf_copy_7_fy0[plane_fy3]);
			}//Pln_Loop3
		}//Pix_Loop2
#endif

		//***********************  KP0
		Pix_Loop_0:
		for(int pixel_fy2=0; pixel_fy2<XI_PIX_PROC;pixel_fy2++)
		{
#pragma HLS unroll
			Pln_Loop3_0:
			for(int plane_fy3=0; plane_fy3<4;plane_fy3++)
			{
#pragma HLS unroll
				short input_mux;
				if(pixel_fy2<6)
				{
					if(conv_desc.sos_enable == 1)
						input_mux = input_buf_copy_reg_dup_fy0[plane_fy3][pixel_fy2];
					else
						input_mux = k_buf_copy_0_reg_fy0[plane_fy3];
				}
				else
				{
					input_mux = k_buf_copy_0_reg_fy0[plane_fy3];
				}

				mac_res_temp_0_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_reg_dup_fy0[plane_fy3][pixel_fy2] * input_mux);
			}//Pln_Loop3
		}//Pix_Loop2

		//***********************  KP1
		Pix_Loop_1:
		for(int pixel_fy2=0; pixel_fy2<XI_PIX_PROC;pixel_fy2++)
		{
#pragma HLS unroll
			Pln_Loop3_1:
			for(int plane_fy3=0; plane_fy3<4;plane_fy3++)
			{
#pragma HLS unroll
				short input_mux;
				if((pixel_fy2>5) && (pixel_fy2<12))
				{
					if(conv_desc.sos_enable == 1)
						input_mux = input_buf_copy_reg_dup_fy0[plane_fy3][pixel_fy2];
					else
						input_mux = k_buf_copy_1_reg_fy0[plane_fy3];
				}
				else
				{
					input_mux = k_buf_copy_1_reg_fy0[plane_fy3];
				}

				mac_res_temp_1_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_reg_dup_fy0[plane_fy3][pixel_fy2] * input_mux);
			}//Pln_Loop3
		}//Pix_Loop2

		//***********************  KP2
		Pix_Loop_2:
		for(int pixel_fy2=0; pixel_fy2<XI_PIX_PROC;pixel_fy2++)
		{
#pragma HLS unroll
			Pln_Loop3_2:
			for(int plane_fy3=0; plane_fy3<4;plane_fy3++)
			{
#pragma HLS unroll
				short input_mux;
				if((pixel_fy2>11) && (pixel_fy2<18))
				{
					if(conv_desc.sos_enable == 1)
						input_mux = input_buf_copy_reg_dup_fy0[plane_fy3][pixel_fy2];
					else
						input_mux = k_buf_copy_2_reg_fy0[plane_fy3];
				}
				else
				{
					input_mux = k_buf_copy_2_reg_fy0[plane_fy3];
				}

				mac_res_temp_2_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_reg_dup_fy0[plane_fy3][pixel_fy2] * input_mux);
			}//Pln_Loop3
		}//Pix_Loop2

		//***********************  KP3
		Pix_Loop_3:
		for(int pixel_fy2=0; pixel_fy2<XI_PIX_PROC;pixel_fy2++)
		{
#pragma HLS unroll
			Pln_Loop3_3:
			for(int plane_fy3=0; plane_fy3<4;plane_fy3++)
			{
#pragma HLS unroll
				short input_mux;
				if((pixel_fy2>17) && (pixel_fy2<24))
				{
					if(conv_desc.sos_enable == 1)
						input_mux = input_buf_copy_reg_dup_fy0[plane_fy3][pixel_fy2];
					else
						input_mux = k_buf_copy_3_reg_fy0[plane_fy3];
				}
				else
				{
					input_mux = k_buf_copy_3_reg_fy0[plane_fy3];
				}

				if(pixel_fy2==22)
					int gghh=0;


				mac_res_temp_3_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_reg_dup_fy0[plane_fy3][pixel_fy2] * input_mux);
			}//Pln_Loop3
		}//Pix_Loop2

		//***********************  KP4
		Pix_Loop_4:
		for(int pixel_fy2=0; pixel_fy2<XI_PIX_PROC;pixel_fy2++)
		{
#pragma HLS unroll
			Pln_Loop3_4:
			for(int plane_fy3=0; plane_fy3<4;plane_fy3++)
			{
#pragma HLS unroll
				short input_mux;
				if((pixel_fy2>23) && (pixel_fy2<30))
				{
					if(conv_desc.sos_enable == 1)
						input_mux = input_buf_copy_reg_dup_1_fy0[plane_fy3][pixel_fy2];
					else
						input_mux = k_buf_copy_4_reg_fy0[plane_fy3];
				}
				else
				{
					input_mux = k_buf_copy_4_reg_fy0[plane_fy3];
				}

				mac_res_temp_4_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_reg_dup_1_fy0[plane_fy3][pixel_fy2] * input_mux);
			}//Pln_Loop3
		}//Pix_Loop2

		//***********************  KP5
		Pix_Loop_5:
		for(int pixel_fy2=0; pixel_fy2<XI_PIX_PROC;pixel_fy2++)
		{
#pragma HLS unroll
			Pln_Loop3_5:
			for(int plane_fy3=0; plane_fy3<4;plane_fy3++)
			{
#pragma HLS unroll
				short input_mux;
				if((pixel_fy2>29) && (pixel_fy2<36))
				{
					if(conv_desc.sos_enable == 1)
						input_mux = input_buf_copy_reg_dup_1_fy0[plane_fy3][pixel_fy2];
					else
						input_mux = k_buf_copy_5_reg_fy0[plane_fy3];
				}
				else
				{
					input_mux = k_buf_copy_5_reg_fy0[plane_fy3];
				}

				mac_res_temp_5_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_reg_dup_1_fy0[plane_fy3][pixel_fy2] * input_mux);
			}//Pln_Loop3
		}//Pix_Loop2

		//***********************  KP6
		Pix_Loop_6:
		for(int pixel_fy2=0; pixel_fy2<XI_PIX_PROC;pixel_fy2++)
		{
#pragma HLS unroll
			Pln_Loop3_6:
			for(int plane_fy3=0; plane_fy3<4;plane_fy3++)
			{
#pragma HLS unroll
				short input_mux;
				if((pixel_fy2>35) && (pixel_fy2<42))
				{
					if(conv_desc.sos_enable == 1)
						input_mux = input_buf_copy_reg_dup_1_fy0[plane_fy3][pixel_fy2];
					else
						input_mux = k_buf_copy_6_reg_fy0[plane_fy3];
				}
				else
				{
					input_mux = k_buf_copy_6_reg_fy0[plane_fy3];
				}

				mac_res_temp_6_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_reg_dup_1_fy0[plane_fy3][pixel_fy2] * input_mux);
			}//Pln_Loop3
		}//Pix_Loop2

		//***********************  KP7
		Pix_Loop_7:
		for(int pixel_fy2=0; pixel_fy2<XI_PIX_PROC;pixel_fy2++)
		{
#pragma HLS unroll
			Pln_Loop3_7:
			for(int plane_fy3=0; plane_fy3<4;plane_fy3++)
			{
#pragma HLS unroll
				short input_mux;
				if((pixel_fy2>41) && (pixel_fy2<48))
				{
					if(conv_desc.sos_enable == 1)
						input_mux = input_buf_copy_reg_dup_1_fy0[plane_fy3][pixel_fy2];
					else
						input_mux = k_buf_copy_7_reg_fy0[plane_fy3];
				}
				else
				{
					input_mux = k_buf_copy_7_reg_fy0[plane_fy3];
				}

				mac_res_temp_7_fy0[pixel_fy2][plane_fy3] += (input_buf_copy_reg_dup_1_fy0[plane_fy3][pixel_fy2] * input_mux);
			}//Pln_Loop3
		}//Pix_Loop2

	}//Compute_Loop

	Pix4_Loop:
	for(ap_uint<8> pix8_fy0 = 0; pix8_fy0<XI_PIX_PROC; pix8_fy0+=8)
	{
#pragma HLS pipeline
#pragma HLS DEPENDENCE variable=result_0_ping_fe0 inter false
#pragma HLS DEPENDENCE variable=result_0_ping_fe0 intra false
#pragma HLS DEPENDENCE variable=result_1_ping_fe0 inter false
#pragma HLS DEPENDENCE variable=result_1_ping_fe0 intra false
#pragma HLS DEPENDENCE variable=result_2_ping_fe0 inter false
#pragma HLS DEPENDENCE variable=result_2_ping_fe0 intra false
#pragma HLS DEPENDENCE variable=result_3_ping_fe0 inter false
#pragma HLS DEPENDENCE variable=result_3_ping_fe0 intra false
#pragma HLS DEPENDENCE variable=result_4_ping_fe0 inter false
#pragma HLS DEPENDENCE variable=result_4_ping_fe0 intra false
#pragma HLS DEPENDENCE variable=result_5_ping_fe0 inter false
#pragma HLS DEPENDENCE variable=result_5_ping_fe0 intra false
#pragma HLS DEPENDENCE variable=result_6_ping_fe0 inter false
#pragma HLS DEPENDENCE variable=result_6_ping_fe0 intra false
#pragma HLS DEPENDENCE variable=result_7_ping_fe0 inter false
#pragma HLS DEPENDENCE variable=result_7_ping_fe0 intra false
		Pix64_Loop:
		for(ap_uint<4> pixel_fy4=0; pixel_fy4<8; pixel_fy4++)
		{
#pragma HLS UNROLL
			ap_int<32> ker0_add1_fy0,ker0_add2_fy0, ker1_add1_fy0,ker1_add2_fy0,ker2_add1_fy0,ker2_add2_fy0,ker3_add1_fy0,ker3_add2_fy0;
			ap_int<32> ker4_add1_fy0,ker4_add2_fy0, ker5_add1_fy0,ker5_add2_fy0,ker6_add1_fy0,ker6_add2_fy0,ker7_add1_fy0,ker7_add2_fy0;
#pragma HLS resource variable = ker0_add1_fy0 core = AddSub
#pragma HLS resource variable = ker0_add2_fy0 core = AddSub
#pragma HLS resource variable = ker1_add1_fy0 core = AddSub
#pragma HLS resource variable = ker1_add2_fy0 core = AddSub
#pragma HLS resource variable = ker2_add1_fy0 core = AddSub
#pragma HLS resource variable = ker2_add2_fy0 core = AddSub
#pragma HLS resource variable = ker3_add1_fy0 core = AddSub
#pragma HLS resource variable = ker3_add2_fy0 core = AddSub
#pragma HLS resource variable = ker4_add1_fy0 core = AddSub
#pragma HLS resource variable = ker4_add2_fy0 core = AddSub
#pragma HLS resource variable = ker5_add1_fy0 core = AddSub
#pragma HLS resource variable = ker5_add2_fy0 core = AddSub
#pragma HLS resource variable = ker6_add1_fy0 core = AddSub
#pragma HLS resource variable = ker6_add2_fy0 core = AddSub
#pragma HLS resource variable = ker7_add1_fy0 core = AddSub
#pragma HLS resource variable = ker7_add2_fy0 core = AddSub

			ap_uint<8> pixel_fy3 = pixel_fy4 + pix8_fy0;
			ap_int<32> mac_0[4] ;
			ap_int<32> mac_1[4] ;
			ap_int<32> mac_2[4] ;
			ap_int<32> mac_3[4] ;
			ap_int<32> mac_4[4] ;
			ap_int<32> mac_5[4] ;
			ap_int<32> mac_6[4] ;
			ap_int<32> mac_7[4] ;
#pragma HLS ARRAY_PARTITION variable=mac_0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=mac_1 complete dim=0
#pragma HLS ARRAY_PARTITION variable=mac_2 complete dim=0
#pragma HLS ARRAY_PARTITION variable=mac_3 complete dim=0
#pragma HLS ARRAY_PARTITION variable=mac_4 complete dim=0
#pragma HLS ARRAY_PARTITION variable=mac_5 complete dim=0
#pragma HLS ARRAY_PARTITION variable=mac_6 complete dim=0
#pragma HLS ARRAY_PARTITION variable=mac_7 complete dim=0

			for(ap_uint<3> i=0; i<4; i++)
			{
#pragma HLS UNROLL
				if(pixel_fy3<XI_PIX_PROC)
				{
					mac_0[i] = (ap_int<32>)mac_res_temp_0_fy0[pixel_fy3][i];
					mac_1[i] = (ap_int<32>)mac_res_temp_1_fy0[pixel_fy3][i];
					mac_2[i] = (ap_int<32>)mac_res_temp_2_fy0[pixel_fy3][i];
					mac_3[i] = (ap_int<32>)mac_res_temp_3_fy0[pixel_fy3][i];
					mac_4[i] = (ap_int<32>)mac_res_temp_4_fy0[pixel_fy3][i];
					mac_5[i] = (ap_int<32>)mac_res_temp_5_fy0[pixel_fy3][i];
					mac_6[i] = (ap_int<32>)mac_res_temp_6_fy0[pixel_fy3][i];
					mac_7[i] = (ap_int<32>)mac_res_temp_7_fy0[pixel_fy3][i];
				}else{
					mac_0[i] = 0;
					mac_1[i] = 0;
					mac_2[i] = 0;
					mac_3[i] = 0;
					mac_4[i] = 0;
					mac_5[i] = 0;
					mac_6[i] = 0;
					mac_7[i] = 0;
				}
			}
			ap_int<32> result_0;
			ap_int<32> result_1;
			ap_int<32> result_2;
			ap_int<32> result_3;
			ap_int<32> result_4;
			ap_int<32> result_5;
			ap_int<32> result_6;
			ap_int<32> result_7;
#pragma HLS resource variable = result_0 core = AddSub
#pragma HLS resource variable = result_1 core = AddSub
#pragma HLS resource variable = result_2 core = AddSub
#pragma HLS resource variable = result_3 core = AddSub
#pragma HLS resource variable = result_4 core = AddSub
#pragma HLS resource variable = result_5 core = AddSub
#pragma HLS resource variable = result_6 core = AddSub
#pragma HLS resource variable = result_7 core = AddSub
			ker0_add1_fy0 = mac_0[0] + mac_0[1];
			ker1_add1_fy0 = mac_1[0] + mac_1[1];
			ker2_add1_fy0 = mac_2[0] + mac_2[1];
			ker3_add1_fy0 = mac_3[0] + mac_3[1];
			ker4_add1_fy0 = mac_4[0] + mac_4[1];
			ker5_add1_fy0 = mac_5[0] + mac_5[1];
			ker6_add1_fy0 = mac_6[0] + mac_6[1];
			ker7_add1_fy0 = mac_7[0] + mac_7[1];

			ker0_add2_fy0 = mac_0[2] + ker0_add1_fy0;
			ker1_add2_fy0 = mac_1[2] + ker1_add1_fy0;
			ker2_add2_fy0 = mac_2[2] + ker2_add1_fy0;
			ker3_add2_fy0 = mac_3[2] + ker3_add1_fy0;
			ker4_add2_fy0 = mac_4[2] + ker4_add1_fy0;
			ker5_add2_fy0 = mac_5[2] + ker5_add1_fy0;
			ker6_add2_fy0 = mac_6[2] + ker6_add1_fy0;
			ker7_add2_fy0 = mac_7[2] + ker7_add1_fy0;

			result_0 = ker0_add2_fy0 + mac_0[3];
			result_1 = ker1_add2_fy0 + mac_1[3];
			result_2 = ker2_add2_fy0 + mac_2[3];
			result_3 = ker3_add2_fy0 + mac_3[3];
			result_4 = ker4_add2_fy0 + mac_4[3];
			result_5 = ker5_add2_fy0 + mac_5[3];
			result_6 = ker6_add2_fy0 + mac_6[3];
			result_7 = ker7_add2_fy0 + mac_7[3];


			ap_int<32> result[8];
#pragma HLS ARRAY_PARTITION variable=result complete dim=0
			result[0] = result_0;
			result[1] = result_1;
			result[2] = result_2;
			result[3] = result_3;
			result[4] = result_4;
			result[5] = result_5;
			result[6] = result_6;
			result[7] = result_7;

			short result_out[8];
#pragma HLS ARRAY_PARTITION variable=result_out complete dim=0

			for(ap_uint<4> k=0; k<8;k++)
			{
#pragma HLS UNROLL
				result_out[k] = (short)(result[k]>>conv_desc.inout_precision);
			}

			result_0_ping_fe0[pixel_fy4.range(1,0)][pix8_fy0.range(7,2) + pixel_fy4[2]] = result_out[0];
			result_1_ping_fe0[pixel_fy4.range(1,0)][pix8_fy0.range(7,2) + pixel_fy4[2]] = result_out[1];
			result_2_ping_fe0[pixel_fy4.range(1,0)][pix8_fy0.range(7,2) + pixel_fy4[2]] = result_out[2];
			result_3_ping_fe0[pixel_fy4.range(1,0)][pix8_fy0.range(7,2) + pixel_fy4[2]] = result_out[3];
			result_4_ping_fe0[pixel_fy4.range(1,0)][pix8_fy0.range(7,2) + pixel_fy4[2]] = result_out[4];
			result_5_ping_fe0[pixel_fy4.range(1,0)][pix8_fy0.range(7,2) + pixel_fy4[2]] = result_out[5];
			result_6_ping_fe0[pixel_fy4.range(1,0)][pix8_fy0.range(7,2) + pixel_fy4[2]] = result_out[6];
			result_7_ping_fe0[pixel_fy4.range(1,0)][pix8_fy0.range(7,2) + pixel_fy4[2]] = result_out[7];

		}//Pix64_Loop
	}//pix4_Loop



#if 0//DEBUG_COMPUTE
	//*****************************
	int cnt = 0;
	for(int j=0;j<256;j++){ //planes

		//fprintf(fp_input,"\n Planes num:%d\n",j);

		fprintf(fp_weight,"\n Planes num:%d\n",j);
		for(int i=0;i<25;i++) //pixels
		{
			if(i%5 == 0)
			{
				//fprintf(fp_input,"\n");
				fprintf(fp_weight,"\n");
			}

			//fprintf(fp_input,"%d ",input_global[cnt]);
			fprintf(fp_weight,"%f ",((float)weights_global[cnt])/(1<<13));

			cnt++;
		}

	}

	fprintf(fp_weight,"\n%d____________________________________________________________\n ",(int)ok);
	//******************************

	cnt = 0;
	for(int j=0;j<256;j++){ //planes

		fprintf(fp_input,"\n Planes num:%d\n",j);
		for(int i=0;i<9;i++) //pixels
		{
			if(i%3 == 0)
			{
				//fprintf(fp_input,"\n");
				fprintf(fp_input,"\n");
			}

			//fprintf(fp_input,"%d ",input_global[cnt]);
			fprintf(fp_input,"%f ",((float)input_global[cnt])/(1<<7));

			cnt++;
		}
	}

	fprintf(fp_input,"\n%d____________________________________________________________\n ",(int)ok);
	//******************************
#endif

}

template<int PNKPF>
void OStgBuffSeq8Ker_fx(
		short result_0_ping_fe0[4][XI_PIX_PROC],
		short result_1_ping_fe0[4][XI_PIX_PROC],
		short result_2_ping_fe0[4][XI_PIX_PROC],
		short result_3_ping_fe0[4][XI_PIX_PROC],
		short result_4_ping_fe0[4][XI_PIX_PROC],
		short result_5_ping_fe0[4][XI_PIX_PROC],
		short result_6_ping_fe0[4][XI_PIX_PROC],
		short result_7_ping_fe0[4][XI_PIX_PROC],
		short bias_buff_fb0[XI_BIASMAXSIZE],
		out_pix_struct out_pixels0_fc0,
		ap_uint<16> nk_process_fd0,
		conv_struct conv_desc,
		output_struct output_desc,
		short ostaging_buff0_fb0[4][8][XI_OSTAGEBUFF_DEPTH],
		ap_uint<12> rowsprocessed,
		ap_uint<4> nkpf2_cnt_fe1,
		ap_uint<12> out_row_offset_fb0,
		ap_uint<2> mac_fz0)
{

#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_rows complete dim=1
#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_cols complete dim=1
#pragma HLS data_pack variable=out_pixels0_fc0
#pragma HLS ARRAY_PARTITION variable=result_0_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_1_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_2_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_3_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_4_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_5_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_6_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_7_ping_fe0 complete dim=1
#pragma HLS resource variable=result_0_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_1_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_2_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_3_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_4_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_5_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_6_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_7_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2
#pragma HLS INLINE OFF

	ap_uint<16> outputkernelid_fx0;
	if(conv_desc.lrn_inter_sos_enable == 1 || conv_desc.lrn_intra_sos_enable == 1 )
		outputkernelid_fx0 = (out_pixels0_fc0.current_plane_by4)*4;
	else if(conv_desc.l2_sos_enable == 1)
		outputkernelid_fx0 = 0;
	else//conv
		outputkernelid_fx0 = nk_process_fd0 + nkpf2_cnt_fe1*XI_KER_PROC;

	short biasval_fx0[4][2];
#pragma HLS ARRAY_PARTITION variable=biasval_fx0 complete dim=0

	Dim2_Loop:
	for(ap_uint<4> dim2_fx0=0;dim2_fx0<2;dim2_fx0++)
	{
#pragma HLS unroll
		Dim1_Loop:
		for(ap_uint<4> dim1_fx0=0;dim1_fx0<4;dim1_fx0++)
		{
#pragma HLS unroll
			ap_uint<4> bias_idx_fx0 = dim2_fx0*4 + dim1_fx0;
			biasval_fx0[dim1_fx0][dim2_fx0] = (bias_buff_fb0[outputkernelid_fx0+bias_idx_fx0]);
		}//Dim1_Loop
	}//Dim2_Loop

	ap_uint<16> rows_array_fs0[XI_PIX_PROC],col_array_fs0[XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=rows_array_fs0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=col_array_fs0 complete dim=0
	Pix_Loop:
	for(ap_uint<8> pix_fs0=0;pix_fs0<XI_PIX_PROC;pix_fs0++)
	{
#pragma HLS unroll
		rows_array_fs0[pix_fs0] = out_pixels0_fc0.pix_rows[pix_fs0];
		col_array_fs0[pix_fs0] = out_pixels0_fc0.pix_cols[pix_fs0];
	}//Pix_Loop

	ap_uint<8> result_pix_read[XI_KER_PROC];
#pragma HLS ARRAY_PARTITION variable=result_pix_read complete dim=0
	for(ap_uint<8> ker=0; ker<XI_KER_PROC; ker++)
	{
#pragma HLS unroll
		if(conv_desc.sos_enable == 1)
			result_pix_read[ker] = ker*6;
		else
			result_pix_read[ker] = 0;
	}

	Write_Loop:
	for(ap_uint<8> ostg_pix1_fx0=0;ostg_pix1_fx0<conv_desc.pix_per_kp;ostg_pix1_fx0++)
	{
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=ostaging_buff0_fb0 inter false
#pragma HLS DEPENDENCE variable=ostaging_buff0_fb0 intra false

		ap_uint<16> row_proc_prod_fx0 = rowsprocessed*output_desc.width;
		ap_uint<16> pixbuff_planeoffset_fx0 = (outputkernelid_fx0.range(15,5)) * (conv_desc.out_pix);

		ap_uint<16> in_pix_row_fx0 = rows_array_fs0[ostg_pix1_fx0] - out_row_offset_fb0;
		ap_uint<12> pix_col = col_array_fs0[ostg_pix1_fx0];
		ap_uint<14> pixbuff_rowoffset_fx0 = in_pix_row_fx0 * output_desc.width;
		ap_uint<14> pixbuff_row_fx0 = (pix_col + pixbuff_rowoffset_fx0);
		ap_uint<6> index_fx0 = (outputkernelid_fx0 & 0x1F);
		ap_uint<12> pixbuff_index_fx0 = pixbuff_row_fx0 + pixbuff_planeoffset_fx0;
		bool expression1_fx0 = (pixbuff_row_fx0 < row_proc_prod_fx0);
		ap_uint<3> dim2_ostg1_fx0 = (index_fx0.range(4,2));
		ap_uint<3> dim2_ostg2_fx0 = (index_fx0.range(4,2)) + 1;

		short result_ostg_fx0[4][2];
#pragma HLS ARRAY_PARTITION variable=result_ostg_fx0 complete dim=0
		short result_inc_fx0[4][2];
#pragma HLS ARRAY_PARTITION variable=result_inc_fx0 complete dim=0
		short read_ostg_fx0[4][2];
#pragma HLS ARRAY_PARTITION variable=read_ostg_fx0 complete dim=0
		short ostg_src_fx0[4][4][2];
#pragma HLS ARRAY_PARTITION variable=ostg_src_fx0 complete dim=0

		Dim2_Loop1:
		for(ap_uint<4> dim2_fx1=0;dim2_fx1<4;dim2_fx1++)
		{
			Dim1_Loop1:
			for(ap_uint<4> dim1_fx1=0;dim1_fx1<4;dim1_fx1++)
			{
				Dim3_Loop1:
				for(ap_uint<4> dim3_fx0=0;dim3_fx0<2;dim3_fx0++)
				{
					ap_uint<3> dim22 = dim1_fx1*2 + dim3_fx0;
					ostg_src_fx0[dim1_fx1][dim2_fx1][dim3_fx0] 	  = ostaging_buff0_fb0[dim2_fx1][dim22][pixbuff_index_fx0];
				}//Dim3_Loop1
			}//Dim1_Loop1
		}//Dim2_Loop1

		Dim2_Loop2:
		for(int dim2_fx2=0;dim2_fx2<2;dim2_fx2++)
		{
#pragma HLS UNROLL
			Dim1_Loop2:
			for(int dim1_fx2=0;dim1_fx2<4;dim1_fx2++)
			{
#pragma HLS UNROLL
				read_ostg_fx0[dim1_fx2][dim2_fx2] =     ostg_src_fx0[dim2_ostg1_fx0.range(2,1)][dim1_fx2][dim2_fx2];
			}//Dim1_Loop2
		}//Dim2_Loop2

		Dim2_Loop4:
		for(int dim2_fx4=0;dim2_fx4<2;dim2_fx4++)
		{
#pragma HLS UNROLL
			Dim1_Loop4:
			for(int dim1_fx4=0;dim1_fx4<4;dim1_fx4++)
			{
#pragma HLS UNROLL
				if(conv_desc.sos_enable == 0)
				{
					if(out_pixels0_fc0.current_plane_by4 !=0)
					{
						result_inc_fx0[dim1_fx4][dim2_fx4] = read_ostg_fx0[dim1_fx4][dim2_fx4];
					}
					else
					{
						result_inc_fx0[dim1_fx4][dim2_fx4] = biasval_fx0[dim1_fx4][dim2_fx4];
					}
				}
				else
				{
					result_inc_fx0[dim1_fx4][dim2_fx4] = 0;
				}
			}//Dim1_Loop4
		}//Dim2_Loop4

		ap_uint<2> pix1_dim = ostg_pix1_fx0[1]*2;
		ap_uint<2> pix2_dim = ostg_pix1_fx0[1]*2 + 1;

		result_ostg_fx0[0][0] = result_0_ping_fe0[result_pix_read[0].range(1,0)][result_pix_read[0].range(7,2)]+result_inc_fx0[0][0];
		result_ostg_fx0[1][0] = result_1_ping_fe0[result_pix_read[1].range(1,0)][result_pix_read[1].range(7,2)]+result_inc_fx0[1][0];
		result_ostg_fx0[2][0] = result_2_ping_fe0[result_pix_read[2].range(1,0)][result_pix_read[2].range(7,2)]+result_inc_fx0[2][0];
		result_ostg_fx0[3][0] = result_3_ping_fe0[result_pix_read[3].range(1,0)][result_pix_read[3].range(7,2)]+result_inc_fx0[3][0];
		result_ostg_fx0[0][1] = result_4_ping_fe0[result_pix_read[4].range(1,0)][result_pix_read[4].range(7,2)]+result_inc_fx0[0][1];
		result_ostg_fx0[1][1] = result_5_ping_fe0[result_pix_read[5].range(1,0)][result_pix_read[5].range(7,2)]+result_inc_fx0[1][1];
		result_ostg_fx0[2][1] = result_6_ping_fe0[result_pix_read[6].range(1,0)][result_pix_read[6].range(7,2)]+result_inc_fx0[2][1];
		result_ostg_fx0[3][1] = result_7_ping_fe0[result_pix_read[7].range(1,0)][result_pix_read[7].range(7,2)]+result_inc_fx0[3][1];

		for(ap_uint<4> ker=0; ker<XI_KER_PROC; ker++)
		{
#pragma HLS unroll
			result_pix_read[ker]++;
		}

		short result_ostg_transpose[8];
#pragma HLS ARRAY_PARTITION variable=result_ostg_transpose complete dim=0
		for(ap_uint<4> ker1 = 0, ker2 = 4; ker1 < 4; ker1++, ker2++ )
		{
#pragma HLS UNROLL
			if(conv_desc.l2_sos_enable == 1)
			{
				if(ker1==0)
					result_ostg_transpose[ker1] = result_ostg_fx0[ker1][0];
				else
					result_ostg_transpose[ker1] = 0;

				result_ostg_transpose[ker2] = 0;
			}
			else
			{
				result_ostg_transpose[ker1] = result_ostg_fx0[ker1][0];
				result_ostg_transpose[ker2] = result_ostg_fx0[ker1][1];
			}
		}

		Dim2_Loop3:
		for(int dim2_fx3=0;dim2_fx3<8;dim2_fx3++)
		{
#pragma HLS UNROLL
			Dim1_Loop3:
			for(int dim1_fx3=0;dim1_fx3<4;dim1_fx3++)
			{
#pragma HLS UNROLL

				if(conv_desc.lrn_intra_sos_enable == 1)
				{
					if(expression1_fx0 == 1  && dim1_fx3 == mac_fz0)
					{
						ostaging_buff0_fb0[dim1_fx3][dim2_fx3][pixbuff_index_fx0]= result_ostg_transpose[dim2_fx3];
					}
				}
				else
				{
					if(expression1_fx0 == 1 )
					{
						if(dim2_fx3==dim2_ostg1_fx0)
						{

							ostaging_buff0_fb0[dim1_fx3][dim2_fx3][pixbuff_index_fx0]= result_ostg_fx0[dim1_fx3][0];
						}
						else if(dim2_fx3==(dim2_ostg2_fx0))
						{
							ostaging_buff0_fb0[dim1_fx3][dim2_fx3][pixbuff_index_fx0]= result_ostg_fx0[dim1_fx3][1];
						}
					}
				}

			}//Dim1_Loop3
		}//Dim2_Loop3

	}//Write_Loop

}

template<int CFILTER_SIZE,int CCONV_STRIDE,int CNUM_KERNELS,int IINPUT_PLANES,int PNKPF>
void Compute4Ker_fw(input_struct input_desc,
		weight_struct weight_desc,
		conv_struct conv_desc,
		weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024],
		ap_uint<4> nkpf_cnt_fe1,
		short result_0_ping_fe0[XI_PIX_PROC],
		short result_1_ping_fe0[XI_PIX_PROC],
		short result_2_ping_fe0[XI_PIX_PROC],
		short result_3_ping_fe0[XI_PIX_PROC])
{
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=2
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM

#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=2

#pragma HLS ARRAY_PARTITION variable=result_0_ping_fe0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=result_1_ping_fe0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=result_2_ping_fe0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=result_3_ping_fe0 complete dim=0
#pragma HLS INLINE off

	ap_uint<12> loop_cnt_fw0;
	ap_uint<12> k_off_cnt_fw0;


	ap_int<40> mac_res_temp_0_fw0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_0_fw0 complete dim=0
	ap_int<40> mac_res_temp_1_fw0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_1_fw0 complete dim=0
	ap_int<40> mac_res_temp_2_fw0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_2_fw0 complete dim=0
	ap_int<40> mac_res_temp_3_fw0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_3_fw0 complete dim=0

	loop_cnt_fw0 = weight_desc.filter_size_square*(input_desc.compute_planes.range(15,2));
	k_off_cnt_fw0 = (loop_cnt_fw0*nkpf_cnt_fe1);

	Pix_Loop:
	for(int pixel_fw0=0;pixel_fw0<XI_PIX_PROC;pixel_fw0++)
	{
#pragma HLS unroll
		Pln_Loop:
		for(int plane_fw0=0;plane_fw0<4;plane_fw0++)
		{
#pragma HLS unroll
			mac_res_temp_0_fw0[pixel_fw0][plane_fw0] = 0;
			mac_res_temp_1_fw0[pixel_fw0][plane_fw0] = 0;
			mac_res_temp_2_fw0[pixel_fw0][plane_fw0] = 0;
			mac_res_temp_3_fw0[pixel_fw0][plane_fw0] = 0;
		}//plane_fw0
	}//pixel_fw0

#if DEBUG_COMPUTE
	fprintf(fp_input,"\n Current Plane id:%d\n",(int)input_desc.compute_planes);
	fprintf(fp_weight,"\n Current out Plane id:%d  $$$$$$$$$\n",(int)nkpf_cnt_fe1);
	int index_in=0;
	int index=0;
	short input_global[256*225];

	short weights_global[256*512*26];

#endif

	Compute_Loop:
	for(ap_uint<12> inpbuff_addr1_fw0=0, inpbuff_addr2_fw0=512 ,offset_kbuf_fw0 = k_off_cnt_fw0;inpbuff_addr1_fw0<loop_cnt_fw0;
			/*..continue..*/inpbuff_addr1_fw0++,inpbuff_addr2_fw0++,offset_kbuf_fw0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=9 max=9
#pragma HLS PIPELINE

		short input_buf_copy_fw0[4][XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_fw0 complete dim=0
		Pln_Loop1:
		for(ap_uint<3> plane_fw1=0; plane_fw1<4;plane_fw1++)
		{
#pragma HLS unroll
			pixel_fw0:
			for(ap_uint<8> pixel_fw1=0, pix_split_fw0 =0; pixel_fw1<(XI_PIX_PROC/2);pixel_fw1++, pix_split_fw0++)
			{
#pragma HLS unroll
				input_buf_copy_fw0[plane_fw1][pix_split_fw0] = input_buff0_fc0[plane_fw1][pixel_fw1][inpbuff_addr1_fw0];
				input_buf_copy_fw0[plane_fw1][pix_split_fw0+(XI_PIX_PROC/2)] = input_buff0_fc0[plane_fw1][pixel_fw1][inpbuff_addr2_fw0];
			}//pixel_fw0
		}//Pln_Loop1

		weight_width k_buf_copy_0_fw0[4];
		weight_width k_buf_copy_1_fw0[4];
		weight_width k_buf_copy_2_fw0[4];
		weight_width k_buf_copy_3_fw0[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_0_fw0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_1_fw0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_2_fw0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_3_fw0 complete dim=0

		Pln_Loop2:
		for(ap_uint<3> plane_fw2=0; plane_fw2<4;plane_fw2++)
		{
#pragma HLS unroll
			k_buf_copy_0_fw0[plane_fw2] = weight_buff0_fd0[0][plane_fw2][offset_kbuf_fw0];
			k_buf_copy_1_fw0[plane_fw2] = weight_buff0_fd0[1][plane_fw2][offset_kbuf_fw0];
			k_buf_copy_2_fw0[plane_fw2] = weight_buff0_fd0[2][plane_fw2][offset_kbuf_fw0];
			k_buf_copy_3_fw0[plane_fw2] = weight_buff0_fd0[3][plane_fw2][offset_kbuf_fw0];
		}//Pln_Loop2


#if DEBUG_COMPUTE

		//************************************88
		///short read = input_buf_copy_fw0[0][1];
		input_global[index_in]   = input_buf_copy_fw0[0][0];
		input_global[index_in+121] = input_buf_copy_fw0[1][0];
		input_global[index_in+2*121] = input_buf_copy_fw0[2][0];
		input_global[index_in+3*121] = input_buf_copy_fw0[3][0];

		index_in++;
		if(index_in%121 == 0)
			index_in+=(3*121);
		//*************************************88

		//************************************88
		weights_global[index]   = (short)k_buf_copy_0_fw0[0];
		weights_global[index+121] = (short)k_buf_copy_0_fw0[1];
		weights_global[index+2*121] = (short)k_buf_copy_0_fw0[2];
		weights_global[index+3*121] = (short)k_buf_copy_0_fw0[3];
		index++;
		if(index%121 == 0)
			index+=(3*121);
		//*************************************88
#endif

		//	printf("\n%d:\t",(int)inpbuff_addr1_fw0);
		Pix_Loop2:
		for(int pixel_fw2=0; pixel_fw2<XI_PIX_PROC;pixel_fw2++)
		{
#pragma HLS unroll
			Pln_Loop3:
			for(int plane_fw3=0; plane_fw3<4;plane_fw3++)
			{
#pragma HLS unroll
				//		if(pixel_fw2==0)
				//		printf("\t\t %d x%d",(int)(input_buf_copy_fw0[plane_fw3][pixel_fw2] ), (int)( k_buf_copy_0_fw0[plane_fw3]));

				mac_res_temp_0_fw0[pixel_fw2][plane_fw3] += (input_buf_copy_fw0[plane_fw3][pixel_fw2] * k_buf_copy_0_fw0[plane_fw3]);
				mac_res_temp_1_fw0[pixel_fw2][plane_fw3] += (input_buf_copy_fw0[plane_fw3][pixel_fw2] * k_buf_copy_1_fw0[plane_fw3]);
				mac_res_temp_2_fw0[pixel_fw2][plane_fw3] += (input_buf_copy_fw0[plane_fw3][pixel_fw2] * k_buf_copy_2_fw0[plane_fw3]);
				mac_res_temp_3_fw0[pixel_fw2][plane_fw3] += (input_buf_copy_fw0[plane_fw3][pixel_fw2] * k_buf_copy_3_fw0[plane_fw3]);
			}//Pln_Loop3
		}//Pix_Loop2
	}

	//	Once_Loop:
	//	for(short once_fw0 = 0; once_fw0<1; once_fw0++)
	//	{
	//#pragma HLS pipeline
	Pix64_Loop:
	for(ap_uint<8> pixel_fw3=0; pixel_fw3<XI_PIX_PROC; pixel_fw3++)
	{
#pragma HLS UNROLL
		short ker0_add1_fw0, ker0_add2_fw0, ker1_add1_fw0, ker1_add2_fw0, ker2_add1_fw0, ker2_add2_fw0, ker3_add1_fw0, ker3_add2_fw0;

#if 1
		switch(conv_desc.inout_precision)
		{
		case 0: //IN format = OUT format
			ker0_add1_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][0].range(22,7)) + (short)(mac_res_temp_0_fw0[pixel_fw3][1].range(22,7));
			ker0_add2_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][2].range(22,7)) + (short)(mac_res_temp_0_fw0[pixel_fw3][3].range(22,7));
			ker1_add1_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][0].range(22,7)) + (short)(mac_res_temp_1_fw0[pixel_fw3][1].range(22,7));
			ker1_add2_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][2].range(22,7)) + (short)(mac_res_temp_1_fw0[pixel_fw3][3].range(22,7));
			ker2_add1_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][0].range(22,7)) + (short)(mac_res_temp_2_fw0[pixel_fw3][1].range(22,7));
			ker2_add2_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][2].range(22,7)) + (short)(mac_res_temp_2_fw0[pixel_fw3][3].range(22,7));
			ker3_add1_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][0].range(22,7)) + (short)(mac_res_temp_3_fw0[pixel_fw3][1].range(22,7));
			ker3_add2_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][2].range(22,7)) + (short)(mac_res_temp_3_fw0[pixel_fw3][3].range(22,7));
			break;
		case 1:// IN 8.8 ----> OUT 11.5
			ker0_add1_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][0].range(25,10)) + (short)(mac_res_temp_0_fw0[pixel_fw3][1].range(25,10));
			ker0_add2_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][2].range(25,10)) + (short)(mac_res_temp_0_fw0[pixel_fw3][3].range(25,10));
			ker1_add1_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][0].range(25,10)) + (short)(mac_res_temp_1_fw0[pixel_fw3][1].range(25,10));
			ker1_add2_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][2].range(25,10)) + (short)(mac_res_temp_1_fw0[pixel_fw3][3].range(25,10));
			ker2_add1_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][0].range(25,10)) + (short)(mac_res_temp_2_fw0[pixel_fw3][1].range(25,10));
			ker2_add2_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][2].range(25,10)) + (short)(mac_res_temp_2_fw0[pixel_fw3][3].range(25,10));
			ker3_add1_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][0].range(25,10)) + (short)(mac_res_temp_3_fw0[pixel_fw3][1].range(25,10));
			ker3_add2_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][2].range(25,10)) + (short)(mac_res_temp_3_fw0[pixel_fw3][3].range(25,10));
			break;
		case 2:// IN 11.5 ----> OUT 8.8
			ker0_add1_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][0].range(19,4)) + (short)(mac_res_temp_0_fw0[pixel_fw3][1].range(19,4));
			ker0_add2_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][2].range(19,4)) + (short)(mac_res_temp_0_fw0[pixel_fw3][3].range(19,4));
			ker1_add1_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][0].range(19,4)) + (short)(mac_res_temp_1_fw0[pixel_fw3][1].range(19,4));
			ker1_add2_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][2].range(19,4)) + (short)(mac_res_temp_1_fw0[pixel_fw3][3].range(19,4));
			ker2_add1_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][0].range(19,4)) + (short)(mac_res_temp_2_fw0[pixel_fw3][1].range(19,4));
			ker2_add2_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][2].range(19,4)) + (short)(mac_res_temp_2_fw0[pixel_fw3][3].range(19,4));
			ker3_add1_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][0].range(19,4)) + (short)(mac_res_temp_3_fw0[pixel_fw3][1].range(19,4));
			ker3_add2_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][2].range(19,4)) + (short)(mac_res_temp_3_fw0[pixel_fw3][3].range(19,4));
			break;
		case 3:// IN 9.7 ----> OUT 11.5
			ker0_add1_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][0].range(24,9)) + (short)(mac_res_temp_0_fw0[pixel_fw3][1].range(24,9));
			ker0_add2_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][2].range(24,9)) + (short)(mac_res_temp_0_fw0[pixel_fw3][3].range(24,9));
			ker1_add1_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][0].range(24,9)) + (short)(mac_res_temp_1_fw0[pixel_fw3][1].range(24,9));
			ker1_add2_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][2].range(24,9)) + (short)(mac_res_temp_1_fw0[pixel_fw3][3].range(24,9));
			ker2_add1_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][0].range(24,9)) + (short)(mac_res_temp_2_fw0[pixel_fw3][1].range(24,9));
			ker2_add2_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][2].range(24,9)) + (short)(mac_res_temp_2_fw0[pixel_fw3][3].range(24,9));
			ker3_add1_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][0].range(24,9)) + (short)(mac_res_temp_3_fw0[pixel_fw3][1].range(24,9));
			ker3_add2_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][2].range(24,9)) + (short)(mac_res_temp_3_fw0[pixel_fw3][3].range(24,9));
			break;
		case 4:// IN 9.7 ----> OUT 15.1
			ker0_add1_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][0].range(28,13)) + (short)(mac_res_temp_0_fw0[pixel_fw3][1].range(28,13));
			ker0_add2_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][2].range(28,13)) + (short)(mac_res_temp_0_fw0[pixel_fw3][3].range(28,13));
			ker1_add1_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][0].range(28,13)) + (short)(mac_res_temp_1_fw0[pixel_fw3][1].range(28,13));
			ker1_add2_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][2].range(28,13)) + (short)(mac_res_temp_1_fw0[pixel_fw3][3].range(28,13));
			ker2_add1_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][0].range(28,13)) + (short)(mac_res_temp_2_fw0[pixel_fw3][1].range(28,13));
			ker2_add2_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][2].range(28,13)) + (short)(mac_res_temp_2_fw0[pixel_fw3][3].range(28,13));
			ker3_add1_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][0].range(28,13)) + (short)(mac_res_temp_3_fw0[pixel_fw3][1].range(28,13));
			ker3_add2_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][2].range(28,13)) + (short)(mac_res_temp_3_fw0[pixel_fw3][3].range(28,13));
			break;
		case 5:// IN 15.1 ----> OUT 8.8
			ker0_add1_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][0].range(15,0)) + (short)(mac_res_temp_0_fw0[pixel_fw3][1].range(15,0));
			ker0_add2_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][2].range(15,0)) + (short)(mac_res_temp_0_fw0[pixel_fw3][3].range(15,0));
			ker1_add1_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][0].range(15,0)) + (short)(mac_res_temp_1_fw0[pixel_fw3][1].range(15,0));
			ker1_add2_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][2].range(15,0)) + (short)(mac_res_temp_1_fw0[pixel_fw3][3].range(15,0));
			ker2_add1_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][0].range(15,0)) + (short)(mac_res_temp_2_fw0[pixel_fw3][1].range(15,0));
			ker2_add2_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][2].range(15,0)) + (short)(mac_res_temp_2_fw0[pixel_fw3][3].range(15,0));
			ker3_add1_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][0].range(15,0)) + (short)(mac_res_temp_3_fw0[pixel_fw3][1].range(15,0));
			ker3_add2_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][2].range(15,0)) + (short)(mac_res_temp_3_fw0[pixel_fw3][3].range(15,0));
			break;
		case 6:// IN 8.8 ----> OUT 15.1
			ker0_add1_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][0].range(29,14)) + (short)(mac_res_temp_0_fw0[pixel_fw3][1].range(29,14));
			ker0_add2_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][2].range(29,14)) + (short)(mac_res_temp_0_fw0[pixel_fw3][3].range(29,14));
			ker1_add1_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][0].range(29,14)) + (short)(mac_res_temp_1_fw0[pixel_fw3][1].range(29,14));
			ker1_add2_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][2].range(29,14)) + (short)(mac_res_temp_1_fw0[pixel_fw3][3].range(29,14));
			ker2_add1_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][0].range(29,14)) + (short)(mac_res_temp_2_fw0[pixel_fw3][1].range(29,14));
			ker2_add2_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][2].range(29,14)) + (short)(mac_res_temp_2_fw0[pixel_fw3][3].range(29,14));
			ker3_add1_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][0].range(29,14)) + (short)(mac_res_temp_3_fw0[pixel_fw3][1].range(29,14));
			ker3_add2_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][2].range(29,14)) + (short)(mac_res_temp_3_fw0[pixel_fw3][3].range(29,14));
			break;
		default:// IN 15.1 ----> OUT 11.5
			ker0_add1_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][0].range(18,3)) + (short)(mac_res_temp_0_fw0[pixel_fw3][1].range(18,3));
			ker0_add2_fw0 = (short)(mac_res_temp_0_fw0[pixel_fw3][2].range(18,3)) + (short)(mac_res_temp_0_fw0[pixel_fw3][3].range(18,3));
			ker1_add1_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][0].range(18,3)) + (short)(mac_res_temp_1_fw0[pixel_fw3][1].range(18,3));
			ker1_add2_fw0 = (short)(mac_res_temp_1_fw0[pixel_fw3][2].range(18,3)) + (short)(mac_res_temp_1_fw0[pixel_fw3][3].range(18,3));
			ker2_add1_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][0].range(18,3)) + (short)(mac_res_temp_2_fw0[pixel_fw3][1].range(18,3));
			ker2_add2_fw0 = (short)(mac_res_temp_2_fw0[pixel_fw3][2].range(18,3)) + (short)(mac_res_temp_2_fw0[pixel_fw3][3].range(18,3));
			ker3_add1_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][0].range(18,3)) + (short)(mac_res_temp_3_fw0[pixel_fw3][1].range(18,3));
			ker3_add2_fw0 = (short)(mac_res_temp_3_fw0[pixel_fw3][2].range(18,3)) + (short)(mac_res_temp_3_fw0[pixel_fw3][3].range(18,3));
			break;

		}//switch
#else//:TODO Run time precision
		ap_int<30> mac_res_temp_30bit_0[4];
		ap_int<30> mac_res_temp_30bit_1[4];
		ap_int<30> mac_res_temp_30bit_2[4];
		ap_int<30> mac_res_temp_30bit_3[4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_30bit_0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_30bit_1 complete dim=0
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_30bit_2 complete dim=0
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_30bit_3 complete dim=0

		Pln_Loop4:
		for(ap_uint<3> plane_fw4=0; plane_fw4<3; plane_fw4++)
		{
#pragma HLS unroll
			mac_res_temp_30bit_0[plane_fw4] = mac_res_temp_0_fw0[pixel][plane_fw4].range(29,0);
			mac_res_temp_30bit_1[plane_fw4] = mac_res_temp_1_fw0[pixel][plane_fw4].range(29,0);
			mac_res_temp_30bit_2[plane_fw4] = mac_res_temp_2_fw0[pixel][plane_fw4].range(29,0);
			mac_res_temp_30bit_3[plane_fw4] = mac_res_temp_3_fw0[pixel][plane_fw4].range(29,0);
		}

		switch(conv_desc.inout_precision)
		{
		case 0:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(15,0)) + (short)(mac_res_temp_30bit_0[1].range(15,0));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(15,0)) + (short)(mac_res_temp_30bit_0[3].range(15,0));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(15,0)) + (short)(mac_res_temp_30bit_1[1].range(15,0));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(15,0)) + (short)(mac_res_temp_30bit_1[3].range(15,0));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(15,0)) + (short)(mac_res_temp_30bit_2[1].range(15,0));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(15,0)) + (short)(mac_res_temp_30bit_2[3].range(15,0));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(15,0)) + (short)(mac_res_temp_30bit_3[1].range(15,0));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(15,0)) + (short)(mac_res_temp_30bit_3[3].range(15,0));
			break;
		case 1:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(16,1)) + (short)(mac_res_temp_30bit_0[1].range(16,1));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(16,1)) + (short)(mac_res_temp_30bit_0[3].range(16,1));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(16,1)) + (short)(mac_res_temp_30bit_1[1].range(16,1));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(16,1)) + (short)(mac_res_temp_30bit_1[3].range(16,1));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(16,1)) + (short)(mac_res_temp_30bit_2[1].range(16,1));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(16,1)) + (short)(mac_res_temp_30bit_2[3].range(16,1));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(16,1)) + (short)(mac_res_temp_30bit_3[1].range(16,1));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(16,1)) + (short)(mac_res_temp_30bit_3[3].range(16,1));
			break;
		case 2:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(17,2)) + (short)(mac_res_temp_30bit_0[1].range(17,2));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(17,2)) + (short)(mac_res_temp_30bit_0[3].range(17,2));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(17,2)) + (short)(mac_res_temp_30bit_1[1].range(17,2));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(17,2)) + (short)(mac_res_temp_30bit_1[3].range(17,2));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(17,2)) + (short)(mac_res_temp_30bit_2[1].range(17,2));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(17,2)) + (short)(mac_res_temp_30bit_2[3].range(17,2));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(17,2)) + (short)(mac_res_temp_30bit_3[1].range(17,2));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(17,2)) + (short)(mac_res_temp_30bit_3[3].range(17,2));
			break;
		case 3:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(18,3)) + (short)(mac_res_temp_30bit_0[1].range(18,3));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(18,3)) + (short)(mac_res_temp_30bit_0[3].range(18,3));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(18,3)) + (short)(mac_res_temp_30bit_1[1].range(18,3));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(18,3)) + (short)(mac_res_temp_30bit_1[3].range(18,3));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(18,3)) + (short)(mac_res_temp_30bit_2[1].range(18,3));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(18,3)) + (short)(mac_res_temp_30bit_2[3].range(18,3));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(18,3)) + (short)(mac_res_temp_30bit_3[1].range(18,3));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(18,3)) + (short)(mac_res_temp_30bit_3[3].range(18,3));
			break;
		case 4:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(19,4)) + (short)(mac_res_temp_30bit_0[1].range(19,4));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(19,4)) + (short)(mac_res_temp_30bit_0[3].range(19,4));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(19,4)) + (short)(mac_res_temp_30bit_1[1].range(19,4));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(19,4)) + (short)(mac_res_temp_30bit_1[3].range(19,4));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(19,4)) + (short)(mac_res_temp_30bit_2[1].range(19,4));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(19,4)) + (short)(mac_res_temp_30bit_2[3].range(19,4));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(19,4)) + (short)(mac_res_temp_30bit_3[1].range(19,4));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(19,4)) + (short)(mac_res_temp_30bit_3[3].range(19,4));
			break;
		case 5:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(20,5)) + (short)(mac_res_temp_30bit_0[1].range(20,5));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(20,5)) + (short)(mac_res_temp_30bit_0[3].range(20,5));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(20,5)) + (short)(mac_res_temp_30bit_1[1].range(20,5));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(20,5)) + (short)(mac_res_temp_30bit_1[3].range(20,5));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(20,5)) + (short)(mac_res_temp_30bit_2[1].range(20,5));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(20,5)) + (short)(mac_res_temp_30bit_2[3].range(20,5));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(20,5)) + (short)(mac_res_temp_30bit_3[1].range(20,5));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(20,5)) + (short)(mac_res_temp_30bit_3[3].range(20,5));
			break;
		case 6:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(21,6)) + (short)(mac_res_temp_30bit_0[1].range(21,6));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(21,6)) + (short)(mac_res_temp_30bit_0[3].range(21,6));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(21,6)) + (short)(mac_res_temp_30bit_1[1].range(21,6));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(21,6)) + (short)(mac_res_temp_30bit_1[3].range(21,6));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(21,6)) + (short)(mac_res_temp_30bit_2[1].range(21,6));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(21,6)) + (short)(mac_res_temp_30bit_2[3].range(21,6));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(21,6)) + (short)(mac_res_temp_30bit_3[1].range(21,6));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(21,6)) + (short)(mac_res_temp_30bit_3[3].range(21,6));
			break;
		case 7:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(22,7)) + (short)(mac_res_temp_30bit_0[1].range(22,7));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(22,7)) + (short)(mac_res_temp_30bit_0[3].range(22,7));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(22,7)) + (short)(mac_res_temp_30bit_1[1].range(22,7));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(22,7)) + (short)(mac_res_temp_30bit_1[3].range(22,7));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(22,7)) + (short)(mac_res_temp_30bit_2[1].range(22,7));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(22,7)) + (short)(mac_res_temp_30bit_2[3].range(22,7));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(22,7)) + (short)(mac_res_temp_30bit_3[1].range(22,7));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(22,7)) + (short)(mac_res_temp_30bit_3[3].range(22,7));
			break;

		case 8:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(23,8)) + (short)(mac_res_temp_30bit_0[1].range(23,8));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(23,8)) + (short)(mac_res_temp_30bit_0[3].range(23,8));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(23,8)) + (short)(mac_res_temp_30bit_1[1].range(23,8));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(23,8)) + (short)(mac_res_temp_30bit_1[3].range(23,8));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(23,8)) + (short)(mac_res_temp_30bit_2[1].range(23,8));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(23,8)) + (short)(mac_res_temp_30bit_2[3].range(23,8));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(23,8)) + (short)(mac_res_temp_30bit_3[1].range(23,8));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(23,8)) + (short)(mac_res_temp_30bit_3[3].range(23,8));
			break;
		case 9:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(24,9)) + (short)(mac_res_temp_30bit_0[1].range(24,9));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(24,9)) + (short)(mac_res_temp_30bit_0[3].range(24,9));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(24,9)) + (short)(mac_res_temp_30bit_1[1].range(24,9));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(24,9)) + (short)(mac_res_temp_30bit_1[3].range(24,9));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(24,9)) + (short)(mac_res_temp_30bit_2[1].range(24,9));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(24,9)) + (short)(mac_res_temp_30bit_2[3].range(24,9));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(24,9)) + (short)(mac_res_temp_30bit_3[1].range(24,9));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(24,9)) + (short)(mac_res_temp_30bit_3[3].range(24,9));
			break;
		case 10:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(25,10)) + (short)(mac_res_temp_30bit_0[1].range(25,10));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(25,10)) + (short)(mac_res_temp_30bit_0[3].range(25,10));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(25,10)) + (short)(mac_res_temp_30bit_1[1].range(25,10));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(25,10)) + (short)(mac_res_temp_30bit_1[3].range(25,10));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(25,10)) + (short)(mac_res_temp_30bit_2[1].range(25,10));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(25,10)) + (short)(mac_res_temp_30bit_2[3].range(25,10));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(25,10)) + (short)(mac_res_temp_30bit_3[1].range(25,10));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(25,10)) + (short)(mac_res_temp_30bit_3[3].range(25,10));
			break;
		case 11:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(26,11)) + (short)(mac_res_temp_30bit_0[1].range(26,11));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(26,11)) + (short)(mac_res_temp_30bit_0[3].range(26,11));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(26,11)) + (short)(mac_res_temp_30bit_1[1].range(26,11));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(26,11)) + (short)(mac_res_temp_30bit_1[3].range(26,11));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(26,11)) + (short)(mac_res_temp_30bit_2[1].range(26,11));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(26,11)) + (short)(mac_res_temp_30bit_2[3].range(26,11));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(26,11)) + (short)(mac_res_temp_30bit_3[1].range(26,11));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(26,11)) + (short)(mac_res_temp_30bit_3[3].range(26,11));
			break;
		case 12:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(27,12)) + (short)(mac_res_temp_30bit_0[1].range(27,12));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(27,12)) + (short)(mac_res_temp_30bit_0[3].range(27,12));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(27,12)) + (short)(mac_res_temp_30bit_1[1].range(27,12));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(27,12)) + (short)(mac_res_temp_30bit_1[3].range(27,12));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(27,12)) + (short)(mac_res_temp_30bit_2[1].range(27,12));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(27,12)) + (short)(mac_res_temp_30bit_2[3].range(27,12));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(27,12)) + (short)(mac_res_temp_30bit_3[1].range(27,12));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(27,12)) + (short)(mac_res_temp_30bit_3[3].range(27,12));
			break;
		case 13:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(28,13)) + (short)(mac_res_temp_30bit_0[1].range(28,13));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(28,13)) + (short)(mac_res_temp_30bit_0[3].range(28,13));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(28,13)) + (short)(mac_res_temp_30bit_1[1].range(28,13));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(28,13)) + (short)(mac_res_temp_30bit_1[3].range(28,13));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(28,13)) + (short)(mac_res_temp_30bit_2[1].range(28,13));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(28,13)) + (short)(mac_res_temp_30bit_2[3].range(28,13));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(28,13)) + (short)(mac_res_temp_30bit_3[1].range(28,13));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(28,13)) + (short)(mac_res_temp_30bit_3[3].range(28,13));
			break;
		default:
			ker0_add1_fw0 = (short)(mac_res_temp_30bit_0[0].range(29,14)) + (short)(mac_res_temp_30bit_0[1].range(29,14));
			ker0_add2_fw0 = (short)(mac_res_temp_30bit_0[2].range(29,14)) + (short)(mac_res_temp_30bit_0[3].range(29,14));
			ker1_add1_fw0 = (short)(mac_res_temp_30bit_1[0].range(29,14)) + (short)(mac_res_temp_30bit_1[1].range(29,14));
			ker1_add2_fw0 = (short)(mac_res_temp_30bit_1[2].range(29,14)) + (short)(mac_res_temp_30bit_1[3].range(29,14));
			ker2_add1_fw0 = (short)(mac_res_temp_30bit_2[0].range(29,14)) + (short)(mac_res_temp_30bit_2[1].range(29,14));
			ker2_add2_fw0 = (short)(mac_res_temp_30bit_2[2].range(29,14)) + (short)(mac_res_temp_30bit_2[3].range(29,14));
			ker3_add1_fw0 = (short)(mac_res_temp_30bit_3[0].range(29,14)) + (short)(mac_res_temp_30bit_3[1].range(29,14));
			ker3_add2_fw0 = (short)(mac_res_temp_30bit_3[2].range(29,14)) + (short)(mac_res_temp_30bit_3[3].range(29,14));
			break;
		}//switch


#endif
		result_0_ping_fe0[pixel_fw3] = ker0_add1_fw0 + ker0_add2_fw0;
		result_1_ping_fe0[pixel_fw3] = ker1_add1_fw0 + ker1_add2_fw0;
		result_2_ping_fe0[pixel_fw3] = ker2_add1_fw0 + ker2_add2_fw0;
		result_3_ping_fe0[pixel_fw3] = ker3_add1_fw0 + ker3_add2_fw0;

	}//Pix64_Loop
	//	}//once_Loop


#if DEBUG_COMPUTE
	//*****************************
	int cnt = 0;
	for(int j=0;j<96;j++){ //planes

		//fprintf(fp_input,"\n Planes num:%d\n",j);

		//fprintf(fp_weight,"\n Planes num:%d\n",j);
		for(int i=0;i<121;i++) //pixels
		{
			if(i%11 == 0)
			{
				fprintf(fp_input,"\n");
				fprintf(fp_weight,"\n");
			}

			fprintf(fp_input,"%d ",input_global[cnt]);
			fprintf(fp_weight,"%f ",((float)weights_global[cnt])/(1<<7));

			cnt++;
		}

	}

	//fprintf(fp_weight,"\n%d____________________________________________________________\n ",(int)ok);
	//******************************



	//	fprintf(fp_input,"\n%d____________________________________________________________\n ",(int)ok);
	//******************************
#endif

}

template<int CFILTER_SIZE,int CCONV_STRIDE,int CNUM_KERNELS,int IINPUT_PLANES,int PNKPF>
void Compute2Ker_fv(input_struct input_desc,
		weight_struct weight_desc,
		conv_struct conv_desc,
		weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024],
		ap_uint<4> nkpf_cnt,
		short result_0_ping_fe0[XI_PIX_PROC],
		short result_1_ping_fe0[XI_PIX_PROC],
		ap_uint<3> ker_id_ft0)
{
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=2
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM

#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=2

#pragma HLS ARRAY_PARTITION variable=result_0_ping_fe0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=result_1_ping_fe0 complete dim=0
#pragma HLS INLINE off

	ap_uint<10> lcount_fv0;
	ap_uint<12> k_off_cnt_fv0;

	ap_int<40> mac_res_temp_0_fv0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_0_fv0 complete dim=0
	ap_int<40> mac_res_temp_1_fv0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_1_fv0 complete dim=0

	lcount_fv0 = weight_desc.filter_size_square*(input_desc.compute_planes.range(15,2));
	k_off_cnt_fv0 = (2*lcount_fv0*nkpf_cnt) + (ker_id_ft0*lcount_fv0);

	Pix_Loop:
	for(int pixel_fv0=0;pixel_fv0<XI_PIX_PROC;pixel_fv0++)
	{
#pragma HLS unroll
		Pln_Loop:
		for(int plane_fv0=0;plane_fv0<4;plane_fv0++)
		{
#pragma HLS unroll
			mac_res_temp_0_fv0[pixel_fv0][plane_fv0] = 0;
			mac_res_temp_1_fv0[pixel_fv0][plane_fv0] = 0;
		}//Pln_Loop
	}//Pix_Loop

#if 0//DEBUG_COMPUTE
	fprintf(fp_input,"\n Current Plane id:%d\n",(int)current_plane);
	fprintf(fp_weight,"\n Current Plane id:%d\n",(int)current_plane);
#endif

	Compute_Loop:
	for(ap_uint<12> inp_buff_addr1_fv0=0, inp_buff_addr2_fv0=512 ,offset_kbuf_fv0 = k_off_cnt_fv0;inp_buff_addr1_fv0<lcount_fv0;
			/*..continue..*/inp_buff_addr1_fv0++,inp_buff_addr2_fv0++,offset_kbuf_fv0++) //int -> arbitrary precision change
	{
#pragma HLS LOOP_TRIPCOUNT min=121 max=121
#pragma HLS PIPELINE

		short input_buf_copy_fv0[4][XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_fv0 complete dim=0
		Pln_Loop1:
		for(ap_uint<3> palne_fv1=0; palne_fv1<4;palne_fv1++)
		{
#pragma HLS unroll
			Pix_Loop1:
			for(ap_uint<8> pixel_fv1=0, pix_split_fv0 =0; pixel_fv1<(XI_PIX_PROC/2);pixel_fv1++, pix_split_fv0++)
			{
#pragma HLS unroll
				input_buf_copy_fv0[palne_fv1][pix_split_fv0] = input_buff0_fc0[palne_fv1][pixel_fv1][inp_buff_addr1_fv0];
				input_buf_copy_fv0[palne_fv1][pix_split_fv0+(XI_PIX_PROC/2)] = input_buff0_fc0[palne_fv1][pixel_fv1][inp_buff_addr2_fv0];
			}//Pln_Loop1
		}//Pix_Loop1

		weight_width k_buf_copy_0_fv0[4];
		weight_width k_buf_copy_1_fv0[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_0_fv0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_1_fv0 complete dim=0

		Pln_Loop2:
		for(ap_uint<3> plane_fv2=0; plane_fv2<4;plane_fv2++)
		{  //int -> arbitrary precison change
#pragma HLS unroll
			k_buf_copy_0_fv0[plane_fv2] = weight_buff0_fd0[0][plane_fv2][offset_kbuf_fv0];
			k_buf_copy_1_fv0[plane_fv2] = weight_buff0_fd0[1][plane_fv2][offset_kbuf_fv0];
		}////Pln_Loop2

#if XI_PIX_PROC_LUT ==0
		Pix_Loop2:
		for(ap_uint<8> pixel_fv2=0; pixel_fv2<XI_PIX_PROC;pixel_fv2++)
		{
#pragma HLS unroll
			Pln_Loop3:
			for(ap_uint<4> plane_fv3=0; plane_fv3<4;plane_fv3++)
			{
#pragma HLS unroll
				mac_res_temp_0_fv0[pixel_fv2][plane_fv3] += (input_buf_copy_fv0[plane_fv3][pixel_fv2] * k_buf_copy_0_fv0[plane_fv3]);
				mac_res_temp_1_fv0[pixel_fv2][plane_fv3] += (input_buf_copy_fv0[plane_fv3][pixel_fv2] * k_buf_copy_1_fv0[plane_fv3]);
			}//Pln_Loop3
		}//Pix_Loop2
#else

		Pix_Loop3:
		for(int pixel_fv3=0; pixel_fv3<XI_PIX_PROC;pixel_fv3++)
		{
#pragma HLS unroll
			Pln_Loop4:
			for(int plane_fv4=0; plane_fv4<4;plane_fv4++)
			{
#pragma HLS unroll
				if(pixel_fv3 < (XI_PIX_PROC - XI_PIX_PROC_LUT)) // process (XI_PIX_PROC - XI_PIX_PROC_LUT) pixels using DSPs
				{
					mac_res_temp_0_fv0[pixel_fv3][plane_fv4] += (input_buf_copy_fv0[plane_fv4][pixel_fv3] * k_buf_copy_0_fv0[plane_fv4]);
					mac_res_temp_1_fv0[pixel_fv3][plane_fv4] += (input_buf_copy_fv0[plane_fv4][pixel_fv3] * k_buf_copy_1_fv0[plane_fv4]);
				}
				else
				{                              // process XI_PIX_PROC_LUT pixels using LUTs
					ap_int<40> mul_0;
#pragma HLS RESOURCE variable=mul_0 core=Mul_LUT
					mul_0 = (input_buf_copy_fv0[plane_fv4][pixel_fv3] * k_buf_copy_0_fv0[plane_fv4]);
					ap_int<40> mul_1;
#pragma HLS RESOURCE variable=mul_1 core=Mul_LUT
					mul_1 = (input_buf_copy_fv0[plane_fv4][pixel_fv3] * k_buf_copy_1_fv0[plane_fv4]);

					mac_res_temp_0_fv0[pixel_fv3][plane_fv4] += mul_0;
					mac_res_temp_1_fv0[pixel_fv3][plane_fv4] += mul_1;
				}
			}//Pln_Loop4
		}//Pix_Loop3

#endif

	}

	Once_Loop:
	for(short once_fv0 = 0; once_fv0<1; once_fv0++)
	{
#pragma HLS pipeline
		Pix64_Loop:
		for(ap_uint<8> pixel_fv4=0; pixel_fv4<XI_PIX_PROC; pixel_fv4++)
		{
#pragma HLS UNROLL
			short ker0_add1_fv0, ker0_add2_fv0, ker1_add1_fv0, ker1_add2_fv0;


			switch(conv_desc.inout_precision)
			{
			case 0: //IN format = OUT format
				ker0_add1_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][0].range(22,7)) + (short)(mac_res_temp_0_fv0[pixel_fv4][1].range(22,7));
				ker0_add2_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][2].range(22,7)) + (short)(mac_res_temp_0_fv0[pixel_fv4][3].range(22,7));
				ker1_add1_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][0].range(22,7)) + (short)(mac_res_temp_1_fv0[pixel_fv4][1].range(22,7));
				ker1_add2_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][2].range(22,7)) + (short)(mac_res_temp_1_fv0[pixel_fv4][3].range(22,7));

				break;
			case 1:// IN 8.8 ----> OUT 11.5
				ker0_add1_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][0].range(25,10)) + (short)(mac_res_temp_0_fv0[pixel_fv4][1].range(25,10));
				ker0_add2_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][2].range(25,10)) + (short)(mac_res_temp_0_fv0[pixel_fv4][3].range(25,10));
				ker1_add1_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][0].range(25,10)) + (short)(mac_res_temp_1_fv0[pixel_fv4][1].range(25,10));
				ker1_add2_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][2].range(25,10)) + (short)(mac_res_temp_1_fv0[pixel_fv4][3].range(25,10));

				break;
			case 2:// IN 11.5 ----> OUT 8.8
				ker0_add1_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][0].range(19,4)) + (short)(mac_res_temp_0_fv0[pixel_fv4][1].range(19,4));
				ker0_add2_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][2].range(19,4)) + (short)(mac_res_temp_0_fv0[pixel_fv4][3].range(19,4));
				ker1_add1_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][0].range(19,4)) + (short)(mac_res_temp_1_fv0[pixel_fv4][1].range(19,4));
				ker1_add2_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][2].range(19,4)) + (short)(mac_res_temp_1_fv0[pixel_fv4][3].range(19,4));

				break;
			case 3:// IN 9.7 ----> OUT 11.5
				ker0_add1_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][0].range(24,9)) + (short)(mac_res_temp_0_fv0[pixel_fv4][1].range(24,9));
				ker0_add2_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][2].range(24,9)) + (short)(mac_res_temp_0_fv0[pixel_fv4][3].range(24,9));
				ker1_add1_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][0].range(24,9)) + (short)(mac_res_temp_1_fv0[pixel_fv4][1].range(24,9));
				ker1_add2_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][2].range(24,9)) + (short)(mac_res_temp_1_fv0[pixel_fv4][3].range(24,9));

				break;
			case 4:// IN 9.7 ----> OUT 15.1
				ker0_add1_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][0].range(28,13)) + (short)(mac_res_temp_0_fv0[pixel_fv4][1].range(28,13));
				ker0_add2_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][2].range(28,13)) + (short)(mac_res_temp_0_fv0[pixel_fv4][3].range(28,13));
				ker1_add1_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][0].range(28,13)) + (short)(mac_res_temp_1_fv0[pixel_fv4][1].range(28,13));
				ker1_add2_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][2].range(28,13)) + (short)(mac_res_temp_1_fv0[pixel_fv4][3].range(28,13));

				break;
			case 5:// IN 15.1 ----> OUT 8.8
				ker0_add1_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][0].range(15,0)) + (short)(mac_res_temp_0_fv0[pixel_fv4][1].range(15,0));
				ker0_add2_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][2].range(15,0)) + (short)(mac_res_temp_0_fv0[pixel_fv4][3].range(15,0));
				ker1_add1_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][0].range(15,0)) + (short)(mac_res_temp_1_fv0[pixel_fv4][1].range(15,0));
				ker1_add2_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][2].range(15,0)) + (short)(mac_res_temp_1_fv0[pixel_fv4][3].range(15,0));

				break;
			case 6:// IN 8.8 ----> OUT 15.1
				ker0_add1_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][0].range(29,14)) + (short)(mac_res_temp_0_fv0[pixel_fv4][1].range(29,14));
				ker0_add2_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][2].range(29,14)) + (short)(mac_res_temp_0_fv0[pixel_fv4][3].range(29,14));
				ker1_add1_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][0].range(29,14)) + (short)(mac_res_temp_1_fv0[pixel_fv4][1].range(29,14));
				ker1_add2_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][2].range(29,14)) + (short)(mac_res_temp_1_fv0[pixel_fv4][3].range(29,14));

				break;
			default:// IN 15.1 ----> OUT 11.5
				ker0_add1_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][0].range(18,3)) + (short)(mac_res_temp_0_fv0[pixel_fv4][1].range(18,3));
				ker0_add2_fv0 = (short)(mac_res_temp_0_fv0[pixel_fv4][2].range(18,3)) + (short)(mac_res_temp_0_fv0[pixel_fv4][3].range(18,3));
				ker1_add1_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][0].range(18,3)) + (short)(mac_res_temp_1_fv0[pixel_fv4][1].range(18,3));
				ker1_add2_fv0 = (short)(mac_res_temp_1_fv0[pixel_fv4][2].range(18,3)) + (short)(mac_res_temp_1_fv0[pixel_fv4][3].range(18,3));

				break;
			}//switch


			result_0_ping_fe0[pixel_fv4] = ker0_add1_fv0 + ker0_add2_fv0;
			result_1_ping_fe0[pixel_fv4] = ker1_add1_fv0 + ker1_add2_fv0;
		}//Pix64_Loop
	}//Once_Loop


#if 0//DEBUG_COMPUTE
	//*****************************
	int cnt = 0;
	for(int j=0;j<256;j++){ //planes

		//fprintf(fp_input,"\n Planes num:%d\n",j);

		fprintf(fp_weight,"\n Planes num:%d\n",j);
		for(int i=0;i<25;i++) //pixels
		{
			if(i%5 == 0)
			{
				//fprintf(fp_input,"\n");
				fprintf(fp_weight,"\n");
			}

			//fprintf(fp_input,"%d ",input_global[cnt]);
			fprintf(fp_weight,"%f ",((float)weights_global[cnt])/(1<<13));

			cnt++;
		}

	}

	fprintf(fp_weight,"\n%d____________________________________________________________\n ",(int)ok);
	//******************************

	cnt = 0;
	for(int j=0;j<256;j++){ //planes

		fprintf(fp_input,"\n Planes num:%d\n",j);
		for(int i=0;i<9;i++) //pixels
		{
			if(i%3 == 0)
			{
				//fprintf(fp_input,"\n");
				fprintf(fp_input,"\n");
			}

			//fprintf(fp_input,"%d ",input_global[cnt]);
			fprintf(fp_input,"%f ",((float)input_global[cnt])/(1<<7));

			cnt++;
		}
	}

	fprintf(fp_input,"\n%d____________________________________________________________\n ",(int)ok);
	//******************************
#endif

}

template<int CFILTER_SIZE,int CCONV_STRIDE,int CNUM_KERNELS,int IINPUT_PLANES,int PNKPF>
void Compute1Ker_fu(input_struct input_desc,
		weight_struct weight_desc,
		conv_struct conv_desc,
		weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024],
		ap_uint<4> nkpf_cnt_fe1,
		short result_0_ping_fe0[XI_PIX_PROC],
		ap_uint<3> ker_id_ft0)
{
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=2
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM

#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=2

#pragma HLS ARRAY_PARTITION variable=result_0_ping_fe0 complete dim=0
#pragma HLS INLINE off

	ap_uint<10> lcount_fu0;
	ap_uint<12> k_off_cnt_fu0;


	ap_int<40> mac_res_temp0_fu0[XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp0_fu0 complete dim=0

	lcount_fu0 = weight_desc.filter_size_square*(input_desc.compute_planes.range(15,2));
	k_off_cnt_fu0 = (4*lcount_fu0*nkpf_cnt_fe1) + (ker_id_ft0*lcount_fu0);

	Pix_Loop:
	for(ap_uint<8> pixel_fu0=0;pixel_fu0<XI_PIX_PROC;pixel_fu0++)
	{
#pragma HLS unroll
		Pln_Loop:
		for(ap_uint<4> plane_fu0=0;plane_fu0<4;plane_fu0++)
		{
#pragma HLS unroll
			mac_res_temp0_fu0[pixel_fu0][plane_fu0] = 0;
		}//Pln_Loop
	}//Pix_Loop

#if 0//DEBUG_COMPUTE
	fprintf(fp_input,"\n Current Plane id:%d\n",(int)current_plane);
	fprintf(fp_weight,"\n Current Plane id:%d\n",(int)current_plane);
#endif

	Compute_Loop:
	for(ap_uint<12> inp_buff_addr1_fu0=0, inp_buff_addr2_fu0=512 ,offset_kbuf = k_off_cnt_fu0;inp_buff_addr1_fu0<lcount_fu0;
			/*..continue..*/inp_buff_addr1_fu0++,inp_buff_addr2_fu0++,offset_kbuf++)
	{
#pragma HLS LOOP_TRIPCOUNT min=121 max=121
#pragma HLS PIPELINE

		short input_buf_copy_fu0[4][XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_fu0 complete dim=0
		Pln_Loop1:
		for(ap_uint<3> plane_fu1=0; plane_fu1<4;plane_fu1++)
		{
#pragma HLS unroll
			Pix_Loop1:
			for(ap_uint<8> pixel_fu1=0, pix_split_fu0 =0; pixel_fu1<(XI_PIX_PROC/2);pixel_fu1++, pix_split_fu0++)
			{
#pragma HLS unroll
				input_buf_copy_fu0[plane_fu1][pix_split_fu0] = input_buff0_fc0[plane_fu1][pixel_fu1][inp_buff_addr1_fu0];
				input_buf_copy_fu0[plane_fu1][pix_split_fu0+(XI_PIX_PROC/2)] = input_buff0_fc0[plane_fu1][pixel_fu1][inp_buff_addr2_fu0];
			}//Pix_Loop1
		}//Pln_Loop1

		weight_width k_buf_copy_0_fu0[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_0_fu0 complete dim=0

		Pln_Loop2:
		for(ap_uint<3> plane_fu2=0; plane_fu2<4;plane_fu2++)
		{
#pragma HLS unroll
			k_buf_copy_0_fu0[plane_fu2] = weight_buff0_fd0[0][plane_fu2][offset_kbuf];
		}//Pln_Loop2

		Pix_Loop2:
		for(ap_uint<8> pixel_fu2=0; pixel_fu2<XI_PIX_PROC;pixel_fu2++)
		{
#pragma HLS unroll
			Pln_Loop3:
			for(ap_uint<4> plane_fu3=0; plane_fu3<4;plane_fu3++)
			{
#pragma HLS unroll
				mac_res_temp0_fu0[pixel_fu2][plane_fu3] += (input_buf_copy_fu0[plane_fu3][pixel_fu2] * k_buf_copy_0_fu0[plane_fu3]);
			}//Pln_Loop3
		}//Pix_Loop3
	}//Compute_Loop

	Ones_LOOP:
	for(short once_fu0 = 0; once_fu0<1; once_fu0++)
	{
#pragma HLS pipeline
		Pix64_Loop:
		for(ap_uint<8> pixel_fu3=0; pixel_fu3<XI_PIX_PROC; pixel_fu3++)
		{
#pragma HLS UNROLL
			short ker0_add1_fu0, ker0_add2_fu0;

			switch(conv_desc.inout_precision)
			{
			case 0: //IN format = OUT format
				ker0_add1_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][0].range(22,7)) + (short)(mac_res_temp0_fu0[pixel_fu3][1].range(22,7));
				ker0_add2_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][2].range(22,7)) + (short)(mac_res_temp0_fu0[pixel_fu3][3].range(22,7));


				break;
			case 1:// IN 8.8 ----> OUT 11.5
				ker0_add1_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][0].range(25,10)) + (short)(mac_res_temp0_fu0[pixel_fu3][1].range(25,10));
				ker0_add2_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][2].range(25,10)) + (short)(mac_res_temp0_fu0[pixel_fu3][3].range(25,10));


				break;
			case 2:// IN 11.5 ----> OUT 8.8
				ker0_add1_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][0].range(19,4)) + (short)(mac_res_temp0_fu0[pixel_fu3][1].range(19,4));
				ker0_add2_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][2].range(19,4)) + (short)(mac_res_temp0_fu0[pixel_fu3][3].range(19,4));


				break;
			case 3:// IN 9.7 ----> OUT 11.5
				ker0_add1_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][0].range(24,9)) + (short)(mac_res_temp0_fu0[pixel_fu3][1].range(24,9));
				ker0_add2_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][2].range(24,9)) + (short)(mac_res_temp0_fu0[pixel_fu3][3].range(24,9));


				break;
			case 4:// IN 9.7 ----> OUT 15.1
				ker0_add1_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][0].range(28,13)) + (short)(mac_res_temp0_fu0[pixel_fu3][1].range(28,13));
				ker0_add2_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][2].range(28,13)) + (short)(mac_res_temp0_fu0[pixel_fu3][3].range(28,13));


				break;
			case 5:// IN 15.1 ----> OUT 8.8
				ker0_add1_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][0].range(15,0)) + (short)(mac_res_temp0_fu0[pixel_fu3][1].range(15,0));
				ker0_add2_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][2].range(15,0)) + (short)(mac_res_temp0_fu0[pixel_fu3][3].range(15,0));


				break;
			case 6:// IN 8.8 ----> OUT 15.1
				ker0_add1_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][0].range(29,14)) + (short)(mac_res_temp0_fu0[pixel_fu3][1].range(29,14));
				ker0_add2_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][2].range(29,14)) + (short)(mac_res_temp0_fu0[pixel_fu3][3].range(29,14));


				break;
			default:// IN 15.1 ----> OUT 11.5
				ker0_add1_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][0].range(18,3)) + (short)(mac_res_temp0_fu0[pixel_fu3][1].range(18,3));
				ker0_add2_fu0 = (short)(mac_res_temp0_fu0[pixel_fu3][2].range(18,3)) + (short)(mac_res_temp0_fu0[pixel_fu3][3].range(18,3));


				break;
			}//switch

			result_0_ping_fe0[pixel_fu3] = ker0_add1_fu0 + ker0_add2_fu0;
		}//Pix64_Loop
	}//Once_Loop


#if 0//DEBUG_COMPUTE
	//*****************************
	int cnt = 0;
	for(int j=0;j<256;j++){ //planes

		//fprintf(fp_input,"\n Planes num:%d\n",j);

		fprintf(fp_weight,"\n Planes num:%d\n",j);
		for(int i=0;i<25;i++) //pixels
		{
			if(i%5 == 0)
			{
				//fprintf(fp_input,"\n");
				fprintf(fp_weight,"\n");
			}

			//fprintf(fp_input,"%d ",input_global[cnt]);
			fprintf(fp_weight,"%f ",((float)weights_global[cnt])/(1<<13));

			cnt++;
		}

	}

	fprintf(fp_weight,"\n%d____________________________________________________________\n ",(int)ok);
	//******************************

	cnt = 0;
	for(int j=0;j<256;j++){ //planes

		fprintf(fp_input,"\n Planes num:%d\n",j);
		for(int i=0;i<9;i++) //pixels
		{
			if(i%3 == 0)
			{
				//fprintf(fp_input,"\n");
				fprintf(fp_input,"\n");
			}

			//fprintf(fp_input,"%d ",input_global[cnt]);
			fprintf(fp_input,"%f ",((float)input_global[cnt])/(1<<7));

			cnt++;
		}
	}

	fprintf(fp_input,"\n%d____________________________________________________________\n ",(int)ok);
	//******************************
#endif

}

template<int CFILTER_SIZE,int CCONV_STRIDE,int CNUM_KERNELS,int IINPUT_PLANES,int PNKPF>
void Compute_ft(input_struct input_desc,
		weight_struct weight_desc,
		conv_struct conv_desc,
		weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024],
		ap_uint<4> nkpf_cnt_fe1,
		short result_0_ping_fe0[XI_PIX_PROC],
		short result_1_ping_fe0[XI_PIX_PROC],
		short result_2_ping_fe0[XI_PIX_PROC],
		short result_3_ping_fe0[XI_PIX_PROC])
{
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=2
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM

#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=2

#pragma HLS ARRAY_PARTITION variable=result_0_ping_fe0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=result_1_ping_fe0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=result_2_ping_fe0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=result_3_ping_fe0 complete dim=0
#pragma HLS INLINE off

#if XI_KER_PROC==4
	Compute4Ker_fw<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
	(input_desc,weight_desc, conv_desc, weight_buff0_fd0, input_buff0_fc0, nkpf_cnt_fe1,result_0_ping_fe0, result_1_ping_fe0, result_2_ping_fe0, result_3_ping_fe0);
#elif XI_KER_PROC==2
	Compute2Ker_fv<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
	(input_desc,weight_desc, conv_desc, weight_buff0_fd0, input_buff0_fc0, nkpf_cnt_fe1,result_0_ping_fe0,result_1_ping_fe0,0);

	Compute2Ker_fv<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
	(input_desc,weight_desc, conv_desc, weight_buff0_fd0, input_buff0_fc0, nkpf_cnt_fe1,result_2_ping_fe0,result_3_ping_fe0,1);
#elif XI_KER_PROC==1
	Compute1Ker_fu<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
	(input_desc,weight_desc, conv_desc, weight_buff0_fd0, input_buff0_fc0, nkpf_cnt_fe1,result_0_ping_fe0,0);

	Compute1Ker_fu<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
	(input_desc,weight_desc, conv_desc, weight_buff0_fd0, input_buff0_fc0, nkpf_cnt_fe1,result_1_ping_fe0,1);

	Compute1Ker_fu<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
	(input_desc,weight_desc, conv_desc, weight_buff0_fd0, input_buff0_fc0, nkpf_cnt_fe1,result_2_ping_fe0,2);

	Compute1Ker_fu<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
	(input_desc,weight_desc, conv_desc, weight_buff0_fd0, input_buff0_fc0, nkpf_cnt_fe1,result_3_ping_fe0,3);
#endif

}

template<int PNKPF>
void OStgBuffSeq4Ker_fs(short result_0_ping_fe0[XI_PIX_PROC],
		short result_1_ping_fe0[XI_PIX_PROC],
		short result_2_ping_fe0[XI_PIX_PROC],
		short result_3_ping_fe0[XI_PIX_PROC],
		short bias_buff_fb0[XI_BIASMAXSIZE],
		out_pix_struct out_pixels0_fc0,
		ap_uint<16> nk_process_fd0,
		conv_struct conv_desc,
		output_struct output_desc,
		short ostaging_buff0_fb0[4][8][XI_OSTAGEBUFF_PIX][XI_OSTAGEBUFF_DIM4],
		ap_uint<12> rows_to_process_fb0,
		ap_uint<4> nkpf_cnt,
		ap_uint<12> out_row_offset_fb0)
{

#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_rows complete dim=1
#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_cols complete dim=1
#pragma HLS data_pack variable=out_pixels0_fc0
#pragma HLS ARRAY_PARTITION variable=result_0_ping_fe0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=result_1_ping_fe0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=result_2_ping_fe0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=result_3_ping_fe0 complete dim=0
#pragma HLS interface register port=result_0_ping_fe0
#pragma HLS interface register port=result_1_ping_fe0
#pragma HLS interface register port=result_2_ping_fe0
#pragma HLS interface register port=result_3_ping_fe0
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=3
#pragma HLS INLINE OFF

	ap_uint<16> output_kernel_id_fs0 = nk_process_fd0 + nkpf_cnt*4;
	ap_uint<16> row_proc_prod_fs0 = rows_to_process_fb0*output_desc.width;
	ap_uint<16> pixbuff_planeoffset_fs0 = (output_kernel_id_fs0.range(15,5)) * (conv_desc.out_pix);

	short biasval_fs0[4];
#pragma HLS ARRAY_PARTITION variable=biasval_fs0 complete dim=0

	Plane_Loop:
	for(ap_uint<3> plane_fs0=0;plane_fs0<4;plane_fs0++)
	{
#pragma HLS unroll
		biasval_fs0[plane_fs0] = bias_buff_fb0[output_kernel_id_fs0+plane_fs0];
	}//Plane_Loop

	ap_uint<16> rows_array_fs0[XI_PIX_PROC],col_array_fs0[XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=rows_array_fs0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=col_array_fs0 complete dim=0
	Pix_Loop:
	for(ap_uint<8> pix_fs0=0;pix_fs0<XI_PIX_PROC;pix_fs0++)
	{
#pragma HLS unroll
		rows_array_fs0[pix_fs0] = out_pixels0_fc0.pix_rows[pix_fs0];
		col_array_fs0[pix_fs0] = out_pixels0_fc0.pix_cols[pix_fs0];
	}//Pix_Loop


#if XI_OSTAGEBUFF_PIX==4
	Write_Loop:
	for(ap_uint<8> ostg_pix1_fs0=0,ostg_pix2_fs0=1, ostg_pix3_fs1=2, ostg_pix4_fs1=3;ostg_pix1_fs0<XI_PIX_PROC;
			/*..continue..*/ostg_pix1_fs0+=4,ostg_pix2_fs0+=4,ostg_pix3_fs1+=4,ostg_pix4_fs1+=4)
	{
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=ostaging_buff0_fb0 inter false
#pragma HLS DEPENDENCE variable=ostaging_buff0_fb0 intra false

		ap_uint<16> in_pix_row_fs0 = rows_array_fs0[ostg_pix1_fs0] - out_row_offset_fb0;
		ap_uint<12> pix_col_fs0 = col_array_fs0[ostg_pix1_fs0];
		ap_uint<16> in_pix_row_2nd_fs0 = rows_array_fs0[ostg_pix2_fs0] - out_row_offset_fb0;
		ap_uint<12> pix_col_2nd_fs0 = col_array_fs0[ostg_pix2_fs0];
		ap_uint<16> in_pix_row_3rd_fs0 = rows_array_fs0[ostg_pix3_fs1] - out_row_offset_fb0;
		ap_uint<12> pix_col_3rd_fs0 = col_array_fs0[ostg_pix3_fs1];
		ap_uint<16> in_pix_row_4th_fs0 = rows_array_fs0[ostg_pix4_fs1] - out_row_offset_fb0;
		ap_uint<12> pix_col_4th_fs0 = col_array_fs0[ostg_pix4_fs1];

		ap_uint<14> pixbuff_rowoffset_fs0 = in_pix_row_fs0 * output_desc.width;
		ap_uint<14> pixbuff_row_fs0 = (pix_col_fs0 + pixbuff_rowoffset_fs0);

		ap_uint<14> pixbuff_rowoffset_2nd_fs0 = in_pix_row_2nd_fs0 * output_desc.width;
		ap_uint<14> pixbuff_row_2nd_fs0 = (pix_col_2nd_fs0 + pixbuff_rowoffset_2nd_fs0);

		ap_uint<14> pixbuff_rowoffset_3rd_fs0 = in_pix_row_3rd_fs0 * output_desc.width;
		ap_uint<14> pixbuff_row_3rd_fs0 = (pix_col_3rd_fs0 + pixbuff_rowoffset_3rd_fs0);

		ap_uint<14> pixbuff_rowoffset_4th_fs0 = in_pix_row_4th_fs0 * output_desc.width;
		ap_uint<14> pixbuff_row_4th_fs0 = (pix_col_4th_fs0 + pixbuff_rowoffset_4th_fs0);

		bool expression1_fs0 = (pixbuff_row_fs0 < row_proc_prod_fs0);
		bool expression2_fs0 = (pixbuff_row_2nd_fs0 < row_proc_prod_fs0);
		bool expression3_fs0 = (pixbuff_row_3rd_fs0 < row_proc_prod_fs0);
		bool expression4_fs0 = (pixbuff_row_4th_fs0 < row_proc_prod_fs0);

		ap_uint<12> pixbuff_index_fs0 = pixbuff_row_fs0 + pixbuff_planeoffset_fs0;

		ap_uint<3> dim2_ostg_fs0 = output_kernel_id_fs0.range(4,2);

		short ostg_src_fs0[4][8];
#pragma HLS ARRAY_PARTITION variable=ostg_src_fs0 complete dim=0
		short ostg_src_2nd_fs0[4][8];
#pragma HLS ARRAY_PARTITION variable=ostg_src_2nd_fs0 complete dim=0
		short ostg_src_3rd_fs0[4][8];
#pragma HLS ARRAY_PARTITION variable=ostg_src_3rd_fs0 complete dim=0
		short ostg_src_4th_fs0[4][8];
#pragma HLS ARRAY_PARTITION variable=ostg_src_4th_fs0 complete dim=0

		Part1_Loop:
		for(ap_uint<4> partition1_fs0=0;partition1_fs0<4;partition1_fs0++)
		{
			Part2_Loop:
			for(ap_uint<4> partition2_fs0=0;partition2_fs0<8;partition2_fs0++)
			{
				ostg_src_fs0[partition1_fs0][partition2_fs0]  	= ostaging_buff0_fb0[partition1_fs0][partition2_fs0][0][pixbuff_index_fs0.range(11,2)];
				ostg_src_2nd_fs0[partition1_fs0][partition2_fs0] = ostaging_buff0_fb0[partition1_fs0][partition2_fs0][1][pixbuff_index_fs0.range(11,2)];
				ostg_src_3rd_fs0[partition1_fs0][partition2_fs0] = ostaging_buff0_fb0[partition1_fs0][partition2_fs0][2][pixbuff_index_fs0.range(11,2)];
				ostg_src_4th_fs0[partition1_fs0][partition2_fs0] = ostaging_buff0_fb0[partition1_fs0][partition2_fs0][3][pixbuff_index_fs0.range(11,2)];

			}//Part2_Loop
		}//Part1_Loop

		short read_ostg_fs0[4];
#pragma HLS ARRAY_PARTITION variable=read_ostg_fs0 complete dim=1
		short read_ostg_2nd_fs0[4];
#pragma HLS ARRAY_PARTITION variable=read_ostg_2nd_fs0 complete dim=1
		short read_ostg_3rd_fs0[4];
#pragma HLS ARRAY_PARTITION variable=read_ostg_3rd_fs0 complete dim=1
		short read_ostg_4th_fs0[4];
#pragma HLS ARRAY_PARTITION variable=read_ostg_4th_fs0 complete dim=1

		Part1_Loop2:
		for(ap_uint<3> partition1_fs2=0;partition1_fs2<4;partition1_fs2++)
		{
#pragma HLS UNROLL
			read_ostg_fs0[partition1_fs2] = ostg_src_fs0[partition1_fs2][dim2_ostg_fs0];
			read_ostg_2nd_fs0[partition1_fs2] = ostg_src_2nd_fs0[partition1_fs2][dim2_ostg_fs0];
			read_ostg_3rd_fs0[partition1_fs2] = ostg_src_3rd_fs0[partition1_fs2][dim2_ostg_fs0];
			read_ostg_4th_fs0[partition1_fs2] = ostg_src_4th_fs0[partition1_fs2][dim2_ostg_fs0];
		}//Part1_Loop2

		short result_inc_fs0[4];
#pragma HLS ARRAY_PARTITION variable=result_inc_fs0 complete dim=1
		short result_inc_2nd_fs0[4];
#pragma HLS ARRAY_PARTITION variable=result_inc_2nd_fs0 complete dim=1
		short result_inc_3rd_fs0[4];
#pragma HLS ARRAY_PARTITION variable=result_inc_3rd_fs0 complete dim=1
		short result_inc_4th_fs0[4];
#pragma HLS ARRAY_PARTITION variable=result_inc_4th_fs0 complete dim=1

		Part1_Loop3:
		for(ap_uint<3> partition1_fs3=0;partition1_fs3<4;partition1_fs3++)
		{
#pragma HLS UNROLL
			if(out_pixels0_fc0.current_plane_by4 !=0)
			{
				result_inc_fs0[partition1_fs3] = read_ostg_fs0[partition1_fs3];
				result_inc_2nd_fs0[partition1_fs3] = read_ostg_2nd_fs0[partition1_fs3];
				result_inc_3rd_fs0[partition1_fs3] = read_ostg_3rd_fs0[partition1_fs3];
				result_inc_4th_fs0[partition1_fs3] = read_ostg_4th_fs0[partition1_fs3];
			}
			else
			{
				result_inc_fs0[partition1_fs3] 	  = biasval_fs0[partition1_fs3];
				result_inc_2nd_fs0[partition1_fs3] = biasval_fs0[partition1_fs3];
				result_inc_3rd_fs0[partition1_fs3] = biasval_fs0[partition1_fs3];
				result_inc_4th_fs0[partition1_fs3] = biasval_fs0[partition1_fs3];
			}
		}//Part1_Loop3

		short result_ostg_fs0[4];
#pragma HLS ARRAY_PARTITION variable=result_ostg_fs0 complete dim=1
		short result_ostg_2nd_fs0[4];
#pragma HLS ARRAY_PARTITION variable=result_ostg_2nd_fs0 complete dim=1
		short result_ostg_3rd_fs0[4];
#pragma HLS ARRAY_PARTITION variable=result_ostg_3rd_fs0 complete dim=1
		short result_ostg_4th_fs0[4];
#pragma HLS ARRAY_PARTITION variable=result_ostg_4th_fs0 complete dim=1

		result_ostg_fs0[0] = result_0_ping_fe0[ostg_pix1_fs0]+result_inc_fs0[0];
		result_ostg_fs0[1] = result_1_ping_fe0[ostg_pix1_fs0]+result_inc_fs0[1];
		result_ostg_fs0[2] = result_2_ping_fe0[ostg_pix1_fs0]+result_inc_fs0[2];
		result_ostg_fs0[3] = result_3_ping_fe0[ostg_pix1_fs0]+result_inc_fs0[3];

		result_ostg_2nd_fs0[0] = result_0_ping_fe0[ostg_pix2_fs0]+result_inc_2nd_fs0[0];
		result_ostg_2nd_fs0[1] = result_1_ping_fe0[ostg_pix2_fs0]+result_inc_2nd_fs0[1];
		result_ostg_2nd_fs0[2] = result_2_ping_fe0[ostg_pix2_fs0]+result_inc_2nd_fs0[2];
		result_ostg_2nd_fs0[3] = result_3_ping_fe0[ostg_pix2_fs0]+result_inc_2nd_fs0[3];

		result_ostg_3rd_fs0[0] = result_0_ping_fe0[ostg_pix3_fs1]+result_inc_3rd_fs0[0];
		result_ostg_3rd_fs0[1] = result_1_ping_fe0[ostg_pix3_fs1]+result_inc_3rd_fs0[1];
		result_ostg_3rd_fs0[2] = result_2_ping_fe0[ostg_pix3_fs1]+result_inc_3rd_fs0[2];
		result_ostg_3rd_fs0[3] = result_3_ping_fe0[ostg_pix3_fs1]+result_inc_3rd_fs0[3];

		result_ostg_4th_fs0[0] = result_0_ping_fe0[ostg_pix4_fs1]+result_inc_4th_fs0[0];
		result_ostg_4th_fs0[1] = result_1_ping_fe0[ostg_pix4_fs1]+result_inc_4th_fs0[1];
		result_ostg_4th_fs0[2] = result_2_ping_fe0[ostg_pix4_fs1]+result_inc_4th_fs0[2];
		result_ostg_4th_fs0[3] = result_3_ping_fe0[ostg_pix4_fs1]+result_inc_4th_fs0[3];

		Part1_Loop4:
		for(ap_uint<3> partition1_fs4=0;partition1_fs4<4;partition1_fs4++)
		{
#pragma HLS UNROLL
			Part2_Loop4:
			for(ap_uint<4> partition2_fs4=0;partition2_fs4<8;partition2_fs4++)
			{
#pragma HLS UNROLL
				if(partition2_fs4 ==dim2_ostg_fs0){
					if(expression1_fs0)
						ostaging_buff0_fb0[partition1_fs4][partition2_fs4][0][pixbuff_index_fs0.range(11,2)] = result_ostg_fs0[partition1_fs4];
					if(expression2_fs0)
						ostaging_buff0_fb0[partition1_fs4][partition2_fs4][1][pixbuff_index_fs0.range(11,2)] = result_ostg_2nd_fs0[partition1_fs4];
					if(expression3_fs0)
						ostaging_buff0_fb0[partition1_fs4][partition2_fs4][2][pixbuff_index_fs0.range(11,2)] = result_ostg_3rd_fs0[partition1_fs4];
					if(expression4_fs0)
						ostaging_buff0_fb0[partition1_fs4][partition2_fs4][3][pixbuff_index_fs0.range(11,2)] = result_ostg_4th_fs0[partition1_fs4];
				}
			}//Part2_Loop4
		}//Part1_Loop4
	}//Write_Loop
#endif

#if XI_OSTAGEBUFF_PIX==2
	Write_Loop:
	for(ap_uint<8> ostg_pix1_fs1=0,ostg_pix2_fs1=1;ostg_pix1_fs1<XI_PIX_PROC;ostg_pix1_fs1+=2,ostg_pix2_fs1+=2)
	{
#pragma HLS LOOP_TRIPCOUNT min=64 max=64
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=ostaging_buff0_fb0 inter false
#pragma HLS DEPENDENCE variable=ostaging_buff0_fb0 intra false

		ap_uint<16> in_pix_row_fs0 = rows_array_fs0[ostg_pix1_fs1] - out_row_offset_fb0;
		ap_uint<12> pix_col_fs0 = col_array_fs0[ostg_pix1_fs1];
		ap_uint<16> in_pix_row_2nd_fs0 = rows_array_fs0[ostg_pix2_fs1] - out_row_offset_fb0;
		ap_uint<12> pix_col_2nd_fs0 = col_array_fs0[ostg_pix2_fs1];

		ap_uint<14> pixbuff_rowoffset_fs0 = in_pix_row_fs0 * output_desc.width;
		ap_uint<14> pixbuff_row_fs0 = (pix_col_fs0 + pixbuff_rowoffset_fs0);

		ap_uint<14> pixbuff_rowoffset_2nd_fs0 = in_pix_row_2nd_fs0 * output_desc.width;
		ap_uint<14> pixbuff_row_2nd_fs0 = (pix_col_2nd_fs0 + pixbuff_rowoffset_2nd_fs0);

		bool expression1_fs1 = (pixbuff_row_fs0 < row_proc_prod_fs0);
		bool expression2_fs1 = (pixbuff_row_2nd_fs0 < row_proc_prod_fs0);

		ap_uint<12> pixbuff_index_fs1 = pixbuff_row_fs0 + pixbuff_planeoffset_fs0;

		ap_uint<3> dim2_ostg_fs1 = output_kernel_id_fs0.range(4,2);


		short read_ostg_fs1[4];
#pragma HLS ARRAY_PARTITION variable=read_ostg_fs1 complete dim=1
		short read_ostg_2nd_fs1[4];
#pragma HLS ARRAY_PARTITION variable=read_ostg_2nd_fs1 complete dim=1

		short ostg_src_fs1[4][8];
#pragma HLS ARRAY_PARTITION variable=ostg_src_fs1 complete dim=0
		short ostg_src_2nd_fs1[4][8];
#pragma HLS ARRAY_PARTITION variable=ostg_src_2nd_fs1 complete dim=0

		Part1_Loop5:
		for(ap_uint<4> partition1_fs5=0;partition1_fs5<4;partition1_fs5++)
		{
			Part2_Loop5:
			for(ap_uint<4> partition2_fs5=0;partition2_fs5<8;partition2_fs5++)
			{
				ostg_src_fs1[partition1_fs5][partition2_fs5] = ostaging_buff0_fb0[partition1_fs5][partition2_fs5][0][pixbuff_index_fs1.range(11,1)];
				ostg_src_2nd_fs1[partition1_fs5][partition2_fs5] = ostaging_buff0_fb0[partition1_fs5][partition2_fs5][1][pixbuff_index_fs1.range(11,1)];
			}//Part2_Loop5
		}//Part1_Loop5

		Part1_Loop6:
		for(ap_uint<3> partition1_fs6=0;partition1_fs6<4;partition1_fs6++)
		{
#pragma HLS UNROLL
			read_ostg_fs1[partition1_fs6] = ostg_src_fs1[partition1_fs6][dim2_ostg_fs1];
			read_ostg_2nd_fs1[partition1_fs6] = ostg_src_2nd_fs1[partition1_fs6][dim2_ostg_fs1];
		}//Part1_Loop6

		short result_inc_fs1[4];
#pragma HLS ARRAY_PARTITION variable=result_inc_fs1 complete dim=1
		short result_inc_2nd_fs1[4];
#pragma HLS ARRAY_PARTITION variable=result_inc_2nd_fs1 complete dim=1

		Part1_Loop7:
		for(ap_uint<3> partition1_fs7=0;partition1_fs7<4;partition1_fs7++)
		{
#pragma HLS UNROLL
			if(out_pixels0_fc0.current_plane_by4 !=0)
			{
				result_inc_fs1[partition1_fs7] = read_ostg_fs1[partition1_fs7];
				result_inc_2nd_fs1[partition1_fs7] = read_ostg_2nd_fs1[partition1_fs7];
			}
			else
			{
				result_inc_fs1[partition1_fs7] = biasval_fs0[partition1_fs7];
				result_inc_2nd_fs1[partition1_fs7] = biasval_fs0[partition1_fs7];
			}
		}//Part1_Loop7

		short result_ostg_fs1[4];
#pragma HLS ARRAY_PARTITION variable=result_ostg_fs1 complete dim=1
		short result_ostg_2nd_fs1[4];
#pragma HLS ARRAY_PARTITION variable=result_ostg_2nd_fs1 complete dim=1

		result_ostg_fs1[0] = result_0_ping_fe0[ostg_pix1_fs1]+result_inc_fs1[0];
		result_ostg_fs1[1] = result_1_ping_fe0[ostg_pix1_fs1]+result_inc_fs1[1];
		result_ostg_fs1[2] = result_2_ping_fe0[ostg_pix1_fs1]+result_inc_fs1[2];
		result_ostg_fs1[3] = result_3_ping_fe0[ostg_pix1_fs1]+result_inc_fs1[3];

		result_ostg_2nd_fs1[0] = result_0_ping_fe0[ostg_pix2_fs1]+result_inc_2nd_fs1[0];
		result_ostg_2nd_fs1[1] = result_1_ping_fe0[ostg_pix2_fs1]+result_inc_2nd_fs1[1];
		result_ostg_2nd_fs1[2] = result_2_ping_fe0[ostg_pix2_fs1]+result_inc_2nd_fs1[2];
		result_ostg_2nd_fs1[3] = result_3_ping_fe0[ostg_pix2_fs1]+result_inc_2nd_fs1[3];

		Part1_Loop8:
		for(ap_uint<3> partition1_fs8=0;partition1_fs8<4;partition1_fs8++)
		{
#pragma HLS UNROLL
			if(expression1_fs1)
				ostaging_buff0_fb0[partition1_fs8][dim2_ostg_fs1][0][pixbuff_index_fs1.range(11,1)] = result_ostg_fs1[partition1_fs8];
			if(expression2_fs1)
				ostaging_buff0_fb0[partition1_fs8][dim2_ostg_fs1][1][pixbuff_index_fs1.range(11,1)] = result_ostg_2nd_fs1[partition1_fs8];

		}//Part1_Loop8

	}
#endif

#if XI_OSTAGEBUFF_PIX==1

	Write_Loop:
	for(ap_uint<8> ostg_pix1_fs1=0;ostg_pix1_fs1<XI_PIX_PROC;ostg_pix1_fs1++)
	{
#pragma HLS LOOP_TRIPCOUNT min=64 max=64
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=ostaging_buff0_fb0 inter false
#pragma HLS DEPENDENCE variable=ostaging_buff0_fb0 intra false

		ap_uint<16> in_pix_row_fs1 = out_pixels0_fc0.pix_rows[ostg_pix1_fs1] - out_row_offset_fb0;
		ap_uint<12> pix_col_fs1 = out_pixels0_fc0.pix_cols[ostg_pix1_fs1];
		ap_uint<14> pixbuff_rowoffset_fs1 = in_pix_row_fs1 * output_desc.width;
		ap_uint<14> pixbuff_row_fs1 = (pix_col_fs1 + pixbuff_rowoffset_fs1);
		ap_uint<6> index = (output_kernel_id_fs0 & 0x1F);
		ap_uint<12> pixbuff_index_fs1 = pixbuff_row_fs1 + pixbuff_planeoffset_fs0;
		bool expression1_fs1 = (pixbuff_row_fs1 < row_proc_prod_fs0);
		ap_uint<3> dim2_ostg_fs1 = index.range(4,2);

		short read_ostg_fs1[4];
#pragma HLS ARRAY_PARTITION variable=read_ostg_fs1 complete dim=1

		short ostg_src_fs1[4][8];
#pragma HLS ARRAY_PARTITION variable=ostg_src_fs1 complete dim=0

		Part1_Loop5:
		for(ap_uint<4> partition1_fs5=0;partition1_fs5<4;partition1_fs5++)
		{
			Part2_Loop5:
			for(ap_uint<4> partition2_fs5=0;partition2_fs5<8;partition2_fs5++)
			{
				ostg_src_fs1[partition1_fs5][partition2_fs5] = ostaging_buff0_fb0[partition1_fs5][partition2_fs5][0][pixbuff_index_fs1];
			}//Part2_Loop5
		}//Part1_Loop5

		Part1_Loop6:
		for(ap_uint<3> partition1_fs6=0;partition1_fs6<4;partition1_fs6++)
		{
#pragma HLS UNROLL
			read_ostg_fs1[partition1_fs6] = ostg_src_fs1[partition1_fs6][dim2_ostg_fs1];
		}//Part1_Loop6

		short result_inc_fs1[4];
#pragma HLS ARRAY_PARTITION variable=result_inc_fs1 complete dim=1

		Part1_Loop7:
		for(ap_uint<3> partition1_fs7=0;partition1_fs7<4;partition1_fs7++)
		{
#pragma HLS UNROLL
			if(out_pixels0_fc0.current_plane_by4 !=0)
			{
				result_inc_fs1[partition1_fs7] = read_ostg_fs1[partition1_fs7];
			}
			else
			{
				result_inc_fs1[partition1_fs7] = biasval_fs0[partition1_fs7];
			}
		}//Part1_Loop7

		short result_ostg_fs1[4];
#pragma HLS ARRAY_PARTITION variable=result_ostg_fs1 complete dim=1

		result_ostg_fs1[0] = result_0_ping_fe0[ostg_pix1_fs1]+result_inc_fs1[0];
		result_ostg_fs1[1] = result_1_ping_fe0[ostg_pix1_fs1]+result_inc_fs1[1];
		result_ostg_fs1[2] = result_2_ping_fe0[ostg_pix1_fs1]+result_inc_fs1[2];
		result_ostg_fs1[3] = result_3_ping_fe0[ostg_pix1_fs1]+result_inc_fs1[3];

		Part1_Loop8:
		for(ap_uint<3> partition1_fs8=0;partition1_fs8<4;partition1_fs8++)
		{
#pragma HLS UNROLL
			if(expression1_fs1)
			{
				ostaging_buff0_fb0[partition1_fs8][dim2_ostg_fs1][0][pixbuff_index_fs1] = result_ostg_fs1[partition1_fs8];
			}
		}//Part1_Loop8

	}

#endif//XI_OSTAGEBUFF_PIX

}

void InputBuffWrite_fr(short tempvalue_fp0[4],
		ap_uint<10> tempdepth_fp0[7],
		ap_uint<6> tempdim_fp0[7],
		bool tempflags_fp0[6],
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024])
{

#pragma HLS interface register port=tempvalue_fp0
#pragma HLS interface register port=tempdepth_fp0
#pragma HLS interface register port=tempdim_fp0
#pragma HLS interface register port=tempflags_fp0
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=2
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM


#pragma HLS INLINE OFF


	ap_uint<7> pix_to_process_fr0 = XI_PIX_PROC/2;

	Dim1_Loop:
	for(ap_uint<4> inp_buf_dim1_fr0 = 0;inp_buf_dim1_fr0<4;inp_buf_dim1_fr0++)
	{
#pragma HLS UNROLL
		Dim2_Loop:
		for(ap_uint<7> inp_buf_dim2_fr0 = 0;inp_buf_dim2_fr0<pix_to_process_fr0;inp_buf_dim2_fr0++)
		{
#pragma HLS UNROLL
			if(inp_buf_dim2_fr0 == tempdim_fp0[0])
				input_buff0_fc0[inp_buf_dim1_fr0][inp_buf_dim2_fr0][tempdepth_fp0[0]] = tempvalue_fp0[inp_buf_dim1_fr0];
			else if((inp_buf_dim2_fr0 == tempdim_fp0[1]) && (tempflags_fp0[0] == 1))
				input_buff0_fc0[inp_buf_dim1_fr0][inp_buf_dim2_fr0][tempdepth_fp0[1]] = tempvalue_fp0[inp_buf_dim1_fr0];
			else if((inp_buf_dim2_fr0 == tempdim_fp0[2]) && (tempflags_fp0[1] == 1))
				input_buff0_fc0[inp_buf_dim1_fr0][inp_buf_dim2_fr0][tempdepth_fp0[2]] = tempvalue_fp0[inp_buf_dim1_fr0];
			else if((inp_buf_dim2_fr0 == tempdim_fp0[3]) && (tempflags_fp0[2] == 1))
				input_buff0_fc0[inp_buf_dim1_fr0][inp_buf_dim2_fr0][tempdepth_fp0[3]] = tempvalue_fp0[inp_buf_dim1_fr0];
			else if((inp_buf_dim2_fr0 == tempdim_fp0[4]) && (tempflags_fp0[3] == 1))
				input_buff0_fc0[inp_buf_dim1_fr0][inp_buf_dim2_fr0][tempdepth_fp0[4]] = tempvalue_fp0[inp_buf_dim1_fr0];
			else if((inp_buf_dim2_fr0 == tempdim_fp0[5]) && (tempflags_fp0[4] == 1))
				input_buff0_fc0[inp_buf_dim1_fr0][inp_buf_dim2_fr0][tempdepth_fp0[5]] = tempvalue_fp0[inp_buf_dim1_fr0];
			else if((inp_buf_dim2_fr0 == tempdim_fp0[6]) && (tempflags_fp0[5] == 1))
				input_buff0_fc0[inp_buf_dim1_fr0][inp_buf_dim2_fr0][tempdepth_fp0[6]] = tempvalue_fp0[inp_buf_dim1_fr0];
		}//Dim2_Loop
	}//Dim1_Loop

}

template<int CFILTER_SIZE, int CCONV_STRIDE, int CNUM_KERNELS, int INPUTP>
void RowPixFunction_fo(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		out_pix2_struct &outpixels1_fl0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024],
		ap_uint<16> opix_row_fl0,
		ap_uint<16> opix_col_fl0,
		ap_uint<16> opix_row_wo_offset_fl0,
		ap_uint<10> current_plane_fc0,
		ap_uint<10> inputBuff_bram_offset_fl0,
		ap_int<12> pad_row_fc0,
		ap_int<12> pad_row_wo_fc0,
		ap_uint<7> pxcnt_fn0,
		ap_uint<7> pixel_tobe_process_fn0,
		ap_uint<10> iteration_fn0,
		ap_uint<2> mac_fz0)
{
#pragma HLS inline off

	ap_uint<8> fsz_fst_div_fo0 = conv_desc.fsz_by_stride;
	ap_uint<8> fsz_fst_mod_fo0 = conv_desc.fsz_mod_stride;

	bool initial_flag_reg_fo0[XI_PIX_PROC/2];
#pragma HLS ARRAY_PARTITION variable=initial_flag_reg_fo0 complete dim=0
	bool flag_reg_fo0[XI_PIX_PROC/2];
#pragma HLS ARRAY_PARTITION variable=flag_reg_fo0 complete dim=0
	ap_uint<10> addr_bram_fo0[XI_PIX_PROC/2];
#pragma HLS ARRAY_PARTITION variable=addr_bram_fo0 complete dim=0
	ap_uint<3> dilated_counter_fo0[XI_PIX_PROC/2];
#pragma HLS ARRAY_PARTITION variable=dilated_counter_fo0 complete dim=0
	Pix_fo0_Loop:
	for(ap_uint<8> pix_fo0  =0; pix_fo0<XI_PIX_PROC/2; pix_fo0++)
	{
#pragma HLS unroll
		dilated_counter_fo0[pix_fo0]=0;
		addr_bram_fo0[pix_fo0]=inputBuff_bram_offset_fl0;

		if(pix_fo0 == pxcnt_fn0)
			initial_flag_reg_fo0[pix_fo0] = 1;
		else
			initial_flag_reg_fo0[pix_fo0] = 0;
		flag_reg_fo0[pix_fo0] = initial_flag_reg_fo0[pix_fo0];
	}

	//	ap_uint<16> col_max_mul = weight_desc.filter_stride*(pixel_tobe_process_fn0);
	//#pragma HLS RESOURCE variable=col_max_mul core=MulnS latency=2
	ap_uint<16> col_max_fo0 = weight_desc.filter_size_dilated +  weight_desc.filter_stride*(pixel_tobe_process_fn0);

	ap_uint<7> stride_counter_fo0=0;

	//************** LRN inter *******************
	ap_int<8> lrn_counter;
	ap_uint<8> pattern_8kp;
	if(weight_desc.filter_size == 9)
	{
		lrn_counter = 1;
		pattern_8kp = 1;
	}
	else if(weight_desc.filter_size == 7)
	{
		lrn_counter = 0;
		pattern_8kp = 0;
	}
	else if(weight_desc.filter_size == 5)
	{
		lrn_counter = -1;
		pattern_8kp = 0;
	}
	else//(weight_desc.filter_size == 3)
	{
		lrn_counter = -2;
		pattern_8kp = 0;
	}
	ap_uint<16> lrn_row_offset;
	ap_uint<16> lrn_row_offset_opix;
	ap_uint<16> lrn_col_offset;
	ap_uint<30> flag_1st_32pix_buff_previous_iteration, flag_next_32pix_buff_previous_iteration;
	//********************************************

#if 0//!__SYNTHESIS__
	fprintf(stderr, "\n outpix row:col = %d:%d           current_plane=%d", (int)opix_row_fl0  ,(int)opix_col_fl0, (int)current_plane_fc0);
#endif

	/*	for(ap_uint<10> addr=0, addr1=512 ; addr<4; addr++, addr1++)
	{
#pragma HLS PIPELINE
		for(ap_uint<7> pix_buf=0; pix_buf < XI_PIX_PROC/2; pix_buf++)
		{
#pragma HLS unroll
			for(ap_uint<4> pl=0; pl<4; pl++)
			{
#pragma HLS unroll
				if(conv_desc.lrn_inter_sos_enable)
				{
					input_buff0_fc0[pl][pix_buf][addr] = 0;
					input_buff0_fc0[pl][pix_buf][addr1] = 0;
				}
			}
		}
	}*/

	Plane_Loop:
	for(ap_uint<16> plane4_fo0 = 0; plane4_fo0 < conv_desc.feeding_buff_plane_loop_bound; plane4_fo0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=1 max=1
		Row_Loop:
		for(ap_uint<8> row_cnt_fo0 = 0; row_cnt_fo0 < conv_desc.feeding_buff_row_loop_bound; row_cnt_fo0++)
		{
#pragma HLS LOOP_TRIPCOUNT min=7 max=7
			Col_Loop:
			for(ap_uint<16> col_cnt_fo0 = 0; col_cnt_fo0 < col_max_fo0; col_cnt_fo0++)
				//			for(ap_uint<16> col_cnt_fo0 = 0; col_cnt_fo0 < 1; col_cnt_fo0++)
			{
#pragma HLS LOOP_TRIPCOUNT min=100 max=100
#pragma HLS loop_flatten
#pragma HLS PIPELINE

#if 0
				//if(col_cnt_fo0 == 0)
				fprintf(stderr,"\n pl_loop:col_loop: %d:%d ...\t\t",(int)plane4_fo0  ,(int)col_cnt_fo0);
#endif

#if !__SYNTHESIS__
				if(col_cnt_fo0 == 5 && plane4_fo0==3)
					int hhgg=0;
#endif

				ap_uint<16> act_plane4_fo0 = plane4_fo0 + current_plane_fc0;
				ap_uint<16> current_row_id_op_fo0 = opix_row_fl0+iteration_fn0;
				ap_uint<16> current_col_id_op_fo0 = opix_col_fl0*(iteration_fn0==0);

				if(col_cnt_fo0 == 0)
				{
					lrn_row_offset = opix_row_wo_offset_fl0;
					lrn_row_offset_opix = opix_row_fl0;
					lrn_col_offset = opix_col_fl0;
				}
				else if((lrn_col_offset) > (input_desc.width-2))
				{
					lrn_row_offset++;
					lrn_row_offset_opix++;
					lrn_col_offset = 0;
				}
				else
				{
					lrn_col_offset++;
				}

				ap_uint<16> row_id_in_offset_fo0 = current_row_id_op_fo0*weight_desc.filter_stride;
				ap_uint<16> row_id_in_bram_offset_fo0 = (opix_row_wo_offset_fl0+iteration_fn0)*weight_desc.filter_stride;
				ap_uint<16> col_id_in_offset_fo0 = current_col_id_op_fo0*weight_desc.filter_stride;
				ap_uint<16> istg_content_fo0 = input_desc.width*conv_desc.istg_row_count;

				ap_uint<8> row_cnt_dilated_fo0 = row_cnt_fo0*weight_desc.dilation_factor;
				//ap_int<16> row_id_in_fo0 = pad_row_fc0 + row_cnt_dilated_fo0 + row_id_in_offset_fo0;
				ap_int<16> row_id_in_fo0 = -conv_desc.pad_num + row_cnt_dilated_fo0 + row_id_in_offset_fo0;
				ap_int<16> col_id_in0_fo0 = -conv_desc.pad_num + col_cnt_fo0 + col_id_in_offset_fo0;
				ap_int<16> row_id_in0_bram_fo0 = pad_row_wo_fc0 + row_cnt_dilated_fo0 + row_id_in_bram_offset_fo0;
				//ap_int<16> row_id_in_bram_fo0 = -conv_desc.pad_num + row_cnt_dilated_fo0 + row_id_in_bram_offset_fo0;

				ap_int<16> col_id_in_fo0;
				ap_int<16> row_id_in_bram_fo0;
				if(conv_desc.lrn_inter_sos_enable == 1)
				{
					col_id_in_fo0 = lrn_col_offset;
					row_id_in_bram_fo0 = lrn_row_offset;
				}
				else
				{
					col_id_in_fo0  = col_id_in0_fo0 ;
					row_id_in_bram_fo0 = row_id_in0_bram_fo0;
				}

				//************************* LRN INTER************************

				ap_uint<30> flag_1st_32pix_buff, flag_next_32pix_buff;
				/*
				for(ap_uint<5> pix_bit = 0; pix_bit< 26; pix_bit++)
				{
#pragma HLS unroll
					if(pix_bit < 6)
						flag_1st_32pix_buff[pix_bit] = pattern_8kp[0];
					else if(pix_bit > 5 && pix_bit < 12)
						flag_1st_32pix_buff[pix_bit] = pattern_8kp[1];
					else if(pix_bit > 11 && pix_bit < 18)
						flag_1st_32pix_buff[pix_bit] = pattern_8kp[2];
					else if(pix_bit > 17 && pix_bit < 24)
						flag_1st_32pix_buff[pix_bit] = pattern_8kp[3];
					else
						flag_1st_32pix_buff[pix_bit] = pattern_8kp[4];
				}
				 */


				for(ap_uint<8> pix_bit = 0, pattern_bit = 0; pix_bit < XI_PIX_PROC/2; pix_bit+=6 , pattern_bit++)
				{
#pragma HLS unroll
					for(ap_uint<10> pix_kp = 0; pix_kp < 6; pix_kp++)
					{
#pragma HLS unroll
						if(pix_kp == col_cnt_fo0)
							flag_1st_32pix_buff[pix_bit+pix_kp] = pattern_8kp[pattern_bit];
						else
							flag_1st_32pix_buff[pix_bit+pix_kp] = 0;
					}
				}

				for(ap_uint<10> pix_kp = 2; pix_kp < 6; pix_kp++)
				{
#pragma HLS unroll
					if(pix_kp == col_cnt_fo0)
						flag_next_32pix_buff[pix_kp-2] = pattern_8kp[4];
					else
						flag_next_32pix_buff[pix_kp-2] = 0;
				}

				for(ap_uint<10> pix_bit = 4, pattern_bit = 5; pix_bit < 22; pix_bit+=6 , pattern_bit++)
				{
#pragma HLS unroll
					for(ap_uint<10> pix_kp = 0; pix_kp < 6; pix_kp++)
					{
#pragma HLS unroll

						if(pix_kp == col_cnt_fo0)
							flag_next_32pix_buff[pix_bit+pix_kp] = pattern_8kp[pattern_bit];
						else
							flag_next_32pix_buff[pix_bit+pix_kp] = 0;

					}
				}

#if 0//!__SYNTHESIS__
				if(col_cnt_fo0==0){
					fprintf(stderr,"\n planeloop %d  pattern8bit:",(int)plane4_fo0);
					for(int bit = 7; bit>-1; bit--)
						fprintf(stderr,"%d",(int)pattern_8kp[bit]);
					fprintf(stderr,"\t flagpix0: %d  flagpix6: %d  flagpix12: %d ", (int)flag_1st_32pix_buff[0], (int)flag_1st_32pix_buff[6], (int)flag_1st_32pix_buff[12]);
				}
				//if(col_cnt_fo0 == 0)
				{
					for(int bit = 7; bit>-1; bit--)
						fprintf(stderr,"%d",(int)pattern_8kp[bit]);

					fprintf(stderr,"\t\t.....");

					for(int pix = 8; pix>3; pix--)
						fprintf(stderr,"%d",(int)flag_next_32pix_buff[pix]);

					fprintf(stderr,"\t\t.....");

					for(int pix = 3; pix>-1; pix--)
						fprintf(stderr,"%d",(int)flag_next_32pix_buff[pix]);

					fprintf(stderr,"\t\t.....");

					for(int pix = 25; pix>23; pix--)
						fprintf(stderr,"%d",(int)flag_1st_32pix_buff[pix]);

					fprintf(stderr,"\t\t.....");

					for(int pix = 11; pix>5; pix--)
						fprintf(stderr,"%d",(int)flag_1st_32pix_buff[pix]);

					fprintf(stderr,"\t\t.....");

					for(int pix = 5; pix>-1; pix--)
						fprintf(stderr,"%d",(int)flag_1st_32pix_buff[pix]);
				}
				if(plane4_fo0 == 7 && col_cnt_fo0 == 2 )
					int ggg=0;
#endif
				/*
				for(ap_uint<5> pix_bit = 0; pix_bit< 26; pix_bit++)
				{
#pragma HLS unroll
					if(pix_bit < 4)
						flag_next_32pix_buff[pix_bit] = pattern_8kp[4];
					else if(pix_bit > 3 && pix_bit < 10)
						flag_next_32pix_buff[pix_bit] = pattern_8kp[5];
					else if(pix_bit > 9 && pix_bit < 16)
						flag_next_32pix_buff[pix_bit] = pattern_8kp[6];
					else if(pix_bit > 15 && pix_bit < 22)
						flag_next_32pix_buff[pix_bit] = pattern_8kp[7];
					else
						flag_next_32pix_buff[pix_bit] = 0;
				}
				 */
				bool lrn_add_bit;
				if(lrn_counter > -1 && lrn_counter < weight_desc.filter_size)
					lrn_add_bit = 1;
				else
					lrn_add_bit = 0;

				if(col_cnt_fo0 == 5)
					pattern_8kp = pattern_8kp*2 + (ap_uint<8>)lrn_add_bit;

				if(col_cnt_fo0 == 5)
					lrn_counter++;

				ap_int<16> plane_id_lrn = plane4_fo0 + current_plane_fc0*4 - 4;
				ap_uint<2> istg_feedingBuff_dim1_lrn = plane_id_lrn.range(1,0);
				ap_uint<10> feedingBuff_dim3_1st_32pix = plane4_fo0.range(11,2);
				ap_uint<10> feedingBuff_dim3_next_32pix = plane4_fo0.range(11,2) + 512;
				//********************************************

				ap_uint<3> istg_dim2_fo0;
				ap_uint<16> istg_dim3_fo0;
				if(conv_desc.lrn_inter_sos_enable == 1)
				{
					istg_dim2_fo0 = plane_id_lrn.range(4,2);//%8;
					istg_dim3_fo0 = (col_id_in_fo0 + row_id_in_bram_fo0*input_desc.width) + (plane_id_lrn.range(15,5))*istg_content_fo0;
				}
				else if(conv_desc.layer_id == 0)
				{
#if XI_ROW8// 8 rows in istg BRAM
					istg_dim2_fo0 = row_id_in_bram_fo0.range(5,3);///8;
					istg_dim3_fo0 = (input_desc.width * (row_id_in_bram_fo0.range(2,0))) + col_id_in_fo0;
#elif XI_ROW4// 4 rows in istg BRAM
					istg_dim2_fo0 = row_id_in_bram_fo0.range(4,2);///4;
					istg_dim3_fo0 = (input_desc.width * (row_id_in_bram_fo0.range(1,0))) + col_id_in_fo0;
#elif XI_ROW2// 2 rows in istg BRAM
					istg_dim2_fo0 = row_id_in_bram_fo0.range(3,1);///2;
					istg_dim3_fo0 = (input_desc.width * (row_id_in_bram_fo0[0])) + col_id_in_fo0;
#endif
				}
				else
				{
					istg_dim2_fo0 = act_plane4_fo0.range(2,0);//%8;
					istg_dim3_fo0 = (col_id_in_fo0 + row_id_in_bram_fo0*input_desc.width) + (act_plane4_fo0.range(15,3))*istg_content_fo0;
				}

				short istg_value_fo0[4];
#pragma HLS ARRAY_PARTITION variable=istg_value_fo0 complete dim=0
				short istg_value_lrn_inter;

#if 0
				if(conv_desc.lrn_inter_sos_enable == 1)
				{
					if(lrn_counter > 0)
						istg_value_lrn_inter = istaging_buff0_fb0[istg_feedingBuff_dim1_lrn][istg_dim2_fo0][istg_dim3_fo0];
					else
						istg_value_lrn_inter = 0;
				}
				else// reading istage buffer for conv operation
				{
					Plane_fo0_Loop:
					for(ap_uint<3> plane_fo0=0; plane_fo0<4; plane_fo0++)
					{
#pragma HLS UNROLL
						if(row_id_in_fo0 < 0 || col_id_in_fo0 < 0 || row_id_in_fo0 >= input_desc.height || col_id_in_fo0 >= input_desc.width)
							istg_value_fo0[plane_fo0] = 0;
						else
							istg_value_fo0[plane_fo0] = istaging_buff0_fb0[plane_fo0][istg_dim2_fo0][istg_dim3_fo0];
					}//Plane_fo0_Loop
				}
#else
				Plane_fo0_Loop:
				for(ap_uint<3> plane_fo0=0; plane_fo0<4; plane_fo0++)
				{
#pragma HLS UNROLL
					if(conv_desc.lrn_inter_sos_enable == 1)
					{
						if( lrn_counter > 0 && plane_id_lrn > -1 && plane_id_lrn < input_desc.planes)
							istg_value_fo0[plane_fo0] = istaging_buff0_fb0[plane_fo0][istg_dim2_fo0][istg_dim3_fo0];
						else
							istg_value_fo0[plane_fo0] = 0;
					}
					else
					{
						if(row_id_in_fo0 < 0 || col_id_in_fo0 < 0 || row_id_in_fo0 >= input_desc.height || col_id_in_fo0 >= input_desc.width)
							istg_value_fo0[plane_fo0] = 0;
						else
							istg_value_fo0[plane_fo0] = istaging_buff0_fb0[plane_fo0][istg_dim2_fo0][istg_dim3_fo0];
					}
				}//Plane_fo0_Loop
				istg_value_lrn_inter = istg_value_fo0[istg_feedingBuff_dim1_lrn];

#if 0//!__SYNTHESIS__
				//if(col_cnt_fo0 == 0)
				fprintf(stderr," lrn_cnt:istg_value[%d][%d][%d]  : %d:%d ...\t\t",(int)istg_feedingBuff_dim1_lrn, (int)istg_dim2_fo0, (int)istg_dim3_fo0, (int)lrn_counter  ,(int)istg_value_lrn_inter);

				if(istg_value_lrn_inter == 76)
					int stop=0;
#endif

				short istg_value2_fo0[4];
#pragma HLS ARRAY_PARTITION variable=istg_value2_fo0 complete dim=0
				for(ap_uint<3> iter =0;iter<4;iter++)
				{
#pragma HLS UNROLL
					if(mac_fz0 != iter && conv_desc.intra_en)
						istg_value2_fo0[iter] =  0;
					else
						istg_value2_fo0[iter] = istg_value_fo0[iter];
				}
#endif


				ap_uint<16> flag_pattern_fo0 = (fsz_fst_div_fo0*weight_desc.filter_stride);

				bool add_shift_fo0[XI_PIX_PROC/2];
#pragma HLS ARRAY_PARTITION variable=add_shift_fo0 complete dim=0
				Pix_fo1_Loop:
				for(ap_uint<8> pix_fo1  =0; pix_fo1<XI_PIX_PROC/2; pix_fo1++)
				{
#pragma HLS unroll
					if(col_cnt_fo0 < flag_pattern_fo0)
						add_shift_fo0[pix_fo1] = initial_flag_reg_fo0[pix_fo1];
					else
						add_shift_fo0[pix_fo1] = 0;
				}//Pix_fo1_Loop

				bool flag_reg_shift_left_fo0[XI_PIX_PROC/2];
#pragma HLS ARRAY_PARTITION variable=flag_reg_shift_left_fo0 complete dim=0
				Pix_fo2_Loop:
				for(ap_uint<8> pix_fo2  =0; pix_fo2<XI_PIX_PROC/2; pix_fo2++)
				{
#pragma HLS unroll
					if(pix_fo2==0)
						flag_reg_shift_left_fo0[pix_fo2] = 0;
					else
						flag_reg_shift_left_fo0[pix_fo2] = flag_reg_fo0[pix_fo2-1];
				}//Pix_fo2_Loop


				//if(row_cnt_fo0==0 && plane4_fo0)
				//printf("\n************<row:col: %d:%d>", (int)row_cnt_fo0, (int)col_cnt_fo0);

				Pix_fo3_Loop:
				for(ap_uint<8> pix_fo3  =0; pix_fo3<XI_PIX_PROC/2; pix_fo3++)
				{
#pragma HLS unroll
					bool internel_reg1 = flag_reg_shift_left_fo0[pix_fo3];// | add_shift_fo0[pix_fo3];
					bool internel_reg2 = flag_reg_fo0[pix_fo3] & (fsz_fst_mod_fo0!=0);

					if(stride_counter_fo0 == 0)
						flag_reg_fo0[pix_fo3] = ((internel_reg1 | internel_reg2)&(col_cnt_fo0!=0)) | add_shift_fo0[pix_fo3];
					else if(stride_counter_fo0 == (weight_desc.filter_stride-1))
						flag_reg_fo0[pix_fo3] = ((internel_reg1 & internel_reg2)&(col_cnt_fo0!=0)) | add_shift_fo0[pix_fo3];

					//if(row_cnt_fo0==0 && plane4_fo0)
					//	printf("\t%d",(int)flag_reg_fo0[pix_fo3]);
				}//Pix_fo3_Loop

				if((stride_counter_fo0<(weight_desc.filter_stride-1)) && (col_cnt_fo0 < (col_max_fo0-1)))
					stride_counter_fo0++;
				else
					stride_counter_fo0=0;

#if 0
				if(conv_desc.lrn_inter_sos_enable == 1)
				{
					Pix_fo4_Loop:
					for(ap_uint<8> pix_fo4  =0; pix_fo4<XI_PIX_PROC/2; pix_fo4++)
					{
#pragma HLS unroll
						if(flag_1st_32pix_buff[pix_fo4])
							input_buff0_fc0[istg_feedingBuff_dim1_lrn][pix_fo4][feedingBuff_dim3_1st_32pix] = istg_value_lrn_inter;
						else
							input_buff0_fc0[istg_feedingBuff_dim1_lrn][pix_fo4][feedingBuff_dim3_1st_32pix] = 0;

						if(flag_next_32pix_buff[pix_fo4])
							input_buff0_fc0[istg_feedingBuff_dim1_lrn][pix_fo4][feedingBuff_dim3_next_32pix] = istg_value_lrn_inter;
						else
							input_buff0_fc0[istg_feedingBuff_dim1_lrn][pix_fo4][feedingBuff_dim3_next_32pix] = 0;
					}
				}
				else // writting feeding buffer for conv operation
				{
					Pix_fo4_Loop2:
					for(ap_uint<8> pix_fo4  =0; pix_fo4<XI_PIX_PROC/2; pix_fo4++)
					{
#pragma HLS unroll
						Plane_fo1_Loop2:
						for(ap_uint<4> plane_fo1=0; plane_fo1<4; plane_fo1++)
						{
#pragma HLS unroll
							short feeding_buff_data;
							if(mac_fz0 != plane_fo1  && (conv_desc.bn_intra_mean_enable==1 || conv_desc.bn_intra_sos_enable==1 || conv_desc.lrn_intra_sos_enable==1))
								feeding_buff_data = 0;
							else
								feeding_buff_data = istg_value_fo0[plane_fo1];

							if(flag_reg_fo0[pix_fo4])
								input_buff0_fc0[plane_fo1][pix_fo4][addr_bram_fo0[pix_fo4]] = feeding_buff_data;

							short t = input_buff0_fc0[plane_fo1][pix_fo4][addr_bram_fo0[pix_fo4]];
							int j=0;
						}//Plane_fo1_Loop
					}//Pix_fo4_Loop
				}
#else
				Pix_fo4_Loop2:
				for(ap_uint<8> pix_fo4  =0; pix_fo4<XI_PIX_PROC/2; pix_fo4++)
				{
#pragma HLS unroll
					Plane_fo1_Loop2:
					for(ap_uint<4> plane_fo1=0; plane_fo1<4; plane_fo1++)
					{
#pragma HLS unroll
						short feeding_buff_data = istg_value2_fo0[plane_fo1];


						if(conv_desc.lrn_inter_sos_enable == 1)
						{
							if(plane_fo1 == istg_feedingBuff_dim1_lrn){
								if(0)//flag_1st_32pix_buff[pix_fo4] == 0)
									input_buff0_fc0[plane_fo1][pix_fo4][feedingBuff_dim3_1st_32pix] = 0;
								else if(flag_1st_32pix_buff[pix_fo4] == 1)
									input_buff0_fc0[plane_fo1][pix_fo4][feedingBuff_dim3_1st_32pix] = istg_value_lrn_inter;

								if(0)//flag_next_32pix_buff[pix_fo4] == 0)
									input_buff0_fc0[plane_fo1][pix_fo4][feedingBuff_dim3_next_32pix] = 0;
								else if(flag_next_32pix_buff[pix_fo4] == 1)
									input_buff0_fc0[plane_fo1][pix_fo4][feedingBuff_dim3_next_32pix] = istg_value_lrn_inter;
							}

						}
						else
						{
							if(flag_reg_fo0[pix_fo4])
								input_buff0_fc0[plane_fo1][pix_fo4][addr_bram_fo0[pix_fo4]] = feeding_buff_data;
						}



					}//Plane_fo1_Loop
				}//Pix_fo4_Loop

#endif
				if(col_cnt_fo0 == col_max_fo0-1)
				{
					flag_1st_32pix_buff_previous_iteration=flag_1st_32pix_buff;
					flag_next_32pix_buff_previous_iteration=flag_next_32pix_buff;
				}
				else
				{
					flag_1st_32pix_buff_previous_iteration =0;
					flag_next_32pix_buff_previous_iteration=0;
				}
#if !__SYNTHESIS__
				if(flag_1st_32pix_buff[23] ==1)
					int ghhfd =0;

				if((input_buff0_fc0[3][23][0]) > 10000)
					int hhgg=0;
#endif

#if 0//!__SYNTHESIS__

				if(input_buff0_fc0[0][0][1] == 76)
					int hhh=0;

				int hhhhh=0;

				if(input_buff0_fc0[0][0][1] == 0)
					int hhhk=0;

				for(int addr=0; addr < 6; addr++)
				{
					for(int p=0; p<4;p++)
					{
						fprintf(stderr,"inp_buff[%d][0][%d] = %d  ",(int)p, (int)addr, (int)input_buff0_fc0[p][0][addr] );

					}
				}
#endif


#if 0
				if(conv_desc.lrn_inter_sos_enable == 1)
				{
					for(ap_uint<8> pix_fo5  =0; pix_fo5<XI_PIX_PROC/2; pix_fo5++)
					{
#pragma HLS unroll
						if(flag_1st_32pix_buff[pix_fo5])
						{
							//	addr_bram_fo0[pix_fo5] ++;
							outpixels1_fl0.pix_rows[pix_fo5] = current_row_id_op_fo0;
							outpixels1_fl0.pix_cols[pix_fo5] = current_col_id_op_fo0;
						}

						if(flag_next_32pix_buff[pix_fo5])
						{
							//	addr_bram_fo0[pix_fo5] ++;
							outpixels2_fl0.pix_rows[pix_fo5] = current_row_id_op_fo0;
							outpixels2_fl0.pix_cols[pix_fo5] = current_col_id_op_fo0;
						}
					}
				}
				else
				{
					Pix_fo5_Loop:
					for(ap_uint<8> pix_fo5  =0; pix_fo5<XI_PIX_PROC/2; pix_fo5++)
					{
#pragma HLS unroll
						if(flag_reg_fo0[pix_fo5])
						{
							//	addr_bram_fo0[pix_fo5] ++;
							outpixels1_fl0.pix_rows[pix_fo5] = current_row_id_op_fo0;
							outpixels1_fl0.pix_cols[pix_fo5] = current_col_id_op_fo0 + (pix_fo5-pxcnt_fn0);
						}

					}//pix_fo5_Loop
				}
#else
				Pix_fo5_Loop:
				for(ap_uint<8> pix_fo5  =0; pix_fo5<XI_PIX_PROC/2; pix_fo5++)
				{
#pragma HLS unroll
					if(conv_desc.lrn_inter_sos_enable == 1)
					{
						if(col_cnt_fo0 == pix_fo5)
						{
							//	addr_bram_fo0[pix_fo5] ++;
							outpixels1_fl0.pix_rows[pix_fo5] = lrn_row_offset_opix;
							outpixels1_fl0.pix_cols[pix_fo5] = lrn_col_offset;
						}
					}
					else
					{
						if(flag_reg_fo0[pix_fo5])
						{
							//	addr_bram_fo0[pix_fo5] ++;
							outpixels1_fl0.pix_rows[pix_fo5] = current_row_id_op_fo0;
							outpixels1_fl0.pix_cols[pix_fo5] = current_col_id_op_fo0 + (pix_fo5-pxcnt_fn0);
						}
					}

				}//pix_fo5_Loop
				/*
				Pix_fo51_Loop:
				for(ap_uint<8> pix_fo5  =0; pix_fo5<XI_PIX_PROC/2; pix_fo5++)
				{
#pragma HLS unroll
					if(conv_desc.lrn_inter_sos_enable == 1)
					{
						if(flag_next_32pix_buff[pix_fo5])
						{
							//	addr_bram_fo0[pix_fo5] ++;
							outpixels2_fl0.pix_rows[pix_fo5] = current_row_id_op_fo0;
							outpixels2_fl0.pix_cols[pix_fo5] = current_col_id_op_fo0;
						}
					}
				}//pix_fo5_Loop
				 */
#endif

#if 0//!__SYNTHESIS__
				for(int i=0; i<26; i++)
				{
					if (i==0)
						fprintf(stderr, "\n<rowid %d>< colid %d>   %d:%d ", (int)lrn_row_offset, (int)lrn_col_offset, (int)outpixels1_fl0.pix_rows[i], (int)outpixels1_fl0.pix_cols[i]);
					else
						fprintf(stderr, "%d:%d ", (int)outpixels1_fl0.pix_rows[i], (int)outpixels1_fl0.pix_cols[i]);
				}
#endif

				Pix_fo6_Loop:
				for(ap_uint<8> pix_fo6  =0; pix_fo6<XI_PIX_PROC/2; pix_fo6++)
				{
#pragma HLS unroll
					if((flag_reg_fo0[pix_fo6]==1) && (dilated_counter_fo0[pix_fo6]==0))
						addr_bram_fo0[pix_fo6] ++;
				}//pix_fo6_Loop

				Pix_fo7_Loop:
				for(ap_uint<8> pix_fo7  =0; pix_fo7<XI_PIX_PROC/2; pix_fo7++)
				{
#pragma HLS unroll
					if((flag_reg_fo0[pix_fo7]==1) && (dilated_counter_fo0[pix_fo7] != (weight_desc.dilation_factor-1)) && col_cnt_fo0 != (col_max_fo0-1))
						dilated_counter_fo0[pix_fo7]++;
					else
						dilated_counter_fo0[pix_fo7]=0;
				}//pix_fo7_Loop

			}//Row_Loop
#if 0//!__SYNTHESIS__
			for(int pix=0; pix<26; pix++)
			{
				for(int loc=0; loc<4; loc++)
				{
					fprintf(stderr,"\npix%d-----",pix);
					for(int p=0; p<4; p++)
					{
						fprintf(stderr,"\t%d",(int)input_buff0_fc0[p][pix][loc]);
					}
				}
			}

			for(int pix=0; pix<26; pix++)
			{
				for(int loc=0; loc<4; loc++)
				{
					fprintf(stderr,"\npix%d-----",pix+26);
					for(int p=0; p<4; p++)
					{
						fprintf(stderr,"\t%d",(int)input_buff0_fc0[p][pix][loc+512]);
					}
				}
			}
#endif
		}//Col_Loop
	}//Plane_Loop

}


template<int CFILTER_SIZE, int CCONV_STRIDE, int CNUM_KERNELS, int INPUTP>
void RowPixFnloop(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		out_pix2_struct &outpixels1_fl0,
		out_pix2_struct &outpixels2_fl0,
		ap_uint<10>  iteration_cnt_fn0,
		ap_uint<7> pix_count_num,
		ap_uint<16> opix_row_fl0,
		ap_uint<16> opix_col_fl0,
		ap_uint<10>last_row_entries_fn0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024],
		ap_uint<16> opix_row_wo_offset_fl0,
		ap_uint<10> current_plane_fc0,
		ap_int<12> pad_row_fc0,
		ap_int<12> pad_row_wo_fc0,
		ap_uint<2> mac_fz0,
		ap_uint<10> inputBuff_bram_offset_fl0,
		ap_uint<7> pxcnt_fn0
)
{
#pragma HLS INLINE OFF
	Iteration_Loop:
	for(ap_uint<10> iteration_fn0=0; iteration_fn0 <  iteration_cnt_fn0; iteration_fn0++){
#pragma HLS LOOP_TRIPCOUNT min=1 max=1

		ap_uint<7> pixel_tobe_process_fn0;
		if(conv_desc.lrn_inter_sos_enable == 1)
		{
			pixel_tobe_process_fn0 = conv_desc.pix_per_kp;
		}
		else
		{
			if(iteration_fn0==0){
				if((pix_count_num)+opix_col_fl0 >= output_desc.width)
					pixel_tobe_process_fn0 = (output_desc.width) - (opix_col_fl0 + 1);
				else
					pixel_tobe_process_fn0 = (pix_count_num) - 1;
			}else if(iteration_fn0==iteration_cnt_fn0-1){
				pixel_tobe_process_fn0 = last_row_entries_fn0 - 1;
			}else{
				pixel_tobe_process_fn0 = (output_desc.width) - 1;
			}
		}

		RowPixFunction_fo<CFILTER_SIZE, CCONV_STRIDE, CNUM_KERNELS, INPUTP>
		( input_desc, conv_desc, output_desc, weight_desc, outpixels1_fl0, outpixels2_fl0, istaging_buff0_fb0, input_buff0_fc0, opix_row_fl0, opix_col_fl0,
				/*..continue..*/opix_row_wo_offset_fl0, current_plane_fc0, inputBuff_bram_offset_fl0, pad_row_fc0,pad_row_wo_fc0, pxcnt_fn0,
				/*..continue..*/pixel_tobe_process_fn0, iteration_fn0, mac_fz0);

		pxcnt_fn0+=(pixel_tobe_process_fn0+1);
	}//Iteration_Loop
}
template<int CFILTER_SIZE,int CCONV_STRIDE,int CNUM_KERNELS,int INPUTP>
void LoadInputBuff32Pix_fn(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		out_pix2_struct &outpixels1_fl0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024],
		ap_uint<16> opix_row_fl0,
		ap_uint<16> opix_col_fl0,
		ap_uint<16> opix_row_wo_offset_fl0,
		ap_uint<10> current_plane_fc0,
		ap_int<12> pad_row_fc0,
		ap_int<12> pad_row_wo_fc0,
		ap_uint<2> mac_fz0,
		ap_uint<7> start_pix_count_fn0,
		ap_uint<7> pix_count_num,
		ap_uint<10> inputBuff_bram_offset_fl0

)
{
#pragma HLS inline off

	ap_uint<7> pxcnt_fn0;
	if(start_pix_count_fn0 < XI_PIX_PROC/2)
	{
		pxcnt_fn0 = start_pix_count_fn0;//pix_count_num;
	}
	else
	{
		pxcnt_fn0 = start_pix_count_fn0-XI_PIX_PROC/2;
	}

	ap_uint<16> next_col_id_1st32pix_fn0 = opix_col_fl0+pix_count_num-(conv_desc.pix2_div_outwidth * output_desc.width);
	ap_uint<16> row_cnt_fn0 = conv_desc.pix2_div_outwidth;
	ap_uint<16> col_cnt_fn0;
	if(next_col_id_1st32pix_fn0==0){
		col_cnt_fn0 = output_desc.width;
	}else if(next_col_id_1st32pix_fn0<=output_desc.width){
		row_cnt_fn0+=1;
		col_cnt_fn0 = next_col_id_1st32pix_fn0;
	}else{
		row_cnt_fn0+=2;
		col_cnt_fn0 = next_col_id_1st32pix_fn0 - output_desc.width;
	}

	ap_uint<10>  iteration_cnt_fn0;
	if(pix_count_num == 0)
		iteration_cnt_fn0 = 0;
	else if(conv_desc.lrn_inter_sos_enable == 1)
		iteration_cnt_fn0 = 1;
	else
		iteration_cnt_fn0 = row_cnt_fn0;

	ap_uint<10>last_row_entries_fn0 = col_cnt_fn0;

	//	ap_uint<7> pxcnt_fn0=0;
#if 1
	Iteration_Loop:
	for(ap_uint<10> iteration_fn0=0; iteration_fn0 <  iteration_cnt_fn0; iteration_fn0++){
#pragma HLS LOOP_TRIPCOUNT min=1 max=1

		ap_uint<7> pixel_tobe_process_fn0;
		if(conv_desc.lrn_inter_sos_enable == 1)
		{
			pixel_tobe_process_fn0 = conv_desc.pix_per_kp;
		}
		else
		{
			if(iteration_fn0==0){
				if((pix_count_num)+opix_col_fl0 >= output_desc.width)
					pixel_tobe_process_fn0 = (output_desc.width) - (opix_col_fl0 + 1);
				else
					pixel_tobe_process_fn0 = (pix_count_num) - 1;
			}else if(iteration_fn0==iteration_cnt_fn0-1){
				pixel_tobe_process_fn0 = last_row_entries_fn0 - 1;
			}else{
				pixel_tobe_process_fn0 = (output_desc.width) - 1;
			}
		}

		RowPixFunction_fo<CFILTER_SIZE, CCONV_STRIDE, CNUM_KERNELS, INPUTP>
		( input_desc, conv_desc, output_desc, weight_desc, outpixels1_fl0, istaging_buff0_fb0, input_buff0_fc0, opix_row_fl0, opix_col_fl0,
				/*..continue..*/opix_row_wo_offset_fl0, current_plane_fc0, inputBuff_bram_offset_fl0, pad_row_fc0,pad_row_wo_fc0, pxcnt_fn0,
				/*..continue..*/pixel_tobe_process_fn0, iteration_fn0, mac_fz0);

		pxcnt_fn0+=(pixel_tobe_process_fn0+1);
	}//Iteration_Loop
#else

	RowPixFnloop<CFILTER_SIZE, CCONV_STRIDE, CNUM_KERNELS, INPUTP>
	(input_desc,
			conv_desc,
			output_desc,
			weight_desc,
			outpixels1_fl0,
			outpixels2_fl0,
			iteration_cnt_fn0,
			pix_count_num,
			opix_row_fl0,
			opix_col_fl0,
			last_row_entries_fn0,
			istaging_buff0_fb0,
			input_buff0_fc0,
			opix_row_wo_offset_fl0,
			current_plane_fc0,
			pad_row_fc0,
			pad_row_wo_fc0,
			mac_fz0,
			inputBuff_bram_offset_fl0,
			pxcnt_fn0
	);

#endif

}


template<int CFILTER_SIZE,int CCONV_STRIDE,int CNUM_KERNELS,int INPUTP>
void LoadInputBuff_non1x1_fn(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		out_pix2_struct &outpixels1_fl0,
		out_pix2_struct &outpixels2_fl0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024],
		ap_uint<16> pix_row_1st_fl0,
		ap_uint<16> pix_col_1st_fl0,
		ap_uint<16> pix_row_1st_wo_offset_fl0,
		ap_uint<16> pix_row_2nd_fl0,
		ap_uint<16> pix_col_2nd_fl0,
		ap_uint<16> pix_row_2nd_wo_offset_fl0,
		ap_uint<10> current_plane_fc0,
		ap_int<12> pad_row_fc0,
		ap_int<12> pad_row_wo_fc0,
		ap_uint<2> mac_fz0,
		ap_uint<7> start_pix_count_fn0
)
{
#pragma HLS inline off

	ap_uint<7> pix_count_1st_half;
	ap_uint<7> pix_count_2nd_half;
	ap_uint<7> start_pix_count_1st_half_fn0;
	ap_uint<7> start_pix_count_2nd_half_fn0;

	if(conv_desc.lrn_inter_sos_enable == 1)
	{
		pix_count_1st_half = XI_PIX_PROC;
		pix_count_2nd_half = 0;

		start_pix_count_1st_half_fn0 = 0;
		start_pix_count_2nd_half_fn0 = 0;
	}
	else if(conv_desc.lrn_intra_sos_enable == 1)
	{
		if((start_pix_count_fn0+6) < XI_PIX_PROC/2){
			pix_count_1st_half = 6;
			pix_count_2nd_half = 0;

			start_pix_count_1st_half_fn0 = start_pix_count_fn0;
			start_pix_count_2nd_half_fn0 = 0;
		}
		else if( (start_pix_count_fn0) > XI_PIX_PROC/2 && (start_pix_count_fn0+6) > XI_PIX_PROC/2){
			pix_count_1st_half = 0;
			pix_count_2nd_half = 6;

			start_pix_count_1st_half_fn0 = 0;
			start_pix_count_2nd_half_fn0 = start_pix_count_fn0;
		}
		else
		{
			pix_count_1st_half = XI_PIX_PROC/2 - start_pix_count_fn0;
			pix_count_2nd_half = 6 - (XI_PIX_PROC/2 - start_pix_count_fn0);

			start_pix_count_1st_half_fn0 = start_pix_count_fn0;
			start_pix_count_2nd_half_fn0 = XI_PIX_PROC/2;
		}
	}
	else // conv operation
	{
		pix_count_1st_half = XI_PIX_PROC/2;
		pix_count_2nd_half = XI_PIX_PROC/2;

		start_pix_count_1st_half_fn0 = 0;
		start_pix_count_2nd_half_fn0 = XI_PIX_PROC/2;
	}

#if 0//!__SYNTHESIS__
	for(int i=0; i< 1024; i++)
	{
		for(int j=0; j< XI_PIX_PROC/2; j++)
		{
			for(int k=0; k< 4; k++)
			{
				input_buff0_fc0[k][j][i] = 9999;
			}
		}
	}
#endif


	for(ap_uint<10> addr=0, addr1=512 ; addr<4; addr++, addr1++)
	{
#pragma HLS PIPELINE
		for(ap_uint<7> pix_buf=0; pix_buf < XI_PIX_PROC/2; pix_buf++)
		{
#pragma HLS unroll
			for(ap_uint<4> pl=0; pl<4; pl++)
			{
#pragma HLS unroll

				input_buff0_fc0[pl][pix_buf][addr] = 0;
				input_buff0_fc0[pl][pix_buf][addr1] = 0;

			}
		}
	}

	LoadInputBuff32Pix_fn<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,INPUTP>
	(input_desc, conv_desc, output_desc,weight_desc, outpixels1_fl0,istaging_buff0_fb0, input_buff0_fc0, pix_row_1st_fl0,pix_col_1st_fl0,
			/*..continue..*/pix_row_1st_wo_offset_fl0,current_plane_fc0, pad_row_fc0, pad_row_wo_fc0,mac_fz0,start_pix_count_1st_half_fn0,pix_count_1st_half, 0);

#if 0
	for(int pix=0; pix<26; pix++)
	{
		for(int loc=0; loc<4; loc++)
		{
			fprintf(stderr,"\npix%d-----",pix);
			for(int p=0; p<4; p++)
			{
				fprintf(stderr,"\t%d",(int)input_buff0_fc0[p][pix][loc]);
			}
		}
	}

	for(int pix=0; pix<26; pix++)
	{
		for(int loc=0; loc<4; loc++)
		{
			fprintf(stderr,"\npix%d-----",pix+26);
			for(int p=0; p<4; p++)
			{
				fprintf(stderr,"\t%d",(int)input_buff0_fc0[p][pix][loc+512]);
			}
		}
	}
#endif

	LoadInputBuff32Pix_fn<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,INPUTP>
	(input_desc, conv_desc, output_desc,weight_desc, outpixels2_fl0,istaging_buff0_fb0, input_buff0_fc0, pix_row_2nd_fl0,pix_col_2nd_fl0,
			/*..continue..*/pix_row_2nd_wo_offset_fl0,current_plane_fc0, pad_row_fc0, pad_row_wo_fc0,mac_fz0,start_pix_count_2nd_half_fn0,pix_count_2nd_half, 512);

	int h=0;
}

template<int CFILTER_SIZE,int CCONV_STRIDE,int CNUM_KERNELS,int INPUTP>
void LoadInputBuff1x1_fm(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024],
		ap_uint<16> pix_rw_1st_fl0,
		ap_uint<16> pix_cl_1st_fl0,
		ap_uint<16> pix_rw_1st_wo_offset_fl0,
		ap_uint<16> pix_rw_2nd_wo_offset_fl0,
		ap_uint<10> current_plane_fc0,
		out_pix2_struct &outpixels1_fl0,
		out_pix2_struct &outpixels2_fl0,
		ap_uint<2> mac_fz0,
		ap_uint<8> start_pix_cnt,
		bool plane_select[4][2],
		ap_uint<10> plane_start)
{
#pragma HLS ARRAY_PARTITION variable=plane_select complete dim=0
#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=2
#pragma HLS resource variable=istaging_buff0_fb0 latency=2 core=RAM_T2P_BRAM

#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=2
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM

#pragma HLS ARRAY_PARTITION variable=outpixels2_fl0.pix_cols complete dim=1
#pragma HLS ARRAY_PARTITION variable=outpixels2_fl0.pix_rows complete dim=1

#pragma HLS ARRAY_PARTITION variable=outpixels2_fl0.pix_cols complete dim=1
#pragma HLS ARRAY_PARTITION variable=outpixels2_fl0.pix_rows complete dim=1
#pragma HLS inline off

	ap_uint<12> outpix_row_fm0 = pix_rw_1st_fl0;//_wo_offset1;
	ap_uint<12> outpix_row_wo_offset_fm0 = pix_rw_1st_wo_offset_fl0;
	ap_uint<12> outpix_col_fm0 = pix_cl_1st_fl0;

	ap_uint<12> outwidth_off_fm0;
#pragma HLS RESOURCE variable=outwidth_off_fm0 core=MulnS latency=2
	outwidth_off_fm0 = (outpix_row_fm0*output_desc.width);
	ap_uint<12> outwidth_wo_off_fm0;
#pragma HLS RESOURCE variable=outwidth_wo_off_fm0 core=MulnS latency=2
	outwidth_wo_off_fm0 = (outpix_row_wo_offset_fm0*output_desc.width);
	ap_uint<16> stgrow_wd_fm0;
#pragma HLS RESOURCE variable=stgrow_wd_fm0 core=MulnS latency=2
	stgrow_wd_fm0 = conv_desc.istg_row_count*input_desc.width;

	Plane_1x1_Loop:
	for(ap_uint<10> plane_loop_fm0=0; plane_loop_fm0 < conv_desc.loop_bound_1x1_filter; plane_loop_fm0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=16 max=16
		Pix_1x1_Loop:
		for(ap_uint<7>  pix_fm0=0;pix_fm0<(conv_desc.pix_per_kp/2);pix_fm0++)
		{
#pragma HLS loop_flatten
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=input_buff0_fc0 intra false
#pragma HLS DEPENDENCE variable=input_buff0_fc0 inter false
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 intra false
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 inter false
			ap_uint<7>  pix_cnt_fm0= 2*(pix_fm0) + start_pix_cnt;
			///WIthout Divns
			/*
			{
				int row_off = pix_rw_1st_fl0*1;
				int col_off = pix_cl_1st_fl0*1;
				int row_off_bram = pix_rw_1st_wo_offset_fl0*1;
				int row_id_in = 0 + row_off;
				int col_id_in = 0 + pix_cnt_fm0 + col_off;;
				int row_id_in_bram = 0 + row_off_bram;
			}
			 */

			ap_uint<10> plane_loop0_initial_fm0;
			ap_uint<10> plane_loop1_initial_fm0;
			plane_loop0_initial_fm0 = plane_loop_fm0*2 + 0,
					plane_loop1_initial_fm0 = plane_loop_fm0*2 + 1;

			ap_uint<16> plane_loop_offset;
			plane_loop_offset = current_plane_fc0 + plane_start;

			ap_uint<16> plane_loop0_fm0 = plane_loop0_initial_fm0+plane_loop_offset;
			ap_uint<16> plane_loop1_fm0 = plane_loop1_initial_fm0+plane_loop_offset;

			ap_uint<16> outpix_index_0_fm0 = outwidth_off_fm0 + outpix_col_fm0 + pix_cnt_fm0;
			ap_uint<16> outpix_index_1_fm0 = outwidth_off_fm0 + outpix_col_fm0 + pix_cnt_fm0 + 1;
	//		ap_uint<16> outpix_index_1_fm0 = outwidth_off_fm0 + outpix_col_fm0 + (pix_cnt_fm0*weight_desc.filter_stride) + weight_desc.filter_stride;

			ap_uint<16> outpix_index_0_wo_offset_fm0 = outwidth_wo_off_fm0 + outpix_col_fm0 + pix_cnt_fm0;
			ap_uint<16> outpix_index_1_wo_offset_fm0 = outwidth_wo_off_fm0 + outpix_col_fm0 + pix_cnt_fm0 + 1;
	//		ap_uint<16> outpix_index_1_wo_offset_fm0 = outwidth_wo_off_fm0 + outpix_col_fm0 + (pix_cnt_fm0*weight_desc.filter_stride) + weight_desc.filter_stride;

			ap_uint<12> outpix_row_0,outpix_row_1;
#pragma HLS resource variable=outpix_row_0 core=DivnS
#pragma HLS resource variable=outpix_row_1 core=DivnS
			outpix_row_0 = outpix_index_0_fm0/output_desc.width ;
			outpix_row_1 = outpix_index_1_fm0/output_desc.width ;

			ap_uint<12> outpix_row_0_wo_offset_fm0,outpix_row_1_wo_offset_fm0;
#pragma HLS resource variable=outpix_row_0_wo_offset_fm0 core=DivnS
#pragma HLS resource variable=outpix_row_1_wo_offset_fm0 core=DivnS
			outpix_row_0_wo_offset_fm0 = outpix_index_0_wo_offset_fm0/output_desc.width ;
			outpix_row_1_wo_offset_fm0 = outpix_index_1_wo_offset_fm0/output_desc.width ;

			ap_uint<16> outpix_row_off_0_fm0,outpix_row_off_1_fm0;
#pragma HLS RESOURCE variable=outpix_row_off_0_fm0 core=MulnS latency=2
#pragma HLS RESOURCE variable=outpix_row_off_1_fm0 core=MulnS latency=2
			outpix_row_off_0_fm0 = outpix_row_0*output_desc.width;
			outpix_row_off_1_fm0 = outpix_row_1*output_desc.width;

			ap_uint<14> outpix_col_0_fm0 = outpix_index_0_fm0 - outpix_row_off_0_fm0;
			ap_uint<14> outpix_col_1_fm0 = outpix_index_1_fm0 - outpix_row_off_1_fm0;

//			ap_uint<14> inpix_col_0_fm0 = (outpix_index_0_fm0 * weight_desc.filter_stride) - outpix_row_off_0_fm0;
//			ap_uint<14> inpix_col_1_fm0 = (outpix_index_0_fm0 * weight_desc.filter_stride) - outpix_row_off_1_fm0;

			if(pix_cnt_fm0 < (XI_PIX_PROC/2))
			{

				outpixels1_fl0.pix_rows[pix_cnt_fm0] = outpix_row_0;
				outpixels1_fl0.pix_cols[pix_cnt_fm0] = outpix_col_0_fm0;

				outpixels1_fl0.pix_rows[pix_cnt_fm0+1] = outpix_row_1;
				outpixels1_fl0.pix_cols[pix_cnt_fm0+1] = outpix_col_1_fm0;

			}
			else
			{
				outpixels2_fl0.pix_rows[pix_cnt_fm0-(XI_PIX_PROC/2)] = outpix_row_0;
				outpixels2_fl0.pix_cols[pix_cnt_fm0-(XI_PIX_PROC/2)] = outpix_col_0_fm0;

				outpixels2_fl0.pix_rows[(pix_cnt_fm0-(XI_PIX_PROC/2))+1] = outpix_row_1;
				outpixels2_fl0.pix_cols[(pix_cnt_fm0-(XI_PIX_PROC/2))+1] = outpix_col_1_fm0;
			}

			ap_uint<16> outpix_row_0_wo_offset_0_fm0;
			ap_uint<16> outpix_row_0_wo_offset_1_fm0;
			ap_uint<16> stgrow_wd_off_fm0 ;

#pragma HLS RESOURCE variable=outpix_row_0_wo_offset_0_fm0 core=MulnS latency=2
			outpix_row_0_wo_offset_0_fm0 = outpix_row_0_wo_offset_fm0*input_desc.width;
#pragma HLS RESOURCE variable=outpix_row_0_wo_offset_1_fm0 core=MulnS latency=2
			outpix_row_0_wo_offset_1_fm0 = outpix_row_1_wo_offset_fm0*input_desc.width;

#pragma HLS RESOURCE variable=stgrow_wd_off_fm0 core=MulnS latency=2
			stgrow_wd_off_fm0 = stgrow_wd_fm0*(plane_loop0_fm0.range(15,3));


			ap_uint<14> source_dim3_0_fm0 = ( (outpix_row_0_wo_offset_0_fm0 + outpix_col_0_fm0)*weight_desc.filter_stride) + stgrow_wd_off_fm0;
			ap_uint<14> source_dim3_1_fm0 = ( (outpix_row_0_wo_offset_1_fm0 + outpix_col_1_fm0)*weight_desc.filter_stride) + stgrow_wd_off_fm0;
//			ap_uint<14> source_dim3_0_fm0 = outpix_row_0_wo_offset_0_fm0 + (outpix_col_0_fm0*1) + stgrow_wd_off_fm0;
//			ap_uint<14> source_dim3_1_fm0 = outpix_row_0_wo_offset_1_fm0 + (outpix_col_1_fm0*1) + stgrow_wd_off_fm0;
			ap_uint<3> source_dim2_0_fm0 = plane_loop0_fm0.range(2,0);
			ap_uint<3> source_dim2_1_fm0 = plane_loop1_fm0.range(2,0);

			ap_uint<7> destination_dim2_0_fm0, destination_dim2_1_fm0;
			ap_uint<10> destination_dim3_0_fm0, destination_dim3_1_fm0;
			if(pix_cnt_fm0 < XI_PIX_PROC/2)
			{
				destination_dim2_0_fm0 = pix_cnt_fm0;
				destination_dim2_1_fm0 = pix_cnt_fm0+1 ;

				destination_dim3_0_fm0 = plane_loop0_initial_fm0;//plane_loop0_fm0 - current_plane_fc0;//.range(1,0);
				destination_dim3_1_fm0 = plane_loop1_initial_fm0;//plane_loop1_fm0 - current_plane_fc0;//.range(1,0);
			}
			else
			{
				destination_dim2_0_fm0 = pix_cnt_fm0 - XI_PIX_PROC/2;
				destination_dim2_1_fm0 = pix_cnt_fm0+1 - XI_PIX_PROC/2;

				destination_dim3_0_fm0 = plane_loop0_initial_fm0 +512;//plane_loop0_fm0-current_plane_fc0 + 512;
				destination_dim3_1_fm0 = plane_loop1_initial_fm0 +512;//plane_loop1_fm0-current_plane_fc0 + 512;
			}




			//** generate destination_dim3, destination_dim2
			short registers1_fm0[4][8][2];
#pragma HLS ARRAY_PARTITION variable=registers1_fm0 complete dim=0
			Iter_Loop:
			for(int iter_fm0=0;iter_fm0<8;iter_fm0++)
			{
#pragma HLS UNROLL
				Iter1_Loop:
				for(int iter1_fm0=0;iter1_fm0<4;iter1_fm0++)
				{
#pragma HLS UNROLL
					registers1_fm0[iter1_fm0][iter_fm0][0] = istaging_buff0_fb0[iter1_fm0][iter_fm0][source_dim3_0_fm0];
					registers1_fm0[iter1_fm0][iter_fm0][1] = istaging_buff0_fb0[iter1_fm0][iter_fm0][source_dim3_1_fm0];
				}//Iter1_Loop
			}//Iter_Loop

			short registers2_fm0[4][2][2];
#pragma HLS ARRAY_PARTITION variable=registers2_fm0 complete dim=0

			Iter_Loop1:
			for(int iter_fm1=0;iter_fm1<2;iter_fm1++)
			{
#pragma HLS UNROLL
				Iter1_Loop1:
				for(int iter1_fm1=0;iter1_fm1<4;iter1_fm1++)
				{
#pragma HLS UNROLL
					if(iter_fm1==0)
					{
						if(plane_select[iter1_fm1][iter_fm1]==1)
						{
							registers2_fm0[iter1_fm1][0][0] = registers1_fm0[iter1_fm1][source_dim2_0_fm0][0];
							registers2_fm0[iter1_fm1][0][1] = registers1_fm0[iter1_fm1][source_dim2_0_fm0][1];
						}
						else
						{
							registers2_fm0[iter1_fm1][0][0] = 0;
							registers2_fm0[iter1_fm1][0][1] = 0;
						}
					}
					else
					{
						if(plane_select[iter1_fm1][iter_fm1]==1)
						{
							registers2_fm0[iter1_fm1][1][0] = registers1_fm0[iter1_fm1][source_dim2_1_fm0][0];
							registers2_fm0[iter1_fm1][1][1] = registers1_fm0[iter1_fm1][source_dim2_1_fm0][1];
						}
						else
						{
							registers2_fm0[iter1_fm1][0][0] = 0;
							registers2_fm0[iter1_fm1][0][1] = 0;
						}
					}


				}//Iter1_Loop1
			}//Iter_Loop1


			Dim2_Loop:
			for(ap_uint<7> dim2_fm0=0; dim2_fm0 < XI_PIX_PROC/2; dim2_fm0++)
			{
#pragma HLS unroll
				Dim1_Loop:
				for(ap_uint<3> dim1=0; dim1 < 4; dim1++)
				{
#pragma HLS unroll
					if(dim2_fm0 == destination_dim2_0_fm0)
					{
						input_buff0_fc0[dim1][dim2_fm0][destination_dim3_0_fm0] = registers2_fm0[dim1][0][0];
						input_buff0_fc0[dim1][dim2_fm0][destination_dim3_1_fm0] = registers2_fm0[dim1][1][0];
					}
					else if(dim2_fm0 == destination_dim2_1_fm0)
					{
						input_buff0_fc0[dim1][dim2_fm0][destination_dim3_0_fm0] = registers2_fm0[dim1][0][1];
						input_buff0_fc0[dim1][dim2_fm0][destination_dim3_1_fm0] = registers2_fm0[dim1][1][1];
					}
				}//Dim1_Loop
			}//Dim2_Loop

		}//Pix_1x1_Loop
	}//Plane_1x1_Loop

}

void lrn_inter_plane_select(ap_uint<8> *pattern_next_state, ap_uint<8> pad_num, ap_uint<16> input_planes, ap_uint<16> center_plane, ap_uint<8> ker_id, bool plane_select[4][2], ap_uint<16> plane_start)
{
#pragma HLS ARRAY_PARTITION variable=plane_select complete dim=0
#pragma HLS inline off

	ap_uint<8> pattern_present_state = *pattern_next_state;
	if(pattern_present_state[7]==1)
		*pattern_next_state = 31;
	else
		*pattern_next_state = (pattern_present_state<<1) + pattern_present_state[4];

	ap_int<16> plane_offset = ker_id + center_plane - pad_num;
	if(plane_offset < 0)
	{
		plane_start  = 0;
	}
	else
	{
		plane_start = 4*plane_offset.range(15,2);
	}

	for(ap_uint<4> plane = 0; plane < 8; plane++)
	{
#pragma HLS unroll
		ap_uint<16> plane_id = plane + plane_offset;
		if(plane_id > input_planes)
			plane_select[plane_id.range(1,0)][plane_id[0]] = 0;
		else
			plane_select[plane_id.range(1,0)][plane_id[0]] = pattern_present_state[plane];
	}
}

template<int CFILTER_SIZE,int CCONV_STRIDE,int CNUM_KERNELS,int INPUTP>
void LoadFeedingBuff_fl(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		out_pix_struct &out_pixels0_fc0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024],
		ap_uint<16> pc_fc0,
		ap_uint<10> current_plane_fc0,
		ap_uint<12> out_row_offset_fb0,
		ap_int<12> pad_row_fc0,
		ap_int<12> pad_row_wo_fc0,
		ap_uint<16> row_id_1st_32pix_fc0,
		ap_uint<16> col_id_1st_32pix_fc0,
		ap_uint<16> row_id_2nd_32pix_fc0,
		ap_uint<16> col_id_2nd_32pix_fc0,
		ap_uint<2> mac_fz0)
{
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=2
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM

#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=2
#pragma HLS resource variable=istaging_buff0_fb0 latency=2 core=RAM_T2P_BRAM
#pragma HLS inline off

	ap_uint<16> pix_row_1st_wo_offset_fl0 	= row_id_1st_32pix_fc0;
	ap_uint<16> pix_row_1st_fl0 			= row_id_1st_32pix_fc0 + out_row_offset_fb0;
	ap_uint<16> pix_col_1st_fl0 			= col_id_1st_32pix_fc0;
	ap_uint<16> pix_row_2nd_wo_offset_fl0 	= row_id_2nd_32pix_fc0;
	ap_uint<16> pix_row_2nd_fl0	 			= row_id_2nd_32pix_fc0 + out_row_offset_fb0;
	ap_uint<16> pix_col_2nd_fl0 			= col_id_2nd_32pix_fc0;

	out_pix2_struct outpixels1_fl0;
	out_pix2_struct outpixels2_fl0;
#pragma HLS ARRAY_PARTITION variable=outpixels1_fl0.pix_cols complete dim=1
#pragma HLS ARRAY_PARTITION variable=outpixels1_fl0.pix_rows complete dim=1
#pragma HLS ARRAY_PARTITION variable=outpixels2_fl0.pix_cols complete dim=1
#pragma HLS ARRAY_PARTITION variable=outpixels2_fl0.pix_rows complete dim=1

	ap_uint<10> center_plane = current_plane_fc0;
	bool plane_select[4][2];
#pragma HLS ARRAY_PARTITION variable=plane_select complete dim=0
	bool plane_select_all[4][2];
#pragma HLS ARRAY_PARTITION variable=plane_select_all complete dim=0
	for(ap_uint<4> pl=0; pl<4; pl++)
	{
#pragma HLS unroll
		for(ap_uint<3> set=0; set<2; set++)
		{
#pragma HLS unroll
			plane_select_all[pl][set] = 1;
		}
	}

#if 0
	if(conv_desc.lrn_inter_sos_enable==1)
	{
		LoadInputBuff_non1x1_fn<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,INPUTP>
		(input_desc, conv_desc, output_desc,weight_desc, outpixels1_fl0, outpixels2_fl0,istaging_buff0_fb0, input_buff0_fc0, pix_row_1st_fl0,pix_col_1st_fl0,
				/*..continue..*/pix_row_1st_wo_offset_fl0, 0,0,0,current_plane_fc0, pad_row_fc0, pad_row_wo_fc0,mac_fz0,0);
	}
	else if(conv_desc.lrn_intra_sos_enable==1 || conv_desc.bn_intra_sos_enable==1)
	{
		for(ap_uint<7> start_pix_per_ker, ker = 0; ker < XI_KER_PROC; start_pix_per_ker+=conv_desc.pix_per_kp, ker++)
		{
			LoadInputBuff_non1x1_fn<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,INPUTP>
			(input_desc, conv_desc, output_desc,weight_desc, outpixels1_fl0, outpixels1_fl0,istaging_buff0_fb0, input_buff0_fc0, pix_row_1st_fl0,pix_col_1st_fl0,
					/*..continue..*/pix_row_1st_wo_offset_fl0, 0,0,0,current_plane_fc0, pad_row_fc0, pad_row_wo_fc0,mac_fz0,start_pix_per_ker);
		}
	}
	else if(conv_desc.l2_sos_enable==1)
	{
		LoadInputBuff1x1_fm<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,INPUTP>
		(input_desc, conv_desc, output_desc,istaging_buff0_fb0, input_buff0_fc0, pix_row_1st_fl0,pix_col_1st_fl0, pix_row_1st_wo_offset_fl0,
				/*..continue..*/pix_row_2nd_wo_offset_fl0,current_plane_fc0, outpixels1_fl0, outpixels2_fl0,mac_fz0,0,plane_select_all,0);
	}
	else// conv operation
	{
		if(weight_desc.filter_size == 1)
		{
			LoadInputBuff1x1_fm<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,INPUTP>
			(input_desc, conv_desc, output_desc,istaging_buff0_fb0, input_buff0_fc0, pix_row_1st_fl0,pix_col_1st_fl0, pix_row_1st_wo_offset_fl0,
					/*..continue..*/pix_row_2nd_wo_offset_fl0,current_plane_fc0, outpixels1_fl0, outpixels2_fl0,mac_fz0,0,plane_select_all,0);
		}
		else
		{
			LoadInputBuff_non1x1_fn<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,INPUTP>
			(input_desc, conv_desc, output_desc,weight_desc, outpixels1_fl0, outpixels2_fl0,istaging_buff0_fb0, input_buff0_fc0, pix_row_1st_fl0,pix_col_1st_fl0,
					/*..continue..*/pix_row_1st_wo_offset_fl0, pix_row_2nd_fl0,pix_col_2nd_fl0,pix_row_2nd_wo_offset_fl0,current_plane_fc0, pad_row_fc0, pad_row_wo_fc0,mac_fz0,0);
		}
	}
#endif

#if 1
	ap_uint<7> ker_loop_cnt;
	if(conv_desc.lrn_intra_sos_enable==1 || conv_desc.bn_intra_sos_enable==1)
		ker_loop_cnt = XI_KER_PROC;
	else
		ker_loop_cnt = 1;

	ap_uint<16> var1, var2, var3;
	if(conv_desc.sos_enable == 0)
	{
		var1=pix_row_2nd_fl0;
		var2=pix_col_2nd_fl0;
		var3=pix_row_2nd_wo_offset_fl0;
	}
	else
	{
		var1=0;
		var2=0;
		var3=0;
	}

	if(conv_desc.l2_sos_enable==1 || (conv_desc.sos_enable == 0 && weight_desc.filter_size == 1) )
	{
		LoadInputBuff1x1_fm<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,INPUTP>
		(input_desc, conv_desc, output_desc, weight_desc,istaging_buff0_fb0, input_buff0_fc0, pix_row_1st_fl0,pix_col_1st_fl0, pix_row_1st_wo_offset_fl0,
				/*..continue..*/pix_row_2nd_wo_offset_fl0,current_plane_fc0, outpixels1_fl0, outpixels2_fl0,mac_fz0,0,plane_select_all,0);
	}
	else //if(conv_desc.lrn_inter_sos_enable==1 || conv_desc.lrn_intra_sos_enable==1 || conv_desc.bn_intra_sos_enable==1 || (conv_desc.sos_enable == 0 && weight_desc.filter_size != 1))
	{
		for(ap_uint<7> start_pix_per_ker, ker = 0; ker < ker_loop_cnt; start_pix_per_ker+=conv_desc.pix_per_kp, ker++)
		{
			LoadInputBuff_non1x1_fn<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,INPUTP>
			(input_desc, conv_desc, output_desc,weight_desc, outpixels1_fl0, outpixels2_fl0,istaging_buff0_fb0, input_buff0_fc0, pix_row_1st_fl0,pix_col_1st_fl0,
					/*..continue..*/pix_row_1st_wo_offset_fl0, var1,var2,var3,current_plane_fc0, pad_row_fc0, pad_row_wo_fc0,mac_fz0,start_pix_per_ker);
		}
	}
#endif

	Pix32_1st_Loop:
	for(ap_uint<7> iter0_fl0=0;iter0_fl0<XI_PIX_PROC/2;iter0_fl0++)
	{
#pragma HLS UNROLL
		out_pixels0_fc0.pix_rows[iter0_fl0] = outpixels1_fl0.pix_rows[iter0_fl0];
		out_pixels0_fc0.pix_cols[iter0_fl0] = outpixels1_fl0.pix_cols[iter0_fl0];
	}//Pix32_1st_Loop

	Pix32_2nd_Loop:
	for(ap_uint<7> iter1_fl0=XI_PIX_PROC/2;iter1_fl0<XI_PIX_PROC;iter1_fl0++)
	{
#pragma HLS UNROLL
		out_pixels0_fc0.pix_rows[iter1_fl0] = outpixels2_fl0.pix_rows[iter1_fl0-(XI_PIX_PROC/2)];
		out_pixels0_fc0.pix_cols[iter1_fl0] = outpixels2_fl0.pix_cols[iter1_fl0-(XI_PIX_PROC/2)];
	}//Pix32_2nd_Loop
	out_pixels0_fc0.current_plane_by4 = current_plane_fc0;

}

template<int OUT_WW>
void OutputWrite_fk(conv_struct conv_desc,
		ap_uint<16> pixbuff_planeoffset_fj0,
		gmem_outputtype *gmem_output1_fa0,
		gmem_outputtype *gmem_output2_fa0,
		bool oStagBuf_dim2_bool_fj0,
		short ostaging_buff0_fb0[4][8][XI_OSTAGEBUFF_DEPTH],
		ap_uint<16> layerx_loop_cnt_fj0,
		output_struct output_desc)
{
#pragma HLS inline off


	Write_Loop:
	for(ap_uint<16> ddr_pntr_fk0=0; ddr_pntr_fk0<layerx_loop_cnt_fj0; ddr_pntr_fk0++)
	{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=11*120 max=11*120
		if(ddr_pntr_fk0==224)
			int h=0;
		ap_uint<16> pixbuff_index_fk0 = pixbuff_planeoffset_fj0 + ddr_pntr_fk0;

		ap_uint<16> ddr_pnt_size_div_fk0 = ddr_pntr_fk0 / output_desc.size;
		ap_uint<16> ddr_pnt_size_mod_fk0 = ddr_pntr_fk0 - (ddr_pnt_size_div_fk0*output_desc.size);
		ap_uint<16> ostg_addr_fk0;
		bool eff_plane32_bool_fk0;
		if(output_desc.height == conv_desc.ostg_row_count)
		{
			ostg_addr_fk0 = ddr_pnt_size_div_fk0.range(15,1)*conv_desc.out_pix + ddr_pnt_size_mod_fk0;
			//ap_uint<16> plane32_offset_fk0 = (2*ddr_pnt_size_div_fk0);
			eff_plane32_bool_fk0 = ddr_pnt_size_div_fk0[0];

		}
		else
		{
			ostg_addr_fk0 = pixbuff_index_fk0;
			eff_plane32_bool_fk0 = oStagBuf_dim2_bool_fj0;

		}

#if !XI_OTPT_64bit_PORT// 8 plane 128bit IO port
		outtype word1_128bit_fk0;outtype word2_128bit_fk0;

		Outtype_Loop:
		for(ap_uint<8> planes_fh0=0, bit_fh0=0,bit2_fh0=64;planes_fh0<4;planes_fh0++, bit_fh0+=16,bit2_fh0+=16)
		{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL

			ap_int<16> word_fh0;
			ap_int<16> word2_fh0;
			ap_int<16> word3_fh0;
			ap_int<16> word4_fh0;
			if(eff_plane32_bool_fk0 == 0)
			{
#if XI_OSTAGEBUFF_PIX==4
				fprintf(stderr,"ostg_pix_buff =4 pending");//TODO
#elif XI_OSTAGEBUFF_PIX==2
				word_fh0 = ostaging_buff0_fb0[planes_fh0.range(1,0)][0][ostg_addr_fk0];
				word2_fh0 = ostaging_buff0_fb0[planes_fh0.range(1,0)][1][ostg_addr_fk0];
				word3_fh0 = ostaging_buff0_fb0[planes_fh0.range(1,0)][2][ostg_addr_fk0];
				word4_fh0 = ostaging_buff0_fb0[planes_fh0.range(1,0)][3][ostg_addr_fk0];
#elif XI_OSTAGEBUFF_PIX==1
				fprintf(stderr,"ostg_pix_buff =1 pending");		//TODO
#endif
			}
			else
			{
#if XI_OSTAGEBUFF_PIX==4
				fprintf(stderr,"ostg_pix_buff =4 pending");//TODO
#elif XI_OSTAGEBUFF_PIX==2
				word_fh0 = ostaging_buff0_fb0[planes_fh0.range(1,0)][4][ostg_addr_fk0];
				word2_fh0 = ostaging_buff0_fb0[planes_fh0.range(1,0)][5][ostg_addr_fk0];
				word3_fh0 = ostaging_buff0_fb0[planes_fh0.range(1,0)][6][ostg_addr_fk0];
				word4_fh0 = ostaging_buff0_fb0[planes_fh0.range(1,0)][7][ostg_addr_fk0];
#elif XI_OSTAGEBUFF_PIX==1
				fprintf(stderr,"ostg_pix_buff =1 pending");		//TODO
#endif
			}

			short word_relu_fh0;
			short word2_relu_fh0;
			short word3_relu_fh0;
			short word4_relu_fh0;

			if((conv_desc.relu_en == 1) && (word_fh0[15] == 1))
				word_relu_fh0 = 0;
			else
				word_relu_fh0 = word_fh0;

			if((conv_desc.relu_en == 1) && (word2_fh0[15] == 1))
				word2_relu_fh0 = 0;
			else
				word2_relu_fh0 = word2_fh0;

			if((conv_desc.relu_en == 1) && (word3_fh0[15] == 1))
				word3_relu_fh0 = 0;
			else
				word3_relu_fh0 = word3_fh0;

			if((conv_desc.relu_en == 1) && (word4_fh0[15] == 1))
				word4_relu_fh0 = 0;
			else
				word4_relu_fh0 = word4_fh0;


			word1_128bit_fk0.range(bit_fh0+15,bit_fh0) = word_relu_fh0;
			word1_128bit_fk0.range(bit2_fh0+15,bit2_fh0) = word2_relu_fh0;
			word2_128bit_fk0.range(bit_fh0+15,bit_fh0) = word3_relu_fh0;
			word2_128bit_fk0.range(bit2_fh0+15,bit2_fh0) = word4_relu_fh0;
		}//Outtype_loop
		gmem_output1_fa0[ddr_pntr_fk0] = word1_128bit_fk0;
		gmem_output2_fa0[ddr_pntr_fk0] = word2_128bit_fk0;
	}




#if 0//!__SYNTHESIS__
	printf("\n add:%d",(int)ddr_pntr_fk0);
	for(int i=0,bit=0;i<8;i++,bit=bit+16)
	{
		short temp = gmem_output1_fa0[ddr_pntr_fk0].range(bit+15,bit);
		printf("\t %d",temp);
	}
#endif

	/*		outtype word_128bit_fk0=0;
				for(ap_uint<7> planes_fk0=0,bit_fk0=0; planes_fk0<8; planes_fk0++, bit_fk0+=16)
				{
		#pragma HLS UNROLL
					ap_uint<3> dim2_ostg_fk0 = oStagBuf_dim2_fj0+planes_fk0[2];
					ap_int<16> word_fk0;

		#if XI_OSTAGEBUFF_PIX==4
					word_fk0 = ostaging_buff0_fb0[planes_fk0.range(1,0)][dim2_ostg_fk0][pixbuff_index_fk0.range(1,0)][pixbuff_index_fk0.range(11,2)];
		#elif XI_OSTAGEBUFF_PIX==2
					word_fk0 = oStagingBuff_fb0[planes_fk0.range(1,0)][dim2_ostg_fk0][pixbuff_index_fk0[0]][pixbuff_index_fk0.range(11,1)];
		#elif XI_OSTAGEBUFF_PIX==1
					word_fk0 = oStagingBuff_fb0[planes_fk0.range(1,0)][dim2_ostg_fk0][0][pixbuff_index_fk0];
		#endif

					bool relu_en_fk0 = !((conv_desc.relu_en == 1) && (word_fk0[15] == 1));
					ap_int<16> word_16bit_relu;

					for(ap_uint<5> bitwise_fk0=0; bitwise_fk0<16; bitwise_fk0++){    //int -> arbitrary precision  change
		#pragma HLS unroll
						word_16bit_relu[bitwise_fk0] = word_fk0[bitwise_fk0] & relu_en_fk0;
					}

					word_128bit_fk0.range(bit_fk0+15,bit_fk0) = word_16bit_relu;
				}
				gmem_output1_fa0[ddr_pntr_fk0] = word_128bit_fk0;*/
#else//4 plane 64bit IO port

	outtype word_64bit_fk0=0;
	Out_Type_Loop:
	for(ap_uint<7> planes_fk1=0,bit_fk1=0; planes_fk1<4; planes_fk1++, bit_fk1+=16)
	{
#pragma HLS UNROLL
		ap_int<16> word_fk1;
#if XI_OSTAGEBUFF_PIX==4
		word_fk1 = ostaging_buff0_fb0[planes_fk1][oStagBuf_dim2_fj0][pixbuff_index_fk0.range(1,0)][pixbuff_index_fk0.range(11,2)];
#elif XI_OSTAGEBUFF_PIX==2
		word_fk1 = ostaging_buff0_fb0[planes_fk1][oStagBuf_dim2_fj0][pixbuff_index_fk0[0]][pixbuff_index_fk0.range(11,1)];
#elif XI_OSTAGEBUFF_PIX==1
		word_fk1 = ostaging_buff0_fb0[planes_fk1][oStagBuf_dim2_fj0][0][pixbuff_index_fk0];
#endif

		bool relu_en_fk1 = !((conv_desc.relu_en == 1) && (word_fk1[15] == 1));
		ap_int<16> word_16bit_relu;

		Relu_Loop:
		for(ap_uint<5> bitwise_fk1=0; bitwise_fk1<16; bitwise_fk1++)
		{
#pragma HLS unroll
			word_16bit_relu[bitwise_fk1] = word_fk1[bitwise_fk1] & relu_en_fk1;
		}//Relu_Loop

		word_64bit_fk0.range(bit_fk1+15,bit_fk1) = word_16bit_relu;
	}//Out_Type_Loop
	gmem_output1_fa0[ddr_pntr_fk0] = word_64bit_fk0;
#endif


}

template<int CNUM_KERNELS, int OUT_WW>
void StoreOStagingBuff_fj(output_struct output_desc,
		conv_struct conv_desc,
		ap_uint<12> out_row_offset_fb0,
		gmem_outputtype * gmem_output1_fa0,
		gmem_outputtype * gmem_output2_fa0,
		short ostaging_buff0_fb0[4][8][XI_OSTAGEBUFF_DEPTH],
		ap_uint<12> rows_to_process_fb0,
		bool write_en_fb0)
{
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2

#pragma HLS inline off

	ap_uint<16> layerx_loop_cnt_fj0;
	ap_uint<16> layerx_img_fj0;

	ap_uint<10> out_plane_align16 = output_desc.planes.range(15,4) + output_desc.planes[3];

#if !XI_OTPT_64bit_PORT//##8 planes
	if(output_desc.height == conv_desc.ostg_row_count)
	{
		layerx_img_fj0 = 16;
		layerx_loop_cnt_fj0 = output_desc.size * out_plane_align16;
	}
	else
	{
		layerx_img_fj0 = output_desc.planes;
		layerx_loop_cnt_fj0 = rows_to_process_fb0*output_desc.width;
	}
#endif

	if(write_en_fb0)
	{
#if !XI_OTPT_64bit_PORT//8 planes
		Img_Out_Loop:
		for(ap_uint<16> outImg_fj0=0; outImg_fj0<layerx_img_fj0; outImg_fj0+=16)
		{
#pragma HLS LOOP_TRIPCOUNT min=2 max=2


			bool oStagBuf_dim2_bool_fj0 = outImg_fj0[4];
			ap_uint<16> pixbuff_planeoffset_fj0 = (outImg_fj0.range(15,5)) * (conv_desc.out_pix);

			ap_uint<16>  outimgGby8_fj0;

			outimgGby8_fj0 = outImg_fj0.range(15,4);


			ap_uint<32> offset_fj0 = ((out_row_offset_fb0)*output_desc.width)  + ((outimgGby8_fj0*output_desc.size));
			//			fprintf(stderr,"offset_fj0 =%d   outImg_fj0 = %d out_row_offset_fb0=%d \n",(int)offset_fj0,(int)outImg_fj0,(int)out_row_offset_fb0);


			OutputWrite_fk<OUT_WW>
			(conv_desc, pixbuff_planeoffset_fj0, gmem_output1_fa0+offset_fj0,gmem_output2_fa0+offset_fj0, oStagBuf_dim2_bool_fj0, ostaging_buff0_fb0, layerx_loop_cnt_fj0, output_desc);

		}//Img_Out_Loop

#else// 4 plane
		ap_uint<32> out_hw_fj0 = output_desc.height*output_desc.width;
		Img_Out_Loop:
		for(ap_uint<16> outImg_fj1=0; outImg_fj1<output_desc.planes; outImg_fj1+=4)
		{
#pragma HLS LOOP_TRIPCOUNT min=24 max=24

			ap_uint<3> oStagBuf_dim2_fj1 = outImg_fj1.range(4,2);
			ap_uint<12> pixbuff_planeoffset_fj1 = (outImg_fj1.range(15,5)) * (conv_desc.out_pix);

			ap_uint<16> outImgby4_fj1;
			if(outImg_fj1 < (output_desc.planes/2))
				outImgby4_fj1 = outImg_fj1.range(15,2);
			else
				outImgby4_fj1 = outImg_fj1.range(15,2) - (output_desc.planes(15,3)*conv_desc.buff_split);

			ap_uint<32> offset_fj1 = ((out_row_offset_fb0)*output_desc.width)  + (outImgby4_fj1*out_hw_fj0);

			if((outImg_fj1 < (output_desc.planes/2)) || (conv_desc.buff_split==0))
				OutputWrite_fk<OUT_WW>
			(conv_desc, pixbuff_planeoffset_fj1, gmem_output1_fa0+offset_fj1, oStagBuf_dim2_fj1, ostaging_buff0_fb0, rows_to_process_fb0, output_desc);
			else
				OutputWrite_fk<OUT_WW>(conv_desc, pixbuff_planeoffset_fj1, gmem_output2_fa0+offset_fj1, oStagBuf_dim2_fj1, ostaging_buff0_fb0, layerx_loop_cnt_fj0, output_desc);
		}//Img_Out_Loop
#endif
	}
}
template<int CNUM_KERNELS, int OUT_WW>
void StoreOStagingBuff_En_fj(output_struct output_desc,
		conv_struct conv_desc,
		ap_uint<12> out_row_offset_fb0,
		gmem_outputtype * gmem_output1_fa0,
		gmem_outputtype * gmem_output2_fa0,
		short ostaging_buff0_fb0[4][8][XI_OSTAGEBUFF_DEPTH],
		ap_uint<12> rows_to_process_fb0,
		bool write_en_fb0)
{
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2

#pragma HLS inline off
	//if(!(conv_desc.lrn_pns_enable || conv_desc.bn_snb_enable || conv_desc.elem_wise_add_en || conv_desc.bn_conv_fuse_en))
	if(!(conv_desc.lrn_pns_enable || conv_desc.bn_snb_enable || conv_desc.elem_wise_add_en))
	{
		StoreOStagingBuff_fj<CNUM_KERNELS, OUT_WW>
		(output_desc, conv_desc, out_row_offset_fb0, gmem_output1_fa0, gmem_output2_fa0, ostaging_buff0_fb0, rows_to_process_fb0,write_en_fb0);
	}
}
void InputReadLayer1_fi_mean_sub(ap_uint<16> layerx_loop_cnt_fg0,
		input_struct input_desc ,
		gmem_inputtype_layer1 *gmem_input_layer1_fa0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH])
{
#pragma HLS INLINE OFF

	short mean_sub_fi0[4];
#pragma HLS ARRAY_PARTITION variable=mean_sub_fi0 complete dim=0
	Mean_Loop:
	for(ap_uint<4> plane_fi1=0;plane_fi1<4;plane_fi1++)
	{
#pragma HLS UNROLL
		mean_sub_fi0[plane_fi1]=input_desc.mean_value[plane_fi1];
	}// Mean_Loop

	ap_uint<3> rowindex_fi0=0;

	//#if XI_MEAN_SUB
	Input_Layer1_mean_sub_Loop:
	for(ap_uint<16> ddr_pntr_fi0=0; ddr_pntr_fi0<layerx_loop_cnt_fg0/2;ddr_pntr_fi0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=4800 max=4800
#pragma HLS pipeline
		ap_uint<14> rowcnt_fi0;
#pragma HLS resource variable=rowcnt_fi0 core=DivnS
		rowcnt_fi0 = (ddr_pntr_fi0*2)/input_desc.width;

		ap_uint<16> col_fi0 = (ddr_pntr_fi0*2) - (rowcnt_fi0*input_desc.width);
		ap_uint<14> row_stag_index_fi0;
#if XI_ROW2
		row_stag_index_fi0 = rowcnt_fi0[0]*input_desc.width;
		if(rowcnt_fi0[0] == 0 && rowcnt_fi0 != 0 && col_fi0==0)
			rowindex_fi0++;
#elif XI_ROW4
		row_stag_index_fi0 = rowcnt_fi0.range(1,0)*input_desc.width;
		if(rowcnt_fi0.range(1,0) == 0 && rowcnt_fi0 != 0 && col_fi0==0)
			rowindex_fi0++;
#elif XI_ROW8
		row_stag_index_fi0 = rowcnt_fi0.range(2,0)*input_desc.width;
		if(rowcnt_fi0.range(2,0) == 0 && rowcnt_fi0 != 0 && col_fi0==0)
			rowindex_fi0++;
#endif
		ap_uint<14> istg_addr_fi0 = col_fi0+row_stag_index_fi0;

		inputtype read_64bit_fi0 = gmem_input_layer1_fa0[ddr_pntr_fi0];
		Inputtype_mean_sub_Loop:
		for(ap_uint<7> planes_fi0=0, bit_fi0=0, bit2_fi0=32;planes_fi0<4;planes_fi0++, bit_fi0+=8, bit2_fi0+=8)
		{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL
			short word_fi0 = (short)(read_64bit_fi0.range(bit_fi0+7,bit_fi0)<<7) - mean_sub_fi0[planes_fi0];
			short word2_fi0 = (short)(read_64bit_fi0.range(bit2_fi0+7,bit2_fi0)<<7) - mean_sub_fi0[planes_fi0];

			istaging_buff0_fb0[planes_fi0][rowindex_fi0][istg_addr_fi0] = word_fi0;
			istaging_buff0_fb0[planes_fi0][rowindex_fi0][istg_addr_fi0+1] = word2_fi0;
		}//Inputtype_Loop
	}//Input_Layer1_Loop
	//#else

	//#endif
}

void InputReadLayer1_fi(ap_uint<16> layerx_loop_cnt_fg0,
		input_struct input_desc ,
		gmem_inputtype_layer1 *gmem_input_layer1_fa0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH])
{
#pragma HLS INLINE OFF

	short mean_sub_fi0[4];
#pragma HLS ARRAY_PARTITION variable=mean_sub_fi0 complete dim=0
	Mean_Loop:
	for(ap_uint<4> plane_fi1=0;plane_fi1<4;plane_fi1++)
	{
#pragma HLS UNROLL
		mean_sub_fi0[plane_fi1]=input_desc.mean_value[plane_fi1];
	}// Mean_Loop

	ap_uint<3> rowindex_fi0=0;

	//#if XI_MEAN_SUB

	//#else
	Input_Layer1_Loop:
	for(ap_uint<16> ddr_pntr_fi0=0; ddr_pntr_fi0<layerx_loop_cnt_fg0;ddr_pntr_fi0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=4800 max=4800
#pragma HLS pipeline
		ap_uint<14> rowcnt_fi0;
#pragma HLS resource variable=rowcnt_fi0 core=DivnS
		rowcnt_fi0 = ddr_pntr_fi0/input_desc.width;

		ap_uint<16> col_fi0 = ddr_pntr_fi0 - (rowcnt_fi0*input_desc.width);
		ap_uint<14> row_stag_index_fi0;
#if XI_ROW2
		row_stag_index_fi0 = rowcnt_fi0[0]*input_desc.width;
		if(rowcnt_fi0[0] == 0 && rowcnt_fi0 != 0 && col_fi0==0)
			rowindex_fi0++;
#elif XI_ROW4
		row_stag_index_fi0 = rowcnt_fi0.range(1,0)*input_desc.width;
		if(rowcnt_fi0.range(1,0) == 0 && rowcnt_fi0 != 0 && col_fi0==0)
			rowindex_fi0++;
#elif XI_ROW8
		row_stag_index_fi0 = rowcnt_fi0.range(2,0)*input_desc.width;
		if(rowcnt_fi0.range(2,0) == 0 && rowcnt_fi0 != 0 && col_fi0==0)
			rowindex_fi0++;
#endif

		inputtype read_64bit_fi0 = gmem_input_layer1_fa0[ddr_pntr_fi0];
		Inputtype_Loop:
		for(ap_uint<7> planes_fi0=0, bit_fi0=0;planes_fi0<4;planes_fi0++, bit_fi0+=16)
		{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL
			short word_fi0 = read_64bit_fi0.range(bit_fi0+15,bit_fi0);
			istaging_buff0_fb0[planes_fi0][rowindex_fi0][col_fi0+row_stag_index_fi0] = word_fi0;
		}//Inputtype_Loop
	}//Input_Layer1_Loop
	//#endif
}
void WriteToIstg(inputtype2 read1_128bit_fh0,
		inputtype2 read2_128bit_fh0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<16> istg_addr_fh0,
		bool eff_plane32_bool_fh0

)
{
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 intra false
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 inter false

#pragma HLS interface register port=istg_addr_fh0
#pragma HLS interface register port=eff_plane32_bool_fh0

#pragma HLS INLINE OFF
	Inputtype_Loop:
	for(ap_uint<8> planes_fh0=0, bit_fh0=0,bit2_fh0=64;planes_fh0<4;planes_fh0++, bit_fh0+=16,bit2_fh0+=16)
	{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL

		short word_fh0 =read1_128bit_fh0.range(bit_fh0+15,bit_fh0);
		short word2_fh0 = read1_128bit_fh0.range(bit2_fh0+15,bit2_fh0);
		short word3_fh0 =read2_128bit_fh0.range(bit_fh0+15,bit_fh0);
		short word4_fh0 = read2_128bit_fh0.range(bit2_fh0+15,bit2_fh0);
		if(eff_plane32_bool_fh0 == 0)
		{
			istaging_buff0_fb0[planes_fh0.range(1,0)][0][istg_addr_fh0] = word_fh0;
			istaging_buff0_fb0[planes_fh0.range(1,0)][1][istg_addr_fh0] = word2_fh0;
			istaging_buff0_fb0[planes_fh0.range(1,0)][2][istg_addr_fh0] = word3_fh0;
			istaging_buff0_fb0[planes_fh0.range(1,0)][3][istg_addr_fh0] = word4_fh0;
		}
		else
		{
			istaging_buff0_fb0[planes_fh0.range(1,0)][4][istg_addr_fh0] = word_fh0;
			istaging_buff0_fb0[planes_fh0.range(1,0)][5][istg_addr_fh0] = word2_fh0;
			istaging_buff0_fb0[planes_fh0.range(1,0)][6][istg_addr_fh0] = word3_fh0;
			istaging_buff0_fb0[planes_fh0.range(1,0)][7][istg_addr_fh0] = word4_fh0;
		}

	}//Inputtype_Loop
}
void InputReadLayerOther_fh(conv_struct conv_desc,
		ap_uint<16> layerx_loop_cnt_fg0,
		input_struct input_desc ,
		bool plane32_bool_fg0,
		gmem_inputtype_layerx *gmem_input_layer_other1_fa0,
		gmem_inputtype_layerx *gmem_input_layer_other2_fa0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<12> startrow_fg0,
		ap_uint<12> endrow_fg0,
		ap_uint<16> input_plane_fg0)
{
#pragma HLS INLINE OFF

	ap_uint<16> in_plane_width_fh0;
#pragma HLS resource variable=in_plane_width_fh0 core=MulnS latency=2
	in_plane_width_fh0 = (input_plane_fg0.range(15,5))*input_desc.width;

	ap_uint<16> row_stage_offset_A_fh0;
#pragma HLS resource variable=row_stage_offset_A_fh0 core=MulnS latency=2
	row_stage_offset_A_fh0 = in_plane_width_fh0*(conv_desc.istg_row_count);

	Input_Layerx_Loop:
	for(ap_uint<16> ddr_pntr_fh0=0;ddr_pntr_fh0<layerx_loop_cnt_fg0;ddr_pntr_fh0++)
	{
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 intra false
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 inter false
#pragma HLS LOOP_TRIPCOUNT min=51*479 max=51*479

		ap_uint<16> row1_div_fh0;
#pragma HLS resource variable=row1_div_fh0 core=DivnS
		row1_div_fh0 = (ddr_pntr_fh0/input_desc.width);
		ap_uint<16> row_fh0 = row1_div_fh0 + startrow_fg0;

		ap_uint<12> num_rows_fh0 = row_fh0 - startrow_fg0;
		ap_uint<16> row_stage_offset_B_fh0;
#pragma HLS RESOURCE variable=row_stage_offset_B_fh0 core=MulnS latency=2
		row_stage_offset_B_fh0=num_rows_fh0*input_desc.width;

		ap_uint<16> row_stage_offset_fh0 = row_stage_offset_A_fh0 + row_stage_offset_B_fh0;

		ap_uint<16> col_off_fh0;
#pragma HLS RESOURCE variable=col_off_fh0 core=MulnS latency=2
		col_off_fh0 = row1_div_fh0 * input_desc.width;

		ap_uint<16> col_fh0 = ddr_pntr_fh0 - col_off_fh0;

		ap_uint<16> ddr_pnt_size_div_fh0 = ddr_pntr_fh0 / input_desc.size;
		ap_uint<16> ddr_pnt_size_mod_fh0 = ddr_pntr_fh0 - (ddr_pnt_size_div_fh0*input_desc.size);
		ap_uint<16> istg_addr_fh0;
		bool eff_plane32_bool_fh0;
		ap_uint<16> height = input_desc.height ,istg_row_count = conv_desc.istg_row_count;
		ap_uint<32> size = input_desc.size;
		ap_uint<16> istg_addr1,istg_addr2;
#pragma HLS RESOURCE variable=istg_addr1 core=MulnS latency=2
		istg_addr1 = (ddr_pnt_size_div_fh0.range(15,1)*size) + ddr_pnt_size_mod_fh0;

		istg_addr2 = col_fh0 + row_stage_offset_fh0;


		if(height == istg_row_count)
		{
			istg_addr_fh0 = istg_addr1;
			//ap_uint<16> plane32_offset_fg0 = ddr_pnt_size_div_fh0*2;
			eff_plane32_bool_fh0 = ddr_pnt_size_div_fh0[0];
		}
		else
		{
			istg_addr_fh0 = istg_addr2;
			eff_plane32_bool_fh0 = plane32_bool_fg0;
		}


#if !XI_INPT_64bit_PORT//8 plane 128bit IO port
		inputtype2 read1_128bit_fh0,read2_128bit_fh0;
		read1_128bit_fh0 = gmem_input_layer_other1_fa0[ddr_pntr_fh0];
		read2_128bit_fh0 = gmem_input_layer_other2_fa0[ddr_pntr_fh0];

		//WriteToIstg(read1_128bit_fh0, read2_128bit_fh0,	istaging_buff0_fb0,	istg_addr_fh0,	eff_plane32_bool_fh0);
		Inputtype_Loop:
		for(ap_uint<8> planes_fh0=0, bit_fh0=0,bit2_fh0=64;planes_fh0<4;planes_fh0++, bit_fh0+=16,bit2_fh0+=16)
		{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL

			short word_fh0 =read1_128bit_fh0.range(bit_fh0+15,bit_fh0);
			short word2_fh0 = read1_128bit_fh0.range(bit2_fh0+15,bit2_fh0);
			if(eff_plane32_bool_fh0 == 0)
			{
				istaging_buff0_fb0[planes_fh0.range(1,0)][0][istg_addr_fh0] = word_fh0;
				istaging_buff0_fb0[planes_fh0.range(1,0)][1][istg_addr_fh0] = word2_fh0;
			}
			else
			{
				istaging_buff0_fb0[planes_fh0.range(1,0)][4][istg_addr_fh0] = word_fh0;
				istaging_buff0_fb0[planes_fh0.range(1,0)][5][istg_addr_fh0] = word2_fh0;
			}

		}//Inputtype_Loop

		Inputtype_Loop2:
		for(ap_uint<8> planes_fh0=0, bit_fh0=0,bit2_fh0=64;planes_fh0<4;planes_fh0++, bit_fh0+=16,bit2_fh0+=16)
		{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL

			short word3_fh0 =read2_128bit_fh0.range(bit_fh0+15,bit_fh0);
			short word4_fh0 = read2_128bit_fh0.range(bit2_fh0+15,bit2_fh0);
			if(eff_plane32_bool_fh0 == 0)
			{
				istaging_buff0_fb0[planes_fh0.range(1,0)][2][istg_addr_fh0] = word3_fh0;
				istaging_buff0_fb0[planes_fh0.range(1,0)][3][istg_addr_fh0] = word4_fh0;
			}
			else
			{
				istaging_buff0_fb0[planes_fh0.range(1,0)][6][istg_addr_fh0] = word3_fh0;
				istaging_buff0_fb0[planes_fh0.range(1,0)][7][istg_addr_fh0] = word4_fh0;
			}

		}//Inputtype_Loop
#else//4 plane 64 bitIO port
		inputtype2 read_64bit_fh0;
		read_64bit_fh0 = gmem_input_layer_other1_fa0[ddr_pntr_fh0];
		Inputtype_Loop:
		for(ap_uint<7> planes_fh1=0, bit_fh1=0;planes_fh1<4;planes_fh1++, bit_fh1+=16)
		{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL
			short word_fh1 = read_64bit_fh0.range(bit_fh1+15,bit_fh1);
			istaging_buff0_fb0[planes_fh1][plane32_fg0][col_fh0+row_stage_offset_fh0] = word_fh1;
		}//Inputtype_Loop
#endif

	}//Input_Layerx_Loop

}
void ReadMean(input_struct input_desc,
		gmem_inputtype_layer1 *gmem_mean_fa0,
		short mean_buff[2][1024],
		short variance_buff[2][1024],
		short beta_buff[2][1024],
		ap_uint<16> loop_cnt,
		ap_uint<4> cnt)
{
#pragma HLS INLINE OFF
#pragma HLS ARRAY_PARTITION variable=mean_buff complete dim=1
#pragma HLS ARRAY_PARTITION variable=variance_buff complete dim=1
#pragma HLS ARRAY_PARTITION variable=beta_buff complete dim=1
#pragma HLS resource variable=mean_buff core=RAM_T2P_BRAM
#pragma HLS resource variable=variance_buff core=RAM_T2P_BRAM
#pragma HLS resource variable=beta_buff core=RAM_T2P_BRAM

	for(ap_uint<16> ddr_ptr=0;ddr_ptr<loop_cnt;ddr_ptr++)
			{
	#pragma HLS PIPELINE
				ap_uint<64> word_64bit = gmem_mean_fa0[ddr_ptr];

	#if 1
				short val_1 = word_64bit.range(15,0);
				short val_2 = word_64bit.range(31,16);
				short val_3 = word_64bit.range(47,32);
				short val_4 =word_64bit.range(63,48);

	#endif
				ap_uint<10> bram_addr=ddr_ptr*2;
				if(cnt==0)
				{
					mean_buff[0][bram_addr] = val_1;//word_64bit.range(15,0);
					mean_buff[1][bram_addr] = val_2;//word_64bit.range(31,16);
					mean_buff[0][bram_addr+1] = val_3;//word_64bit.range(47,32);
					mean_buff[1][bram_addr+1] = val_4;//word_64bit.range(64,48);
				}
				else if(cnt==1)
				{
					variance_buff[0][bram_addr] = val_1;//word_64bit.range(15,0);
					variance_buff[1][bram_addr] = val_2;//word_64bit.range(31,16);
					variance_buff[0][bram_addr+1] = val_3;//word_64bit.range(47,32);
					variance_buff[1][bram_addr+1] = val_4;//word_64bit.range(64,48);
				}

				else if(cnt==2){
					beta_buff[0][bram_addr] = val_1;//word_64bit.range(15,0);
					beta_buff[1][bram_addr] = val_2;//word_64bit.range(31,16);
					beta_buff[0][bram_addr+1] = val_3;//word_64bit.range(47,32);
					beta_buff[1][bram_addr+1] = val_4;//word_64bit.range(64,48);

				}

			}

		}



void LoadMeanBuff(input_struct input_desc,
		gmem_inputtype_layer1 *gmem_mean_fa0,
		short mean_buff[2][1024],
		short variance_buff[2][1024],
		short beta_buff[2][1024])
{
#pragma HLS INLINE OFF
#pragma HLS ARRAY_PARTITION variable=mean_buff complete dim=1
#pragma HLS ARRAY_PARTITION variable=variance_buff complete dim=1
#pragma HLS ARRAY_PARTITION variable=beta_buff complete dim=1
#pragma HLS resource variable=mean_buff core=RAM_T2P_BRAM
#pragma HLS resource variable=variance_buff core=RAM_T2P_BRAM
#pragma HLS resource variable=beta_buff core=RAM_T2P_BRAM

	//fprintf(stderr,"\n inside load mean");
	int offset =0;
	ap_uint<16> loop_cnt = input_desc.planes/4;
	for(ap_uint<4> cnt=0;cnt<3;cnt++)
	{
		ReadMean(input_desc,
				gmem_mean_fa0+offset,
				mean_buff,
				variance_buff,
				beta_buff,
				loop_cnt,
				cnt);

		offset = offset + input_desc.planes/4;
	}


#if !__SYNTHESIS__
	/*
	fprintf(stderr,"\n mean and var buff values\n");
	for(int i=0;i<4;i++)
	{
		fprintf(stderr,"m[0][%d]=%d m[1][%d]=%d v[0][%d]=%d v[1][%d]=%d ",i,mean_buff[0][i],i,mean_buff[1][i],i,variance_buff[0][i],i,variance_buff[1][i]);
	}


	fprintf(stderr,"\n Gamma' values\n");
	for(ap_uint<7> i=0;i<64;i++)
		{
		fprintf(stderr,"\n%d",variance_buff[i[0]][i.range(5,1)]);
		}*/
	/*
	fprintf(stderr,"\n Beta values\n");
	for(ap_uint<7> i=0;i<64;i++)
		{
		fprintf(stderr,"\n%d",beta_buff[i[0]][i.range(5,1)]);
		}
		*/
#endif

}

void WriteNormData(conv_struct conv_desc,
		ap_uint<16> layerx_loop_cnt_fg0,
		input_struct input_desc ,
		ap_uint<3> plane32,
		gmem_inputtype_layer1 *gmem_istg_out1_fa0,
		gmem_inputtype_layer1 *gmem_istg_out2_fa0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<12> startrow_fg0,
		ap_uint<12> endrow_fg0,
		ap_uint<16> input_plane_fg0
)
{
#pragma HLS INLINE OFF


	ap_uint<16> stg_row_cnt = conv_desc.istg_row_count;
	ap_uint<16> width = input_desc.width;


	/*	ap_uint<16> in_plane_width_fh0;
#pragma HLS resource variable=in_plane_width_fh0 core=MulnS latency=2
	in_plane_width_fh0 = (input_plane_fg0.range(15,5))*input_desc.width;

	ap_uint<16> row_stage_offset_A_fh0;
#pragma HLS resource variable=row_stage_offset_A_fh0 core=MulnS latency=2
	row_stage_offset_A_fh0 = in_plane_width_fh0*(conv_desc.istg_row_count);
	 */
	Input_Layerx_Loop:
	for(ap_uint<16> ddr_pntr_fh0=0;ddr_pntr_fh0<layerx_loop_cnt_fg0;ddr_pntr_fh0++)
	{
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 intra false
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 inter false
#pragma HLS LOOP_TRIPCOUNT min=51*479 max=51*479
		/*
		ap_uint<16> row1_div_fh0;
#pragma HLS resource variable=row1_div_fh0 core=DivnS
		row1_div_fh0 = (ddr_pntr_fh0/input_desc.width);
		ap_uint<16> row_fh0 = row1_div_fh0 + startrow_fg0;

		ap_uint<12> num_rows_fh0 = row_fh0 - startrow_fg0;
		ap_uint<16> row_stage_offset_B_fh0;
#pragma HLS RESOURCE variable=row_stage_offset_B_fh0 core=MulnS latency=2
		row_stage_offset_B_fh0=num_rows_fh0*input_desc.width;

		ap_uint<16> row_stage_offset_fh0 = row_stage_offset_A_fh0 + row_stage_offset_B_fh0;

		ap_uint<16> col_off_fh0;
#pragma HLS RESOURCE variable=col_off_fh0 core=MulnS latency=2
		col_off_fh0 = row1_div_fh0 * input_desc.width;

		ap_uint<16> col_fh0 = ddr_pntr_fh0 - col_off_fh0;
		 */



		ap_uint<128> word_128bit;
		ap_uint<4> dim2 = ddr_pntr_fh0[0]+4*plane32;
		ap_uint<16> dim3 = (ddr_pntr_fh0/2)+ (input_plane_fg0.range(15,5)*stg_row_cnt* width);

		for(ap_uint<7> planes=0,bit=0;planes<8;planes++,bit+=16)
		{
#pragma HLS UNROLL
			short word_16bit =istaging_buff0_fb0[planes.range(1,0)][dim2+2*planes[2]][dim3];
			word_128bit.range(bit+15,bit) = word_16bit;
		}

		gmem_istg_out1_fa0[ddr_pntr_fh0] = word_128bit.range(63,0);
		gmem_istg_out2_fa0[ddr_pntr_fh0] = word_128bit.range(127,64);


	}



}


void ReadNormData(conv_struct conv_desc,
		ap_uint<16> layerx_loop_cnt_fg0,
		input_struct input_desc ,
		bool plane32,
		gmem_inputtype_layer1 *gmem_input_layer1_fa0,
		gmem_inputtype_layer1 *gmem_inp_norm_2_fa0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<12> startrow_fg0,
		ap_uint<12> endrow_fg0,
		ap_uint<16> input_plane_fg0,
		short mean[2][8],
		short variance[2][8],
		short beta[2][8],
		bool norm_flags[9][12],
		bool prec_2,
		ap_uint<4> flagset
)
{
#pragma HLS INLINE OFF
	bool ddr_read_en = norm_flags[flagset][0];
	bool beta_sel  = norm_flags[flagset][1];
	bool mean_sel  = norm_flags[flagset][2];
	bool variance_sel  = norm_flags[flagset][3];
	bool relu_en  = norm_flags[flagset][4];
	bool elem_wise_mul_sel = norm_flags[flagset][5];
	bool elem_wise_add_sel = norm_flags[flagset][6];
	bool pwr_sel           = norm_flags[flagset][7];
	bool pwr_0p75           = norm_flags[flagset][8];
	bool mul_in_sel			= norm_flags[flagset][9];
	/*
	ap_uint<16> in_plane_width_fh0;
#pragma HLS resource variable=in_plane_width_fh0 core=MulnS latency=2
	in_plane_width_fh0 = (input_plane_fg0.range(15,5))*input_desc.width;

	ap_uint<16> row_stage_offset_A_fh0;
#pragma HLS resource variable=row_stage_offset_A_fh0 core=MulnS latency=2
	row_stage_offset_A_fh0 = in_plane_width_fh0*(conv_desc.istg_row_count);
	 */
	 
	 	short val_reg[2][4];
#pragma HLS ARRAY_PARTITION variable=val_reg complete dim=0


	static ap_uint<16> LUT_0p75[1024] = {0,46340,27554,20329,16384,13859,12087,10768,9741,8918,8240,7672,7187,6768,6402,6079,5792,5535,5302,5092,4899,4723,4561,4412,4273,4144,4024,3912,3807,3708,3615,3527,3444,3365,3291,3220,3153,3088,3027,2969,2913,2860,2808,2759,2712,2667,2623,2581,2541,2502,2464,2428,2393,2359,2326,2294,2263,2233,2204,2176,2149,2123,2097,2072,2048,2024,2001,1978,1956,1935,1914,1894,1874,1855,1836,1818,1800,1782,1765,1748,1732,1716,1700,1685,1670,1655,1640,1626,1612,1599,1585,1572,1559,1547,1535,1522,1510,1499,1487,1476,
			1465,1454,1443,1433,1422,1412,1402,1392,1383,1373,1364,1355,1346,1337,1328,1319,1311,1302,1294,1286,1278,1270,1262,1254,1247,1239,1232,1224,1217,1210,1203,1196,1189,1183,1176,1170,1163,1157,1150,1144,1138,1132,1126,1120,1114,1109,1103,1097,1092,1086,1081,1075,1070,1065,1060,1054,1049,1044,1039,1034,1030,1025,1020,1015,1011,1006,1002,997,993,988,984,979,975,971,967,963,959,954,950,946,942,939,935,931,927,923,920,916,912,909,905,901,898,894,891,888,884,881,877,874,
			871,868,864,861,858,855,852,849,846,843,840,837,834,831,828,825,822,819,816,814,811,808,805,803,800,797,795,792,789,787,784,782,779,777,774,772,769,767,764,762,759,757,755,752,750,748,746,743,741,739,737,734,732,730,728,726,724,721,719,717,715,713,711,709,707,705,703,701,699,697,695,693,691,689,688,686,684,682,680,678,677,675,673,671,669,668,666,664,662,661,659,657,656,654,652,651,649,647,646,644,
			642,641,639,638,636,634,633,631,630,628,627,625,624,622,621,619,618,616,615,613,612,611,609,608,606,605,604,602,601,599,598,597,595,594,593,591,590,589,587,586,585,583,582,581,580,578,577,576,575,573,572,571,570,569,567,566,565,564,563,561,560,559,558,557,556,554,553,552,551,550,549,548,547,545,544,543,542,541,540,539,538,537,536,535,534,533,532,531,530,529,528,527,526,525,524,523,522,521,520,519,
			518,517,516,515,514,513,512,511,510,509,508,507,506,505,504,503,503,502,501,500,499,498,497,496,495,495,494,493,492,491,490,489,489,488,487,486,485,484,484,483,482,481,480,479,479,478,477,476,475,475,474,473,472,471,471,470,469,468,468,467,466,465,465,464,463,462,462,461,460,459,459,458,457,456,456,455,454,454,453,452,451,451,450,449,449,448,447,447,446,445,444,444,443,442,442,441,440,440,439,438,
			438,437,436,436,435,435,434,433,433,432,431,431,430,429,429,428,428,427,426,426,425,424,424,423,423,422,421,421,420,420,419,418,418,417,417,416,415,415,414,414,413,413,412,411,411,410,410,409,409,408,408,407,406,406,405,405,404,404,403,403,402,402,401,400,400,399,399,398,398,397,397,396,396,395,395,394,394,393,393,392,392,391,391,390,390,389,389,388,388,387,387,386,386,385,385,384,384,383,383,382,
			382,381,381,380,380,379,379,378,378,378,377,377,376,376,375,375,374,374,373,373,372,372,372,371,371,370,370,369,369,368,368,368,367,367,366,366,365,365,365,364,364,363,363,362,362,362,361,361,360,360,359,359,359,358,358,357,357,357,356,356,355,355,355,354,354,353,353,353,352,352,351,351,351,350,350,349,349,349,348,348,348,347,347,346,346,346,345,345,344,344,344,343,343,343,342,342,341,341,341,340,
			340,340,339,339,339,338,338,337,337,337,336,336,336,335,335,335,334,334,334,333,333,333,332,332,332,331,331,330,330,330,329,329,329,328,328,328,327,327,327,326,326,326,325,325,325,324,324,324,323,323,323,323,322,322,322,321,321,321,320,320,320,319,319,319,318,318,318,317,317,317,317,316,316,316,315,315,315,314,314,314,313,313,313,313,312,312,312,311,311,311,310,310,310,310,309,309,309,308,308,308,
			308,307,307,307,306,306,306,306,305,305,305,304,304,304,304,303,303,303,302,302,302,302,301,301,301,301,300,300,300,299,299,299,299,298,298,298,298,297,297,297,296,296,296,296,295,295,295,295,294,294,294,294,293,293,293,293,292,292,292,292,291,291,291,291,290,290,290,290,289,289,289,289,288,288,288,288,287,287,287,287,286,286,286,286,285,285,285,285,284,284,284,284,283,283,283,283,282,282,282,282,
			282,281,281,281,281,280,280,280,280,279,279,279,279,279,278,278,278,278,277,277,277,277,276,276,276,276,276,275,275,275,275,274,274,274,274,274,273,273,273,273,272,272,272,272,272,271,271,271,271,271,270,270,270,270,269,269,269,269,269,268,268,268,268,268,267,267,267,267,267,266,266,266,266,265,265,265,265,265,264,264,264,264,264,263,263,263,263,263,262,262,262,262,262,261,261,261,261,261,260,260,
			260,260,260,260,259,259,259,259,259,258,258,258,258,258,257,257,257,257,257,256,256,256,256,256},LUT_0p5[1024] = {1,3,4,5,6,8,1};

#pragma HLS resource variable=LUT_0p75 core=ROM_nP_BRAM
#pragma HLS resource variable=LUT_0p5 core=ROM_nP_BRAM

	ap_uint<16> stg_row_cnt = conv_desc.istg_row_count;
	ap_uint<16> width = input_desc.width;

	Input_Layerx_Loop:
	for(ap_uint<16> ddr_pntr_fh0=0;ddr_pntr_fh0<layerx_loop_cnt_fg0;ddr_pntr_fh0++)
	{

#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 intra false
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 inter false
#pragma HLS LOOP_TRIPCOUNT min=51*479 max=51*479



		/*		ap_uint<16> row1_div_fh0;
#pragma HLS resource variable=row1_div_fh0 core=DivnS
		row1_div_fh0 = (ddr_pntr_fh0/input_desc.width);
		ap_uint<16> row_fh0 = row1_div_fh0 + startrow_fg0;

		ap_uint<12> num_rows_fh0 = row_fh0 - startrow_fg0;
		ap_uint<16> row_stage_offset_B_fh0;
#pragma HLS RESOURCE variable=row_stage_offset_B_fh0 core=MulnS latency=2
		row_stage_offset_B_fh0=num_rows_fh0*input_desc.width;

		ap_uint<16> row_stage_offset_fh0 = row_stage_offset_A_fh0 + row_stage_offset_B_fh0;

		ap_uint<16> col_off_fh0;
#pragma HLS RESOURCE variable=col_off_fh0 core=MulnS latency=2
		col_off_fh0 = row1_div_fh0 * input_desc.width;

		ap_uint<16> col_fh0 = ddr_pntr_fh0 - col_off_fh0;
		 */
		ap_uint<128> word_128bit,word_128bit_istg,word_128bit_ddr;
		ap_uint<64> word1_64bit_ddr=0,word2_64bit_ddr=0;
		ap_int<16> shift_res;
		short add_sub_op2,add_sub_res,mul_op,word_relu;
		ap_int<32> add_sub_op3;
		ap_int<32> mul_res,mul_add_res;
		ap_uint<4> dim2 = ddr_pntr_fh0[0]+4*plane32;
		ap_uint<4> dim2_plus_2 = dim2+2;
		ap_uint<16> dim3 = (ddr_pntr_fh0/2)+ (input_plane_fg0.range(15,5)*stg_row_cnt* width);
		ap_int<16> res_pwr,pwr_out;
		//ap_int<16> beta = conv_desc.norm_k;
		ap_uint<16> mul_in = conv_desc.mul_in;
		ap_uint<4> norm_prec;
		if(!prec_2)
			norm_prec = conv_desc.norm_prec;
		else
			norm_prec =conv_desc.norm_prec_2;

		ap_uint<4> norm_prec_3 =conv_desc.norm_prec_3;

		//if(ddr_read_en)
		word1_64bit_ddr = gmem_input_layer1_fa0[ddr_pntr_fh0];
		word2_64bit_ddr = gmem_inp_norm_2_fa0[ddr_pntr_fh0];

		word_128bit_ddr.range(63,0) = word1_64bit_ddr;
		word_128bit_ddr.range(127,64) = word2_64bit_ddr;

#if !__SYNTHESIS__
		short val1 = word1_64bit_ddr.range(15,0);
		short val2 = word1_64bit_ddr.range(31,16);
		short val3 = word1_64bit_ddr.range(47,32);
		short val4 = word1_64bit_ddr.range(63,48);
		short val5 = word2_64bit_ddr.range(15,0);
		short val6 = word2_64bit_ddr.range(31,16);
		short val7 = word2_64bit_ddr.range(47,32);
		short val8 = word2_64bit_ddr.range(63,48);
		short val_128_1 = word_128bit.range(15,0);
		short val_128_2 = word_128bit.range(31,16);

#endif
		for(ap_uint<7> planes=0,bit=0;planes<8;planes++,bit+=16)
		{
#pragma HLS UNROLL
			word_128bit_istg.range(bit+15,bit) = istaging_buff0_fb0[planes.range(1,0)][dim2+2*planes[2]][dim3];
			//fprintf(stderr,"\n dim1 = %d dim2 = %d dim3 = %d ",(int)planes.range(1,0),(int)(dim2+2*planes[2]),(int)dim3);
		}

		if(ddr_read_en)
			word_128bit = word_128bit_ddr;
		else
			word_128bit = word_128bit_istg;
		//ap_uint<14> depth_off = (input_plane_fg0/2);

		ap_uint<3> depth_off = ddr_pntr_fh0[0]*2;
		for(ap_uint<7> planes=0,bit=0;planes<8;planes++,bit+=16)
		{
#pragma HLS UNROLL
			ap_uint<4> istg_dim2 = dim2+2*planes[2];
			short word = word_128bit.range(bit+15,bit);


			short word_istg = word_128bit_istg.range(bit+15,bit);

			//fprintf(stderr,"\n dim1 = %d dim2 = %d ",(int)planes[0],(int)(depth_off+(planes/2)+(2*planes[2])));

			if(mean_sel)
				add_sub_op2 = -1*mean[planes[0]][depth_off+(planes/2)+(2*planes[2])];
			else if(elem_wise_add_sel)
				add_sub_op2 =word_istg;
			else
				add_sub_op2 = 0;

			//add_sub_res = word+add_sub_op2;


			if(variance_sel)
				mul_op = variance[planes[0]][depth_off+(planes/2)+(2*planes[2])];
			else if(elem_wise_mul_sel)
				mul_op = word_istg;
			else if(mul_in_sel)
				mul_op = mul_in;
			else
				mul_op=1;
#pragma HLS resource variable = mul_res core =DSP48
			mul_res = (word+add_sub_op2) * mul_op;

#if 0//!__SYNTHESIS__
		if(ddr_pntr_fh0 ==0)
		fprintf(stderr,"\n**word = %d mean=%d gamma = %d scale= %d\n",(int)word,(int)add_sub_op2,(int)mul_op,(int)mul_res);
#endif

			if(beta_sel)
				add_sub_op3 = (beta[planes[0]][depth_off+(planes/2)+(2*planes[2])])<<norm_prec_3;//15;
			else
				add_sub_op3 = 0;

			mul_add_res = mul_res + add_sub_op3;

			shift_res = mul_add_res >> norm_prec;
#if !__SYNTHESIS__
		if(shift_res == 13647)
			int k=0;
#endif
			ap_uint<10> addr_pwr = shift_res;

			ap_int<16> power_0p75 = LUT_0p75[addr_pwr];
			ap_int<16> power_0p5 = LUT_0p5[addr_pwr];

			if(pwr_0p75)
				pwr_out = power_0p75;
			else
				pwr_out = power_0p5;

			if(pwr_sel)
				res_pwr = pwr_out;
			else
				res_pwr = shift_res;




			if(relu_en && shift_res[15]==1)
				word_relu = 0;
			else
				word_relu = res_pwr;
			//fprintf(stderr,"\n dim1 = %d dim2 = %d dim3 = %d ",(int)planes.range(1,0),(int)(dim2+2*planes[2]),(int)dim3);
			//istaging_buff0_fb0[planes.range(1,0)][istg_dim2][dim3] = word_relu;
			val_reg[planes[2]][planes.range(1,0)] = word_relu;
		}
		
		for(ap_uint<4> t_dim1=0;t_dim1<4;t_dim1++)
		{
#pragma HLS UNROLL
			for(ap_uint<4> t_dim2=0;t_dim2<8;t_dim2++)
			{
#pragma HLS UNROLL
				if(t_dim2 == dim2)
					istaging_buff0_fb0[t_dim1][t_dim2][dim3] = val_reg[0][t_dim1];
				else if(t_dim2 == dim2_plus_2)
					istaging_buff0_fb0[t_dim1][t_dim2][dim3] = val_reg[1][t_dim1];

			}
		}


	}



}
void Normalization(conv_struct conv_desc,
		ap_uint<16> layerx_loop_cnt_fg0,
		input_struct input_desc ,
		gmem_inputtype_layer1 *gmem_input_layer1_fa0,
		gmem_inputtype_layer1 *gmem_inp_norm_2_fa0,
		gmem_biastype *gmem_bias_fa0,
		gmem_inputtype_layer1 	*gmem_inp_norm_3_fa0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<12> startrow_fg0,
		ap_uint<12> endrow_fg0,
		short mean_buff[2][1024],
		short variance_buff[2][1024],
		short beta_buff[2][1024],
		ap_uint<32> start_off_A_fg0,
		bool norm_flags[9][12],
		ap_uint<4> flagset
)
{
#pragma HLS INLINE OFF

	bool read_2nd_ptr = norm_flags[flagset][11];
	bool norm_write_en = norm_flags[flagset][10];

	bool prec_2 = read_2nd_ptr;
	ap_uint<32> start_off;// =start_off_A_fg0*2;
	for(ap_uint<16> imgG=0,bram_addr=0;imgG<input_desc.planes;imgG+=16,bram_addr+=8)
	{
#pragma HLS LOOP_TRIPCOUNT min=0 max=0

		//read mean and variance corresponding to 16 planes
		short mean[2][8],variance[2][8],beta[2][8];
#pragma HLS ARRAY_PARTITION variable=mean complete dim=0
#pragma HLS ARRAY_PARTITION variable=variance complete dim=0
#pragma HLS ARRAY_PARTITION variable=beta complete dim=0

		for(ap_uint<3> dim1=0;dim1<2;dim1++)
		{
#pragma HLS UNROLL
			for(ap_uint<5> cnt=0;cnt<8;cnt++)
			{
#pragma HLS UNROLL
				mean[dim1][cnt] = 	mean_buff[dim1][bram_addr+cnt];
				variance[dim1][cnt] = variance_buff[dim1][bram_addr+cnt];
				beta[dim1][cnt] = beta_buff[dim1][bram_addr+cnt];
			}

		}
		//ap_uint<2> plane32 = imgG.range(4,3);
		bool plane32 = imgG[4];
	//	ap_uint<16> imgGby8;
		//imgGby8 = imgG.range(15,3);

		ap_uint<16> imgGby16;
		imgGby16 = imgG.range(15,4);

		ap_uint<32> offset_l1;

		offset_l1 = (imgGby16)*input_desc.size;

		//if(imgG%16==0)
			start_off = start_off_A_fg0*2 + offset_l1*2;

		//		fprintf(stderr,"\n start_off = %d start_off_A_fg0*2 = %d ", (int)start_off , (int)start_off_A_fg0*2);


		//if(imgG[3]==0)
		{
			if(!read_2nd_ptr)
				ReadNormData(conv_desc, 2*layerx_loop_cnt_fg0, input_desc, plane32, gmem_input_layer1_fa0 + start_off,gmem_inp_norm_2_fa0 + start_off, istaging_buff0_fb0, startrow_fg0, endrow_fg0, imgG,mean,variance,beta,norm_flags,prec_2,flagset);
			else
				ReadNormData(conv_desc, 2*layerx_loop_cnt_fg0, input_desc, plane32, gmem_bias_fa0 + start_off, gmem_inp_norm_3_fa0 + start_off, istaging_buff0_fb0, startrow_fg0, endrow_fg0, imgG,mean,variance,beta,norm_flags,prec_2,flagset);
		}
	/*	else
		{
			if(!read_2nd_ptr)
				ReadNormData(conv_desc, 2*layerx_loop_cnt_fg0, input_desc, plane32, gmem_inp_norm_2_fa0 + start_off, istaging_buff0_fb0, startrow_fg0, endrow_fg0, imgG,mean,variance,beta,norm_flags,prec_2,flagset);
			else
				ReadNormData(conv_desc, 2*layerx_loop_cnt_fg0, input_desc, plane32, gmem_inp_norm_3_fa0 + start_off, istaging_buff0_fb0, startrow_fg0, endrow_fg0, imgG,mean,variance,beta,norm_flags,prec_2,flagset);
		}*/
#if 0
		fprintf(stderr,"\n values inside normalization \n");
		fprintf(stderr,"\n layerx_loop_cnt_fg0 = %d  plane32 = %d start_off = %d startrow_fg0 = %d endrow_fg0 = %d imgG = %d",(int)layerx_loop_cnt_fg0,(int)plane32,(int)start_off,(int)startrow_fg0,(int)endrow_fg0,(int)imgG);
#endif
		/*
		if(norm_write_en)
		{
			if(imgG[3]==0)
				WriteNormData(conv_desc, 2*layerx_loop_cnt_fg0, input_desc,plane32,gmem_istg_out_1_fa0 + start_off,istaging_buff0_fb0,startrow_fg0,endrow_fg0,imgG);
			else
				WriteNormData(conv_desc, 2*layerx_loop_cnt_fg0, input_desc,plane32,gmem_istg_out_2_fa0 + start_off,istaging_buff0_fb0,startrow_fg0,endrow_fg0,imgG);
		}
		*/
	}
#if 0//!__SYNTHESIS__
	fprintf(stderr,"\n istage values \n");
	for(int k=0;k<2;k++)
	{
		for(int i=0;i<4;i++)
		{
			fprintf(stderr,"\n");
			for(int j=0;j<16;j++)
			{
				fprintf(stderr,"istg[%d][%d][%d] = %d ",i,k,j,istaging_buff0_fb0[i][k][j]);
			}
		}
	}
#endif

}


template<int IN_WW,int IN_HH,int IINPUT_PLANES>
void LoadIStagingBuff_fg(input_struct input_desc,
		conv_struct conv_desc,
		gmem_inputtype_layer1 *gmem_input_layer1_fa0,
		gmem_inputtype_layerx *gmem_input_layer_other1_fa0,
		gmem_inputtype_layerx *gmem_input_layer_other2_fa0,
		gmem_biastype *gmem_bias_fa0,
		gmem_inputtype_layer1 *gmem_inp_norm_2_fa0,
		gmem_inputtype_layer1 	*gmem_inp_norm_3_fa0,
		gmem_inputtype_layer1 *gmem_istg_out_1_fa0,
		gmem_inputtype_layer1 *gmem_istg_out_2_fa0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		ap_int<12> startrow_in_fb0,
		ap_int<12> endrow_in_fb0,
		ap_int<12> norm_startrow_in_fb0,
		ap_int<12> norm_endrow_in_fb0,
		short mean_buff[2][1024],
		short variance_buff[2][1024],
		short beta_buff[2][1024])
{
#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=2
#pragma HLS resource variable=istaging_buff0_fb0 core=RAM_T2P_BRAM latency=2
#pragma HLS INLINE OFF

	//short mean_buff[2][1024];
	//short variance_buff[2][1024];
	//short beta_buff[2][1024];
#pragma HLS ARRAY_PARTITION variable=mean_buff complete dim=1
#pragma HLS ARRAY_PARTITION variable=variance_buff complete dim=1
#pragma HLS ARRAY_PARTITION variable=beta_buff complete dim=1
#pragma HLS resource variable=mean_buff core=RAM_T2P_BRAM
#pragma HLS resource variable=variance_buff core=RAM_T2P_BRAM
#pragma HLS resource variable=beta_buff core=RAM_T2P_BRAM

	ap_uint<12> startrow_fg0,endrow_fg0;

	if(startrow_in_fb0<0)
		startrow_fg0 = 0;
	else
		startrow_fg0 = startrow_in_fb0;



	if(endrow_in_fb0<input_desc.height)
		endrow_fg0 = endrow_in_fb0;
	else
		endrow_fg0 = input_desc.height-1;


#if 0//!__SYNTHESIS__
fprintf(stderr,"\n*****start_row = %d   end_row = %d",(int)startrow_fg0,(int)endrow_fg0);
#endif


	ap_uint<32> input_hw_fg0;
#pragma HLS RESOURCE variable=input_hw_fg0 core=MulnS latency=2
	input_hw_fg0 = input_desc.size;

	ap_uint<12> num_rows_fg0 = endrow_fg0-startrow_fg0 +1;

	ap_uint<32> start_off_A_fg0;
#pragma HLS RESOURCE variable=start_off_A_fg0 core=MulnS latency=2
	start_off_A_fg0 = startrow_fg0*input_desc.width;
	ap_uint<32> start_off_l1_fg0 ;
	//#if XI_MEAN_SUB
	if(input_desc.mean_sub_flag)
	{
		start_off_l1_fg0 = (startrow_fg0*input_desc.width)/2;
	}
	else
	{
		//#else
		start_off_l1_fg0 = startrow_fg0*input_desc.width;
	}
	//#endif

	ap_uint<10> in_plane_align16 = input_desc.planes.range(15,4) + input_desc.planes[3];


	ap_uint<16> layer1_loop_cnt_fg0 = num_rows_fg0 * input_desc.width;
	ap_uint<16> layerx_loop_cnt_fg0;

#if !XI_INPT_64bit_PORT//##8 planes
	ap_uint<16> layerx_img_fg0;
	if((input_desc.height == conv_desc.istg_row_count) && (!(conv_desc.bn_snb_enable || conv_desc.elem_wise_add_en || conv_desc.bn_conv_fuse_en)))
	{
		layerx_img_fg0 = 16;
		layerx_loop_cnt_fg0 = input_desc.size * in_plane_align16;
	}
	else
	{
		layerx_img_fg0 = input_desc.planes;
		layerx_loop_cnt_fg0 = num_rows_fg0 * input_desc.width;
	}
#endif

	if(conv_desc.lrn_inter_sos_enable || conv_desc.lrn_pns_enable || conv_desc.bn_snb_enable || conv_desc.elem_wise_add_en || conv_desc.bn_conv_fuse_en)

	{
		bool norm_flags[9][12];
#pragma HLS ARRAY_PARTITION variable=norm_flags complete dim=0
		//bool norm_flags[10];
		ap_uint<8> mode = conv_desc.opcode;
		//bool write_en;

					//LRN SOS
					norm_flags[0][0] = 1; //ddr_en
					norm_flags[0][1] = 0; //beta_sel
					norm_flags[0][2] = 0; //mean_sel
					norm_flags[0][3] = 0; //var_sel
					norm_flags[0][4] = 0; //relu
					norm_flags[0][5] = 0; //elem_wise_mul
					norm_flags[0][6] = 0; //elem_wise_add
					norm_flags[0][7] = 0; //pwr_sel
					norm_flags[0][8] = 0; //pwr_0p75
					norm_flags[0][9] = 1; //mul_in_sel
					norm_flags[0][10] = 1; //write_en
					norm_flags[0][11] = 0;
					//LRN POWER AND SCALE
					norm_flags[1][0] = 1; //ddr_en
					norm_flags[1][1] = 1; //beta_sel
					norm_flags[1][2] = 0; //mean_sel
					norm_flags[1][3] = 0; //var_sel
					norm_flags[1][4] = 0; //relu
					norm_flags[1][5] = 0; //elem_wise_mul
					norm_flags[1][6] = 0; //elem_wise_add
					norm_flags[1][7] = 1; //pwr_sel
					norm_flags[1][8] = 1; //pwr_0p75
					norm_flags[1][9] = 1; //mul_in_sel
					norm_flags[1][10] = 1;
					norm_flags[1][11] = 0;

					norm_flags[2][0] = 1; //ddr_en
					norm_flags[2][1] = 0; //beta_sel
					norm_flags[2][2] = 0; //mean_sel
					norm_flags[2][3] = 0; //var_sel
					norm_flags[2][4] = 0; //relu
					norm_flags[2][5] = 1; //elem_wise_mul
					norm_flags[2][6] = 0; //elem_wise_add
					norm_flags[2][7] = 0; //pwr_sel
					norm_flags[2][8] = 0; //pwr_0p75
					norm_flags[2][9] = 0; //mul_in_sel
					norm_flags[2][10] = 1;
					norm_flags[2][11] = 1;
					// BN Scale and Bias
				/*	norm_flags[3][0] = 1; //ddr_en
					norm_flags[3][1] = 0; //beta_sel
					norm_flags[3][2] = 1; //mean_sel
					norm_flags[3][3] = 1; //var_sel
					norm_flags[3][4] = 0; //relu
					norm_flags[3][5] = 0; //elem_wise_mul
					norm_flags[3][6] = 0; //elem_wise_add
					norm_flags[3][7] = 0; //pwr_sel
					norm_flags[3][8] = 0; //pwr_0p75
					norm_flags[3][9] = 0; //mul_in_sel
					norm_flags[3][10] = 0;
					norm_flags[3][11] = 0;

					norm_flags[4][0] = 0; //ddr_en
					norm_flags[4][1] = 1; //beta_sel
					norm_flags[4][2] = 0; //mean_sel
					norm_flags[4][3] = 0; //var_sel
					norm_flags[4][4] = 0; //relu
					norm_flags[4][5] = 0; //elem_wise_mul
					norm_flags[4][6] = 0; //elem_wise_add
					norm_flags[4][7] = 0; //pwr_sel
					norm_flags[4][8] = 0; //pwr_0p75
					norm_flags[4][9] = 1; //mul_in_sel
					norm_flags[4][10] = 1;
					norm_flags[4][11] = 0;
*/

					norm_flags[3][0] = 1; //ddr_en
					norm_flags[3][1] = 1; //beta_sel
					norm_flags[3][2] = 1; //mean_sel
					norm_flags[3][3] = 1; //var_sel
					norm_flags[3][4] = 1;//;//0; //relu
					norm_flags[3][5] = 0; //elem_wise_mul
					norm_flags[3][6] = 0; //elem_wise_add
					norm_flags[3][7] = 0; //pwr_sel
					norm_flags[3][8] = 0; //pwr_0p75
					norm_flags[3][9] = 0; //mul_in_sel
					norm_flags[3][10] = 1; //write_en
					norm_flags[3][11] = 0; //read_2nd_ptr
					//elem-wise add
					norm_flags[5][0] = 1; //ddr_en
					norm_flags[5][1] = 0; //beta_sel
					norm_flags[5][2] = 0; //mean_sel
					norm_flags[5][3] = 0; //var_sel
					norm_flags[5][4] = 0; //relu
					norm_flags[5][5] = 0; //elem_wise_mul
					norm_flags[5][6] = 0; //elem_wise_add
					norm_flags[5][7] = 0; //pwr_sel
					norm_flags[5][8] = 0; //pwr_0p75
					norm_flags[5][9] = 0; //mul_in_sel
					norm_flags[5][10] = 0; //write_en
					norm_flags[5][11] = 0; //read_2nd_ptr

					norm_flags[6][0] = 1; //ddr_en
					norm_flags[6][1] = 0; //beta_sel
					norm_flags[6][2] = 0; //mean_sel
					norm_flags[6][3] = 0; //var_sel
					norm_flags[6][4] = 0; //relu
					norm_flags[6][5] = 0; //elem_wise_mul
					norm_flags[6][6] = 1; //elem_wise_add
					norm_flags[6][7] = 0; //pwr_sel
					norm_flags[6][8] = 0; //pwr_0p75
					norm_flags[6][9] = 0; //mul_in_sel
					norm_flags[6][10] =1; //write_en
					norm_flags[6][11] = 1; //read_2_ptr
					// BN + Conv Fuse
					norm_flags[7][0] = 1; //ddr_en
					norm_flags[7][1] = 1; //beta_sel
					norm_flags[7][2] = 1; //mean_sel
					norm_flags[7][3] = 1; //var_sel
					norm_flags[7][4] = 1;//;//0; //relu
					norm_flags[7][5] = 0; //elem_wise_mul
					norm_flags[7][6] = 0; //elem_wise_add
					norm_flags[7][7] = 0; //pwr_sel
					norm_flags[7][8] = 0; //pwr_0p75
					norm_flags[7][9] = 0; //mul_in_sel
					norm_flags[7][10] = 0; //write_en
					norm_flags[7][11] = 0; //read_2nd_ptr


					//default
					norm_flags[8][0] = 1; //ddr_en
					norm_flags[8][1] = 1; //beta_sel
					norm_flags[8][2] = 1; //mean_sel
					norm_flags[8][3] = 1; //var_sel
					norm_flags[8][4] = 1; //relu
					norm_flags[8][5] = 1; //elem_wise_mul
					norm_flags[8][6] = 1; //elem_wise_add
					norm_flags[8][7] = 1; //pwr_sel
					norm_flags[8][8] = 1; //pwr_0p75
					norm_flags[8][9] = 1; //mul_in_sel
					norm_flags[8][10] = 0;



					if(startrow_fg0==0 && (conv_desc.bn_snb_enable || conv_desc.bn_conv_fuse_en))
						LoadMeanBuff(input_desc, gmem_inp_norm_3_fa0, mean_buff, variance_buff, beta_buff);
					for(ap_uint<4> loop_cnt=0;loop_cnt<conv_desc.norm_loop_cnt;loop_cnt++)
					{
						ap_uint<4> flagset = conv_desc.norm_flag_set[loop_cnt];
					Normalization(conv_desc, layerx_loop_cnt_fg0, input_desc,gmem_input_layer1_fa0,gmem_inp_norm_2_fa0,gmem_bias_fa0,gmem_inp_norm_3_fa0,
							istaging_buff0_fb0,
							startrow_fg0,
							endrow_fg0,
							mean_buff,
							variance_buff,
							beta_buff,
							start_off_A_fg0,
							norm_flags,
							flagset);

					}

/*
		if(conv_desc.lrn_inter_sos_enable) //LRN SOS
		{



			Normalization(conv_desc, layerx_loop_cnt_fg0, input_desc,gmem_input_layer1_fa0,gmem_inp_norm_2_fa0,gmem_bias_fa0,gmem_inp_norm_3_fa0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0,
					istaging_buff0_fb0,
					startrow_fg0,
					endrow_fg0,
					mean_buff,
					variance_buff,
					beta_buff,
					start_off_A_fg0,
					norm_flags,
					write_en,
					0);//need to generate cnrtl signal
		}//LRN SOS

		else if(mode ==9) //LRN POWER AND SCALE
		{

			norm_flags[0] = 1; //ddr_en
			norm_flags[1] = 1; //beta_sel
			norm_flags[2] = 0; //mean_sel
			norm_flags[3] = 0; //var_sel
			norm_flags[4] = 0; //relu
			norm_flags[5] = 0; //elem_wise_mul
			norm_flags[6] = 0; //elem_wise_add
			norm_flags[7] = 1; //pwr_sel
			norm_flags[8] = 1; //pwr_0p75
			norm_flags[9] = 1; //mul_in_sel
			write_en =1;

			Normalization(conv_desc, layerx_loop_cnt_fg0, input_desc,gmem_input_layer1_fa0,gmem_inp_norm_2_fa0,gmem_bias_fa0,gmem_inp_norm_3_fa0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0,
					istaging_buff0_fb0,
					startrow_fg0,
					endrow_fg0,
					mean_buff,
					variance_buff,
					beta_buff,
					start_off_A_fg0,
					norm_flags,
					write_en,
					0);//need to generate cnrtl signal


			norm_flags[0] = 1; //ddr_en
			norm_flags[1] = 0; //beta_sel
			norm_flags[2] = 0; //mean_sel
			norm_flags[3] = 0; //var_sel
			norm_flags[4] = 0; //relu
			norm_flags[5] = 1; //elem_wise_mul
			norm_flags[6] = 0; //elem_wise_add
			norm_flags[7] = 0; //pwr_sel
			norm_flags[8] = 0; //pwr_0p75
			norm_flags[9] = 0; //mul_in_sel
			write_en=1;


			Normalization(conv_desc, layerx_loop_cnt_fg0, input_desc,gmem_input_layer1_fa0,gmem_inp_norm_2_fa0,gmem_bias_fa0,gmem_inp_norm_3_fa0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0,
					istaging_buff0_fb0,
					startrow_fg0,
					endrow_fg0,
					mean_buff,
					variance_buff,
					beta_buff,
					start_off_A_fg0,
					norm_flags,
					write_en,
					1);

		}//LRN POWER AND SCALE

		else if(conv_desc.bn_snb_enable) // BN Scale and Bias
		{

			if(startrow_in_fb0==0)
			LoadMeanBuff(input_desc, gmem_bias_fa0, mean_buff, variance_buff, beta_buff);

			norm_flags[0] = 1; //ddr_en
			norm_flags[1] = 0; //beta_sel
			norm_flags[2] = 1; //mean_sel
			norm_flags[3] = 1; //var_sel
			norm_flags[4] = 0; //relu
			norm_flags[5] = 0; //elem_wise_mul
			norm_flags[6] = 0; //elem_wise_add
			norm_flags[7] = 0; //pwr_sel
			norm_flags[8] = 0; //pwr_0p75
			norm_flags[9] = 0; //mul_in_sel
			write_en =0;//1;

			Normalization(conv_desc, layerx_loop_cnt_fg0, input_desc,gmem_input_layer1_fa0,gmem_inp_norm_2_fa0,gmem_bias_fa0,gmem_inp_norm_3_fa0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0,
					istaging_buff0_fb0,
					startrow_fg0,
					endrow_fg0,
					mean_buff,
					variance_buff,
					beta_buff,
					start_off_A_fg0,
					norm_flags,
					write_en,
					0);//need to generate cnrtl signal

						norm_flags[0] = 0; //ddr_en
						norm_flags[1] = 1; //beta_sel
						norm_flags[2] = 0; //mean_sel
						norm_flags[3] = 0; //var_sel
						norm_flags[4] = 0; //relu
						norm_flags[5] = 0; //elem_wise_mul
						norm_flags[6] = 0; //elem_wise_add
						norm_flags[7] = 0; //pwr_sel
						norm_flags[8] = 0; //pwr_0p75
						norm_flags[9] = 1; //mul_in_sel
						write_en =1;

						Normalization(conv_desc, layerx_loop_cnt_fg0, input_desc,gmem_input_layer1_fa0,gmem_inp_norm_2_fa0,gmem_bias_fa0,gmem_inp_norm_3_fa0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0,
								istaging_buff0_fb0,
								startrow_fg0,
								endrow_fg0,
								mean_buff,
								variance_buff,
								beta_buff,
								start_off_A_fg0,
								norm_flags,
								write_en,
								0);//need to generate cnrtl signal


		}

		else if(conv_desc.elem_wise_add_en) // Elem-wise Add
		{

			norm_flags[0] = 1; //ddr_en
			norm_flags[1] = 0; //beta_sel
			norm_flags[2] = 0; //mean_sel
			norm_flags[3] = 0; //var_sel
			norm_flags[4] = 0; //relu
			norm_flags[5] = 0; //elem_wise_mul
			norm_flags[6] = 0; //elem_wise_add
			norm_flags[7] = 0; //pwr_sel
			norm_flags[8] = 0; //pwr_0p75
			norm_flags[9] = 0; //mul_in_sel
			write_en =0;//1;

			Normalization(conv_desc, layerx_loop_cnt_fg0, input_desc,gmem_input_layer1_fa0,gmem_inp_norm_2_fa0,gmem_bias_fa0,gmem_inp_norm_3_fa0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0,
					istaging_buff0_fb0,
					startrow_fg0,
					endrow_fg0,
					mean_buff,
					variance_buff,
					beta_buff,
					start_off_A_fg0,
					norm_flags,
					write_en,
					0);//need to generate cnrtl signal

						norm_flags[0] = 1; //ddr_en
						norm_flags[1] = 0; //beta_sel
						norm_flags[2] = 0; //mean_sel
						norm_flags[3] = 0; //var_sel
						norm_flags[4] = 0; //relu
						norm_flags[5] = 0; //elem_wise_mul
						norm_flags[6] = 1; //elem_wise_add
						norm_flags[7] = 0; //pwr_sel
						norm_flags[8] = 0; //pwr_0p75
						norm_flags[9] = 1; //mul_in_sel
						write_en =1;

						Normalization(conv_desc, layerx_loop_cnt_fg0, input_desc,gmem_input_layer1_fa0,gmem_inp_norm_2_fa0,gmem_bias_fa0,gmem_inp_norm_3_fa0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0,
								istaging_buff0_fb0,
								startrow_fg0,
								endrow_fg0,
								mean_buff,
								variance_buff,
								beta_buff,
								start_off_A_fg0,
								norm_flags,
								write_en,
								1);//need to generate cnrtl signal


		}

		else{
			norm_flags[0] = 1; //ddr_en
			norm_flags[1] = 1; //beta_sel
			norm_flags[2] = 1; //mean_sel
			norm_flags[3] = 1; //var_sel
			norm_flags[4] = 1; //relu
			norm_flags[5] = 1; //elem_wise_mul
			norm_flags[6] = 1; //elem_wise_add
			norm_flags[7] = 1; //pwr_sel
			norm_flags[8] = 1; //pwr_0p75
			norm_flags[9] = 1; //mul_in_sel
			write_en = 0;
			Normalization(conv_desc, layerx_loop_cnt_fg0, input_desc,gmem_input_layer1_fa0,gmem_inp_norm_2_fa0,gmem_bias_fa0,gmem_inp_norm_3_fa0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0,
					istaging_buff0_fb0,
					startrow_fg0,
					endrow_fg0,
					mean_buff,
					variance_buff,
					beta_buff,
					start_off_A_fg0,
					norm_flags,
					write_en,
					1);
		}




*/
	}//bn_en

	else if((startrow_fg0 < input_desc.height))
	{
		if(conv_desc.layer_id != 0)
		{
#if !XI_INPT_64bit_PORT//##8 planes

			for(ap_uint<16> imgG_fg0=0;imgG_fg0<layerx_img_fg0;imgG_fg0+=16)
			{
#pragma HLS LOOP_TRIPCOUNT min=0 max=0

				bool plane32_bool_fg0 = imgG_fg0[4];
				ap_uint<16> imgGby8_fg0;

				imgGby8_fg0 = imgG_fg0.range(15,4);

				ap_uint<32> offset_l1_fg0;
#pragma HLS RESOURCE variable=offset_l1_fg0 core=MulnS latency=2
				offset_l1_fg0 = (imgGby8_fg0)*input_hw_fg0;

				ap_uint<32> start_off_fg0 = start_off_A_fg0 + offset_l1_fg0;

				//			fprintf(stderr,"start_off_fg0 =%d   imgG_fg0 = %d offset_l1_fg0=%d \n",(int)start_off_fg0,(int)imgG_fg0,(int)offset_l1_fg0);

				InputReadLayerOther_fh
				(conv_desc, layerx_loop_cnt_fg0, input_desc, plane32_bool_fg0, gmem_input_layer_other1_fa0 + start_off_fg0,gmem_input_layer_other2_fa0 + start_off_fg0, istaging_buff0_fb0, startrow_fg0, endrow_fg0, imgG_fg0);

			}

#else//## 4plane
			In_Img_Loop:
			for(ap_uint<16> input_plane_fg1=0;input_plane_fg1<input_desc.planes;input_plane_fg1+=4)
			{
#pragma HLS LOOP_TRIPCOUNT min=0 max=0
				ap_uint<3> plane32_fg1 = input_plane_fg1.range(4,2);

				ap_uint<16> in_plane_by4_fg0;

				if(input_plane_fg1 < (input_desc.planes/2))
					in_plane_by4_fg0 = input_plane_fg1.range(15,2);
				else
					in_plane_by4_fg0 = input_plane_fg1.range(15,2) - (input_desc.planes(15,3)*conv_desc.buff_split);

				ap_uint<32> offset_l1_fg1;
#pragma HLS RESOURCE variable=offset_l1_fg1 core=MulnS latency=2
				offset_l1_fg1 = (in_plane_by4_fg0)*input_hw_fg0;

				ap_uint<32> start_off_fg1 = start_off_A_fg0 + offset_l1_fg1;

				if((input_plane_fg1 < (input_desc.planes/2)) || (conv_desc.buff_split==0))
					InputReadLayerOther_fh
					(conv_desc, layerx_loop_cnt_fg0, input_desc, plane32_fg1, gmem_input_layer_other1_fa0 + start_off_fg1, istaging_buff0_fb0, startrow_fg0, endrow_fg0, input_plane_fg1);
				else
					InputReadLayerOther_fh
					(conv_desc, layerx_loop_cnt_fg0, input_desc, plane32_fg1, gmem_input_layer_other2_fa0 + start_off_fg1, istaging_buff0_fb0, startrow_fg0, endrow_fg0, input_plane_fg1);
			}//In_Img_Loop
#endif

		}
		else
		{
			if(input_desc.mean_sub_flag)
			{
				InputReadLayer1_fi_mean_sub(layer1_loop_cnt_fg0, input_desc,  gmem_input_layer1_fa0 + start_off_l1_fg0, istaging_buff0_fb0);
			}else
			{
				InputReadLayer1_fi(layer1_loop_cnt_fg0, input_desc,  gmem_input_layer1_fa0 + start_off_l1_fg0, istaging_buff0_fb0);
			}

		}
	}





}

template<int KNK>
void LoadBiasBuffers_ff(gmem_biastype *gmem_bias_fa0,
		short bias_buff_fb0[XI_BIASMAXSIZE],
		ap_uint<16> outplanes_fb0)
{
#pragma HLS INLINE OFF
#if XI_KER_PROC==8
#pragma HLS ARRAY_PARTITION variable=bias_buff_fb0 cyclic factor=8
#else
#pragma HLS ARRAY_PARTITION variable=bias_buff_fb0 cyclic factor=4
#endif

	Biasread_Loop:
	for(ap_uint<16> ddr_pntr_ff0=0;ddr_pntr_ff0<(outplanes_fb0>>XI_BIASPACKCOUNT_LOG2);ddr_pntr_ff0++)
	{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=24 max=24
		biastype bias_word_64bit_ff0 = gmem_bias_fa0[ddr_pntr_ff0];
		ap_uint<16>bram_index_ff0 = ddr_pntr_ff0*(1<<XI_BIASPACKCOUNT_LOG2);
		Biastype_Loop:
		for(ap_uint<7> word_ff0=0,bit_ff0=0;word_ff0<(1<<XI_BIASPACKCOUNT_LOG2);word_ff0++,bit_ff0+=16)
		{
#pragma HLS UNROLL
			bias_buff_fb0[bram_index_ff0+word_ff0] = (short)(bias_word_64bit_ff0.range(bit_ff0+15,bit_ff0));
		}//Biastype_Loop
	}//Biasread_Loop
}

template<int IN_WW,int IN_HH,int OUT_WW,int OUT_HH,int CNUM_KERNELS,int CFILTER_SIZE,int CCONV_STRIDE,int PPOOL_STRIDE,int PPOOL_SIZE,int IINPUT_PLANES,int PNKPF>
void ProcResult_fe(input_struct input_desc,
		conv_struct conv_desc,
		weight_struct weight_desc,
		ap_uint<16> nk_process_fd0,
		output_struct output_desc,
		out_pix_struct out_pixels0_fc0,
		weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024],
		short ostaging_buff0_fb0[4][8][XI_OSTAGEBUFF_DEPTH],
		short bias_buff_fb0[XI_BIASMAXSIZE],
		ap_uint<12> rows_to_process_fb0,
		ap_uint<12> out_row_offset_fb0,
		ap_uint<2> mac_fz0)
{
#pragma HLS resource variable=weight_buff0_fd0 latency=4
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=2
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=2
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2
#if XI_KER_PROC==8
#pragma HLS ARRAY_PARTITION variable=bias_buff_fb0 cyclic factor=8
#else
#pragma HLS ARRAY_PARTITION variable=bias_buff_fb0 cyclic factor=4
#endif
#pragma HLS resource variable=bias_buff_fb0 latency=4
#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_cols complete dim=1
#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_rows complete dim=1
#pragma HLS data_pack variable=out_pixels0_fc0
#pragma HLS inline off

	short result_0_ping_fe0[4][XI_PIX_PROC];
	short result_1_ping_fe0[4][XI_PIX_PROC];
	short result_2_ping_fe0[4][XI_PIX_PROC];
	short result_3_ping_fe0[4][XI_PIX_PROC];
	short result_0_pong_fe0[4][XI_PIX_PROC];
	short result_1_pong_fe0[4][XI_PIX_PROC];
	short result_2_pong_fe0[4][XI_PIX_PROC];
	short result_3_pong_fe0[4][XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=result_0_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_1_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_2_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_3_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_0_pong_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_1_pong_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_2_pong_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_3_pong_fe0 complete dim=1
#pragma HLS resource variable=result_0_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_1_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_2_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_3_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_0_pong_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_1_pong_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_2_pong_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_3_pong_fe0 latency=4 core=RAM_T2P_BRAM

#if XI_KER_PROC==8
	short result_4_ping_fe0[4][XI_PIX_PROC];
	short result_5_ping_fe0[4][XI_PIX_PROC];
	short result_6_ping_fe0[4][XI_PIX_PROC];
	short result_7_ping_fe0[4][XI_PIX_PROC];
	short result_4_pong_fe0[4][XI_PIX_PROC];
	short result_5_pong_fe0[4][XI_PIX_PROC];
	short result_6_pong_fe0[4][XI_PIX_PROC];
	short result_7_pong_fe0[4][XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=result_4_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_5_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_6_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_7_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_4_pong_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_5_pong_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_6_pong_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_7_pong_fe0 complete dim=1
#pragma HLS resource variable=result_4_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_5_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_6_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_7_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_4_pong_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_5_pong_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_6_pong_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_7_pong_fe0 latency=4 core=RAM_T2P_BRAM

	Compute8Ker_fy<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
	(conv_desc, weight_buff0_fd0, input_buff0_fc0, 0,result_0_ping_fe0, result_1_ping_fe0, result_2_ping_fe0, result_3_ping_fe0,result_4_ping_fe0,
			/*..continue..*/result_5_ping_fe0, result_6_ping_fe0, result_7_ping_fe0);

	bool flag_fe0=0;
	Nkpf_Loop:
	for(ap_uint<4> nkpf_cnt_fe0=1, nkpf2_cnt_fe0=0;nkpf_cnt_fe0<weight_desc.nkpf; nkpf_cnt_fe0++,nkpf2_cnt_fe0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=7 max=7

		if(flag_fe0==0)
		{

			Compute8Ker_fy<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
			(conv_desc, weight_buff0_fd0, input_buff0_fc0, nkpf_cnt_fe0,result_0_pong_fe0, result_1_pong_fe0, result_2_pong_fe0, result_3_pong_fe0, result_4_pong_fe0,
					/*..continue..*/result_5_pong_fe0, result_6_pong_fe0, result_7_pong_fe0);

			OStgBuffSeq8Ker_fx<PNKPF>
			( result_0_ping_fe0, result_1_ping_fe0, result_2_ping_fe0, result_3_ping_fe0,result_4_ping_fe0, result_5_ping_fe0, result_6_ping_fe0, result_7_ping_fe0,
					/*..continue..*/bias_buff_fb0, out_pixels0_fc0, nk_process_fd0, conv_desc, output_desc, ostaging_buff0_fb0,rows_to_process_fb0, nkpf2_cnt_fe0,out_row_offset_fb0, mac_fz0);

			flag_fe0=1;
		}
		else
		{

			Compute8Ker_fy<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
			(conv_desc, weight_buff0_fd0, input_buff0_fc0, nkpf_cnt_fe0,result_0_ping_fe0, result_1_ping_fe0, result_2_ping_fe0, result_3_ping_fe0,result_4_ping_fe0,
					/*..continue..*/result_5_ping_fe0, result_6_ping_fe0, result_7_ping_fe0);

			OStgBuffSeq8Ker_fx<PNKPF>
			( result_0_pong_fe0, result_1_pong_fe0, result_2_pong_fe0, result_3_pong_fe0,result_4_pong_fe0, result_5_pong_fe0, result_6_pong_fe0, result_7_pong_fe0,
					/*..continue..*/bias_buff_fb0, out_pixels0_fc0, nk_process_fd0, conv_desc, output_desc, ostaging_buff0_fb0,rows_to_process_fb0, nkpf2_cnt_fe0,out_row_offset_fb0, mac_fz0);

			flag_fe0=0;
		}
	}//Nkpf_Loop

	ap_uint<4> nkpf2_cnt_fe1=weight_desc.nkpf-1;
	if(flag_fe0==0)
	{
		OStgBuffSeq8Ker_fx<PNKPF>
		( result_0_ping_fe0, result_1_ping_fe0, result_2_ping_fe0, result_3_ping_fe0,result_4_ping_fe0, result_5_ping_fe0, result_6_ping_fe0, result_7_ping_fe0,
				/*..continue..*/bias_buff_fb0, out_pixels0_fc0, nk_process_fd0, conv_desc, output_desc, ostaging_buff0_fb0,rows_to_process_fb0, nkpf2_cnt_fe1,out_row_offset_fb0, mac_fz0);
	}
	else
	{
		OStgBuffSeq8Ker_fx<PNKPF>
		( result_0_pong_fe0, result_1_pong_fe0, result_2_pong_fe0, result_3_pong_fe0,result_4_pong_fe0, result_5_pong_fe0, result_6_pong_fe0, result_7_pong_fe0,
				/*..continue..*/bias_buff_fb0, out_pixels0_fc0, nk_process_fd0, conv_desc, output_desc, ostaging_buff0_fb0,rows_to_process_fb0, nkpf2_cnt_fe1,out_row_offset_fb0, mac_fz0);
	}
#if !__SYNETHSIS__
	int ggg=0;
#endif

#elif XI_KER_PROC==4 || XI_KER_PROC==1 || XI_KER_PROC==2

	Compute_ft<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
	(input_desc,weight_desc, conv_desc, weight_buff0_fd0, input_buff0_fc0, 0,result_0_ping_fe0, result_1_ping_fe0, result_2_ping_fe0, result_3_ping_fe0);

	bool flag_fe1=0;
	Nkpf_Loop:
	for(ap_uint<4> nkpf_cnt_fe1=1, nkpf2_cnt_fe2=0;nkpf_cnt_fe1<weight_desc.nkpf; nkpf_cnt_fe1++,nkpf2_cnt_fe2++)
	{
#pragma HLS LOOP_TRIPCOUNT min=3 max=3

		if(flag_fe1==0)
		{

			Compute_ft<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
			(input_desc,weight_desc, conv_desc, weight_buff0_fd0, input_buff0_fc0, nkpf_cnt_fe1,result_0_pong_fe0, result_1_pong_fe0, result_2_pong_fe0, result_3_pong_fe0);

			OStgBuffSeq4Ker_fs<PNKPF>
			( result_0_ping_fe0, result_1_ping_fe0, result_2_ping_fe0, result_3_ping_fe0,bias_buff_fb0, out_pixels0_fc0,
					/*..continue..*/nk_process_fd0, conv_desc, output_desc, ostaging_buff0_fb0,rows_to_process_fb0, nkpf2_cnt_fe2,out_row_offset_fb0);

			flag_fe1=1;
		}
		else
		{

			Compute_ft<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
			(input_desc,weight_desc, conv_desc, weight_buff0_fd0, input_buff0_fc0, nkpf_cnt_fe1,result_0_ping_fe0, result_1_ping_fe0, result_2_ping_fe0, result_3_ping_fe0);

			OStgBuffSeq4Ker_fs<PNKPF>
			( result_0_pong_fe0, result_1_pong_fe0, result_2_pong_fe0, result_3_pong_fe0,bias_buff_fb0, out_pixels0_fc0,
					/*..continue..*/nk_process_fd0, conv_desc, output_desc, ostaging_buff0_fb0,rows_to_process_fb0, nkpf2_cnt_fe2,out_row_offset_fb0);

			flag_fe1=0;
		}
	}//Nkpf_Loop
	ap_uint<4> nkpf2_cnt_fe3=weight_desc.nkpf-1;
	if(flag_fe1==0)
	{
		OStgBuffSeq4Ker_fs<PNKPF>
		( result_0_ping_fe0, result_1_ping_fe0, result_2_ping_fe0, result_3_ping_fe0,bias_buff_fb0, out_pixels0_fc0,
				/*..continue..*/nk_process_fd0, conv_desc, output_desc, ostaging_buff0_fb0,rows_to_process_fb0, nkpf2_cnt_fe3,out_row_offset_fb0);
	}
	else
	{
		OStgBuffSeq4Ker_fs<PNKPF>
		( result_0_pong_fe0, result_1_pong_fe0, result_2_pong_fe0, result_3_pong_fe0,bias_buff_fb0, out_pixels0_fc0,
				/*..continue..*/nk_process_fd0, conv_desc, output_desc, ostaging_buff0_fb0,rows_to_process_fb0, nkpf2_cnt_fe3,out_row_offset_fb0);
	}
#endif//XI_KER_PROC

}

template<int IN_WW,int IN_HH,int OUT_WW,int OUT_HH,int CNUM_KERNELS,int CFILTER_SIZE,int CCONV_STRIDE,int PPOOL_STRIDE,int PPOOL_SIZE,int IINPUT_PLANES,int KERCNT,int PNKPF>
void ProcWeightBuff_fd(input_struct input_desc,
		conv_struct conv_desc,
		weight_struct weight_desc,
		output_struct output_desc,
		out_pix_struct out_pixels0_fc0,
		short input_buff0_fc0[4][XI_PIX_PROC/2][1024],
		short ostaging_buff0_fb0[4][8][XI_OSTAGEBUFF_DEPTH],
		gmem_weighttype *gmem_weight1_fa0,
		gmem_weighttype *gmem_weight2_fa0,
		short bias_buff_fb0[XI_BIASMAXSIZE],
		ap_uint<12> rows_to_process_fb0,
		ap_uint<12> out_row_offset_fb0,
		ap_uint<2> mac_fz0,
		ap_uint<16> pc_proc_wts)
{
#pragma HLS INLINE OFF

	bool load_ker_enable;
	if(weight_desc.slk_counter == 1)
	{
		if(pc_proc_wts == 0 && out_row_offset_fb0 == 0)
			load_ker_enable = 1;
		else
			load_ker_enable = 0;
	}
	else
	{
		load_ker_enable = 1;
	}
	//	fprintf(stderr,"\nout_row = %d \t pc = %d \t slk = %d\load_ker_read = %d",(int)out_row_offset_fb0, (int)pc_proc_wts, (int)weight_desc.slk_counter, (int)load_ker_enable);
	bool load_ker_en;
	if(conv_desc.sos_enable == 0 && load_ker_enable == 1)
		load_ker_en =1;
	else
		load_ker_en =0;

	weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],weight_buff1_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH];
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=2
#pragma HLS ARRAY_PARTITION variable=weight_buff1_fd0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=weight_buff1_fd0 complete dim=2
#pragma HLS resource variable=weight_buff0_fd0 latency=4
#pragma HLS resource variable=weight_buff1_fd0 latency=4

	ap_uint<12> wt_const_fd0;
	ap_uint<24> weight_offset_fd0;
	ap_uint<24> weight_offset_inc_fd0;
	ap_uint<14> weight_loopCount_fd0;
#pragma HLS RESOURCE variable=weight_loopCount_fd0 core=MulnS latency=2

#if XI_KER_PROC==8

	wt_const_fd0=weight_desc.nkpf*(weight_desc.filter_size_square);
	weight_offset_fd0 = (((weight_desc.filter_size_square)*(out_pixels0_fc0.current_plane_by4))*output_desc.planes)>>3;
	weight_offset_inc_fd0 = (wt_const_fd0*(input_desc.compute_planes))/4;

	weight_loopCount_fd0 = (input_desc.compute_planes.range(15,2))*wt_const_fd0.range(11,0);//*(weight_desc.slk_en | (!weight_desc.slk_counter));

#elif XI_KER_PROC==4
#if XI_WEIGHTS_8B
	wt_const_fd0=weight_desc.nkpf*(weight_desc.filter_size_square);
	weight_offset_fd0 = ((((weight_desc.filter_size_square)*(out_pixels0_fc0.current_plane_by4))*output_desc.planes)>>2);
	weight_offset_inc_fd0 = ((wt_const_fd0*(input_desc.compute_planes)).range(18,2));

	weight_loopCount_fd0 = (input_desc.compute_planes.range(15,2))*wt_const_fd0.range(11,0)*(*weight_desc.slk_en | (!weight_desc.slk_counter));

#else
	wt_const_fd0=weight_desc.nkpf*(weight_desc.filter_size_square+1);
	weight_offset_fd0 = (((weight_desc.filter_size_square+1)*(out_pixels0_fc0.current_plane_by4))*output_desc.planes)>>3;
	weight_offset_inc_fd0 = (wt_const_fd0*(input_desc.compute_planes)).range(18,3);

	weight_loopCount_fd0 = (input_desc.compute_planes.range(15,2))*wt_const_fd0.range(11,1)*(*weight_desc.slk_en | (!weight_desc.slk_counter));

#endif

#elif XI_KER_PROC ==1

	wt_const_fd0=weight_desc.nkpf*(weight_desc.filter_size_square);
	weight_offset_fd0 = (((weight_desc.filter_size_square)*(out_pixels0_fc0.current_plane_by4))*output_desc.planes);
	weight_offset_inc_fd0 = (wt_const_fd0*(input_desc.compute_planes));//.range(18,3);

	weight_loopCount_fd0 = (input_desc.compute_planes)*wt_const_fd0;//.range(11,1);

#elif XI_KER_PROC ==2

	wt_const_fd0=weight_desc.nkpf*(weight_desc.filter_size_square)/2;
	weight_offset_fd0 = (((weight_desc.filter_size_square)*(out_pixels0_fc0.current_plane_by4))*output_desc.planes);
	weight_offset_inc_fd0 = (wt_const_fd0*(input_desc.compute_planes));//.range(18,3);

	weight_loopCount_fd0 = (input_desc.compute_planes)*wt_const_fd0;//.range(11,1);

#endif//XI_KER_PROC

#if !__SYNTHESIS__
	int curr_pl = out_pixels0_fc0.current_plane_by4*4;
#endif

	LoadKernelsEn_fz<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>(weight_desc,weight_buff0_fd0,gmem_weight1_fa0+weight_offset_fd0,gmem_weight2_fa0+weight_offset_fd0, weight_loopCount_fd0, load_ker_en);

	bool flagpro_fd0 = 0;
	ap_uint<16> nk_fd0 = 0,nk_process_fd0=0;
#if XI_KER_PROC==1
	Nk_Loop:
	for(nk_fd0=(4*weight_desc.nkpf),nk_process_fd0=0; nk_fd0 < weight_desc.num_kernels; nk_fd0 += (4nk_fd0.nkpf),nk_process_fd0+=(4*weight_desc.nkpf))
#else
		Nk_Loop:
		for(nk_fd0=(XI_KER_PROC*weight_desc.nkpf),nk_process_fd0=0; nk_fd0 < conv_desc.ker_loop_cnt; nk_fd0 += (XI_KER_PROC*weight_desc.nkpf),nk_process_fd0+=(XI_KER_PROC*weight_desc.nkpf))
#endif
		{
#pragma HLS LOOP_TRIPCOUNT min=0 max=0
			weight_offset_fd0 += weight_offset_inc_fd0;

			if(flagpro_fd0==0)
			{
				LoadKernelsEn_fz<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>( weight_desc,weight_buff1_fd0,gmem_weight1_fa0+weight_offset_fd0,gmem_weight2_fa0+weight_offset_fd0, weight_loopCount_fd0, load_ker_en);
				ProcResult_fe<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,PNKPF>
				(input_desc,conv_desc,weight_desc,nk_process_fd0,output_desc,out_pixels0_fc0, weight_buff0_fd0,input_buff0_fc0,ostaging_buff0_fb0,bias_buff_fb0,rows_to_process_fb0,out_row_offset_fb0, mac_fz0);
				flagpro_fd0 = 1;
			}
			else
			{
				LoadKernelsEn_fz<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>(weight_desc,weight_buff0_fd0,gmem_weight1_fa0+weight_offset_fd0,gmem_weight2_fa0+weight_offset_fd0, weight_loopCount_fd0, load_ker_en);
				ProcResult_fe<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,PNKPF>
				(input_desc,conv_desc,weight_desc,nk_process_fd0,output_desc,out_pixels0_fc0, weight_buff1_fd0,input_buff0_fc0,ostaging_buff0_fb0,bias_buff_fb0,rows_to_process_fb0,out_row_offset_fb0, mac_fz0);
				flagpro_fd0 = 0;
			}

		}//Nk_Loop

	if(flagpro_fd0==1)
	{
		ProcResult_fe<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,PNKPF>
		(input_desc,conv_desc,weight_desc,nk_process_fd0,output_desc,out_pixels0_fc0, weight_buff1_fd0,input_buff0_fc0,ostaging_buff0_fb0,bias_buff_fb0,rows_to_process_fb0,out_row_offset_fb0, mac_fz0);
	}
	else
	{
		ProcResult_fe<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,PNKPF>
		(input_desc,conv_desc,weight_desc,nk_process_fd0,output_desc,out_pixels0_fc0, weight_buff0_fd0,input_buff0_fc0,ostaging_buff0_fb0,bias_buff_fb0,rows_to_process_fb0,out_row_offset_fb0, mac_fz0);
	}


//	weight_desc.slk_en = 0;
}

template<int IN_WW,int IN_HH,int OUT_WW,int OUT_HH,int CNUM_KERNELS,int CFILTER_SIZE,int CCONV_STRIDE,int PPOOL_STRIDE,int PPOOL_SIZE,int IINPUT_PLANES,int KERCNT,int OUTIMGCNT,int PNKPF>
void ProcInputBuff_fc(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		gmem_weighttype *gmem_weight1_fa0,
		gmem_weighttype *gmem_weight2_fa0,
		short bias_buff_fb0[XI_BIASMAXSIZE],
		ap_uint<12> out_row_offset_fb0,
		ap_uint<12> rows_to_process_fb0,
		short ostaging_buff0_fb0[4][8][XI_OSTAGEBUFF_DEPTH],
		ap_int<12> startrow_fb0,
		ap_uint<2> mac_fz0)
{
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2

#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=2
#pragma HLS resource variable=istaging_buff0_fb0 core=RAM_T2P_BRAM latency=2
#if XI_KER_PROC==8
#pragma HLS ARRAY_PARTITION variable=bias_buff_fb0 cyclic factor=8
#else
#pragma HLS ARRAY_PARTITION variable=bias_buff_fb0 cyclic factor=4
#endif
#pragma HLS resource variable=bias_buff_fb0 latency=4
#pragma HLS INLINE OFF


	short input_buff0_fc0[4][XI_PIX_PROC/2][1024],input_buff1_fc0[4][XI_PIX_PROC/2][1024];
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=2
#pragma HLS ARRAY_PARTITION variable=input_buff1_fc0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=input_buff1_fc0 complete dim=2
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=input_buff1_fc0 latency=4 core=RAM_T2P_BRAM

	out_pix_struct out_pixels0_fc0,out_pixels1_fc0;
#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_cols complete dim=1
#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_rows complete dim=1
#pragma HLS ARRAY_PARTITION variable=out_pixels1_fc0.pix_cols complete dim=1
#pragma HLS ARRAY_PARTITION variable=out_pixels1_fc0.pix_rows complete dim=1

	bool en_pad_fc0 = (startrow_fb0<0);

	ap_int<12> pad_row_fc0;
	if(en_pad_fc0)
		pad_row_fc0=startrow_fb0;
	else
		pad_row_fc0=-conv_desc.pad_num;

	ap_int<12> pad_row_wo_fc0 = (startrow_fb0*en_pad_fc0);

	ap_uint<10> current_plane_fc0=0;
	ap_uint<8> straddle_counter_fc0=conv_desc.straddle;

	ap_uint<16> total_pixel_fc0;
#pragma HLS RESOURCE variable=total_pixel_fc0 core=MulnS latency=2
	total_pixel_fc0=rows_to_process_fb0*output_desc.width;

	ap_uint<16> pcmf_off1_fc0;
	pcmf_off1_fc0 = (total_pixel_fc0/conv_desc.pix_per_kp) +1;
	ap_uint<8> pcmf_m1=(conv_desc.straddle-1);

	ap_uint<16>  pcmf_off_fc0;
#pragma HLS RESOURCE variable=pcmf_off_fc0 core=MulnS latency=2
	pcmf_off_fc0 = pcmf_off1_fc0*pcmf_m1;

	ap_uint<16> pc_loop_mul_fc0;
#pragma HLS RESOURCE variable=pc_loop_mul_fc0 core=MulnS latency=2
	pc_loop_mul_fc0=conv_desc.pix_per_kp * pcmf_off_fc0;

	ap_uint<16> pc_loop_max_fc0;
	pc_loop_max_fc0= total_pixel_fc0 + pc_loop_mul_fc0;

	ap_uint<16> row_id_1st_32pix_fc0=0;
	ap_uint<16> col_id_1st_32pix_fc0=0;
	ap_uint<16> next_col_id_1st_32pix_fc0=conv_desc.pix_mod_outwidth;
	ap_uint<16> row_id_2nd_32pix_fc0=conv_desc.pix2_div_outwidth;
	ap_uint<16> col_id_2nd_32pix_fc0=conv_desc.pix2_mod_outwidth;
	ap_uint<16> next_col_id_2nd_32pix_fc0=col_id_2nd_32pix_fc0+conv_desc.pix_mod_outwidth;
	ap_uint<16> pc_proc_wts = 0;

	LoadFeedingBuff_fl<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2)>
	(input_desc, conv_desc, output_desc,weight_desc,out_pixels0_fc0,istaging_buff0_fb0, input_buff0_fc0 ,0, current_plane_fc0, out_row_offset_fb0,
			/*..continue..*/pad_row_fc0, pad_row_wo_fc0,row_id_1st_32pix_fc0, col_id_1st_32pix_fc0, row_id_2nd_32pix_fc0, col_id_2nd_32pix_fc0, mac_fz0);

	bool flag_fc0 = 0;
	Pc_Loop:
	for(ap_uint<16> pc_encoded_fc0=conv_desc.pix_per_kp, pc_fc0=0;pc_encoded_fc0<pc_loop_max_fc0;pc_encoded_fc0=pc_encoded_fc0+conv_desc.pix_per_kp)
	{
#pragma HLS LOOP_TRIPCOUNT min=59 max=59

		straddle_counter_fc0--;
		current_plane_fc0+=(conv_desc.straddle_in_plane_inc_by4);//(input_desc.compute_planes.range(11,2));
		if(straddle_counter_fc0==0)
		{
			straddle_counter_fc0=conv_desc.straddle;
			current_plane_fc0=0;
			pc_fc0+=conv_desc.pix_per_kp;

			row_id_1st_32pix_fc0+=(conv_desc.pix_by_outwidth + (next_col_id_1st_32pix_fc0>=output_desc.width));
			col_id_1st_32pix_fc0+=(conv_desc.pix_mod_outwidth - (output_desc.width*(next_col_id_1st_32pix_fc0>=output_desc.width)));
			next_col_id_1st_32pix_fc0=col_id_1st_32pix_fc0+conv_desc.pix_mod_outwidth;

			row_id_2nd_32pix_fc0+=(conv_desc.pix_by_outwidth + (next_col_id_2nd_32pix_fc0>=output_desc.width));
			col_id_2nd_32pix_fc0+=(conv_desc.pix_mod_outwidth - (output_desc.width*(next_col_id_2nd_32pix_fc0>=output_desc.width)));
			next_col_id_2nd_32pix_fc0=col_id_2nd_32pix_fc0+conv_desc.pix_mod_outwidth;
		}

		if(!flag_fc0)
		{
			LoadFeedingBuff_fl<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2)>
			(input_desc, conv_desc, output_desc,weight_desc, out_pixels1_fc0,istaging_buff0_fb0, input_buff1_fc0, pc_fc0,current_plane_fc0,out_row_offset_fb0,
					/*..continue..*/pad_row_fc0, pad_row_wo_fc0,row_id_1st_32pix_fc0, col_id_1st_32pix_fc0, row_id_2nd_32pix_fc0, col_id_2nd_32pix_fc0, mac_fz0);

			ProcWeightBuff_fd<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,KERCNT,PNKPF>
			(input_desc, conv_desc, weight_desc,output_desc,out_pixels0_fc0,input_buff0_fc0,ostaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,bias_buff_fb0,rows_to_process_fb0,out_row_offset_fb0, mac_fz0,pc_proc_wts);

			flag_fc0 = 1;
		}
		else
		{
			LoadFeedingBuff_fl<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2)>
			(input_desc, conv_desc,output_desc,weight_desc,out_pixels0_fc0,istaging_buff0_fb0, input_buff0_fc0, pc_fc0,current_plane_fc0, out_row_offset_fb0,
					/*..continue..*/pad_row_fc0, pad_row_wo_fc0,row_id_1st_32pix_fc0, col_id_1st_32pix_fc0, row_id_2nd_32pix_fc0, col_id_2nd_32pix_fc0, mac_fz0);

			ProcWeightBuff_fd<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,KERCNT,PNKPF>
			(input_desc, conv_desc, weight_desc,output_desc,out_pixels1_fc0,input_buff1_fc0,ostaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,bias_buff_fb0,rows_to_process_fb0,out_row_offset_fb0, mac_fz0,pc_proc_wts);
			flag_fc0 = 0;
		}
		pc_proc_wts = pc_encoded_fc0;
	}//Pc_Loop

	if(flag_fc0 == 1)
	{
		ProcWeightBuff_fd<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,KERCNT,PNKPF>
		(input_desc, conv_desc, weight_desc,output_desc,out_pixels1_fc0,input_buff1_fc0,ostaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,bias_buff_fb0,rows_to_process_fb0,out_row_offset_fb0, mac_fz0,pc_proc_wts);
	}
	else
	{
		ProcWeightBuff_fd<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,KERCNT,PNKPF>
		(input_desc, conv_desc, weight_desc,output_desc,out_pixels0_fc0,input_buff0_fc0,ostaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,bias_buff_fb0,rows_to_process_fb0,out_row_offset_fb0, mac_fz0,pc_proc_wts);
	}

}

template<int IN_WW,int IN_HH,int OUT_WW,int OUT_HH,int CNUM_KERNELS,int CFILTER_SIZE,int CCONV_STRIDE,int PPOOL_STRIDE,int PPOOL_SIZE,int IINPUT_PLANES,int KERCNT,int OUTIMGCNT,int PNKPF>
void ProcFeedingBuff_fz(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		gmem_weighttype *gmem_weight1_fa0,
		gmem_weighttype *gmem_weight2_fa0,
		short bias_buff_fb0[XI_BIASMAXSIZE],
		ap_uint<12> out_row_offset_fb0,
		ap_uint<12> rows_to_process_fb0,
		short ostaging_buff0_fb0[4][8][XI_OSTAGEBUFF_DEPTH],
		ap_int<12> startrow_fb0)
{
#pragma HLS INLINE OFF


	for(ap_uint<3> mac_fz0 = 0; mac_fz0 < conv_desc.mac_loop_count; mac_fz0++)
	{
		ProcInputBuff_fc<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,(CNUM_KERNELS>>5),((OUT_WW*6)>>5),PNKPF>
		(input_desc, conv_desc,output_desc,weight_desc,istaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,bias_buff_fb0,out_row_offset_fb0,rows_to_process_fb0, ostaging_buff0_fb0,startrow_fb0, mac_fz0);
	}
}

void StoreNormData(conv_struct conv_desc,
		ap_uint<16> layerx_loop_cnt_fg0,
		input_struct input_desc ,
		gmem_inputtype_layer1 *gmem_istg_out_1_fa0,
		gmem_inputtype_layer1 *gmem_istg_out_2_fa0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<12> startrow_fg0,
		ap_uint<12> endrow_fg0,
		ap_uint<32> start_off_A_fg0
)
{
#pragma HLS INLINE OFF
	ap_uint<32> start_off;// =start_off_A_fg0*2;
	for(ap_uint<16> imgG=0;imgG<input_desc.planes;imgG+=16)
	{
		ap_uint<2> plane32 = imgG[4];

		ap_uint<16> imgGby16;
		imgGby16 = imgG.range(15,4);

		ap_uint<32> offset_l1;

		offset_l1 = (imgGby16)*input_desc.size;

		//if(imgG%16==0)
			start_off = start_off_A_fg0*2 + offset_l1*2;
#if 0
		fprintf(stderr,"\n values inside procfeedingbuff \n");
				fprintf(stderr,"\n layerx_loop_cnt_fg0 = %d  plane32 = %d start_off = %d startrow_fg0 = %d endrow_fg0 = %d imgG = %d",(int)layerx_loop_cnt_fg0,(int)plane32,(int)start_off,(int)startrow_fg0,(int)endrow_fg0,(int)imgG);
#endif
	//if(imgG[3]==0)
			WriteNormData(conv_desc, 2*layerx_loop_cnt_fg0, input_desc,plane32,gmem_istg_out_1_fa0 + start_off,gmem_istg_out_2_fa0 + start_off,istaging_buff0_fb0,startrow_fg0,endrow_fg0,imgG);
		//	else
			//WriteNormData(conv_desc, 2*layerx_loop_cnt_fg0, input_desc,plane32,gmem_istg_out_2_fa0 + start_off,istaging_buff0_fb0,startrow_fg0,endrow_fg0,imgG);
	}

}
template<int IN_WW,int IN_HH,int OUT_WW,int OUT_HH,int CNUM_KERNELS,int CFILTER_SIZE,int CCONV_STRIDE,int PPOOL_STRIDE,int PPOOL_SIZE,int IINPUT_PLANES,int KERCNT,int OUTIMGCNT,int PNKPF>
void ProcFeedingBuff_En_fz(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],
		gmem_weighttype *gmem_weight1_fa0,
		gmem_weighttype *gmem_weight2_fa0,
		short bias_buff_fb0[XI_BIASMAXSIZE],
		ap_uint<12> out_row_offset_fb0,
		ap_uint<12> rows_to_process_fb0,
		short ostaging_buff0_fb0[4][8][XI_OSTAGEBUFF_DEPTH],
		ap_int<12> startrow_fb0,
		ap_int<12> startrow_in_fb0,
		ap_int<12> endrow_in_fb0,
		gmem_inputtype_layer1 *gmem_istg_out_1_fa0,
		gmem_inputtype_layer1 *gmem_istg_out_2_fa0)
{
#pragma HLS INLINE OFF



	ap_uint<12> startrow_fg0,endrow_fg0;

	if(startrow_in_fb0<0)
		startrow_fg0 = 0;
	else
		startrow_fg0 = startrow_in_fb0;



	if(endrow_in_fb0<input_desc.height)
		endrow_fg0 = endrow_in_fb0;
	else
		endrow_fg0 = input_desc.height-1;

	ap_uint<12> num_rows_fg0 = endrow_fg0-startrow_fg0 +1;

	ap_uint<16> layerx_loop_cnt_fg0;
	layerx_loop_cnt_fg0 = num_rows_fg0 * input_desc.width;

	ap_uint<32> start_off_A_fg0;
#pragma HLS RESOURCE variable=start_off_A_fg0 core=MulnS latency=2
	start_off_A_fg0 = startrow_fg0*input_desc.width;


	//if(!(conv_desc.lrn_pns_enable || conv_desc.bn_snb_enable || conv_desc.elem_wise_add_en || conv_desc.bn_conv_fuse_en))
	if(!(conv_desc.lrn_pns_enable || conv_desc.bn_snb_enable || conv_desc.elem_wise_add_en))
	{
		ProcFeedingBuff_fz<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,(CNUM_KERNELS>>5),((OUT_WW*6)>>5),PNKPF>
		(input_desc, conv_desc,output_desc,weight_desc,istaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,bias_buff_fb0,out_row_offset_fb0,rows_to_process_fb0, ostaging_buff0_fb0,startrow_fb0);
	}

	else
	{

		StoreNormData(conv_desc,
				layerx_loop_cnt_fg0,
				input_desc ,
				gmem_istg_out_1_fa0,
				gmem_istg_out_2_fa0,
				istaging_buff0_fb0,
				startrow_fg0,
				endrow_fg0,
				start_off_A_fg0);
	}

}
template<int IN_WW,int IN_HH,int OUT_WW,int OUT_HH,int CNUM_KERNELS,int CFILTER_SIZE,int CCONV_STRIDE,int PPOOL_STRIDE,int PPOOL_SIZE,int IINPUT_PLANES,int PNKPF>
void ProcIStagingBuff_fb(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		gmem_inputtype_layer1 *gmem_input_layer1_fa0,
		gmem_inputtype_layerx *gmem_input_layer_other1_fa0,
		gmem_inputtype_layerx *gmem_input_layer_other2_fa0,
		gmem_outputtype *gmem_output1_fa0,
		gmem_outputtype *gmem_output2_fa0,
		gmem_weighttype *gmem_weight1_fa0,
		gmem_weighttype *gmem_weight2_fa0,
		gmem_biastype *gmem_bias_fa0,
		gmem_inputtype_layer1 *gmem_inp_norm_2_fa0,
		gmem_inputtype_layer1 	*gmem_inp_norm_3_fa0,
		gmem_inputtype_layer1 *gmem_istg_out_1_fa0,
		gmem_inputtype_layer1 *gmem_istg_out_2_fa0)
{
#pragma HLS INLINE OFF

	short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH],istaging_buff1_fb0[4][8][XI_ISTAGEBUFF_DEPTH];
#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=2
#pragma HLS ARRAY_PARTITION variable=istaging_buff1_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=istaging_buff1_fb0 complete dim=2
#pragma HLS resource variable=istaging_buff0_fb0 core=RAM_T2P_BRAM latency=2
#pragma HLS resource variable=istaging_buff1_fb0 core=RAM_T2P_BRAM latency=2

	short ostaging_buff0_fb0[4][8][XI_OSTAGEBUFF_DEPTH],ostaging_buff1_fb0[4][8][XI_OSTAGEBUFF_DEPTH];
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2
#pragma HLS ARRAY_PARTITION variable=ostaging_buff1_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff1_fb0 complete dim=2
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM
#pragma HLS resource variable=ostaging_buff1_fb0 core=RAM_S2P_BRAM


	short mean_buff[2][1024];
	short variance_buff[2][1024];
	short beta_buff[2][1024];
#pragma HLS ARRAY_PARTITION variable=mean_buff complete dim=1
#pragma HLS ARRAY_PARTITION variable=variance_buff complete dim=1
#pragma HLS ARRAY_PARTITION variable=beta_buff complete dim=1
#pragma HLS resource variable=mean_buff core=RAM_T2P_BRAM
#pragma HLS resource variable=variance_buff core=RAM_T2P_BRAM
#pragma HLS resource variable=beta_buff core=RAM_T2P_BRAM


	short bias_buff_fb0[XI_BIASMAXSIZE];
#if XI_KER_PROC==8
#pragma HLS ARRAY_PARTITION variable=bias_buff_fb0 cyclic factor=8
#else
#pragma HLS ARRAY_PARTITION variable=bias_buff_fb0 cyclic factor=4
#endif

	///////Read Biad into BRAMS
	LoadBiasBuffers_ff<(CNUM_KERNELS>>XI_BIASPACKCOUNT_LOG2)>(gmem_bias_fa0,bias_buff_fb0,output_desc.planes);

	ap_uint<12> rows_to_process_fb0;
	rows_to_process_fb0 = conv_desc.ostg_row_count;
	ap_uint<12> out_row_offset_fb0=0;
	ap_uint<12> out_row_offset_wr_fb0=0;
	ap_int<12> startrow_fb0,endrow_fb0, startrow_process_fb0;
	ap_int<12> norm_startrow_fb0,norm_endrow_fb0;
	ap_int<12> startrow_inc_fb0 = conv_desc.ostg_row_count*weight_desc.filter_stride;


	startrow_fb0 = -conv_desc.pad_num;
	norm_startrow_fb0 = -conv_desc.pad_num;;
	endrow_fb0 = startrow_inc_fb0 + weight_desc.filter_size_dilated - weight_desc.filter_stride - conv_desc.pad_num -1;
	norm_endrow_fb0 = startrow_inc_fb0 + weight_desc.filter_size_dilated - weight_desc.filter_stride - conv_desc.pad_num -1;
	ap_uint<12> outrow_max_fb0=output_desc.height;
	if(conv_desc.read_from_ddr == 1)
	{
		LoadIStagingBuff_fg<IN_WW,IN_HH,IINPUT_PLANES>
		(input_desc, conv_desc, gmem_input_layer1_fa0, gmem_input_layer_other1_fa0, gmem_input_layer_other2_fa0,gmem_bias_fa0,gmem_inp_norm_2_fa0,gmem_inp_norm_3_fa0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0, istaging_buff0_fb0, startrow_fb0, endrow_fb0,norm_startrow_fb0,norm_endrow_fb0,mean_buff,variance_buff,beta_buff);
	}
	startrow_process_fb0 =-conv_desc.pad_num;
	startrow_fb0 += startrow_inc_fb0;
	endrow_fb0 += startrow_inc_fb0;

	bool write_en_fb0=0;
	bool pingpong_flag_fb0=0;

	ProciStg_Loop:
	for(ap_uint<12> output_row_fb0=conv_desc.ostg_row_count, output_row_next_fb0=conv_desc.ostg_row_count*2; output_row_fb0<output_desc.height;
			/*..continue..*/output_row_fb0 += conv_desc.ostg_row_count, output_row_next_fb0 += conv_desc.ostg_row_count)
	{
#pragma HLS LOOP_TRIPCOUNT min=59 max=59

		if(!pingpong_flag_fb0)
		{
			ProcFeedingBuff_En_fz<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,(CNUM_KERNELS>>5),((OUT_WW*6)>>5),PNKPF>
			(input_desc, conv_desc,output_desc,weight_desc,istaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,bias_buff_fb0,out_row_offset_fb0,rows_to_process_fb0, ostaging_buff0_fb0,startrow_process_fb0,norm_startrow_fb0,norm_endrow_fb0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0);

			StoreOStagingBuff_En_fj<CNUM_KERNELS, OUT_WW>
			(output_desc, conv_desc, out_row_offset_wr_fb0, gmem_output1_fa0, gmem_output2_fa0, ostaging_buff1_fb0, rows_to_process_fb0,write_en_fb0);

			LoadIStagingBuff_fg<IN_WW,IN_HH,IINPUT_PLANES>
			(input_desc, conv_desc,gmem_input_layer1_fa0, gmem_input_layer_other1_fa0, gmem_input_layer_other2_fa0,gmem_bias_fa0,gmem_inp_norm_2_fa0,gmem_inp_norm_3_fa0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0, istaging_buff1_fb0,startrow_fb0, endrow_fb0,norm_startrow_fb0,norm_endrow_fb0,mean_buff,variance_buff,beta_buff);

			pingpong_flag_fb0 = 1;
		}
		else
		{
			ProcFeedingBuff_En_fz<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,(CNUM_KERNELS>>5),((OUT_WW*6)>>5),PNKPF>
			(input_desc, conv_desc,output_desc,weight_desc,istaging_buff1_fb0, gmem_weight1_fa0,gmem_weight2_fa0,bias_buff_fb0,out_row_offset_fb0,rows_to_process_fb0, ostaging_buff1_fb0,startrow_process_fb0,norm_startrow_fb0,norm_endrow_fb0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0);
			StoreOStagingBuff_En_fj<CNUM_KERNELS, OUT_WW>
			(output_desc, conv_desc, out_row_offset_wr_fb0, gmem_output1_fa0, gmem_output2_fa0, ostaging_buff0_fb0, rows_to_process_fb0,write_en_fb0);

			LoadIStagingBuff_fg<IN_WW,IN_HH,IINPUT_PLANES>
			(input_desc, conv_desc,gmem_input_layer1_fa0, gmem_input_layer_other1_fa0, gmem_input_layer_other2_fa0,gmem_bias_fa0,gmem_inp_norm_2_fa0,gmem_inp_norm_3_fa0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0, istaging_buff0_fb0,startrow_fb0, endrow_fb0,norm_startrow_fb0,norm_endrow_fb0,mean_buff,variance_buff,beta_buff);

			pingpong_flag_fb0 = 0;
		}

		startrow_process_fb0 += startrow_inc_fb0;
		startrow_fb0 += startrow_inc_fb0;
		norm_startrow_fb0 += startrow_inc_fb0;
		endrow_fb0 += startrow_inc_fb0;
		norm_endrow_fb0 += startrow_inc_fb0;
		outrow_max_fb0 = output_row_next_fb0;
		out_row_offset_wr_fb0 = out_row_offset_fb0;
		out_row_offset_fb0+=conv_desc.ostg_row_count;
		write_en_fb0 = 1;
	}//ProciStg_Loop

	ap_uint<12> rows_to_process_last_fb0 = conv_desc.ostg_row_count - (outrow_max_fb0 - output_desc.height);
#if 1
	if(pingpong_flag_fb0 == 1)
	{
		ProcFeedingBuff_En_fz<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,
		/*..continue..*/PPOOL_SIZE,IINPUT_PLANES,(CNUM_KERNELS>>5),((OUT_WW*6)>>5),PNKPF>
		(input_desc, conv_desc,output_desc,weight_desc,istaging_buff1_fb0, gmem_weight1_fa0,gmem_weight2_fa0,bias_buff_fb0,out_row_offset_fb0,
				/*..continue..*/rows_to_process_last_fb0, ostaging_buff1_fb0, startrow_process_fb0,norm_startrow_fb0,norm_endrow_fb0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0);

		StoreOStagingBuff_En_fj<CNUM_KERNELS, OUT_WW>
		(output_desc, conv_desc, out_row_offset_wr_fb0, gmem_output1_fa0,gmem_output2_fa0, ostaging_buff0_fb0, rows_to_process_fb0,write_en_fb0);

		StoreOStagingBuff_En_fj<CNUM_KERNELS, OUT_WW>
		(output_desc, conv_desc, out_row_offset_fb0, gmem_output1_fa0, gmem_output2_fa0, ostaging_buff1_fb0, rows_to_process_last_fb0,1);

	}
	else
	{
		ProcFeedingBuff_En_fz<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,
		/*..continue..*/PPOOL_SIZE,IINPUT_PLANES,(CNUM_KERNELS>>5),((OUT_WW*6)>>5),PNKPF>
		(input_desc, conv_desc,output_desc,weight_desc,istaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,bias_buff_fb0,out_row_offset_fb0,
				/*..continue..*/rows_to_process_last_fb0, ostaging_buff0_fb0,startrow_process_fb0,norm_startrow_fb0,norm_endrow_fb0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0);

		StoreOStagingBuff_En_fj<CNUM_KERNELS, OUT_WW>
		(output_desc, conv_desc, out_row_offset_wr_fb0, gmem_output1_fa0, gmem_output2_fa0, ostaging_buff1_fb0, rows_to_process_fb0,write_en_fb0);

		StoreOStagingBuff_En_fj<CNUM_KERNELS, OUT_WW>
		(output_desc, conv_desc, out_row_offset_fb0, gmem_output1_fa0, gmem_output2_fa0, ostaging_buff0_fb0, rows_to_process_last_fb0,1);
	}

	if(conv_desc.write_to_ddr == 0)
		CopyOstgToIstg(istaging_buff0_fb0, ostaging_buff0_fb0, conv_desc, output_desc);
#endif

}

template<int IN_WW,int IN_HH,int OUT_WW,int OUT_HH,int CNUM_KERNELS,int CFILTER_SIZE,int CCONV_STRIDE,int PPOOL_STRIDE,int PPOOL_SIZE,int IINPUT_PLANES,int PNKPF>
void ConvLayer_fa(input_struct input_desc,
		weight_struct weight_desc,
		output_struct output_desc,
		conv_struct conv_desc,
		gmem_weighttype *weights1,
		gmem_weighttype *weights2,
		gmem_inputtype_layer1 *input_1st,
		gmem_inputtype_layerx *input_other1,
		gmem_inputtype_layerx *input_other2,
		gmem_biastype *bias,
		gmem_outputtype *output1,
		gmem_outputtype *output2,
		gmem_inputtype_layer1 *inp_norm_2,
		gmem_inputtype_layer1 *inp_norm_3,
		gmem_inputtype_layer1 *istg_out1,
		gmem_inputtype_layer1 *istg_out2)
{
#pragma HLS INLINE OFF

	ap_uint<32> input_group_offset_1st_fa0,input_group_offset_other_fa0;
	ap_uint<32> weights_group_offset_fa0,output_group_offset_fa0,bias_group_offset_fa0;

	OffsetMemPointer_ffb(input_desc, output_desc, weight_desc, conv_desc, input_group_offset_1st_fa0,input_group_offset_other_fa0,
			/*..continue..*/weights_group_offset_fa0,output_group_offset_fa0,bias_group_offset_fa0);

	gmem_inputtype_layer1 	*gmem_input_layer1_fa0 		= input_1st + input_group_offset_1st_fa0;
	gmem_inputtype_layerx 	*gmem_input_layer_other1_fa0= input_other1 + input_group_offset_other_fa0;
	gmem_inputtype_layerx 	*gmem_input_layer_other2_fa0= input_other2 + input_group_offset_other_fa0;
	gmem_weighttype 		*gmem_weight1_fa0 			= weights1 + weights_group_offset_fa0;
	gmem_weighttype 		*gmem_weight2_fa0 			= weights2 + weights_group_offset_fa0;
	gmem_biastype 			*gmem_bias_fa0 				= bias + bias_group_offset_fa0;
	gmem_outputtype 		*gmem_output1_fa0 			= output1 + output_group_offset_fa0;
	gmem_outputtype 		*gmem_output2_fa0 			= output2 + output_group_offset_fa0;
	gmem_inputtype_layer1 	*gmem_inp_norm_2_fa0 		= inp_norm_2;
	gmem_inputtype_layer1 	*gmem_inp_norm_3_fa0		= inp_norm_3;
	gmem_inputtype_layer1 	*gmem_istg_out_1_fa0 		= istg_out1;
	gmem_inputtype_layer1 	*gmem_istg_out_2_fa0 		= istg_out2;

	/*	fprintf(stderr,"\n");
	for(int i=0,bit=0;i<16;i++,bit+=8)
	{
		ap_int<128> test = gmem_weight2_fa0[0];
		ap_int<8> wt = test.range((bit+7),bit);

		fprintf(stderr,"wt2[%d] = %d\t",(int)i,(int)wt);
	}
	fprintf(stderr,"\n");*/


	ProcIStagingBuff_fb<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,PNKPF>
	(input_desc, conv_desc, output_desc, weight_desc, gmem_input_layer1_fa0, gmem_input_layer_other1_fa0,gmem_input_layer_other2_fa0,
			/*..continue..*/gmem_output1_fa0,gmem_output2_fa0,gmem_weight1_fa0,gmem_weight2_fa0,gmem_bias_fa0,gmem_inp_norm_2_fa0,gmem_inp_norm_3_fa0,gmem_istg_out_1_fa0,gmem_istg_out_2_fa0);
}

template<int IN_WW,int IN_HH,int OUT_WW,int OUT_HH,int CNUM_KERNELS,int CFILTER_SIZE,int CCONV_STRIDE,int PPOOL_STRIDE,int PPOOL_SIZE,int IINPUT_PLANES,int PNKPF>
void Convolution(gmem_weighttype *weights1,
		gmem_weighttype *weights2,
		gmem_outputtype *output1,
		gmem_outputtype *output2,
		gmem_inputtype_layerx *input_other1,
		gmem_inputtype_layerx *input_other2,
		gmem_inputtype_layer1 *input_1st,
		gmem_biastype *bias,
		gmem_inputtype_layer1 *inp_norm_2,
		gmem_inputtype_layer1 *inp_norm_3,
		gmem_inputtype_layer1 *istg_out1,
		gmem_inputtype_layer1 *istg_out2,
		int *scalar_conv_args)
{
#pragma HLS INLINE OFF

	input_struct input_desc;
	output_struct output_desc;
	weight_struct weight_desc;
	conv_struct conv_desc;
	LoadDesc_ffa(scalar_conv_args, input_desc, output_desc, weight_desc, conv_desc);

	ConvLayer_fa<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,PNKPF>
	(input_desc, weight_desc, output_desc, conv_desc,weights1+weight_desc.weight_offset,weights2+weight_desc.weight_offset,input_1st,input_other1,input_other2,bias+output_desc.bias_offset,output1+output_desc.out_offset,output2+output_desc.out_offset,inp_norm_2,inp_norm_3,istg_out1,istg_out2);


}
