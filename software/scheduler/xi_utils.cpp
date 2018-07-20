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

#include "xi_utils.hpp"
#include <iostream>
#include <stdio.h>

#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)

//# Re-arranges data for HW FC
void fc_inputdatawrite(IO_DATA_TYPE *conv_out, IO_DATA_TYPE *conv_out1, int height, int width, int indepth, bool relu,
		IO_DATA_TYPE *kernel_output, char *output_file_path)
{
	int ofms  = height*width;
	int packs = ofms*PLANE_PACK;
	int depth = indepth/(PLANE_PACK<<1);

#if XI_UTILS_CONSOLE_TEXT
	std::cout << ofms << " x " << packs << " x " << depth << std::endl;
#endif

	int counter = 0;
	for(int t = 0; t < depth; ++t)
	{
		for(int pack = 0; pack < PLANE_PACK; ++pack)
		{
			for(int i = 0; i < ofms; ++i)
			{
				int batch_offset = (t*packs + i*PLANE_PACK + pack) * XBATCH_SIZE;
				for(int batch_id = 0; batch_id < XBATCH_SIZE; ++batch_id)
				{
					kernel_output[counter] = conv_out[batch_offset + batch_id];
					counter++;
				}
			}
		}
		for(int pack = 0; pack < PLANE_PACK; ++pack)
		{
			for(int i = 0; i < ofms; ++i)
			{
				int batch_offset = (t*packs + i*PLANE_PACK + pack) * XBATCH_SIZE;
				for(int batch_id = 0; batch_id < XBATCH_SIZE; ++batch_id)
				{
					kernel_output[counter] = conv_out1[batch_offset + batch_id];
					counter++;
				}
			}
		}
	}

#if XI_UTILS_CONSOLE_TEXT
	std::cout << "\n** Re-arrange : Done " << std::endl;
#endif

}//# fc_inputdatawrite

//# Re-arranges data for HW FC
void fc_inputdatawrite_float(IO_DATA_TYPE *conv_out, IO_DATA_TYPE *conv_out1, int height, int width, int indepth, bool relu,
		SW_FC_DATA_TYPE *kernel_output, float sf_in)
{
	int ofms  = height*width;
	int packs = ofms*PLANE_PACK;
	int depth = indepth/(PLANE_PACK<<1);

#if XI_UTILS_CONSOLE_TEXT
	std::cout << ofms << " x " << packs << " x " << depth << std::endl;
#endif

	SW_FC_DATA_TYPE *kernel_output1 = &kernel_output[indepth*ofms];

	int counter = 0;
	int counter1 = 0;
	for(int t = 0; t < depth; ++t)
	{
		for(int pack = 0; pack < PLANE_PACK; ++pack)
		{
			for(int i = 0; i < ofms; ++i)
			{
				int batch_offset = (t*packs + i*PLANE_PACK + pack) * XBATCH_SIZE;
				for(int batch_id = 0; batch_id < XBATCH_SIZE; ++batch_id)
				{
					//kernel_output[counter] = conv_out[batch_offset + batch_id];
					IO_DATA_TYPE fxval = conv_out[batch_offset + batch_id];
					float fval = ((IO_DATA_TYPE1)(fxval*sf_in));
					if(batch_id==0)
					{
						kernel_output[counter] = fval;
						counter++;
					}
					else
					{
						kernel_output1[counter1] = fval;
						counter1++;
					}
				}
			}
		}
		for(int pack = 0; pack < PLANE_PACK; ++pack)
		{
			for(int i = 0; i < ofms; ++i)
			{
				int batch_offset = (t*packs + i*PLANE_PACK + pack) * XBATCH_SIZE;
				for(int batch_id = 0; batch_id < XBATCH_SIZE; ++batch_id)
				{
					//kernel_output[counter] = conv_out1[batch_offset + batch_id];
					IO_DATA_TYPE fxval = conv_out1[batch_offset + batch_id];
					float fval = ((IO_DATA_TYPE1)(fxval*sf_in));
					if(batch_id==0)
					{
						kernel_output[counter] = fval;
						counter++;
					}
					else
					{
						kernel_output1[counter1] = fval;
						counter1++;
					}
				}
			}
		}
	}

#if XI_UTILS_CONSOLE_TEXT
	std::cout << "\n** Re-arrange : Done " << std::endl;
#endif

}//# fc_inputdatawrite_float

