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

void loadfcWgtsTXT(HFC_WGT_TYPE* ptrRetArray1, HFC_WGT_TYPE* ptrRetArray2, const float *wts_buf,
		int m_order, int n_order, int wt_bw, int wt_fl)
{

	//fprintf(stderr, "FC m : %d  n : %d\n", m_order, n_order);
#if 0
#if DATA_IN_BINARY
	FILE* fp = fopen(path, "rb");
#else
	FILE* fp = fopen(path, "r");
#endif
#endif
	int n_order_new, m_order_new;

	n_order_new = AlignSize(n_order, 32);
	m_order_new = AlignSize(m_order, 32);
	//fprintf(stderr, "FC m_new : %d  n_new : %d\n", m_order_new, n_order_new);
#if 0
	if(fp == NULL)
	{
		printf("\n** Error : Cannot open the file : %s **\n", path);
		exit(-1);
	}

#if DATA_IN_BINARY
	float *wts_buf = (float *)malloc(n_order_new*m_order*sizeof(float));
	fread(wts_buf, sizeof(float)*n_order*m_order, 1, fp);
#endif
#endif
#if HOST_DEBUG
	FILE *fpW = fopen("fcWgts.txt", "w");
#endif

	int wt_idx = 0;
	float f_val;
	int idx = 0;
	int idx1 = 0;
	int idx2 = 0;

	for(int i = 0; i < m_order_new; i++)
	{
		for(int j = 0; j < n_order_new; j++)
		{
			if(i < m_order)
			{
				if(j < n_order)
				{
#if 0
#if DATA_IN_BINARY
					f_val = wts_buf[wt_idx++];
#else
					fscanf(fp, "%f ", &f_val);
#endif
#endif
					f_val = wts_buf[wt_idx++];
				}
				else
					f_val = 0;
			}
			else
			{
				f_val = 0;
			}
			Trim2FixedPoint(&f_val, wt_bw, wt_fl, ROUND_NEAREST, 1);
			HFC_WGT_TYPE ival = (HFC_WGT_TYPE)f_val;
			HFC_WGT_TYPE fxval = ConvertToFP(f_val, ival, wt_fl);//fbits);
			if((idx%32)<16)
			{
				ptrRetArray1[idx1] = (HFC_WGT_TYPE)fxval;
				idx1++;
			}
			else
			{
				ptrRetArray2[idx2] = (HFC_WGT_TYPE)fxval;
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

	//fclose(fp);
}
// loadfcWgtsTXT

void loadfcBiasTXT(HFC_BIAS_TYPE *ptrRetArray, const float *bias_buf, int nElems, int bs_bw, int bs_fl)
{
#if 0
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
#endif
	float f_val;
	for(int i = 0; i < nElems; i++)
	{
#if 0
#if DATA_IN_BINARY
		f_val = bias_buf[i];
#else
		fscanf(fp, "%f ", &f_val);
#endif
#endif
		f_val = bias_buf[i];
		Trim2FixedPoint(&f_val, bs_bw, bs_fl, ROUND_NEAREST, 1);

		HFC_BIAS_TYPE ival = (HFC_BIAS_TYPE)f_val;
		HFC_BIAS_TYPE fxval = ConvertToFP(f_val, ival, bs_fl);
		ptrRetArray[i] = fxval;
	}

#if DATA_IN_BINARY
	free(bias_buf);
#endif

	//fclose(fp);
}
// loadfcBiasTXT

template<typename Ptr_Type>
void loadfcconvBiasTXT(Ptr_Type *ptrRetArray, const float *bias_buf, const float *wts_th_arr, float th_out, float th_in,
		int nElems, int in_bw, int op_bw, int wt_bw, int bs_bw, int bs_fl, string quant_scheme)
{
#if 0
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
#endif

	int n_size = AlignSize(nElems, ALIGN_FACTOR(HFC_BIAS_PORT_WIDTH, HFC_BIAS_DATA_WIDTH));

	float f_val;


	Ptr_Type fxval;

	for(int i = 0; i < n_size; i++)
	{
#if 0
#if DATA_IN_BINARY
		f_val = bias_buf[i];
#else
		fscanf(fp, "%f ", &f_val);
#endif
#endif
		if(i <  nElems)
		{
			f_val = bias_buf[i];

			if(quant_scheme.compare("Xilinx") == 0)
			{
				Trim2FixedPointBias_Offline(&f_val, bs_bw, th_out);
				fxval = (Ptr_Type)f_val;
			}
			else
			{
				Trim2FixedPoint(&f_val, bs_bw, bs_fl, ROUND_NEAREST, 1);

				Ptr_Type ival = (Ptr_Type)f_val;
				fxval = ConvertToFP(f_val, ival, bs_fl);
			}
		}
		else
		{
			fxval = 0;
		}

		ptrRetArray[i] = fxval;
	}

#if DATA_IN_BINARY
	free(bias_buf);
#endif

	//fclose(fp);

	//# Load scaling factors in case of offline quant mode
	if(quant_scheme.compare("Xilinx") == 0)
	{
		//# shift is used 8.8 format
		int shift_factor = 22;

		//# Compute scale factor
		float sf_in	 = th_in  / (pow(2, in_bw-1) - 1);
		float sf_out = th_out / (pow(2, op_bw-1) - 1);

		//# After bias, load scale factors
		Ptr_Type *InPtr = ptrRetArray + n_size;
		uint32_t *dstPtr = (uint32_t*)InPtr;

		uint32_t fx_val;

		for(int i = 0; i < n_size; i++)
		{
			float sf_param = wts_th_arr[i] / (pow(2, wt_bw-1) - 1);

			if(i < nElems)
			{
				f_val = (sf_param * sf_in / sf_out) * (1 << shift_factor);
				fx_val = (uint32_t)f_val;
			}
			else
			{
				fx_val = 0;
			}

			dstPtr[i] = fx_val;
		}
	}

}
// loadfcBiasTXT

template<typename Ptr_Type>
void loadfcconvWgts(Ptr_Type* ptrRetArray1, Ptr_Type* ptrRetArray2, const float *wts_buf,
		const float *wts_th_buf,
		int m_order, int n_order, int wt_bw, int wt_fl,
		string quant_scheme)
{

	int n_order_new, m_order_new;

	//TODO :: Check ALign factor Anusha
	//# Compute align factor
	n_order_new = AlignSize(n_order, ALIGN_FACTOR(HFC_IN_PORT_WIDTH, HFC_IN_DATA_WIDTH));
	m_order_new = AlignSize(m_order, ALIGN_FACTOR(HFC_BIAS_PORT_WIDTH, HFC_BIAS_DATA_WIDTH));

	fprintf(stderr, "n_order_new = %d, m_order_new = %d\n", n_order_new, m_order_new);

	int wt_idx = 0;
	float f_val;
	int idx = 0;
	int idx1 = 0;
	int idx2 = 0;
	float th_param;

	Ptr_Type fxval;

	for(int row = 0; row < n_order_new; row+=16)
	{
		for(int col = 0; col < m_order_new; col++)
		{
			if((quant_scheme.compare("Xilinx") == 0) && (col < m_order))
			{
				th_param = wts_th_buf[col];
			}

			idx = 0;
			wt_idx = col*n_order_new + row;

			for(int row16 = 0; row16 < 16; row16++)
			{
				if((col < m_order) && (row < n_order))
				{
					f_val = wts_buf[wt_idx++];
				}
				else
				{
					f_val = 0;
				}

				if(quant_scheme.compare("Xilinx") == 0)
				{
					Trim2FixedPoint_Offline(&f_val, wt_bw, th_param);
					fxval = (Ptr_Type)f_val;
				}
				else
				{
					Trim2FixedPoint(&f_val, wt_bw, wt_fl, ROUND_NEAREST, 1);
					Ptr_Type ival = (Ptr_Type)f_val;
					fxval = ConvertToFP(f_val, ival, wt_fl);
				}

				if(idx<8)
				{
					ptrRetArray1[idx1] = (Ptr_Type)fxval;
					idx1++;
				}
				else
				{
					ptrRetArray2[idx2] = (Ptr_Type)fxval;
					idx2++;
				}
				idx++;

			}  //for(int row16 = 0; row16 < 16; row16++)

		}  //for(int col = 0; col < m_order_new; col++)

	}  //for(int row = 0; row < n_order_new; row+=16)

}  // loadconvfcWgtsTXT

void loadswfcWghtsBias(SW_FC_DATA_TYPE *ptrRetArray, const float *wgts_buf, int m_order, int n_order)
{

	float f_val;
	int nElems = m_order * n_order;
#if 0
	for(int i = 0; i < nElems; i++)
	{
#if 0
#if DATA_IN_BINARY
		f_val = bias_buf[i];
#else
		fscanf(fp, "%f ", &f_val);
#endif
#endif
		f_val = wgts_buf[i];
		//Trim2FixedPoint(&f_val, bs_bw, bs_fl, ROUND_NEAREST, 1);

		ptrRetArray[i] = f_val;
	}
#endif

    //vector<float> WT; WT.resize(N*O);
    for(int i=0; i<n_order; ++i) {  //O
        for(int j=0; j<m_order; ++j) {  //N
        //	 ptrRetArray[j*n_order + i] = wgts_buf[i*m_order + j];
        	ptrRetArray[i*m_order + j] = wgts_buf[i*m_order + j];
        }
    }

#if DATA_IN_BINARY
	free(bias_buf);
#endif

	//fclose(fp);
}
// loadfcBiasTXT

#endif //_XCHANGE_FC_UTILS_HPP_
