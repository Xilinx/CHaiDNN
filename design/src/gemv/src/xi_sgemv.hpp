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

#ifndef _XI_SGEMV_HPP_
#define _XI_SGEMV_HPP_

#define BRAM_SIZE 1024
//#define OUT_BRAM_SIZE  4096

#include "../include/xi_cnn_types.h"

#if _ASSERT_
#include "assert.h"
#endif

#if _DEBUG_
FILE * fp1 = fopen("dut.txt","w");
#endif

#if 1
//## Process Function
template<int IN_BUS_WIDTH, int IN_BUS_SHIFT_CNT>
void dot16(XI_IN_TYPE(IN_BUS_WIDTH) input_vector1, XI_IN_TYPE(IN_BUS_WIDTH) input_vector2, float* output)
{
#pragma HLS inline off

	float prod_buf[GMEM_ELEMENT];
	//float add_s1[1];
	//float add_s2[4];
	//float add_s3[2];
	float dot_res;

	Loop_16mul_stage:
	for(int process = 0; process < GMEM_ELEMENT; process++)
	{
#pragma HLS unroll
		ap_uint32_t i_value1 = (ap_uint32_t)input_vector1.range((((process+1)<<5) - 1), (process<<5));
		float f_value1 = *((float *)&i_value1);

		ap_uint32_t i_value2 = (ap_uint32_t)input_vector2.range((((process+1)<<5) - 1), (process<<5));
		float f_value2 = *((float *)&i_value2);

		float f_result = f_value1 * f_value2;
		//#pragma HLS resource variable=f_result core=FMul_fulldsp

		prod_buf[process] = f_result;
	}

	dot_res = prod_buf[0] + prod_buf[1];
	*output = dot_res;
}

//## Process Function
template<int IN_BUS_WIDTH, int IN_BUS_SHIFT_CNT>
void dot32(XI_IN_TYPE(IN_BUS_WIDTH) input_vector1, XI_IN_TYPE(IN_BUS_WIDTH) input_vector2, data_t* output)
{
#pragma HLS inline off

	data_t prod_buf[GMEM_ELEMENT];
	data_t add_s1[16];
	data_t add_s2[8];
	data_t add_s3[4];
	data_t add_s4[2];
	data_t dot_res;

	Loop_32mul_stage:
	for(int process = 0, bit=0; process < GMEM_ELEMENT; process++, bit+=16)
	{
#pragma HLS unroll
		ap_uint16_t i_value1 = (ap_uint16_t)input_vector1.range(bit + 15, bit);
		data_t f_value1 = *((data_t *)&i_value1);

		ap_uint16_t i_value2 = (ap_uint16_t)input_vector2.range(bit + 15, bit);
		data_t f_value2 = *((data_t *)&i_value2);

		data_t f_result = f_value1 * f_value2;
		//#pragma HLS resource variable=f_result core=FMul_fulldsp

		prod_buf[process] = f_result;
	}

	Loop_16add_stage:
	for(int l1 = 0, index1 = 0, index2 = 1; l1 < GMEM_ELEMENT_BY2; l1++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s1[l1] = prod_buf[index1] + prod_buf[index2];
	}

#if 0
	Loop_8add_stage:
	for(int l2 = 0, index1 = 0, index2 = 1; l2 < 8; l2++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s2[l2] = add_s1[index1] + add_s1[index2];
	}

	Loop_4add_stage:
	for(int l3 = 0, index1 = 0, index2 = 1; l3 < 4; l3++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s3[l3] = add_s2[index1] + add_s2[index2];
	}

	Loop_2add_stage:
	for(int l4 = 0, index1 = 0, index2 = 1; l4 < 2; l4++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s4[l4] = add_s3[index1] + add_s3[index2];
	}
#endif

	dot_res = add_s1[0] + add_s1[1];
	*output = dot_res;
}

