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


void LoadDesc_ffa(int *scalar_conv_args,
		input_struct &input_desc,
		output_struct &output_desc,
		weight_struct &weight_desc,
		conv_struct &conv_desc,
		group_conv_struct &group_desc)
{
#pragma HLS INLINE off
	int scalar_args[128];

	Arg_Loop:
	for (ap_uint<10> arg = 0; arg < 128; arg++)
	{
#pragma HLS PIPELINE
		scalar_args[arg] = scalar_conv_args[arg];
	}

	int l_input_height_ffa0     	= scalar_args[0];
	int l_input_Width_ffa0         	= scalar_args[1];
	int l_output_height_ffa0     	= scalar_args[2];
	int l_output_width_ffa0     	= scalar_args[3];
	int l_conv_stride_ffa0         	= scalar_args[6];
	int l_filter_height_ffa0     	= scalar_args[7];
	int l_pad_ffa0                 	= scalar_args[9];
	int l_relu_ffa0             	= scalar_args[10];
	int l_group_ffa0             	= scalar_args[11];
	int l_layer_ffa0             	= scalar_args[12];
	int l_nkpf_ffa0             	= scalar_args[13];
	int l_istg_row_cnt_ffa0     	= scalar_args[14];
	int l_ostg_row_cnt_ffa0     	= scalar_args[15];
	int l_straddle_factor_ffa0     	= scalar_args[17];
	int l_out_offset_ffa0         	= scalar_args[18];
#if !__SYNTHESIS__
	bool l_single_load_kernel_ffa0 	= 0;
#else
	bool l_single_load_kernel_ffa0 	= scalar_args[19];
#endif
	int l_inout_precision_ffa0     	= scalar_args[20];
	int l_shift_precision2_ffa0 	= scalar_args[21];
	int l_mean_value0_ffa0         	= scalar_args[22];
	int l_mean_value1_ffa0         	= scalar_args[23];
	int l_mean_value2_ffa0         	= scalar_args[24];
	int l_mean_value3_ffa0         	= scalar_args[25];
	bool l_mean_sub_flag_ffa0     	= scalar_args[26];
	int l_bias_offset_ffa0         	= scalar_args[27];
	int l_weight_offset_ffa0     	= scalar_args[28];
	int l_dilation_factor_ffa0     	= scalar_args[29];
	int l_read_from_ddr         	= scalar_args[31];
	int l_write_to_ddr             	= scalar_args[32];
	int l_norm_k                 	= scalar_args[33];
	int l_opcode                 	= scalar_args[34];
	int l_mul_in                 	= scalar_args[35];
	int l_norm_prec             	= scalar_args[36];
	int l_norm_prec_2             	= scalar_args[37];
	int l_norm_prec_3             	= scalar_args[38];
	bool l_int6_en_in             	= scalar_args[39];
	bool l_int6_en_out             	= scalar_args[40];
	int l_rounding_conv             = scalar_args[41];
	int l_rounding_3dconv           = scalar_args[42];
	int l_conv_3d_ip_h              = scalar_args[43];
	int l_conv_3d_ip_w              = scalar_args[44];
	int l_conv_3d_op_h              = scalar_args[45];
	int l_conv_3d_op_w              = scalar_args[46];
	int l_conv_3d_ft_h              = scalar_args[47];
	int l_conv_3d_ft_w              = scalar_args[48];
	int l_conv_3d_fsz2              = scalar_args[49];
	int l_conv_3d_ip_pln            = scalar_args[50];
	int l_conv_3d_op_pln            = scalar_args[51];
	int l_conv_3d_pad               = scalar_args[52];
	int l_conv_3d_stride            = scalar_args[53];
	int l_conv_3d_relu              = scalar_args[54];
	int l_conv_3d_inp_offset        = scalar_args[55];
	int l_conv_3d_istg_row_cnt      = scalar_args[56];
	int l_conv_3d_outshift          = scalar_args[57];
	int l_conv_3d_ostg_row_cnt            = scalar_args[58];
	int l_conv_3d_ip_size           = scalar_args[59];

	int l_input_planes_align4_ffa0       	= scalar_args[60];
	int l_compute_planes_align4_ffa0     	= scalar_args[61];
	int l_output_planes_align16_ffa0     	= scalar_args[62];
	int l_input_heightxwidth_ffa0         	= scalar_args[64];
	int l_output_heightxwidth_ffa0         	= scalar_args[65];
	int l_filter_heightxwidth_ffa0         	= scalar_args[66];
	int l_filter_height_dil_ffa0         	= scalar_args[67];
	int l_istg_row_x_width_ffa0         	= scalar_args[69];
	int l_ostg_row_x_width_ffa0         	= scalar_args[70];
	int l_dil_filter_by_stride_ffa0     	= scalar_args[71];
	int l_dil_filter_mod_stride_ffa0     	= scalar_args[72];
	int l_pixproc_by_outwidth_ffa0         	= scalar_args[73];
	int l_pixproc_mod_outwidth_ffa0     	= scalar_args[74];
	int l_pixprocby2_by_outwidth_ffa0     	= scalar_args[75];
	int l_pixprocby2_mod_outwidth_ffa0     	= scalar_args[76];
	int l_compute_loop_count_ffa0         	= scalar_args[77];
	int l_input_1st_off_group             	= scalar_args[78];
	int l_input_other_off_group         	= scalar_args[79];
	int l_weight_off_group                 	= scalar_args[80];
	int l_output_off_group                 	= scalar_args[81];
	int l_bias_off_group                 	= scalar_args[82];
	int l_startrow_inc                     	= scalar_args[83];
	int l_in_size_x_plane16             	= scalar_args[84];
	int l_out_size_x_plane16             	= scalar_args[85];
	int l_row8_x_iw                     	= scalar_args[86];
	int l_conv3d_op_size                 = scalar_args[87];
	int l_ostg_row_cnt_last_itr_ffa0     	= scalar_args[88];
	int l_ostg_row_last_itr_x_width_ffa0	= scalar_args[89];
	int l_pc_loop_bound                 	= scalar_args[90];
	int l_pc_loop_bound_last_itr         	= scalar_args[91];
	int l_weights_loop_cnt                 	= scalar_args[92];
	int l_weights_offset_scale             	= scalar_args[93];
	int l_dil_filter_intdiv_stride_x_stride_ffa0 = scalar_args[94];
	int l_pixprocby2_by_outwidth_x_ow_ffa0 	= scalar_args[95];
	int l_crelu_out_offset0                 = scalar_args[96];
	int l_crelu_out_offset1                 = scalar_args[97];
	int l_instartRow                 = scalar_args[98];
	int l_inendRow                 = scalar_args[99];
	int l_avg_pool_mul                = scalar_args[100];
	int l_pool_ew                = scalar_args[101];
	int l_scale_offset                 = scalar_args[102];
	bool l_offline_quant_en   = scalar_args[103];
	conv_desc.in_start_row = (ap_int<16>)l_instartRow;
	conv_desc.in_end_row = (ap_int<16>)l_inendRow;
	conv_desc.avg_pool_mul = (ap_uint<8>)l_avg_pool_mul;
	conv_desc.pool_ew = (ap_uint<16>)l_pool_ew;
	conv_desc.scale_offset = l_scale_offset;
	conv_desc.offline_quant_en = l_offline_quant_en;


	conv_desc.rounding_conv = l_rounding_conv;
	conv_desc.rounding_bn = l_rounding_3dconv;
#if XI_BATCH1_EN==1
	conv_desc.batch1_int8_en = 1;//l_batch1_int8_en;
#else
	conv_desc.batch1_int8_en = 0;
#endif

	weight_desc.wts_loop_cnt = (ap_uint<14> ) l_weights_loop_cnt;
	weight_desc.wts_offset_scale = (ap_uint<24> ) l_weights_offset_scale;

	group_desc.input_1st_offset = l_input_1st_off_group;
	group_desc.input_other_offset = l_input_other_off_group;
	group_desc.weight_offset = l_weight_off_group;
	group_desc.output_offset = l_output_off_group;
	group_desc.bias_offset = l_bias_off_group;

	conv_desc.rounding_conv = l_rounding_conv;
	conv_desc.rounding_bn = l_rounding_3dconv;

	conv_desc.pc_loop_bound = l_pc_loop_bound;
	conv_desc.pc_loop_bound_last_itr = l_pc_loop_bound_last_itr;

	conv_desc.startrow_inc = (ap_uint<16> ) l_startrow_inc;
	input_desc.in_size_x_plane16 = (ap_uint<16> ) l_in_size_x_plane16;
	output_desc.out_size_x_plane16 = (ap_uint<16> ) l_out_size_x_plane16;
	output_desc.crelu_offset_ptr0 = l_crelu_out_offset0;
	output_desc.crelu_offset_ptr1 = l_crelu_out_offset1;
	input_desc.row8_x_iw = (ap_uint<16> ) l_row8_x_iw; 

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
	//**** 14- BN
	//**** 15- BN_SnB + C-Relu
	//**** 16- BN + C-Relu
	//**** 17- Scale + Relu
	//**** 18- Conv + Crelu
	//**** 19- 3DConv + Conv
	//**** 20- Maxpool
	//**** 21- Avg pool
	//**** 22- FC
	//**** 23- pool + Conv
	//**** 24- 3D conv
	ap_uint<8> opcode = (ap_uint<8> ) l_opcode;

	//************************** Opcode control signals ************************//
	//
	if (opcode == 4)
		conv_desc.bn_intra_sos_enable = 1;
	else
		conv_desc.bn_intra_sos_enable = 0;

	if (opcode == 5)
		conv_desc.bn_intra_mean_enable = 1;
	else
		conv_desc.bn_intra_mean_enable = 0;

	if (opcode == 6)
		conv_desc.lrn_intra_sos_enable = 1;
	else
		conv_desc.lrn_intra_sos_enable = 0;

	if (opcode == 7)
		conv_desc.lrn_inter_sos_enable = 1;
	else
		conv_desc.lrn_inter_sos_enable = 0;

	if (opcode == 8)
		conv_desc.l2_sos_enable = 1;
	else
		conv_desc.l2_sos_enable = 0;
#if !XI_DISABLE_SOS
	if ((opcode == 4) || (opcode == 6) || (opcode == 7) || (opcode == 8))//BN_intra_sos,  lrn inter and lrn inter, l2 norm
		conv_desc.sos_enable = 1;
	else
#endif
		conv_desc.sos_enable = 0;

	if (opcode == 9)
		conv_desc.lrn_pns_enable = 1;
	else
		conv_desc.lrn_pns_enable = 0;

	if (opcode == 10)
	{
		conv_desc.bn_snb_enable = 1;
		conv_desc.norm_flag_set[0] = 3;
		conv_desc.norm_flag_set[1] = 4;
		conv_desc.norm_flag_set[2] = 0;
		conv_desc.norm_flag_set[3] = 0;
		conv_desc.norm_loop_cnt = 1;

	}
	else
		conv_desc.bn_snb_enable = 0;

	if (opcode == 11)
	{
		conv_desc.elem_wise_add_en = 1;
		conv_desc.norm_flag_set[0] = 5;
		conv_desc.norm_flag_set[1] = 6;
		conv_desc.norm_flag_set[2] = 0;
		conv_desc.norm_flag_set[3] = 0;
		conv_desc.norm_loop_cnt = 2;
	}
	else
		conv_desc.elem_wise_add_en = 0;

	// BN SnB + Conv
	if (opcode == 12)
	{
		conv_desc.bn_conv_fuse_en = 1;
		conv_desc.norm_flag_set[0] = 7;
		conv_desc.norm_flag_set[1] = 0;
		conv_desc.norm_flag_set[2] = 0;
		conv_desc.norm_flag_set[3] = 0;
		conv_desc.norm_loop_cnt = 1;
	}
	else
		conv_desc.bn_conv_fuse_en = 0;

	//BN (x-u)/(sqrt(sigma))
	if (opcode == 14)
	{
		conv_desc.bn_en = 1;
		conv_desc.norm_flag_set[0] = 4;
		conv_desc.norm_flag_set[1] = 0;
		conv_desc.norm_flag_set[2] = 0;
		conv_desc.norm_flag_set[3] = 0;
		conv_desc.norm_loop_cnt = 1;
	}
	else
		conv_desc.bn_en = 0;

	//BN SnB + C-Relu
	if (opcode == 15)
	{
		conv_desc.bn_snb_c_relu_en = 1;
		conv_desc.norm_flag_set[0] = 8;
		conv_desc.norm_flag_set[1] = 0;
		conv_desc.norm_flag_set[2] = 0;
		conv_desc.norm_flag_set[3] = 0;
		conv_desc.norm_loop_cnt = 1;
	}
	else
		conv_desc.bn_snb_c_relu_en = 0;

	//BN + C-Relu
	if (opcode == 16)
	{
		conv_desc.bn_c_relu_en = 1;
		conv_desc.norm_flag_set[0] = 9;
		conv_desc.norm_flag_set[1] = 0;
		conv_desc.norm_flag_set[2] = 0;
		conv_desc.norm_flag_set[3] = 0;
		conv_desc.norm_loop_cnt = 1;
	}
	else
		conv_desc.bn_c_relu_en = 0;

	//scale + Relu
	if (opcode == 17)
	{
		conv_desc.scale_relu_en = 1;
		conv_desc.norm_flag_set[0] = 10;
		conv_desc.norm_flag_set[1] = 0;
		conv_desc.norm_flag_set[2] = 0;
		conv_desc.norm_flag_set[3] = 0;
		conv_desc.norm_loop_cnt = 1;
	}
	else
		conv_desc.scale_relu_en = 0;

	if (opcode == 18)
	{
		conv_desc.conv_c_relu_en = 1;
		conv_desc.conv_c_relu_loop_cnt = 2;
	}
	else
	{
		conv_desc.conv_c_relu_en = 0;
		conv_desc.conv_c_relu_loop_cnt = 1;
	}
	if(opcode == 19)
	{
		conv_desc.conv3d_intg_en = 1;
	}
	else{
		conv_desc.conv3d_intg_en = 0;
	}
	if(opcode == 20)
	{
		conv_desc.max_pool_en = 1;
	}
	else{
		conv_desc.max_pool_en = 0;
	}
	if(opcode == 21)
	{
		conv_desc.avg_pool_en = 1;
	}
	else{
		conv_desc.avg_pool_en = 0;
	}

	if(opcode == 22)
	{
		conv_desc.fc_enable = 1;
	}
	else{
		conv_desc.fc_enable = 0;
	}

	if(opcode == 23)
	{
		conv_desc.pool_fuse_en = 1;
	}
	else{
		conv_desc.pool_fuse_en = 0;
	}

	//
	//******************************* End *************************************//

	input_desc.width = (ap_uint<16> ) (l_input_Width_ffa0);
	input_desc.height = (ap_uint<16> ) (l_input_height_ffa0);
	input_desc.size = (ap_uint<32> ) (l_input_heightxwidth_ffa0);
	input_desc.planes = (ap_uint<16> ) (l_input_planes_align4_ffa0);
	input_desc.compute_planes = (ap_uint<16> ) l_compute_planes_align4_ffa0;
	input_desc.mean_value[0] = (short) l_mean_value0_ffa0;
	input_desc.mean_value[1] = (short) l_mean_value1_ffa0;
	input_desc.mean_value[2] = (short) l_mean_value2_ffa0;
	input_desc.mean_value[3] = (short) l_mean_value3_ffa0;
	input_desc.mean_sub_flag = l_mean_sub_flag_ffa0;

	output_desc.width = (ap_uint<16> ) l_output_width_ffa0;
	output_desc.height = (ap_uint<16> ) l_output_height_ffa0;
	output_desc.size = (ap_uint<32> ) (l_output_heightxwidth_ffa0);
	output_desc.planes = (ap_uint<16> ) (l_output_planes_align16_ffa0);
	output_desc.out_offset = l_out_offset_ffa0;
	output_desc.bias_offset = l_bias_offset_ffa0;

	weight_desc.num_kernels = (ap_uint<16> ) (l_output_planes_align16_ffa0);
	weight_desc.filter_size = (ap_uint<8> ) (l_filter_height_ffa0);
	weight_desc.filter_size_square = (ap_uint<8> ) l_filter_heightxwidth_ffa0;
	if (conv_desc.lrn_inter_sos_enable == 1)
	{
		weight_desc.nkpf = 1;
		weight_desc.filter_stride = 1;
	}
	else
	{
		weight_desc.nkpf = (ap_uint<4> ) (l_nkpf_ffa0);
		weight_desc.filter_stride = (ap_uint<4> ) (l_conv_stride_ffa0);
	}

	weight_desc.slk_counter = l_single_load_kernel_ffa0;
	weight_desc.weight_offset = l_weight_offset_ffa0;
	weight_desc.dilation_factor = (ap_uint<3> ) l_dilation_factor_ffa0;

	if (conv_desc.lrn_inter_sos_enable == 1)
		weight_desc.filter_size_dilated = 1;
	else
		weight_desc.filter_size_dilated = l_filter_height_dil_ffa0;

	conv_desc.group_en = (bool) (l_group_ffa0);
	if (conv_desc.conv_c_relu_en == 0)
	{
		conv_desc.relu_en = (bool) l_relu_ffa0;
	}
	else
	{
		conv_desc.relu_en = true;
	}

	if (l_layer_ffa0 == 0)
		conv_desc.layer_id = 0;
	else
		conv_desc.layer_id = 1;
	//    conv_desc.inout_precision = (ap_uint<5>)(l_inout_precision_ffa0-XI_SX);
	conv_desc.inout_precision = (ap_uint<5> ) (l_inout_precision_ffa0);
	conv_desc.shift_precision2 = (ap_uint<4> ) (l_shift_precision2_ffa0);
	conv_desc.pad_en = (bool) l_pad_ffa0;

	if (conv_desc.lrn_inter_sos_enable == 1)
	{
		conv_desc.pad_num = 0;
	}
	else
	{
		conv_desc.pad_num = (ap_uint<8> ) l_pad_ffa0;
	}
	conv_desc.istg_row_count = (ap_uint<16> ) (l_istg_row_cnt_ffa0);
	conv_desc.ostg_row_count = (ap_uint<16> ) (l_ostg_row_cnt_ffa0);
	conv_desc.ostg_row_cnt_last_itr = (ap_uint<16> ) (l_ostg_row_cnt_last_itr_ffa0);
	conv_desc.straddle = l_straddle_factor_ffa0;
	conv_desc.iw_mul_istg_row_count = l_istg_row_x_width_ffa0;
	conv_desc.ow_mul_ostg_row_count = l_ostg_row_x_width_ffa0;
	conv_desc.ow_mul_ostg_row_cnt_last_itr = l_ostg_row_last_itr_x_width_ffa0;
	conv_desc.fsz_by_stride = (ap_uint<8> ) l_dil_filter_by_stride_ffa0;
	conv_desc.fsz_mod_stride = (ap_uint<8> ) l_dil_filter_mod_stride_ffa0;
	conv_desc.read_from_ddr = (bool) l_read_from_ddr;
	conv_desc.write_to_ddr = (bool) l_write_to_ddr;

	conv_desc.fsz_by_stride_x_stride = (ap_uint<8> ) l_dil_filter_intdiv_stride_x_stride_ffa0;

	conv_desc.norm_k = (ap_int<16> ) l_norm_k;

	conv_desc.mul_in = (ap_uint<18> ) l_mul_in;
	conv_desc.norm_prec = (ap_uint<5> ) l_norm_prec;
	conv_desc.out_pix = l_ostg_row_x_width_ffa0;

	if ((opcode == 4) || (opcode == 5) || (opcode == 6))//BN_intra sos, BN_intra mean, LRN intra sos
		conv_desc.mac_loop_count = 4;
	else
		// lrn inter , conv, l2 norm
		conv_desc.mac_loop_count = 1;

	conv_desc.opcode = opcode;
	conv_desc.norm_prec_2 = (ap_uint<5> ) l_norm_prec_2;
	conv_desc.norm_prec_3 = (ap_uint<5> ) l_norm_prec_3;

	if (conv_desc.batch1_int8_en == 0)
	{
		conv_desc.int6_en_in = l_int6_en_in;
		conv_desc.int6_en_out = l_int6_en_out;
	}
	else
	{
		conv_desc.int6_en_in = 1;
		conv_desc.int6_en_out = 0;
	}

	if (opcode == 6 || opcode == 7 || opcode == 8)//LRN_intersos, LRN_intrasos, l2_sos
		conv_desc.pix_per_kp = 6;
	else if (opcode == 4 || opcode == 5)    //BN man, BN sos
		conv_desc.pix_per_kp = 1;
	else //conv, convRLU, DilConv, DilConvRLU
	{
		if (conv_desc.int6_en_in == 1)
			conv_desc.pix_per_kp = XI_PIX_PROC;
		else
			conv_desc.pix_per_kp = XI_PIX_PROC / 2;
	}


	if (conv_desc.int6_en_in == 1)
		conv_desc.int8_2x_loopbound = 1;
	else
		conv_desc.int8_2x_loopbound = 2;

	if (opcode == 6) //LRN_intersos
		conv_desc.loop_bound_1x1_filter = 1;
	else
		conv_desc.loop_bound_1x1_filter = input_desc.compute_planes.range(15,
				3);

	if (conv_desc.lrn_inter_sos_enable)
	{
		conv_desc.feeding_buff_plane_loop_bound = 16;
		conv_desc.feeding_buff_row_loop_bound = 1;
	}
	else //conv, convRLU, DilConv, DilConvRLU
	{
		conv_desc.feeding_buff_plane_loop_bound = input_desc.compute_planes.range(15, 2);
		conv_desc.feeding_buff_row_loop_bound = weight_desc.filter_size;
	}

	if (conv_desc.lrn_inter_sos_enable == 1 || conv_desc.lrn_intra_sos_enable == 1)
	{
		conv_desc.straddle_in_plane_inc_by4 = XI_KER_PROC / 4;
	}
	else //conv
	{
		conv_desc.straddle_in_plane_inc_by4 = input_desc.compute_planes.range(11, 2);
	}

	conv_desc.pix_by_outwidth = l_pixproc_by_outwidth_ffa0;
	conv_desc.pix_mod_outwidth = l_pixproc_mod_outwidth_ffa0;
	conv_desc.pix2_div_outwidth = l_pixprocby2_by_outwidth_ffa0;
	conv_desc.pix2_div_outwidth_x_ow = l_pixprocby2_by_outwidth_x_ow_ffa0;
	conv_desc.pix2_mod_outwidth = l_pixprocby2_mod_outwidth_ffa0;

	if (conv_desc.lrn_inter_sos_enable == 1)
		conv_desc.compute_loop_count = 4; //16 feature map to accumulate
	else if (conv_desc.lrn_intra_sos_enable == 1)
		conv_desc.compute_loop_count = weight_desc.filter_size_square;
	else
		conv_desc.compute_loop_count = l_compute_loop_count_ffa0; //weight_desc.filter_size_square*(input_desc.compute_planes.range(11,2));

	conv_desc.intra_en = (conv_desc.bn_intra_mean_enable || conv_desc.bn_intra_sos_enable || conv_desc.lrn_intra_sos_enable);

	if (conv_desc.sos_enable == 1)
	{
		conv_desc.ker_loop_cnt = 1;
	}
	else
	{
		conv_desc.ker_loop_cnt = weight_desc.num_kernels;
	}

	ap_uint<10> accum_fil_size = 0;
#pragma HLS ARRAY_PARTITION variable=conv_desc.feed_addr16 complete dim=0
	for (ap_uint<5> pix_load = 0; pix_load < 16; pix_load++)
	{
#pragma HLS pipeline
		conv_desc.feed_addr16[pix_load] = accum_fil_size; //pix_load*weight_desc.filter_size_square;
		accum_fil_size += weight_desc.filter_size_square;
	}

	conv_desc.conv3d_inp_offset = (ap_uint<10>)l_conv_3d_inp_offset;
	conv_desc.conv3d_ip_h    = (ap_uint<16>)    l_conv_3d_ip_h;
	conv_desc.conv3d_ip_w    = (ap_uint<16>)    l_conv_3d_ip_w;
	conv_desc.conv3d_op_h    = (ap_uint<16>)     l_conv_3d_op_h;
	conv_desc.conv3d_op_w    = (ap_uint<16>)     l_conv_3d_op_w;
	conv_desc.conv3d_ft_h    = (ap_uint<8>)     l_conv_3d_ft_h;
	conv_desc.conv3d_ft_w    = (ap_uint<8>)     l_conv_3d_ft_w;
	conv_desc.conv3d_fsz2    = (ap_uint<8>)     l_conv_3d_fsz2;
	conv_desc.conv3d_ip_pln    = (ap_uint<16>)     l_conv_3d_ip_pln;
	conv_desc.conv3d_op_pln    = (ap_uint<16>)  l_conv_3d_op_pln;
	conv_desc.conv3d_pad    = (ap_uint<8>) l_conv_3d_pad;
	conv_desc.conv3d_stride    = (ap_uint<3>) l_conv_3d_stride;
	conv_desc.conv3d_op_rows= (ap_uint<10>) l_conv_3d_ostg_row_cnt;
	conv_desc.conv3d_outshift = (ap_uint<6>)l_conv_3d_outshift;
	conv_desc.conv3d_relu_en = (bool) l_conv_3d_relu;
	conv_desc.conv3d_ip_size = (ap_uint<32>)l_conv_3d_ip_size;
	conv_desc.conv3d_istg_row_cnt = (ap_uint<10>)l_conv_3d_istg_row_cnt;
	conv_desc.conv3d_rndval = (ap_int<24>)l_rounding_3dconv;
	conv_desc.conv3d_op_size = (ap_uint<32>) l_conv3d_op_size;
	//****************************NOTE*******************
	//in BN intra assume, input height and width < 16 and filter size is same as input size
	//L2 norm, same as conv but we have to use 6 pix_proc instead of 52 pix_proc

}

void reg_addr_istg_rd_nrm(ap_uint<16> *istg_addr, ap_uint<16> *istg_addr_reg)
{
#pragma HLS interface register port=istg_addr_reg
#pragma HLS inline off

	*istg_addr_reg = *istg_addr;


}

void reg_addr_istg_wrt_nrm(ap_uint<16> *istg_addr, ap_uint<16> *istg_addr_reg)
{
#pragma HLS interface register port=istg_addr_reg
#pragma HLS inline off

	*istg_addr_reg = *istg_addr;


}

void reg_addr_istg_o2i(ap_uint<14> *istg_addr, ap_uint<14> *istg_addr_reg)
{
#pragma HLS interface register port=istg_addr_reg
#pragma HLS inline off

	*istg_addr_reg = *istg_addr;


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
	I_hw_ffb0 = input_desc.size;
	int I_hwp_ffb0;
#pragma HLS RESOURCE variable=I_hwp_ffb0 core=MulnS latency=2
	I_hwp_ffb0 = I_hw_ffb0 * input_desc.planes;

	int K_nkp_ffb0;
#pragma HLS RESOURCE variable=K_nkp_ffb0 core=MulnS latency=2
	K_nkp_ffb0 = weight_desc.num_kernels * input_desc.planes;
	int K_nkpfsz2_ffb0;
#pragma HLS RESOURCE variable=K_nkpfsz2_ffb0 core=MulnS latency=2
#if XI_KER_PROC==8 || XI_KER_PROC==16
	K_nkpfsz2_ffb0 = K_nkp_ffb0*(weight_desc.filter_size_square);
#endif

	int O_hw_ffb0;
#pragma HLS RESOURCE variable=O_hw_ffb0 core=MulnS latency=2
	O_hw_ffb0 = output_desc.height * output_desc.width;
	int O_hwp_ffb0;
#pragma HLS RESOURCE variable=O_hwp_ffb0 core=MulnS latency=2
	O_hwp_ffb0 = O_hw_ffb0 * output_desc.planes;

	if (conv_desc.group_en == 0)
	{
		input_group_offset_1st_fa0 = 0;
		input_group_offset_other_fa0 = 0;
		weights_group_offset_fa0 = 0;
		output_group_offset_fa0 = 0;
		bias_group_offset_fa0 = 0;
	}
	else
	{
		input_group_offset_1st_fa0 = (I_hwp_ffb0) >> XI_INPUTPACKCOUNT_LOG2;
		input_group_offset_other_fa0 = (I_hwp_ffb0)>> (XI_INPUTPACKCOUNT2_LOG2);
		output_group_offset_fa0 = (O_hwp_ffb0) >> (XI_OUTPUTPACKCOUNT_LOG2);
		weights_group_offset_fa0 = (K_nkpfsz2_ffb0)>> (XI_WEIGHTPACKCOUNT_LOG2);
		bias_group_offset_fa0 = (output_desc.planes) >> XI_BIASPACKCOUNT_LOG2;
	}

}
#if XI_ODBC_EN
void CopyOstgToIstg(ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<72> ostaging_buff0_fb0[2][8][XI_OSTAGEBUFF_DEPTH],
		conv_struct conv_desc,
		output_struct output_desc)
{
#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=1
#if XI_ISTG_URAM_EN==0
#pragma HLS resource variable=istaging_buff0_fb0 core=RAM_T2P_BRAM latency=2
#else
#pragma HLS RESOURCE variable=istaging_buff0_fb0 core=XPM_MEMORY uram
#endif

#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2
#if XI_OSTG_URAM_EN==0
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM latency=2
#else
#pragma HLS RESOURCE variable=ostaging_buff0_fb0 core=XPM_MEMORY uram
#endif
#pragma HLS INLINE OFF

	ap_uint<16> loop_bound = output_desc.size;
	copy_to_istg_loop1:
	for (ap_uint<16> planes = 0; planes < output_desc.planes; planes += 32)
	{
		copy_to_istg_loop2:
		for (ap_uint<16> size = 0; size < loop_bound;size++)
		{

#pragma HLS PIPELINE
#pragma HLS loop_flatten

			ap_uint<16> bram_depth = size + (planes / 32) * loop_bound;
			//ap_uint<16> ostg_bram_depth = size + (planes/32) *conv_desc.out_pix;

			copy_to_istg_loop3:
			for (int dim2 = 0; dim2 < 8; dim2++)
			{
#pragma HLS UNROLL
				for (int dim1 = 0, bit=0, bit2=0; dim1 < 4; dim1++, bit+=16, bit2+=18)
				{
#pragma HLS UNROLL

					ap_int<18> temp, temp1;
					ap_int<18> temp_shift, temp1_shift;
					temp = ostaging_buff0_fb0[0][dim2][bram_depth].range(bit2+17, bit2);
					temp1 = ostaging_buff0_fb0[1][dim2][bram_depth].range(bit2+17, bit2);
					temp_shift = temp >> conv_desc.inout_precision;
					temp1_shift = temp1 >> conv_desc.inout_precision;

					ap_int<8> word, word1;
					ap_int<16> word_relu;

					if (conv_desc.int6_en_out == 0)
					{
						if (temp_shift > 127)
							word = 127;
						else if (temp_shift < -128)
							word = -128;
						else
							word = (ap_int<8> ) temp_shift;
					}
					else
					{
						if (temp_shift > 31)
							word = 31;
						else if (temp_shift < -32)
							word = -32;
						else
							word = (ap_int<8> ) temp_shift;
					}

					if (conv_desc.int6_en_out == 0)
					{
						if (temp1_shift > 127)
							word1 = 127;
						else if (temp1_shift < -128)
							word1 = -128;
						else
							word1 = (ap_int<8> ) temp1_shift;

					}
					else
					{
						if (temp1_shift > 31)
							word1 = 31;
						else if (temp1_shift < -32)
							word1 = -32;
						else
							word1 = (ap_int<8> ) temp1_shift;

					}

					if ((conv_desc.relu_en == 1) && (temp_shift[17] == 1))
						word_relu.range(7, 0) = 0;
					else
						word_relu.range(7, 0) = word;//word_neg_fh0 & mask_int6;

					if ((conv_desc.relu_en == 1) && (temp1_shift[17] == 1))
						word_relu.range(15, 8) = 0;
					else
						word_relu.range(15, 8) = word1;    //word2_neg_fh0 & mask_int6;

					istaging_buff0_fb0[dim2][bram_depth].range(bit+15,bit) = word_relu;

				}    //dim2
			}    //dim1

		} //copy_to_istg_loop
	}

}
#endif
template<int CFILTER_SIZE, int CCONV_STRIDE, int CNUM_KERNELS, int IINPUT_PLANES, int PNKPF>
void LoadKernels_fz(weight_struct weight_desc,
		weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],
		gmem_weighttype *gmem_weight1_fa0, gmem_weighttype *gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
		gmem_weighttype *gmem_weight3_fa0,
		gmem_weighttype *gmem_weight4_fa0,
#endif
		ap_uint<14> weight_loopCount_fd0) {
#pragma HLS INLINE OFF
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=2
#if XI_MERGE_WEIGHTBUFF
#pragma HLS ARRAY_RESHAPE variable=weight_buff0_fd0 complete dim=1
#else
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=1
#endif
#if XI_WTS_URAM_EN==0
#pragma HLS resource variable=weight_buff0_fd0 latency=4
#else
#pragma HLS RESOURCE variable=weight_buff0_fd0 core=XPM_MEMORY uram
#endif

#if XI_KER_PROC==16

	Weight_Loop:
	for(ap_uint<14> ddrpntr_fz0=0;ddrpntr_fz0< weight_loopCount_fd0;ddrpntr_fz0++)
	{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=128 max=128

		ap_uint<512> weight_512bit_fz0;
		weight_512bit_fz0.range(127,  0) = gmem_weight1_fa0[ddrpntr_fz0];
		weight_512bit_fz0.range(255,128) = gmem_weight2_fa0[ddrpntr_fz0];
		weight_512bit_fz0.range(383,256) = gmem_weight3_fa0[ddrpntr_fz0];
		weight_512bit_fz0.range(511,384) = gmem_weight4_fa0[ddrpntr_fz0];

		Weighttype_Loop:
		for(ap_uint<10> word_cnt_fz0=0,bit=0;word_cnt_fz0<64;word_cnt_fz0++,bit+=8)
		{
#pragma HLS UNROLL
			weight_buff0_fd0[word_cnt_fz0.range(5,2)][word_cnt_fz0.range(1,0)][ddrpntr_fz0] = weight_512bit_fz0.range((bit+7),bit);
		} //Weighttype_Loop

	} //Weight_Loop

#elif XI_KER_PROC==8

	Weight_Loop:
	for(ap_uint<14> ddrpntr_fz0=0;ddrpntr_fz0< weight_loopCount_fd0;ddrpntr_fz0++)
	{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=128 max=128

		ap_uint<256> weight_256bit_fz0;
#if XI_WTS_PORT_64BIT_EN==0
		weight_256bit_fz0.range(127,0) 	 = gmem_weight1_fa0[ddrpntr_fz0];
		weight_256bit_fz0.range(255,128) = gmem_weight2_fa0[ddrpntr_fz0];
#else
		weight_256bit_fz0.range(63,0) 	 = gmem_weight1_fa0[ddrpntr_fz0];
		weight_256bit_fz0.range(127,64)  = gmem_weight2_fa0[ddrpntr_fz0];
		weight_256bit_fz0.range(191,128) = gmem_weight3_fa0[ddrpntr_fz0];
		weight_256bit_fz0.range(255,192) = gmem_weight4_fa0[ddrpntr_fz0];
#endif

		Weighttype_Loop:
		for(ap_uint<10> word_cnt_fz0=0,bit=0;word_cnt_fz0<32;word_cnt_fz0++,bit+=8)
		{
#pragma HLS UNROLL
			char a = weight_256bit_fz0.range((bit+7),bit);
			weight_buff0_fd0[word_cnt_fz0.range(4,2)][word_cnt_fz0.range(1,0)][ddrpntr_fz0] = weight_256bit_fz0.range((bit+7),bit);
		} //Weighttype_Loop

	} //Weight_Loop
#endif//XI_KER_PROC

}

template<int CFILTER_SIZE, int CCONV_STRIDE, int CNUM_KERNELS, int IINPUT_PLANES, int PNKPF>
void LoadKernelsEn_fz(weight_struct weight_desc,
		weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],
		gmem_weighttype *gmem_weight1_fa0, gmem_weighttype *gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
		gmem_weighttype *gmem_weight3_fa0,
		gmem_weighttype *gmem_weight4_fa0,
#endif
		ap_uint<14> weight_loopCount_fd0, bool lk_enable) {
#pragma HLS INLINE OFF

	if (lk_enable == 1) {
		LoadKernels_fz<CFILTER_SIZE, CCONV_STRIDE, CNUM_KERNELS,
		(IINPUT_PLANES >> 2), PNKPF>(weight_desc, weight_buff0_fd0,
				gmem_weight1_fa0, gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
				gmem_weight3_fa0, gmem_weight4_fa0,
#endif
				weight_loopCount_fd0);
#if XI_DP_ENABLE
		for(ap_uint<5> ker=0;ker<XI_KER_PROC;ker++)
		{
#pragma HLS unroll
			for(ap_uint<3> pl=0;pl<4;pl++)
			{
#pragma HLS unroll
				weight_buff0_fd0[ker][pl][weight_loopCount_fd0] = 0;
			}
		}
#endif
	}
}

void sat_24to18bit(ap_int<24> in, ap_int<18> *out)
{
#pragma HLS interface register port=out
#pragma HLS inline off
	ap_int<7> sat_test  = (ap_int<7> )in.range(23,17);
	ap_int<18> res18bit = (ap_int<18> )in.range(17,0);
	ap_int<18> res;
	if(sat_test[6]==0 && sat_test.range(5,0)!=0)
		res=131071;
	else if(sat_test[6]==1 && sat_test.range(5,0)!=63)
		res=-131072;
	else
		res=res18bit;

	*out = res;
}

void sat_20to18bit(ap_int<20> in, ap_int<18> *out)
{
#pragma HLS interface register port=out
#pragma HLS inline off
	ap_int<3> sat_test  = (ap_int<3> )in.range(19,17);
	ap_int<18> res18bit = (ap_int<18> )in.range(17,0);
	ap_int<18> res;
	if(sat_test[2]==0 && sat_test.range(1,0)!=0)
		res=131071;
	else if(sat_test[2]==1 && sat_test.range(1,0)!=3)
		res=-131072;
	else
		res=res18bit;

	*out = res;
}

#if XI_DP_ENABLE

//#include "compute16ker_dp.hpp"
void pix_proc_fn(ap_int<6> a1in_1,ap_int<6> a2in_1,ap_int<8> b1in_1,ap_int<8> b2in_1,ap_int<8> c1in_1,ap_int<8> c2in_1,
		ap_int<6> a1in_2,ap_int<6> a2in_2,ap_int<8> b1in_2,ap_int<8> b2in_2,ap_int<8> c1in_2,ap_int<8> c2in_2,
		ap_int<42> *mac_res_temp_0,ap_int<42> *mac_res_temp_1,
		bool ap_clk_div2,
		bool clear)
{
#pragma HLS INLINE OFF

	ap_int<42> mac_0,mac_1;
	mac_0 = *mac_res_temp_0;
	mac_1 = *mac_res_temp_1;
	mac_0 = __builtin_mac6x2_mac8x1(a1in_1,a2in_1,b1in_1,b2in_1,c1in_1,c2in_1,mac_0,clear,ap_clk_div2);
	mac_1 = __builtin_mac6x2_mac8x1(a1in_2,a2in_2,b1in_2,b2in_2,c1in_2,c2in_2,mac_1,clear,ap_clk_div2);

	*mac_res_temp_0 = mac_0;
	*mac_res_temp_1 = mac_1;
}

void ker_proc_fn(conv_struct conv_desc,ap_uint<8> ker,ap_uint<8> pix1,ap_uint<8> pix2,short input_buf_copy_fy0[4][XI_PIX_PROC],weight_width k_buf_copy_fy0[4],ap_int<42> mac_res_temp_fy0[XI_PIX_PROC][2],bool clear, bool ap_clk_div2)
{
#pragma HLS INLINE OFF

	char input_mux[4];
#pragma HLS ARRAY_PARTITION variable=input_mux complete dim=0
	ap_int<16> input_reg[4];
#pragma HLS ARRAY_PARTITION variable=input_reg complete dim=0

	Pix_Loop3:
	for(int pixel_fy2=0; pixel_fy2<XI_PIX_PROC;pixel_fy2++)
	{
#pragma HLS unroll
		Pln_Loop3:
		for(int plane_fy3=0; plane_fy3<4;plane_fy3++)
		{
#pragma HLS unroll
			//if(pixel_fy2==14 && ker==2 && inp_buff_addr1_fy0==47)
			//int ghg=0;

			if((pixel_fy2>=pix1) && (pixel_fy2<pix2))
			{
				if(conv_desc.sos_enable == 1)
					input_mux[plane_fy3] = input_buf_copy_fy0[plane_fy3][pixel_fy2];
				else
					input_mux[plane_fy3] = k_buf_copy_fy0[plane_fy3];
			}
			else
			{
				input_mux[plane_fy3] = k_buf_copy_fy0[plane_fy3];
			}

			input_reg[plane_fy3] = input_buf_copy_fy0[plane_fy3][pixel_fy2];

		} //pln loop

		ap_int<6> a1in_1,a2in_1,a1in_2,a2in_2;
		ap_int<8> b1in_1,b2in_1,b1in_2,b2in_2,c1in_1,c1in_2,c2in_1,c2in_2;
		a1in_1 = input_reg[0].range(13,8); a2in_1 = input_reg[1].range(13,8);
		a1in_2 = input_reg[2].range(13,8); a2in_2 = input_reg[3].range(13,8);
		b1in_1 = input_reg[0].range(7,0); b2in_1 = input_reg[1].range(7,0);
		b1in_2 = input_reg[2].range(7,0); b2in_2 = input_reg[3].range(7,0);
		c1in_1 = input_mux[0];c2in_1 =  input_mux[1];
		c1in_2 = input_mux[2];c2in_2 =  input_mux[3];
		pix_proc_fn(a1in_1, a2in_1, b1in_1,b2in_1, c1in_1, c2in_1,
				a1in_2,a2in_2, b1in_2, b2in_2, c1in_2, c2in_2,
				&mac_res_temp_fy0[pixel_fy2][0],&mac_res_temp_fy0[pixel_fy2][1],
				ap_clk_div2,
				clear);

	}//Pix_Loop2

}

void mux_sel_reg(ap_uint<8> pixel_fy3[XI_KER_PROC], ap_uint<4> pixel_fy4, ap_uint<8> pix8_fy0)
{
#pragma HLS interface register port=pixel_fy3
#pragma HLS interface register port=pixel_fy4
#pragma HLS interface register port=pix8_fy0
#pragma HLS ARRAY_PARTITION variable=pixel_fy3 complete dim=0
#pragma HLS inline off

	ap_uint<8> pixel_add = pixel_fy4 + pix8_fy0;

	for(ap_uint<6> i=0;i<XI_KER_PROC;i++)
	{
#pragma HLS UNROLL
		pixel_fy3[i] = pixel_add;
	}
}

void treeAdd_sat24to18bit(ap_int<24> in0, ap_int<24> in1, ap_int<18> *result)
{
#pragma HLS interface register port=in0
#pragma HLS interface register port=in1
#pragma HLS interface register port=result
#pragma HLS inline off

	ap_int<24> add_out = in1 + in0;

	ap_int<7> sat_test  = (ap_int<7> )add_out.range(23,17);
	ap_int<18> res18bit = (ap_int<18> )add_out.range(17,0);
	ap_int<18> res;
	if(sat_test[6]==0 && sat_test.range(5,0)!=0)
		res=131071;
	else if(sat_test[6]==1 && sat_test.range(5,0)!=63)
		res=-131072;
	else
		res=res18bit;

	*result = res;
}

void treeAdd_sat20to18bit(ap_int<20> in0, ap_int<20> in1, ap_int<18> *result)
{
#pragma HLS interface register port=in0
#pragma HLS interface register port=in1
#pragma HLS interface register port=result
#pragma HLS inline off
	ap_int<20> add_out = in1 + in0;

	ap_int<3> sat_test  = (ap_int<3> )add_out.range(19,17);
	ap_int<18> res18bit = (ap_int<18> )add_out.range(17,0);
	ap_int<18> res;
	if(sat_test[2]==0 && sat_test.range(1,0)!=0)
		res=131071;
	else if(sat_test[2]==1 && sat_test.range(1,0)!=3)
		res=-131072;
	else
		res=res18bit;

	*result = res;
}

void mux_out_reg(conv_struct conv_desc, ap_int<42> mac_out[XI_KER_PROC][2], ap_int<24> add_in_24bit_img0[XI_KER_PROC][2], ap_int<20> add_in_20bit_img1[XI_KER_PROC][2])
{
#pragma HLS interface register port=mac_out
#pragma HLS interface register port=add_in_24bit_img0
#pragma HLS interface register port=add_in_20bit_img1
#pragma HLS ARRAY_PARTITION variable=mac_out complete dim=0
#pragma HLS ARRAY_PARTITION variable=add_in_24bit_img0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=add_in_20bit_img1 complete dim=0
#pragma HLS inline off
	for(ap_uint<6> ker=0; ker<XI_KER_PROC; ker++)
	{
#pragma HLS UNROLL
		for(ap_uint<3> pl=0; pl<2; pl++)
		{
#pragma HLS UNROLL
			if(conv_desc.int6_en_in == 0 || conv_desc.batch1_int8_en == 1)
			{
				add_in_24bit_img0[ker][pl] = (ap_int<24>)mac_out[ker][pl].range(23,0);
			}
			else
			{
				add_in_24bit_img0[ker][pl] = (ap_int<24>)((ap_int<20>)mac_out[ker][pl].range(19,0));
			}
			add_in_20bit_img1[ker][pl] = (ap_int<20>)mac_out[ker][pl].range(39,20) + mac_out[ker][pl][19];
		}
	}
}

template<int CFILTER_SIZE, int CCONV_STRIDE, int CNUM_KERNELS, int IINPUT_PLANES, int PNKPF>
void Compute16Ker_fy(conv_struct conv_desc,
		weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],
		ap_uint<64> input_buff0_fc0[XI_PIX_PROC / 2][1024],
		ap_uint<4> nkpf_cnt_fe0,
		ap_int<36> result_ping_fe0[XI_KER_PROC][XI_PIX_PROC],
		ap_uint<16> nk_process_fd0,
		out_pix_struct out_pixels0_fc0,
		output_struct output_desc,
		ap_uint<14> weight_buff_offset,
		bool ap_clk_div2)
{

#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#if XI_FEED_URAM_EN==0
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=input_buff0_fc0 core=XPM_MEMORY uram
#endif


#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=2
#if XI_MERGE_WEIGHTBUFF
#pragma HLS ARRAY_RESHAPE variable=weight_buff0_fd0 complete dim=1
#else
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=1
#endif
#if XI_WTS_URAM_EN==0
#pragma HLS resource variable=weight_buff0_fd0 latency=4
#else
#pragma HLS RESOURCE variable=weight_buff0_fd0 core=XPM_MEMORY uram
#endif
#if XI_RESULT_BUFFER_FF
#pragma HLS ARRAY_PARTITION variable=result_ping_fe0 complete dim=0
#elif XI_RESULT_BUFFER_LUTRAM
#pragma HLS ARRAY_PARTITION variable=result_ping_fe0 complete dim=1
#pragma HLS resource variable=result_ping_fe0 latency=4 core=RAM_2P_LUTRAM
#else
#pragma HLS ARRAY_PARTITION variable=result_ping_fe0 complete dim=1
#pragma HLS resource variable=result_ping_fe0 latency=4 core=RAM_T2P_BRAM
#endif

#pragma HLS INLINE off

	ap_uint<16> outputkernelid_fx0 = nk_process_fd0 + nkpf_cnt_fe0 * XI_KER_PROC;

	ap_int<42> mac_res_temp_fy0_0[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_0 complete dim=0

	ap_int<42> mac_res_temp_fy0_1[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_1 complete dim=0

	ap_int<42> mac_res_temp_fy0_2[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_2 complete dim=0

	ap_int<42> mac_res_temp_fy0_3[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_3 complete dim=0

	ap_int<42> mac_res_temp_fy0_4[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_4 complete dim=0

	ap_int<42> mac_res_temp_fy0_5[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_5 complete dim=0

	ap_int<42> mac_res_temp_fy0_6[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_6 complete dim=0

	ap_int<42> mac_res_temp_fy0_7[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_7 complete dim=0
#if XI_KER_PROC==16
	ap_int<42> mac_res_temp_fy0_8[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_8 complete dim=0

	ap_int<42> mac_res_temp_fy0_9[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_9 complete dim=0

	ap_int<42> mac_res_temp_fy0_10[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_10 complete dim=0

	ap_int<42> mac_res_temp_fy0_11[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_11 complete dim=0

	ap_int<42> mac_res_temp_fy0_12[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_12 complete dim=0

	ap_int<42> mac_res_temp_fy0_13[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_13 complete dim=0

	ap_int<42> mac_res_temp_fy0_14[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_14 complete dim=0

	ap_int<42> mac_res_temp_fy0_15[XI_PIX_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0_15 complete dim=0
#endif

	ap_uint<12> k_off_cnt_fy0;
	k_off_cnt_fy0 = weight_buff_offset; //(conv_desc.compute_loop_count*nkpf_cnt_fe0);

	bool clear;

#if DEBUG_COMPUTE
	fprintf(fp_input,"\n Current Plane id:%d\n",(int)current_plane);
	fprintf(fp_weight,"\n Current Plane id:%d\n",(int)current_plane);
#endif

	Compute_Loop:
	for (ap_uint<12> inp_buff_addr1_fy0 = 0, inp_buff_addr2_fy0 = 512, offset_kbuf_fy0 = k_off_cnt_fy0;inp_buff_addr1_fy0 < conv_desc.compute_loop_count+1;
			/*..continue..*/inp_buff_addr1_fy0++, inp_buff_addr2_fy0++, offset_kbuf_fy0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=16 max=16
#pragma HLS PIPELINE

		if(inp_buff_addr1_fy0==0)
			clear = 1;
		else
			clear = 0;
		short input_buf_copy_fy0[4][XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_fy0 complete dim=0

		Pln_Loop1:
		for (ap_uint<8> plane_fy1 = 0, bit=0; plane_fy1 < 4; plane_fy1++,bit+=16)
		{
#pragma HLS unroll
			Pix_Loop1:
			for(ap_uint<8> pixel_fy1=0, pix_split_fy0 =0; pixel_fy1<(XI_PIX_PROC/2);
					pixel_fy1++, pix_split_fy0++)
			{
#pragma HLS unroll
				input_buf_copy_fy0[plane_fy1][pix_split_fy0] 		     = input_buff0_fc0[pixel_fy1][inp_buff_addr1_fy0].range(bit+15,bit);
				input_buf_copy_fy0[plane_fy1][pix_split_fy0+(XI_PIX_PROC/2)] = input_buff0_fc0[pixel_fy1][inp_buff_addr2_fy0].range(bit+15,bit);
			} //Pix_Loop1
		} //Pln_Loop1


		weight_width k_buf_copy_fy0_0[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_0 complete dim=0

		weight_width k_buf_copy_fy0_1[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_1 complete dim=0

		weight_width k_buf_copy_fy0_2[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_2 complete dim=0

		weight_width k_buf_copy_fy0_3[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_3 complete dim=0

		weight_width k_buf_copy_fy0_4[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_4 complete dim=0

		weight_width k_buf_copy_fy0_5[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_5 complete dim=0

		weight_width k_buf_copy_fy0_6[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_6 complete dim=0

		weight_width k_buf_copy_fy0_7[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_7 complete dim=0
#if XI_KER_PROC==16
		weight_width k_buf_copy_fy0_8[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_8 complete dim=0

		weight_width k_buf_copy_fy0_9[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_9 complete dim=0

		weight_width k_buf_copy_fy0_10[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_10 complete dim=0

		weight_width k_buf_copy_fy0_11[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_11 complete dim=0

		weight_width k_buf_copy_fy0_12[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_12 complete dim=0

		weight_width k_buf_copy_fy0_13[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_13 complete dim=0

		weight_width k_buf_copy_fy0_14[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_14 complete dim=0

		weight_width k_buf_copy_fy0_15[4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0_15 complete dim=0
#endif

		Pln_Loop2:
		for (ap_uint<3> plane_fy2 = 0; plane_fy2 < 4;plane_fy2++)
		{
#pragma HLS unroll
			k_buf_copy_fy0_0[plane_fy2] = weight_buff0_fd0[0][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_1[plane_fy2] = weight_buff0_fd0[1][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_2[plane_fy2] = weight_buff0_fd0[2][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_3[plane_fy2] = weight_buff0_fd0[3][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_4[plane_fy2] = weight_buff0_fd0[4][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_5[plane_fy2] = weight_buff0_fd0[5][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_6[plane_fy2] = weight_buff0_fd0[6][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_7[plane_fy2] = weight_buff0_fd0[7][plane_fy2][offset_kbuf_fy0];
#if XI_KER_PROC==16
			k_buf_copy_fy0_8[plane_fy2] = weight_buff0_fd0[8][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_9[plane_fy2] = weight_buff0_fd0[9][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_10[plane_fy2] = weight_buff0_fd0[10][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_11[plane_fy2] = weight_buff0_fd0[11][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_12[plane_fy2] = weight_buff0_fd0[12][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_13[plane_fy2] = weight_buff0_fd0[13][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_14[plane_fy2] = weight_buff0_fd0[14][plane_fy2][offset_kbuf_fy0];
			k_buf_copy_fy0_15[plane_fy2] = weight_buff0_fd0[15][plane_fy2][offset_kbuf_fy0];
#endif
		} //Pln_Loop2
#if DEBUG_COMPUTE
		fprintf(stderr,"\n");
#endif
		//***********************  KP
		ker_proc_fn(conv_desc,0,0,3,input_buf_copy_fy0,k_buf_copy_fy0_0,mac_res_temp_fy0_0,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,1,3,6,input_buf_copy_fy0,k_buf_copy_fy0_1,mac_res_temp_fy0_1,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,2,6,9,input_buf_copy_fy0,k_buf_copy_fy0_2,mac_res_temp_fy0_2,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,3,9,12,input_buf_copy_fy0,k_buf_copy_fy0_3,mac_res_temp_fy0_3,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,4,12,15,input_buf_copy_fy0,k_buf_copy_fy0_4,mac_res_temp_fy0_4,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,5,15,18,input_buf_copy_fy0,k_buf_copy_fy0_5,mac_res_temp_fy0_5,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,6,18,21,input_buf_copy_fy0,k_buf_copy_fy0_6,mac_res_temp_fy0_6,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,7,21,24,input_buf_copy_fy0,k_buf_copy_fy0_7,mac_res_temp_fy0_7,clear,ap_clk_div2);
#if XI_KER_PROC==16
		ker_proc_fn(conv_desc,8,24,27,input_buf_copy_fy0,k_buf_copy_fy0_8,mac_res_temp_fy0_8,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,9,27,30,input_buf_copy_fy0,k_buf_copy_fy0_9,mac_res_temp_fy0_9,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,10,30,33,input_buf_copy_fy0,k_buf_copy_fy0_10,mac_res_temp_fy0_10,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,11,33,36,input_buf_copy_fy0,k_buf_copy_fy0_11,mac_res_temp_fy0_11,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,12,36,39,input_buf_copy_fy0,k_buf_copy_fy0_12,mac_res_temp_fy0_12,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,13,39,42,input_buf_copy_fy0,k_buf_copy_fy0_13,mac_res_temp_fy0_13,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,14,42,45,input_buf_copy_fy0,k_buf_copy_fy0_14,mac_res_temp_fy0_14,clear,ap_clk_div2);
		ker_proc_fn(conv_desc,15,45,48,input_buf_copy_fy0,k_buf_copy_fy0_15,mac_res_temp_fy0_15,clear,ap_clk_div2);
#endif
	}//Compute_Loop
#if DEBUG_COMPUTE
	fprintf(stderr,"\nCOMPUTE DONE\n");
#endif

	Pix4_Loop:
	for(ap_uint<8> pix8_fy0 = 0; pix8_fy0<XI_PIX_PROC; pix8_fy0+=2)
	{
#pragma HLS pipeline

		Pix64_Loop:
		for(ap_uint<4> pixel_fy4=0; pixel_fy4<2; pixel_fy4++)
		{
#pragma HLS UNROLL

			ap_uint<8> pixel_fy3[XI_KER_PROC];// = pixel_fy4 + pix8_fy0;
#pragma HLS ARRAY_PARTITION variable=pixel_fy3 complete dim=0
			mux_sel_reg(pixel_fy3, pixel_fy4, pix8_fy0);
			ap_int<42> mac_out[XI_KER_PROC][2];
#pragma HLS ARRAY_PARTITION variable=mac_out complete dim=0


			for(ap_uint<3> pl=0; pl<2; pl++)
			{
#pragma HLS UNROLL
#if PIX_PROC_ODD
				if(pixel_fy3<XI_PIX_PROC)
				{
#endif					
					mac_out[0][pl] = mac_res_temp_fy0_0[pixel_fy3[0]][pl];
					mac_out[1][pl] = mac_res_temp_fy0_1[pixel_fy3[1]][pl];
					mac_out[2][pl] = mac_res_temp_fy0_2[pixel_fy3[2]][pl];
					mac_out[3][pl] = mac_res_temp_fy0_3[pixel_fy3[3]][pl];
					mac_out[4][pl] = mac_res_temp_fy0_4[pixel_fy3[4]][pl];
					mac_out[5][pl] = mac_res_temp_fy0_5[pixel_fy3[5]][pl];
					mac_out[6][pl] = mac_res_temp_fy0_6[pixel_fy3[6]][pl];
					mac_out[7][pl] = mac_res_temp_fy0_7[pixel_fy3[7]][pl];
#if XI_KER_PROC==16
					mac_out[8][pl] = mac_res_temp_fy0_8[pixel_fy3[8]][pl];
					mac_out[9][pl] = mac_res_temp_fy0_9[pixel_fy3[9]][pl];
					mac_out[10][pl] = mac_res_temp_fy0_10[pixel_fy3[10]][pl];
					mac_out[11][pl] = mac_res_temp_fy0_11[pixel_fy3[11]][pl];
					mac_out[12][pl] = mac_res_temp_fy0_12[pixel_fy3[12]][pl];
					mac_out[13][pl] = mac_res_temp_fy0_13[pixel_fy3[13]][pl];
					mac_out[14][pl] = mac_res_temp_fy0_14[pixel_fy3[14]][pl];
					mac_out[15][pl] = mac_res_temp_fy0_15[pixel_fy3[15]][pl];
#endif
#if PIX_PROC_ODD
				}else{
					mac_out[0][pl] = 0;
					mac_out[1][pl] = 0;
					mac_out[2][pl] = 0;
					mac_out[3][pl] = 0;
					mac_out[4][pl] = 0;
					mac_out[5][pl] = 0;
					mac_out[6][pl] = 0;
					mac_out[7][pl] = 0;
#if XI_KER_PROC==16
					mac_out[8][pl] = 0;
					mac_out[9][pl] = 0;
					mac_out[10][pl] = 0;
					mac_out[11][pl] = 0;
					mac_out[12][pl] = 0;
					mac_out[13][pl] = 0;
					mac_out[14][pl] = 0;
					mac_out[15][pl] = 0;
#endif
				}
#endif
			}


			ap_int<24> add_in_24bit_img0[XI_KER_PROC][2];
			ap_int<20> add_in_20bit_img1[XI_KER_PROC][2];
#pragma HLS ARRAY_PARTITION variable=add_in_24bit_img0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=add_in_20bit_img1 complete dim=0
			mux_out_reg(conv_desc, mac_out, add_in_24bit_img0, add_in_20bit_img1);

			ap_int<18> result_img0[XI_KER_PROC];
			for(ap_uint<6> ker=0; ker<XI_KER_PROC; ker++)
			{
#pragma HLS UNROLL


				ap_int<18> res_img0;
				treeAdd_sat24to18bit(add_in_24bit_img0[ker][0],add_in_24bit_img0[ker][1], &res_img0);//
				result_img0[ker] = res_img0;
			}

			ap_int<18> result_img1[XI_KER_PROC];
			for(ap_uint<6> ker=0; ker<XI_KER_PROC; ker++)
			{
#pragma HLS UNROLL

				ap_int<18> res_img1;
				treeAdd_sat20to18bit(add_in_20bit_img1[ker][0],add_in_20bit_img1[ker][1], &res_img1);//
				result_img1[ker] = res_img1;
			}

			for(ap_uint<6> ker=0; ker<XI_KER_PROC;ker++)
			{
#pragma HLS UNROLL
				result_ping_fe0[ker][pix8_fy0 + pixel_fy4[0]].range(17 ,0) = (ap_int<18>)result_img0[ker];
				result_ping_fe0[ker][pix8_fy0 + pixel_fy4[0]].range(35,18) = (ap_int<18>)result_img1[ker];
			}

		}//Pix64_Loop
	}//pix4_Loop


}

#else

void treeAdd_sat24to18bit(ap_int<24> in0, ap_int<24> in1, ap_int<24> in2, ap_int<24> in3, ap_int<18> *result)
{
#pragma HLS interface register port=in0
#pragma HLS interface register port=in1
#pragma HLS interface register port=in2
#pragma HLS interface register port=in3
#pragma HLS inline off
	ap_int<24> add1 = in1 + in0;
	ap_int<24> add2 = in3 + in2;
	ap_int<24> add_out = add1 + add2;

	ap_int<7> sat_test  = (ap_int<7> )add_out.range(23,17);
	ap_int<18> res18bit = (ap_int<18> )add_out.range(17,0);
	ap_int<18> res;
	if(sat_test[6]==0 && sat_test.range(5,0)!=0)
		res=131071;
	else if(sat_test[6]==1 && sat_test.range(5,0)!=63)
		res=-131072;
	else
		res=res18bit;

	*result = res;
}

void treeAdd_sat20to18bit(ap_int<20> in0, ap_int<20> in1, ap_int<20> in2, ap_int<20> in3, ap_int<18> *result)
{
#pragma HLS interface register port=in0
#pragma HLS interface register port=in1
#pragma HLS interface register port=in2
#pragma HLS interface register port=in3
#pragma HLS inline off
	ap_int<20> add1 = in1 + in0;
	ap_int<20> add2 = in3 + in2;
	ap_int<20> add_out = add1 + add2;

	ap_int<3> sat_test  = (ap_int<3> )add_out.range(19,17);
	ap_int<18> res18bit = (ap_int<18> )add_out.range(17,0);
	ap_int<18> res;
	if(sat_test[2]==0 && sat_test.range(1,0)!=0)
		res=131071;
	else if(sat_test[2]==1 && sat_test.range(1,0)!=3)
		res=-131072;
	else
		res=res18bit;

	*result = res;
}

template<int CFILTER_SIZE, int CCONV_STRIDE, int CNUM_KERNELS, int IINPUT_PLANES, int PNKPF>
void Compute16Ker_fy(conv_struct conv_desc,
		weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],
		ap_uint<64> input_buff0_fc0[XI_PIX_PROC / 2][1024],
		ap_uint<4> nkpf_cnt_fe0,
		ap_int<36> result_ping_fe0[XI_KER_PROC][XI_PIX_PROC],
		ap_uint<16> nk_process_fd0,
		out_pix_struct out_pixels0_fc0,
		output_struct output_desc,
		ap_uint<14> weight_buff_offset,
		bool ap_clk_div2)
{
	//*********** debugging *************//
	for (int i = 0; i < 16; i++)
	{
		if (out_pixels0_fc0.pix_rows[i] == 1 && out_pixels0_fc0.pix_cols[i] == 1)
		{
			int hh = 0;
		}
	}
	if (out_pixels0_fc0.pix_rows[0] == 1 && out_pixels0_fc0.pix_cols[0] == 0)
	{
		int b = out_pixels0_fc0.pix_rows[0];
		int a = out_pixels0_fc0.pix_cols[0];
		//int h=a;
	}
	//********** end **********//
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#if XI_FEED_URAM_EN==0
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=input_buff0_fc0 core=XPM_MEMORY uram
#endif


#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=2
#if XI_MERGE_WEIGHTBUFF
#pragma HLS ARRAY_RESHAPE variable=weight_buff0_fd0 complete dim=1
#else
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=1
#endif
#if XI_WTS_URAM_EN==0
#pragma HLS resource variable=weight_buff0_fd0 latency=4
#else
#pragma HLS RESOURCE variable=weight_buff0_fd0 core=XPM_MEMORY uram
#endif
#if XI_RESULT_BUFFER_FF
#pragma HLS ARRAY_PARTITION variable=result_ping_fe0 complete dim=0
#elif XI_RESULT_BUFFER_LUTRAM
#pragma HLS ARRAY_PARTITION variable=result_ping_fe0 complete dim=1
#pragma HLS resource variable=result_ping_fe0 latency=4 core=RAM_2P_LUTRAM
#else
#pragma HLS ARRAY_PARTITION variable=result_ping_fe0 complete dim=1
#pragma HLS resource variable=result_ping_fe0 latency=4 core=RAM_T2P_BRAM
#endif

#pragma HLS INLINE off

	ap_uint<16> outputkernelid_fx0 = nk_process_fd0 + nkpf_cnt_fe0 * XI_KER_PROC;

	ap_int<42> mac_res_temp_fy0[XI_KER_PROC][XI_PIX_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_res_temp_fy0 complete dim=0

	ap_uint<12> k_off_cnt_fy0;
	k_off_cnt_fy0 = weight_buff_offset; //(conv_desc.compute_loop_count*nkpf_cnt_fe0);

	for (ap_uint<6> ker = 0; ker < XI_KER_PROC; ker++)
	{
#pragma HLS unroll
		Pix_Loop:
		for (ap_uint<8> pixel_fy0 = 0; pixel_fy0 < XI_PIX_PROC;pixel_fy0++)
		{
#pragma HLS unroll
			Pln_Loop:
			for (ap_uint<4> plane_fy0 = 0; plane_fy0 < 4;
					plane_fy0++)
			{
#pragma HLS unroll
				mac_res_temp_fy0[ker][pixel_fy0][plane_fy0] = 0;
			} //Pln_Loop
		} //Pix_Loop
	} //ker_loop

#if DEBUG_COMPUTE
	fprintf(fp_input,"\n Current Plane id:%d\n",(int)current_plane);
	fprintf(fp_weight,"\n Current Plane id:%d\n",(int)current_plane);
#endif

	Compute_Loop:
	for (ap_uint<12> inp_buff_addr1_fy0 = 0, inp_buff_addr2_fy0 = 512, offset_kbuf_fy0 = k_off_cnt_fy0;inp_buff_addr1_fy0 < conv_desc.compute_loop_count;
			/*..continue..*/inp_buff_addr1_fy0++, inp_buff_addr2_fy0++, offset_kbuf_fy0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=16 max=16
#pragma HLS PIPELINE

		short input_buf_copy_fy0[4][XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_fy0 complete dim=0

		Pln_Loop1:
		for (ap_uint<8> plane_fy1 = 0, bit=0; plane_fy1 < 4; plane_fy1++,bit+=16)
		{
#pragma HLS unroll
			Pix_Loop1:
			for(ap_uint<8> pixel_fy1=0, pix_split_fy0 =0; pixel_fy1<(XI_PIX_PROC/2);
					pixel_fy1++, pix_split_fy0++)
			{
#pragma HLS unroll
				input_buf_copy_fy0[plane_fy1][pix_split_fy0] 		     = input_buff0_fc0[pixel_fy1][inp_buff_addr1_fy0].range(bit+15,bit);
				input_buf_copy_fy0[plane_fy1][pix_split_fy0+(XI_PIX_PROC/2)] = input_buff0_fc0[pixel_fy1][inp_buff_addr2_fy0].range(bit+15,bit);
			} //Pix_Loop1
		} //Pln_Loop1


		weight_width k_buf_copy_fy0[XI_KER_PROC][4];
#pragma HLS ARRAY_PARTITION variable=k_buf_copy_fy0 complete dim=0

		for (ap_uint<6> ker = 0; ker < XI_KER_PROC; ker++)
		{
#pragma HLS unroll
			Pln_Loop2:
			for (ap_uint<3> plane_fy2 = 0; plane_fy2 < 4;plane_fy2++)
			{
#pragma HLS unroll
				k_buf_copy_fy0[ker][plane_fy2] = weight_buff0_fd0[ker][plane_fy2][offset_kbuf_fy0];
			} //Pln_Loop2
		}
		//	 fprintf(stderr,"\n");
		//***********************  KP
		for (ap_uint<8> ker = 0, pix1 = 0, pix2 = 3; ker < XI_KER_PROC;ker++, pix1 += 3, pix2 += 3)
		{
#pragma HLS unroll
			Pix_Loop3:
			for (int pixel_fy2 = 0; pixel_fy2 < XI_PIX_PROC;pixel_fy2++)
			{
#pragma HLS unroll
				Pln_Loop3:
				for (int plane_fy3 = 0; plane_fy3 < 4; plane_fy3++)
				{
#pragma HLS unroll
					char input_mux;
					if ((pixel_fy2 >= pix1) && (pixel_fy2 < pix2))
					{
						if (conv_desc.sos_enable == 1)
							input_mux = input_buf_copy_fy0[plane_fy3][pixel_fy2];
						else
							input_mux = k_buf_copy_fy0[ker][plane_fy3];
					}
					else
					{
						input_mux = k_buf_copy_fy0[ker][plane_fy3];
					}

					ap_int<16> input_reg = input_buf_copy_fy0[plane_fy3][pixel_fy2];
					ap_int<27> in_27bit;
					ap_int<27> in2_27bit;
					in2_27bit.range(26, 21) = input_reg.range(13, 8);
					in2_27bit.range(20, 0) = 0;
					ap_int<8> in1 = input_reg.range(7, 0);
					in_27bit = in2_27bit + in1;
					mac_res_temp_fy0[ker][pixel_fy2][plane_fy3] += (in_27bit * input_mux);

				}                    //Pln_Loop3

			}//Pix_Loop2
		}//ker_loop
	}//Compute_Loop
#if DEBUG_COMPUTE
	fprintf(stderr,"\nCOMPUTE DONE\n");
#endif
	Pix4_Loop:
	for (ap_uint<8> pix8_fy0 = 0; pix8_fy0 < XI_PIX_PROC; pix8_fy0 += 2)
	{
#pragma HLS pipeline

		Pix64_Loop:
		for (ap_uint<4> pixel_fy4 = 0; pixel_fy4 < 2; pixel_fy4++)
		{
#pragma HLS UNROLL

			ap_uint<8> pixel_fy3 = pixel_fy4 + pix8_fy0;
			ap_int<42> mac_out[XI_KER_PROC][4];
#pragma HLS ARRAY_PARTITION variable=mac_out complete dim=0

			for (ap_uint<6> ker = 0; ker < XI_KER_PROC; ker++)
			{
#pragma HLS UNROLL
				for (ap_uint<3> pl = 0; pl < 4; pl++)
				{
#pragma HLS UNROLL
					if (pixel_fy3 < XI_PIX_PROC)
					{
						mac_out[ker][pl] = mac_res_temp_fy0[ker][pixel_fy3][pl];
					}
					else
					{
						mac_out[ker][pl] = 0;
					}
				}
			}

			ap_int<24> add_in_24bit_img0[XI_KER_PROC][4];
			ap_int<20> add_in_20bit_img1[XI_KER_PROC][4];

			for (ap_uint<6> ker = 0; ker < XI_KER_PROC; ker++)
			{
#pragma HLS UNROLL
				for (ap_uint<3> pl = 0; pl < 4; pl++)
				{
#pragma HLS UNROLL
					if (conv_desc.int6_en_in == 0 || conv_desc.batch1_int8_en == 1)
					{
						add_in_24bit_img0[ker][pl] = (ap_int<24> ) mac_out[ker][pl].range(23, 0);
					}
					else
					{
						add_in_24bit_img0[ker][pl] = (ap_int<24> ) ((ap_int<20> ) mac_out[ker][pl].range(
								19, 0));
					}
					add_in_20bit_img1[ker][pl] = (ap_int<20> ) mac_out[ker][pl].range(40, 21) + mac_out[ker][pl][20];
				}
			}

			ap_int<18> result_img0[XI_KER_PROC];
			for (ap_uint<6> ker = 0; ker < XI_KER_PROC; ker++)
			{
#pragma HLS UNROLL

				ap_int<18> res_img0;
				treeAdd_sat24to18bit(add_in_24bit_img0[ker][0],add_in_24bit_img0[ker][1],add_in_24bit_img0[ker][2],add_in_24bit_img0[ker][3], &res_img0);//
				result_img0[ker] = res_img0;

			}

			ap_int<18> result_img1[XI_KER_PROC];
			for (ap_uint<6> ker = 0; ker < XI_KER_PROC; ker++)
			{
#pragma HLS UNROLL
				ap_int<18> res_img1;
				treeAdd_sat20to18bit(add_in_20bit_img1[ker][0],add_in_20bit_img1[ker][1],add_in_20bit_img1[ker][2],add_in_20bit_img1[ker][3], &res_img1);//
				result_img1[ker] = res_img1;
			}

			for (ap_uint<6> ker = 0; ker < XI_KER_PROC; ker++)
			{
#pragma HLS UNROLL
				result_ping_fe0[ker][pix8_fy0 + pixel_fy4[0]].range(17, 0) = (ap_int<18> ) result_img0[ker];
				result_ping_fe0[ker][pix8_fy0 + pixel_fy4[0]].range(35, 18) = (ap_int<18> ) result_img1[ker];
			}

		}    //Pix64_Loop
	}    //pix4_Loop

}
#endif// DP_enable

void add_sat(ap_int<18> in1, ap_int<18> in2, ap_int<18> *out)
{
#pragma HLS interface register port=in1
#pragma HLS interface register port=in2
#pragma HLS inline off
	ap_int<19> sum_temp=in1 + in2;

	bool sum_overflow = (sum_temp.range(18,17)==2) || (sum_temp.range(18,17)==1);
	bool in1_overflow = (in1 == 131071 || in1 == -131072);
	bool in2_overflow = (in2 == 131071 || in2 == -131072);

	ap_int<18> sum_out;

	if(sum_overflow==1 && in1_overflow==0 && in2_overflow==0 )
	{
		if(sum_temp[18]==0)
			sum_out = 131071;
		else
			sum_out = -131072;
	}
	else
	{
		if(in1 == 131071 || in2 == 131071)
			sum_out = 131071;
		else if(in1 == -131072 || in2 == -131072)
			sum_out = -131072;
		else
			sum_out = (ap_int<18>)sum_temp;
	}


	*out = sum_out;//in1 + in2;
}

template<int PNKPF>
void OStgBuffSeq_fx(ap_int<36> result_ping_fe0[XI_KER_PROC][XI_PIX_PROC],
		out_pix_struct out_pixels0_fc0, ap_uint<16> nk_process_fd0,
		conv_struct conv_desc, output_struct output_desc,
		ap_uint<72> ostaging_buff0_fb0[2][XI_OSTG_BUFFER_SET][XI_OSTAGEBUFF_DEPTH],
		ap_uint<16> ostgrow_x_width, ap_uint<4> nkpf2_cnt_fe1,
		ap_uint<16> out_row_offset_fb0, ap_uint<2> mac_fz0) {

#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_rows complete dim=1
#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_cols complete dim=1
#pragma HLS data_pack variable=out_pixels0_fc0

#if XI_RESULT_BUFFER_FF
#pragma HLS ARRAY_PARTITION variable=result_ping_fe0 complete dim=0
#elif XI_RESULT_BUFFER_LUTRAM
#pragma HLS ARRAY_PARTITION variable=result_ping_fe0 complete dim=1
#pragma HLS resource variable=result_ping_fe0 latency=4 core=RAM_2P_LUTRAM
#else
#pragma HLS ARRAY_PARTITION variable=result_ping_fe0 complete dim=1
#pragma HLS resource variable=result_ping_fe0 latency=4 core=RAM_T2P_BRAM
#endif
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2
#if XI_OSTG_URAM_EN==0
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM latency=2
#else
#pragma HLS RESOURCE variable=ostaging_buff0_fb0 core=XPM_MEMORY uram
#endif

#pragma HLS INLINE OFF

	ap_uint<16> outputkernelid_fx0;
	if (conv_desc.lrn_inter_sos_enable == 1 || conv_desc.lrn_intra_sos_enable == 1)
		outputkernelid_fx0 = (out_pixels0_fc0.current_plane_by4) * 4;
	else if (conv_desc.l2_sos_enable == 1)
		outputkernelid_fx0 = 0;
	else
		//conv
		outputkernelid_fx0 = nk_process_fd0 + nkpf2_cnt_fe1 * XI_KER_PROC;

	ap_uint<8> result_pix1_read[XI_KER_PROC];
#pragma HLS ARRAY_PARTITION variable=result_pix1_read complete dim=0
	for (ap_uint<8> ker = 0; ker < XI_KER_PROC; ker++)
	{
#pragma HLS unroll
		if (conv_desc.sos_enable == 1)
			result_pix1_read[ker] = ker * 3;
		else
			result_pix1_read[ker] = 0;
	}
	ap_uint<8> result_pix2_read[XI_KER_PROC];
#pragma HLS ARRAY_PARTITION variable=result_pix2_read complete dim=0
	for (ap_uint<8> ker = 0; ker < XI_KER_PROC; ker++)
	{
#pragma HLS unroll
		if (conv_desc.sos_enable == 1)
			result_pix2_read[ker] = ker * 3;
		else
			result_pix2_read[ker] = XI_PIX_PROC / 2;
	}

	ap_uint<14> pix_cnt_inc = output_desc.pix_cnt;

	ap_uint<16> row_proc_prod_fx0 = ostgrow_x_width;//rowsprocessed*output_desc.width;



	Write_Loop:
	for (ap_uint<8> ostg_pix1_fx0 = 0;ostg_pix1_fx0 < conv_desc.pix_per_kp; ostg_pix1_fx0++)
	{
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=ostaging_buff0_fb0 inter false
#pragma HLS DEPENDENCE variable=ostaging_buff0_fb0 intra false

#if XI_OSTG_BUFFER_SET==8
		ap_uint<16> pixbuff_planeoffset_fx0 = (outputkernelid_fx0.range(15, 5))* (conv_desc.out_pix);
		ap_uint<6> index_fx0 = outputkernelid_fx0.range(4, 0); //(outputkernelid_fx0 & 0x1F);
		ap_uint<3> dim2_ostg1_fx0 = (index_fx0.range(4, 2));
		ap_uint<3> dim2_ostg2_fx0 = (index_fx0.range(4, 2)) + 1;
#else
		ap_uint<16> pixbuff_planeoffset_fx0 = (outputkernelid_fx0.range(15, 4))
																												* (conv_desc.out_pix);
		ap_uint<6> index_fx0 = outputkernelid_fx0.range(3, 0); //(outputkernelid_fx0 & 0x1F);
		ap_uint<3> dim2_ostg1_fx0 = (index_fx0.range(3, 2));
		ap_uint<3> dim2_ostg2_fx0 = (index_fx0.range(3, 2)) + 1;
#endif

		ap_uint<14> pixbuff_row_fx0 = pix_cnt_inc;        //output_desc.pix_cnt;
		pix_cnt_inc++;        //
		ap_uint<12> pixbuff_index_fx0 = pixbuff_row_fx0
				+ pixbuff_planeoffset_fx0;

		bool expression1_fx0 = (pixbuff_row_fx0 < row_proc_prod_fx0);

		ap_int<72> ostg_src_fx0[2][8];
#pragma HLS ARRAY_PARTITION variable=ostg_src_fx0 complete dim=0
		Dim1_Loop1: for (ap_uint<4> dim1_fx1 = 0; dim1_fx1 < XI_OSTG_BUFFER_SET;
				dim1_fx1++) {
			ostg_src_fx0[0][dim1_fx1] =
					ostaging_buff0_fb0[0][dim1_fx1][pixbuff_index_fx0];
			ostg_src_fx0[1][dim1_fx1] =
					ostaging_buff0_fb0[1][dim1_fx1][pixbuff_index_fx0];
		}        //Dim1_Loop1

#if XI_KER_PROC==16
		ap_int<18> read_ostg_fx0[2][4][4];
#pragma HLS ARRAY_PARTITION variable=read_ostg_fx0 complete dim=0

		Dim1_Loop1_img:
		for (ap_uint<3> img = 0; img < 2; img++)
		{
#pragma HLS UNROLL
			Dim1_Loop2:
			for (int dim1_fx2 = 0,bit18=0; dim1_fx2 < 4; dim1_fx2++, bit18+=18)
			{
#pragma HLS UNROLL
#if XI_OSTG_BUFFER_SET==8
				if (outputkernelid_fx0[4] == 0)
				{
					read_ostg_fx0[img][dim1_fx2][0] = ostg_src_fx0[img][0].range(bit18 + 17,bit18);
					read_ostg_fx0[img][dim1_fx2][1] = ostg_src_fx0[img][1].range(bit18 + 17,bit18);
					read_ostg_fx0[img][dim1_fx2][2] = ostg_src_fx0[img][2].range(bit18 + 17,bit18);
					read_ostg_fx0[img][dim1_fx2][3] = ostg_src_fx0[img][3].range(bit18 + 17,bit18);
				}
				else
				{
					read_ostg_fx0[img][dim1_fx2][0] = ostg_src_fx0[img][4].range(bit18 + 17,bit18);
					read_ostg_fx0[img][dim1_fx2][1] = ostg_src_fx0[img][5].range(bit18 + 17,bit18);
					read_ostg_fx0[img][dim1_fx2][2] = ostg_src_fx0[img][6].range(bit18 + 17,bit18);
					read_ostg_fx0[img][dim1_fx2][3] = ostg_src_fx0[img][7].range(bit18 + 17,bit18);
				}
#else
				read_ostg_fx0[img][dim1_fx2][0] = ostg_src_fx0[img][0].range(bit18 + 17,bit18);
				read_ostg_fx0[img][dim1_fx2][1] = ostg_src_fx0[img][1].range(bit18 + 17,bit18);
				read_ostg_fx0[img][dim1_fx2][2] = ostg_src_fx0[img][2].range(bit18 + 17,bit18);
				read_ostg_fx0[img][dim1_fx2][3] = ostg_src_fx0[img][3].range(bit18 + 17,bit18);
#endif
			}        //Dim1_Loop2
		}

		ap_int<18> result_inc_fx0[2][4][4];
#pragma HLS ARRAY_PARTITION variable=result_inc_fx0 complete dim=0
		Dim2_Loop4:
		for (int dim2_fx4 = 0; dim2_fx4 < 4; dim2_fx4++)
		{
#pragma HLS UNROLL
			Dim1_Loop4:
			for (int dim1_fx4 = 0; dim1_fx4 < 4; dim1_fx4++)
			{
#pragma HLS UNROLL
				if (conv_desc.sos_enable == 0)
				{
					if (out_pixels0_fc0.current_plane_by4 != 0)
					{
						result_inc_fx0[0][dim1_fx4][dim2_fx4] = read_ostg_fx0[0][dim1_fx4][dim2_fx4];
						result_inc_fx0[1][dim1_fx4][dim2_fx4] = read_ostg_fx0[1][dim1_fx4][dim2_fx4];
					}
					else
					{

						result_inc_fx0[0][dim1_fx4][dim2_fx4] = 0;//b2;    ////<<XI_SX;
						result_inc_fx0[1][dim1_fx4][dim2_fx4] = 0;//b2;    //<<XI_SX;
					}
				}
				else
				{
					result_inc_fx0[0][dim1_fx4][dim2_fx4] = 0;
					result_inc_fx0[1][dim1_fx4][dim2_fx4] = 0;
				}
			}//Dim1_Loop4
		}//Dim2_Loop4



		ap_int<36> result_ker_pix1[4][4];
#pragma HLS ARRAY_PARTITION variable=result_ker_pix1 complete dim=0
		result_ker_pix1[0][0] = result_ping_fe0[0][result_pix1_read[0]];
		result_ker_pix1[1][0] = result_ping_fe0[1][result_pix1_read[1]];
		result_ker_pix1[2][0] = result_ping_fe0[2][result_pix1_read[2]];
		result_ker_pix1[3][0] = result_ping_fe0[3][result_pix1_read[3]];
		result_ker_pix1[0][1] = result_ping_fe0[4][result_pix1_read[4]];
		result_ker_pix1[1][1] = result_ping_fe0[5][result_pix1_read[5]];
		result_ker_pix1[2][1] = result_ping_fe0[6][result_pix1_read[6]];
		result_ker_pix1[3][1] = result_ping_fe0[7][result_pix1_read[7]];
		result_ker_pix1[0][2] = result_ping_fe0[8][result_pix1_read[8]];
		result_ker_pix1[1][2] = result_ping_fe0[9][result_pix1_read[9]];
		result_ker_pix1[2][2] = result_ping_fe0[10][result_pix1_read[10]];
		result_ker_pix1[3][2] = result_ping_fe0[11][result_pix1_read[11]];
		result_ker_pix1[0][3] = result_ping_fe0[12][result_pix1_read[12]];
		result_ker_pix1[1][3] = result_ping_fe0[13][result_pix1_read[13]];
		result_ker_pix1[2][3] = result_ping_fe0[14][result_pix1_read[14]];
		result_ker_pix1[3][3] = result_ping_fe0[15][result_pix1_read[15]];
		ap_int<36> result_ker_pix2[4][4];
#pragma HLS ARRAY_PARTITION variable=result_ker_pix2 complete dim=0
		result_ker_pix2[0][0] = result_ping_fe0[0][result_pix2_read[0]];
		result_ker_pix2[1][0] = result_ping_fe0[1][result_pix2_read[1]];
		result_ker_pix2[2][0] = result_ping_fe0[2][result_pix2_read[2]];
		result_ker_pix2[3][0] = result_ping_fe0[3][result_pix2_read[3]];
		result_ker_pix2[0][1] = result_ping_fe0[4][result_pix2_read[4]];
		result_ker_pix2[1][1] = result_ping_fe0[5][result_pix2_read[5]];
		result_ker_pix2[2][1] = result_ping_fe0[6][result_pix2_read[6]];
		result_ker_pix2[3][1] = result_ping_fe0[7][result_pix2_read[7]];
		result_ker_pix2[0][2] = result_ping_fe0[8][result_pix2_read[8]];
		result_ker_pix2[1][2] = result_ping_fe0[9][result_pix2_read[9]];
		result_ker_pix2[2][2] = result_ping_fe0[10][result_pix2_read[10]];
		result_ker_pix2[3][2] = result_ping_fe0[11][result_pix2_read[11]];
		result_ker_pix2[0][3] = result_ping_fe0[12][result_pix2_read[12]];
		result_ker_pix2[1][3] = result_ping_fe0[13][result_pix2_read[13]];
		result_ker_pix2[2][3] = result_ping_fe0[14][result_pix2_read[14]];
		result_ker_pix2[3][3] = result_ping_fe0[15][result_pix2_read[15]];

		ap_int<18> result_ostg_fx0[2][4][4];
#pragma HLS ARRAY_PARTITION variable=result_ostg_fx0 complete dim=0

		for (ap_uint<3> dim1 = 0; dim1 < 4; dim1++)
		{
#pragma HLS unroll
			for (ap_uint<3> dim2 = 0; dim2 < 4; dim2++)
			{
#pragma HLS unroll
				ap_int<18> compute_pix1;
				ap_int<18> compute_pix2;

				if (conv_desc.int6_en_in == 1)
				{
					compute_pix1 = result_ker_pix1[dim1][dim2].range(17, 0);
					compute_pix2 = result_ker_pix1[dim1][dim2].range(35, 18);
				}
				else
				{
					compute_pix1 = result_ker_pix1[dim1][dim2].range(17, 0);
					compute_pix2 = result_ker_pix2[dim1][dim2].range(17, 0);
				}

				ap_int<18> add_pix1;
				ap_int<18> add_pix2;

				ap_int<18> buffer_pix1 = result_inc_fx0[0][dim1][dim2];
				ap_int<18> buffer_pix2 = result_inc_fx0[1][dim1][dim2];

				add_sat(compute_pix1, buffer_pix1, &add_pix1);
				add_sat(compute_pix2, buffer_pix2, &add_pix2);

				result_ostg_fx0[0][dim1][dim2] = add_pix1;
				result_ostg_fx0[1][dim1][dim2] = add_pix2;

			}
		}

		ap_uint<72> result_ostg_packed[2][4];
#pragma HLS ARRAY_PARTITION variable=result_ostg_packed complete dim=0

		for (ap_uint<10> dim1 = 0, bit=0; dim1 < 4; dim1++, bit+=18)
		{
#pragma HLS unroll
			for (ap_uint<3> dim2 = 0; dim2 < 4; dim2++)
			{
#pragma HLS unroll
				result_ostg_packed[0][dim2].range(bit+17, bit) = result_ostg_fx0[0][dim1][dim2];
				result_ostg_packed[1][dim2].range(bit+17, bit) = result_ostg_fx0[1][dim1][dim2];
			}
		}

		for (ap_uint<6> ker = 0; ker < XI_KER_PROC; ker++)
		{
#pragma HLS unroll
			result_pix1_read[ker]++;
			result_pix2_read[ker]++;
		}

		for (ap_uint<3> img = 0; img < 2; img++)
		{
#pragma HLS UNROLL
			Dim2_Loop3:
			for (ap_uint<5> dim2_fx3 = 0; dim2_fx3 < XI_OSTG_BUFFER_SET;dim2_fx3++)
			{
#pragma HLS UNROLL
#if XI_OSTG_BUFFER_SET==8
				if (expression1_fx0 == 1)
				{
					if ((outputkernelid_fx0[4] == 0 && dim2_fx3 < 4) || (outputkernelid_fx0[4] == 1 && dim2_fx3 > 3))
					{
						ostaging_buff0_fb0[img][dim2_fx3][pixbuff_index_fx0] = result_ostg_packed[img][dim2_fx3.range(1, 0)];
					}

				}
#else
				ostaging_buff0_fb0[img][dim2_fx3][pixbuff_index_fx0] = result_ostg_packed[img][dim2_fx3];
#endif

			}//Dim2_Loop3
		}
#elif XI_KER_PROC==8

		ap_int<18> read_ostg_fx0[2][4][2];
#pragma HLS ARRAY_PARTITION variable=read_ostg_fx0 complete dim=0

		Dim1_Loop1_img:
		for (ap_uint<3> img = 0; img < 2; img++)
		{
#pragma HLS UNROLL
			Dim1_Loop2:
			for (int dim1_fx2 = 0,bit18=0; dim1_fx2 < 4; dim1_fx2++, bit18+=18)
			{
#pragma HLS UNROLL

#if XI_OSTG_BUFFER_SET==8
				if (outputkernelid_fx0.range(4,3) == 0)
				{
					read_ostg_fx0[img][dim1_fx2][0] = ostg_src_fx0[img][0].range(bit18 + 17,bit18);
					read_ostg_fx0[img][dim1_fx2][1] = ostg_src_fx0[img][1].range(bit18 + 17,bit18);
				}

				else if (outputkernelid_fx0.range(4,3) == 1)
				{
					read_ostg_fx0[img][dim1_fx2][0] = ostg_src_fx0[img][2].range(bit18 + 17,bit18);
					read_ostg_fx0[img][dim1_fx2][1] = ostg_src_fx0[img][3].range(bit18 + 17,bit18);
				}

				else if (outputkernelid_fx0.range(4,3) == 2)
				{
					read_ostg_fx0[img][dim1_fx2][0] = ostg_src_fx0[img][4].range(bit18 + 17,bit18);
					read_ostg_fx0[img][dim1_fx2][1] = ostg_src_fx0[img][5].range(bit18 + 17,bit18);
				}

				else
				{
					read_ostg_fx0[img][dim1_fx2][0] = ostg_src_fx0[img][6].range(bit18 + 17,bit18);
					read_ostg_fx0[img][dim1_fx2][1] = ostg_src_fx0[img][7].range(bit18 + 17,bit18);
				}
#else
				if (outputkernelid_fx0[2] == 0)
				{
					read_ostg_fx0[img][dim1_fx2][0] = ostg_src_fx0[img][0].range(bit18 + 17,bit18);
					read_ostg_fx0[img][dim1_fx2][1] = ostg_src_fx0[img][1].range(bit18 + 17,bit18);
				}

				else
				{
					read_ostg_fx0[img][dim1_fx2][0] = ostg_src_fx0[img][2].range(bit18 + 17,bit18);
					read_ostg_fx0[img][dim1_fx2][1] = ostg_src_fx0[img][3].range(bit18 + 17,bit18);
				}
#endif
			}						//Dim1_Loop2
		}

		ap_int<18> result_inc_fx0[2][4][2];
#pragma HLS ARRAY_PARTITION variable=result_inc_fx0 complete dim=0
		Dim2_Loop4:
		for (int dim2_fx4 = 0; dim2_fx4 < 2; dim2_fx4++)
		{
#pragma HLS UNROLL
			Dim1_Loop4:
			for (int dim1_fx4 = 0; dim1_fx4 < 4; dim1_fx4++)
			{
#pragma HLS UNROLL
				if (conv_desc.sos_enable == 0)
				{
					if (out_pixels0_fc0.current_plane_by4 != 0)
					{
						result_inc_fx0[0][dim1_fx4][dim2_fx4] = read_ostg_fx0[0][dim1_fx4][dim2_fx4];
						result_inc_fx0[1][dim1_fx4][dim2_fx4] = read_ostg_fx0[1][dim1_fx4][dim2_fx4];
					}
					else
					{

						result_inc_fx0[0][dim1_fx4][dim2_fx4] = 0;//b2;    ////<<XI_SX;
						result_inc_fx0[1][dim1_fx4][dim2_fx4] = 0;//b2;    //<<XI_SX;
					}
				}
				else
				{
					result_inc_fx0[0][dim1_fx4][dim2_fx4] = 0;
					result_inc_fx0[1][dim1_fx4][dim2_fx4] = 0;
				}
			}//Dim1_Loop4
		}//Dim2_Loop4



		ap_int<36> result_ker_pix1[4][2];
#pragma HLS ARRAY_PARTITION variable=result_ker_pix1 complete dim=0
		result_ker_pix1[0][0] = result_ping_fe0[0][result_pix1_read[0]];
		result_ker_pix1[1][0] = result_ping_fe0[1][result_pix1_read[1]];
		result_ker_pix1[2][0] = result_ping_fe0[2][result_pix1_read[2]];
		result_ker_pix1[3][0] = result_ping_fe0[3][result_pix1_read[3]];
		result_ker_pix1[0][1] = result_ping_fe0[4][result_pix1_read[4]];
		result_ker_pix1[1][1] = result_ping_fe0[5][result_pix1_read[5]];
		result_ker_pix1[2][1] = result_ping_fe0[6][result_pix1_read[6]];
		result_ker_pix1[3][1] = result_ping_fe0[7][result_pix1_read[7]];
		ap_int<36> result_ker_pix2[4][2];
#pragma HLS ARRAY_PARTITION variable=result_ker_pix2 complete dim=0
		result_ker_pix2[0][0] = result_ping_fe0[0][result_pix2_read[0]];
		result_ker_pix2[1][0] = result_ping_fe0[1][result_pix2_read[1]];
		result_ker_pix2[2][0] = result_ping_fe0[2][result_pix2_read[2]];
		result_ker_pix2[3][0] = result_ping_fe0[3][result_pix2_read[3]];
		result_ker_pix2[0][1] = result_ping_fe0[4][result_pix2_read[4]];
		result_ker_pix2[1][1] = result_ping_fe0[5][result_pix2_read[5]];
		result_ker_pix2[2][1] = result_ping_fe0[6][result_pix2_read[6]];
		result_ker_pix2[3][1] = result_ping_fe0[7][result_pix2_read[7]];

		ap_int<18> result_ostg_fx0[2][4][2];
#pragma HLS ARRAY_PARTITION variable=result_ostg_fx0 complete dim=0

		for (ap_uint<3> dim1 = 0; dim1 < 4; dim1++)
		{
#pragma HLS unroll
			for (ap_uint<3> dim2 = 0; dim2 < 2; dim2++)
			{
#pragma HLS unroll
				ap_int<18> compute_pix1;
				ap_int<18> compute_pix2;

				if (conv_desc.int6_en_in == 1)
				{
					compute_pix1 = result_ker_pix1[dim1][dim2].range(17, 0);
					compute_pix2 = result_ker_pix1[dim1][dim2].range(35, 18);
				}
				else
				{
					compute_pix1 = result_ker_pix1[dim1][dim2].range(17, 0);
					compute_pix2 = result_ker_pix2[dim1][dim2].range(17, 0);
				}

				ap_int<18> add_pix1;
				ap_int<18> add_pix2;

				ap_int<18> buffer_pix1 = result_inc_fx0[0][dim1][dim2];
				ap_int<18> buffer_pix2 = result_inc_fx0[1][dim1][dim2];

				add_sat(compute_pix1, buffer_pix1, &add_pix1);
				add_sat(compute_pix2, buffer_pix2, &add_pix2);

				result_ostg_fx0[0][dim1][dim2] = add_pix1;
				result_ostg_fx0[1][dim1][dim2] = add_pix2;

			}
		}

		ap_uint<72> result_ostg_packed[2][2];
#pragma HLS ARRAY_PARTITION variable=result_ostg_packed complete dim=0

		for (ap_uint<10> dim1 = 0, bit=0; dim1 < 4; dim1++, bit+=18)
		{
#pragma HLS unroll
			for (ap_uint<3> dim2 = 0; dim2 < 2; dim2++)
			{
#pragma HLS unroll
				result_ostg_packed[0][dim2].range(bit+17, bit) = result_ostg_fx0[0][dim1][dim2];
				result_ostg_packed[1][dim2].range(bit+17, bit) = result_ostg_fx0[1][dim1][dim2];
			}
		}

		for (ap_uint<6> ker = 0; ker < XI_KER_PROC; ker++)
		{
#pragma HLS unroll
			result_pix1_read[ker]++;
			result_pix2_read[ker]++;
		}



		for (ap_uint<3> img = 0; img < 2; img++)
		{
#pragma HLS UNROLL
			if (expression1_fx0 == 1)
			{
#if	XI_OSTG_BUFFER_SET==8
				if (outputkernelid_fx0.range(4,3) == 0)
				{
					ostaging_buff0_fb0[img][0][pixbuff_index_fx0] = result_ostg_packed[img][0];
					ostaging_buff0_fb0[img][1][pixbuff_index_fx0] = result_ostg_packed[img][1];
				}

				if (outputkernelid_fx0.range(4,3) == 1)
				{
					ostaging_buff0_fb0[img][2][pixbuff_index_fx0] = result_ostg_packed[img][0];
					ostaging_buff0_fb0[img][3][pixbuff_index_fx0] = result_ostg_packed[img][1];
				}

				if (outputkernelid_fx0.range(4,3) == 2)
				{
					ostaging_buff0_fb0[img][4][pixbuff_index_fx0] = result_ostg_packed[img][0];
					ostaging_buff0_fb0[img][5][pixbuff_index_fx0] = result_ostg_packed[img][1];
				}

				if (outputkernelid_fx0.range(4,3) == 3)
				{
					ostaging_buff0_fb0[img][6][pixbuff_index_fx0] = result_ostg_packed[img][0];
					ostaging_buff0_fb0[img][7][pixbuff_index_fx0] = result_ostg_packed[img][1];
				}
#else
				if (outputkernelid_fx0[3] == 0)
				{
					ostaging_buff0_fb0[img][0][pixbuff_index_fx0] = result_ostg_packed[img][0];
					ostaging_buff0_fb0[img][1][pixbuff_index_fx0] = result_ostg_packed[img][1];
				}

				if (outputkernelid_fx0[3] == 1)
				{
					ostaging_buff0_fb0[img][2][pixbuff_index_fx0] = result_ostg_packed[img][0];
					ostaging_buff0_fb0[img][3][pixbuff_index_fx0] = result_ostg_packed[img][1];
				}
#endif
			}

		}//Dim2_Loop3
#endif

	}//Write_Loop

}

void InputBuffCopy(short input_buf_copy_fy0[4][XI_PIX_PROC],
		short input_buf_copy_reg_fy0[4][XI_PIX_PROC])
{
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_fy0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=input_buf_copy_reg_fy0 complete dim=0

#pragma HLS interface register port=input_buf_copy_reg_fy0
#pragma HLS INLINE off

	Pln_Loop1:
	for (ap_uint<3> plane_fy1 = 0; plane_fy1 < 4; plane_fy1++)
	{
#pragma HLS unroll
		Pix_Loop1:
		for (ap_uint<8> pixel_fy1 = 0; pixel_fy1 < (XI_PIX_PROC);pixel_fy1++)
		{
#pragma HLS unroll
			input_buf_copy_reg_fy0[plane_fy1][pixel_fy1] = input_buf_copy_fy0[plane_fy1][pixel_fy1];

		}//Pix_Loop1
	}//Pln_Loop1
}

void reg_addr_rowPixFun(ap_uint<3> in3bit, istg_datatype in16bit,ap_uint<3> *out3bit, istg_datatype *out16bit)
{
#pragma HLS interface register port=in3bit
#pragma HLS interface register port=in16bit
#pragma HLS inline off
	*out3bit = in3bit;
	*out16bit = in16bit;

}

void reg_data_rowPixfun(short istg_value_fo0[4], short istg_value_fo0_reg[4])
{
#pragma HLS ARRAY_PARTITION variable=istg_value_fo0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=istg_value_fo0_reg complete dim=0
#pragma HLS interface register port=istg_value_fo0_reg
#pragma HLS inline off
	for (ap_uint<3> pl = -0; pl < 4; pl++)
	{
#pragma HLS UNROLL
		istg_value_fo0_reg[pl] = istg_value_fo0[pl];
	}
}

template<int CFILTER_SIZE, int CCONV_STRIDE, int CNUM_KERNELS, int INPUTP>
void RowPixFunction_fo(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<64> input_buff0_fc0[XI_PIX_PROC / 2][1024],
		ap_uint<16> opix_row_fl0, ap_uint<16> opix_col_fl0,
		ap_uint<16> opix_row_wo_offset_fl0, ap_uint<16> current_plane_fc0,
		ap_uint<10> inputBuff_bram_offset_fl0, ap_int<16> pad_row_fc0,
		ap_int<16> pad_row_wo_fc0, ap_uint<8> pxcnt_fn0,
		ap_uint<8> pixel_tobe_process_fn0, ap_uint<10> iteration_fn0,
		ap_uint<2> mac_fz0)
{
#pragma HLS inline off

	ap_uint<8> fsz_fst_div_fo0 = conv_desc.fsz_by_stride;
	ap_uint<8> fsz_fst_mod_fo0 = conv_desc.fsz_mod_stride;

	bool initial_flag_reg_fo0[XI_PIX_PROC / 2];
#pragma HLS ARRAY_PARTITION variable=initial_flag_reg_fo0 complete dim=0
	bool flag_reg_fo0[XI_PIX_PROC / 2];
#pragma HLS ARRAY_PARTITION variable=flag_reg_fo0 complete dim=0
	ap_uint<10> addr_bram_fo0[XI_PIX_PROC / 2];
#pragma HLS ARRAY_PARTITION variable=addr_bram_fo0 complete dim=0
	ap_uint<3> dilated_counter_fo0[XI_PIX_PROC / 2];
#pragma HLS ARRAY_PARTITION variable=dilated_counter_fo0 complete dim=0
	Pix_fo0_Loop:
	for (ap_uint<8> pix_fo0 = 0; pix_fo0 < XI_PIX_PROC / 2;pix_fo0++)
	{
#pragma HLS unroll
		dilated_counter_fo0[pix_fo0] = 0;
		addr_bram_fo0[pix_fo0] = inputBuff_bram_offset_fl0;

		if (pix_fo0 == pxcnt_fn0)
			initial_flag_reg_fo0[pix_fo0] = 1;
		else
			initial_flag_reg_fo0[pix_fo0] = 0;
		flag_reg_fo0[pix_fo0] = initial_flag_reg_fo0[pix_fo0];
	}

	ap_uint<16> col_max_fo0 = weight_desc.filter_size_dilated + weight_desc.filter_stride * (pixel_tobe_process_fn0);

	ap_uint<8> stride_counter_fo0 = 0;

	//************** LRN inter *******************
	ap_int<8> lrn_counter;
	ap_uint<8> pattern_8kp;
	if (weight_desc.filter_size == 9)
	{
		lrn_counter = 1;
		pattern_8kp = 1;
	}
	else if (weight_desc.filter_size == 7)
	{
		lrn_counter = 0;
		pattern_8kp = 0;
	}
	else if (weight_desc.filter_size == 5)
	{
		lrn_counter = -1;
		pattern_8kp = 0;
	}
	else //(weight_desc.filter_size == 3)
	{
		lrn_counter = -2;
		pattern_8kp = 0;
	}
	ap_uint<16> lrn_row_offset;
	ap_uint<16> lrn_row_offset_opix;
	ap_uint<16> lrn_col_offset;
	ap_uint<30> flag_1st_32pix_buff_previous_iteration,
	flag_next_32pix_buff_previous_iteration;
	//********************************************

	Plane_Loop:
	for (ap_uint<16> plane4_fo0 = 0;plane4_fo0 < conv_desc.feeding_buff_plane_loop_bound;plane4_fo0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=1 max=1
		Row_Loop:
		for (ap_uint<8> row_cnt_fo0 = 0;row_cnt_fo0 < conv_desc.feeding_buff_row_loop_bound;row_cnt_fo0++)
		{
#pragma HLS LOOP_TRIPCOUNT min=7 max=7
			Col_Loop:
			for (ap_uint<16> col_cnt_fo0 = 0;col_cnt_fo0 < col_max_fo0; col_cnt_fo0++)
			{
#pragma HLS DEPENDENCE variable=input_buff0_fc0 intra false
#pragma HLS DEPENDENCE variable=input_buff0_fc0 inter false
#pragma HLS LOOP_TRIPCOUNT min=100 max=100
#pragma HLS loop_flatten
#pragma HLS PIPELINE

				ap_uint<16> act_plane4_fo0 = plane4_fo0 + current_plane_fc0;
				ap_uint<16> current_row_id_op_fo0 = opix_row_fl0 + iteration_fn0;
				ap_uint<16> current_col_id_op_fo0;
				if (iteration_fn0 == 0)
					current_col_id_op_fo0 = opix_col_fl0;
				else
					current_col_id_op_fo0 = 0;

				if (col_cnt_fo0 == 0)
				{
					lrn_row_offset = opix_row_wo_offset_fl0;
					lrn_row_offset_opix = opix_row_fl0;
					lrn_col_offset = opix_col_fl0;
				}
				else if ((lrn_col_offset) > (input_desc.width - 2))
				{
					lrn_row_offset++;
					lrn_row_offset_opix++;
					lrn_col_offset = 0;
				}
				else
				{
					lrn_col_offset++;
				}



				ap_uint<16> row_id_in_offset_fo0 = current_row_id_op_fo0 * weight_desc.filter_stride;
				ap_uint<16> row_id_in_bram_add = (opix_row_wo_offset_fl0 + iteration_fn0);

				ap_uint<16> row_id_in_bram_offset_fo0;
#pragma HLS resource variable=row_id_in_bram_offset_fo0 core=DSP48 latency=2 
				row_id_in_bram_offset_fo0 = row_id_in_bram_add * weight_desc.filter_stride;

				ap_uint<16> col_id_in_offset_fo0 = current_col_id_op_fo0 * weight_desc.filter_stride;
				ap_uint<16> istg_content_fo0 = conv_desc.iw_mul_istg_row_count;

				ap_uint<8> row_cnt_dilated_fo0 = row_cnt_fo0 * weight_desc.dilation_factor;
				ap_int<16> row_id_in_fo0 = -conv_desc.pad_num + row_cnt_dilated_fo0 + row_id_in_offset_fo0;
				ap_int<16> col_id_in0_fo0 = -conv_desc.pad_num + col_cnt_fo0 + col_id_in_offset_fo0;
				ap_int<16> row_id_in0_bram_fo0 = pad_row_wo_fc0 + row_cnt_dilated_fo0 + row_id_in_bram_offset_fo0;

				ap_int<16> col_id_in_fo0;
				ap_int<16> row_id_in_bram_fo0;
				if (conv_desc.lrn_inter_sos_enable == 1)
				{
					col_id_in_fo0 = lrn_col_offset;
					row_id_in_bram_fo0 = lrn_row_offset;
				}
				else
				{
					col_id_in_fo0 = col_id_in0_fo0;
					row_id_in_bram_fo0 = row_id_in0_bram_fo0;
				}

				//************************* LRN INTER************************

				ap_uint<64> flag_1st_32pix_buff, flag_next_32pix_buff;

				for (ap_uint<8> pix_bit = 0, pattern_bit = 0;pix_bit < XI_PIX_PROC / 2;pix_bit += 6, pattern_bit++)
				{
#pragma HLS unroll
					for (ap_uint<10> pix_kp = 0; pix_kp < 6; pix_kp++)
					{
#pragma HLS unroll
						if (pix_kp == col_cnt_fo0)
							flag_1st_32pix_buff[pix_bit + pix_kp] = pattern_8kp[pattern_bit];
						else
							flag_1st_32pix_buff[pix_bit + pix_kp] = 0;
					}
				}

				for (ap_uint<10> pix_kp = 2; pix_kp < 6; pix_kp++)
				{
#pragma HLS unroll
					if (pix_kp == col_cnt_fo0)
						flag_next_32pix_buff[pix_kp - 2] = pattern_8kp[4];
					else
						flag_next_32pix_buff[pix_kp - 2] = 0;
				}

				for (ap_uint<10> pix_bit = 4, pattern_bit = 5; pix_bit < 22;pix_bit += 6, pattern_bit++)
				{
#pragma HLS unroll
					for (ap_uint<10> pix_kp = 0; pix_kp < 6; pix_kp++)
					{
#pragma HLS unroll

						if (pix_kp == col_cnt_fo0)
							flag_next_32pix_buff[pix_bit + pix_kp] = pattern_8kp[pattern_bit];
						else
							flag_next_32pix_buff[pix_bit + pix_kp] = 0;

					}
				}

				bool lrn_add_bit;
				if (lrn_counter > -1 && lrn_counter < weight_desc.filter_size)
					lrn_add_bit = 1;
				else
					lrn_add_bit = 0;

				if (col_cnt_fo0 == 5)
					pattern_8kp = pattern_8kp * 2 + (ap_uint<8> ) lrn_add_bit;

				if (col_cnt_fo0 == 5)
					lrn_counter++;

				ap_int<16> plane_id_lrn = plane4_fo0 + current_plane_fc0 * 4 - 4;
				ap_uint<2> istg_feedingBuff_dim1_lrn = plane_id_lrn.range(1, 0);
				ap_uint<10> feedingBuff_dim3_1st_32pix = plane4_fo0.range(11,2);
				ap_uint<10> feedingBuff_dim3_next_32pix = plane4_fo0.range(11,2) + 512;
				//********************************************

				ap_uint<3> istg_dim2_fo0;
				istg_datatype istg_dim3_fo0;
				if (conv_desc.lrn_inter_sos_enable == 1)
				{
					istg_dim2_fo0 = plane_id_lrn.range(4, 2);
					istg_dim3_fo0 = (col_id_in_fo0 + row_id_in_bram_fo0 * input_desc.width) + (plane_id_lrn.range(15, 5)) * istg_content_fo0;
				}
				else
				{
#if XI_ROW8// 8 rows in istg BRAM
					istg_dim2_fo0 = row_id_in_bram_fo0.range(5,3); ///8;
					istg_dim3_fo0 = (input_desc.width * (row_id_in_bram_fo0.range(2,0))) + col_id_in_fo0;
#elif XI_ROW4// 4 rows in istg BRAM
					istg_dim2_fo0 = row_id_in_bram_fo0.range(4,2); ///4;
					istg_dim3_fo0 = (input_desc.width * (row_id_in_bram_fo0.range(1,0))) + col_id_in_fo0;
#elif XI_ROW2// 2 rows in istg BRAM
					istg_dim2_fo0 = row_id_in_bram_fo0.range(3,1); ///2;
					istg_dim3_fo0 = (input_desc.width * (row_id_in_bram_fo0[0])) + col_id_in_fo0;
#endif
				}

				ap_uint<3> istg_dim2_fo0_reg;
				istg_datatype istg_dim3_fo0_reg;
				reg_addr_rowPixFun(istg_dim2_fo0, istg_dim3_fo0,&istg_dim2_fo0_reg, &istg_dim3_fo0_reg);

				short istg_value_fo0[4];
#pragma HLS ARRAY_PARTITION variable=istg_value_fo0 complete dim=0
				short istg_value_lrn_inter;
				//istg_datatype buff_index = istg_dim3_fo0_reg;
				ap_uint<64> read_istg = istaging_buff0_fb0[istg_dim2_fo0_reg][istg_dim3_fo0_reg];

				Plane_fo0_Loop:
				for (ap_uint<10> plane_fo0 = 0, bit=0; plane_fo0 < 4;plane_fo0++, bit+=16)
				{
#pragma HLS UNROLL
					if (conv_desc.lrn_inter_sos_enable == 1)
					{
						if (lrn_counter > 0 && plane_id_lrn > -1 && plane_id_lrn < input_desc.planes)
							istg_value_fo0[plane_fo0] = read_istg.range(bit+15, bit);
						else
							istg_value_fo0[plane_fo0] = 0;
					}
					else
					{
						if (row_id_in_fo0 < 0 || col_id_in_fo0 < 0 || row_id_in_fo0 >= input_desc.height || col_id_in_fo0 >= input_desc.width)
							istg_value_fo0[plane_fo0] = 0;
						else
							istg_value_fo0[plane_fo0] = read_istg.range(bit+15, bit);
					}
				} //Plane_fo0_Loop

				short istg_value_fo0_reg[4];
#pragma HLS ARRAY_PARTITION variable=istg_value_fo0_reg complete dim=0
				reg_data_rowPixfun(istg_value_fo0, istg_value_fo0_reg);

				istg_value_lrn_inter = istg_value_fo0_reg[istg_feedingBuff_dim1_lrn];

				short istg_value2_fo0[4];
#pragma HLS ARRAY_PARTITION variable=istg_value2_fo0 complete dim=0
				for (ap_uint<3> iter = 0; iter < 4; iter++)
				{
#pragma HLS UNROLL
					if (mac_fz0 != iter && conv_desc.intra_en)
						istg_value2_fo0[iter] = 0;
					else
						istg_value2_fo0[iter] = istg_value_fo0_reg[iter];
				}

				ap_uint<16> flag_pattern_fo0 = conv_desc.fsz_by_stride_x_stride;

				bool add_shift_fo0[XI_PIX_PROC / 2];
#pragma HLS ARRAY_PARTITION variable=add_shift_fo0 complete dim=0
				Pix_fo1_Loop:
				for (ap_uint<8> pix_fo1 = 0;
						pix_fo1 < XI_PIX_PROC / 2; pix_fo1++)
				{
#pragma HLS unroll
					if (col_cnt_fo0 < flag_pattern_fo0)
						add_shift_fo0[pix_fo1] = initial_flag_reg_fo0[pix_fo1];
					else
						add_shift_fo0[pix_fo1] = 0;
				} //Pix_fo1_Loop

				bool flag_reg_shift_left_fo0[XI_PIX_PROC / 2];
#pragma HLS ARRAY_PARTITION variable=flag_reg_shift_left_fo0 complete dim=0
				Pix_fo2_Loop:
				for (ap_uint<8> pix_fo2 = 0;
						pix_fo2 < XI_PIX_PROC / 2; pix_fo2++)
				{
#pragma HLS unroll
					if (pix_fo2 == 0)
						flag_reg_shift_left_fo0[pix_fo2] = 0;
					else
						flag_reg_shift_left_fo0[pix_fo2] = flag_reg_fo0[pix_fo2 - 1];
				} //Pix_fo2_Loop

				Pix_fo3_Loop:
				for (ap_uint<8> pix_fo3 = 0;
						pix_fo3 < XI_PIX_PROC / 2; pix_fo3++)
				{
#pragma HLS unroll
					bool internel_reg1 = flag_reg_shift_left_fo0[pix_fo3];
					bool internel_reg2 = flag_reg_fo0[pix_fo3] & (fsz_fst_mod_fo0 != 0);

					if (stride_counter_fo0 == 0)
						flag_reg_fo0[pix_fo3] = ((internel_reg1 | internel_reg2)& (col_cnt_fo0 != 0)) | add_shift_fo0[pix_fo3];
					else if (stride_counter_fo0 == (weight_desc.filter_stride - 1))
						flag_reg_fo0[pix_fo3] = ((internel_reg1 & internel_reg2)& (col_cnt_fo0 != 0)) | add_shift_fo0[pix_fo3];
				} //Pix_fo3_Loop

				if ((stride_counter_fo0 < (weight_desc.filter_stride - 1))
						&& (col_cnt_fo0 < (col_max_fo0 - 1)))
					stride_counter_fo0++;
				else
					stride_counter_fo0 = 0;


				ap_uint<64> in_data1_packed;
				ap_uint<64> in_data2_packed;
				Plane_fo1_Loop2:
				for (ap_uint<10> plane_fo1 = 0, bit=0;plane_fo1 < 4; plane_fo1++, bit+=16)
				{
#pragma HLS unroll
					ap_int<16> feeding_buff_data = istg_value2_fo0[plane_fo1];

					if (conv_desc.lrn_inter_sos_enable == 1)
					{
						if (plane_fo1 == istg_feedingBuff_dim1_lrn)
						{
							/*
												if (flag_1st_32pix_buff[pix_fo4] == 1)
													input_buff0_fc0[plane_fo1][pix_fo4][feedingBuff_dim3_1st_32pix] = istg_value_lrn_inter;

												if (flag_next_32pix_buff[pix_fo4] == 1)
													input_buff0_fc0[plane_fo1][pix_fo4][feedingBuff_dim3_next_32pix] = istg_value_lrn_inter;
							 */
						}
					}
					else // SOS disable
					{
						ap_int<16> in_data1;
						ap_int<16> in_data2;
						if (conv_desc.int6_en_in == 0 || conv_desc.batch1_int8_en == 1)
						{
							char data_1 = feeding_buff_data.range(7, 0);
							in_data1.range(7, 0) = data_1;
							in_data1.range(15, 8) = 0; //data_1;
						}
						else
						{
							in_data1.range(7, 0) = feeding_buff_data.range(7, 0);
							in_data1.range(15, 8) = feeding_buff_data.range(15, 8);
						}
						char data_2 = feeding_buff_data.range(15, 8);
						in_data2.range(7, 0) = data_2;
						in_data2.range(15, 8) = 0;

						in_data1_packed.range(bit+15,bit) = in_data1;
						in_data2_packed.range(bit+15,bit) = in_data2;


					}
				} //Plane_fo1_Loop

				Pix_fo4_Loop2:
				for (ap_uint<8> pix_fo4 = 0;pix_fo4 < XI_PIX_PROC / 2; pix_fo4++)
				{
#pragma HLS unroll
					if (flag_reg_fo0[pix_fo4])
					{
						input_buff0_fc0[pix_fo4][addr_bram_fo0[pix_fo4]] = in_data1_packed;

						if (conv_desc.int6_en_in == 0)
						{
							input_buff0_fc0[pix_fo4][addr_bram_fo0[pix_fo4] + 512] = in_data2_packed;
						}
					}
				}

				if (col_cnt_fo0 == col_max_fo0 - 1)
				{
					flag_1st_32pix_buff_previous_iteration = flag_1st_32pix_buff;
					flag_next_32pix_buff_previous_iteration = flag_next_32pix_buff;
				}
				else
				{
					flag_1st_32pix_buff_previous_iteration = 0;
					flag_next_32pix_buff_previous_iteration = 0;
				}



				Pix_fo6_Loop:
				for (ap_uint<8> pix_fo6 = 0;
						pix_fo6 < XI_PIX_PROC / 2; pix_fo6++)
				{
#pragma HLS unroll
					if ((flag_reg_fo0[pix_fo6] == 1)
							&& (dilated_counter_fo0[pix_fo6] == 0))
						addr_bram_fo0[pix_fo6]++;
				} //pix_fo6_Loop

				Pix_fo7_Loop:
				for (ap_uint<8> pix_fo7 = 0;
						pix_fo7 < XI_PIX_PROC / 2; pix_fo7++)
				{
#pragma HLS unroll
					if ((flag_reg_fo0[pix_fo7] == 1) && (dilated_counter_fo0[pix_fo7] != (weight_desc.dilation_factor - 1)) && col_cnt_fo0 != (col_max_fo0 - 1))
						dilated_counter_fo0[pix_fo7]++;
					else
						dilated_counter_fo0[pix_fo7] = 0;
				} //pix_fo7_Loop

			} //Row_Loop
		} //Col_Loop
	} //Plane_Loop

} //RowPixFunction_fo

template<int CFILTER_SIZE, int CCONV_STRIDE, int CNUM_KERNELS, int INPUTP>
void LoadInputBuff32Pix_fn(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<64> input_buff0_fc0[XI_PIX_PROC / 2][1024],
		ap_uint<16> opix_row_fl0,
		ap_uint<16> opix_col_fl0,
		ap_uint<16> opix_row_wo_offset_fl0,
		ap_uint<16> current_plane_fc0,
		ap_int<16> pad_row_fc0,
		ap_int<16> pad_row_wo_fc0,
		ap_uint<2> mac_fz0,
		ap_uint<8> start_pix_count_fn0,
		ap_uint<8> pix_count_num,
		ap_uint<10> inputBuff_bram_offset_fl0
)
{
#pragma HLS inline off

	ap_uint<8> pxcnt_fn0;
	if (start_pix_count_fn0 < XI_PIX_PROC / 2)
	{
		pxcnt_fn0 = start_pix_count_fn0; //pix_count_num;
	}
	else
	{
		pxcnt_fn0 = start_pix_count_fn0 - XI_PIX_PROC / 2;
	}
	ap_uint<16> next_col_id_1st32pix_fn0 = opix_col_fl0 + pix_count_num - conv_desc.pix2_div_outwidth_x_ow;
	ap_uint<16> row_cnt_fn0 = conv_desc.pix2_div_outwidth;
	ap_uint<16> col_cnt_fn0;
	if (next_col_id_1st32pix_fn0 == 0)
	{
		col_cnt_fn0 = output_desc.width;
	}
	else if (next_col_id_1st32pix_fn0 <= output_desc.width)
	{
		row_cnt_fn0 += 1;
		col_cnt_fn0 = next_col_id_1st32pix_fn0;
	}
	else
	{
		row_cnt_fn0 += 2;
		col_cnt_fn0 = next_col_id_1st32pix_fn0 - output_desc.width;
	}

	ap_uint<10> iteration_cnt_fn0;
	if (pix_count_num == 0)
		iteration_cnt_fn0 = 0;
	else if (conv_desc.lrn_inter_sos_enable == 1)
		iteration_cnt_fn0 = 1;
	else
		iteration_cnt_fn0 = row_cnt_fn0;

	ap_uint<10> last_row_entries_fn0 = col_cnt_fn0;

	Iteration_Loop:
	for (ap_uint<10> iteration_fn0 = 0;iteration_fn0 < iteration_cnt_fn0; iteration_fn0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=1 max=1

		ap_uint<8> pixel_tobe_process_fn0;
		if (conv_desc.lrn_inter_sos_enable == 1)
		{
			pixel_tobe_process_fn0 = conv_desc.pix_per_kp;
		}
		else
		{
			if (iteration_fn0 == 0)
			{
				if ((pix_count_num) + opix_col_fl0 >= output_desc.width)
					pixel_tobe_process_fn0 = (output_desc.width)
					- (opix_col_fl0 + 1);
				else
					pixel_tobe_process_fn0 = (pix_count_num) - 1;
			}
			else if (iteration_fn0 == iteration_cnt_fn0 - 1)
			{
				pixel_tobe_process_fn0 = last_row_entries_fn0 - 1;
			}
			else
			{
				pixel_tobe_process_fn0 = (output_desc.width) - 1;
			}
		}

		RowPixFunction_fo<CFILTER_SIZE, CCONV_STRIDE, CNUM_KERNELS, INPUTP>
		(input_desc, conv_desc, output_desc, weight_desc,istaging_buff0_fb0, input_buff0_fc0, opix_row_fl0, opix_col_fl0,
				/*..continue..*/opix_row_wo_offset_fl0, current_plane_fc0,inputBuff_bram_offset_fl0, pad_row_fc0, pad_row_wo_fc0,pxcnt_fn0,
				/*..continue..*/pixel_tobe_process_fn0, iteration_fn0, mac_fz0);

		pxcnt_fn0 += (pixel_tobe_process_fn0 + 1);
	} //Iteration_Loop

}

template<int CFILTER_SIZE, int CCONV_STRIDE, int CNUM_KERNELS, int INPUTP>
void LoadInputBuff_non1x1_fn(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<64> input_buff0_fc0[XI_PIX_PROC / 2][1024],
		ap_uint<16> pix_row_1st_fl0,
		ap_uint<16> pix_col_1st_fl0,
		ap_uint<16> pix_row_1st_wo_offset_fl0,
		ap_uint<16> pix_row_2nd_fl0,
		ap_uint<16> pix_col_2nd_fl0,
		ap_uint<16> pix_row_2nd_wo_offset_fl0,
		ap_uint<16> current_plane_fc0,
		ap_int<16> pad_row_fc0,
		ap_int<16> pad_row_wo_fc0,
		ap_uint<2> mac_fz0,
		ap_uint<8> start_pix_count_fn0)
{
#pragma HLS inline off

	ap_uint<8> pix_count_1st_half;
	ap_uint<8> pix_count_2nd_half;
	ap_uint<8> start_pix_count_1st_half_fn0;
	ap_uint<8> start_pix_count_2nd_half_fn0;

	if (conv_desc.lrn_inter_sos_enable == 1)
	{
		pix_count_1st_half = XI_PIX_PROC;
		pix_count_2nd_half = 0;

		start_pix_count_1st_half_fn0 = 0;
		start_pix_count_2nd_half_fn0 = 0;
	}
	else if (conv_desc.lrn_intra_sos_enable == 1)
	{
		if ((start_pix_count_fn0 + 6) < XI_PIX_PROC / 2)
		{
			pix_count_1st_half = 6;
			pix_count_2nd_half = 0;

			start_pix_count_1st_half_fn0 = start_pix_count_fn0;
			start_pix_count_2nd_half_fn0 = 0;
		}
		else if ((start_pix_count_fn0) > XI_PIX_PROC / 2 && (start_pix_count_fn0 + 6) > XI_PIX_PROC / 2)
		{
			pix_count_1st_half = 0;
			pix_count_2nd_half = 6;

			start_pix_count_1st_half_fn0 = 0;
			start_pix_count_2nd_half_fn0 = start_pix_count_fn0;
		}
		else
		{
			pix_count_1st_half = XI_PIX_PROC / 2 - start_pix_count_fn0;
			pix_count_2nd_half = 6 - (XI_PIX_PROC / 2 - start_pix_count_fn0);

			start_pix_count_1st_half_fn0 = start_pix_count_fn0;
			start_pix_count_2nd_half_fn0 = XI_PIX_PROC / 2;
		}
	}
	else // conv operation
	{
		pix_count_1st_half = XI_PIX_PROC / 2;
		pix_count_2nd_half = XI_PIX_PROC / 2;

		start_pix_count_1st_half_fn0 = 0;
		start_pix_count_2nd_half_fn0 = XI_PIX_PROC / 2;
	}

	for (ap_uint<10> addr = 0, addr1 = 512; addr < 4; addr++, addr1++)
	{
#pragma HLS DEPENDENCE variable=input_buff0_fc0 inter false
#pragma HLS PIPELINE
		for (ap_uint<8> pix_buf = 0; pix_buf < XI_PIX_PROC / 2; pix_buf++)
		{
#pragma HLS unroll

			input_buff0_fc0[pix_buf][addr] = 0;
			input_buff0_fc0[pix_buf][addr1] = 0;

		}
	}

	LoadInputBuff32Pix_fn<CFILTER_SIZE, CCONV_STRIDE, CNUM_KERNELS, INPUTP>
	(input_desc, conv_desc, output_desc, weight_desc, istaging_buff0_fb0,input_buff0_fc0, pix_row_1st_fl0, pix_col_1st_fl0,
			/*..continue..*/pix_row_1st_wo_offset_fl0, current_plane_fc0,pad_row_fc0, pad_row_wo_fc0, mac_fz0, start_pix_count_1st_half_fn0,pix_count_1st_half, 0);

	if (conv_desc.int6_en_in == 1)
	{
		LoadInputBuff32Pix_fn<CFILTER_SIZE, CCONV_STRIDE, CNUM_KERNELS, INPUTP>
		(input_desc, conv_desc, output_desc, weight_desc,istaging_buff0_fb0, input_buff0_fc0, pix_row_2nd_fl0,pix_col_2nd_fl0,
				/*..continue..*/pix_row_2nd_wo_offset_fl0, current_plane_fc0,pad_row_fc0, pad_row_wo_fc0, mac_fz0,start_pix_count_2nd_half_fn0, pix_count_2nd_half, 512);
	}
}


void lrn_inter_plane_select(ap_uint<8> *pattern_next_state,
		ap_uint<8> pad_num,
		ap_uint<16> input_planes,
		ap_uint<16> center_plane,
		ap_uint<8> ker_id,
		bool plane_select[4][2],
		ap_uint<16> plane_start)
{
#pragma HLS ARRAY_PARTITION variable=plane_select complete dim=0
#pragma HLS inline off

	ap_uint<8> pattern_present_state = *pattern_next_state;
	if (pattern_present_state[7] == 1)
		*pattern_next_state = 31;
	else
		*pattern_next_state = (pattern_present_state << 1)+ pattern_present_state[4];

	ap_int<16> plane_offset = ker_id + center_plane - pad_num;
	if (plane_offset < 0)
	{
		plane_start = 0;
	}
	else
	{
		plane_start = 4 * plane_offset.range(15, 2);
	}

	for (ap_uint<4> plane = 0; plane < 8; plane++)
	{
#pragma HLS unroll
		ap_uint<16> plane_id = plane + plane_offset;
		if (plane_id > input_planes)
			plane_select[plane_id.range(1, 0)][plane_id[0]] = 0;
		else
			plane_select[plane_id.range(1, 0)][plane_id[0]] = pattern_present_state[plane];
	}
}
void reg_addr_feedLayerX(ap_uint<14> istg_addr_off[16], ap_uint<14> istg_addr_off_reg[16])
{
#pragma HLS interface register port=istg_addr_off_reg
#pragma HLS ARRAY_PARTITION variable=istg_addr_off complete dim=0
#pragma HLS ARRAY_PARTITION variable=istg_addr_off_reg complete dim=0
#pragma HLS inline off
	for(ap_uint<5> pl =0; pl<16; pl++)
	{
#pragma HLS unroll
		istg_addr_off_reg[pl] = istg_addr_off[pl];
	}
}

void reg_addr_feedlrx_1(ap_uint<14> istg_addr_off[8], ap_uint<14> istg_addr_off_reg[8])
{
#pragma HLS interface register port=istg_addr_off_reg
#pragma HLS ARRAY_PARTITION variable=istg_addr_off complete dim=0
#pragma HLS ARRAY_PARTITION variable=istg_addr_off_reg complete dim=0
#pragma HLS inline off
	for(ap_uint<5> pl =0; pl<8; pl++)
	{
#pragma HLS unroll
		istg_addr_off_reg[pl] = istg_addr_off[pl];
	}
}

void reg_addr_feedlrx_2(ap_uint<14> istg_addr_off[8], ap_uint<14> istg_addr_off_reg[8])
{
#pragma HLS interface register port=istg_addr_off_reg
#pragma HLS ARRAY_PARTITION variable=istg_addr_off complete dim=0
#pragma HLS ARRAY_PARTITION variable=istg_addr_off_reg complete dim=0
#pragma HLS inline off
	for(ap_uint<5> pl =0; pl<8; pl++)
	{
#pragma HLS unroll
		istg_addr_off_reg[pl] = istg_addr_off[pl];
	}
}
void reg_data_feedLayerX(short data_istg[4][16], short data_istg_reg[4][16])
{
#pragma HLS interface register port=data_istg_reg
#pragma HLS ARRAY_PARTITION variable=data_istg complete dim=0
#pragma HLS ARRAY_PARTITION variable=data_istg_reg complete dim=0
#pragma HLS inline off
	for(ap_uint<3> d1 =0; d1<4; d1++)
	{
		for(ap_uint<5> d2 =0; d2<16; d2++)
		{
			data_istg_reg[d1][d2] = data_istg[d1][d2];
		}
	}
}

void LoadInputBuffLayerX_fm(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<64> input_buff0_fc0[XI_PIX_PROC / 2][1024],
		ap_uint<16> current_plane_by_4,
		ap_int<16> in_row_num[XI_PIX_PROC],
		ap_int<16> in_col_num[XI_PIX_PROC],
		ap_int<16> in_row_num_istg[XI_PIX_PROC],
		ap_uint<10> feeding_buff_off[16],
		ap_uint<8> pxcnt_loopmax)
{
#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=1
#if XI_ISTG_URAM_EN==0
#pragma HLS resource variable=istaging_buff0_fb0 core=RAM_T2P_BRAM latency=2
#else
#pragma HLS RESOURCE variable=istaging_buff0_fb0 core=XPM_MEMORY uram
#endif
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#if XI_FEED_URAM_EN==0
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=input_buff0_fc0 core=XPM_MEMORY uram
#endif


#pragma HLS ARRAY_PARTITION variable=in_row_num complete dim=0
#pragma HLS ARRAY_PARTITION variable=in_col_num complete dim=0
#pragma HLS ARRAY_PARTITION variable=in_row_num_istg complete dim=0
#pragma HLS ARRAY_PARTITION variable=feeding_buff_off complete dim=0
#pragma HLS inline off

	ap_uint<9> ker_row_dilated = 0;
	ap_uint<9> ker_col_dilated = 0;
	ap_uint<12> patch_ele;

	Int8_2x_loop:
	for (ap_uint<2> int8_2x_cnt = 0;int8_2x_cnt < conv_desc.int8_2x_loopbound; int8_2x_cnt++)
	{
#pragma HLS LOOP_TRIPCOUNT min=1 max=1
		PLANE:
		for (ap_uint<16> plane_num = 0;plane_num < input_desc.compute_planes; plane_num += 64)
		{
#pragma HLS LOOP_TRIPCOUNT min=1 max=1
			PXCNT:
			for (ap_uint<8> pix_cnt_by16 = 0;pix_cnt_by16 < pxcnt_loopmax; pix_cnt_by16++)
			{
#pragma HLS LOOP_TRIPCOUNT min=1 max=2
				PXUPDT:
				for (ap_uint<5> pix_update = 0, pu2 = 16;pix_update < 16; pix_update++, pu2--)
				{
					ROW:
					for (ap_uint<9> ker_row = 0;ker_row < weight_desc.filter_size; ker_row++)
					{
#pragma HLS LOOP_TRIPCOUNT min=5 max=5
						COL:
						for (ap_uint<9> ker_col = 0;ker_col < weight_desc.filter_size; ker_col++)
						{
#pragma HLS LOOP_TRIPCOUNT min=5 max=5
#pragma HLS pipeline
#pragma HLS loop_flatten
#pragma HLS DEPENDENCE variable=input_buff0_fc0 intra false
#pragma HLS DEPENDENCE variable=input_buff0_fc0 inter false
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 intra false
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 inter false

							ap_uint<8> pix_cnt = pix_cnt_by16 << 4;

							//*****************pixel index generation******************//
							ap_uint<4> pix16[16];
#pragma HLS ARRAY_PARTITION variable=pix16 complete dim=0
							ap_uint<4> pix16_2[16];
#pragma HLS ARRAY_PARTITION variable=pix16_2 complete dim=0
							for (ap_uint<7> pix_w = 0; pix_w < 16; pix_w++)
							{
#pragma HLS unroll
								pix16[pix_w] = pix_update + pix_w;
								pix16_2[pix_w] = pu2 + pix_w;
							} //pix_w

							if (ker_col == 0)
								ker_col_dilated = 0;
							if (ker_row == 0)
								ker_row_dilated = 0;

							ap_uint<14> istgrow_x_width = conv_desc.istg_row_count * input_desc.width;

							//*****************istg buff address genration******************//

							ap_uint<14> istg_addr_off[16];
#pragma HLS ARRAY_PARTITION variable=istg_addr_off complete dim=0
							bool pad_flags[16];
#pragma HLS ARRAY_PARTITION variable=pad_flags complete dim=0
							for (ap_uint<5> pix_w = 0; pix_w < 16; pix_w++)
							{
#pragma HLS unroll
								ap_uint<16> istg_row_istg = (ap_uint<16>)in_row_num_istg[pix_w + pix_cnt] + (ap_uint<16>)ker_row_dilated;
								ap_int<16> istg_row = in_row_num[pix_w + pix_cnt] + ker_row_dilated;
								ap_int<16> istg_col = in_col_num[pix_w + pix_cnt] + ker_col_dilated;
								ap_uint<16> istg_col_us = istg_col;
								ap_uint<16> plane_add_by4 = plane_num.range(15, 2) + current_plane_by_4;
								ap_uint<13> addr_off_istg = (plane_add_by4 / 8) * istgrow_x_width;
								ap_uint<13> addr = istg_row_istg * input_desc.width + istg_col_us + addr_off_istg;


								ap_uint<4> pix_sum = pix_w + pix_update;
								if (pix_sum[3] == 1)
									istg_addr_off[pix_w] = addr + istgrow_x_width;
								else
									istg_addr_off[pix_w] = addr;

								if (istg_row[15] == 0 && istg_col[15] == 0 && istg_col < input_desc.width && istg_row < input_desc.height)
									pad_flags[pix_w] = 0;
								else
									pad_flags[pix_w] = 1;
							}                        //pix_w

							//*********************** data read from istg buff *************************//
							ap_uint<14> istg_addr_off_reg[16];
#pragma HLS ARRAY_PARTITION variable=istg_addr_off_reg complete dim=0
							reg_addr_feedLayerX(istg_addr_off,istg_addr_off_reg);


							ap_uint<14> istg_addr_1[8];
#pragma HLS ARRAY_PARTITION variable=istg_addr_1 complete dim=0

							ap_uint<14> istg_addr_2[8];
#pragma HLS ARRAY_PARTITION variable=istg_addr_2 complete dim=0

							ap_uint<14> istg_addr_reg_1[8];
#pragma HLS ARRAY_PARTITION variable=istg_addr_reg_1 complete dim=0

							ap_uint<14> istg_addr_reg_2[8];
#pragma HLS ARRAY_PARTITION variable=istg_addr_reg_2 complete dim=0

							for (ap_uint<5> pix_w1 = 0, pix_w2 = 8; pix_w1 < 8;pix_w1++, pix_w2++)
							{
#pragma HLS unroll
								istg_addr_1[pix_w1]	= istg_addr_off_reg[pix16_2[pix_w1]];
								istg_addr_2[pix_w1]	= istg_addr_off_reg[pix16_2[pix_w2]];

							}

							reg_addr_feedlrx_1(istg_addr_1, istg_addr_reg_1);
							reg_addr_feedlrx_2(istg_addr_2, istg_addr_reg_2);

							short data_istg_reg[4][16];
#pragma HLS ARRAY_PARTITION variable=data_istg_reg complete dim=0
							short data_istg[4][16];
#pragma HLS ARRAY_PARTITION variable=data_istg complete dim=0
							for (ap_uint<5> pix_w1 = 0, pix_w2 = 8; pix_w1 < 8;pix_w1++, pix_w2++)
							{
#pragma HLS unroll
								for (ap_uint<10> pl = 0, bit=0; pl < 4; pl++, bit+=16)
								{
#pragma HLS unroll
									istg_datatype buffer_index1 = istg_addr_reg_1[pix_w1];
									istg_datatype buffer_index2 = istg_addr_reg_2[pix_w1];
									data_istg_reg[pl.range(1,0)][pix_w1] = (short)istaging_buff0_fb0[pix_w1][buffer_index1].range(bit+15,bit);
									data_istg_reg[pl.range(1,0)][pix_w2] = (short)istaging_buff0_fb0[pix_w1][buffer_index2].range(bit+15,bit);
								}
							}

							reg_data_feedLayerX(data_istg_reg, data_istg);
							//******************** data rearragement and zero padding **********************************//
							ap_uint<64> data_inbuff[16];
#pragma HLS ARRAY_PARTITION variable=data_inbuff complete dim=0
							ap_uint<64> data_inbuff2[16];
#pragma HLS ARRAY_PARTITION variable=data_inbuff2 complete dim=0
							for (ap_uint<5> pix_w = 0; pix_w < 16; pix_w++)
							{
#pragma HLS unroll
								for (ap_uint<10> pl = 0, bit=0; pl < 4; pl++, bit+=16)
								{
#pragma HLS unroll
									ap_int<16> istg_value;
									if (pad_flags[pix_w] == 0)
										istg_value = data_istg[pl][pix16[pix_w]];
									else
										istg_value = 0;

									ap_int<16> in_data1;
									ap_int<16> in_data2;

									char data1 = istg_value.range(7, 0);
									char data2 = istg_value.range(15, 8);
									in_data2.range(7, 0) = data2;
									in_data2.range(15, 8) = 0;
									if (conv_desc.int6_en_in == 0 || conv_desc.batch1_int8_en == 1)
									{
										in_data1.range(7, 0) = data1;
										in_data1.range(15, 8) = 0;
									}
									else
										in_data1 = istg_value;

									data_inbuff[pix_w].range(bit+15,bit) = in_data1;
									data_inbuff2[pix_w].range(bit+15,bit) = in_data2;

								}
							}


							//************************** feeding buff address genration and write enable flag generation ****************//
							ap_uint<12> feeding_buff_addr[16];
#pragma HLS ARRAY_PARTITION variable=feeding_buff_addr complete dim=0
							bool pix_wr_flag[16];
#pragma HLS ARRAY_PARTITION variable=pix_wr_flag complete dim=0


							if (ker_row == 0 && ker_col == 0)
								patch_ele = 0;

							ap_uint<12> addr_off_inBuff = plane_num.range(15, 2) * weight_desc.filter_size_square;
							for (ap_uint<5> pix_w = 0; pix_w < 16; pix_w++)
							{
#pragma HLS unroll
								ap_uint<12> addr = feeding_buff_off[pix16[pix_w]] + addr_off_inBuff;

								ap_uint<12> addr_to_feeedingbuff;
#pragma HLS resource variable=addr_to_feeedingbuff core=AddSubnS
								addr_to_feeedingbuff = addr + patch_ele;
								feeding_buff_addr[pix_w] = addr_to_feeedingbuff;

								if (addr_to_feeedingbuff < 512)
									pix_wr_flag[pix_w] = 1;
								else
									pix_wr_flag[pix_w] = 0;

							}
							patch_ele++;

							//**** update next ieration
							if (ker_col == (weight_desc.filter_size - 1))
								ker_row_dilated += weight_desc.dilation_factor;
							ker_col_dilated += weight_desc.dilation_factor;
							//************************

							//************************ feeding buff writting **********************************//
#if XI_PIX_PROC==4

							for(ap_uint<5> pix_w = 0; pix_w < 2; pix_w++)
							{
#pragma HLS unroll
								if(pix_wr_flag[pix_w] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[pix_w][feeding_buff_addr[pix_w]] = data_inbuff[pix_w];
									else
										input_buff0_fc0[pix_w][feeding_buff_addr[pix_w]+512] = data_inbuff2[pix_w];
								}
								if(pix_wr_flag[pix_w+2] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[pix_w][feeding_buff_addr[pix_w+2]+512] = data_inbuff[pix_w+2];
								}
							}//pix_w
#elif XI_PIX_PROC==8
							for(ap_uint<5> pix_w = 0; pix_w < 4; pix_w++)
							{
#pragma HLS unroll
								if(pix_wr_flag[pix_w] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[pix_w][feeding_buff_addr[pix_w]] = data_inbuff[pix_w];
									else
										input_buff0_fc0[pix_w][feeding_buff_addr[pix_w]+512] = data_inbuff2[pix_w];
								}
								if(pix_wr_flag[pix_w+4] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[pix_w][feeding_buff_addr[pix_w+4]+512] = data_inbuff[pix_w+4];
								}
							}//pix_w
#elif XI_PIX_PROC==16
							for(ap_uint<5> pix_w = 0; pix_w < 8; pix_w++)
							{
#pragma HLS unroll
								if(pix_wr_flag[pix_w] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[pix_w][feeding_buff_addr[pix_w]] = data_inbuff[pix_w];
									else
										input_buff0_fc0[pix_w][feeding_buff_addr[pix_w]+512] = data_inbuff2[pix_w];
								}
								if(pix_wr_flag[pix_w+8] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[pix_w][feeding_buff_addr[pix_w+8]+512] = data_inbuff[pix_w+8];
								}
							}//pix_w
#elif XI_PIX_PROC == 32
							ap_uint<10> pix_sharing_off;
							if(pix_cnt < XI_PIX_PROC/2)
								pix_sharing_off = 0;
							else
								pix_sharing_off = 512;

							for(ap_uint<6> pix_w = 0; pix_w < 16; pix_w++)
							{
#pragma HLS unroll
								if(pix_wr_flag[pix_w] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[pix_w+0][feeding_buff_addr[pix_w]+pix_sharing_off] = data_inbuff[pix_w];
									else
										input_buff0_fc0[pix_w+0][feeding_buff_addr[pix_w]+512] = data_inbuff2[pix_w];
								}
							}//pix_w

#elif XI_PIX_PROC == 24
							ap_uint<10> pix_sharing_off;
							if(pix_cnt < XI_PIX_PROC/2)
								pix_sharing_off = 0;
							else
								pix_sharing_off = 512;

							if(pix_cnt == 0)
							{
								for(ap_uint<6> pix_w = 0; pix_w < 12; pix_w++)
								{
#pragma HLS unroll
									if(pix_wr_flag[pix_w] == 1)
									{
										if(int8_2x_cnt == 0)
											input_buff0_fc0[pix_w+0][feeding_buff_addr[pix_w]+pix_sharing_off] = data_inbuff[pix_w];
										else
											input_buff0_fc0[pix_w+0][feeding_buff_addr[pix_w]+512] = data_inbuff2[pix_w];
									}
								}//pix_w

								if(pix_wr_flag[12] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[0][feeding_buff_addr[12]+512] = data_inbuff[12];
								}
								if(pix_wr_flag[13] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[1][feeding_buff_addr[13]+512] = data_inbuff[13];
								}

								if(pix_wr_flag[14] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[2][feeding_buff_addr[14]+512] = data_inbuff[14];
								}
								if(pix_wr_flag[15] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[3][feeding_buff_addr[15]+512] = data_inbuff[15];
								}

							}
							else
							{
								for(ap_uint<6> pix_w = 0; pix_w < 8; pix_w++)
								{
#pragma HLS unroll
									if(pix_wr_flag[pix_w] == 1)
									{
										if(int8_2x_cnt == 0)
											input_buff0_fc0[pix_w+4][feeding_buff_addr[pix_w]+512] = data_inbuff[pix_w];
									}
								}//pix_w
							}

#elif XI_PIX_PROC == 28
							ap_uint<10> pix_sharing_off;
							if(pix_cnt < XI_PIX_PROC/2)
								pix_sharing_off = 0;
							else
								pix_sharing_off = 512;

							if(pix_cnt == 0)
							{
								for(ap_uint<6> pix_w = 0; pix_w < 14; pix_w++)
								{
#pragma HLS unroll
									if(pix_wr_flag[pix_w] == 1)
									{
										if(int8_2x_cnt == 0)
											input_buff0_fc0[pix_w+0][feeding_buff_addr[pix_w]] = data_inbuff[pix_w];
										else
											input_buff0_fc0[pix_w+0][feeding_buff_addr[pix_w]+512] = data_inbuff2[pix_w];
									}
								}//pix_w
								if(pix_wr_flag[14] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[0][feeding_buff_addr[14]] = data_inbuff[14];
								}
								if(pix_wr_flag[15] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[1][feeding_buff_addr[15]] = data_inbuff[15];
								}
							}
							else
							{
								for(ap_uint<6> pix_w = 0; pix_w < 12; pix_w++)
								{
#pragma HLS unroll
									if(pix_wr_flag[pix_w] == 1)
									{
										if(int8_2x_cnt == 0)
											input_buff0_fc0[pix_w+2][feeding_buff_addr[pix_w]] = data_inbuff[pix_w];
									}
								}//pix_w
							}

#elif XI_PIX_PROC == 26
							ap_uint<10> pix_sharing_off;
							if(pix_cnt < XI_PIX_PROC/2)
								pix_sharing_off = 0;
							else
								pix_sharing_off = 512;

							if(pix_cnt == 0)
							{
								for(ap_uint<6> pix_w = 0; pix_w < 13; pix_w++)
								{
#pragma HLS unroll
									if(pix_wr_flag[pix_w] == 1)
									{
										if(int8_2x_cnt == 0)
											input_buff0_fc0[pix_w+0][feeding_buff_addr[pix_w]] = data_inbuff[pix_w];
										else
											input_buff0_fc0[pix_w+0][feeding_buff_addr[pix_w]+512] = data_inbuff2[pix_w];
									}
								}//pix_w
								if(pix_wr_flag[13] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[0][feeding_buff_addr[13]+512] = data_inbuff[13];
								}
								if(pix_wr_flag[14] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[1][feeding_buff_addr[14]+512] = data_inbuff[14];
								}
								if(pix_wr_flag[15] == 1)
								{
									if(int8_2x_cnt == 0)
										input_buff0_fc0[2][feeding_buff_addr[15]+512] = data_inbuff[15];
								}
							}
							else
							{
								for(ap_uint<6> pix_w = 0; pix_w < 10; pix_w++)
								{
#pragma HLS unroll
									if(pix_wr_flag[pix_w] == 1)
									{
										if(int8_2x_cnt == 0)
											input_buff0_fc0[pix_w+3][feeding_buff_addr[pix_w]+512] = data_inbuff[pix_w];
									}
								}//pix_w
							}

#elif XI_PIX_PROC == 52

							//********
							ap_uint<10> pix_sharing_off;
							if(pix_cnt < XI_PIX_PROC/2)
								pix_sharing_off = 0;
							else
								pix_sharing_off = 512;

							if(pix_cnt == 0)
							{
								for(ap_uint<6> pix_w = 0; pix_w < 16; pix_w++)
								{
#pragma HLS unroll
									if(pix_wr_flag[pix_w] == 1)
									{
										if(int8_2x_cnt == 0)
											input_buff0_fc0[pix_w+0][feeding_buff_addr[pix_w]] = data_inbuff[pix_w];
										else
											input_buff0_fc0[pix_w+0][feeding_buff_addr[pix_w]+512] = data_inbuff2[pix_w];
									}
								}//pix_w
							}
							else if(pix_cnt == 16)
							{
								for(ap_uint<6> pix_w = 0; pix_w < 10; pix_w++)
								{
#pragma HLS unroll
									for(ap_uint<3> pl = 0; pl < 4; pl++)
									{
#pragma HLS unroll
										if(pix_wr_flag[pix_w] == 1)
										{
											if(int8_2x_cnt == 0)
												input_buff0_fc0[pix_w+16][feeding_buff_addr[pix_w]] = data_inbuff[pix_w];
											else
												input_buff0_fc0[pix_w+16][feeding_buff_addr[pix_w]+512] = data_inbuff2[pix_w];
										}
									}//pl
								}//pix_w

								for(ap_uint<6> pix_w = 10, pix_bram=0; pix_w < 16; pix_w++, pix_bram++)
								{
#pragma HLS unroll
									if(pix_wr_flag[pix_w] == 1)
									{
										if(int8_2x_cnt == 0)
											input_buff0_fc0[pix_bram][feeding_buff_addr[pix_w]+512] = data_inbuff[pix_w];
									}
								}//pix_w
							}
							else if(pix_cnt == 32)
							{

								for(ap_uint<7> pix_w = 0, pix_bram=6; pix_w < 16; pix_w++, pix_bram++)
								{
#pragma HLS unroll
									if(pix_wr_flag[pix_w] == 1)
									{
										if(int8_2x_cnt == 0)
											input_buff0_fc0[pix_bram][feeding_buff_addr[pix_w]+512] = data_inbuff[pix_w];

									}
								}//pix_w
							}
							else
							{

								for(ap_uint<7> pix_w = 0, pix_bram=22; pix_w < 4; pix_w++, pix_bram++)
								{
#pragma HLS unroll
									if(pix_wr_flag[pix_w] == 1)
									{
										if(int8_2x_cnt == 0)
											input_buff0_fc0[pix_bram][feeding_buff_addr[pix_w]+512] = data_inbuff[pix_w];

									}
								}//pix_w
							}
#elif XI_PIX_PROC == 64
							ap_uint<10> pix_sharing_off;
							if(pix_cnt < XI_PIX_PROC/2)
								pix_sharing_off = 0;
							else
								pix_sharing_off = 512;

							if(pix_cnt.range(4,0) == 0)
							{
								for(ap_uint<6> pix_w = 0; pix_w < 16; pix_w++)
								{
#pragma HLS unroll
									if(pix_wr_flag[pix_w] == 1)
									{
										if(int8_2x_cnt == 0)
											input_buff0_fc0[pix_w+0][feeding_buff_addr[pix_w]+pix_sharing_off] = data_inbuff[pix_w];
										else
											input_buff0_fc0[pix_w+0][feeding_buff_addr[pix_w]+512] = data_inbuff2[pix_w];
									}
								}//pix_w
							}

							else
							{
								for(ap_uint<6> pix_w = 0; pix_w < 16; pix_w++)
								{
#pragma HLS unroll
									if(pix_wr_flag[pix_w] == 1)
									{
										if(int8_2x_cnt == 0)
											input_buff0_fc0[pix_w+16][feeding_buff_addr[pix_w]+pix_sharing_off] = data_inbuff[pix_w];
										else
											input_buff0_fc0[pix_w+0][feeding_buff_addr[pix_w]+512] = data_inbuff2[pix_w];
									}
								}//pix_w
							}
#endif//64 pix_proc

						}//pix_update
					}//pix_cnt
				}//ker_col
			}//ker_row
		}//plane_num
	}//int8 loop

}//LoadInputBuffLayerX_fm

template<int CFILTER_SIZE, int CCONV_STRIDE, int CNUM_KERNELS, int INPUTP>
void LoadFeedingBuff_fl(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		out_pix_struct &out_pixels0_fc0,
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<64> input_buff0_fc0[XI_PIX_PROC / 2][1024],
		ap_uint<16> pc_fc0,
		ap_uint<16> current_plane_fc0,
		ap_uint<16> out_row_offset_fb0,
		ap_int<16> pad_row_fc0,
		ap_int<16> pad_row_wo_fc0,
		ap_uint<16> row_id_1st_32pix_fc0,
		ap_uint<16> col_id_1st_32pix_fc0,
		ap_uint<16> row_id_2nd_32pix_fc0,
		ap_uint<16> col_id_2nd_32pix_fc0,
		ap_uint<2> mac_fz0,
		ap_uint<16> *pc_out)
{
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#if XI_FEED_URAM_EN==0
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=input_buff0_fc0 core=XPM_MEMORY uram
#endif

#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=1
#if XI_ISTG_URAM_EN==0
#pragma HLS resource variable=istaging_buff0_fb0 core=RAM_T2P_BRAM latency=2
#else
#pragma HLS RESOURCE variable=istaging_buff0_fb0 core=XPM_MEMORY uram
#endif
#pragma HLS inline off

	*pc_out = pc_fc0;


	ap_uint<16> pix_row_1st_wo_offset_fl0 = row_id_1st_32pix_fc0;
	ap_uint<16> pix_row_1st_fl0 = row_id_1st_32pix_fc0 + out_row_offset_fb0;
	ap_uint<16> pix_col_1st_fl0 = col_id_1st_32pix_fc0;
	ap_uint<16> pix_row_2nd_wo_offset_fl0 = row_id_2nd_32pix_fc0;
	ap_uint<16> pix_row_2nd_fl0 = row_id_2nd_32pix_fc0 + out_row_offset_fb0;
	ap_uint<16> pix_col_2nd_fl0 = col_id_2nd_32pix_fc0;

	out_pix2_struct outpixels1_fl0;
	out_pix2_struct outpixels2_fl0;
#pragma HLS ARRAY_PARTITION variable=outpixels1_fl0.pix_cols complete dim=1
#pragma HLS ARRAY_PARTITION variable=outpixels1_fl0.pix_rows complete dim=1
#pragma HLS ARRAY_PARTITION variable=outpixels2_fl0.pix_cols complete dim=1
#pragma HLS ARRAY_PARTITION variable=outpixels2_fl0.pix_rows complete dim=1

	ap_uint<16> center_plane = current_plane_fc0;

	bool plane_select_all[4][2];
#pragma HLS ARRAY_PARTITION variable=plane_select_all complete dim=0
	for (ap_uint<4> pl = 0; pl < 4; pl++)
	{
#pragma HLS unroll
		for (ap_uint<3> set = 0; set < 2; set++)
		{
#pragma HLS unroll
			plane_select_all[pl][set] = 1;
		}
	}

	ap_uint<8> ker_loop_cnt;
	if (conv_desc.lrn_intra_sos_enable == 1 || conv_desc.bn_intra_sos_enable == 1)
		ker_loop_cnt = XI_KER_PROC;
	else
		ker_loop_cnt = 1;

	ap_uint<16> var1, var2, var3;
	if (conv_desc.sos_enable == 0)
	{
		var1 = pix_row_2nd_fl0;
		var2 = pix_col_2nd_fl0;
		var3 = pix_row_2nd_wo_offset_fl0;
	}
	else
	{
		var1 = 0;
		var2 = 0;
		var3 = 0;
	}

	ap_int<16> out_row_istg = pix_row_1st_wo_offset_fl0;
	ap_int<16> out_row = pix_row_1st_fl0;
	ap_int<16> out_col = pix_col_1st_fl0;
	ap_int<16> in_row_num[XI_PIX_PROC];
	ap_int<16> in_col_num[XI_PIX_PROC];
	ap_int<16> in_row_num_istg[XI_PIX_PROC];
#pragma HLS ARRAY_PARTITION variable=in_row_num complete dim=0
#pragma HLS ARRAY_PARTITION variable=in_col_num complete dim=0
#pragma HLS ARRAY_PARTITION variable=in_row_num_istg complete dim=0

	for (ap_uint<8> pix = 0; pix < XI_PIX_PROC; pix++)
	{
#pragma HLS pipeline
		if (out_col == output_desc.width)
		{
			out_col = 0;
			out_row++;
			out_row_istg++;
		}

		in_row_num[pix] = out_row * weight_desc.filter_stride - conv_desc.pad_num;
		in_col_num[pix] = out_col * weight_desc.filter_stride - conv_desc.pad_num;
		in_row_num_istg[pix] = out_row_istg * weight_desc.filter_stride + pad_row_wo_fc0;    //conv_desc.pad_num;

		out_pixels0_fc0.pix_rows[pix] = out_row;
		out_pixels0_fc0.pix_cols[pix] = out_col;

		out_col++;
	}

	ap_uint<10> feeding_buff_off[16];
#pragma HLS ARRAY_PARTITION variable=feeding_buff_off complete dim=0
#pragma HLS ARRAY_PARTITION variable=conv_desc.feed_addr16 complete dim=0
	for (ap_uint<5> pix_load = 0; pix_load < 16; pix_load++)
	{
#pragma HLS unroll
		feeding_buff_off[pix_load] = conv_desc.feed_addr16[pix_load];
	}

	bool inc1 = (conv_desc.pix_per_kp.range(3, 0) != 0);
	ap_uint<8> pxcnt_loopmax = conv_desc.pix_per_kp.range(7, 4) + inc1;

	if (conv_desc.l2_sos_enable == 1 || (conv_desc.sos_enable == 0 && conv_desc.layer_id != 0))
	{

		LoadInputBuffLayerX_fm(input_desc, conv_desc, output_desc, weight_desc,istaging_buff0_fb0, input_buff0_fc0,
				/*..continue..*/current_plane_fc0, in_row_num, in_col_num,in_row_num_istg, feeding_buff_off, pxcnt_loopmax);

	}
	else
	{
		for (ap_uint<8> start_pix_per_ker, ker = 0; ker < ker_loop_cnt;start_pix_per_ker += conv_desc.pix_per_kp, ker++)
		{

			LoadInputBuff_non1x1_fn<CFILTER_SIZE, CCONV_STRIDE, CNUM_KERNELS,INPUTP>
			(input_desc, conv_desc, output_desc, weight_desc,istaging_buff0_fb0, input_buff0_fc0, pix_row_1st_fl0,pix_col_1st_fl0,
					/*..continue..*/pix_row_1st_wo_offset_fl0, var1, var2, var3,current_plane_fc0, pad_row_fc0, pad_row_wo_fc0, mac_fz0,start_pix_per_ker);
		}
	}

	out_pixels0_fc0.current_plane_by4 = current_plane_fc0;
#if XI_DP_ENABLE
	ap_uint<10> dim3 = conv_desc.compute_loop_count;
	for(ap_uint<7> pix =0;pix<XI_PIX_PROC/2;pix++)
	{
#pragma HLS UNROLL

		input_buff0_fc0[pix][dim3] = 0;
		input_buff0_fc0[pix][dim3+512] = 0;

	}
#endif


}

ap_int<32> abs_32bit(ap_int<32> in)
																														{

	if (in[31] == 0)
		return in;
	else
		return -in;
																														}

ap_int<24> abs_24bit(ap_int<24> in)
																														{

	if (in[23] == 0)
		return in;
	else
		return -in;
																														}

void round_fun(ap_int<32> in, ap_int<32> rounding_value, ap_int<32> *out,ap_uint<5> inout_precision)
{
	ap_int<32> sum = abs_32bit(in) + rounding_value;
	*out = sum >> inout_precision;
}

void round_fun24bit(ap_int<24> in, ap_int<24> rounding_value, ap_int<24> *out,ap_uint<5> inout_precision)
{
	ap_int<24> sum = abs_24bit(in) + rounding_value;
	*out = sum >> inout_precision;
}

void saturation_fun(ap_int<24> word2_neg2, ap_int<8> *out, bool int6_en_out)
{
#pragma HLS interface register port=word2_neg2
#pragma HLS interface register port=out
#pragma HLS inline off
	ap_int<8> word2_neg2_p;
	if (int6_en_out == 0)
	{
		if (word2_neg2 > 127)
			word2_neg2_p = 127;
		else if (word2_neg2 < -128)
			word2_neg2_p = -128;
		else
			word2_neg2_p = (ap_int<8> ) word2_neg2;

	}
	else
	{
		if (word2_neg2 > 31)
			word2_neg2_p = 31;
		else if (word2_neg2 < -32)
			word2_neg2_p = -32;
		else
			word2_neg2_p = (ap_int<8> ) word2_neg2;

	}

	*out = word2_neg2_p;
}

void saturation_round(ap_int<32> ostg_word_out_fh0[2][OUT_PARTITION],
		ap_int<16> word_relu_fh0[OUT_PARTITION],
		ap_uint<5> inout_precision,
		bool int6_en_out,
		bool relu_en,
		bool crelu_bit,
		ap_int<32> rounding_value)
{
#pragma HLS ARRAY_PARTITION variable=ostg_word_out_fh0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=word_relu_fh0 complete dim=0
#pragma HLS interface register port=ostg_word_out_fh0
#pragma HLS interface register port=word_relu_fh0
#pragma HLS inline

	for (ap_uint<4> pl = 0; pl < OUT_PARTITION; pl++)
	{
#pragma HLS UNROLL
		ap_int<32> word_img0_mux;
		ap_int<32> word_img1_mux;
		round_fun(ostg_word_out_fh0[0][pl], rounding_value, &word_img0_mux,inout_precision);
		round_fun(ostg_word_out_fh0[1][pl], rounding_value, &word_img1_mux,inout_precision);

		ap_int<32> word_img0;
		ap_int<32> word_img1;

		if (ostg_word_out_fh0[0][pl][31] == 0)
			word_img0 = word_img0_mux;
		else
			word_img0 = -word_img0_mux;

		if (ostg_word_out_fh0[1][pl][31] == 0)
			word_img1 = word_img1_mux;
		else
			word_img1 = -word_img1_mux;

		ap_int<32> word_neg_fh0;
		ap_int<32> word2_neg_fh0;
		if (crelu_bit == 0)
		{
			word_neg_fh0 = word_img0;
			word2_neg_fh0 = word_img1;
		}
		else
		{
			word_neg_fh0 = -word_img0;
			word2_neg_fh0 = -word_img1;
		}

		ap_int<18> word_neg2 = word_neg_fh0;
		ap_int<18> word2_neg2 = word2_neg_fh0;

		ap_int<8> word_neg2_p;
		ap_int<8> word2_neg2_p;

		saturation_fun(word_neg2, &word_neg2_p, int6_en_out);
		saturation_fun(word2_neg2, &word2_neg2_p, int6_en_out);

		ap_int<8> word_neg3 = word_neg2_p;
		ap_int<8> word2_neg3 = word2_neg2_p;

		if ((relu_en == 1) && (word_neg_fh0[17] == 1))
			word_relu_fh0[pl].range(7, 0) = 0;
		else
			word_relu_fh0[pl].range(7, 0) = word_neg3;

		if ((relu_en == 1) && (word2_neg_fh0[17] == 1))
			word_relu_fh0[pl].range(15, 8) = 0;
		else
			word_relu_fh0[pl].range(15, 8) = word2_neg3;
	}
}

void saturation_round_batch1(ap_int<24> ostg_word_fh0,    ap_int<8> *word_relu_fh0, ap_uint<5> inout_precision,bool int6_en_out, bool relu_en, bool crelu_bit,ap_int<24> rounding_value)
{
#pragma HLS interface register port=ostg_word_fh0
#pragma HLS interface register port=word_relu_fh0
#pragma HLS inline off

	ap_int<24> word_img0_mux;
	round_fun24bit(ostg_word_fh0, rounding_value, &word_img0_mux,inout_precision);

	ap_int<24> word_img0;

	if (ostg_word_fh0[23] == 0)
		word_img0 = word_img0_mux;
	else
		word_img0 = -word_img0_mux;

	ap_int<24> word_neg_fh0;
	if (crelu_bit == 0)
	{
		word_neg_fh0 = word_img0;
	}
	else
	{
		word_neg_fh0 = -word_img0;
	}

	ap_int<24> word_neg2 = word_neg_fh0;

	ap_int<8> word_neg2_p;

	saturation_fun(word_neg2, &word_neg2_p, int6_en_out);

	ap_int<8> word_neg3 = word_neg2_p;

	if ((relu_en == 1) && (word_neg_fh0[23] == 1))
		*word_relu_fh0 = 0;
	else
		*word_relu_fh0 = word_neg3;
}
void reg_scale_val(ap_uint<24> scale_value_batch2[4][4],ap_uint<24> scale_value_batch2_reg[4][4])
{
#pragma HLS INLINE OFF
#pragma HLS interface register port=scale_value_batch2_reg
#pragma HLS ARRAY_PARTITION variable=scale_value_batch2_reg complete dim=0
#pragma HLS ARRAY_PARTITION variable=scale_value_batch2 complete dim=0
	for(int i=0;i<4;i++)
	{
#pragma HLS UNROLL
		for(int j=0;j<4;j++)
		{
#pragma HLS UNROLL
			scale_value_batch2_reg[i][j] = scale_value_batch2[i][j];
		}
	}

}
template<int OUT_WW>
void OutputWrite_fk(conv_struct conv_desc, ap_uint<16> pixbuff_planeoffset_fj0,
		gmem_outputtype *gmem_output1_fa0,
#if !XI_SINGLE_IO_PORT_EN
		gmem_outputtype *gmem_output2_fa0,
#endif
		bool oStagBuf_dim2_bool_fj0,
		ap_uint<72> ostaging_buff0_fb0[2][XI_OSTG_BUFFER_SET][XI_OSTAGEBUFF_DEPTH],
		ap_uint<16> layerx_loop_cnt_fj0, output_struct output_desc,
		bool crelu_bit, ap_uint<16> row_stage_offset, bool write_normal,
		ap_int<16> bias_buff_fb0[8][XI_BIASMAXSIZE],
		ap_uint<24> scale_buff_fb0[8][XI_BIASMAXSIZE], ap_uint<16> outImg_fj0,
		ap_uint<16> *fc_pixel_count,
		ap_uint<2> plane_mod2)
{
#pragma HLS inline off

	ap_uint<16> counter_ow_x_ostgrow_t = 0;
	ap_uint<16> counter_plane16_t = 0;
	ap_uint<16> offset_all_featureMap = 0;
	bool eff_plane32_bool_fh0_t = oStagBuf_dim2_bool_fj0;
	ap_uint<2> eff_plane_mod2 = plane_mod2;

	ap_int<32> rounding_value = conv_desc.rounding_conv;

	ap_uint<16> fc_bias_idx = *fc_pixel_count;


	Write_Loop:
#if !XI_IO_64bit_PORT_EN
	for (ap_uint<16> ddr_pntr_fk0 = 0;    ddr_pntr_fk0 < layerx_loop_cnt_fj0; ddr_pntr_fk0++)
#else
		for (ap_uint<16> ddr_pntr_fk0 = 0;    ddr_pntr_fk0 < layerx_loop_cnt_fj0*2; ddr_pntr_fk0++)
#endif
		{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=11*120 max=11*120


			ap_uint<16> ostg_addr_new = row_stage_offset + counter_ow_x_ostgrow_t + offset_all_featureMap;

			ap_uint<16> ostg_addr_fk0 = ostg_addr_new;
			bool eff_plane32_bool_fk0 = eff_plane32_bool_fh0_t;



			ap_uint<128> word1_128bit_fk0;
			ap_uint<128> word2_128bit_fk0;

			if(conv_desc.batch1_int8_en==0)//#if XI_BATCH1_EN==0
			{
#if !XI_SINGLE_IO_PORT_EN
				ap_uint<16> bias_index_conv = (outImg_fj0/16) + counter_plane16_t;
				ap_uint<16> bias_index_fc = fc_bias_idx/16;
#else//if !XI_IO_64bit_PORT_EN
				ap_uint<16> bias_index_conv = (outImg_fj0/8) + counter_plane16_t;
				ap_uint<16> bias_index_fc = fc_bias_idx/8;
#endif
				ap_uint<16> bias_index;
				if(conv_desc.fc_enable == 1)
					bias_index = bias_index_fc;
				else
					bias_index = bias_index_conv;

#if !XI_SINGLE_IO_PORT_EN
				ap_uint<16> bias_index1 = bias_index*2;
				ap_uint<16> bias_index2 = bias_index*2 + 1;
#else//if XI_SINGLE_IO_PORT_EN && !XI_IO_64bit_PORT_EN
				ap_uint<16> bias_index1 = bias_index;
				ap_uint<16> bias_index2 = bias_index;
#endif
				ap_int<16> bias_from_bram[16];
#pragma HLS ARRAY_PARTITION variable=bias_from_bram complete dim=0
				for(ap_uint<5> idx=0; idx<8;idx++)
				{
#pragma HLS UNROLL
					ap_int<16> word1 = bias_buff_fb0[idx][bias_index1];
					ap_int<16> word2 = bias_buff_fb0[idx][bias_index2];
					bias_from_bram[idx] 	= ((ap_int<16>)word1);
					bias_from_bram[idx+8] 	= ((ap_int<16>)word2);
				}

				ap_uint<24> scale_from_bram[16];
#pragma HLS ARRAY_PARTITION variable=scale_from_bram complete dim=0
				for(ap_uint<5> idx=0; idx<8;idx++)
				{
#pragma HLS UNROLL
					ap_uint<24> scaleword1 = scale_buff_fb0[idx][bias_index1];
					ap_uint<24> scaleword2 = scale_buff_fb0[idx][bias_index2];
					scale_from_bram[idx] 	= ((ap_uint<24>)scaleword1);
					scale_from_bram[idx+8] 	= ((ap_uint<24>)scaleword2);
				}

				ap_int<32> bias_value_batch2[4][4];
				ap_uint<24> scale_value_batch2[4][4],scale_value_batch2_reg[4][4];
#pragma HLS ARRAY_PARTITION variable=scale_value_batch2_reg complete dim=0
#pragma HLS ARRAY_PARTITION variable=scale_value_batch2 complete dim=0
				ap_int<16> bias_value_fc = bias_from_bram[fc_bias_idx.range(3,0)];
				ap_uint<24> scale_value_fc = scale_from_bram[fc_bias_idx.range(3,0)];
				for(ap_uint<5> idx=0; idx<16;idx++)
				{
#pragma HLS UNROLL
					if(conv_desc.fc_enable == 1)
					{

						if(conv_desc.offline_quant_en==1)
						{
							// bias conversion: from Q8.8 to Q8.18
							bias_value_batch2[idx.range(1,0)][idx.range(3,2)] 	= ((ap_int<32>)bias_value_fc)<<14;
							scale_value_batch2[idx.range(1,0)][idx.range(3,2)] 	= scale_value_fc;
						}
						else
						{
							bias_value_batch2[idx.range(1,0)][idx.range(3,2)] 	= (ap_int<32>)bias_value_fc;
							scale_value_batch2[idx.range(1,0)][idx.range(3,2)] 	= 1;
						}
					}
					else
					{
						if(conv_desc.offline_quant_en==1)
						{
							// bias conversion: from Q8.8 to Q8.18
							bias_value_batch2[idx.range(1,0)][idx.range(3,2)] 	= ((ap_int<32>)bias_from_bram[idx])<<14;
							scale_value_batch2[idx.range(1,0)][idx.range(3,2)] 	= scale_from_bram[idx];
						}
						else
						{

							bias_value_batch2[idx.range(1,0)][idx.range(3,2)] 	= (ap_int<32>)bias_from_bram[idx];
							scale_value_batch2[idx.range(1,0)][idx.range(3,2)] 	= 1;
						}
					}


				}
				reg_scale_val(scale_value_batch2,scale_value_batch2_reg);
				Outtype_Loop:
				for (ap_uint<8> planes_fh0 = 0, bit_fh0 = 0,bit2_fh0 = 64, bit18=0; planes_fh0 < 4;    planes_fh0++, bit_fh0 += 16, bit2_fh0 += 16, bit18+=18)
				{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL

					ap_int<18> ostg_word_fh0[2][4];
#pragma HLS ARRAY_PARTITION variable=ostg_word_fh0 complete dim=0

#if XI_OSTG_BUFFER_SET==8

#if !XI_SINGLE_IO_PORT_EN && !XI_IO_64bit_PORT_EN
					if (eff_plane32_bool_fk0 == 0)
					{
						for (ap_uint<4> pl = 0; pl < 4; pl++)
						{
#pragma HLS UNROLL
							ostg_word_fh0[0][pl] =    ostaging_buff0_fb0[0][pl][ostg_addr_fk0].range(bit18 + 17, bit18);
							ostg_word_fh0[1][pl] =    ostaging_buff0_fb0[1][pl][ostg_addr_fk0].range(bit18 + 17, bit18);
						}
					}
					else
					{
						for (ap_uint<4> pl = 0; pl < 4; pl++)
						{
#pragma HLS UNROLL
							ostg_word_fh0[0][pl] =  ostaging_buff0_fb0[0][pl + 4][ostg_addr_fk0].range(bit18 + 17, bit18);
							ostg_word_fh0[1][pl] =  ostaging_buff0_fb0[1][pl + 4][ostg_addr_fk0].range(bit18 + 17, bit18);
						}
					}
#else
					if (eff_plane_mod2.range(1,0) == 0)
					{
						for (ap_uint<4> pl = 0; pl < 2; pl++)
						{
#pragma HLS UNROLL
							ostg_word_fh0[0][pl] = ostaging_buff0_fb0[0][pl][ostg_addr_fk0].range(bit18 + 17, bit18);
							ostg_word_fh0[1][pl] = ostaging_buff0_fb0[1][pl][ostg_addr_fk0].range(bit18 + 17, bit18);
						}
					}
					else if (eff_plane_mod2.range(1,0) == 1)
					{
						for (ap_uint<4> pl = 0; pl < 2; pl++)
						{
#pragma HLS UNROLL
							ostg_word_fh0[0][pl] = ostaging_buff0_fb0[0][pl + 2][ostg_addr_fk0].range(bit18 + 17, bit18);
							ostg_word_fh0[1][pl] = ostaging_buff0_fb0[1][pl + 2][ostg_addr_fk0].range(bit18 + 17, bit18);
						}
					}
					else if (eff_plane_mod2.range(1,0) == 2)
					{
						for (ap_uint<4> pl = 0; pl < 2; pl++)
						{
#pragma HLS UNROLL
							ostg_word_fh0[0][pl] = ostaging_buff0_fb0[0][pl + 4][ostg_addr_fk0].range(bit18 + 17, bit18);
							ostg_word_fh0[1][pl] = ostaging_buff0_fb0[1][pl + 4][ostg_addr_fk0].range(bit18 + 17, bit18);
						}
					}
					else
					{
						for (ap_uint<4> pl = 0; pl < 2; pl++)
						{
#pragma HLS UNROLL
							ostg_word_fh0[0][pl] = ostaging_buff0_fb0[0][pl + 6][ostg_addr_fk0].range(bit18 + 17, bit18);
							ostg_word_fh0[1][pl] = ostaging_buff0_fb0[1][pl + 6][ostg_addr_fk0].range(bit18 + 17, bit18);
						}
					}
#endif//NUM_IO_PORT==2

#else//XI_OSTG_BUFFER_SET==4
#if !XI_SINGLE_IO_PORT_EN && !XI_IO_64bit_PORT_EN
					for (ap_uint<4> pl = 0; pl < 4; pl++)
					{
#pragma HLS UNROLL
						ostg_word_fh0[0][pl] = ostaging_buff0_fb0[0][pl][ostg_addr_fk0].range(bit18 + 17, bit18);
						ostg_word_fh0[1][pl] = ostaging_buff0_fb0[1][pl][ostg_addr_fk0].range(bit18 + 17, bit18);
					}
#elif XI_SINGLE_IO_PORT_EN && !XI_IO_64bit_PORT_EN
					if (eff_plane_mod2[0] == 0)
					{
						for (ap_uint<4> pl = 0; pl < 2; pl++)
						{
#pragma HLS UNROLL
							ostg_word_fh0[0][pl] = ostaging_buff0_fb0[0][pl][ostg_addr_fk0].range(bit18 + 17, bit18);
							ostg_word_fh0[1][pl] = ostaging_buff0_fb0[1][pl][ostg_addr_fk0].range(bit18 + 17, bit18);
						}
					}
					else
					{
						for (ap_uint<4> pl = 0; pl < 2; pl++)
						{
#pragma HLS UNROLL
							ostg_word_fh0[0][pl] = ostaging_buff0_fb0[0][pl+2][ostg_addr_fk0].range(bit18 + 17, bit18);
							ostg_word_fh0[1][pl] = ostaging_buff0_fb0[1][pl+2][ostg_addr_fk0].range(bit18 + 17, bit18);
						}
					}
#else
					if (ddr_pntr_fk0[0] == 0)
					{
						for (ap_uint<4> pl = 0, pld = 0; pl < 4; pl+=2, pld++)
						{
#pragma HLS UNROLL
							ostg_word_fh0[0][pld] = ostaging_buff0_fb0[0][pl][ostg_addr_fk0].range(bit18 + 17, bit18);
							ostg_word_fh0[1][pld] = ostaging_buff0_fb0[1][pl][ostg_addr_fk0].range(bit18 + 17, bit18);
						}
					}
					else
					{
						for (ap_uint<4> pl = 1, pld = 0; pl < 4; pl+=2, pld++)
						{
#pragma HLS UNROLL
							ostg_word_fh0[0][pld] = ostaging_buff0_fb0[0][pl][ostg_addr_fk0].range(bit18 + 17, bit18);
							ostg_word_fh0[1][pld] = ostaging_buff0_fb0[1][pl][ostg_addr_fk0].range(bit18 + 17, bit18);
						}
					}
#endif

#endif//XI_OSTG_BUFFER_SET

					ap_int<32> ostg_word_out_fh0[2][OUT_PARTITION];
#pragma HLS ARRAY_PARTITION variable=ostg_word_fh0 complete dim=0
#pragma HLS resource variable=ostg_word_out_fh0 core=DSP48 latency=2
#if !XI_SINGLE_IO_PORT_EN && XI_IO_64bit_PORT_EN
					for (ap_uint<4> pl = 0; pl < OUT_PARTITION; pl++)
					{
#pragma HLS UNROLL
						ap_int<18> mul_in1_0,mul_in1_1;
						ap_int<24> mul_in2;
						ap_int<32> add_in2;
						if (ddr_pntr_fk0[0] == 0)
						{
							mul_in1_0 = ostg_word_fh0[0][pl];
							mul_in1_1 = ostg_word_fh0[1][pl];
							mul_in2 = scale_value_batch2_reg[planes_fh0][2*pl];
							add_in2 = bias_value_batch2[planes_fh0][2*pl];
						}
						else
						{
							mul_in1_0 = ostg_word_fh0[0][pl];
							mul_in1_1 = ostg_word_fh0[1][pl];
							mul_in2 = scale_value_batch2_reg[planes_fh0][2*pl + 1];
							add_in2 = bias_value_batch2[planes_fh0][2*pl + 1];
						}
						ostg_word_out_fh0[0][pl] = mul_in1_0*mul_in2 + add_in2;//ostg_word_fh0[0][pl] * scale_value_batch2_reg[planes_fh0][pl] + bias_value_batch2[planes_fh0][pl];
						ostg_word_out_fh0[1][pl] = mul_in1_1*mul_in2 + add_in2;//ostg_word_fh0[1][pl]	* scale_value_batch2_reg[planes_fh0][pl] + bias_value_batch2[planes_fh0][pl];
					}
#else
					for (ap_uint<4> pl = 0; pl < OUT_PARTITION; pl++)
					{
#pragma HLS UNROLL
						ostg_word_out_fh0[0][pl] = ostg_word_fh0[0][pl] * scale_value_batch2_reg[planes_fh0][pl] + bias_value_batch2[planes_fh0][pl];
						ostg_word_out_fh0[1][pl] = ostg_word_fh0[1][pl]	* scale_value_batch2_reg[planes_fh0][pl] + bias_value_batch2[planes_fh0][pl];
					}
#endif

					ap_int<16> word_relu_fh0[OUT_PARTITION];
					saturation_round(ostg_word_out_fh0, word_relu_fh0,conv_desc.inout_precision, conv_desc.int6_en_out,conv_desc.relu_en, crelu_bit, rounding_value);

					word1_128bit_fk0.range(bit_fh0 + 15, bit_fh0) = word_relu_fh0[0];
					word1_128bit_fk0.range(bit2_fh0 + 15, bit2_fh0) = word_relu_fh0[1];
#if !XI_SINGLE_IO_PORT_EN && !XI_IO_64bit_PORT_EN
					word2_128bit_fk0.range(bit_fh0 + 15, bit_fh0) = word_relu_fh0[2];
					word2_128bit_fk0.range(bit2_fh0 + 15, bit2_fh0) = word_relu_fh0[3];
#endif
				}//Outtype_loop
			}
			else
			{
				Outtype_Loop1:
				for (ap_uint<10> set_fh0 = 0, bit_fh0 = 0; set_fh0 < 4;    set_fh0++, bit_fh0 += 32)
				{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL

					ap_uint<72> word72bit_ostg = ostaging_buff0_fb0[0][set_fh0][ostg_addr_fk0];
					ap_uint<32> word32bit_sat;

					for(ap_uint<10> plane=0, bit18=0, bit8=0; plane<4; plane++,bit18+=18,bit8+=8)
					{
#pragma HLS UNROLL
						ap_int<18> ostg_word_fh0 =  word72bit_ostg.range(bit18 + 17, bit18);
						ap_int<8> word_relu_fh0;
						saturation_round_batch1(ostg_word_fh0, &word_relu_fh0,
								conv_desc.inout_precision, 0,
								conv_desc.relu_en, crelu_bit, rounding_value);
						word32bit_sat.range(bit8 + 7, bit8) = word_relu_fh0;
					}

					word1_128bit_fk0.range(bit_fh0 + 31, bit_fh0) = word32bit_sat;

				}//Outtype_loop

#if XI_OSTG_BUFFER_SET==8
				Outtype_Loop2:
				for (ap_uint<10> set_fh0 = 4, bit_fh0 = 0; set_fh0 < 8; set_fh0++, bit_fh0 += 32)
				{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL

					ap_uint<72> word72bit_ostg = ostaging_buff0_fb0[0][set_fh0][ostg_addr_fk0];
					ap_uint<32> word32bit_sat;

					for(ap_uint<10> plane=0, bit18=0, bit8=0; plane<4; plane++,bit18+=18,bit8+=8)
					{
#pragma HLS UNROLL
						ap_int<18> ostg_word_fh0 = word72bit_ostg.range(bit18 + 17, bit18);
						ap_int<8> word_relu_fh0;
						saturation_round_batch1(ostg_word_fh0, &word_relu_fh0,
								conv_desc.inout_precision, 0,
								conv_desc.relu_en, crelu_bit, rounding_value);
						word32bit_sat.range(bit8 + 7, bit8) = word_relu_fh0;
					}

					word2_128bit_fk0.range(bit_fh0 + 31, bit_fh0) = word32bit_sat;

				} //Outtype_loop
#endif

			}//Batch1 or batch2

			ap_uint<128> write1,write2;
			if(write_normal==1)
			{
				write1 = word1_128bit_fk0;
			}
			else
			{
				write1 = word2_128bit_fk0;
			}
#if	!XI_IO_64bit_PORT_EN
			gmem_output1_fa0[ddr_pntr_fk0] = write1;
#else
			gmem_output1_fa0[ddr_pntr_fk0] = write1.range(63,0);
#endif

#if !XI_SINGLE_IO_PORT_EN && !XI_IO_64bit_PORT_EN
			;
			if(write_normal==1)
			{
				write2 = word2_128bit_fk0;
			}
			else
			{
				write2 = word1_128bit_fk0;
			}
			gmem_output2_fa0[ddr_pntr_fk0] = write2;
#elif !XI_SINGLE_IO_PORT_EN && XI_IO_64bit_PORT_EN
			gmem_output2_fa0[ddr_pntr_fk0] = write1.range(127,64);
#endif

#if !XI_IO_64bit_PORT_EN
			counter_ow_x_ostgrow_t++;
#else
			if(ddr_pntr_fk0[0]==1)
				counter_ow_x_ostgrow_t++;
#endif//XI_IO_64bit_PORT_EN

			if (counter_ow_x_ostgrow_t == conv_desc.ow_mul_ostg_row_count)
			{
				counter_ow_x_ostgrow_t = 0;
				counter_plane16_t++;

#if XI_OSTG_BUFFER_SET==8
				if(conv_desc.batch1_int8_en==0)
				{
#if !XI_SINGLE_IO_PORT_EN && !XI_IO_64bit_PORT_EN
					if (eff_plane32_bool_fh0_t == 1)
						offset_all_featureMap += conv_desc.ow_mul_ostg_row_count;
					eff_plane32_bool_fh0_t = !eff_plane32_bool_fh0_t;
#else
					if(eff_plane_mod2 == 3)
						offset_all_featureMap += conv_desc.ow_mul_ostg_row_count;

#endif
				}
				else
				{
					offset_all_featureMap += conv_desc.ow_mul_ostg_row_count;
				}
#else
#if !XI_SINGLE_IO_PORT_EN && !XI_IO_64bit_PORT_EN
				offset_all_featureMap += conv_desc.ow_mul_ostg_row_count;
#elif XI_SINGLE_IO_PORT_EN && !XI_IO_64bit_PORT_EN
				if(eff_plane_mod2[0] == 1)
					offset_all_featureMap += conv_desc.ow_mul_ostg_row_count;
#else
				if(ddr_pntr_fk0[0] == 1)
					offset_all_featureMap += conv_desc.ow_mul_ostg_row_count;
#endif//port1

#endif//ostg_buffer_set
				eff_plane_mod2++;
			}
			fc_bias_idx++;
		}//Write output loop
	*fc_pixel_count = fc_bias_idx;

}

template<int CNUM_KERNELS, int OUT_WW>
void StoreOStagingBuff_fj(output_struct output_desc, conv_struct conv_desc,
		ap_uint<16> out_row_offset_fb0, gmem_outputtype * gmem_output1_fa0,
#if !XI_SINGLE_IO_PORT_EN
		gmem_outputtype * gmem_output2_fa0,
#endif
		ap_uint<72> ostaging_buff0_fb0[2][XI_OSTG_BUFFER_SET][XI_OSTAGEBUFF_DEPTH],
		ap_uint<16> ostg_row_x_width, bool write_en_fb0,
		ap_int<16> bias_buff_fb0[8][XI_BIASMAXSIZE],
		ap_uint<24> scale_buff_fb0[8][XI_BIASMAXSIZE],
		ap_uint<16> *fc_pixel_count) {
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2
#if XI_OSTG_URAM_EN==0
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM latency=2
#else
#pragma HLS RESOURCE variable=ostaging_buff0_fb0 core=XPM_MEMORY uram
#endif
#if XI_BIAS_URAM_EN==0
#pragma HLS ARRAY_PARTITION variable=bias_buff_fb0 complete dim=1
#pragma HLS resource variable=bias_buff_fb0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=bias_buff_fb0 core=XPM_MEMORY uram
#pragma HLS ARRAY_RESHAPE variable=bias_buff_fb0 complete dim=1
#endif
#if XI_SCALE_URAM_EN==0
#pragma HLS ARRAY_PARTITION variable=scale_buff_fb0 complete dim=1
#pragma HLS resource variable=scale_buff_fb0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=scale_buff_fb0 core=XPM_MEMORY uram
#pragma HLS ARRAY_RESHAPE variable=scale_buff_fb0 complete dim=1
#endif
#pragma HLS inline off

	ap_uint<16> layerx_loop_cnt_fj0;
	ap_uint<16> layerx_img_fj0;

	ap_uint<10> out_plane_align16 = output_desc.planes.range(15, 4)    + output_desc.planes[3];

	if (output_desc.height == conv_desc.ostg_row_count)
	{
#if !XI_SINGLE_IO_PORT_EN
		if(conv_desc.batch1_int8_en==0)
		{		layerx_img_fj0 = 16;
		layerx_loop_cnt_fj0 = output_desc.out_size_x_plane16;
		}
		else
		{
			layerx_img_fj0 = 32;
			layerx_loop_cnt_fj0 = output_desc.out_size_x_plane16/2;
		}
#else
		layerx_img_fj0 = 8;
		layerx_loop_cnt_fj0 = output_desc.out_size_x_plane16*2;
#endif
	}
	else
	{
		layerx_img_fj0 = output_desc.planes;
		layerx_loop_cnt_fj0 = ostg_row_x_width;
	}


	if (write_en_fb0)
	{

		CRELU_LOOP:
		for (ap_uint<3> crelu_itr_fj0 = 0;    crelu_itr_fj0 < conv_desc.conv_c_relu_loop_cnt;    crelu_itr_fj0++)
		{
			ap_uint<32> crelu_off0, crelu_off1;
			if (crelu_itr_fj0 == 0)
			{
				crelu_off0 = 0;
				crelu_off1 = 0;
			}
			else
			{
				crelu_off0 = output_desc.crelu_offset_ptr0;
				crelu_off1 = output_desc.crelu_offset_ptr1;
			}

			ap_uint<16> pixbuff_planeoffset_fj0 = 0;
			ap_uint<32> plane16_data_offset = 0;
			ap_uint<16> row_stage_offset = 0;
			ap_uint<16> plane_packed;
#if !XI_SINGLE_IO_PORT_EN
			if(conv_desc.batch1_int8_en==1)
				plane_packed = 32;
			else
				plane_packed = 16;
#else
			plane_packed = 8;
#endif

			Img_Out_Loop:
			for (ap_uint<16> outImg_fj0 = 0;outImg_fj0 < layerx_img_fj0; outImg_fj0 += plane_packed)
			{
#pragma HLS LOOP_TRIPCOUNT min=2 max=2

				bool oStagBuf_dim2_bool_fj0 = outImg_fj0[4];
				ap_uint<2> plane_mod2 = outImg_fj0.range(4,3);
				int crelu_base_addr;
#pragma HLS resource variable=crelu_base_addr core=DSP48 latency=2
				crelu_base_addr = ((out_row_offset_fb0)    * output_desc.width) + plane16_data_offset;
				ap_uint<32> offset_fj0 =  crelu_base_addr + crelu_off0;
				ap_uint<32> offset_fj1 =  crelu_base_addr + crelu_off1;

				bool crelu_bit = crelu_itr_fj0[0];
				bool write_normal = (crelu_bit==0)||(crelu_off0 == crelu_off1);

				ap_uint<32> offset_fj2;
				if(write_normal==1)
					offset_fj2 = offset_fj0;
				else
					offset_fj2 = offset_fj1;

				gmem_outputtype * gmem_output1_add,* gmem_output2_add;
#pragma HLS resource variable=gmem_output1_add core=AddSubnS latency=2
#pragma HLS resource variable=gmem_output2_add core=AddSubnS latency=2
#if !XI_IO_64bit_PORT_EN
				gmem_output1_add = gmem_output1_fa0 + offset_fj0;
#else
				gmem_output1_add = gmem_output1_fa0 + offset_fj0*2;
#endif
#if !XI_SINGLE_IO_PORT_EN
#if !XI_IO_64bit_PORT_EN
				gmem_output2_add = gmem_output2_fa0 + offset_fj2;
#else
				gmem_output2_add = gmem_output2_fa0 + offset_fj2*2;
#endif//XI_IO_64bit_PORT_EN
#endif//XI_SINGLE_IO_PORT_EN

				OutputWrite_fk<OUT_WW>
				(conv_desc, pixbuff_planeoffset_fj0,gmem_output1_add,
#if !XI_SINGLE_IO_PORT_EN
						gmem_output2_add,
#endif
						oStagBuf_dim2_bool_fj0,
						ostaging_buff0_fb0, layerx_loop_cnt_fj0, output_desc,crelu_bit, row_stage_offset, write_normal,bias_buff_fb0,scale_buff_fb0,outImg_fj0, fc_pixel_count,plane_mod2);


#if !XI_SINGLE_IO_PORT_EN
				if(conv_desc.batch1_int8_en==1)
				{
					pixbuff_planeoffset_fj0 += (conv_desc.out_pix);
					row_stage_offset += conv_desc.ow_mul_ostg_row_count;
				}
				else
				{
#if XI_OSTG_BUFFER_SET==8
					if (oStagBuf_dim2_bool_fj0 == 1)
					{
						pixbuff_planeoffset_fj0 += (conv_desc.out_pix);
						row_stage_offset += conv_desc.ow_mul_ostg_row_count;
					}
#else
					pixbuff_planeoffset_fj0 += (conv_desc.out_pix);
					row_stage_offset += conv_desc.ow_mul_ostg_row_count;
#endif
				}
#else
#if XI_OSTG_BUFFER_SET==8
				if(plane_mod2==3)
				{
					pixbuff_planeoffset_fj0 += (conv_desc.out_pix);
					row_stage_offset += conv_desc.ow_mul_ostg_row_count;
				}
#else
				if(plane_mod2[0]==1)
				{
					pixbuff_planeoffset_fj0 += (conv_desc.out_pix);
					row_stage_offset += conv_desc.ow_mul_ostg_row_count;
				}
#endif

#endif

				plane16_data_offset += output_desc.size;

			}//Img_Out_Loop
		}//crelu_loop


	}
}
template<int CNUM_KERNELS, int OUT_WW>
void StoreOStagingBuff_En_fj(output_struct output_desc, conv_struct conv_desc,
		ap_uint<16> out_row_offset_fb0, gmem_outputtype * gmem_output1_fa0,
#if !XI_SINGLE_IO_PORT_EN
		gmem_outputtype * gmem_output2_fa0,
#endif
		ap_uint<72> ostaging_buff0_fb0[2][XI_OSTG_BUFFER_SET][XI_OSTAGEBUFF_DEPTH],
		bool write_en_fb0, bool last_itr_stage,
		ap_int<16> bias_buff_fb0[8][XI_BIASMAXSIZE],
		ap_uint<24> scale_buff_fb0[8][XI_BIASMAXSIZE],
		ap_uint<16> *fc_pixel_count) {
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2
#if XI_OSTG_URAM_EN==0
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM latency=2
#else
#pragma HLS RESOURCE variable=ostaging_buff0_fb0 core=XPM_MEMORY uram
#endif

#if XI_BIAS_URAM_EN==0
#pragma HLS ARRAY_PARTITION variable=bias_buff_fb0 complete dim=1
#pragma HLS resource variable=bias_buff_fb0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=bias_buff_fb0 core=XPM_MEMORY uram
#pragma HLS ARRAY_RESHAPE variable=bias_buff_fb0 complete dim=1
#endif

#if XI_SCALE_URAM_EN==0
#pragma HLS ARRAY_PARTITION variable=scale_buff_fb0 complete dim=1
#pragma HLS resource variable=scale_buff_fb0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=scale_buff_fb0 core=XPM_MEMORY uram
#pragma HLS ARRAY_RESHAPE variable=scale_buff_fb0 complete dim=1
#endif
#pragma HLS inline off



	ap_uint<16> ostg_row_x_width;
	if (last_itr_stage == 0)
		ostg_row_x_width = conv_desc.ow_mul_ostg_row_count;
	else
		ostg_row_x_width = conv_desc.ow_mul_ostg_row_cnt_last_itr;


	if (!(conv_desc.lrn_pns_enable || conv_desc.bn_snb_enable || conv_desc.elem_wise_add_en || conv_desc.bn_snb_c_relu_en || conv_desc.bn_en || conv_desc.bn_c_relu_en || conv_desc.scale_relu_en || conv_desc.max_pool_en|| conv_desc.avg_pool_en))
	{
		StoreOStagingBuff_fj<CNUM_KERNELS, OUT_WW>
		(output_desc, conv_desc,out_row_offset_fb0, gmem_output1_fa0,
#if !XI_SINGLE_IO_PORT_EN
				gmem_output2_fa0,
#endif
				ostaging_buff0_fb0, ostg_row_x_width, write_en_fb0,bias_buff_fb0,scale_buff_fb0, fc_pixel_count);
	}
}
void InputReadLayer1_fi_mean_sub(ap_uint<16> layer1_loop_cnt_fg0,
		input_struct input_desc,
		conv_struct conv_desc,
		gmem_inputtype_layer1 *gmem_input_layer1_fa0,
		short istaging_buff0_fb0[4][8][XI_ISTAGEBUFF_DEPTH])
{
#pragma HLS INLINE OFF

	ap_int<8> mean_sub_fi0[4];
#pragma HLS ARRAY_PARTITION variable=mean_sub_fi0 complete dim=0
	Mean_Loop:
	for (ap_uint<4> plane_fi1 = 0; plane_fi1 < 4; plane_fi1++)
	{
#pragma HLS UNROLL
		mean_sub_fi0[plane_fi1] = input_desc.mean_value[plane_fi1];
	}

	ap_uint<16> counter_row8_x_iw = 0;
	ap_uint<3> istg_dim2 = 0;

	ap_uint<3> rowindex_fi0 = 0;
	Input_Layer1_Loop:
	for (ap_uint<16> ddr_pntr_fi0 = 0;ddr_pntr_fi0 < layer1_loop_cnt_fg0; ddr_pntr_fi0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=4800 max=4800
#pragma HLS pipeline

		inputtype read_64bit_fi0 = gmem_input_layer1_fa0[ddr_pntr_fi0];

		if(conv_desc.batch1_int8_en==0)
		{
			ap_uint<16> istg_dim3 = counter_row8_x_iw;
			Inputtype_Loop:
			for (ap_uint<8> planes_fi0 = 0, bit_fi0 = 0;planes_fi0 < 4; planes_fi0++, bit_fi0 += 16)
			{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL
				ap_int<16> input_2xint8_fi0 = read_64bit_fi0.range(bit_fi0 + 15,bit_fi0);

				//********************************************
				//******* Mean Sub Q8.0 formate **************
				//********************************************
				ap_int<8> input_img_0 = (ap_int<8> ) input_2xint8_fi0.range(7, 0)- mean_sub_fi0[planes_fi0];
				ap_int<8> input_img_1 = (ap_int<8> ) input_2xint8_fi0.range(15, 8)- mean_sub_fi0[planes_fi0];

				ap_int<16> istg_2xint8_fi0;
				istg_2xint8_fi0.range(7, 0) = input_img_0;
				istg_2xint8_fi0.range(15, 8) = input_img_1;


				istaging_buff0_fb0[planes_fi0][istg_dim2][istg_dim3] = (short) istg_2xint8_fi0;
			}            //Inputtype_Loop

			counter_row8_x_iw++;
			if (counter_row8_x_iw == input_desc.row8_x_iw)
			{
				counter_row8_x_iw = 0;
				istg_dim2++;
			}
		}
		else
		{
			ap_uint<32> read_32bit_e0 = read_64bit_fi0.range(31,0);
			ap_uint<32> read_32bit_e1 = read_64bit_fi0.range(63,32);

			ap_uint<16> istg_dim3_e0 = counter_row8_x_iw;
			ap_uint<16> istg_dim3_e1 = counter_row8_x_iw+1;

			Inputtype_Loop1:
			for (ap_uint<8> planes_fi0 = 0, bit_fi0 = 0; planes_fi0 < 4; planes_fi0++, bit_fi0 += 8)
			{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL
				ap_int<8> input_1xint8_fi0 = read_32bit_e0.range(bit_fi0 + 7, bit_fi0);

				//********************************************
				//******* Mean Sub Qx.x formate **************
				//********************************************
				ap_int<8> input_img_0 = input_1xint8_fi0    - mean_sub_fi0[planes_fi0];
				istaging_buff0_fb0[planes_fi0][istg_dim2][istg_dim3_e0] =    (short) input_img_0;
			}            //Inputtype_Loop1

			Inputtype_Loop2:
			for (ap_uint<8> planes_fi0 = 0, bit_fi0 = 0; planes_fi0 < 4; planes_fi0++, bit_fi0 += 8)
			{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL
				ap_int<8> input_1xint8_fi0 = read_32bit_e1.range(bit_fi0 + 7, bit_fi0);

				//********************************************
				//******* Mean Sub Qx.x formate **************
				//********************************************
				ap_int<8> input_img_0 = input_1xint8_fi0    - mean_sub_fi0[planes_fi0];
				istaging_buff0_fb0[planes_fi0][istg_dim2][istg_dim3_e1] =    (short) input_img_0;
			}            //Inputtype_Loop2

			counter_row8_x_iw+=2;
			if (counter_row8_x_iw == input_desc.row8_x_iw)
			{
				counter_row8_x_iw = 0;
				istg_dim2++;
			}
		}//batch 1 or 2
	}            //Input_Layer1_Loop
	//#endif

}

void InputReadLayer1_fi(ap_uint<16> layerx_loop_cnt_fg0,
		input_struct input_desc,
		conv_struct conv_desc,
		gmem_inputtype_layer1 *gmem_input_layer1_fa0,
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH])
{
#pragma HLS INLINE OFF

	ap_uint<3> rowindex_fi0 = 0;

	ap_uint<16> counter_row8_x_iw = 0;
	ap_uint<3> istg_dim2 = 0;

	Input_Layer1_Loop:
	for (ap_uint<16> ddr_pntr_fi0 = 0;ddr_pntr_fi0 < layerx_loop_cnt_fg0; ddr_pntr_fi0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=4800 max=4800
#pragma HLS pipeline

		inputtype read_64bit_fi0 = gmem_input_layer1_fa0[ddr_pntr_fi0];

		if(conv_desc.batch1_int8_en==0)
		{

			ap_uint<16> istg_dim3 = counter_row8_x_iw;
			istaging_buff0_fb0[istg_dim2][istg_dim3] = read_64bit_fi0;

			counter_row8_x_iw++;
			if (counter_row8_x_iw == input_desc.row8_x_iw)
			{
				counter_row8_x_iw = 0;
				istg_dim2++;
			}
		}
		else
		{
			ap_uint<32> read_32bit_e0 = read_64bit_fi0.range(31,0);
			ap_uint<32> read_32bit_e1 = read_64bit_fi0.range(63,32);

			ap_uint<16> istg_dim3_e0 = counter_row8_x_iw;
			ap_uint<16> istg_dim3_e1 = counter_row8_x_iw+1;
			ap_uint<64> write_to_istg1, write_to_istg2;
			Inputtype_Loop1:
			for (ap_uint<8> planes_fi0 = 0, bit_fi0 = 0,bit16=0; planes_fi0 < 4; planes_fi0++, bit_fi0 += 8, bit16 += 16)
			{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL
				ap_int<8> input_1xint8_fi0 = read_32bit_e0.range(bit_fi0 + 7, bit_fi0);

				//********************************************
				//******* Mean Sub Qx.x formate **************
				//********************************************
				ap_int<8> input_img_0 = input_1xint8_fi0;//    - mean_sub_fi0[planes_fi0];
				write_to_istg1.range(bit16 + 15, bit16) = (short) input_img_0;
			}            //Inputtype_Loop1
			istaging_buff0_fb0[istg_dim2][istg_dim3_e0] = write_to_istg1;

			Inputtype_Loop2:
			for (ap_uint<8> planes_fi0 = 0, bit_fi0 = 0,bit16=0; planes_fi0 < 4; planes_fi0++, bit_fi0 += 8, bit16 += 16)
			{
#pragma HLS LOOP_TRIPCOUNT min=4 max=4
#pragma HLS UNROLL
				ap_int<8> input_1xint8_fi0 = read_32bit_e1.range(bit_fi0 + 7, bit_fi0);

				//********************************************
				//******* Mean Sub Qx.x formate **************
				//********************************************
				ap_int<8> input_img_0 = input_1xint8_fi0;
				write_to_istg2.range(bit16 + 15, bit16) =    (short) input_img_0;
			}            //Inputtype_Loop2
			istaging_buff0_fb0[istg_dim2][istg_dim3_e1] =write_to_istg2;

			counter_row8_x_iw+=2;
			if (counter_row8_x_iw == input_desc.row8_x_iw)
			{
				counter_row8_x_iw = 0;
				istg_dim2++;
			}
		}//batch1 or 2

	}    //Input_Layer1_Loop

}


void reg_16bit(ap_uint<16> in_reg, ap_uint<16> *out)
{
#pragma HLS interface register port=in_reg
#pragma HLS INLINE OFF
	*out = in_reg;
}

void InputReadLayerOther_fh(conv_struct conv_desc,
		ap_uint<16> layerx_loop_cnt_fg0,
		input_struct input_desc,
		bool plane32_bool_fg0,
		gmem_inputtype_layerx *gmem_input_layer_other1_fa0,
#if !XI_SINGLE_IO_PORT_EN
		gmem_inputtype_layerx *gmem_input_layer_other2_fa0,
#endif
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<16> startrow_fg0,
		ap_uint<16> endrow_fg0,
		ap_uint<16> row_stage_offset_A_fg0,
		ap_uint<2> plane_mod2)
{
#pragma HLS INLINE OFF

	ap_uint<16> counter_iw_x_istgrow_t = 0;
	ap_uint<16> counter_plane16_t      = 0;
	ap_uint<16> offset_all_featureMap  = 0;
	bool        eff_plane32_bool_fh0_t = plane32_bool_fg0;
	ap_uint<2> eff_plane_mod2			= plane_mod2;

	Input_Layerx_Loop:
#if XI_IO_64bit_PORT_EN==0
	for (ap_uint<16> ddr_pntr_fh0 = 0; ddr_pntr_fh0 < layerx_loop_cnt_fg0; ddr_pntr_fh0++)
#else
		for (ap_uint<16> ddr_pntr_fh0 = 0; ddr_pntr_fh0 < layerx_loop_cnt_fg0*2; ddr_pntr_fh0++)
#endif
		{
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 intra false
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 inter false
#pragma HLS LOOP_TRIPCOUNT min=51*479 max=51*479

			inputtype2 read1_port_fh0, read2_port_fh0;
			read1_port_fh0 = gmem_input_layer_other1_fa0[ddr_pntr_fh0];
#if !XI_SINGLE_IO_PORT_EN
			read2_port_fh0 = gmem_input_layer_other2_fa0[ddr_pntr_fh0];
#endif

			ap_uint<16> istg_addr_fh0 = row_stage_offset_A_fg0 + counter_iw_x_istgrow_t + offset_all_featureMap  + conv_desc.conv3d_inp_offset;

			bool eff_plane32_bool_fh0 = eff_plane32_bool_fh0_t;

#if !XI_SINGLE_IO_PORT_EN
			if(conv_desc.batch1_int8_en==0)
			{
#if XI_IO_64bit_PORT_EN==0
				istaging_buff0_fb0[eff_plane32_bool_fh0*4+0][istg_addr_fh0] = read1_port_fh0.range(63,0);
				istaging_buff0_fb0[eff_plane32_bool_fh0*4+2][istg_addr_fh0] = read2_port_fh0.range(63, 0);
				istaging_buff0_fb0[eff_plane32_bool_fh0*4+1][istg_addr_fh0] = read1_port_fh0.range(127, 64);
				istaging_buff0_fb0[eff_plane32_bool_fh0*4+3][istg_addr_fh0] = read2_port_fh0.range(127, 64);
#else
				if(ddr_pntr_fh0[0]==0)
				{
					istaging_buff0_fb0[eff_plane32_bool_fh0*4+0][istg_addr_fh0] = read1_port_fh0.range(63,0);
					istaging_buff0_fb0[eff_plane32_bool_fh0*4+2][istg_addr_fh0] = read2_port_fh0.range(63,0);
				}
				else
				{
					istaging_buff0_fb0[eff_plane32_bool_fh0*4+1][istg_addr_fh0] = read1_port_fh0.range(63,0);
					istaging_buff0_fb0[eff_plane32_bool_fh0*4+3][istg_addr_fh0] = read2_port_fh0.range(63,0);
				}
#endif
			}
			else//BATCh1 int8 enable
			{


				ap_uint<64> write_to_istg1[4], write_to_istg2[4];
#if !XI_IO_64bit_PORT_EN
				for (ap_uint<10> dim1 = 0, bit = 0; dim1 < 4; dim1++, bit+=32)
#else
					for (ap_uint<10> dim1 = 0, bit = 0; dim1 < 2; dim1++, bit+=32)
#endif//XI_IO_64bit_PORT_EN
					{
#pragma HLS UNROLL
						ap_uint<32> word32bit = read1_port_fh0.range(bit + 31, bit);

						for(ap_uint<10> bit1=0, bit2=0; bit1 < 32; bit1+=8, bit2+=16)
						{
#pragma HLS UNROLL
							write_to_istg1[dim1].range(7+bit2,bit2)    = word32bit.range(bit1+ 7,bit1);
							write_to_istg1[dim1].range(15+bit2,8+bit2) = 0;
						}

					}
#if !XI_IO_64bit_PORT_EN
				istaging_buff0_fb0[0][istg_addr_fh0] = write_to_istg1[0];
				istaging_buff0_fb0[1][istg_addr_fh0] = write_to_istg1[1];
				istaging_buff0_fb0[2][istg_addr_fh0] = write_to_istg1[2];
				istaging_buff0_fb0[3][istg_addr_fh0] = write_to_istg1[3];
#else
				if(ddr_pntr_fh0[0]==0)
				{
					istaging_buff0_fb0[0][istg_addr_fh0] = write_to_istg1[0];
					istaging_buff0_fb0[1][istg_addr_fh0] = write_to_istg1[1];
				}
				if(ddr_pntr_fh0[0]==1)
				{
					istaging_buff0_fb0[2][istg_addr_fh0] = write_to_istg1[0];
					istaging_buff0_fb0[3][istg_addr_fh0] = write_to_istg1[1];
				}
#endif//XI_IO_64bit_PORT_EN

#if !XI_IO_64bit_PORT_EN
				for (ap_uint<10> dim1 = 0, bit = 0; dim1 < 4; dim1++, bit+=32)
#else
					for (ap_uint<10> dim1 = 0, bit = 0; dim1 < 2; dim1++, bit+=32)
#endif//XI_IO_64bit_PORT_EN
					{
#pragma HLS UNROLL
						ap_uint<32> word32bit = read2_port_fh0.range(bit + 31, bit);

						for(ap_uint<10> bit1=0, bit2=0; bit1 < 32; bit1+=8, bit2+=16)
						{
#pragma HLS UNROLL
							write_to_istg2[dim1].range(7+bit2,bit2)    = word32bit.range(bit1+ 7,bit1);
							write_to_istg2[dim1].range(15+bit2,8+bit2) = 0;
						}

					}

#if !XI_IO_64bit_PORT_EN
				istaging_buff0_fb0[4][istg_addr_fh0] = write_to_istg2[0];
				istaging_buff0_fb0[5][istg_addr_fh0] = write_to_istg2[1];
				istaging_buff0_fb0[6][istg_addr_fh0] = write_to_istg2[2];
				istaging_buff0_fb0[7][istg_addr_fh0] = write_to_istg2[3];

#else
				if(ddr_pntr_fh0[0]==0)
				{
					istaging_buff0_fb0[4][istg_addr_fh0] = write_to_istg1[0];
					istaging_buff0_fb0[5][istg_addr_fh0] = write_to_istg1[1];
				}
				if(ddr_pntr_fh0[0]==1)
				{
					istaging_buff0_fb0[6][istg_addr_fh0] = write_to_istg1[0];
					istaging_buff0_fb0[7][istg_addr_fh0] = write_to_istg1[1];
				}
#endif//XI_IO_64bit_PORT_EN

			}///batch 1 or 2
#else
			if(eff_plane_mod2==0)
			{
#if !XI_IO_64bit_PORT_EN
				istaging_buff0_fb0[0][istg_addr_fh0] = read1_port_fh0.range(63,0);
				istaging_buff0_fb0[1][istg_addr_fh0] = read1_port_fh0.range(127, 64);
#else
				if(ddr_pntr_fh0[0]==0)
				{
					istaging_buff0_fb0[0][istg_addr_fh0] = read1_port_fh0.range(63,0);
				}
				if(ddr_pntr_fh0[0]==1)
				{
					istaging_buff0_fb0[1][istg_addr_fh0] = read1_port_fh0.range(63, 0);
				}
#endif
			}
			if(eff_plane_mod2==1)
			{
#if !XI_IO_64bit_PORT_EN
				istaging_buff0_fb0[2][istg_addr_fh0] = read1_port_fh0.range(63,0);
				istaging_buff0_fb0[3][istg_addr_fh0] = read1_port_fh0.range(127, 64);
#else
				if(ddr_pntr_fh0[0]==0)
				{
					istaging_buff0_fb0[2][istg_addr_fh0] = read1_port_fh0.range(63,0);
				}
				if(ddr_pntr_fh0[0]==1)
				{
					istaging_buff0_fb0[3][istg_addr_fh0] = read1_port_fh0.range(63, 0);
				}
#endif
			}
			if(eff_plane_mod2==2)
			{
#if !XI_IO_64bit_PORT_EN
				istaging_buff0_fb0[4][istg_addr_fh0] = read1_port_fh0.range(63,0);
				istaging_buff0_fb0[5][istg_addr_fh0] = read1_port_fh0.range(127, 64);
#else
				if(ddr_pntr_fh0[0]==0)
				{
					istaging_buff0_fb0[4][istg_addr_fh0] = read1_port_fh0.range(63,0);
				}
				if(ddr_pntr_fh0[0]==1)
				{
					istaging_buff0_fb0[5][istg_addr_fh0] = read1_port_fh0.range(63, 0);
				}
#endif
			}
			if(eff_plane_mod2==3)
			{
#if !XI_IO_64bit_PORT_EN
				istaging_buff0_fb0[6][istg_addr_fh0] = read1_port_fh0.range(63,0);
				istaging_buff0_fb0[7][istg_addr_fh0] = read1_port_fh0.range(127, 64);
#else
				if(ddr_pntr_fh0[0]==0)
				{
					istaging_buff0_fb0[6][istg_addr_fh0] = read1_port_fh0.range(63,0);
				}
				if(ddr_pntr_fh0[0]==1)
				{
					istaging_buff0_fb0[7][istg_addr_fh0] = read1_port_fh0.range(63, 0);
				}
#endif
			}
#endif//port1

#if XI_IO_64bit_PORT_EN==0
			counter_iw_x_istgrow_t++;
#else
			if(ddr_pntr_fh0[0] == 1)
				counter_iw_x_istgrow_t++;
#endif

			if (counter_iw_x_istgrow_t == conv_desc.iw_mul_istg_row_count)
			{
				counter_iw_x_istgrow_t = 0;
				counter_plane16_t++;
#if !XI_SINGLE_IO_PORT_EN
				if(conv_desc.batch1_int8_en==0)
				{
					if (eff_plane32_bool_fh0_t == 1)
						offset_all_featureMap += conv_desc.iw_mul_istg_row_count;
					eff_plane32_bool_fh0_t = !eff_plane32_bool_fh0_t;
				}
				else
				{
					offset_all_featureMap += conv_desc.iw_mul_istg_row_count;
				}
#else

				if(eff_plane_mod2==3)
					offset_all_featureMap += conv_desc.iw_mul_istg_row_count;
				eff_plane_mod2++;
#endif

			}

		}//Input_Layerx_Loop2

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

	for (ap_uint<16> ddr_ptr = 0; ddr_ptr < loop_cnt; ddr_ptr++)
	{
#pragma HLS PIPELINE
		ap_uint<64> word_64bit = gmem_mean_fa0[ddr_ptr];

#if 1
		short val_1 = word_64bit.range(15, 0);
		short val_2 = word_64bit.range(31, 16);
		short val_3 = word_64bit.range(47, 32);
		short val_4 = word_64bit.range(63, 48);

#endif
		ap_uint<10> bram_addr = ddr_ptr * 2;
		if (cnt == 0)
		{
			mean_buff[0][bram_addr] = val_1;
			mean_buff[1][bram_addr] = val_2;
			mean_buff[0][bram_addr + 1] = val_3;
			mean_buff[1][bram_addr + 1] = val_4;
		}
		else if (cnt == 1)
		{
			variance_buff[0][bram_addr] = val_1;
			variance_buff[1][bram_addr] = val_2;
			variance_buff[0][bram_addr + 1] = val_3;
			variance_buff[1][bram_addr + 1] = val_4;
		}

		else if (cnt == 2)
		{
			beta_buff[0][bram_addr] = val_1;
			beta_buff[1][bram_addr] = val_2;
			beta_buff[0][bram_addr + 1] = val_3;
			beta_buff[1][bram_addr + 1] = val_4;

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

	int offset = 0;
	ap_uint<16> loop_cnt = input_desc.planes / 4;
	for (ap_uint<4> cnt = 0; cnt < 3; cnt++)
	{
		ReadMean(input_desc, gmem_mean_fa0 + offset, mean_buff, variance_buff,beta_buff, loop_cnt, cnt);

		offset = offset + input_desc.planes / 4;
	}

}

void WriteNormData(conv_struct conv_desc, ap_uint<16> layerx_loop_cnt_fg0,
		input_struct input_desc,
		ap_uint<3> plane32,
		gmem_inputtype_layer1 *gmem_istg_out1_fa0,
#if !XI_SINGLE_IO_PORT_EN
		gmem_inputtype_layer1 *gmem_istg_out2_fa0,
#endif
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<12> startrow_fg0, ap_uint<12> endrow_fg0,
		ap_uint<16> input_plane_fg0) {
#pragma HLS INLINE OFF

	ap_uint<16> stg_row_cnt;


	stg_row_cnt = conv_desc.conv3d_op_rows;

	ap_uint<16> width;
	width = conv_desc.conv3d_op_w;

	Input_Layerx_Loop: for (ap_uint<16> ddr_pntr_fh0 = 0;
			ddr_pntr_fh0 < layerx_loop_cnt_fg0; ddr_pntr_fh0++) {
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 intra false
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 inter false
#pragma HLS LOOP_TRIPCOUNT min=51*479 max=51*479

#if !XI_SINGLE_IO_PORT_EN
		ap_uint<128> word_128bit;
		ap_uint<4> dim2 = ddr_pntr_fh0[0] + 4 * plane32;
		ap_uint<16> dim3 = (ddr_pntr_fh0 / 2)
																														+ (input_plane_fg0.range(15, 5) * stg_row_cnt * width);
		ap_uint<16> dim3_reg;
		reg_addr_istg_wrt_nrm(&dim3, &dim3_reg);

		gmem_istg_out1_fa0[ddr_pntr_fh0] = istaging_buff0_fb0[dim2 + 0][dim3];
		gmem_istg_out2_fa0[ddr_pntr_fh0] = istaging_buff0_fb0[dim2 + 2][dim3];
#else
		ap_uint<64> word_64bit;
		ap_uint<3> dim2 = ddr_pntr_fh0[0] + 2 * input_plane_fg0.range(15,3);
		ap_uint<16> dim3 = (ddr_pntr_fh0 / 2) + (input_plane_fg0.range(15, 5) * stg_row_cnt * width);
		ap_uint<16> dim3_reg;
		reg_addr_istg_wrt_nrm(&dim3 , &dim3_reg);

		gmem_istg_out1_fa0[ddr_pntr_fh0] = istaging_buff0_fb0[dim2][dim3];

#endif
	}

}

void saturation_fun(ap_int<32> in24bit, ap_int<8> in8bit, ap_int<8> *out8bit, ap_uint<32> overflow_pattern, bool int6_en)
{
#pragma HLS inline

	ap_int<8> in8bit_sat;
	if (in24bit[23] == 0)        //positive accumulator
	{
		bool and_out = in24bit & overflow_pattern;
		if (and_out == 0)
			in8bit_sat = in8bit;
		else
		{
			if (int6_en == 0)
				in8bit_sat = 127;
			else
				in8bit_sat = 31;
		}
	}
	else        // negative accumulator
	{
		bool and_out = ~in24bit & overflow_pattern;
		if (and_out == 0)
			in8bit_sat = in8bit;
		else
		{
			if (int6_en == 0)
				in8bit_sat = -128;
			else
				in8bit_sat = -32;
		}
	}
	*out8bit = in8bit_sat;
}

void ReadNormData(conv_struct conv_desc,
		ap_uint<16> layerx_loop_cnt_fg0,
		input_struct input_desc,
		bool plane32,
		gmem_inputtype_layer1 *gmem_input_layer1_fa0,
		gmem_inputtype_layer1 *gmem_inp_norm_2_fa0,
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<12> startrow_fg0,
		ap_uint<12> endrow_fg0,
		ap_uint<16> input_plane_fg0,
		short mean[2][8],
		short variance[2][8],
		short beta[2][8],
		bool norm_flags[12][13],
		bool prec_2,
		ap_uint<4> flagset)
{
#pragma HLS INLINE OFF
	bool ddr_read_en = norm_flags[flagset][0];
	bool beta_sel = norm_flags[flagset][1];
	bool mean_sel = norm_flags[flagset][2];
	bool variance_sel = norm_flags[flagset][3];
	bool relu_en = norm_flags[flagset][4];
	bool elem_wise_mul_sel = norm_flags[flagset][5];
	bool elem_wise_add_sel = norm_flags[flagset][6];

	bool mul_in_sel = norm_flags[flagset][9];


	short val_reg[2][4];
#pragma HLS ARRAY_PARTITION variable=val_reg complete dim=0


	ap_uint<16> stg_row_cnt = conv_desc.istg_row_count;
	ap_uint<16> width = input_desc.width;

	Input_Layerx_Loop:
	for (ap_uint<16> ddr_pntr_fh0 = 0;ddr_pntr_fh0 < layerx_loop_cnt_fg0; ddr_pntr_fh0++)
	{

#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 intra false
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 inter false
#pragma HLS LOOP_TRIPCOUNT min=51*479 max=51*479


		ap_uint<128> word_128bit, word_128bit_istg, word_128bit_ddr;
		ap_uint<64> word1_64bit_ddr = 0, word2_64bit_ddr = 0;

		ap_int<32> shift_res, word_relu;
		ap_int<8> shift_res_h, shift_res_l;
		ap_int<16> add_sub_op2_l, add_sub_op2_h, add_sub_res_l, add_sub_res_h,
		mul_op_l, mul_op_h, word_relu_h, word_relu_l;
		ap_int<16> add_sub_op3;
		ap_int<32> add_sub_op3_h, add_sub_op3_l;

		ap_int<32> mul_add_res;
		ap_int<32> mul_res_l, mul_res_h, mul_add_res_l, mul_add_res_h;
		ap_uint<4> dim2 = ddr_pntr_fh0[0] + 4 * plane32;
		ap_uint<4> dim2_plus_2 = dim2 + 2;
		ap_uint<16> dim3 = (ddr_pntr_fh0 / 2) + (input_plane_fg0.range(15, 5) * stg_row_cnt * width);
		ap_uint<16> dim3_reg;		
		ap_int<16> res_pwr, pwr_out;

		ap_uint<16> mul_in = conv_desc.mul_in;
		ap_uint<5> norm_prec;
		if (!prec_2)
			norm_prec = conv_desc.norm_prec;
		else
			norm_prec = conv_desc.norm_prec_2;

		ap_uint<5> norm_prec_3 = conv_desc.norm_prec_3;


		word1_64bit_ddr = gmem_input_layer1_fa0[ddr_pntr_fh0];
		word2_64bit_ddr = gmem_inp_norm_2_fa0[ddr_pntr_fh0];

		word_128bit_ddr.range(63, 0) = word1_64bit_ddr;
		word_128bit_ddr.range(127, 64) = word2_64bit_ddr;


		reg_addr_istg_rd_nrm(&dim3, &dim3_reg);

		ap_uint<64> istg_rd1, istg_rd2;
		for (ap_uint<4> t_dim2 = 0; t_dim2 < 8; t_dim2++)
		{
#pragma HLS UNROLL
			if (t_dim2 == dim2)
				istg_rd1 = istaging_buff0_fb0[t_dim2][dim3_reg];
			else if (t_dim2 == dim2_plus_2)
				istg_rd2 = istaging_buff0_fb0[t_dim2][dim3_reg];

		}

		word_128bit_istg.range(63,0)   = istg_rd1;
		word_128bit_istg.range(127,64) = istg_rd2;

		if (ddr_read_en)
			word_128bit = word_128bit_ddr;
		else
			word_128bit = word_128bit_istg;


		ap_uint<3> depth_off = ddr_pntr_fh0[0] * 2;
		for (ap_uint<7> planes = 0, bit = 0; planes < 8; planes++, bit += 16)
		{
#pragma HLS UNROLL
			ap_uint<4> istg_dim2 = dim2 + 2 * planes[2];
			ap_int<16> word = word_128bit.range(bit + 15, bit);

			ap_int<16> word_istg = word_128bit_istg.range(bit + 15, bit);

			ap_int<16> word_l = word.range(7, 0) * (1 << 8);
			ap_int<16> word_h = word.range(15, 8) * (1 << 8);


			if (mean_sel)
			{
				ap_int<16> mean_val = -1 * mean[planes[0]][depth_off + (planes / 2) + (2 * planes[2])];
				add_sub_op2_l = mean_val;
				add_sub_op2_h = mean_val;

			}
			else if (elem_wise_add_sel)
			{
				add_sub_op2_l = word_istg.range(7, 0) * (1 << 8);
				add_sub_op2_h = word_istg.range(15, 8) * (1 << 8);

			}
			else
			{
				add_sub_op2_l = 0;
				add_sub_op2_h = 0;
			}



			if (variance_sel)
			{
				ap_int<16> mul_op = variance[planes[0]][depth_off + (planes / 2)
														+ (2 * planes[2])];
				mul_op_l = mul_op;
				mul_op_h = mul_op;

			}
			else if (elem_wise_mul_sel)
			{
				mul_op_l = word_istg.range(7, 0) * (1 << 8);
				mul_op_h = word_istg.range(15, 8) * (1 << 8);
			}
			else if (mul_in_sel)
			{
				mul_op_l = mul_in.range(7, 0) * (1 << 8);
				mul_op_h = mul_in.range(15, 8) * (1 << 8);
			}
			else
			{
				mul_op_l = 1;
				mul_op_h = 1;
			}


			if (beta_sel)
			{
				add_sub_op3 = (beta[planes[0]][depth_off + (planes / 2) + (2 * planes[2])]);
				add_sub_op3_l = (add_sub_op3 * (1 << norm_prec_3));
				add_sub_op3_h = (add_sub_op3 * (1 << norm_prec_3));

			}
			else
			{
				add_sub_op3_l = 0;
				add_sub_op3_h = 0;

			}

#pragma HLS resource variable = mul_add_res_l core =DSP48
#pragma HLS resource variable = mul_add_res_h core =DSP48
			mul_add_res_h = ((word_h + add_sub_op2_h) * mul_op_h) + add_sub_op3_h;
			mul_add_res_l = ((word_l + add_sub_op2_l) * mul_op_l) + add_sub_op3_l;

			ap_int<16> in16bit_h = mul_add_res_h >> norm_prec;
			ap_int<16> in16bit_l = mul_add_res_l >> norm_prec;
			ap_int<8> cmp_h,cmp_l;

			if(conv_desc.int6_en_out)
			{
				cmp_h = 31;
				cmp_l = -32;
			}
			else{
				cmp_h = 127;
				cmp_l = -128;
			}

			if (in16bit_h > cmp_h)
			{
				shift_res_h = cmp_h;
			}
			else if (in16bit_h < cmp_l)
			{
				shift_res_h = cmp_l;
			}
			else
			{
				shift_res_h = in16bit_h;
			}

			if (in16bit_l > cmp_h)
			{
				shift_res_l = cmp_h;
			}
			else if (in16bit_l < cmp_l)
			{
				shift_res_l = cmp_l;
			}
			else
			{
				shift_res_l = in16bit_l;
			}


			if (relu_en && shift_res_h[7] == 1)
				word_relu_h = 0;
			else
				word_relu_h = shift_res_h;

			if (relu_en && shift_res_l[7] == 1)
				word_relu_l = 0;
			else
				word_relu_l = shift_res_l;

			word_relu.range(7, 0) = word_relu_l;
			word_relu.range(15, 8) = word_relu_h;
			val_reg[planes[2]][planes.range(1, 0)] = word_relu;
		}
		ap_uint<64> val_reg_packed[2];
#pragma HLS ARRAY_PARTITION variable=val_reg_packed complete dim=1

		for (ap_uint<8> t_dim1 = 0, bit=0; t_dim1 < 4; t_dim1++, bit+=16)
		{
#pragma HLS UNROLL
			val_reg_packed[0].range(bit+15, bit) = val_reg[0][t_dim1];
			val_reg_packed[1].range(bit+15, bit) = val_reg[1][t_dim1];
		}

		for (ap_uint<4> t_dim2 = 0; t_dim2 < 8; t_dim2++)
		{
#pragma HLS UNROLL
			if (t_dim2 == dim2)
				istaging_buff0_fb0[t_dim2][dim3] = val_reg_packed[0];
			else if (t_dim2 == dim2_plus_2)
				istaging_buff0_fb0[t_dim2][dim3] = val_reg_packed[1];

		}

	}

}

void Normalization(conv_struct conv_desc, ap_uint<16> layerx_loop_cnt_fg0,
		input_struct input_desc, gmem_inputtype_layer1 *gmem_input_layer1_fa0,
		gmem_inputtype_layer1 *gmem_inp_norm_2_fa0,
		gmem_biastype *gmem_bias_fa0,
		gmem_inputtype_layer1 *gmem_inp_norm_3_fa0,
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<12> startrow_fg0, ap_uint<12> endrow_fg0,
		short mean_buff[2][1024], short variance_buff[2][1024],
		short beta_buff[2][1024], ap_uint<32> start_off_A_fg0,
		bool norm_flags[12][13], ap_uint<4> flagset)
{
#pragma HLS INLINE OFF

	bool read_2nd_ptr = norm_flags[flagset][11];


	bool prec_2 = read_2nd_ptr;
	ap_uint<32> start_off;
	for (ap_uint<16> imgG = 0, bram_addr = 0; imgG < input_desc.planes; imgG += 16, bram_addr += 8)
	{
#pragma HLS LOOP_TRIPCOUNT min=0 max=0

		//read mean and variance corresponding to 16 planes
		short mean[2][8], variance[2][8], beta[2][8];
#pragma HLS ARRAY_PARTITION variable=mean complete dim=0
#pragma HLS ARRAY_PARTITION variable=variance complete dim=0
#pragma HLS ARRAY_PARTITION variable=beta complete dim=0

		for (ap_uint<3> dim1 = 0; dim1 < 2; dim1++)
		{
#pragma HLS UNROLL
			for (ap_uint<5> cnt = 0; cnt < 8; cnt++)
			{
#pragma HLS UNROLL
				mean[dim1][cnt] = mean_buff[dim1][bram_addr + cnt];
				variance[dim1][cnt] = variance_buff[dim1][bram_addr + cnt];
				beta[dim1][cnt] = beta_buff[dim1][bram_addr + cnt];
			}

		}

		bool plane32 = imgG[4];

		ap_uint<16> imgGby16;
		imgGby16 = imgG.range(15, 4);

		ap_uint<32> offset_l1;

		offset_l1 = (imgGby16) * input_desc.size;


		start_off = start_off_A_fg0 * 2 + offset_l1 * 2;



		if (!read_2nd_ptr)
			ReadNormData(conv_desc, 2 * layerx_loop_cnt_fg0, input_desc,plane32, gmem_input_layer1_fa0 + start_off,gmem_inp_norm_2_fa0 + start_off, istaging_buff0_fb0,startrow_fg0, endrow_fg0, imgG, mean, variance, beta,norm_flags, prec_2, flagset);
		else
			ReadNormData(conv_desc, 2 * layerx_loop_cnt_fg0, input_desc,plane32, gmem_bias_fa0 + start_off,gmem_inp_norm_3_fa0 + start_off, istaging_buff0_fb0,startrow_fg0, endrow_fg0, imgG, mean, variance, beta,norm_flags, prec_2, flagset);


	}


}
void Conv3dWtsRead(ap_uint<32> wts_buf[4][1024],gmem_inputtype_layer1 *gmem_inp_norm_2_fa0,gmem_inputtype_layer1 *gmem_inp_norm_3_fa0,short loop_cnt)
{
#pragma HLS INLINE OFF
#pragma HLS ARRAY_PARTITION variable=wts_buf complete dim=1
#pragma HLS resource variable=wts_buf core=RAM_T2P_BRAM
	for(ap_uint<16> ddr_ptr=0;ddr_ptr<loop_cnt;ddr_ptr++)
	{
#pragma HLS PIPELINE
		ap_uint<128> word_128bit;
		word_128bit.range(63,0) = gmem_inp_norm_2_fa0[ddr_ptr];
		word_128bit.range(127,64) = gmem_inp_norm_3_fa0[ddr_ptr];

		ap_int<8> word_1 = word_128bit.range(7,0);
		ap_int<8> word_2 = word_128bit.range(15,8);
		ap_int<8> word_3 = word_128bit.range(23,16);
		ap_int<8> word_4 = word_128bit.range(31,24);
		ap_int<8> word_5 = word_128bit.range(39,32);
		ap_int<8> word_6 = word_128bit.range(47,40);
		ap_int<8> word_7 = word_128bit.range(55,48);
		ap_int<8> word_8 = word_128bit.range(63,56);

		for(int k=0,bit=0;k<4;k++,bit+=32)
		{
#pragma HLS UNROLL
			wts_buf[k][ddr_ptr.range(9,0)] = word_128bit.range(bit+31,bit);

		}


	}
}
void Conv3dBiasRead(ap_uint<32> bias_buf[8][512],ap_uint<32> scale_buf[8][512],gmem_inputtype_layer1 *gmem_istg_out_1_fa0,gmem_inputtype_layer1 *gmem_istg_out_2_fa0,short loop_cnt)
{
#pragma HLS INLINE OFF
#pragma HLS ARRAY_PARTITION variable=bias_buf complete dim=1
#pragma HLS resource variable=bias_buf core=RAM_T2P_BRAM
#pragma HLS ARRAY_PARTITION variable=scale_buf complete dim=1
#pragma HLS resource variable=scale_buf core=RAM_T2P_BRAM

	for(ap_uint<16> ddr_ptr=0;ddr_ptr < 2*loop_cnt;ddr_ptr++)
	{
#pragma HLS PIPELINE
		ap_uint<128> word_128bit;
		word_128bit.range(63,0) = gmem_istg_out_1_fa0[ddr_ptr];
		word_128bit.range(127,64) = gmem_istg_out_2_fa0[ddr_ptr];


		for(int k=0,bit=0;k<4;k++,bit+=32)
		{
#pragma HLS UNROLL
			if(ddr_ptr < loop_cnt)
				bias_buf[(4*ddr_ptr[0])+k][ddr_ptr.range(9,1)] = word_128bit.range(bit+31,bit);
			else
				scale_buf[(4*ddr_ptr[0])+k][ddr_ptr.range(9,1)- (loop_cnt/2)] = word_128bit.range(bit+31,bit);
		}


	}
}


void reg_addr_conv3d(ap_uint<14> *istg_addr, ap_uint<14> *istg_addr_reg)
{
#pragma HLS interface register port=istg_addr_reg
#pragma HLS inline off

	*istg_addr_reg = *istg_addr;


}
void reg_depth_conv3d(ap_uint<16> *istg_addr, ap_uint<16> *istg_addr_reg)
{
#pragma HLS interface register port=istg_addr_reg
#pragma HLS inline off

	*istg_addr_reg = *istg_addr;


}
#if XI_POOL_PROC_8_PLN
#define	LOOP_DIM2 2
#define WTS_DIM   2
#else
#define	LOOP_DIM2 8
#define WTS_DIM   4
#endif
void reg_bias_conv3d(ap_int<24> bias[4][LOOP_DIM2], ap_int<24> bias_reg[4][LOOP_DIM2])
{
#pragma HLS interface register port=bias_reg
#pragma HLS inline off

	for(int i=0;i<4;i++)
	{
		for(int j=0;j<8;j++)
		{
			bias_reg[i][j] = bias[i][j];
		}
	}



}

void add_24bit_conv3d(ap_int<24> val_1, ap_int<24> val_2, ap_int<24> *out)
{
#pragma HLS interface register port=out
#pragma HLS inline off

	*out = val_1 + val_2;
}
void reg16bit_conv3d(ap_int<16> temp1, ap_int<16> temp2,ap_int<16> temp3, ap_int<16> temp4,ap_int<16> *temp1_out, ap_int<16> *temp2_out,ap_int<16> *temp3_out, ap_int<16> *temp4_out)
{
#pragma HLS interface register port=temp1_out
#pragma HLS interface register port=temp2_out
#pragma HLS interface register port=temp3_out
#pragma HLS interface register port=temp4_out
#pragma HLS inline off

	*temp1_out = temp1;
	*temp2_out = temp2;
	*temp3_out = temp3;
	*temp4_out = temp4;
}

void Conv3dTop(conv_struct conv_desc,
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<16> conv3d_loop_bound, ap_int<12> conv3d_startrow_ip,
		ap_uint<32> bias_buf[8][512], ap_uint<32> scale_buf[8][512],
#if !XI_DISABLE_BN
		ap_uint<32> wts_buf[4][1024],
#endif
		ap_int<12> conv3d_startrow_op

) {
#pragma HLS INLINE OFF




	ap_uint<16> op_h = conv_desc.conv3d_op_h,op_w  = conv_desc.conv3d_op_w ,ip_h = conv_desc.conv3d_ip_h,ip_w = conv_desc.conv3d_ip_w;
	ap_uint<3> ps_h = conv_desc.conv3d_stride,ps_w= conv_desc.conv3d_stride;
	ap_uint<4> ft_h = conv_desc.conv3d_ft_h,ft_w= conv_desc.conv3d_ft_w;
	ap_uint<3> h_pad = conv_desc.conv3d_pad,v_pad= conv_desc.conv3d_pad;
	ap_uint<8> fsz2 = conv_desc.conv3d_fsz2;
	ap_uint<12> op_planes = conv_desc.conv3d_op_pln,ip_planes = conv_desc.conv3d_ip_pln;
	ap_uint<6> outshift = conv_desc.conv3d_outshift;
	ap_uint<10> op_rows = conv_desc.conv3d_op_rows;
	short loop_bound = conv3d_loop_bound;
	ap_uint<12> wts_off=0;
	ap_uint<10> stg_rw_cnt;
	bool conv_3d_en = conv_desc.conv3d_intg_en;
	bool avg_pool_en = conv_desc.avg_pool_en;
	ap_uint<8> avg_pool_mul = conv_desc.avg_pool_mul;


	stg_rw_cnt = conv_desc.conv3d_istg_row_cnt;

	char comp_cnt=0;
	unsigned short oid=0;
	ap_int<24> sum[4][LOOP_DIM2],sum_b1[4][LOOP_DIM2],bias[4][LOOP_DIM2],bias_reg[4][LOOP_DIM2];
#pragma HLS ARRAY_PARTITION variable=bias complete dim=0
#pragma HLS ARRAY_PARTITION variable=bias_reg complete dim=0
	ap_uint<16> scale[4][LOOP_DIM2];
	ap_int<8> inp[4][LOOP_DIM2],wts[4][LOOP_DIM2],inp_b1[4][LOOP_DIM2];
	ap_int<8> max[4][LOOP_DIM2],max_b1[4][LOOP_DIM2];
	ap_int<24> roundval = conv_desc.conv3d_rndval;


	ap_uint<32> wts_word1[4];
	ap_uint<32> wts_word2[4];

	//max array initialization


	for(int tt_dim1=0;tt_dim1<4;tt_dim1++)
	{
#pragma HLS UNROLL
		for(int t_dim2=0;t_dim2<LOOP_DIM2;t_dim2++)
		{
#pragma HLS UNROLL
			max[tt_dim1][t_dim2] = -128;
			max_b1[tt_dim1][t_dim2] = -128;
		}

	}


#if XI_POOL_PROC_8_PLN
	int bias_dim=0;bool inc_pln_dim=0;ap_uint<3> dim_pln=4;
	int istg_dim_inc=0;
	for(int pln=0;pln<ip_planes;pln+=8)
#else
		for(int pln=0,bias_dim=0;pln<ip_planes;pln+=32,bias_dim+=2)
#endif
		{


			short row=0,col=0;
			
			char w_row=0,w_col=0;
			wts_off = (pln/32)*2*fsz2;
			oid = (pln/32)*conv_desc.conv3d_op_rows*conv_desc.conv3d_op_w;
#if XI_POOL_PROC_8_PLN
			if(pln>0 && pln%32==0)
			{
				bias_dim++;
				istg_dim_inc = 0;
			}

			dim_pln[2] = !dim_pln[2];
#endif
			ap_int<16> temp_1,temp_2,temp_3,temp_4;
			ap_int<16> temp_1_out,temp_2_out,temp_3_out,temp_4_out;
			for(int dim1=0;dim1<LOOP_DIM2;dim1++)
			{
#pragma HLS UNROLL

#if XI_POOL_PROC_8_PLN
				ap_uint<32> bias_1 = bias_buf[dim1+dim_pln][bias_dim];
				ap_uint<32> bias_2 = bias_buf[dim1+dim_pln+2][bias_dim];
#else
				ap_uint<32> bias_1 = bias_buf[dim1][bias_dim];
				ap_uint<32> bias_2 = bias_buf[dim1][bias_dim+1];
#endif

				if(conv_3d_en)
				{
					temp_1 = bias_1.range(15,0);
					temp_2 = bias_1.range(31,16);
					temp_3 = bias_2.range(47,32);
					temp_4 = bias_2.range(63,48);

				}
				else{
					temp_1 = 0;
					temp_2 = 0;
					temp_3 = 0;
					temp_4 = 0;
				}
				reg16bit_conv3d(temp_1,temp_2,temp_3,temp_4,&temp_1_out,&temp_2_out,&temp_3_out,&temp_4_out);
				bias[0][dim1] = (ap_int<24>)(temp_1* (1<< outshift));
				bias[1][dim1] = (ap_int<24>)(temp_2* (1 << outshift));
				bias[2][dim1] = (ap_int<24>)(temp_3* (1 << outshift));
				bias[3][dim1] = (ap_int<24>)(temp_4* (1 << outshift));
			}
			reg_bias_conv3d(bias, bias_reg);
			ap_uint<16> sc_temp_1,sc_temp_2,sc_temp_3,sc_temp_4;
			for(int dim1=0;dim1<LOOP_DIM2;dim1++)
			{
#pragma HLS UNROLL

#if XI_POOL_PROC_8_PLN
				ap_uint<32> scale_1 = scale_buf[dim1+dim_pln][bias_dim];
				ap_uint<32> scale_2 = scale_buf[dim1+dim_pln+2][bias_dim];

#else
				ap_uint<32> scale_1 = scale_buf[dim1][bias_dim];
				ap_uint<32> scale_2 = scale_buf[dim1][bias_dim+1];
#endif
				if(conv_3d_en || conv_desc.avg_pool_en)
				{
					sc_temp_1 = scale_1.range(15,0);
					sc_temp_2 = scale_1.range(31,16);
					sc_temp_3 = scale_2.range(15,0);
					sc_temp_4 = scale_2.range(31,16);

				}
				else{
					sc_temp_1 = 1;
					sc_temp_2 = 1;
					sc_temp_3 = 1;
					sc_temp_4 = 1;
				}

				scale[0][dim1] = (sc_temp_1);
				scale[1][dim1] = (sc_temp_2);
				scale[2][dim1] = (sc_temp_3);
				scale[3][dim1] = (sc_temp_4);
			}



			for(int i=0;i<loop_bound;i++)
			{
#pragma HLS PIPELINE
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 intra false
#pragma HLS DEPENDENCE variable=istaging_buff0_fb0 inter false


				if(w_col == ft_w)
				{
					w_col=0;w_row++;
				}
				if(w_row == ft_h)
				{
					col += ps_w;
					w_row =0;
				}



				if((col+ft_w-1) >= conv_desc.pool_ew +2*h_pad)
				{
					row += ps_h;
					col = 0;
				}


				short col_id = w_col+col-h_pad;
				short row_id_istg = row+w_row;
				short row_id_pad = row+w_row+conv3d_startrow_ip;



				ap_uint<16> depth,depth_off,depth_reg;

				ap_uint<16> pln_offset;
#pragma HLS resource variable=pln_offset core=DSP48 latency=2
				pln_offset = (pln/32)*stg_rw_cnt*ip_w;


				depth_off = col_id + conv_desc.conv3d_inp_offset + pln_offset;

				if(conv3d_startrow_op == 0)
					depth_reg = (row_id_pad)*ip_w +  depth_off;
				else
					depth_reg = (row_id_istg)*ip_w + depth_off;

				reg_depth_conv3d(&depth_reg,&depth);


				int t_dim1=0;
				bool in_pad_reg;
				if((((row_id_pad)<0) || col_id<0 || col_id >= ip_w || (row_id_pad) >= ip_h))
					in_pad_reg = 1;
				else
					in_pad_reg = 0;
				int bit;




				for(t_dim1=0, bit=0;t_dim1<4;t_dim1++, bit+=16)
				{
#pragma HLS UNROLL
					for(int dim2=0;dim2<LOOP_DIM2;dim2++)
					{
#pragma HLS UNROLL
						if(in_pad_reg)
						{
							inp[t_dim1][dim2] = 0;
							inp_b1[t_dim1][dim2] = 0;

						}
						else{

#if XI_POOL_PROC_8_PLN
							ap_int<16> word= istaging_buff0_fb0[dim2+istg_dim_inc][depth].range(bit+15, bit);
#else
							ap_int<16> word= istaging_buff0_fb0[dim2][depth].range(bit+15, bit);
#endif
							inp[t_dim1][dim2] = word.range(7,0);
							inp_b1[t_dim1][dim2] = word.range(15,8);
						}
					}
				}

				int tt_dim1=0;
				for(tt_dim1=0;tt_dim1<WTS_DIM;tt_dim1++)
				{
#pragma HLS UNROLL
#if !XI_DISABLE_BN
					if(conv_3d_en)
					{


						wts_word1[tt_dim1] = wts_buf[tt_dim1][comp_cnt+wts_off];
						wts_word2[tt_dim1] = wts_buf[tt_dim1][comp_cnt+wts_off+fsz2];
						wts[0][tt_dim1] = wts_word1[tt_dim1].range(7,0);
						wts[1][tt_dim1] = wts_word1[tt_dim1].range(15,8);
						wts[2][tt_dim1] = wts_word1[tt_dim1].range(23,16);
						wts[3][tt_dim1] = wts_word1[tt_dim1].range(31,24);
#if !XI_POOL_PROC_8_PLN
						wts[0][tt_dim1+4] = wts_word2[tt_dim1].range(7,0);
						wts[1][tt_dim1+4] = wts_word2[tt_dim1].range(15,8);
						wts[2][tt_dim1+4] = wts_word2[tt_dim1].range(23,16);
						wts[3][tt_dim1+4] = wts_word2[tt_dim1].range(31,24);
#endif
					}

					else
#endif
					{
						wts_word1[tt_dim1] = avg_pool_mul;
						wts_word2[tt_dim1] = avg_pool_mul;
						wts[0][tt_dim1] = avg_pool_mul;
						wts[1][tt_dim1] = avg_pool_mul;
						wts[2][tt_dim1] = avg_pool_mul;
						wts[3][tt_dim1] = avg_pool_mul;
#if !XI_POOL_PROC_8_PLN
						wts[0][tt_dim1+4] = avg_pool_mul;
						wts[1][tt_dim1+4] = avg_pool_mul;
						wts[2][tt_dim1+4] = avg_pool_mul;
						wts[3][tt_dim1+4] = avg_pool_mul;
#endif
					}
				}

				for(int dim1=0;dim1<4;dim1++)
				{
#pragma HLS UNROLL
					for(int dim2=0;dim2<LOOP_DIM2;dim2++)
					{
#pragma HLS UNROLL
						if (comp_cnt==0) {
							sum[dim1][dim2] = inp[dim1][dim2]*wts[dim1][dim2];
							sum_b1[dim1][dim2] = inp_b1[dim1][dim2]*wts[dim1][dim2];
						} else {
							sum[dim1][dim2] += inp[dim1][dim2]*wts[dim1][dim2];
							sum_b1[dim1][dim2] += inp_b1[dim1][dim2]*wts[dim1][dim2];
						}

					}
				}

				for(int p_dim1=0;p_dim1<4;p_dim1++)
				{
#pragma HLS UNROLL
					for(int p_dim2=0;p_dim2<LOOP_DIM2;p_dim2++)
					{
#pragma HLS UNROLL
						max[p_dim1][p_dim2] = max[p_dim1][p_dim2] > inp[p_dim1][p_dim2] ? max[p_dim1][p_dim2] : inp[p_dim1][p_dim2];
						max_b1[p_dim1][p_dim2] = max_b1[p_dim1][p_dim2] > inp_b1[p_dim1][p_dim2] ? max_b1[p_dim1][p_dim2] : inp_b1[p_dim1][p_dim2];
					}

				}
				comp_cnt++;w_col++;
				if(comp_cnt == fsz2 )
				{

					comp_cnt = 0;
#if XI_POOL_PROC_8_PLN
					ap_uint<64> istg_word[2];
#endif
					for(int dim1=0, bitw=0;dim1<4;dim1++, bitw+=16)
					{
#pragma HLS UNROLL
						for(int dim2=0;dim2<LOOP_DIM2;dim2++)
						{
#pragma HLS UNROLL
							ap_int<24> temp =  (sum[dim1][dim2]*scale[dim1][dim2] + bias[dim1][dim2]);
							ap_int<24> temp_b1 =  (sum_b1[dim1][dim2]*scale[dim1][dim2] + bias[dim1][dim2]);
							ap_int<24> rnd_add_to_shift;
							ap_int<24> rnd_add_to_shift_b1,rnd_add_sum,rnd_add_sum_b1;
							ap_int<24> temp_abs,temp_abs_b1;
							temp_abs = abs_24bit(temp);
							temp_abs_b1 = abs_24bit(temp_b1);

							add_24bit_conv3d(temp_abs, roundval, &rnd_add_sum);
							add_24bit_conv3d(temp_abs_b1, roundval, &rnd_add_sum_b1);
							if(conv_3d_en)
							{
								rnd_add_to_shift =  rnd_add_sum;
								rnd_add_to_shift_b1 = rnd_add_sum_b1;
							}
							else{
								rnd_add_to_shift = temp;
								rnd_add_to_shift_b1 = temp_b1;
							}
							ap_int<24> rnd_add =  rnd_add_to_shift >> outshift;
							ap_int<24> rnd_add_b1 = rnd_add_to_shift_b1 >> outshift;

							ap_int<24> word;
							ap_int<24> word_b1;
							if(temp[23] == 0)
							{
								word  = rnd_add;

							}
							else
							{
								word = -rnd_add;

							}
							if(temp_b1[23] == 0)
							{
								word_b1  = rnd_add_b1;

							}
							else
							{
								word_b1 = -rnd_add_b1;

							}
							ap_int<8> word_sat,word_sat_b1;
							if(word > 127)
								word_sat = 127;
							else if(word < -128)
								word_sat = -128;
							else
								word_sat = (ap_int<8>)word.range(7,0);

							if(word_b1 > 127)
								word_sat_b1 = 127;
							else if(word_b1 < -128)
								word_sat_b1 = -128;
							else
								word_sat_b1 = (ap_int<8>)word_b1.range(7,0);

							ap_int<8> word_relu,word_relu_b1;
							ap_int<16> word_istg;

							if(conv_desc.conv3d_relu_en & word[23]==1)
								word_relu = 0;
							else
								word_relu = word_sat;
							if(conv_desc.conv3d_relu_en & word_b1[23]==1)
								word_relu_b1 = 0;
							else
								word_relu_b1 = word_sat_b1;
							if(conv_3d_en)
							{
								word_istg.range(7,0) = word_relu;
								word_istg.range(15,8) = word_relu_b1;
							}
							else if(avg_pool_en){
								word_istg.range(7,0) = rnd_add.range(7,0);
								word_istg.range(15,8) = rnd_add_b1.range(7,0);
							}
							else{
								word_istg.range(7,0) = max[dim1][dim2];
								word_istg.range(15,8) = max_b1[dim1][dim2];
							}

#if XI_POOL_PROC_8_PLN
							istg_word[dim2].range(bitw+15, bitw) = (short)word_istg;
#else
							istaging_buff0_fb0[dim2][oid].range(bitw+15, bitw) = (short)word_istg;
#endif
							//reset max values again
							max[dim1][dim2] = -128;
							max_b1[dim1][dim2] = -128;


						}
					}
#if XI_POOL_PROC_8_PLN
					istaging_buff0_fb0[istg_dim_inc][oid] = istg_word[0];
					istaging_buff0_fb0[istg_dim_inc+1][oid] = istg_word[1];
#endif
					oid++;

				}
			}
#if XI_POOL_PROC_8_PLN
			istg_dim_inc+=2;
#endif
		}
}


template<int IN_WW, int IN_HH, int IINPUT_PLANES>
void LoadIStagingBuff_fg(input_struct input_desc,
		conv_struct conv_desc,
		gmem_inputtype_layer1 *gmem_input_layer1_fa0,
		gmem_inputtype_layerx *gmem_input_layer_other1_fa0,
#if !XI_SINGLE_IO_PORT_EN
		gmem_inputtype_layerx *gmem_input_layer_other2_fa0,
#endif
		gmem_biastype *gmem_bias_fa0,
#if !XI_DISABLE_BN
		gmem_inputtype_layer1 *gmem_inp_norm_2_fa0,
		gmem_inputtype_layer1 *gmem_inp_norm_3_fa0,
#endif
		gmem_inputtype_layer1 *gmem_istg_out_1_fa0,
#if !XI_SINGLE_IO_PORT_EN
		gmem_inputtype_layer1 *gmem_istg_out_2_fa0,
#endif
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		ap_int<16> startrow_in_fb0,
		ap_int<16> endrow_in_fb0,
		ap_int<12> norm_startrow_in_fb0,
		ap_int<12> norm_endrow_in_fb0,
#if !XI_DISABLE_BN		
		short mean_buff[2][1024],
		short variance_buff[2][1024],
		short beta_buff[2][1024],
#endif		
		ap_uint<16> conv3d_loop_bound)
{
#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=1
#if XI_ISTG_URAM_EN==0
#pragma HLS resource variable=istaging_buff0_fb0 core=RAM_T2P_BRAM latency=2
#else
#pragma HLS RESOURCE variable=istaging_buff0_fb0 core=XPM_MEMORY uram
#endif
#pragma HLS INLINE OFF

#if !XI_DISABLE_BN
#pragma HLS ARRAY_PARTITION variable=mean_buff complete dim=1
#pragma HLS ARRAY_PARTITION variable=variance_buff complete dim=1
#pragma HLS ARRAY_PARTITION variable=beta_buff complete dim=1
#pragma HLS resource variable=mean_buff core=RAM_T2P_BRAM
#pragma HLS resource variable=variance_buff core=RAM_T2P_BRAM
#pragma HLS resource variable=beta_buff core=RAM_T2P_BRAM
#endif

	ap_uint<16> startrow_fg0, endrow_fg0;
	ap_uint<12> norm_endrow_fg0;

	bool zeroRead_en = (endrow_in_fb0 < 0) || (startrow_in_fb0 > (conv_desc.conv3d_ip_h-1));

	if (startrow_in_fb0 < 0)
		startrow_fg0 = 0;
	else
		startrow_fg0 = (ap_uint<16>)startrow_in_fb0;

	if(endrow_in_fb0<conv_desc.conv3d_ip_h)
		endrow_fg0 = (ap_uint<16>)endrow_in_fb0;
	else
		endrow_fg0 = conv_desc.conv3d_ip_h-1;

	if (norm_endrow_in_fb0 < input_desc.height)
		norm_endrow_fg0 = norm_endrow_in_fb0;
	else
		norm_endrow_fg0 = input_desc.height - 1;

	ap_uint<32> input_hw_fg0;
	input_hw_fg0 = conv_desc.conv3d_ip_size;//input_desc.size;

	ap_uint<16> num_rows_fg0;
	if(zeroRead_en==1)
		num_rows_fg0 = 0;
	else
		num_rows_fg0 = endrow_fg0 - startrow_fg0 + 1;

	ap_uint<12> norm_num_rows_fg0 = norm_endrow_fg0 - norm_startrow_in_fb0 + 1;

	ap_uint<32> start_off_A_fg0;
#pragma HLS RESOURCE variable=start_off_A_fg0 core=MulnS latency=2
	start_off_A_fg0 = startrow_fg0*conv_desc.conv3d_ip_w;
	ap_uint<32> start_off_l1_fg0;

	if(conv_desc.batch1_int8_en == 1)
		start_off_l1_fg0 = start_off_A_fg0 / 2;
	else
		start_off_l1_fg0 = start_off_A_fg0;

	ap_uint<16> in_plane_align16 = conv_desc.conv3d_ip_pln.range(15,4) + conv_desc.conv3d_ip_pln[3];

	ap_uint<16> layer1_loop_cnt_fg0;
	if(conv_desc.batch1_int8_en==1)
	{
		//************************************
		//************* In Batch1, if image size is odd then it is align to x2
		ap_uint<16> num_row_align2;
		if(num_rows_fg0[0] == 1)
			num_row_align2 = num_rows_fg0 + 1;
		else
			num_row_align2 = num_rows_fg0;
		layer1_loop_cnt_fg0 = num_row_align2.range(15,1) * input_desc.width;
	}
	else
		layer1_loop_cnt_fg0 = num_rows_fg0 * input_desc.width;

	ap_uint<16> layerx_loop_cnt_fg0;

	ap_uint<16> layerx_img_fg0;
	if((conv_desc.conv3d_ip_h == conv_desc.conv3d_istg_row_cnt) && (!(conv_desc.bn_snb_enable || conv_desc.elem_wise_add_en || conv_desc.bn_conv_fuse_en || conv_desc.bn_snb_c_relu_en)))
	{
#if !XI_SINGLE_IO_PORT_EN
		if(conv_desc.batch1_int8_en==0)//#if XI_BATCH1_EN==0
		{		layerx_img_fg0 = 16;
		layerx_loop_cnt_fg0 = input_desc.in_size_x_plane16;
		}
		else
		{
			layerx_img_fg0 = 32;
			layerx_loop_cnt_fg0 = input_desc.in_size_x_plane16/2;
		}
#else
		layerx_img_fg0 = 8;
		layerx_loop_cnt_fg0 = input_desc.in_size_x_plane16*2;
#endif
	}
	else
	{
		layerx_img_fg0 = conv_desc.conv3d_ip_pln;
		layerx_loop_cnt_fg0 = num_rows_fg0 * conv_desc.conv3d_ip_w;
	}

	ap_uint<32> offset_l1_fg0 = 0;
	ap_uint<16> row_stage_offset_A_fg0 = 0;

	if (conv_desc.lrn_inter_sos_enable || conv_desc.lrn_pns_enable || conv_desc.bn_snb_enable || conv_desc.elem_wise_add_en || conv_desc.bn_conv_fuse_en || conv_desc.bn_snb_c_relu_en || conv_desc.bn_en || conv_desc.bn_c_relu_en || conv_desc.scale_relu_en)
	{
		bool norm_flags[12][13];
#pragma HLS ARRAY_PARTITION variable=norm_flags complete dim=0

		ap_uint<8> mode = conv_desc.opcode;


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
		norm_flags[0][12] = 0; //crelu_en
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
		norm_flags[1][12] = 0; //crelu_en

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
		norm_flags[2][12] = 0; //crelu_en

		// BN Scale and Bias
		norm_flags[3][0] = 1; //ddr_en
		norm_flags[3][1] = 1; //beta_sel
		norm_flags[3][2] = 1; //mean_sel
		norm_flags[3][3] = 1; //var_sel
		norm_flags[3][4] = 1; //;//0; //relu
		norm_flags[3][5] = 0; //elem_wise_mul
		norm_flags[3][6] = 0; //elem_wise_add
		norm_flags[3][7] = 0; //pwr_sel
		norm_flags[3][8] = 0; //pwr_0p75
		norm_flags[3][9] = 0; //mul_in_sel
		norm_flags[3][10] = 1; //write_en
		norm_flags[3][11] = 0; //read_2nd_ptr
		norm_flags[3][12] = 0; //crelu_en
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
		norm_flags[5][12] = 0; //crelu_en

		norm_flags[6][0] = 1; //ddr_en
		norm_flags[6][1] = 0; //beta_sel
		norm_flags[6][2] = 0; //mean_sel
		norm_flags[6][3] = 0; //var_sel
		norm_flags[6][4] = conv_desc.relu_en;  //relu
		norm_flags[6][5] = 0; //elem_wise_mul
		norm_flags[6][6] = 1; //elem_wise_add
		norm_flags[6][7] = 0; //pwr_sel
		norm_flags[6][8] = 0; //pwr_0p75
		norm_flags[6][9] = 0; //mul_in_sel
		norm_flags[6][10] = 1; //write_en
		norm_flags[6][11] = 1; //read_2_ptr
		norm_flags[6][12] = 0; //crelu_en
		// BN + Conv Fuse
		norm_flags[7][0] = 1; //ddr_en
		norm_flags[7][1] = 1; //beta_sel
		norm_flags[7][2] = 1; //mean_sel
		norm_flags[7][3] = 1; //var_sel
		norm_flags[7][4] = 1; //;//0; //relu
		norm_flags[7][5] = 0; //elem_wise_mul
		norm_flags[7][6] = 0; //elem_wise_add
		norm_flags[7][7] = 0; //pwr_sel
		norm_flags[7][8] = 0; //pwr_0p75
		norm_flags[7][9] = 0; //mul_in_sel
		norm_flags[7][10] = 0; //write_en
		norm_flags[7][11] = 0; //read_2nd_ptr
		norm_flags[7][12] = 0; //crelu_en
		//BN
		norm_flags[4][0] = 1; //ddr_en
		norm_flags[4][1] = 0; //beta_sel
		norm_flags[4][2] = 1; //mean_sel
		norm_flags[4][3] = 1; //var_sel
		norm_flags[4][4] = 0; //relu
		norm_flags[4][5] = 0; //elem_wise_mul
		norm_flags[4][6] = 0; //elem_wise_add
		norm_flags[4][7] = 0; //pwr_sel
		norm_flags[4][8] = 0; //pwr_0p75
		norm_flags[4][9] = 0; //mul_in_sel
		norm_flags[4][10] = 1; //write_en
		norm_flags[4][11] = 0; //read_2nd_ptr
		norm_flags[4][12] = 0; //crelu_en
		//BN_SnB + C_Relu
		norm_flags[8][0] = 1; //ddr_en
		norm_flags[8][1] = 1; //beta_sel
		norm_flags[8][2] = 1; //mean_sel
		norm_flags[8][3] = 1; //var_sel
		norm_flags[8][4] = 1; //;//0; //relu
		norm_flags[8][5] = 0; //elem_wise_mul
		norm_flags[8][6] = 0; //elem_wise_add
		norm_flags[8][7] = 0; //pwr_sel
		norm_flags[8][8] = 0; //pwr_0p75
		norm_flags[8][9] = 0; //mul_in_sel
		norm_flags[8][10] = 1; //write_en
		norm_flags[8][11] = 0; //read_2nd_ptr
		norm_flags[8][12] = 1; //crelu_en
		//BN + C Relu
		norm_flags[9][0] = 1; //ddr_en
		norm_flags[9][1] = 0; //beta_sel
		norm_flags[9][2] = 1; //mean_sel
		norm_flags[9][3] = 1; //var_sel
		norm_flags[9][4] = 0; //relu
		norm_flags[9][5] = 0; //elem_wise_mul
		norm_flags[9][6] = 0; //elem_wise_add
		norm_flags[9][7] = 0; //pwr_sel
		norm_flags[9][8] = 0; //pwr_0p75
		norm_flags[9][9] = 0; //mul_in_sel
		norm_flags[9][10] = 1; //write_en
		norm_flags[9][11] = 0; //read_2nd_ptr
		norm_flags[9][12] = 1; //crelu_en

		//Scale + Relu
		norm_flags[10][0] = 1; //ddr_en
		norm_flags[10][1] = 1; //beta_sel
		norm_flags[10][2] = 0; //mean_sel
		norm_flags[10][3] = 1; //var_sel
		norm_flags[10][4] = 1; //;//0; //relu
		norm_flags[10][5] = 0; //elem_wise_mul
		norm_flags[10][6] = 0; //elem_wise_add
		norm_flags[10][7] = 0; //pwr_sel
		norm_flags[10][8] = 0; //pwr_0p75
		norm_flags[10][9] = 0; //mul_in_sel
		norm_flags[10][10] = 1; //write_en
		norm_flags[10][11] = 0; //read_2nd_ptr
		norm_flags[10][12] = 0; //crelu_en

		//default
		norm_flags[11][0] = 1; //ddr_en
		norm_flags[11][1] = 1; //beta_sel
		norm_flags[11][2] = 1; //mean_sel
		norm_flags[11][3] = 1; //var_sel
		norm_flags[11][4] = 1; //relu
		norm_flags[11][5] = 1; //elem_wise_mul
		norm_flags[11][6] = 1; //elem_wise_add
		norm_flags[11][7] = 1; //pwr_sel
		norm_flags[11][8] = 1; //pwr_0p75
		norm_flags[11][9] = 1; //mul_in_sel
		norm_flags[11][10] = 1;
		norm_flags[11][11] = 1;
		norm_flags[11][12] = 0; //crelu_en
#if !XI_DISABLE_BN
		if (startrow_fg0 == 0 && (conv_desc.bn_snb_enable || conv_desc.bn_conv_fuse_en || conv_desc.bn_snb_c_relu_en || conv_desc.bn_en || conv_desc.bn_c_relu_en || conv_desc.scale_relu_en))
			LoadMeanBuff(input_desc, gmem_inp_norm_3_fa0, mean_buff, variance_buff, beta_buff);

		for (ap_uint<4> loop_cnt = 0; loop_cnt < conv_desc.norm_loop_cnt;loop_cnt++)
		{
			ap_uint<4> flagset = conv_desc.norm_flag_set[loop_cnt];
			Normalization(conv_desc, layerx_loop_cnt_fg0, input_desc,gmem_input_layer1_fa0, gmem_inp_norm_2_fa0, gmem_bias_fa0,gmem_inp_norm_3_fa0, istaging_buff0_fb0, startrow_fg0,endrow_fg0, mean_buff, variance_buff, beta_buff,
					start_off_A_fg0, norm_flags, flagset);

		}
#endif
	} //bn_en
	else if((startrow_fg0 < conv_desc.conv3d_ip_h))
	{
		if (conv_desc.layer_id != 0)
		{
			ap_uint<16> plane_packed;
#if !XI_SINGLE_IO_PORT_EN
			if(conv_desc.batch1_int8_en==1)//#if XI_BATCH1_EN==1
				plane_packed = 32;
			else
				plane_packed = 16;
#else
			plane_packed = 8;
#endif

			for (ap_uint<16> plane_loop_fg0 = 0; plane_loop_fg0 < layerx_img_fg0;    plane_loop_fg0 += plane_packed)
			{
#pragma HLS LOOP_TRIPCOUNT min=0 max=0

				bool plane32_bool_fg0 = plane_loop_fg0[4];
				ap_uint<2> plane_mod2 = plane_loop_fg0.range(4,3);

#if !XI_IO_64bit_PORT_EN
				ap_uint<32> start_off_fg0 = start_off_A_fg0 + offset_l1_fg0;
#else
				ap_uint<32> start_off_fg0 = 2*(start_off_A_fg0 + offset_l1_fg0);
#endif//XI_IO_64bit_PORT_EN

				InputReadLayerOther_fh
				(conv_desc, layerx_loop_cnt_fg0, input_desc, plane32_bool_fg0,
						gmem_input_layer_other1_fa0 + start_off_fg0,
#if !XI_SINGLE_IO_PORT_EN
						gmem_input_layer_other2_fa0 + start_off_fg0,
#endif
						istaging_buff0_fb0, startrow_fg0, endrow_fg0,
						row_stage_offset_A_fg0,
						plane_mod2);

				offset_l1_fg0 += input_hw_fg0;


#if !XI_SINGLE_IO_PORT_EN
				if(conv_desc.batch1_int8_en==1)
				{
					row_stage_offset_A_fg0 += conv_desc.iw_mul_istg_row_count;
				}
				else
				{
					if (plane32_bool_fg0 == 1 )
						row_stage_offset_A_fg0 += conv_desc.iw_mul_istg_row_count;
				}//batch1 or 2
#else
				if(plane_mod2==3)
					row_stage_offset_A_fg0 += conv_desc.iw_mul_istg_row_count;
#endif

			}
		}
		else
		{
#if XI_MEANSUB_EN
			InputReadLayer1_fi_mean_sub(layer1_loop_cnt_fg0, input_desc,conv_desc,  gmem_input_layer1_fa0 + start_off_l1_fg0, istaging_buff0_fb0);
#else
			InputReadLayer1_fi(layer1_loop_cnt_fg0, input_desc,conv_desc,  gmem_input_layer1_fa0 + start_off_l1_fg0, istaging_buff0_fb0);
#endif

		}
	}

}

void LoadBiasBuffers_ff(
		gmem_biastype *gmem_bias_fa0,
		ap_int<16> bias_buff_fb0[8][XI_BIASMAXSIZE],
		ap_uint<16> outplanes_fb0)
{
#pragma HLS INLINE OFF
#if XI_BIAS_URAM_EN==0
#pragma HLS ARRAY_PARTITION variable=bias_buff_fb0 complete dim=1
#pragma HLS resource variable=bias_buff_fb0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=bias_buff_fb0 core=XPM_MEMORY uram
#pragma HLS ARRAY_RESHAPE variable=bias_buff_fb0 complete dim=1
#endif

	Biasread_Loop:
	for(ap_uint<16> ddr_pntr_ff0=0;ddr_pntr_ff0<(outplanes_fb0 >> XI_BIASPACKCOUNT_LOG2);ddr_pntr_ff0++)
	{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=24 max=24
		biastype bias_word_64bit_ff0 = gmem_bias_fa0[ddr_pntr_ff0];
#if 8==8
		ap_uint<16> bram_index_ff0 = ddr_pntr_ff0/2;
#elif 8==4
		ap_uint<16> bram_index_ff0 = ddr_pntr_ff0;
#endif
		Biastype_Loop:
		for(ap_uint<8> word_ff0=0,bit_ff0=0;word_ff0<(1<<XI_BIASPACKCOUNT_LOG2);word_ff0++,bit_ff0+=16)
		{
#pragma HLS UNROLL
			ap_int<16> bias_value = (ap_int<16>)(bias_word_64bit_ff0.range(bit_ff0+15,bit_ff0));

#if 8==8
			bias_buff_fb0[word_ff0 + (4*ddr_pntr_ff0[0])][bram_index_ff0] = bias_value;
#elif 8==4
			bias_buff_fb0[word_ff0][bram_index_ff0] = bias_value;
#endif
		}//Biastype_Loop

	}//Biasread_Loop
}

void LoadScaleBuffers_ff(
		gmem_biastype *gmem_bias_fa0,
		ap_uint<24> scale_buff_fb0[8][XI_BIASMAXSIZE],
		ap_uint<16> outplanes_fb0)
{
#pragma HLS INLINE OFF
#if XI_SCALE_URAM_EN==0
#pragma HLS ARRAY_PARTITION variable=scale_buff_fb0 complete dim=1
#pragma HLS resource variable=scale_buff_fb0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=scale_buff_fb0 core=XPM_MEMORY uram
#pragma HLS ARRAY_RESHAPE variable=scale_buff_fb0 complete dim=1
#endif

	Biasread_Loop:
	for(ap_uint<16> ddr_pntr_ff0=0;ddr_pntr_ff0<(outplanes_fb0 >> 1);ddr_pntr_ff0++)
	{
#pragma HLS PIPELINE
#pragma HLS LOOP_TRIPCOUNT min=24 max=24
		biastype scale_word_64bit_ff0 = gmem_bias_fa0[ddr_pntr_ff0];

#if		8==8
		ap_uint<16> bram_index_ff0 = ddr_pntr_ff0/4;
#elif 8==4
		ap_uint<16> bram_index_ff0 = ddr_pntr_ff0/2;
#endif

		ap_int<24> scale_value_0 = (ap_int<24>)(scale_word_64bit_ff0.range(31,0));
		ap_int<24> scale_value_1 = (ap_int<24>)(scale_word_64bit_ff0.range(63,32));

#if		8==8
		if(ddr_pntr_ff0.range(1,0) == 0)
		{
			scale_buff_fb0[0 + 0][bram_index_ff0] = scale_value_0;
			scale_buff_fb0[0 + 1][bram_index_ff0] = scale_value_1;
		}

		if(ddr_pntr_ff0.range(1,0) == 1)
		{
			scale_buff_fb0[2 + 0][bram_index_ff0] = scale_value_0;
			scale_buff_fb0[2 + 1][bram_index_ff0] = scale_value_1;
		}

		if(ddr_pntr_ff0.range(1,0) == 2)
		{
			scale_buff_fb0[4 + 0][bram_index_ff0] = scale_value_0;
			scale_buff_fb0[4 + 1][bram_index_ff0] = scale_value_1;
		}

		if(ddr_pntr_ff0.range(1,0) == 3)
		{
			scale_buff_fb0[6 + 0][bram_index_ff0] = scale_value_0;
			scale_buff_fb0[6 + 1][bram_index_ff0] = scale_value_1;
		}
#elif 8==4

		if(ddr_pntr_ff0[0] == 0)
		{
			scale_buff_fb0[0 + 0][bram_index_ff0] = scale_value_0;
			scale_buff_fb0[0 + 1][bram_index_ff0] = scale_value_1;
		}

		if(ddr_pntr_ff0[0] == 1)
		{
			scale_buff_fb0[2 + 0][bram_index_ff0] = scale_value_0;
			scale_buff_fb0[2 + 1][bram_index_ff0] = scale_value_1;
		}
#endif

	}//scaleread_Loop
}

template<int IN_WW, int IN_HH, int OUT_WW, int OUT_HH, int CNUM_KERNELS, int CFILTER_SIZE, int CCONV_STRIDE, int PPOOL_STRIDE, int PPOOL_SIZE, int IINPUT_PLANES, int PNKPF>
void ProcResult_fe(input_struct input_desc,
		conv_struct conv_desc,
		weight_struct weight_desc,
		ap_uint<16> nk_process_fd0,
		output_struct output_desc,
		out_pix_struct out_pixels0_fc0,
		weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],
		ap_uint<64> input_buff0_fc0[XI_PIX_PROC/2][1024],
		ap_uint<72> ostaging_buff0_fb0[2][XI_OSTG_BUFFER_SET][XI_OSTAGEBUFF_DEPTH],
		ap_uint<16> ostgrow_x_width,
		ap_uint<16> out_row_offset_fb0,
		ap_uint<2> mac_fz0,
		bool ap_clk_div2)
{
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=2
#if XI_MERGE_WEIGHTBUFF
#pragma HLS ARRAY_RESHAPE variable=weight_buff0_fd0 complete dim=1
#else
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=1
#endif
#if XI_WTS_URAM_EN==0
#pragma HLS resource variable=weight_buff0_fd0 latency=4
#else
#pragma HLS RESOURCE variable=weight_buff0_fd0 core=XPM_MEMORY uram
#endif

#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#if XI_FEED_URAM_EN==0
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=input_buff0_fc0 core=XPM_MEMORY uram
#endif

#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2
#if XI_OSTG_URAM_EN==0
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM latency=2
#else
#pragma HLS RESOURCE variable=ostaging_buff0_fb0 core=XPM_MEMORY uram
#endif

#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_cols complete dim=1
#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_rows complete dim=1
#pragma HLS data_pack variable=out_pixels0_fc0
#pragma HLS inline off

	ap_int<36> result_ping_fe0[XI_KER_PROC][XI_PIX_PROC];
	ap_int<36> result_pong_fe0[XI_KER_PROC][XI_PIX_PROC];

#if XI_RESULT_BUFFER_FF
#pragma HLS ARRAY_PARTITION variable=result_ping_fe0 complete dim=0
#pragma HLS ARRAY_PARTITION variable=result_pong_fe0 complete dim=0
#elif XI_RESULT_BUFFER_LUTRAM
#pragma HLS ARRAY_PARTITION variable=result_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_pong_fe0 complete dim=1
#pragma HLS resource variable=result_ping_fe0 latency=4 core=RAM_2P_LUTRAM
#pragma HLS resource variable=result_pong_fe0 latency=4 core=RAM_2P_LUTRAM
#else
#pragma HLS ARRAY_PARTITION variable=result_ping_fe0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=result_pong_fe0 complete dim=1
#pragma HLS resource variable=result_ping_fe0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=result_pong_fe0 latency=4 core=RAM_T2P_BRAM
#endif


	ap_uint<14> weight_buff_offset = 0;

	Compute16Ker_fy<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
	(conv_desc, weight_buff0_fd0, input_buff0_fc0, 0,result_ping_fe0, nk_process_fd0, out_pixels0_fc0,output_desc, weight_buff_offset,ap_clk_div2);

	bool flag_fe0=0;
	Nkpf_Loop:
	for(ap_uint<4> nkpf_cnt_fe0=1, nkpf2_cnt_fe0=0;nkpf_cnt_fe0<weight_desc.nkpf; nkpf_cnt_fe0++,nkpf2_cnt_fe0++)
	{
#pragma HLS LOOP_TRIPCOUNT min=7 max=7
		weight_buff_offset += conv_desc.compute_loop_count;
		if(flag_fe0==0)
		{

			Compute16Ker_fy<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
			(conv_desc, weight_buff0_fd0, input_buff0_fc0, nkpf_cnt_fe0,result_pong_fe0, nk_process_fd0,out_pixels0_fc0,output_desc, weight_buff_offset,ap_clk_div2);

			OStgBuffSeq_fx<PNKPF>
			( result_ping_fe0, out_pixels0_fc0, nk_process_fd0, conv_desc, output_desc, ostaging_buff0_fb0,ostgrow_x_width, nkpf2_cnt_fe0,out_row_offset_fb0, mac_fz0);

			flag_fe0=1;
		}
		else
		{

			Compute16Ker_fy<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>
			(conv_desc, weight_buff0_fd0, input_buff0_fc0, nkpf_cnt_fe0,result_ping_fe0, nk_process_fd0,out_pixels0_fc0,output_desc, weight_buff_offset,ap_clk_div2);

			OStgBuffSeq_fx<PNKPF>
			( result_pong_fe0, out_pixels0_fc0, nk_process_fd0, conv_desc, output_desc, ostaging_buff0_fb0,ostgrow_x_width, nkpf2_cnt_fe0,out_row_offset_fb0, mac_fz0);

			flag_fe0=0;
		}

	}//Nkpf_Loop

	ap_uint<4> nkpf2_cnt_fe1=weight_desc.nkpf-1;
	if(flag_fe0==0)
	{
		OStgBuffSeq_fx<PNKPF>
		( result_ping_fe0, out_pixels0_fc0, nk_process_fd0, conv_desc, output_desc, ostaging_buff0_fb0,ostgrow_x_width, nkpf2_cnt_fe1,out_row_offset_fb0, mac_fz0);
	}
	else
	{
		OStgBuffSeq_fx<PNKPF>
		( result_pong_fe0, out_pixels0_fc0, nk_process_fd0, conv_desc, output_desc, ostaging_buff0_fb0,ostgrow_x_width, nkpf2_cnt_fe1,out_row_offset_fb0, mac_fz0);
	}

}

template<int IN_WW, int IN_HH, int OUT_WW, int OUT_HH, int CNUM_KERNELS, int CFILTER_SIZE, int CCONV_STRIDE, int PPOOL_STRIDE, int PPOOL_SIZE, int IINPUT_PLANES, int KERCNT, int PNKPF>
void ProcWeightBuff_fd(input_struct input_desc,
		conv_struct conv_desc,
		weight_struct weight_desc,
		output_struct output_desc,
		out_pix_struct out_pixels0_fc0,
		ap_uint<64> input_buff0_fc0[XI_PIX_PROC / 2][1024],
		ap_uint<72> ostaging_buff0_fb0[2][XI_OSTG_BUFFER_SET][XI_OSTAGEBUFF_DEPTH],
		gmem_weighttype *gmem_weight1_fa0, gmem_weighttype *gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
		gmem_weighttype *gmem_weight3_fa0,
		gmem_weighttype *gmem_weight4_fa0,
#endif
		ap_uint<16> ostgrow_x_width, ap_uint<16> out_row_offset_fb0,
		ap_uint<2> mac_fz0, ap_uint<16> pc_proc_wts, ap_uint<16> pix_proc_cnt,
		bool ap_clk_div2) {
#pragma HLS interface register port=pc_proc_wts
#pragma HLS INLINE OFF

	output_desc.pix_cnt = pix_proc_cnt;

	bool load_ker_enable;
	if (weight_desc.slk_counter == 1)
	{
		if (pc_proc_wts == 0 && out_row_offset_fb0 == 0)
			load_ker_enable = 1;
		else
			load_ker_enable = 0;
	}
	else
	{
		load_ker_enable = 1;
	}
	bool load_ker_en;
	if (conv_desc.sos_enable == 0 && load_ker_enable == 1)
		load_ker_en = 1;
	else
		load_ker_en = 0;

	weight_width weight_buff0_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH],weight_buff1_fd0[XI_KER_PROC][4][XI_WEIGHTBUFF_DEPTH];
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=2
#pragma HLS ARRAY_PARTITION variable=weight_buff1_fd0 complete dim=2
#if XI_MERGE_WEIGHTBUFF
#pragma HLS ARRAY_RESHAPE variable=weight_buff0_fd0 complete dim=1
#pragma HLS ARRAY_RESHAPE variable=weight_buff1_fd0 complete dim=1
#else
#pragma HLS ARRAY_PARTITION variable=weight_buff0_fd0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=weight_buff1_fd0 complete dim=1
#endif
#if XI_WTS_URAM_EN==0
#pragma HLS resource variable=weight_buff0_fd0 latency=4
#pragma HLS resource variable=weight_buff1_fd0 latency=4
#else
#pragma HLS RESOURCE variable=weight_buff0_fd0 core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=weight_buff1_fd0 core=XPM_MEMORY uram
#endif
	ap_uint<12> wt_const_fd0;
	ap_uint<24> weight_offset_fd0;
	ap_uint<24> weight_offset_inc_fd0;
	ap_uint<14> weight_loopCount_fd0;

#if XI_KER_PROC==16

	weight_offset_fd0 = weight_desc.wts_offset_scale*out_pixels0_fc0.current_plane_by4;
	weight_offset_inc_fd0 = weight_desc.wts_loop_cnt;
	weight_loopCount_fd0 = weight_desc.wts_loop_cnt;

#elif XI_KER_PROC==8

	weight_offset_fd0 = weight_desc.wts_offset_scale*out_pixels0_fc0.current_plane_by4;
	int test_weight_offset_fd0 = (((weight_desc.filter_size_square)*(out_pixels0_fc0.current_plane_by4))*output_desc.planes)>>3;
	weight_offset_inc_fd0 = weight_desc.wts_loop_cnt;
	weight_loopCount_fd0 = weight_desc.wts_loop_cnt;
#endif//XI_KER_PROC


	LoadKernelsEn_fz<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>(weight_desc,weight_buff0_fd0,gmem_weight1_fa0+weight_offset_fd0,gmem_weight2_fa0+weight_offset_fd0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
			gmem_weight3_fa0+weight_offset_fd0,gmem_weight4_fa0+weight_offset_fd0,
#endif
			weight_loopCount_fd0,load_ker_en);

	bool flagpro_fd0 = 0;
	ap_uint<16> nk_fd0 = 0, nk_process_fd0=0;
	Nk_Loop:
	for(nk_fd0=(XI_KER_PROC*weight_desc.nkpf),nk_process_fd0=0; nk_fd0 < conv_desc.ker_loop_cnt; nk_fd0 += (XI_KER_PROC*weight_desc.nkpf),nk_process_fd0+=(XI_KER_PROC*weight_desc.nkpf))
	{
#pragma HLS LOOP_TRIPCOUNT min=0 max=0
		weight_offset_fd0 += weight_offset_inc_fd0;

		if(flagpro_fd0==0)
		{
			LoadKernelsEn_fz<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>( weight_desc,weight_buff1_fd0,gmem_weight1_fa0+weight_offset_fd0,gmem_weight2_fa0+weight_offset_fd0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
					gmem_weight3_fa0+weight_offset_fd0,gmem_weight4_fa0+weight_offset_fd0,
#endif
					weight_loopCount_fd0,load_ker_en);
			ProcResult_fe<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,PNKPF>
			(input_desc,conv_desc,weight_desc,nk_process_fd0,output_desc,out_pixels0_fc0, weight_buff0_fd0,input_buff0_fc0,ostaging_buff0_fb0, ostgrow_x_width, out_row_offset_fb0,mac_fz0,ap_clk_div2);
			flagpro_fd0 = 1;
		}
		else
		{
			LoadKernelsEn_fz<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2),PNKPF>(weight_desc,weight_buff0_fd0,gmem_weight1_fa0+weight_offset_fd0,gmem_weight2_fa0+weight_offset_fd0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
					gmem_weight3_fa0+weight_offset_fd0,gmem_weight4_fa0+weight_offset_fd0,
#endif
					weight_loopCount_fd0,load_ker_en);
			ProcResult_fe<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,PNKPF>
			(input_desc,conv_desc,weight_desc,nk_process_fd0,output_desc,out_pixels0_fc0, weight_buff1_fd0,input_buff0_fc0,ostaging_buff0_fb0, ostgrow_x_width, out_row_offset_fb0,mac_fz0,ap_clk_div2);
			flagpro_fd0 = 0;
		}

	}//Nk_Loop

	if(flagpro_fd0==1)
	{
		ProcResult_fe<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,PNKPF>
		(input_desc,conv_desc,weight_desc,nk_process_fd0,output_desc,out_pixels0_fc0, weight_buff1_fd0,input_buff0_fc0,ostaging_buff0_fb0, ostgrow_x_width,out_row_offset_fb0, mac_fz0,ap_clk_div2);
	}
	else
	{
		ProcResult_fe<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,PNKPF>
		(input_desc,conv_desc,weight_desc,nk_process_fd0,output_desc,out_pixels0_fc0, weight_buff0_fd0,input_buff0_fc0,ostaging_buff0_fb0,ostgrow_x_width,out_row_offset_fb0, mac_fz0,ap_clk_div2);
	}

}

template<int IN_WW, int IN_HH, int OUT_WW, int OUT_HH, int CNUM_KERNELS, int CFILTER_SIZE, int CCONV_STRIDE, int PPOOL_STRIDE, int PPOOL_SIZE, int IINPUT_PLANES, int KERCNT, int OUTIMGCNT, int PNKPF>
void ProcInputBuff_fc(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		gmem_weighttype *gmem_weight1_fa0, gmem_weighttype *gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
		gmem_weighttype *gmem_weight3_fa0,
		gmem_weighttype *gmem_weight4_fa0,
#endif
		ap_uint<16> out_row_offset_fb0,
		ap_uint<72> ostaging_buff0_fb0[2][XI_OSTG_BUFFER_SET][XI_OSTAGEBUFF_DEPTH],
		ap_int<16> startrow_fb0, ap_uint<2> mac_fz0, bool last_itr_stage,
		bool ap_clk_div2) {
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2
#if XI_OSTG_URAM_EN==0
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM latency=2
#else
#pragma HLS RESOURCE variable=ostaging_buff0_fb0 core=XPM_MEMORY uram
#endif

#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=1
#if XI_ISTG_URAM_EN==0
#pragma HLS resource variable=istaging_buff0_fb0 core=RAM_T2P_BRAM latency=2
#else
#pragma HLS RESOURCE variable=istaging_buff0_fb0 core=XPM_MEMORY uram
#endif
#pragma HLS INLINE OFF

	
	ap_uint<64> input_buff0_fc0[XI_PIX_PROC/2][1024],input_buff1_fc0[XI_PIX_PROC/2][1024];
#pragma HLS ARRAY_PARTITION variable=input_buff0_fc0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=input_buff1_fc0 complete dim=1
#if XI_FEED_URAM_EN==0
#pragma HLS resource variable=input_buff0_fc0 latency=4 core=RAM_T2P_BRAM
#pragma HLS resource variable=input_buff1_fc0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=input_buff0_fc0 core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=input_buff1_fc0 core=XPM_MEMORY uram
#endif

	out_pix_struct out_pixels0_fc0,out_pixels1_fc0;
#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_cols complete dim=1
#pragma HLS ARRAY_PARTITION variable=out_pixels0_fc0.pix_rows complete dim=1
#pragma HLS ARRAY_PARTITION variable=out_pixels1_fc0.pix_cols complete dim=1
#pragma HLS ARRAY_PARTITION variable=out_pixels1_fc0.pix_rows complete dim=1

	bool en_pad_fc0 = (startrow_fb0 < 0);

	ap_int<16> pad_row_fc0;
	if (en_pad_fc0)
		pad_row_fc0 = startrow_fb0;
	else
		pad_row_fc0 = -conv_desc.pad_num;


	ap_int<16> pad_row_wo_fc0;
	if (en_pad_fc0 == 1)
		pad_row_wo_fc0 = startrow_fb0;
	else
		pad_row_wo_fc0 = 0;

	ap_uint<16> current_plane_fc0 = 0;
	ap_uint<8> straddle_counter_fc0 = conv_desc.straddle;

	ap_uint<16> pc_loop_max_fc0;
	ap_uint<16> ostgrow_x_width;
	if (last_itr_stage == 0)
	{
		ostgrow_x_width = conv_desc.ow_mul_ostg_row_count;
		pc_loop_max_fc0 = conv_desc.pc_loop_bound;
	}
	else
	{
		ostgrow_x_width = conv_desc.ow_mul_ostg_row_cnt_last_itr;
		pc_loop_max_fc0 = conv_desc.pc_loop_bound_last_itr;
	}



	ap_uint<16> row_id_1st_32pix_fc0 = 0;
	ap_uint<16> col_id_1st_32pix_fc0 = 0;
	ap_uint<16> next_col_id_1st_32pix_fc0 = conv_desc.pix_mod_outwidth;
	ap_uint<16> row_id_2nd_32pix_fc0 = conv_desc.pix2_div_outwidth;
	ap_uint<16> col_id_2nd_32pix_fc0 = conv_desc.pix2_mod_outwidth;
	ap_uint<16> next_col_id_2nd_32pix_fc0 = col_id_2nd_32pix_fc0 + conv_desc.pix_mod_outwidth;
	ap_uint<16> pc_proc_wts = 0;
	ap_uint<16> pc_ping, pc_pong;


	LoadFeedingBuff_fl<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2)>
	(input_desc, conv_desc, output_desc,weight_desc,out_pixels0_fc0,istaging_buff0_fb0, input_buff0_fc0 ,0, current_plane_fc0, out_row_offset_fb0,
			/*..continue..*/pad_row_fc0, pad_row_wo_fc0,row_id_1st_32pix_fc0, col_id_1st_32pix_fc0, row_id_2nd_32pix_fc0, col_id_2nd_32pix_fc0, mac_fz0, &pc_ping);

	bool flag_fc0 = 0;
	Pc_Loop:
	for(ap_uint<16> pc_encoded_fc0=conv_desc.pix_per_kp, pc_fc0=0;pc_encoded_fc0<pc_loop_max_fc0;pc_encoded_fc0=pc_encoded_fc0+conv_desc.pix_per_kp)
	{
#pragma HLS LOOP_TRIPCOUNT min=59 max=59

		straddle_counter_fc0--;
#pragma HLS resource variable=current_plane_fc0 core=AddSubnS
		current_plane_fc0+=(conv_desc.straddle_in_plane_inc_by4);
		if(straddle_counter_fc0==0)
		{
			straddle_counter_fc0=conv_desc.straddle;
			current_plane_fc0=0;

			pc_fc0 += conv_desc.pix_per_kp;

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
					/*..continue..*/pad_row_fc0, pad_row_wo_fc0,row_id_1st_32pix_fc0, col_id_1st_32pix_fc0, row_id_2nd_32pix_fc0, col_id_2nd_32pix_fc0, mac_fz0,&pc_pong);

			ProcWeightBuff_fd<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,KERCNT,PNKPF>
			(input_desc, conv_desc, weight_desc,output_desc,out_pixels0_fc0,input_buff0_fc0,ostaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
					gmem_weight3_fa0,gmem_weight4_fa0,
#endif
					ostgrow_x_width, out_row_offset_fb0, mac_fz0, pc_proc_wts,pc_ping,ap_clk_div2);

			flag_fc0 = 1;
		}
		else
		{
			LoadFeedingBuff_fl<CFILTER_SIZE,CCONV_STRIDE,CNUM_KERNELS,(IINPUT_PLANES>>2)>
			(input_desc, conv_desc,output_desc,weight_desc,out_pixels0_fc0,istaging_buff0_fb0, input_buff0_fc0, pc_fc0,current_plane_fc0,out_row_offset_fb0,
					/*..continue..*/pad_row_fc0, pad_row_wo_fc0,row_id_1st_32pix_fc0, col_id_1st_32pix_fc0, row_id_2nd_32pix_fc0, col_id_2nd_32pix_fc0, mac_fz0,&pc_ping);

			ProcWeightBuff_fd<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,KERCNT,PNKPF>
			(input_desc, conv_desc, weight_desc,output_desc,out_pixels1_fc0,input_buff1_fc0,ostaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
					gmem_weight3_fa0,gmem_weight4_fa0,
#endif
					ostgrow_x_width, out_row_offset_fb0, mac_fz0, pc_proc_wts,pc_pong,ap_clk_div2);
			flag_fc0 = 0;
		}
		pc_proc_wts = pc_encoded_fc0;
	}//Pc_Loop

	if(flag_fc0 == 1)
	{
		ProcWeightBuff_fd<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,KERCNT,PNKPF>
		(input_desc, conv_desc, weight_desc,output_desc,out_pixels1_fc0,input_buff1_fc0,ostaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
				gmem_weight3_fa0,gmem_weight4_fa0,
#endif
				ostgrow_x_width, out_row_offset_fb0, mac_fz0, pc_proc_wts,pc_pong,ap_clk_div2);
	}
	else
	{
		ProcWeightBuff_fd<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,KERCNT,PNKPF>
		(input_desc, conv_desc, weight_desc,output_desc,out_pixels0_fc0,input_buff0_fc0,ostaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
				gmem_weight3_fa0,gmem_weight4_fa0,
#endif
				ostgrow_x_width, out_row_offset_fb0, mac_fz0, pc_proc_wts,pc_ping,ap_clk_div2);
	}

}

template<int IN_WW, int IN_HH, int OUT_WW, int OUT_HH, int CNUM_KERNELS,int CFILTER_SIZE, int CCONV_STRIDE, int PPOOL_STRIDE, int PPOOL_SIZE,int IINPUT_PLANES, int KERCNT, int OUTIMGCNT, int PNKPF>
void ProcFeedingBuff_fz(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		gmem_weighttype *gmem_weight1_fa0, gmem_weighttype *gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
		gmem_weighttype *gmem_weight3_fa0,
		gmem_weighttype *gmem_weight4_fa0,
#endif
		ap_uint<16> out_row_offset_fb0,
		ap_uint<72> ostaging_buff0_fb0[2][XI_OSTG_BUFFER_SET][XI_OSTAGEBUFF_DEPTH],
		ap_int<16> startrow_fb0, bool last_itr_stage, bool ap_clk_div2) {
#pragma HLS INLINE OFF


	for(ap_uint<3> mac_fz0 = 0; mac_fz0 < conv_desc.mac_loop_count;mac_fz0++)
	{
		ProcInputBuff_fc<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,(CNUM_KERNELS>>5),((OUT_WW*6)>>5),PNKPF>
		(input_desc, conv_desc,output_desc,weight_desc,istaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
				gmem_weight3_fa0,gmem_weight4_fa0,
#endif
				out_row_offset_fb0, ostaging_buff0_fb0, startrow_fb0, mac_fz0,last_itr_stage,ap_clk_div2);
	}
}

void StoreNormData(conv_struct conv_desc, ap_uint<16> layerx_loop_cnt_fg0,
		input_struct input_desc, gmem_inputtype_layer1 *gmem_istg_out_1_fa0,
#if !XI_SINGLE_IO_PORT_EN
		gmem_inputtype_layer1 *gmem_istg_out_2_fa0,
#endif
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		ap_uint<12> startrow_fg0,
		ap_uint<12> endrow_fg0,
		ap_uint<32> start_off_A_fg0)
{
#pragma HLS INLINE OFF
	ap_uint<32> start_off;

	ap_uint<16> loop_bound;

	loop_bound = conv_desc.conv3d_op_pln;

#if !XI_SINGLE_IO_PORT_EN
	for (ap_uint<16> imgG = 0; imgG < loop_bound; imgG += 16)
#else
		for(ap_uint<16> imgG=0;imgG<loop_bound;imgG+=8)
#endif
		{

#if !XI_SINGLE_IO_PORT_EN
			ap_uint<2> plane32 = imgG[4];

			ap_uint<16> imgGby16;
			imgGby16 = imgG.range(15,4);

			ap_uint<32> offset_l1;

			offset_l1 = (imgGby16)*conv_desc.conv3d_op_size;


			start_off = start_off_A_fg0*2 + offset_l1*2;

			WriteNormData(conv_desc, 2*layerx_loop_cnt_fg0, input_desc,plane32,gmem_istg_out_1_fa0 + start_off,gmem_istg_out_2_fa0 + start_off,istaging_buff0_fb0,startrow_fg0,endrow_fg0,imgG);

#else
			ap_uint<2> plane32 = imgG[5];

			ap_uint<16> imgGby8;
			imgGby8 = imgG.range(15,3);

			ap_uint<32> offset_l1;

			offset_l1 = (imgGby8)*conv_desc.conv3d_op_size;
			start_off = start_off_A_fg0*2 + offset_l1*2;


			WriteNormData(conv_desc, 2*layerx_loop_cnt_fg0, input_desc,plane32,gmem_istg_out_1_fa0 + start_off,istaging_buff0_fb0,startrow_fg0,endrow_fg0,imgG);

#endif
		}

}
template<int IN_WW,int IN_HH,int OUT_WW,int OUT_HH,int CNUM_KERNELS,int CFILTER_SIZE,int CCONV_STRIDE,int PPOOL_STRIDE,int PPOOL_SIZE,int IINPUT_PLANES,int KERCNT,int OUTIMGCNT,int PNKPF>
void ProcFeedingBuff_En_fz(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],
		gmem_weighttype *gmem_weight1_fa0, gmem_weighttype *gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
		gmem_weighttype *gmem_weight3_fa0,
		gmem_weighttype *gmem_weight4_fa0,
#endif
#if !XI_DISABLE_BN
		gmem_inputtype_layer1 *gmem_inp_norm_2_fa0,
		gmem_inputtype_layer1 *gmem_inp_norm_3_fa0,
#endif
		ap_uint<16> out_row_offset_fb0,
		ap_uint<72> ostaging_buff0_fb0[2][XI_OSTG_BUFFER_SET][XI_OSTAGEBUFF_DEPTH],
		ap_int<16> startrow_fb0,

		gmem_inputtype_layer1 *gmem_istg_out_1_fa0,
#if !XI_SINGLE_IO_PORT_EN
		gmem_inputtype_layer1 *gmem_istg_out_2_fa0,
#endif
		ap_uint<16> conv3d_loop_bound,
		gmem_inputtype_layer1 *gmem_input_layer1_fa0,
		gmem_biastype *gmem_bias_fa0,
		ap_int<12> conv3d_startrow_op,
		ap_int<12> conv3d_endrow_op,
		ap_int<12> conv3d_startrow_ip,
		ap_uint<32> conv3d_bias_buf[8][512],
		ap_uint<32> conv_3d_scale_buf[8][512],
#if !XI_DISABLE_BN
		ap_uint<32> conv3d_wts_buf[4][1024],
#endif
		bool last_itr_stage, bool ap_clk_div2) {
#pragma HLS INLINE OFF

	if (conv_desc.conv3d_intg_en || conv_desc.avg_pool_en
			|| conv_desc.max_pool_en || conv_desc.pool_fuse_en) {
		Conv3dTop(conv_desc, istaging_buff0_fb0, conv3d_loop_bound,
				conv3d_startrow_ip, conv3d_bias_buf, conv_3d_scale_buf,
#if !XI_DISABLE_BN
				conv3d_wts_buf,
#endif
				conv3d_startrow_op);


	}

	ap_uint<16> startrow_fg0,endrow_fg0;


	if(conv3d_startrow_op<0)
		startrow_fg0 = 0;
	else
		startrow_fg0 = conv3d_startrow_op;




	if(conv3d_endrow_op<conv_desc.conv3d_op_h)
		endrow_fg0 = conv3d_endrow_op;
	else
		endrow_fg0 = conv_desc.conv3d_op_h-1;
	ap_uint<16> num_rows_fg0 = endrow_fg0-startrow_fg0 +1;

	ap_uint<16> layerx_loop_cnt_fg0;
	layerx_loop_cnt_fg0 = num_rows_fg0 * conv_desc.conv3d_op_w;


	ap_uint<32> start_off_A_fg0;

	start_off_A_fg0 = startrow_fg0*conv_desc.conv3d_op_w;


	if(!(conv_desc.lrn_pns_enable || conv_desc.bn_snb_enable || conv_desc.elem_wise_add_en || conv_desc.bn_snb_c_relu_en || conv_desc.bn_en || conv_desc.bn_c_relu_en || conv_desc.scale_relu_en || conv_desc.max_pool_en || conv_desc.avg_pool_en))
	{
		ProcFeedingBuff_fz<IN_WW,IN_HH,OUT_WW,OUT_HH,CNUM_KERNELS,CFILTER_SIZE,CCONV_STRIDE,PPOOL_STRIDE,PPOOL_SIZE,IINPUT_PLANES,(CNUM_KERNELS>>5),((OUT_WW*6)>>5),PNKPF>
		(input_desc, conv_desc,output_desc,weight_desc,istaging_buff0_fb0,gmem_weight1_fa0,gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
				gmem_weight3_fa0,gmem_weight4_fa0,
#endif
				out_row_offset_fb0, ostaging_buff0_fb0, startrow_fb0,last_itr_stage,ap_clk_div2);
	}

	else
	{

		StoreNormData(conv_desc,
				layerx_loop_cnt_fg0,
				input_desc ,
				gmem_istg_out_1_fa0,
#if !XI_SINGLE_IO_PORT_EN
				gmem_istg_out_2_fa0,
#endif
				istaging_buff0_fb0, startrow_fg0, endrow_fg0, start_off_A_fg0);
	}

}

ap_uint<16> get_conv3d_loop_bound(conv_struct conv_desc,ap_uint<10> conv3d_op_row_rem)
        																																																																																						{
#pragma HLS INLINE OFF


	return (ap_uint<16>)(((conv3d_op_row_rem<conv_desc.conv3d_op_rows)?(ap_uint<16>)conv3d_op_row_rem :(ap_uint<16>)conv_desc.conv3d_op_rows)*(ap_uint<16>)conv_desc.conv3d_op_w*(ap_uint<16>)conv_desc.conv3d_fsz2);  																																																		}

template<int IN_WW,int IN_HH,int OUT_WW,int OUT_HH,int CNUM_KERNELS,int CFILTER_SIZE,int CCONV_STRIDE,int PPOOL_STRIDE,int PPOOL_SIZE,int IINPUT_PLANES,int PNKPF>
void ProcIStagingBuff_fb(input_struct input_desc,
		conv_struct conv_desc,
		output_struct output_desc,
		weight_struct weight_desc,
		gmem_inputtype_layer1 *gmem_input_layer1_fa0,
		gmem_inputtype_layerx *gmem_input_layer_other1_fa0,
#if !XI_SINGLE_IO_PORT_EN
		gmem_inputtype_layerx *gmem_input_layer_other2_fa0,
#endif
		gmem_outputtype *gmem_output1_fa0,
#if !XI_SINGLE_IO_PORT_EN
		gmem_outputtype *gmem_output2_fa0,
#endif
		gmem_weighttype *gmem_weight1_fa0, gmem_weighttype *gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
		gmem_weighttype *gmem_weight3_fa0,
		gmem_weighttype *gmem_weight4_fa0,
#endif
		gmem_biastype *gmem_bias_fa0,
#if !XI_DISABLE_BN
		gmem_inputtype_layer1 *gmem_inp_norm_2_fa0,
		gmem_inputtype_layer1 *gmem_inp_norm_3_fa0,
#endif
		gmem_inputtype_layer1 *gmem_istg_out_1_fa0,
#if !XI_SINGLE_IO_PORT_EN
		gmem_inputtype_layer1 *gmem_istg_out_2_fa0,
#endif
		bool ap_clk_div2) {
#pragma HLS INLINE OFF

	ap_uint<64> istaging_buff0_fb0[8][XI_ISTAGEBUFF_DEPTH],istaging_buff1_fb0[8][XI_ISTAGEBUFF_DEPTH];

#pragma HLS ARRAY_PARTITION variable=istaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=istaging_buff1_fb0 complete dim=1
#if XI_ISTG_URAM_EN==0
#pragma HLS resource variable=istaging_buff0_fb0 core=RAM_T2P_BRAM latency=2
#pragma HLS resource variable=istaging_buff1_fb0 core=RAM_T2P_BRAM latency=2
#else
#pragma HLS RESOURCE variable=istaging_buff0_fb0 core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=istaging_buff1_fb0 core=XPM_MEMORY uram
#endif

	ap_uint<72> ostaging_buff0_fb0[2][XI_OSTG_BUFFER_SET][XI_OSTAGEBUFF_DEPTH],
	ostaging_buff1_fb0[2][XI_OSTG_BUFFER_SET][XI_OSTAGEBUFF_DEPTH];
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff0_fb0 complete dim=2
#pragma HLS ARRAY_PARTITION variable=ostaging_buff1_fb0 complete dim=1
#pragma HLS ARRAY_PARTITION variable=ostaging_buff1_fb0 complete dim=2
#if XI_OSTG_URAM_EN==0
#pragma HLS resource variable=ostaging_buff0_fb0 core=RAM_S2P_BRAM latency=2
#pragma HLS resource variable=ostaging_buff1_fb0 core=RAM_S2P_BRAM latency=2
#else
#pragma HLS RESOURCE variable=ostaging_buff0_fb0 core=XPM_MEMORY uram
#pragma HLS RESOURCE variable=ostaging_buff1_fb0 core=XPM_MEMORY uram
#endif

#if !XI_DISABLE_BN
	short mean_buff[2][1024];
	short variance_buff[2][1024];
	short beta_buff[2][1024];
#pragma HLS ARRAY_PARTITION variable=mean_buff complete dim=1
#pragma HLS ARRAY_PARTITION variable=variance_buff complete dim=1
#pragma HLS ARRAY_PARTITION variable=beta_buff complete dim=1
#pragma HLS resource variable=mean_buff core=RAM_T2P_BRAM
#pragma HLS resource variable=variance_buff core=RAM_T2P_BRAM
#pragma HLS resource variable=beta_buff core=RAM_T2P_BRAM
#endif

	ap_int<16> bias_buff_fb0[8][XI_BIASMAXSIZE];
#if XI_BIAS_URAM_EN==0
#pragma HLS ARRAY_PARTITION variable=bias_buff_fb0 complete dim=1
#pragma HLS resource variable=bias_buff_fb0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=bias_buff_fb0 core=XPM_MEMORY uram
#pragma HLS ARRAY_RESHAPE variable=bias_buff_fb0 complete dim=1
#endif

	ap_uint<24> scale_buff_fb0[8][XI_BIASMAXSIZE];
#if XI_SCALE_URAM_EN==0
#pragma HLS ARRAY_PARTITION variable=scale_buff_fb0 complete dim=1
#pragma HLS resource variable=scale_buff_fb0 latency=4 core=RAM_T2P_BRAM
#else
#pragma HLS RESOURCE variable=scale_buff_fb0 core=XPM_MEMORY uram
#pragma HLS ARRAY_RESHAPE variable=scale_buff_fb0 complete dim=1
#endif

	ap_uint<16> fc_pixel_count = 0;

	ap_uint<32> conv_3d_bias_buf[8][512];
#pragma HLS ARRAY_PARTITION variable=conv_3d_bias_buf complete dim=1
#pragma HLS resource variable=conv_3d_bias_buf core=RAM_T2P_BRAM

	ap_uint<32> conv_3d_scale_buf[8][512];
#pragma HLS ARRAY_PARTITION variable=conv_3d_scale_buf complete dim=1
#pragma HLS resource variable=conv_3d_scale_buf core=RAM_T2P_BRAM

	short bias_loop_cnt = conv_desc.conv3d_op_pln >> 3;
#if !XI_SINGLE_IO_PORT_EN
	if (conv_desc.conv3d_intg_en)
		Conv3dBiasRead(conv_3d_bias_buf, conv_3d_scale_buf, gmem_istg_out_1_fa0,gmem_istg_out_2_fa0, bias_loop_cnt);
	else if (conv_desc.avg_pool_en)
#else
		if(conv_desc.avg_pool_en)
#endif
			Conv3dBiasRead(conv_3d_bias_buf, conv_3d_scale_buf,gmem_input_layer1_fa0, gmem_bias_fa0, bias_loop_cnt);
#if !XI_DISABLE_BN
	ap_uint<32> conv_3d_wts_buf[4][1024];
#pragma HLS ARRAY_PARTITION variable=conv_3d_wts_buf complete dim=1
#pragma HLS resource variable=conv_3d_wts_buf core=RAM_T2P_BRAM

	short wt_loop_cnt = ((conv_desc.conv3d_ip_pln >> 4) * conv_desc.conv3d_fsz2);

	if (conv_desc.conv3d_intg_en)
		Conv3dWtsRead(conv_3d_wts_buf, gmem_inp_norm_2_fa0, gmem_inp_norm_3_fa0,
				wt_loop_cnt);
#endif
	ap_uint<16> bias_size;
	if(conv_desc.fc_enable == 1)
		bias_size = output_desc.height;
	else
		bias_size = output_desc.planes;


	if(!(conv_desc.max_pool_en || conv_desc.avg_pool_en))
	{
		LoadBiasBuffers_ff(gmem_bias_fa0,bias_buff_fb0,bias_size);//output_desc.planes);
		LoadScaleBuffers_ff(gmem_bias_fa0+conv_desc.scale_offset,scale_buff_fb0,bias_size);//output_desc.planes);
	}

	ap_uint<16> rows_to_process_fb0;
	rows_to_process_fb0 = conv_desc.ostg_row_count;
	ap_uint<16> out_row_offset_fb0=0;
	ap_uint<16> out_row_offset_wr_fb0=0;
	ap_int<16> startrow_fb0,endrow_fb0, startrow_process_fb0;
	ap_int<12> norm_startrow_fb0,norm_endrow_fb0,conv3d_endrow_op,conv3d_startrow_op=0;
	ap_int<16> startrow_inc_fb0 = conv_desc.startrow_inc;
	ap_uint<10> conv3d_op_row_rem = input_desc.height;
	ap_uint<16> conv3d_loop_bound = 0;



	startrow_fb0 = conv_desc.in_start_row;
	norm_startrow_fb0 = -conv_desc.pad_num;

	endrow_fb0 = conv_desc.in_end_row;



	ap_uint<16> outrow_max_fb0=output_desc.height;

	conv3d_endrow_op = conv_desc.conv3d_op_rows-1;



	conv3d_loop_bound = get_conv3d_loop_bound(conv_desc,conv3d_op_row_rem);

	conv3d_op_row_rem -= conv_desc.conv3d_op_rows;

	if(conv_desc.read_from_ddr == 1)
	{
#if XI_DISABLE_BN
		LoadIStagingBuff_fg<IN_WW,IN_HH,IINPUT_PLANES>
		(input_desc, conv_desc, gmem_input_layer1_fa0, gmem_input_layer_other1_fa0,
#if !XI_SINGLE_IO_PORT_EN
				gmem_input_layer_other2_fa0,
#endif
				gmem_bias_fa0,gmem_istg_out_1_fa0,
#if !XI_SINGLE_IO_PORT_EN
				gmem_istg_out_2_fa0,
#endif
				istaging_buff0_fb0, startrow_fb0, endrow_fb0,norm_startrow_fb0,norm_endrow_fb0,conv3d_loop_bound);
#else
		LoadIStagingBuff_fg<IN_WW, IN_HH, IINPUT_PLANES>(input_desc, conv_desc,
				gmem_input_layer1_fa0, gmem_input_layer_other1_fa0,
#if !XI_SINGLE_IO_PORT_EN
				gmem_input_layer_other2_fa0,
#endif
				gmem_bias_fa0, gmem_inp_norm_2_fa0,
				gmem_inp_norm_3_fa0, gmem_istg_out_1_fa0, gmem_istg_out_2_fa0,
				istaging_buff0_fb0, startrow_fb0, endrow_fb0, norm_startrow_fb0,
				norm_endrow_fb0, mean_buff, variance_buff, beta_buff,
				conv3d_loop_bound);
#endif	
	}
	startrow_process_fb0 = -conv_desc.pad_num;





	bool write_en_fb0 = 0;
	bool pingpong_flag_fb0 = 0;

	ProciStg_Loop:
	for (ap_uint<16> output_row_fb0 = conv_desc.ostg_row_count,output_row_next_fb0 = conv_desc.ostg_row_count * 2;output_row_fb0 < output_desc.height;
			/*..continue..*/output_row_fb0 += conv_desc.ostg_row_count, output_row_next_fb0 += conv_desc.ostg_row_count)
	{
#pragma HLS LOOP_TRIPCOUNT min=59 max=59

		if (!pingpong_flag_fb0)
		{

			ProcFeedingBuff_En_fz<IN_WW, IN_HH, OUT_WW, OUT_HH, CNUM_KERNELS,CFILTER_SIZE, CCONV_STRIDE, PPOOL_STRIDE, PPOOL_SIZE,IINPUT_PLANES, (CNUM_KERNELS >> 5), ((OUT_WW * 6) >> 5),PNKPF>
			(input_desc, conv_desc, output_desc, weight_desc,
					istaging_buff0_fb0, gmem_weight1_fa0, gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
					gmem_weight3_fa0, gmem_weight4_fa0,
#endif
#if !XI_DISABLE_BN
					gmem_inp_norm_2_fa0, gmem_inp_norm_3_fa0,
#endif
					out_row_offset_fb0, ostaging_buff0_fb0,
					startrow_process_fb0,

					gmem_istg_out_1_fa0,
#if !XI_SINGLE_IO_PORT_EN
					gmem_istg_out_2_fa0,
#endif
					conv3d_loop_bound, gmem_input_layer1_fa0, gmem_bias_fa0,
					conv3d_startrow_op, conv3d_endrow_op, startrow_fb0,
					conv_3d_bias_buf, conv_3d_scale_buf,
#if !XI_DISABLE_BN
					conv_3d_wts_buf,
#endif
					0, ap_clk_div2);

			StoreOStagingBuff_En_fj<CNUM_KERNELS, OUT_WW>(output_desc,
					conv_desc, out_row_offset_wr_fb0, gmem_output1_fa0,
#if !XI_SINGLE_IO_PORT_EN
					gmem_output2_fa0,
#endif
					ostaging_buff1_fb0, write_en_fb0, 0,
					bias_buff_fb0, scale_buff_fb0, &fc_pixel_count);
#pragma HLS resource variable=startrow_fb0 core=AddSubnS
			startrow_fb0 += startrow_inc_fb0;
			endrow_fb0 += startrow_inc_fb0;

#if XI_DISABLE_BN
			LoadIStagingBuff_fg<IN_WW,IN_HH,IINPUT_PLANES>
			(input_desc, conv_desc, gmem_input_layer1_fa0, gmem_input_layer_other1_fa0,
#if !XI_SINGLE_IO_PORT_EN
					gmem_input_layer_other2_fa0,
#endif
					gmem_bias_fa0,gmem_istg_out_1_fa0,
#if !XI_SINGLE_IO_PORT_EN
					gmem_istg_out_2_fa0,
#endif
					istaging_buff1_fb0, startrow_fb0, endrow_fb0,norm_startrow_fb0,norm_endrow_fb0,conv3d_loop_bound);
#else
			LoadIStagingBuff_fg<IN_WW, IN_HH, IINPUT_PLANES>(input_desc,
					conv_desc, gmem_input_layer1_fa0,
					gmem_input_layer_other1_fa0,
#if !XI_SINGLE_IO_PORT_EN
					gmem_input_layer_other2_fa0,
#endif
					gmem_bias_fa0, gmem_inp_norm_2_fa0, gmem_inp_norm_3_fa0,
					gmem_istg_out_1_fa0, gmem_istg_out_2_fa0,
					istaging_buff1_fb0, startrow_fb0, endrow_fb0,
					norm_startrow_fb0, norm_endrow_fb0, mean_buff,
					variance_buff, beta_buff, conv3d_loop_bound);
#endif

			pingpong_flag_fb0 = 1;
		} else {
			ProcFeedingBuff_En_fz<IN_WW, IN_HH, OUT_WW, OUT_HH, CNUM_KERNELS,
			CFILTER_SIZE, CCONV_STRIDE, PPOOL_STRIDE, PPOOL_SIZE,
			IINPUT_PLANES, (CNUM_KERNELS >> 5), ((OUT_WW * 6) >> 5),
			PNKPF>(input_desc, conv_desc, output_desc, weight_desc,
					istaging_buff1_fb0, gmem_weight1_fa0, gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
					gmem_weight3_fa0,gmem_weight4_fa0,
#endif
#if !XI_DISABLE_BN
					gmem_inp_norm_2_fa0, gmem_inp_norm_3_fa0,
#endif
					out_row_offset_fb0, ostaging_buff1_fb0,
					startrow_process_fb0,
					gmem_istg_out_1_fa0,
#if !XI_SINGLE_IO_PORT_EN
					gmem_istg_out_2_fa0,
#endif
					conv3d_loop_bound, gmem_input_layer1_fa0, gmem_bias_fa0,
					conv3d_startrow_op, conv3d_endrow_op, startrow_fb0,
					conv_3d_bias_buf, conv_3d_scale_buf,
#if !XI_DISABLE_BN
					conv_3d_wts_buf,
#endif
					0, ap_clk_div2);

			StoreOStagingBuff_En_fj<CNUM_KERNELS, OUT_WW>(output_desc,
					conv_desc, out_row_offset_wr_fb0, gmem_output1_fa0,
#if !XI_SINGLE_IO_PORT_EN
					gmem_output2_fa0,
#endif
					ostaging_buff0_fb0, write_en_fb0, 0,
					bias_buff_fb0, scale_buff_fb0, &fc_pixel_count);

#pragma HLS resource variable=startrow_fb0 core=AddSubnS	
			startrow_fb0 += startrow_inc_fb0;
			endrow_fb0 += startrow_inc_fb0;

#if XI_DISABLE_BN
			LoadIStagingBuff_fg<IN_WW,IN_HH,IINPUT_PLANES>
			(input_desc, conv_desc, gmem_input_layer1_fa0, gmem_input_layer_other1_fa0,
#if !XI_SINGLE_IO_PORT_EN
					gmem_input_layer_other2_fa0,
#endif
					gmem_bias_fa0,gmem_istg_out_1_fa0,
#if !XI_SINGLE_IO_PORT_EN
					gmem_istg_out_2_fa0,
#endif
					istaging_buff0_fb0, startrow_fb0, endrow_fb0,norm_startrow_fb0,norm_endrow_fb0,conv3d_loop_bound);
#else
			LoadIStagingBuff_fg<IN_WW, IN_HH, IINPUT_PLANES>(input_desc,
					conv_desc, gmem_input_layer1_fa0,
#if !XI_SINGLE_IO_PORT_EN
					gmem_input_layer_other1_fa0,
#endif
					gmem_input_layer_other2_fa0,
					gmem_bias_fa0, gmem_inp_norm_2_fa0, gmem_inp_norm_3_fa0,
					gmem_istg_out_1_fa0, gmem_istg_out_2_fa0,
					istaging_buff0_fb0, startrow_fb0, endrow_fb0,
					norm_startrow_fb0, norm_endrow_fb0, mean_buff,
					variance_buff, beta_buff, conv3d_loop_bound);
#endif
			pingpong_flag_fb0 = 0;
		}
		conv3d_loop_bound = get_conv3d_loop_bound(conv_desc,conv3d_op_row_rem);
		conv3d_op_row_rem -= conv_desc.conv3d_op_rows;
		conv3d_startrow_op = conv3d_endrow_op +1;
		conv3d_endrow_op += conv_desc.conv3d_op_rows;
		startrow_process_fb0 += startrow_inc_fb0;



		outrow_max_fb0 = output_row_next_fb0;
		out_row_offset_wr_fb0 = out_row_offset_fb0;
		out_row_offset_fb0+=conv_desc.ostg_row_count;
		write_en_fb0 = 1;
	}//ProciStg_Loop

	ap_uint<16> rows_to_process_last_fb0 = conv_desc.ostg_row_count - (outrow_max_fb0 - output_desc.height);
#if 1
	if (pingpong_flag_fb0 == 1) {
		ProcFeedingBuff_En_fz<IN_WW, IN_HH, OUT_WW, OUT_HH, CNUM_KERNELS,
		CFILTER_SIZE, CCONV_STRIDE, PPOOL_STRIDE,
		/*..continue..*/PPOOL_SIZE, IINPUT_PLANES, (CNUM_KERNELS >> 5),
		((OUT_WW * 6) >> 5), PNKPF>(input_desc, conv_desc, output_desc,
				weight_desc, istaging_buff1_fb0, gmem_weight1_fa0,
				gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
				gmem_weight3_fa0,gmem_weight4_fa0,
#endif
#if !XI_DISABLE_BN
				gmem_inp_norm_2_fa0, gmem_inp_norm_3_fa0,
#endif
				out_row_offset_fb0,
				/*..continue..*/ostaging_buff1_fb0, startrow_process_fb0,
				gmem_istg_out_1_fa0,
#if !XI_SINGLE_IO_PORT_EN
				gmem_istg_out_2_fa0,
#endif
				conv3d_loop_bound, gmem_input_layer1_fa0, gmem_bias_fa0,
				conv3d_startrow_op, conv3d_endrow_op, startrow_fb0,
				conv_3d_bias_buf, conv_3d_scale_buf,
#if !XI_DISABLE_BN
				conv_3d_wts_buf,
#endif
				1, ap_clk_div2);

		StoreOStagingBuff_En_fj<CNUM_KERNELS, OUT_WW>
		(output_desc, conv_desc,out_row_offset_wr_fb0, gmem_output1_fa0,
#if !XI_SINGLE_IO_PORT_EN
				gmem_output2_fa0,
#endif
				ostaging_buff0_fb0, write_en_fb0, 0,bias_buff_fb0,scale_buff_fb0, &fc_pixel_count);

		StoreOStagingBuff_En_fj<CNUM_KERNELS, OUT_WW>
		(output_desc, conv_desc,out_row_offset_fb0, gmem_output1_fa0,
#if !XI_SINGLE_IO_PORT_EN
				gmem_output2_fa0,
#endif
				ostaging_buff1_fb0, 1, 1,bias_buff_fb0,scale_buff_fb0, &fc_pixel_count);

	} else {
		ProcFeedingBuff_En_fz<IN_WW, IN_HH, OUT_WW, OUT_HH, CNUM_KERNELS,
		CFILTER_SIZE, CCONV_STRIDE, PPOOL_STRIDE,
		/*..continue..*/PPOOL_SIZE, IINPUT_PLANES, (CNUM_KERNELS >> 5),
		((OUT_WW * 6) >> 5), PNKPF>(input_desc, conv_desc, output_desc,
				weight_desc, istaging_buff0_fb0, gmem_weight1_fa0,
				gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
				gmem_weight3_fa0,gmem_weight4_fa0,
#endif
#if !XI_DISABLE_BN
				gmem_inp_norm_2_fa0, gmem_inp_norm_3_fa0,
#endif
				out_row_offset_fb0,
				/*..continue..*/ostaging_buff0_fb0, startrow_process_fb0,
				gmem_istg_out_1_fa0,
#if !XI_SINGLE_IO_PORT_EN
				gmem_istg_out_2_fa0,
#endif
				conv3d_loop_bound, gmem_input_layer1_fa0, gmem_bias_fa0,
				conv3d_startrow_op, conv3d_endrow_op, startrow_fb0,
				conv_3d_bias_buf, conv_3d_scale_buf,
#if !XI_DISABLE_BN
				conv_3d_wts_buf,
#endif
				1, ap_clk_div2);

		StoreOStagingBuff_En_fj<CNUM_KERNELS, OUT_WW>
		(output_desc, conv_desc, out_row_offset_wr_fb0, gmem_output1_fa0,
#if !XI_SINGLE_IO_PORT_EN
				gmem_output2_fa0,
#endif
				ostaging_buff1_fb0, write_en_fb0, 0,bias_buff_fb0,scale_buff_fb0, &fc_pixel_count);

		StoreOStagingBuff_En_fj<CNUM_KERNELS, OUT_WW>
		(output_desc, conv_desc,out_row_offset_fb0, gmem_output1_fa0,
#if !XI_SINGLE_IO_PORT_EN
				gmem_output2_fa0,
#endif
				ostaging_buff0_fb0, 1, 1,bias_buff_fb0,scale_buff_fb0, &fc_pixel_count);
	}
#if XI_ODBC_EN
	if (conv_desc.write_to_ddr == 0)
		CopyOstgToIstg(istaging_buff0_fb0, ostaging_buff0_fb0, conv_desc,output_desc);
#endif	
#endif

}

template<int IN_WW, int IN_HH, int OUT_WW, int OUT_HH, int CNUM_KERNELS,int CFILTER_SIZE, int CCONV_STRIDE, int PPOOL_STRIDE, int PPOOL_SIZE,int IINPUT_PLANES, int PNKPF>
void ConvLayer_fa(input_struct input_desc,
		weight_struct weight_desc,
		output_struct output_desc,
		conv_struct conv_desc,
		group_conv_struct group_desc,
		gmem_weighttype *weights1,
		gmem_weighttype *weights2,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
		gmem_weighttype *weights3,
		gmem_weighttype *weights4,
#endif
		gmem_inputtype_layer1 *input_1st, gmem_inputtype_layerx *input_other1,
#if !XI_SINGLE_IO_PORT_EN
		gmem_inputtype_layerx *input_other2,
#endif
		gmem_biastype *bias,
		gmem_outputtype *output1,
#if !XI_SINGLE_IO_PORT_EN
		gmem_outputtype *output2,
#endif
#if !XI_DISABLE_BN
		gmem_inputtype_layer1 *inp_norm_2, gmem_inputtype_layer1 *inp_norm_3,
#endif
		gmem_inputtype_layer1 *istg_out1,
#if !XI_SINGLE_IO_PORT_EN
		gmem_inputtype_layer1 *istg_out2,
#endif
		bool ap_clk_div2) {
#pragma HLS INLINE OFF

	ap_uint<32> input_group_offset_1st_fa0, input_group_offset_other_fa0,
	weights_group_offset_fa0, output_group_offset_fa0,
	bias_group_offset_fa0;

	if (conv_desc.group_en == 0)
	{
		input_group_offset_1st_fa0 = 0;
		input_group_offset_other_fa0 = 0;
		weights_group_offset_fa0 = 0;
		output_group_offset_fa0 = 0;
		bias_group_offset_fa0 = 0;
	}
	else
	{
		input_group_offset_1st_fa0 = group_desc.input_1st_offset;
		input_group_offset_other_fa0 = group_desc.input_other_offset;
		weights_group_offset_fa0 = group_desc.weight_offset;
		output_group_offset_fa0 = group_desc.output_offset;
		bias_group_offset_fa0 = group_desc.bias_offset;
	}


	gmem_inputtype_layer1 *gmem_input_layer1_fa0 = input_1st + input_group_offset_1st_fa0;
	gmem_inputtype_layerx *gmem_input_layer_other1_fa0 = input_other1 + input_group_offset_other_fa0;
#if !XI_SINGLE_IO_PORT_EN
	gmem_inputtype_layerx *gmem_input_layer_other2_fa0 = input_other2 + input_group_offset_other_fa0;
#endif
	gmem_weighttype *gmem_weight1_fa0 = weights1 + weights_group_offset_fa0;
	gmem_weighttype *gmem_weight2_fa0 = weights2 + weights_group_offset_fa0;
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
	gmem_weighttype *gmem_weight3_fa0 = weights3 + weights_group_offset_fa0;
	gmem_weighttype *gmem_weight4_fa0 = weights4 + weights_group_offset_fa0;
#endif
	gmem_biastype *gmem_bias_fa0 = bias + bias_group_offset_fa0;
	gmem_outputtype *gmem_output1_fa0 = output1 + output_group_offset_fa0;
#if !XI_SINGLE_IO_PORT_EN
	gmem_outputtype *gmem_output2_fa0 = output2 + output_group_offset_fa0;
#endif
#if !XI_DISABLE_BN
	gmem_inputtype_layer1 *gmem_inp_norm_2_fa0 = inp_norm_2;
	gmem_inputtype_layer1 *gmem_inp_norm_3_fa0 = inp_norm_3;
#endif
	gmem_inputtype_layer1 *gmem_istg_out_1_fa0 = istg_out1;
#if !XI_SINGLE_IO_PORT_EN
	gmem_inputtype_layer1 *gmem_istg_out_2_fa0 = istg_out2;
#endif

	ProcIStagingBuff_fb<IN_WW, IN_HH, OUT_WW, OUT_HH, CNUM_KERNELS,CFILTER_SIZE, CCONV_STRIDE, PPOOL_STRIDE, PPOOL_SIZE, IINPUT_PLANES,PNKPF>
	(input_desc, conv_desc, output_desc, weight_desc,
			gmem_input_layer1_fa0, gmem_input_layer_other1_fa0,
#if !XI_SINGLE_IO_PORT_EN
			gmem_input_layer_other2_fa0,
#endif
			gmem_output1_fa0,
#if !XI_SINGLE_IO_PORT_EN
			gmem_output2_fa0,
#endif
			gmem_weight1_fa0, gmem_weight2_fa0,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
			gmem_weight3_fa0,gmem_weight4_fa0,
#endif
			gmem_bias_fa0,
#if !XI_DISABLE_BN
			gmem_inp_norm_2_fa0, gmem_inp_norm_3_fa0,
#endif
			gmem_istg_out_1_fa0,
#if !XI_SINGLE_IO_PORT_EN
			gmem_istg_out_2_fa0,
#endif
			ap_clk_div2);
}

template<int IN_WW, int IN_HH, int OUT_WW, int OUT_HH, int CNUM_KERNELS,
int CFILTER_SIZE, int CCONV_STRIDE, int PPOOL_STRIDE, int PPOOL_SIZE,
int IINPUT_PLANES, int PNKPF>
void Convolution(gmem_weighttype *weights1, gmem_weighttype *weights2,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
		gmem_weighttype *weights3,
		gmem_weighttype *weights4,
#endif
		gmem_outputtype *output1,
#if !XI_SINGLE_IO_PORT_EN
		gmem_outputtype *output2,
#endif
		gmem_inputtype_layerx *input_other1,
#if !XI_SINGLE_IO_PORT_EN
		gmem_inputtype_layerx *input_other2,
#endif
		gmem_inputtype_layer1 *input_1st,
		gmem_biastype *bias,
#if !XI_DISABLE_BN
		gmem_inputtype_layer1 *inp_norm_2, gmem_inputtype_layer1 *inp_norm_3,
#endif
		gmem_inputtype_layer1 *istg_out1,
#if !XI_SINGLE_IO_PORT_EN
		gmem_inputtype_layer1 *istg_out2,
#endif
		int *scalar_conv_args, bool ap_clk_div2) {
#pragma HLS INLINE OFF

	input_struct input_desc;
	output_struct output_desc;
	weight_struct weight_desc;
	conv_struct conv_desc;
	group_conv_struct group_desc;
	LoadDesc_ffa(scalar_conv_args, input_desc, output_desc, weight_desc,conv_desc, group_desc);

	ConvLayer_fa<IN_WW, IN_HH, OUT_WW, OUT_HH, CNUM_KERNELS, CFILTER_SIZE,
	CCONV_STRIDE, PPOOL_STRIDE, PPOOL_SIZE, IINPUT_PLANES, PNKPF>(
			input_desc, weight_desc, output_desc, conv_desc, group_desc,
			weights1 + weight_desc.weight_offset,
			weights2 + weight_desc.weight_offset,

			/*..continue..*/
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
			weights3 + weight_desc.weight_offset,weights4 + weight_desc.weight_offset,
#endif
			input_1st, input_other1,
#if !XI_SINGLE_IO_PORT_EN
			input_other2,
#endif
			/*..continue..*/bias + output_desc.bias_offset,
			output1 + output_desc.out_offset,
#if !XI_SINGLE_IO_PORT_EN
			output2 + output_desc.out_offset,
#endif
			/*..continue..*/
#if !XI_DISABLE_BN
			inp_norm_2, inp_norm_3,
#endif
			istg_out1,
#if !XI_SINGLE_IO_PORT_EN
			istg_out2,
#endif
			ap_clk_div2);

}
