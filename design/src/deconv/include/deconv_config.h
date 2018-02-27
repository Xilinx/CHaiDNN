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

#ifndef DECONVCONFIG_H
#define DECONVCONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ap_int.h>


#if RUN_ON_SDSOC
#include "xi_deconv_sdsoc.h"
#endif

typedef ap_uint<64> ap_uint64;

#define SIZEOF(x) (sizeof(x))
#define MAX(a,b) (a>=b?a:b)
#define MAXLOC(a,b,c,d) (a>=b?c:d)

//#include "au_conv_def.h"
//#if (SP_ENABLE == 1)
//#define DTYPE 0			// 0 - Single Precision, 1 - Half Precision, 2 - Fixed Point
//#elif (HP_ENABLE == 1)
//#define DTYPE 1			// 0 - Single Precision, 1 - Half Precision, 2 - Fixed Point
//#elif (FP_ENABLE == 1)
//#define DTYPE 2			// 0 - Single Precision, 1 - Half Precision, 2 - Fixed Point
//#endif

#if DECONV_FLOAT
#define DTYPE 0
#else
#define DTYPE 1
#endif

#if (DTYPE == 0)
#define INTYPE_FLOAT float
#define INTYPE float		// datatype of input port
#define WTYPE float			// datatype of weights port
#define OUTTYPE float		// datatype of output port

#define INBRAMTYPE float 	//	datatype of input buffer
#define WBRAMTYPE float		//	datatype of weight buffer
#define OUTBRAMTYPE float	//	datatype of output buffer

#elif (DTYPE == 2)

#define INTYPE half		// datatype of input port
#define WTYPE half			// datatype of weights port
#define OUTTYPE half		// datatype of output port

#define INBRAMTYPE half //	datatype of input buffer
#define WBRAMTYPE half		//	datatype of weight buffer
#define OUTBRAMTYPE half	//	datatype of output buffer

#elif (DTYPE == 1)

#define INTYPE_FLOAT short
#define INTYPE short		// datatype of input port
#define WTYPE short			// datatype of weights port
#define OUTTYPE short		// datatype of output port

#define INBRAMTYPE short //	datatype of input buffer
#define WBRAMTYPE short		//	datatype of weight buffer
#define OUTBRAMTYPE int	//	datatype of output buffer

#endif

#define OUTIDTYPE unsigned long long int	// datatype of output id

#define OUTI_AP_DTYPE ap_uint<64>	// datatype of output id
#define OUTIDPACKS 2		// Number of elems in the pack
#define IDTYPE int			// datatype of the argmax ID

#define NMAX 8			// Maximum number of input planes allowed
#define WIMAX 32		// Maximum size of the input
#define KMAX 64			// Maximum width/height of the filter
#define WOMAX 512		// Maximum width/height of the output // Typically 512, For Alex 1024

#define DECONV_Tn 8			// Process Tn planes in parallel
#define Tp 2			// Process Tp pixels/plane in parallel

#define MINUS_INFINITY -10000.0f		// A very large negative value
#define DBGARRAY 0

#endif
