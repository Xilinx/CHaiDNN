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

#ifndef _XI_SGEMV_CONFIG_H_
#define _XI_SGEMV_CONFIG_H_

#include <string.h>
#include "ap_int.h"

#ifdef __SDSOC
#define GMEM_TYPE ap_uint<128>
#define data_t short

#define NPC1 2
#pragma SDS data zero_copy(weights1[0:4096*9216>>NPC1])
#pragma SDS data zero_copy(weights2[0:4096*9216>>NPC1])
#pragma SDS data zero_copy(input[0:4096>>NPC1])
#pragma SDS data zero_copy(bias[0:9216])
#pragma SDS data zero_copy(output[0:4096])
#pragma SDS data zero_copy(scalar_gemv_args[0:32])

#pragma SDS data sys_port(weights1:ps_e_S_AXI_HP0_FPD)
#pragma SDS data sys_port(weights2:ps_e_S_AXI_HP1_FPD)
#pragma SDS data sys_port(input:ps_e_S_AXI_HP0_FPD)
#pragma SDS data sys_port(bias:ps_e_S_AXI_HP0_FPD)
#pragma SDS data sys_port(output:ps_e_S_AXI_HP0_FPD)

#define IN_NUM_BYTES 4*2
#define OUT_NUM_BYTES 4

//## Top function declaration
void xiSgemvTop(
		GMEM_TYPE * weights1,
		GMEM_TYPE * weights2,
		GMEM_TYPE * input,
		GMEM_TYPE * bias,
		GMEM_TYPE * output,
		int *scalar_gemv_args);
#endif
#define FP_ENABLE 1

#define FILEIO 1

//# Set the fully connected layer
#define FC6 1
#define FC7 0
#define FC8 0
#if FC6
#define MORDER 64
#define NORDER 64
#define KORDER 1
#elif FC7
#define MORDER 4096
#define NORDER 4096
#define KORDER 1
#else
#define MORDER 1024
#define NORDER 4096
#define KORDER 1
#endif

#define ERROR_THRESHOLD 0.5f

//## Type of gmem
//#define GMEM_TYPE ap_uint<64>

//#define SP_ENABLE 0
//#define HP_ENABLE 0
//#define FP_ENABLE 1

#if SP_ENABLE
    #define GMEM_ELEMENT 16
 	 #define data_t float
	#define MORDER_TRIP (MORDER)
	#define NORDER_TRIP (NORDER>>4)
	#define TOT_TRIP	(MORDER*(NORDER>>4))
#endif

#if HP_ENABLE
    #define GMEM_ELEMENT 32
	#include <hls_half.h>
 	 #define data_t half
	#define MORDER_TRIP (MORDER)
	#define NORDER_TRIP (NORDER>>5)
	#define TOT_TRIP	(MORDER*(NORDER>>5))
#endif

typedef unsigned int uint32_t;

//##Enable for Assertion
#define _ASSERT_ 0

//##Enable this for simulation purpose
#define _DEBUG_ 0
#define _DEBUG_tb_ 0

#define WEIGHT_8BIT_EN 1
#if WEIGHT_8BIT_EN
#define ACCUMULATOR_PRECISION 0
#else
#define ACCUMULATOR_PRECISION 7
#endif
//## Type of gmem

#define GMEM_TYPE ap_uint<128>//unsigned long long int
#define GMEM_GEMV_WTTYPE unsigned long long int
#define GMEM_AP_TYPE ap_uint<128>
#if WEIGHT_8BIT_EN
	#define TYPE_INPUT_BUFF ap_uint<128>
#else
	#define TYPE_INPUT_BUFF ap_uint<64>
#endif
//#if SDSOC
#if FP_ENABLE
    #define GMEM_ELEMENT 4
    #define GMEM_SHIFT_CNT 2
    #define LOG2_GMEM_ELEMENT 2
 	#define data_t short
 	#define data_update_t int
	#define MORDER_TRIP1 MORDER/4096
	#define NORDER_TRIP1 NORDER/4096
	#define MORDER_TRIP2 4096
	#define NORDER_TRIP2 4096/GMEM_ELEMENT
	//#define TOT_TRIP	(MORDER*(NORDER>>LOG2_GMEM_ELEMENT))
//#define BRAM_SIZE 4096/GMEM_ELEMENT
#define OUT_SIZE  1024
#define Y_BRAM_TYPE int
#define Y_BRAM_CNT 1//12

#define GMEM_ELEMENT_BY2 2

#define MORDER_TRIP (MORDER)
#define NORDER_TRIP (NORDER>>3)
#define TOT_TRIP	(MORDER*(NORDER>>5))

#endif

typedef unsigned int uint32_t;

#endif//_XI_SGEMV_CONFIG_H_


