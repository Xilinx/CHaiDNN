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

#ifndef _XI_CONV_DESC_H_
#define _XI_CONV_DESC_H_

typedef struct input_descriptor {
	ap_uint<16> width;
	ap_uint<16> height;
	ap_uint<32> size;
	ap_uint<16> planes;
	ap_uint<16> compute_planes;
	ap_int<8> mean_value[4];// Mean Value for Plane wise mean sub
        bool mean_sub_flag;
    	ap_uint<16> in_size_x_plane16;
    	ap_uint<16> row8_x_iw;
}input_struct;

typedef struct output_descriptor {
	ap_uint<16> width;
	ap_uint<16> height;
	ap_uint<32> size;
	ap_uint<16> planes;
	ap_uint<32> out_offset;
        ap_uint<32> bias_offset;
      	ap_uint<16> out_size_x_plane16;
      	ap_uint<32> crelu_offset_ptr0;
      	ap_uint<32> crelu_offset_ptr1;
      	ap_uint<16> pix_cnt;
}output_struct;

typedef struct weight_descriptor {
	ap_uint<16> num_kernels;
	ap_uint<8> filter_size;
	ap_uint<16> filter_size_square;
	ap_uint<4> filter_stride;
	ap_uint<4> nkpf;// number of kernels processing factor
	bool slk_en;// single Load kernel Enable
	bool slk_counter;// SLK counter
        ap_uint<32> weight_offset;
        ap_uint<3> dilation_factor;
        ap_uint<8> filter_size_dilated;
        ap_uint<14> wts_loop_cnt;
        ap_uint<24> wts_offset_scale;
}weight_struct;

typedef struct conv_descriptor {
	ap_int<16> in_start_row;
	ap_int<16> in_end_row;
	bool group_en;
	bool pad_en;
	ap_uint<8> pad_num;
	bool relu_en;
	ap_uint<4> layer_id;
	ap_uint<16> ostg_row_count;
	ap_uint<16> ostg_row_cnt_last_itr;
	ap_uint<16> istg_row_count;
	ap_uint<8> straddle;
	ap_uint<12> compute_loop_count;
	ap_uint<16> out_pix;
	ap_uint<5> shift_precision1;
	ap_uint<5> shift_precision2;
	ap_uint<5> inout_precision;
	bool buff_split;
	ap_uint<16> pix_by_outwidth;
	ap_uint<16> pix_mod_outwidth;
	ap_uint<16> pix2_div_outwidth;
	ap_uint<16> pix2_div_outwidth_x_ow;
	ap_uint<16> pix2_mod_outwidth;
	ap_uint<8> fsz_by_stride;
	ap_uint<8> fsz_mod_stride;
	ap_uint<3> mac_loop_count;
	ap_uint<8> opcode;
	bool conv_enable;
	bool lrn_intra_sos_enable;
	bool lrn_inter_sos_enable;
	bool bn_intra_sos_enable;
	bool l2_sos_enable;
	bool sos_enable;
	bool bn_intra_mean_enable;
	bool bn_snb_enable;
	ap_uint<8> pix_per_kp;
	ap_uint<12> loop_bound_1x1_filter;
	ap_uint<16> feeding_buff_plane_loop_bound;
	ap_uint<8>  feeding_buff_row_loop_bound;
	ap_uint<10> straddle_in_plane_inc_by4;
	bool read_from_ddr;
	bool write_to_ddr;
	bool bn_en;
	ap_int<16> bn_beta;
	ap_uint<4> mode;
	ap_uint<32> elem_wise_offset;
	ap_uint<32> bn_mean_offset;
	bool intra_en;
	ap_uint<18> mul_in;
	ap_uint<5> norm_prec;
	ap_uint<8> adder_loop_cnt;
	ap_uint<16> ker_loop_cnt;
	ap_uint<5> norm_prec_2;
	ap_uint<5> norm_prec_3;
	bool lrn_pns_enable;
	ap_int<16> norm_k;
	ap_uint<4> norm_loop_cnt;
	ap_uint<4> norm_flag_set[4];
	bool elem_wise_add_en;
	bool bn_conv_fuse_en;
	bool bn_snb_c_relu_en;
	bool bn_c_relu_en;
	bool scale_relu_en;
	bool conv_c_relu_en;
	ap_uint<2> conv_c_relu_loop_cnt;
	bool int6_en_in;
	bool int6_en_out;
	ap_uint<2> int8_2x_loopbound;
	ap_uint<24> overflow_pattern;
	ap_uint<24> overflow_pattern_bn;
	ap_uint<16> iw_mul_istg_row_count;
	ap_uint<16> ow_mul_ostg_row_count;
	ap_uint<16> ow_mul_ostg_row_cnt_last_itr;
	ap_uint<10> feed_addr16[16];
	int rounding_conv;
	int rounding_bn;
	ap_uint<16> startrow_inc;
	ap_uint<16> pc_loop_bound_last_itr;
	ap_uint<16> pc_loop_bound;
	ap_uint<8> fsz_by_stride_x_stride;

	bool conv3d_intg_en;
	ap_uint<10> conv3d_inp_offset;
	ap_uint<16> conv3d_ip_h;
	ap_uint<16> conv3d_ip_w;
	ap_uint<32> conv3d_ip_size;
	ap_uint<32> conv3d_op_size;
	ap_uint<16> conv3d_op_h;
	ap_uint<16> conv3d_op_w;
	ap_uint<16> conv3d_ip_pln;
	ap_uint<16> conv3d_op_pln;
	ap_uint<8> conv3d_pad;
	ap_uint<3> conv3d_stride;
	ap_uint<8> conv3d_ft_h;
	ap_uint<8> conv3d_ft_w;
	ap_uint<8> conv3d_fsz2;
	ap_uint<10> conv3d_op_rows;
	ap_uint<10> conv3d_istg_row_cnt;
	ap_uint<6> conv3d_outshift;
	bool conv3d_relu_en;
	bool batch1_int8_en;
	bool max_pool_en;
	bool avg_pool_en;
	bool pool_fuse_en;
	ap_uint<8> avg_pool_mul;
	ap_int<24> conv3d_rndval;
	bool fc_enable;
	ap_uint<16> pool_ew;
	int scale_offset;
	bool offline_quant_en;
} conv_struct;

typedef struct group_conv_descriptor {
	ap_uint<32> input_1st_offset;
	ap_uint<32> input_other_offset;
	ap_uint<32> weight_offset;
	ap_uint<32> output_offset;
	ap_uint<32> bias_offset;
} group_conv_struct;

typedef struct output_pixels_descriptor {
	ap_uint<16> pix_rows[XI_PIX_PROC];
	ap_uint<16> pix_cols[XI_PIX_PROC];
	ap_uint<16> current_plane_by4;
} out_pix_struct;

typedef struct output_pixels2_descriptor {
	ap_uint<16> pix_rows[XI_PIX_PROC / 2];
	ap_uint<16> pix_cols[XI_PIX_PROC / 2];
} out_pix2_struct;

#endif// _XI_CONV_DESC_H_