void fc_inputdatawrite_float_dynamicfixed(IO_DATA_TYPE *conv_out, IO_DATA_TYPE *conv_out1, int height, int width, int indepth, bool relu,
		SW_FC_DATA_TYPE *kernel_output, int in_fbits)
{
	int ofms  = height*width;
	int packs = ofms*PLANE_PACK;
	int depth = indepth/(PLANE_PACK<<1);

#if XI_UTILS_CONSOLE_TEXT
	std::cout << ofms << " x " << packs << " x " << depth << std::endl;
#endif

	SW_FC_DATA_TYPE *kernel_output1 = &kernel_output[indepth*ofms];

	int counter = 0;
	int counter1 = 0;
	for(int t = 0; t < depth; ++t)
	{
		for(int pack = 0; pack < PLANE_PACK; ++pack)
		{
			for(int i = 0; i < ofms; ++i)
			{
				int batch_offset = (t*packs + i*PLANE_PACK + pack) * XBATCH_SIZE;
				for(int batch_id = 0; batch_id < XBATCH_SIZE; ++batch_id)
				{
					//kernel_output[counter] = conv_out[batch_offset + batch_id];
					IO_DATA_TYPE fxval = conv_out[batch_offset + batch_id];
					//float fval = ((IO_DATA_TYPE1)(fxval*sf_in));
					float fval = ((float)fxval)/(1 << in_fbits);
					if(batch_id==0)
					{
						kernel_output[counter] = fval;
						counter++;
					}
					else
					{
						kernel_output1[counter1] = fval;
						counter1++;
					}
				}
			}
		}
		for(int pack = 0; pack < PLANE_PACK; ++pack)
		{
			for(int i = 0; i < ofms; ++i)
			{
				int batch_offset = (t*packs + i*PLANE_PACK + pack) * XBATCH_SIZE;
				for(int batch_id = 0; batch_id < XBATCH_SIZE; ++batch_id)
				{
					//kernel_output[counter] = conv_out1[batch_offset + batch_id];
					IO_DATA_TYPE fxval = conv_out1[batch_offset + batch_id];
					//float fval = ((IO_DATA_TYPE1)(fxval*sf_in));
					float fval = ((float)fxval)/(1 << in_fbits);
					if(batch_id==0)
					{
						kernel_output[counter] = fval;
						counter++;
					}
					else
					{
						kernel_output1[counter1] = fval;
						counter1++;
					}
				}
			}
		}
	}

#if XI_UTILS_CONSOLE_TEXT
	std::cout << "\n** Re-arrange : Done " << std::endl;
#endif

}//# fc_inputdatawrite_float1


//# Re-arranges data for HW deconv
void DeconvInReArrange(IO_DATA_TYPE *input1, IO_DATA_TYPE *input2, short *output, int height, int width, int inp_planes)
{
	int hxw = height*width;
	int n_planes = AlignSize(inp_planes, 16);
	int act_size = n_planes * hxw;

#if XI_UTILS_CONSOLE_TEXT
	std::cout << "\n[DECON] (h, w, planes) : " << height << " " << width << " " << inp_planes << std::endl;
	std::cout << "\n[DECON] n_planes : " << n_planes << std::endl;
#endif


	int idx1 = 0;
	int idx2 = 0;

#if 0
	FILE *fp = fopen("deconv_data.txt", "w");
	if(fp == NULL)
	{
		std :: cout << "can't create file - " << fp << std :: endl;
		return;
	}
#endif

	//# Re-arrange input data for Deconv
	for(int pl16 = 0; pl16 < inp_planes; pl16 += 16)
	{
		//std::cout <<  "[DECON] pl16 :" << pl16 << std::endl;
		for(int size = 0; size < hxw; size++)
		{
			for(int pl = 0; pl < 16; pl++)
			{
				int outidx = size + (hxw*pl) + (hxw*pl16);

				IO_DATA_TYPE a;
				for(int batch_id = 0; batch_id < XBATCH_SIZE; ++batch_id)
				{
					if(pl < 8)
					{
						a = input1[idx1++];
					}
					else
					{
						a = input2[idx2++];
					}

					int out_offset = batch_id * act_size;
					output[out_offset+outidx] = (short)a;
#if 0
					fprintf(fp, "outidx : %d out_offset : %d val[%d] : %hd\n", outidx, out_offset, out_offset+outidx, output[out_offset+outidx]);
#endif
				}
			}
		}
	}
#if 0
	fclose(fp);
#endif
}//# DeconvInReArrange


