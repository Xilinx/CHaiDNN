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

#define FCN            0
#define XNET		   0

#define DECONV_FLOAT   0
#define WEIGHTS_8B     1

/////////////////////////////////////
//***** CONV DESIGN PARAMETER
#define PIX_PROC 32
#define KER_PROC 8
#define PIX_PROC_LUT 0

#if WEIGHTS_8B
#define CONV_WT_TYPE         char
#else
#define CONV_WT_TYPE         short
#endif


#if KER_PROC==8
#define ROW8 1
#define ROW4 0
#define ROW2 0
#define ISTG_BUFF_DEPTH 6144
#define OSTG_BUFF_DEPTH 4096
#define KBUFF_DEPTH 	2048
#elif KER_PROC==4
#define ROW8 0
#define ROW4 0
#define ROW2 1
#define ISTG_BUFF_DEPTH 1024
#define OSTG_BUFF_DEPTH 1024
#define KBUFF_DEPTH 2048
#elif KER_PROC==1
#define ROW8 0
#define ROW4 0
#define ROW2 1
#define ISTG_BUFF_DEPTH 1024
#define OSTG_BUFF_DEPTH 1024
#define KBUFF_DEPTH 4096//2048//1024
#endif//KER_PROC

//
///////////// END //////////////////

#endif //_XI_DNN_CONV_UTILS_H_