//## Process Function
template<int IN_BUS_WIDTH, int IN_BUS_SHIFT_CNT>
void dot32_fxdpt(XI_IN_TYPE(IN_BUS_WIDTH) input_vector1, TYPE_INPUT_BUFF input_vector2, int* output)
{
#pragma HLS inline off

	int prod_buf[8];
#pragma HLS array_partition variable=prod_buf complete dim=1
	int add_s1[4];
#pragma HLS array_partition variable=add_s1 complete dim=1
	int add_s2[2];
#pragma HLS array_partition variable=add_s2 complete dim=1
	int dot_res;

	Loop_32mul_stage:
	for(int process = 0, bit=0, bit2=0; process < 8; process++, bit+=16, bit2+=8)
	{
#pragma HLS unroll
		ap_int<8> i_value1 = (ap_int<8>)input_vector1.range(bit2 + 7, bit2);
		short i_value2 = (short)input_vector2.range(bit + 15, bit);
#if 0//!__SYNTHESIS__
		fprintf(stderr,"\n[%d] %f x %f ",process,(float)i_value2/(float)(1<<2),(float)i_value1/(float)(1<<7));
#endif
		int f_result = (int)(i_value1 * i_value2);

		prod_buf[process] = f_result;
	}

	Loop_16add_stage:
	for(int l1 = 0, index1 = 0, index2 = 1; l1 < 4; l1++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s1[l1] = prod_buf[index1] + prod_buf[index2];
	}
	Loop_8add_stage:
	for(int l1 = 0, index1 = 0, index2 = 1; l1 < 2; l1++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s2[l1] = add_s1[index1] + add_s1[index2];
	}

	dot_res = add_s2[0] + add_s2[1];
	*output = dot_res>>ACCUMULATOR_PRECISION; // multiplication result save as 24 bit. 31-24=7
#if 0//!__SYNTHESIS__
	fprintf(stderr,"\ndot = %f", (float)dot_res/(float)(1<<9));
#endif
}

void dot8_fxdpt(ap_uint<64> weights, TYPE_INPUT_BUFF inputs, int* output)
{
#pragma HLS inline off

	int prod_buf[8];
#pragma HLS array_partition variable=prod_buf complete dim=1
	int add_s1[4];
#pragma HLS array_partition variable=add_s1 complete dim=1
	int add_s2[2];
#pragma HLS array_partition variable=add_s2 complete dim=1
	int dot_res;

	Loop_32mul_stage:
	for(int process = 0, bit=0, bit2=0; process < 8; process++, bit+=16, bit2+=8)
	{
#pragma HLS unroll
		ap_int<8> i_value1 = (ap_int<8>)weights.range(bit2 + 7, bit2);
		short i_value2 = (short)inputs.range(bit + 15, bit);
#if 0//!__SYNTHESIS__
		fprintf(stderr,"\n[%d] %f x %f ",process,(float)i_value2/(float)(1<<2),(float)i_value1/(float)(1<<7));
#endif
		int f_result = (int)(i_value1 * i_value2);

		prod_buf[process] = f_result;
	}

	Loop_16add_stage:
	for(int l1 = 0, index1 = 0, index2 = 1; l1 < 4; l1++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s1[l1] = prod_buf[index1] + prod_buf[index2];
	}
	Loop_8add_stage:
	for(int l1 = 0, index1 = 0, index2 = 1; l1 < 2; l1++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s2[l1] = add_s1[index1] + add_s1[index2];
	}

	dot_res = add_s2[0] + add_s2[1];
	*output = dot_res>>ACCUMULATOR_PRECISION; // multiplication result save as 24 bit. 31-24=7
#if 0//!__SYNTHESIS__
	fprintf(stderr,"\ndot = %f", (float)dot_res/(float)(1<<9));
#endif
}

#else

//## Process Function
void dot16(ap_uint512_t input_vector1, ap_uint512_t input_vector2, data_t* output)
{
#pragma HLS inline off

	data_t prod_buf[16];
	data_t add_s1[8];
	data_t add_s2[4];
	data_t add_s3[2];
	data_t dot_res;

	//fprintf(stderr,"\n dot product:\t");
	Loop_16mul_stage:
	for(int process = 0, bit=0; process < 16; process++, bit+=32)
	{
#pragma HLS unroll
		ap_uint32_t i_value1 = (ap_uint32_t)input_vector1.range(bit+31, bit);
		data_t f_value1 = *(data_t *)&i_value1;

		ap_uint32_t i_value2 = (ap_uint32_t)input_vector2.range(bit+31, bit);
		data_t f_value2 = *(data_t *)&i_value2;

		data_t f_result = f_value1 * f_value2;
#pragma HLS resource variable=f_result core=FMul_fulldsp

		//	fprintf(stderr, "%fx%f=%f,,",f_value1, f_value2, f_result);
		prod_buf[process] = f_result;
	}

	Loop_8add_stage:
	for(int l1 = 0, index1 = 0, index2 = 1; l1 < 8; l1++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s1[l1] = prod_buf[index1] + prod_buf[index2];
	}

	Loop_4add_stage:
	for(int l2 = 0, index1 = 0, index2 = 1; l2 < 4; l2++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s2[l2] = add_s1[index1] + add_s1[index2];
	}

	Loop_2add_stage:
	for(int l3 = 0, index1 = 0, index2 = 1; l3 < 2; l3++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s3[l3] = add_s2[index1] + add_s2[index2];
	}

	dot_res = add_s3[0] + add_s3[1];
	*output = dot_res;
	//	fprintf(stderr, "\nout_dot = %f", dot_res);
}

