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

#ifndef _XI_DNN_CONV_UTILS_H_
#define _XI_DNN_CONV_UTILS_H_

#include "../include/hw_settings.h"

//# Parallel mac per pixel
#define MAC_PIX_PROC (KER_PROC >> 1)

#define FCN            0
#define XNET		   0

#define DECONV_FLOAT   0
#define WEIGHTS_8B     1

#define	DP_ENABLE		1

#if WEIGHTS_8B
#define CONV_WT_TYPE         signed char
#define CONVFC_WT_TYPE       short
#else
#define CONV_WT_TYPE         short
#endif

#define XI_INPUTPACKCOUNT_LOG2		2
#define XI_INPUTPACKCOUNT2_LOG2		3
#define XI_WEIGHTPACKCOUNT_LOG2		6
#define XI_OUTPUTPACKCOUNT_LOG2		3
#define XI_BIASPACKCOUNT_LOG2		2

#if 0
#if KER_PROC==8 || KER_PROC==16
#if !SINGLE_IO_PORT
//#define XI_ROW8 1
//#define XI_ROW4 0
//#define XI_ROW2 0
#if PORT_BITWIDTH_64BIT
#define XI_ISTAGEBUFF_DEPTH 1024
#define XI_OSTAGEBUFF_DEPTH 1024
#define XI_WEIGHTBUFF_DEPTH 1024
#define XI_OSTG_BUFFER_SET 	4
#else
#define XI_ISTAGEBUFF_DEPTH 8192
#define XI_OSTAGEBUFF_DEPTH 2048
#define XI_WEIGHTBUFF_DEPTH 2048
#define XI_OSTG_BUFFER_SET  8
#endif  //#if PORT_BITWIDTH_64BIT
#else
#define XI_ISTAGEBUFF_DEPTH 1024//*******************1024
#define XI_OSTAGEBUFF_DEPTH 1024//*******************1024
#define XI_WEIGHTBUFF_DEPTH 1024
#define XI_OSTG_BUFFER_SET  4
#endif  //#if !SINGLE_IO_PORT
#elif KER_PROC==4
//#define XI_ROW8 0
//#define XI_ROW4 0
//#define XI_ROW2 1
#define XI_ISTAGEBUFF_DEPTH 1024
#define XI_OSTAGEBUFF_DEPTH 1024
#define XI_WEIGHTBUFF_DEPTH 2048
#elif KER_PROC==1
//#define XI_ROW8 0
//#define XI_ROW4 0
//#define XI_ROW2 1
#define XI_ISTAGEBUFF_DEPTH 1024
#define XI_OSTAGEBUFF_DEPTH 1024
#define XI_WEIGHTBUFF_DEPTH 4096//2048//1024
#endif//KER_PROC
#endif

#define XI_ROW8 (XI_ISTAGEBUFF_DEPTH>=4096)
#define XI_ROW4 (XI_ISTAGEBUFF_DEPTH>=2048) & (XI_ISTAGEBUFF_DEPTH<4096)
#define XI_ROW2 (XI_ISTAGEBUFF_DEPTH<2048)
#define FEEDING_BUFF_DEPTH 1024

//
///////////// END //////////////////

#endif //_XI_DNN_CONV_UTILS_H_
