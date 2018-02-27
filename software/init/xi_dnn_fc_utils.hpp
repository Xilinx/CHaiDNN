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

#ifndef _XCHANGE_FC_UTILS_HPP_
#define _XCHANGE_FC_UTILS_HPP_

void loadfcWgtsTXT(CONV_WT_TYPE* ptrRetArray1, CONV_WT_TYPE* ptrRetArray2, const char* path,
		int m_order, int n_order, int fbits, int wt_bw, int wt_fl)
{

#if DATA_IN_BINARY
	FILE* fp = fopen(path, "rb");
#else
	FILE* fp = fopen(path, "r");
#endif

	//int fbits = 7;//fcLayer_precision[layer_id].wgt_fbits;

	int n_order_new;

	n_order_new = AlignSize(n_order, 32);

	if(fp == NULL)
	{
		printf("\n** Error : Cannot open the file : %s **\n", path);
		exit(-1);
	}

#if DATA_IN_BINARY
	float *wts_buf = (float *)malloc(n_order_new*m_order*sizeof(float));
	fread(wts_buf, sizeof(float)*n_order*m_order, 1, fp);
#endif

#if HOST_DEBUG
	FILE *fpW = fopen("fcWgts.txt", "w");
#endif

	int wt_idx = 0;
	float f_val;
	int idx = 0;
	int idx1 = 0;
	int idx2 = 0;

	for(int i = 0; i < m_order; i++)
	{
		for(int j = 0; j < n_order_new; j++)
		{
			//idx = j + (i*n_order_new);
			if(j < n_order)
			{
#if DATA_IN_BINARY
				f_val = wts_buf[wt_idx++];
#else
				fscanf(fp, "%f ", &f_val);
#endif
				Trim2FixedPoint(&f_val, wt_bw, wt_fl, ROUND_NEAREST, 1);
			}
			else{
				f_val = 0;
			}

			short ival = (short)f_val;
			CONV_WT_TYPE fxval = ConvertToFP(f_val, ival, fbits);
			if((idx%32)<16)
			{
				ptrRetArray1[idx1] = (CONV_WT_TYPE)fxval;
				idx1++;
			}
			else
			{
				ptrRetArray2[idx2] = (CONV_WT_TYPE)fxval;
				idx2++;
			}
			idx++;


#if HOST_DEBUG
			fprintf(fpW, "%f ", f_val);
#endif
		}
#if HOST_DEBUG
		fprintf(fpW, "\n");
#endif
	}
#if HOST_DEBUG
	fclose(fpW);
#endif

#if DATA_IN_BINARY
    free(wts_buf);
#endif

	fclose(fp);
}
// loadfcWgtsTXT

void loadfcBiasTXT(short *ptrRetArray, const char* path, int nElems, int fbits, int bs_bw, int bs_fl)
{
	//int fbits = 3;//fcLayer_precision[layer_id].bias_fbits;
#if DATA_IN_BINARY
	FILE* fp = fopen(path, "rb");
#else
	FILE* fp = fopen(path, "r");
#endif

	if(fp == NULL)
	{
		printf("\n** Error : Cannot open the file : %s **\n", path);
		exit(-1);
	}

#if DATA_IN_BINARY
	float *bias_buf = (float *)malloc(nElems*sizeof(float));
	fread(bias_buf, sizeof(float)*nElems, 1, fp);
#endif

	float f_val;
	for(int i = 0; i < nElems; i++)
	{

#if DATA_IN_BINARY
		f_val = bias_buf[i];
#else
		fscanf(fp, "%f ", &f_val);
#endif
		Trim2FixedPoint(&f_val, bs_bw, bs_fl, ROUND_NEAREST, 1);

		short ival = (short)f_val;
		short fxval = ConvertToFP(f_val, ival, fbits);
		ptrRetArray[i] = fxval;
	}

#if DATA_IN_BINARY
	free(bias_buf);
#endif

	fclose(fp);
}
// loadfcBiasTXT


#endif //_XCHANGE_FC_UTILS_HPP_