//## Process Function
void dot32(ap_uint512_t input_vector1, ap_uint512_t input_vector2, data_t* output)
{
#pragma HLS inline off

	data_t prod_buf[32];
	data_t add_s1[16];
	data_t add_s2[8];
	data_t add_s3[4];
	data_t add_s4[2];
	data_t dot_res;

	Loop_32mul_stage:
	for(int process = 0, bit=0; process < 32; process++, bit+=16)
	{
#pragma HLS unroll
		ap_uint16_t i_value1 = (ap_uint16_t)input_vector1.range(bit + 15, bit);
		data_t f_value1 = *((data_t *)&i_value1);

		ap_uint16_t i_value2 = (ap_uint16_t)input_vector2.range(bit + 15, bit);
		data_t f_value2 = *((data_t *)&i_value2);

		data_t f_result = f_value1 * f_value2;
		//#pragma HLS resource variable=f_result core=FMul_fulldsp

		prod_buf[process] = f_result;
	}

	Loop_16add_stage:
	for(int l1 = 0, index1 = 0, index2 = 1; l1 < 16; l1++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s1[l1] = prod_buf[index1] + prod_buf[index2];
	}

	Loop_8add_stage:
	for(int l2 = 0, index1 = 0, index2 = 1; l2 < 8; l2++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s2[l2] = add_s1[index1] + add_s1[index2];
	}

	Loop_4add_stage:
	for(int l3 = 0, index1 = 0, index2 = 1; l3 < 4; l3++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s3[l3] = add_s2[index1] + add_s2[index2];
	}

	Loop_2add_stage:
	for(int l4 = 0, index1 = 0, index2 = 1; l4 < 2; l4++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s4[l4] = add_s3[index1] + add_s3[index2];
	}

	dot_res = add_s4[0] + add_s4[1];
	*output = dot_res;
}

//## Process Function
void dot32_fxdpt(ap_uint512_t input_vector1, ap_uint512_t input_vector2, int* output)
{
#pragma HLS inline off

	int prod_buf[32];
	int add_s1[16];
	int add_s2[8];
	int add_s3[4];
	int add_s4[2];
	int dot_res;

	Loop_32mul_stage:
	for(int process = 0, bit=0; process < 32; process++, bit+=16)
	{
#pragma HLS unroll
		short i_value1 = (short)input_vector1.range(bit + 15, bit);
		short i_value2 = (short)input_vector2.range(bit + 15, bit);

		int f_result = (int)(i_value1 * i_value2);
		//#pragma HLS resource variable=f_result core=FMul_fulldsp

		prod_buf[process] = f_result;
	}

	Loop_16add_stage:
	for(int l1 = 0, index1 = 0, index2 = 1; l1 < 16; l1++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s1[l1] = prod_buf[index1] + prod_buf[index2];
	}

	Loop_8add_stage:
	for(int l2 = 0, index1 = 0, index2 = 1; l2 < 8; l2++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s2[l2] = add_s1[index1] + add_s1[index2];
	}

	Loop_4add_stage:
	for(int l3 = 0, index1 = 0, index2 = 1; l3 < 4; l3++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s3[l3] = add_s2[index1] + add_s2[index2];
	}

	Loop_2add_stage:
	for(int l4 = 0, index1 = 0, index2 = 1; l4 < 2; l4++, index1 += 2, index2 += 2)
	{
#pragma HLS unroll
		add_s4[l4] = add_s3[index1] + add_s3[index2];
	}

	dot_res = add_s4[0] + add_s4[1];
	*output = dot_res>>7; // multiplication result save as 24 bit. 31-24=7
}
#endif

/*********************************************************************************
 * Main function : GEMV function in single precision
 *********************************************************************************/
