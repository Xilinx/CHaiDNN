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

#ifndef _XCHANGE_NORM_UTILS_HPP_
#define _XCHANGE_NORM_UTILS_HPP_

#include "../include/hw_settings.h"

int loadNormGamma(const float *gamma_buf, int nElems, IO_DATA_TYPE *ptrRetArray, int fbits_gamma)
{
#if 0
#if DATA_IN_BINARY
	FILE *fp_gamma = fopen(path, "rb");
#else
	FILE *fp_gamma = fopen(path, "r");
#endif
#endif
	float val;
	nElems = AlignSize(nElems, KER_PROC);
#if 0
	if(fp_gamma == NULL)
	{
		fprintf(stderr, "\n** Error : Cannot open the file : %s **\n", path);
		//exit(-1);
	}

#if DATA_IN_BINARY
	float *gamma_buf = (float *)malloc(nElems*sizeof(float));
	fread(gamma_buf, sizeof(float)*nElems, 1, fp_gamma);
#endif
#endif
	float f_val;
	for(int i = 0; i < nElems; i++)
	{
#if 0
#if DATA_IN_BINARY
		f_val = gamma_buf[i];
#else
		fscanf(fp_gamma, "%f ", &f_val);
#endif
#endif
		f_val = gamma_buf[i];
		IO_DATA_TYPE ival = (IO_DATA_TYPE)f_val;
		IO_DATA_TYPE fxval = ConvertToFP(f_val, ival, fbits_gamma);
		ptrRetArray[i] = fxval;
	}

#if DATA_IN_BINARY
	free(gamma_buf);
#endif

	//fclose(fp_gamma);
	return 0;
}


int loadNormGamma_float(const float *gamma_buf, int nElems, IO_DATA_TYPE1 *ptrRetArray, int fbits_gamma)
{
#if 0
#if DATA_IN_BINARY
	FILE *fp_gamma = fopen(path, "rb");
#else
	FILE *fp_gamma = fopen(path, "r");
#endif
#endif
	float val;
	nElems = AlignSize(nElems, KER_PROC);
#if 0
	if(fp_gamma == NULL)
	{
		fprintf(stderr, "\n** Error : Cannot open the file : %s **\n", path);
		//exit(-1);
	}

#if DATA_IN_BINARY
	float *gamma_buf = (float *)malloc(nElems*sizeof(float));
	fread(gamma_buf, sizeof(float)*nElems, 1, fp_gamma);
#endif
#endif
	float f_val;
	for(int i = 0; i < nElems; i++)
	{
#if 0
#if DATA_IN_BINARY
		f_val = gamma_buf[i];
#else
		fscanf(fp_gamma, "%f ", &f_val);
#endif
#endif
		f_val = gamma_buf[i];
		//IO_DATA_TYPE ival = (IO_DATA_TYPE)f_val;
		//IO_DATA_TYPE fxval = ConvertToFP(f_val, ival, fbits_gamma);
		ptrRetArray[i] = f_val;///sf_out;
	}

#if DATA_IN_BINARY
	free(gamma_buf);
#endif

	//fclose(fp_gamma);
	return 0;
}

#endif //_XCHANGE_NORM_UTILS_HPP_
