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

#ifndef _AU_MAXPOOL_CONFIG_H_
#define _AU_MAXPOOL_CONFIG_H_

#define GMEM_MAXPOOLTYPE   ap_uint<128>

#define GMEM_ELEMENT   4
#define GMEM_SHIFT_CNT 2

#define NPC1 GMEM_SHIFT_CNT

#define NUM_KERNELS 8
#define OUTWIDTH	480
#define OUTHEIGHT	480

#define OUTPUT_SIZE NUM_KERNELS*OUTWIDTH*OUTHEIGHT

//# Data Copy SDS pragmas
#pragma SDS data zero_copy(in1[0:(OUTPUT_SIZE)>>NPC1])
#pragma SDS data zero_copy(out1[0:(OUTPUT_SIZE)>>NPC1])
#pragma SDS data zero_copy(in2[0:(OUTPUT_SIZE)>>NPC1])
#pragma SDS data zero_copy(out2[0:(OUTPUT_SIZE)>>NPC1])
#pragma SDS data zero_copy(scalar_pool_args[0:16])

#pragma SDS data sys_port(in1:ps_e_S_AXI_HP0_FPD)
#pragma SDS data sys_port(in2:ps_e_S_AXI_HP1_FPD)
#pragma SDS data sys_port(out1:ps_e_S_AXI_HP2_FPD)
#pragma SDS data sys_port(out2:ps_e_S_AXI_HP3_FPD)

void PoolTop(GMEM_MAXPOOLTYPE *in1, GMEM_MAXPOOLTYPE *in2, GMEM_MAXPOOLTYPE *out1, GMEM_MAXPOOLTYPE *out2, int *scalar_pool_args);

#endif // _AU_MAXPOOL_CONFIG_H_