template<int IN_BUS_WIDTH, int IN_BUS_SHIFT_CNT, int Morder_TC, int Norder_TC, int TC>
void xiSgemv(
		GMEM_TYPE * weights1,
		GMEM_TYPE * weights2,
		GMEM_TYPE * input,
		GMEM_TYPE * bias,
		GMEM_TYPE * output,
		int *scalar_gemv_args
)
{
	//#pragma HLS inline off

	int _order_m=scalar_gemv_args[0];
	int _order_n=scalar_gemv_args[1];
	int _en_relu=scalar_gemv_args[2];
	int _fbits_weights=scalar_gemv_args[3];
	int _fbits_input=scalar_gemv_args[4];
	int _fbits_bias=scalar_gemv_args[5];
	int _fbits_output=scalar_gemv_args[6];

	ap_uint<32> n_new = (ap_uint<32>)(_order_n);
	ap_uint<5> check_32 = n_new(2,0);
	bool add_bit;
	if(check_32 == 0)
		add_bit = 0;
	else
		add_bit = 1;
	ap_uint<32> n_order_mul32 = ((n_new >> LOG2_GMEM_ELEMENT) + add_bit) << LOG2_GMEM_ELEMENT;


	/**************
	Matrix : m x n
	Vector : n x 1
	 ***************/
	ap_uint<16> _m = (ap_uint<16>)_order_m;
	ap_uint<16> _n = (ap_uint<16>)n_order_mul32;
	int en_relu = _en_relu;

	bool relu = en_relu ? true : false;

#if FP_ENABLE
	ap_int<8> shift_in  = _fbits_bias - (_fbits_weights + _fbits_input - (ACCUMULATOR_PRECISION));  //rt shift if positive
	ap_int<8> shift_out = (_fbits_weights + _fbits_input - (ACCUMULATOR_PRECISION)) - _fbits_output; //rt shift if positive
	ap_int<8> shift_in_neg = -shift_in;
	ap_int<8> shift_out_neg = -shift_out;
#endif

	//##local storage for x elements and y elements
	TYPE_INPUT_BUFF x_bram[4][BRAM_SIZE];
#pragma HLS array_partition variable=x_bram complete dim=1
#pragma HLS resource variable=x_bram core=RAM_1P_BRAM
#if FP_ENABLE
	Y_BRAM_TYPE y_bram[8][OUT_SIZE];
	//int y_bram[12][OUT_BRAM_SIZE];
#else
	data_t y_bram[12][OUT_BRAM_SIZE];
#endif//FP_ENABLE
#pragma HLS array_partition variable=y_bram complete dim=1
#pragma HLS resource variable=y_bram core=RAM_S2P_BRAM

	//	GMEM_AP_TYPE A_bram[1];

#if FP_ENABLE

//	int res1;
#else
	data_t res = 0;//.0f;
#endif//FP_ENABLE

	int n_by_16 = (_n>>3);
	GMEM_AP_TYPE in_read_next_state=0;
	Read_X:
	for(ap_uint<16> ddr_read = 0; ddr_read < n_by_16; ddr_read++)
	{
#pragma HLS loop_tripcount min=Norder_TC max=Norder_TC
#pragma HLS pipeline II=1

#if WEIGHT_8BIT_EN==0
		x_bram[ddr_read] = input[ddr_read];
#else
		GMEM_AP_TYPE in_read = *(input+ddr_read);
		x_bram[ddr_read.range(1,0)][ddr_read.range(15,2)] = in_read;
#endif
	}

#if 0//!__SYNTHESIS__
	for(int i=0; i< 2;i++)
	{
		fprintf(stderr,"\n");
		ap_uint<512> ww = x_bram[i];
		for(ap_uint<16> word=0, bit=0; word<8; word++,bit+=16)
		{
			short wr = ww.range(15+bit, bit);
			fprintf(stderr,"\t%d", wr);
		}
		fprintf(stderr,"\n");
		for(ap_uint<16> word=0, bit=0; word<8; word++,bit+=16)
		{
			short wr = ww.range(15+bit, bit);
			fprintf(stderr,"\t%f", (float)wr/(float)(1<<2));
		}
		fprintf(stderr,"\n");
	}
#endif

	Read_Y:
	for(ap_uint<16> i = 0; i < _m/8; i++)
	{
#pragma HLS loop_tripcount min=Morder_TC max=Morder_TC
#pragma HLS pipeline II=1

		ap_uint<128> read_bias = bias[i];

#if FP_ENABLE

		for(ap_uint<8> dim = 0, bit = 0; dim<8; dim++, bit+=16)
		{
#pragma HLS unroll
			short val = read_bias.range(15+bit,bit);
			int to_y_bram;
			if(shift_in > 0){
				to_y_bram = ((int)val) >> shift_in;
			}else{
				to_y_bram = ((int)val) << shift_in_neg;
			}

			y_bram[dim][i] = to_y_bram;
		}
#else
		y_bram[0][i] = val;
#endif//FP_ENABLE

	}

	ap_uint<32> loopcnt = _m*n_by_16;

	ap_uint<16> x_idx = 0;
	ap_uint<16> y_idx = 0;

	//GMEM_AP_TYPE input_vector1, input_vector2;

	int res1 = y_bram[0][0];

	Row_Loop:
#if WEIGHT_8BIT_EN==1
	for(ap_uint<32> rows = 0; rows < loopcnt/4; rows++)
#else
		for(int rows = 0; rows < loopcnt; rows++)
#endif
		{
#pragma HLS loop_tripcount min=TC max=TC
#pragma HLS pipeline II=1
#pragma HLS DEPENDENCE variable=y_bram inter false
#pragma HLS DEPENDENCE variable=y_bram intra false

			ap_uint<128> read1_wts = weights1[rows];
			ap_uint<128> read2_wts = weights2[rows];
			ap_uint<64> wts_in1 = read1_wts.range(63,0);
			ap_uint<64> wts_in2 = read1_wts.range(127,64);
			ap_uint<64> wts_in3 = read2_wts.range(63,0);
			ap_uint<64> wts_in4 = read2_wts.range(127,64);

			int dot_out[4];
			// Process function
#if SP_ENABLE
			dot16<IN_BUS_WIDTH, IN_BUS_SHIFT_CNT>(A_bram[0],  x_bram[x_idx],   &res);
#elif HP_ENABLE
			dot32<IN_BUS_WIDTH, IN_BUS_SHIFT_CNT>(A_bram[0],  x_bram[x_idx],   &res);
#elif FP_ENABLE
			//			dot32_fxdpt<IN_BUS_WIDTH, IN_BUS_SHIFT_CNT>(A_bram[0], x_bram[x_idx], &res);
			dot8_fxdpt(wts_in1, x_bram[0][x_idx], &dot_out[0]);
			dot8_fxdpt(wts_in2, x_bram[1][x_idx], &dot_out[1]);
			dot8_fxdpt(wts_in3, x_bram[2][x_idx], &dot_out[2]);
			dot8_fxdpt(wts_in4, x_bram[3][x_idx], &dot_out[3]);
#endif
			int dot_add1 = dot_out[0] + dot_out[1];
			int dot_add2 = dot_out[2] + dot_out[3];
			int res = dot_add1 + dot_add2;
			res1+= res;
			x_idx++;
			if(x_idx==(n_by_16/4))
			{
				x_idx = 0;
				y_bram[y_idx.range(2,0)][y_idx.range(15,3)] = res1;
				y_idx++;
				res1 = y_bram[y_idx.range(2,0)][y_idx.range(15,3)];
			}
		}

#if FP_ENABLE
	int add_l1[6], add_l2[3], add_l3, f_res;
#else
	data_t add_l1[6], add_l2[3], add_l3, f_res;
#endif

	Write_Y:
	for(ap_uint<16> i = 0; i < _m/8; i++)
	{
#pragma HLS loop_tripcount min=Morder_TC max=Morder_TC
#pragma HLS pipeline II=1
#if 0
		for(int j = 0; j < 6; j++)
		{
#pragma HLS unroll
			add_l1[j] = y_bram[2*j][i] + y_bram[2*j+1][i];
		}

		for(int j = 0; j < 3; j++)
		{
#pragma HLS unroll
			add_l2[j] = add_l1[2*j] + add_l1[2*j+1];
		}
#endif

		ap_uint<128> out_write;
		for(ap_uint<8> dim = 0, bit = 0; dim<8; dim++, bit+=16)
		{
#pragma HLS unroll
			add_l3 = y_bram[dim][i];

			if(relu && add_l3 < 0)
				f_res = 0;
			else
				f_res = add_l3;


#if FP_ENABLE
			int to_output;
			if(shift_out > 0)
				to_output = (f_res >> shift_out);
			else
				to_output = (f_res << shift_out_neg);

			out_write.range(bit+15, bit) = to_output;
		}
		output[i] = out_write;
		//fprintf(stderr, "\nDDR_op: [%d] actual:send = %f:%f, shift>>%d", i, (float)add_l3/(float)(1 << (10)), (float)to_output/(float)(1 << (6)), shift_out);
#else
		output[i] = (data_t)f_res;
#endif
	}

}

#endif // _XI_SGEMV_HPP_
