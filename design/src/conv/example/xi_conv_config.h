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

#ifndef _XI_CONV_CONFIG_H_
#define _XI_CONV_CONFIG_H_

#include <ap_int.h>

//#include "au_common_sdsoc.h"

#define GMEM_INPUTTYPE	     unsigned long long int
#define GMEM_INTYPE_OTHER    ap_uint<128>//unsigned long long int
#define GMEM_WEIGHTTYPE      ap_uint<128>//unsigned long long int
#define GMEM_BIASTYPE        unsigned long long int
#define GMEM_OUTTYPE         ap_uint<128>//unsigned long long int


typedef unsigned long long int gmem_inputtype_layer1;
#if XI_INPT_64bit_PORT
typedef unsigned long long int gmem_inputtype_layerx;
#else
typedef ap_uint<128> gmem_inputtype_layerx;
#endif
typedef  unsigned long long int gmem_biastype;

#if XI_OTPT_64bit_PORT
typedef  unsigned long long int gmem_outputtype;
#else
typedef  ap_uint<128> gmem_outputtype;
#endif


#if 0//RUN_ON_SDSOC
#include "auvizdnn_conv_sdsoc.h"
#endif

#define XI_KER_PROC       8
#define XI_PIX_PROC       52
#define XI_PIX_ADDER      56
#define XI_PIX_PROC_LUT   0
#define XI_WEIGHTS_8B     1
#define XI_INPT_64bit_PORT	0 //Enable for 64bit port and disable for 128 bit port
#define XI_OTPT_64bit_PORT	0 //Enable for 64bit port and disable for 128 bit port
#define XI_MEAN_SUB 1
#define XI_BN_EN			1

#if XI_KER_PROC==8
typedef  ap_uint<128>  gmem_weighttype;
#elif XI_KER_PROC==1
typedef  ap_uint<32>  gmem_weighttype;
#else
typedef  ap_uint<128> gmem_weighttype;
#endif

#if XI_KER_PROC==8
#define XI_ROW8 1
#define XI_ROW4 0
#define XI_ROW2 0
#define XI_ISTAGEBUFF_DEPTH 6144
#define XI_OSTAGEBUFF_DEPTH 4096
#define XI_WEIGHTBUFF_DEPTH 2048
#define XI_OSTAGEBUFF_PIX 2//XI_OSTAGEBUFF_DEPTH/2048
#define XI_OSTAGEBUFF_DIM4 2048
#elif XI_KER_PROC==4
#define XI_ROW8 1
#define XI_ROW4 0
#define XI_ROW2 0
#define XI_ISTAGEBUFF_DEPTH 6144
#define XI_OSTAGEBUFF_DEPTH 4096
#define XI_WEIGHTBUFF_DEPTH 4096
#define XI_OSTAGEBUFF_PIX XI_OSTAGEBUFF_DEPTH/1024
#define XI_OSTAGEBUFF_DIM4 1024
#elif XI_KER_PROC==2
#define XI_ROW8 0
#define XI_ROW4 0
#define XI_ROW2 1
#define XI_ISTAGEBUFF_DEPTH 1024
#define XI_OSTAGEBUFF_DEPTH 1024
#define XI_WEIGHTBUFF_DEPTH 4096//2048//1024
#define XI_OSTAGEBUFF_PIX XI_OSTAGEBUFF_DEPTH/1024
#elif XI_KER_PROC==1
#define XI_ROW8 0
#define XI_ROW4 0
#define XI_ROW2 1
#define XI_ISTAGEBUFF_DEPTH 1024
#define XI_OSTAGEBUFF_DEPTH 1024
#define XI_WEIGHTBUFF_DEPTH 4096//2048//1024
#define XI_OSTAGEBUFF_PIX XI_OSTAGEBUFF_DEPTH/1024
#endif//XI_KER_PROC



#if XI_WEIGHTS_8B
typedef ap_int<8> weight_width;
#else
typedef short weight_width;
#endif

typedef ap_uint<64> inputtype ;
#define XI_INPUTPACKCOUNT_LOG2		2
#if XI_INPT_64bit_PORT
typedef ap_uint<64> inputtype2 ;
#define XI_INPUTPACKCOUNT2_LOG2		2
#else
typedef ap_uint<128> inputtype2 ;
#define XI_INPUTPACKCOUNT2_LOG2		3
#endif

#if XI_KER_PROC ==1
typedef ap_uint<32> weighttype ;
#define XI_WEIGHTPACKCOUNT_LOG2		2
#elif XI_KER_PROC ==8
typedef ap_uint<128> weighttype ;
#define XI_WEIGHTPACKCOUNT_LOG2		4
#else
#if XI_WEIGHTS_8B
typedef ap_uint<128> weighttype ;
#define XI_WEIGHTPACKCOUNT_LOG2		4
#else
typedef ap_uint<512> weighttype ;
#define XI_WEIGHTPACKCOUNT_LOG2		5
#endif
#endif
typedef ap_uint<64> biastype ;
#define XI_BIASMAXSIZE			4096//512
#define XI_BIASPACKCOUNT_LOG2		2
#if XI_OTPT_64bit_PORT
typedef ap_uint<64> outtype ;
#define XI_OUTPUTPACKCOUNT_LOG2		2
#else
typedef ap_uint<128> outtype ;
#define XI_OUTPUTPACKCOUNT_LOG2		3
#endif

