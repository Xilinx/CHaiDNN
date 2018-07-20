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
#define DECONV_WT_TYPE float
#else
#define DECONV_WT_TYPE short
#endif


int loadDeconvData(const float *wts_buf, int out_depth, int in_depth, int filter_h, int filter_w, DECONV_WT_TYPE* ptrRetArray, const float* wt_th_vec, int wt_bw, int quant_scheme)
{
	float val;
	short cast;

	int n_ofm = AlignSize(out_depth, 4);

#if FILE_WRITE
	FILE *fpW = fopen("weights.txt", "w");
#endif

	int n_elems = in_depth*filter_h*filter_w;

	float th_param;
	float f_val;
	short fxval;

	int idx = 0;

	for(int i = 0; i < n_ofm; i++)
	{
		if((i < out_depth) && (quant_scheme == 1))
		{
			th_param = wt_th_vec[i];
		}

		for(int j = 0; j < n_elems; j++)
		{
			if(i < out_depth)
			{
				f_val = wts_buf[(i*n_elems)+j];
			}
			else
				f_val = 0;

			if(quant_scheme == 1)
			{
				Trim2FixedPoint_Offline(&f_val, wt_bw, th_param);
				fxval = (short)f_val;
			}
			else
			{

#if DECONV_FLOAT
				ptrRetArray[i] = f_val;
#else
				short ival = (short)f_val;
				fxval = ConvertToFP(f_val, ival, 15);
#endif
			}

			ptrRetArray[idx++] = fxval;

		}

#if FILE_WRITE
		fprintf(fpW, "%f\n", f_val);
#endif
	}

#if FILE_WRITE
	fclose(fpW);
#endif

	return 0;
}

int loadDeconvBiasData(const float *bias_buf, int out_depth, DECONV_WT_TYPE* ptrRetArray, const float th_out, int bias_bw, int quant_scheme)
{
	float val;
	short cast;

	int n_ofm = AlignSize(out_depth, 4);

#if FILE_WRITE
	FILE *fpW = fopen("weights.txt", "w");
#endif

	float th_param;
	float f_val;
	short fxval;

	for(int i = 0; i < n_ofm; i++)
	{
		if(i < out_depth)
		{
			f_val = bias_buf[i];
		}
		else
			f_val = 0;

		if(quant_scheme == 1)
		{
			Trim2FixedPoint_Offline(&f_val, bias_bw, th_out);
			fxval = (short)f_val;
		}
		else
		{

#if DECONV_FLOAT
			ptrRetArray[i] = f_val;
#else
			short ival = (short)f_val;
			fxval = ConvertToFP(f_val, ival, 15);
#endif
		}

		ptrRetArray[i] = fxval;
	}

#if FILE_WRITE
	fprintf(fpW, "%f\n", f_val);
#endif

#if FILE_WRITE
	fclose(fpW);
#endif

	return 0;
}

#endif //_XCHANGE_DECONV_UTILS_HPP_