#define STR(_x_)    #_x_
#define BATCH       2
#define ConvertToFP(fVal, iPart, fbits)	((int)((iPart<<fbits) + ((fVal-(float)iPart))*(1<<fbits)))

//# Convolution out to FC in re-arrange
int convOut_to_fcInAsWts_rearrange(int8_t *convOut, int8_t *fc_in_asWts, int isize)
{
#ifdef SET_MEM_TO_ZERO
	//# For this release only
	for(int vid = 0; vid < 4; ++vid)
	{
		memset(fc_in_asWts[vid], 0, isize);
	}
#endif

#if 0
	FILE *fp = fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_rnd_models/AlexNet-8Bit_withRounding/caffe_ref/pool5_out.txt","r");
	if(fp == NULL)
	{
		//std :: cout << "[ERROR] can't open file - " << std :: endl;
		//return -1;
	}

	//# Fill data in first 64-bits only
	int8_t *data1 = convOut;
	float float_data;
	for(int widx = 0; widx < 9216; ++widx)
	{
		//data1[widx] = widx;
		fscanf(fp, "%f ", &float_data);
		char ival = (char)(float_data);
		char fxval = ConvertToFP( float_data, ival, 3);
		data1[2*widx] = fxval;
		data1[2*widx+1] = fxval;

	}
	fclose(fp);
#endif

	int8_t *data = convOut;
	int id = 0;

	//# Fill data in first 64-bits only
	int stride = 16;
	//for(int idx = 0; idx < isize/(4 * XBATCH_SIZE); ++idx)
	for(int idx = 0; idx < isize/(4); ++idx)
	{
		int index = idx * stride;
		//if(id == 64)
		//printf("stop");


		for(int widx = 0; widx < 4; ++widx)
		{
			fc_in_asWts[index + widx] = *data;
			data++;
			fc_in_asWts[index + widx + 4] = *data;
			data++;
			fc_in_asWts[index + widx + 8] = 0;
			fc_in_asWts[index + widx + 12] = 0;
			id++;
			id++;
		}

	}
#if 0
	FILE *fp1 = fopen("fc_input.txt","w");
	for(int idx = 0; idx < isize; ++idx)
	{
		fprintf(fp1,"%d ",fc_in_asWts[idx]);
	}
	fclose(fp1);
#endif
	return 0;
}

