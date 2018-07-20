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

#include "maxpool_config.h"
#include "pool_dp_2xio.hpp"

#ifndef __SDSOC
int PoolTop(GMEM_MAXPOOLTYPE *in1, GMEM_MAXPOOLTYPE *in2, GMEM_MAXPOOLTYPE *out1, GMEM_MAXPOOLTYPE *out2, GMEM_POOLTYPE_WTS * wts, int *scalar_pool_args)
#else
void PoolTop(GMEM_MAXPOOLTYPE *in1, GMEM_MAXPOOLTYPE *in2, GMEM_MAXPOOLTYPE *out1, GMEM_MAXPOOLTYPE *out2, GMEM_POOLTYPE_WTS * wts, int *scalar_pool_args)
#endif
{
#ifndef __SDSOC
#pragma HLS interface m_axi port=in1       		offset=slave bundle=gmem0 depth=115200
#pragma HLS interface m_axi port=out1			offset=slave bundle=gmem1 depth=115200
#pragma HLS interface m_axi port=in2       		offset=slave bundle=gmem2 depth=115200
#pragma HLS interface m_axi port=out2			offset=slave bundle=gmem3 depth=115200
#pragma HLS interface m_axi port=wts			offset=slave bundle=gmem4 depth=36
#pragma HLS interface m_axi port=scalar_pool_args	offset=slave bundle=gmem5 depth=32
#pragma HLS interface s_axilite port=in1		bundle=control
#pragma HLS interface s_axilite port=out1 		bundle=control
#pragma HLS interface s_axilite port=in2		bundle=control
#pragma HLS interface s_axilite port=out2 		bundle=control
#pragma HLS interface s_axilite port=wts 		bundle=control
#pragma HLS interface s_axilite port=scalar_pool_args 	bundle=control
#pragma HLS interface s_axilite port=return 	bundle=control
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
	unsigned char conv3ds	= (unsigned char)scalar_pool_args[12];
	unsigned char relu		= (unsigned char)scalar_pool_args[13];
	unsigned char outshift	= (unsigned char)scalar_pool_args[14];
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
	poolKernel(in1, in2, out1, out2, wts, in_h, in_w, out_h, out_w, n_planes, ps_h, ps_w, pwin_h, pwin_w, avg_pool, pad, one_by_diviser,
			conv3ds, relu, outshift);
	#ifndef __SDSOC
	return 0;
	#endif
}
