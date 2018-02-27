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

#include <ap_int.h>
#include <string.h>

//# Debug flag
#define DEBUG			0

//# Some FILE Handlers for debug
FILE *iFp = NULL, *oFp = NULL, *rFp = NULL, *mpFp = NULL;

#ifndef __SYNTHESIS__
#define FILE_OUTPUT	0
#else
#define FILE_OUTPUT	0
#endif

FILE * pFp = NULL, *wFp = NULL, *apFp = NULL;

#include "../include/au_pool_2xio_config.h"
#include "../src/pool_dp_2xio.hpp"

void PoolTop(GMEM_MAXPOOLTYPE *in1, GMEM_MAXPOOLTYPE *in2, GMEM_MAXPOOLTYPE *out1, GMEM_MAXPOOLTYPE *out2, int *scalar_pool_args)
{
#if 0
#pragma HLS interface m_axi port=in1       			offset=slave bundle=gmem0 depth=160000
#pragma HLS interface m_axi port=out1				offset=slave bundle=gmem1 depth=40000
#pragma HLS interface m_axi port=in2    	   		offset=slave bundle=gmem2 depth=160000
#pragma HLS interface m_axi port=out2				offset=slave bundle=gmem3 depth=40000
#pragma HLS interface m_axi port=scalar_pool_args	offset=slave bundle=gmem4 depth=40000
#pragma HLS interface s_axilite port=poolin1			bundle=control
#pragma HLS interface s_axilite port=poolout1 			bundle=control
#pragma HLS interface s_axilite port=poolin2			bundle=control
#pragma HLS interface s_axilite port=poolout2 			bundle=control
#pragma HLS interface s_axilite port=scalar_pool_args 	bundle=control
#endif

	short in_h        	= (short)scalar_pool_args[0];
	short in_w        	= (short)scalar_pool_args[1];
	short out_h       	= (short)scalar_pool_args[2];
	short out_w      	= (short)scalar_pool_args[3];
	short n_planes    	= (short)scalar_pool_args[4];
	short ps_h	  	    = (short)scalar_pool_args[5];
	short ps_w	  	    = (short)scalar_pool_args[6];
	short pwin_h	  	= (short)scalar_pool_args[7];
	short pwin_w	  	= (short)scalar_pool_args[8];
	unsigned char avg_pool	= (unsigned char)scalar_pool_args[9];
	unsigned char pad	    = (unsigned char)scalar_pool_args[10];
	unsigned char one_by_diviser	= (unsigned char)scalar_pool_args[11];
#if DEBUG
	fprintf(stderr, "pool-ker args : ");
	for(int i = 0; i < 12; i++)
	{
		fprintf(stderr, "%d ", scalar_pool_args[i]);
	}
	fprintf(stderr, "\n");

	fprintf(stderr, "maxp-args : ");
	fprintf(stderr, "%d %d %d %d %d %d %d %d %d %d %d %d\n",  in_h,  in_w,  out_h,   out_w,   n_planes,
			ps_h,  ps_w,  pwin_h,  pwin_w, avg_pool,
			pad,  one_by_diviser);
#endif

	//# Call Kernel
	poolKernel(in1, in2, out1, out2, in_h, in_w, out_h, out_w, n_planes, ps_h, ps_w, pwin_h, pwin_w, avg_pool, pad, one_by_diviser);
	//return 0;
}