//# FC out to FC in re-arrange
int fcOut_to_fcIn_rearrange(int16_t *fcOut, int8_t *fc_in_asWts, int isize)
{
#ifdef SET_MEM_TO_ZERO
	//# For this release only
	for(int vid = 0; vid < 4; ++vid)
	{
		memset(fc_in_asWts[vid], 0, isize * sizeof(int16_t));
	}
#endif


#if 0
	FILE *fp = fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_rnd_models/AlexNet-8Bit_withRounding/caffe_ref/fc6_out.txt","r");
	if(fp == NULL)
	{
		//std :: cout << "[ERROR] can't open file - " << std :: endl;
		//return -1;
	}

	//# Fill data in first 64-bits only
	int8_t *data1 = (int8_t *)fcOut;
	float float_data;
	for(int widx = 0; widx < 4096; ++widx)
	{
		//data1[widx] = widx;
		fscanf(fp, "%f ", &float_data);
		char ival = (char)(float_data);
		char fxval = ConvertToFP( float_data, ival, 3);
		data1[16*widx+0] = fxval;
		data1[16*widx+1] = 0;
		data1[16*widx+2] = fxval;
		data1[16*widx+3] = 0;
		for(int j=4;j<16;j++)
			data1[16*widx+j] = 0;

	}
	fclose(fp);
#endif

	int8_t *data = (int8_t *)fcOut;
	//# Fill data in first 64-bits only
	int stride = 16;
	//for(int idx = 0; idx < isize/(4 * XBATCH_SIZE); ++idx)

	//fprintf(stderr, "isize : %d\n", isize);

	//int n_size = AlignSize(isize, 16);

	for(int idx = 0; idx < isize/(4); ++idx)
	{
		int index = idx * stride;
		for(int widx = 0; widx < 4; ++widx)
		{
			fc_in_asWts[index + widx] = data[0];
			//data++;
			fc_in_asWts[index + widx + 4] = data[2];
			data+=16;
			//data += 12;//index;
			fc_in_asWts[index + widx + 8] = 0;
			fc_in_asWts[index + widx + 12] = 0;
		}
	}
	return 0;
}

//# FC out to SoftMax in
int fcOut_to_smaxIn(int16_t *fcOut, int8_t *smaxIn, int isize)
{

#if 0
	FILE *fp = fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_rnd_models/AlexNet-8Bit_withRounding/caffe_ref/fc8_out.txt","r");
	if(fp == NULL)
	{
		//std :: cout << "[ERROR] can't open file - " << std :: endl;
		//return -1;
	}

	//# Fill data in first 64-bits only
	int8_t *data1 = (int8_t *)fcOut;
	float float_data;
	for(int widx = 0; widx < 1000; ++widx)
	{
		//data1[widx] = widx;
		fscanf(fp, "%f ", &float_data);
		char ival = (char)(float_data);
		char fxval = ConvertToFP( float_data, ival, 3);
		data1[16*widx+0] = fxval;
		data1[16*widx+1] = 0;
		data1[16*widx+2] = fxval;
		data1[16*widx+3] = 0;
		for(int j=4;j<16;j++)
			data1[16*widx+j] = 0;
	}
	fclose(fp);
#endif

	/*fprintf(stderr, "fcOut_to_smaxIn : isize - %d\n", isize);

	for(int i = 0; i< 32; i++)
	{
		fprintf(stderr, "%d ", fcOut[i]);

		if((i+1)%8 == 0)
			fprintf(stderr, "\n");
	}*/

	int8_t * data_b0 = smaxIn;
	int8_t * data_b1 = smaxIn + isize;
	int8_t * fc_out = (int8_t*)fcOut;
	int idx1 = 0;
	for(int idx = 0; idx < isize; ++idx)
	{
		data_b0[idx1++] = fc_out[0];
		data_b0[idx1++] = fc_out[2];

		//data_b1[idx] = fc_out[2];
		fc_out += 16;  //2
	}

#if 0
	int16_t * fc_out = fcOut;

	for(int idx = 0; idx < isize; ++idx)
	{
		data_b0[idx] = static_cast<int8_t>(fc_out[0]);
		data_b1[idx] = static_cast<int8_t>(fc_out[1]);
		fc_out += 8;  //2
	}


	fprintf(stderr, "After re-arrannge data_b0 :\n");
	for(int i = 0; i< 32; i++)
	{
		fprintf(stderr, "%f ", (float)data_b0[i]/(1 << 3));

		if((i+1)%8 == 0)
			fprintf(stderr, "\n");
	}

	fprintf(stderr, "After re-arrannge data_b1 :\n");
	for(int i = 0; i< 32; i++)
	{
		fprintf(stderr, "%f ", (float)data_b1[i]/(1 << 3));

		if((i+1)%8 == 0)
			fprintf(stderr, "\n");
	}
#endif

	return 0;
}