#define GMEM_ELEMENT   8//4
#define GMEM_SHIFT_CNT 3//2

#define NPC1 GMEM_SHIFT_CNT
#define NPC2 2

#define INWIDTH 	480
#define INHEIGHT	480
#define INPUT_PLANES 8
#define FILTERSIZE	3
#define NUM_KERNELS 8
#define OUTWIDTH	480
#define OUTHEIGHT	480

#define WEIGHT_SIZE 128*128*(FILTERSIZE*FILTERSIZE+1)
#define BIAS_SIZE   512
#define INPUT_SIZE  INWIDTH*INHEIGHT*INPUT_PLANES
#define OUTPUT_SIZE NUM_KERNELS*OUTWIDTH*OUTHEIGHT


#pragma SDS data zero_copy(weights1[0:(WEIGHT_SIZE)>>NPC1])
#pragma SDS data zero_copy(weights2[0:(WEIGHT_SIZE)>>NPC1])
#pragma SDS data zero_copy(bias[0:(BIAS_SIZE)>>NPC2])
#pragma SDS data zero_copy(input_other1[0:(INPUT_SIZE)>>NPC1])
#pragma SDS data zero_copy(input_other2[0:(INPUT_SIZE)>>NPC1])
#pragma SDS data zero_copy(input_1st[0:(INPUT_SIZE)>>NPC2])
#pragma SDS data zero_copy(output1[0:(OUTPUT_SIZE)>>NPC1])
#pragma SDS data zero_copy(output2[0:(OUTPUT_SIZE)>>NPC1])
#pragma SDS data zero_copy(scalar_conv_args[0:40])
#pragma SDS data zero_copy(inp_norm_2[0:(BIAS_SIZE*2)>>NPC1])
#pragma SDS data zero_copy(inp_norm_3[0:(BIAS_SIZE*2)>>NPC1])
#pragma SDS data zero_copy(istg_out1[0:(INPUT_SIZE)>>NPC2])
#pragma SDS data zero_copy(istg_out2[0:(INPUT_SIZE)>>NPC2])


#pragma SDS data sys_port(weights1:ps_e_S_AXI_HP0_FPD)
#pragma SDS data sys_port(weights2:ps_e_S_AXI_HP1_FPD)
#pragma SDS data sys_port(bias:ps_e_S_AXI_HP2_FPD)
#pragma SDS data sys_port(input_other1:ps_e_S_AXI_HP2_FPD)
#pragma SDS data sys_port(input_other2:ps_e_S_AXI_HP3_FPD)
#pragma SDS data sys_port(input_1st:ps_e_S_AXI_HP2_FPD)
#pragma SDS data sys_port(output1:ps_e_S_AXI_HP2_FPD)
#pragma SDS data sys_port(output2:ps_e_S_AXI_HP3_FPD)
#pragma SDS data sys_port(inp_norm_2:ps_e_S_AXI_HP3_FPD)
#pragma SDS data sys_port(inp_norm_3:ps_e_S_AXI_HP3_FPD)
#pragma SDS data sys_port(istg_out1:ps_e_S_AXI_HP0_FPD)
#pragma SDS data sys_port(istg_out2:ps_e_S_AXI_HP1_FPD)


void XiConvolutionTop(GMEM_WEIGHTTYPE *weights1,
			 GMEM_WEIGHTTYPE *weights2,
			 GMEM_OUTTYPE *output1,
		     GMEM_OUTTYPE *output2,
		     GMEM_INTYPE_OTHER *input_other1,
		     GMEM_INTYPE_OTHER *input_other2,
		     GMEM_INPUTTYPE *input_1st,
		     GMEM_BIASTYPE *bias,
			 GMEM_INPUTTYPE *inp_norm_2,
			 GMEM_INPUTTYPE *inp_norm_3,
			 GMEM_INPUTTYPE *istg_out1,
			 GMEM_INPUTTYPE *istg_out2,
		     int *scalar_conv_args);

#if 1
#define XI_IN_H 227 //layer1
#define XI_IN_W 227 //layer1
#define XI_OUT_H 55 //layer1
#define XI_OUT_W 55 //layer1
#define XI_NUM_KERNELS 96
#define XI_FILTER_SIZE 11
#define XI_CONV_STRIDE 4
#define XI_POOL_STRIDE 2
#define XI_POOL_SIZE 3
#define XI_INPUT_PLANES 4
#define XI_NKPF 8//4
#define XI_PAD 0
#endif

#endif//_XI_CONV_CONFIG_H_
