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

#ifndef _XCHANGE_DECONV_UTILS_HPP_
#define _XCHANGE_DECONV_UTILS_HPP_

#define DECONV_FLOAT 0

#if DECONV_FLOAT
int loadDeconvData(const char* path, int nElems, float* ptrRetArray, int wt_bw, int wt_fl)
#else
int loadDeconvData(const char* path, int nElems, short* ptrRetArray, int wt_bw, int wt_fl)
#endif
{
#if DATA_IN_BINARY
	FILE* fp = fopen(path, "rb");
#else
	FILE* fp = fopen(path, "r");
#endif

	float val;
	short cast;

	nElems = AlignSize(nElems, 4);

	if(fp == NULL)
	{
		fprintf(stderr, "\n** Error : Cannot open the file : %s **\n", path);
		//exit(-1);
	}

#if 0
	for(int i = 0; i < nElems; i++)
	{
		fscanf(fp, "%f ", &val);
		cast = (T)val;
		ptrRetArray[i] = cast;

	}
#endif

#if DATA_IN_BINARY
	float *wts_buf = (float *)malloc(nElems*sizeof(float));
	fread(wts_buf, sizeof(float)*nElems, 1, fp);
#endif

	float f_val;
	for(int i = 0; i < nElems; i++)
	{
#if DATA_IN_BINARY
		f_val = wts_buf[i];
#else
		fscanf(fp, "%f ", &f_val);
#endif
		//Trim2FixedPoint(&f_val, wt_bw, wt_fl, ROUND_NEAREST, 1);

#if DECONV_FLOAT
		ptrRetArray[i] = f_val;
#else
		short ival = (short)f_val;
		short fxval = ConvertToFP(f_val, ival, 15);
		ptrRetArray[i] = fxval;
#endif
	}

#if DATA_IN_BINARY
	free(wts_buf);
#endif

	fclose(fp);
	return 0;
}

#endif //_XCHANGE_DECONV_UTILS_HPP_