#include <math.h>
void Trim2FixedPoint_Offline1(float *data, const int bw, float threshold)
{
	//# Compute scale factor
	float scaling_factor = threshold / (pow(2, bw-1) - 1);

	//# Clip the data b/w (-threshold, threshold)
	data[0] = std::max(std::min(data[0], threshold), -threshold);

	if(threshold != 0)
	{
		data[0] /= scaling_factor;
		data[0] = round(data[0]);
		//data[0] *= scaling_factor;
	}
}

//# FC out to SoftMax in
int swfcOut_to_smaxIn1(float *fcOut, int8_t *smaxIn, int isize, float th_in)
{

#if 0
	FILE *fp = fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_rnd_models/AlexNet-8Bit_withRounding/caffe_ref/fc8_out.txt","r");
	if(fp == NULL)
	{
		//std :: cout << "[ERROR] can't open file - " << std :: endl;
		//return -1;
	}

	//# Fill data in first 64-bits only
	int8_t *data1 = (int8_t *)fcOut;
	float float_data;
	for(int widx = 0; widx < 1000; ++widx)
	{
		//data1[widx] = widx;
		fscanf(fp, "%f ", &float_data);
		char ival = (char)(float_data);
		char fxval = ConvertToFP( float_data, ival, 3);
		data1[16*widx+0] = fxval;
		data1[16*widx+1] = 0;
		data1[16*widx+2] = fxval;
		data1[16*widx+3] = 0;
		for(int j=4;j<16;j++)
			data1[16*widx+j] = 0;
	}
	fclose(fp);
#endif

	/*fprintf(stderr, "fcOut_to_smaxIn : isize - %d\n", isize);

	for(int i = 0; i< 32; i++)
	{
		fprintf(stderr, "%d ", fcOut[i]);

		if((i+1)%8 == 0)
			fprintf(stderr, "\n");
	}*/

	int8_t *data_b0 = smaxIn;
	int8_t *data_b1 = smaxIn + isize;
	float *fc_out = (float*)fcOut;
	float *fc_out1 = &fcOut[isize];
	int idx1 = 0;
	float f_val;
	int8_t fxval;
	for(int idx = 0; idx < isize; ++idx)
	{
		if(idx == 354)
		{
		   //printf("stop");
		}

		f_val = fc_out[idx];
		Trim2FixedPoint_Offline1(&f_val, 8, th_in);
		fxval = (int8_t)(f_val);///sf_in);
		data_b0[idx1++] = fxval;//fc_out[idx];

		f_val = fc_out1[idx];
		Trim2FixedPoint_Offline1(&f_val, 8, th_in);
		fxval = (int8_t)(f_val);///sf_in);
		data_b0[idx1++] = fxval;//fc_out1[idx];
	}

#if 0
	int16_t * fc_out = fcOut;

	for(int idx = 0; idx < isize; ++idx)
	{
		data_b0[idx] = static_cast<int8_t>(fc_out[0]);
		data_b1[idx] = static_cast<int8_t>(fc_out[1]);
		fc_out += 8;  //2
	}


	fprintf(stderr, "After re-arrannge data_b0 :\n");
	for(int i = 0; i< 32; i++)
	{
		fprintf(stderr, "%f ", (float)data_b0[i]/(1 << 3));

		if((i+1)%8 == 0)
			fprintf(stderr, "\n");
	}

	fprintf(stderr, "After re-arrannge data_b1 :\n");
	for(int i = 0; i< 32; i++)
	{
		fprintf(stderr, "%f ", (float)data_b1[i]/(1 << 3));

		if((i+1)%8 == 0)
			fprintf(stderr, "\n");
	}
#endif

	return 0;
}

