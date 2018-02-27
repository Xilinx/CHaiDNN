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

#ifndef _XI_DECOV_SDSOC_H_
#define _XI_DECOV_SDSOC_H_


#if DECONV_FLOAT
#define INTYPE_FLOAT      float
#define WTYPE             float
#else
#define INTYPE_FLOAT      short
#define WTYPE             short
#endif

#define GMEM_DECONVTYPE   unsigned long long int
#define OUTIDTYPE         unsigned long long int

#define GMEM_ELEMENT   4
#define GMEM_SHIFT_CNT 2

#define NPC1 GMEM_SHIFT_CNT

#define DECONV_IN_SIZE 2*32*32
#define DECONV_WT_SIZE 2*32*32
#define DECONV_BIAS_SIZE 2
#define DECONV_OUTPUT_SIZE 1*512*512

/*#pragma SDS data data_mover(deconvIN:AXIDMA_SIMPLE)
#pragma SDS data data_mover(deconvWT:AXIDMA_SIMPLE)
#pragma SDS data data_mover(deconvBias:AXIDMA_SIMPLE)
#pragma SDS data data_mover(deconvIDout:AXIDMA_SIMPLE)
#pragma SDS data data_mover(scalar_deconv_args:AXIDMA_SIMPLE)

#pragma SDS data access_pattern(deconvIN:SEQUENTIAL)
#pragma SDS data access_pattern(deconvWT:SEQUENTIAL)
#pragma SDS data access_pattern(deconvBias:SEQUENTIAL)
#pragma SDS data access_pattern(deconvIDout:SEQUENTIAL)
#pragma SDS data access_pattern(scalar_deconv_args:SEQUENTIAL)*/

#pragma SDS data zero_copy(deconvIN[0:DECONV_IN_SIZE])
#pragma SDS data zero_copy(deconvWT[0:DECONV_WT_SIZE])
#pragma SDS data zero_copy(deconvBias[0:DECONV_BIAS_SIZE])
#pragma SDS data zero_copy(deconvIDout[0:(DECONV_OUTPUT_SIZE)>>1])
#pragma SDS data zero_copy(scalar_deconv_args[0:32])

int DeconvTop(INTYPE_FLOAT* deconvIN, WTYPE* deconvWT, WTYPE* deconvBias, GMEM_DECONVTYPE* deconvIDout, int *scalar_deconv_args);

#endif // _XI_DECOV_SDSOC_H_
