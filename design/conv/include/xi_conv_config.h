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

//#define XI_DIET_CHAI_Z 		0 //Enable this Macro to reduce the resource utilization of the design to fit to smaller devices
//#define XI_DIET_CHAI_ZUPLUS	0 //Enable this Macro to reduce the resource utilization of the design to fit to smaller devices


//**** URAM ENABLE FLAGS FOR THE BUFFERS
#define XI_BIAS_URAM_EN 	0
#define XI_WTS_URAM_EN 		0
#define XI_ISTG_URAM_EN 	0
#define XI_OSTG_URAM_EN 	0
#define XI_FEED_URAM_EN 	0
#define XI_SCALE_URAM_EN 	0

//**** CONFIGURABLE MACROS
#if XI_DIET_CHAI_Z || XI_DIET_CHAI_ZUPLUS
#define XI_KER_PROC       	8
#define XI_PIX_PROC       	8
#define XI_ISTAGEBUFF_DEPTH 1024
#define XI_OSTAGEBUFF_DEPTH 1024
#define XI_WEIGHTBUFF_DEPTH 1024
#else
#define XI_KER_PROC       	16
#define XI_PIX_PROC       	4
#define XI_ISTAGEBUFF_DEPTH 8192
#define XI_OSTAGEBUFF_DEPTH 2048
#define XI_WEIGHTBUFF_DEPTH 2048	
#endif

//**DON'T MODIFY THE BELOW MACROS**//
#if XI_DIET_CHAI_Z
#define XI_64BIT_PORT_EN    	1 
#define XI_POOL_PROC_8_PLN 		1
#define XI_DISABLE_BN 			1
#define XI_OSTG_BUFFER_SET 		4
#define XI_SINGLE_IO_PORT_EN 	0
#elif XI_DIET_CHAI_ZUPLUS
#define XI_64BIT_PORT_EN    	0 
#define XI_POOL_PROC_8_PLN 		1
#define XI_DISABLE_BN 			1
#define XI_OSTG_BUFFER_SET 		4
#define XI_SINGLE_IO_PORT_EN 	1
#else
#define XI_64BIT_PORT_EN    	0
#define XI_POOL_PROC_8_PLN 		0
#define XI_DISABLE_BN 			0
#define XI_OSTG_BUFFER_SET 		8
#define XI_SINGLE_IO_PORT_EN 	0
#endif

#define XI_DP_ENABLE 			1
#define XI_DISABLE_SOS 			1
#define XI_ODBC_EN 				0
#define XI_RESULT_BUFFER_FF 	0
#define XI_RESULT_BUFFER_LUTRAM 0
#define XI_BATCH1_EN 	  		0
#define XI_MEANSUB_EN 			0

#if XI_DIET_CHAI_Z ||XI_DIET_CHAI_ZUPLUS || XI_WTS_URAM_EN
#define XI_MERGE_WEIGHTBUFF		1
#else
#define XI_MERGE_WEIGHTBUFF		0
#endif	

#if XI_64BIT_PORT_EN
#define XI_IO_64bit_PORT_EN 1
#define XI_WTS_PORT_64BIT_EN 1
#else
#define XI_IO_64bit_PORT_EN 0
#define XI_WTS_PORT_64BIT_EN 0	
#endif

#define PIX_PROC_ODD 	  	(XI_PIX_PROC%2==1)
#if !XI_SINGLE_IO_PORT_EN
#define LOG2_NUM_PORT_IO 1
#else
#define LOG2_NUM_PORT_IO 0
#endif

#if !XI_SINGLE_IO_PORT_EN && !XI_IO_64bit_PORT_EN
#define OUT_PARTITION 4
#else
#define OUT_PARTITION 2
#endif

#define XI_ROW8 (XI_ISTAGEBUFF_DEPTH>=4096)
#define XI_ROW4 (XI_ISTAGEBUFF_DEPTH>=2048) & (XI_ISTAGEBUFF_DEPTH<4096)
#define XI_ROW2 (XI_ISTAGEBUFF_DEPTH<2048)

#if (XI_ISTAGEBUFF_DEPTH<=1024)
typedef ap_uint<10> istg_datatype;
#elif (XI_ISTAGEBUFF_DEPTH<=2048)
typedef ap_uint<11> istg_datatype;
#elif (XI_ISTAGEBUFF_DEPTH<=4096)
typedef ap_uint<12> istg_datatype;
#elif (XI_ISTAGEBUFF_DEPTH<=8192)
typedef ap_uint<13> istg_datatype;
#endif

///******END********
#define __SDSOC 1