//# FC out to SoftMax in
int swfcOut_to_smaxIn(float *fcOut, float *smaxIn, int isize)
{

#if 0
	FILE *fp = fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_rnd_models/AlexNet-8Bit_withRounding/caffe_ref/fc8_out.txt","r");
	if(fp == NULL)
	{
		//std :: cout << "[ERROR] can't open file - " << std :: endl;
		//return -1;
	}

	//# Fill data in first 64-bits only
	int8_t *data1 = (int8_t *)fcOut;
	float float_data;
	for(int widx = 0; widx < 1000; ++widx)
	{
		//data1[widx] = widx;
		fscanf(fp, "%f ", &float_data);
		char ival = (char)(float_data);
		char fxval = ConvertToFP( float_data, ival, 3);
		data1[16*widx+0] = fxval;
		data1[16*widx+1] = 0;
		data1[16*widx+2] = fxval;
		data1[16*widx+3] = 0;
		for(int j=4;j<16;j++)
			data1[16*widx+j] = 0;
	}
	fclose(fp);
#endif

	/*fprintf(stderr, "fcOut_to_smaxIn : isize - %d\n", isize);

	for(int i = 0; i< 32; i++)
	{
		fprintf(stderr, "%d ", fcOut[i]);

		if((i+1)%8 == 0)
			fprintf(stderr, "\n");
	}*/

	float *data_b0 = smaxIn;
	float *data_b1 = smaxIn + isize;
	float *fc_out = (float*)fcOut;
	float *fc_out1 = &fcOut[isize];
	int idx1 = 0;
	float f_val;
	float f_val1;
	int8_t fxval;
	for(int idx = 0; idx < isize; ++idx)
	{
		if(idx == 354)
		{
		   //printf("stop");
		}

		f_val = fc_out[idx];
		//Trim2FixedPoint_Offline1(&f_val, 8, th_in);
		//fxval = (int8_t)(f_val);///sf_in);
		data_b0[idx1++] = f_val;//fxval;//fc_out[idx];

		f_val1 = fc_out1[idx];
		//Trim2FixedPoint_Offline1(&f_val, 8, th_in);
		//fxval = (int8_t)(f_val);///sf_in);

		//if(f_val != f_val1)
			//printf("stop");
		data_b0[idx1++] = f_val1;//fxval;//fc_out1[idx];
	}

#if 0
	int16_t * fc_out = fcOut;

	for(int idx = 0; idx < isize; ++idx)
	{
		data_b0[idx] = static_cast<int8_t>(fc_out[0]);
		data_b1[idx] = static_cast<int8_t>(fc_out[1]);
		fc_out += 8;  //2
	}


	fprintf(stderr, "After re-arrannge data_b0 :\n");
	for(int i = 0; i< 32; i++)
	{
		fprintf(stderr, "%f ", (float)data_b0[i]/(1 << 3));

		if((i+1)%8 == 0)
			fprintf(stderr, "\n");
	}

	fprintf(stderr, "After re-arrannge data_b1 :\n");
	for(int i = 0; i< 32; i++)
	{
		fprintf(stderr, "%f ", (float)data_b1[i]/(1 << 3));

		if((i+1)%8 == 0)
			fprintf(stderr, "\n");
	}
#endif

	return 0;
}

#if 0
int main(void)
{
	int insize = 4096;
	int size   = insize * 16; //# Too much memory, will be allocated correctly by buffer management

	int8_t *conv_op;
	conv_op = new int8_t[size];
	if(conv_op == NULL){
		std::cerr << "[ERROR] no mem " << STR(conv_op) << std::endl;
		return -1;
	}

	std::vector<int8_t*> fc_in_asWts;
	for(int wptr = 0; wptr < 4; ++wptr)
	{
		int8_t* data = new int8_t[size];
		fc_in_asWts.push_back(data);
		data = NULL;
		if(fc_in_asWts[wptr] == NULL){
			std::cerr << "[ERROR] no mem " << STR(fc_in_asWts[wptr]) << std::endl;
			return -1;
		}
	}

	//# Call re-arrange | Call after linearize buffer call
	int retVal = convOut_2_fcInAsWts_arrange(conv_op, fc_in_asWts, insize);

	return retVal;
}
#endif