#ifdef __SDSOC
#define GMEM_ELEMENT   16//4
#define GMEM_SHIFT_CNT 4//2

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
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
#pragma SDS data zero_copy(weights3[0:(WEIGHT_SIZE)>>NPC1])
#pragma SDS data zero_copy(weights4[0:(WEIGHT_SIZE)>>NPC1])
#endif
#pragma SDS data zero_copy(bias[0:(BIAS_SIZE)>>NPC2])
#pragma SDS data zero_copy(input_other1[0:(INPUT_SIZE)>>NPC1])
#if !XI_SINGLE_IO_PORT_EN
#pragma SDS data zero_copy(input_other2[0:(INPUT_SIZE)>>NPC1])
#endif
#pragma SDS data zero_copy(input_1st[0:(INPUT_SIZE)>>NPC2])
#pragma SDS data zero_copy(output1[0:(OUTPUT_SIZE)>>NPC1])
#if !XI_SINGLE_IO_PORT_EN
#pragma SDS data zero_copy(output2[0:(OUTPUT_SIZE)>>NPC1])
#endif
#if !XI_DISABLE_BN
#pragma SDS data zero_copy(inp_norm_2[0:(INPUT_SIZE)>>NPC2])
#pragma SDS data zero_copy(inp_norm_3[0:(INPUT_SIZE)>>NPC2])
#endif
#pragma SDS data zero_copy(istg_out1[0:(INPUT_SIZE)>>NPC2])
#if !XI_SINGLE_IO_PORT_EN
#pragma SDS data zero_copy(istg_out2[0:(INPUT_SIZE)>>NPC2])
#endif
#pragma SDS data zero_copy(scalar_conv_args[0:32])

#pragma SDS data sys_port(weights1:ps_e_S_AXI_HP0_FPD)
#pragma SDS data sys_port(weights2:ps_e_S_AXI_HP1_FPD)
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8) )
#pragma SDS data sys_port(weights3:ps_e_S_AXI_HP2_FPD)
#pragma SDS data sys_port(weights4:ps_e_S_AXI_HP3_FPD)
#endif
#pragma SDS data sys_port(bias:ps_e_S_AXI_HP0_FPD)
#pragma SDS data sys_port(input_other1:ps_e_S_AXI_HP0_FPD)
#if !XI_SINGLE_IO_PORT_EN
#pragma SDS data sys_port(input_other2:ps_e_S_AXI_HP1_FPD)
#endif
#pragma SDS data sys_port(input_1st:ps_e_S_AXI_HP1_FPD)
#pragma SDS data sys_port(output1:ps_e_S_AXI_HP0_FPD)
#if !XI_SINGLE_IO_PORT_EN
#pragma SDS data sys_port(output2:ps_e_S_AXI_HP1_FPD)
#endif
#if !XI_DISABLE_BN
#pragma SDS data sys_port(inp_norm_2:ps_e_S_AXI_HP2_FPD)
#pragma SDS data sys_port(inp_norm_3:ps_e_S_AXI_HP3_FPD)
#endif
#pragma SDS data sys_port(istg_out1:ps_e_S_AXI_HP1_FPD)
#if !XI_SINGLE_IO_PORT_EN
#pragma SDS data sys_port(istg_out2:ps_e_S_AXI_HP0_FPD)
#endif
#pragma SDS data sys_port(scalar_conv_args:ps_e_S_AXI_HP0_FPD)



typedef unsigned long long int gmem_inputtype_layer1;
#if XI_IO_64bit_PORT_EN
typedef ap_uint<64> gmem_inputtype_layerx;
#else
typedef ap_uint<128> gmem_inputtype_layerx;
#endif
typedef  unsigned long long int gmem_biastype;

#if XI_IO_64bit_PORT_EN
typedef  ap_uint<64> gmem_outputtype;
#else
typedef  ap_uint<128> gmem_outputtype;
#endif

typedef ap_int<8> weight_width;

typedef ap_uint<64> inputtype ;
#define XI_INPUTPACKCOUNT_LOG2		2
#if XI_IO_64bit_PORT_EN
typedef ap_uint<64> inputtype2 ;
#define XI_INPUTPACKCOUNT2_LOG2		3 - LOG2_NUM_PORT_IO
#else
typedef ap_uint<128> inputtype2 ;
#define XI_INPUTPACKCOUNT2_LOG2		4 - LOG2_NUM_PORT_IO
#endif

#if XI_KER_PROC ==8

#if XI_WTS_PORT_64BIT_EN
typedef  ap_uint<64>  gmem_weighttype;
#else
typedef  ap_uint<128>  gmem_weighttype;
#endif
#define XI_WEIGHTPACKCOUNT_LOG2		5

#else//16 KP

typedef  ap_uint<128>  gmem_weighttype;
#define XI_WEIGHTPACKCOUNT_LOG2		6

#endif//KP

typedef ap_uint<64> biastype ;
#define XI_BIASMAXSIZE			1024//512
#define XI_BIASPACKCOUNT_LOG2		2
#if XI_IO_64bit_PORT_EN
typedef ap_uint<64> outtype ;
#define XI_OUTPUTPACKCOUNT_LOG2		3 - LOG2_NUM_PORT_IO
#else
typedef ap_uint<128> outtype ;
#define XI_OUTPUTPACKCOUNT_LOG2		4 - LOG2_NUM_PORT_IO
#endif

#ifdef __HLS_SYN__
#define __SDSVHLS__
#endif
void XiConvolutionTop(				gmem_weighttype *weights1,
		gmem_weighttype *weights2,
#if (XI_KER_PROC==16 || (XI_WTS_PORT_64BIT_EN==1 && XI_KER_PROC==8))
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
		gmem_inputtype_layer1 *inp_norm_2,
		gmem_inputtype_layer1 *inp_norm_3,
#endif
		gmem_inputtype_layer1 *istg_out1,
#if !XI_SINGLE_IO_PORT_EN
		gmem_inputtype_layer1 *istg_out2,
#endif
		int *scalar_conv_args
#ifdef __SDSVHLS__
		, bool ap_clk_div2
#endif
      );
#else
	//TODO
#endif

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
