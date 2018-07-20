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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../include/hw_settings.h"

#ifdef __SDSOC
#include <sds_lib.h>
#define PERFORMANCE_CHECK2			0

long long int clock_start_norm, clock_end_norm,clock_start_norm_inter,clock_end_norm_inter;
//float arm_clock = 1.1*1000000;
//float arm_clock = 1.19*1000000;
long long int frequency_norm = sds_clock_frequency();
float arm_clock_norm = (double)frequency_norm/(double)1000;
float Kerneltime1;
//float arm_clock = 1000/frequency;
#define TIME_STAMP_INIT  clock_start_norm = sds_clock_counter();
#define TIME_STAMP  { \
		clock_end_norm = sds_clock_counter(); \
		fprintf(stderr, "\n** Elapsed Time  : %lf ms\n", (double)(clock_end_norm-clock_start_norm)/arm_clock_norm); clock_start_norm = sds_clock_counter();  \
}
#endif


#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)
#define ConvertToFP(fVal, iPart, fbits)	((int)((iPart<<fbits) + ((fVal-(float)iPart))*(1<<fbits)))

#if 1//(IO_TYPE==16)
#define SUM_DATA_TYPE int
#define MULT_DATA_TYPE long
#else
#define SUM_DATA_TYPE short
#define MULT_DATA_TYPE int
#endif

#define SUM_FLOAT_DATA_TYPE float
#define MULT_FLOAT_DATA_TYPE float

// Function : normalizeLayer_fixp
// This function calculates the L2-Norm of a feature output
// inArray : input feature map of shape [channel, height, width]
// gamma : Scaling factor in normalize layer with shape [channels]
// width, height : width & height of the feature map
// outArray : output result, same size as that of inArray

#if 1
void NormalizationWrapper(IO_DATA_TYPE *inArray,
#if !SINGLE_IO_PORT
		IO_DATA_TYPE *inArray1,
#endif
		IO_DATA_TYPE *gamma,
		IO_DATA_TYPE *outArray,
#if !SINGLE_IO_PORT
		IO_DATA_TYPE *outArray1,
#endif
		int *sumBuffer1, int *scalar_norm_args
)
{
	SUM_DATA_TYPE *sumBuffer = (SUM_DATA_TYPE *)sumBuffer1;
	int channels = scalar_norm_args[0];
	int width    = scalar_norm_args[1];
	int height   = scalar_norm_args[2];
	int fbits_input = scalar_norm_args[3];
	int fbits_gamma = scalar_norm_args[4];
	//scalar_norm_args[6] = 1;  //across_spatial

	int norm_in_fbits = fbits_input;//1;
	int norm_out_fbits = scalar_norm_args[5];//8;

	int batch_size  = scalar_norm_args[7];
	int channelOffset;

	//memcpy(inArray,sds_inArray,height*width*(align_inp_planes)*batch_size*sizeof(IO_DATA_TYPE));
	//memcpy(inArray1,sds_inArray1,height*width*(align_inp_planes)*batch_size*sizeof(IO_DATA_TYPE));


	float eps=1e-10;

	int batch_cnt1;
	int batch_cnt2;

	if(batch_size < BATCH_PACK_SIZE)
	{
		batch_cnt2 = 1;
		batch_cnt1 = batch_size;
	}
	else
	{
		batch_cnt2 = batch_size/BATCH_PACK_SIZE;
		if((batch_cnt2 * BATCH_PACK_SIZE) < batch_size)
			batch_cnt2 += 1;
		batch_cnt1 = BATCH_PACK_SIZE;
	}

	batch_cnt2 = 1;
	batch_cnt1 = 2;

	int batch_size_one_enable =  scalar_norm_args[8];
	batch_size = XBATCH_SIZE;
	int batch_size_loopcnt;
	if(batch_size_one_enable)
	{
		batch_size_loopcnt = 1;
	}
	else
	{
		batch_size_loopcnt = 2;//batch_cnt1;
	}

	batch_size_loopcnt = 2;
#if 0
	fprintf(stderr, "batch_cnt2 = %d\n", batch_cnt2);
	fprintf(stderr, "i_cnt = %d\n", height*width);
	fprintf(stderr, "batch_cnt1 = %d\n", batch_cnt1);
	fprintf(stderr, "channels_cnt = %d\n", channels/2);
	fprintf(stderr, "batch_size_loopcnt = %d\n", batch_size_loopcnt);
	fprintf(stderr, "pack_cnt = %d\n", PLANE_PACK);
#endif
#if PERFORMANCE_CHECK2
	clock_start_norm = sds_clock_counter();
#endif
	for(int batch_id2 = 0; batch_id2 < batch_cnt2; batch_id2++)
	{
		int batch_offset = channels*height*width*batch_id2*batch_cnt1;

		for(int i=0; i<height*width; i++)
		{
			for(int batch_id1=0; batch_id1<batch_cnt1; batch_id1++)
			{
				sumBuffer[i*batch_cnt1+batch_id1] = 0;
			}
		}
		int ch_cnt = channels/HCONV_OUT_PORTS;

		for(int ch=0; ch<ch_cnt; ch+=PLANE_PACK)
		{
			int channelOffset2 = batch_offset + ch*height*width*batch_cnt1;
			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
				{
					IO_DATA_TYPE val;
					SUM_DATA_TYPE sumval = sumBuffer[i*batch_cnt1+batch_id1];
					SUM_DATA_TYPE sumvalint;

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
#if 0
						fprintf(stderr, "batch_id2 = %d\n", batch_id2);
						fprintf(stderr, "ch = %d\n", ch);
						fprintf(stderr, "i = %d\n", i);
						fprintf(stderr, "batch_id1 = %d\n", batch_id1);
						fprintf(stderr, "packch = %d\n", packch);
#endif

						channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						//short *inptr = &inArray[channelOffset];
						//val = *inptr++;
						val = inArray[channelOffset];
						sumvalint = val * val;
						sumvalint = sumvalint >> fbits_input;
						sumval += sumvalint;
					}
					sumBuffer[i*batch_cnt1+batch_id1] = sumval;
				}
			}

#if !SINGLE_IO_PORT
			//ch+=PLANE_PACK;
			//channelOffset = ch * height * width;
			//inptr = &inArray1[channelOffset];
			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
				{
					IO_DATA_TYPE val;
					SUM_DATA_TYPE sumval = sumBuffer[i*batch_cnt1+batch_id1];
					SUM_DATA_TYPE sumvalint;

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
						int channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						//val = *inptr++;
						val = inArray1[channelOffset];
						sumvalint = val * val;
						sumvalint = sumvalint >> fbits_input;
						sumval += sumvalint;
					}
					sumBuffer[i*batch_cnt1+batch_id1] = sumval;
				}
			}
#endif  //#if !SINGLE_IO_PORT
		}  //for(int ch=0; ch<ch_cnt; ch+=PLANE_PACK)

		// Compute square root of sumBuffer
		int fbits_input1  = 30;
		for(int i=0; i<height*width; i++)
		{
			for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
			{
				SUM_DATA_TYPE val = sumBuffer[i*batch_cnt1+batch_id1];
				float tmp_f = (float)val/(float)(1 << (fbits_input));

				if(tmp_f == 0)
					tmp_f = 1e-10;
				float f_val = 1.0f/sqrtf(tmp_f);								// Optim : Store the reciprocal
				//fprintf(stderr, "sumBuffer[%d] = %f\n", i, f_val);
				SUM_DATA_TYPE i_val=(SUM_DATA_TYPE)f_val;
				SUM_DATA_TYPE fxval=ConvertToFP(f_val,i_val,fbits_input1);
				sumBuffer[i*batch_cnt1+batch_id1] = (SUM_DATA_TYPE)fxval;
			}
		}

		int ch1 = 0;
		IO_DATA_TYPE mult2_16bit;
		IO_DATA_TYPE mult2_16bit_ref;
		// Normalize and Scale the values now.
		int fbits_input2  = fbits_input1 + fbits_gamma - (norm_out_fbits - norm_in_fbits);
		ch_cnt = channels/HCONV_OUT_PORTS;
		for(int ch=0; ch < ch_cnt; ch+=PLANE_PACK)
		{
			int channelOffset2 = batch_offset + ch*height*width*batch_cnt1;

			//int channelOffset = ch * height * width;
			//channelOffset = batch_offset + ch*height*width;// + packch*batch_id1;
			//short *inptr = &inArray[channelOffset];
			//short *outptr = &outArray[channelOffset];
			IO_DATA_TYPE gammaVal[PLANE_PACK];
			IO_DATA_TYPE val[PLANE_PACK];

			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				gammaVal[packch] = gamma[ch1+packch];
			}

			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
				{
					SUM_DATA_TYPE rmsVal = (SUM_DATA_TYPE)sumBuffer[i*batch_cnt1+batch_id1];

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
						channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						val[packch] = inArray[channelOffset];//*inptr++;
						MULT_DATA_TYPE mult1 = val[packch] *  gammaVal[packch];
						MULT_DATA_TYPE mult2 = mult1 * rmsVal;
						mult2_16bit = mult2 >> fbits_input2;//40;
						//*outptr++ = mult2_16bit;
						outArray[channelOffset] = mult2_16bit;
					}
				}
			}

			ch1+=PLANE_PACK;

#if !SINGLE_IO_PORT

			//channelOffset = ch * height * width;
			//inptr  = &inArray1[channelOffset];
			//outptr = &outArray1[channelOffset];

			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				gammaVal[packch] = gamma[ch1+packch];
			}

			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
				{
					SUM_DATA_TYPE rmsVal = (SUM_DATA_TYPE)sumBuffer[i*batch_cnt1+batch_id1];

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
						channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						val[packch] = inArray1[channelOffset];//*inptr++;
						//val[packch] = *inptr++;
						MULT_DATA_TYPE mult1 = val[packch] *  gammaVal[packch];
						MULT_DATA_TYPE mult2 = mult1 * rmsVal;
						mult2_16bit = mult2 >> fbits_input2;//40;
						//*outptr++ = mult2_16bit;
						outArray1[channelOffset] = mult2_16bit;
					}
				}
			}
			ch1+=PLANE_PACK;
#endif  //#if !SINGLE_IO_PORT
		}  //for(int ch=0; ch<ch_cnt; ch+=PLANE_PACK)

	}
#if PERFORMANCE_CHECK2
	clock_end_norm = sds_clock_counter();
	Kerneltime1 = ((double)(clock_end_norm-clock_start_norm))/arm_clock_norm;
	fprintf(stderr,"\nAverage time for Norm function dynamic in ms: %f ms\n", Kerneltime1);
#endif
	//for(int batch_id2 = 0; batch_id2 < batch_cnt2; batch_id2++)
	//scalar_norm_args[5] = 0;    //across_spatial
}

void NormalizationWrapper_float(IO_DATA_TYPE *inArray,
#if !SINGLE_IO_PORT
		IO_DATA_TYPE *inArray1,
#endif
		IO_DATA_TYPE1 *gamma,
		IO_DATA_TYPE *outArray,
#if !SINGLE_IO_PORT
		IO_DATA_TYPE *outArray1,
#endif
		IO_DATA_TYPE1 *sumBuffer1, float sf_in, float sf_out, int *scalar_norm_args
)
{
	SUM_FLOAT_DATA_TYPE *sumBuffer = (SUM_FLOAT_DATA_TYPE *)sumBuffer1;
	int channels = scalar_norm_args[0];
	int width    = scalar_norm_args[1];
	int height   = scalar_norm_args[2];
	int fbits_input = scalar_norm_args[3];
	int fbits_gamma = scalar_norm_args[4];
	//scalar_norm_args[6] = 1;  //across_spatial
	float t_sf_out = 1.0f/sf_out;
	int norm_in_fbits = fbits_input;//1;
	int norm_out_fbits = scalar_norm_args[5];//8;
	//int op_bw = scalar_norm_args[10];
	int batch_size  = scalar_norm_args[7];
	int channelOffset;

	//memcpy(inArray,sds_inArray,height*width*(align_inp_planes)*batch_size*sizeof(IO_DATA_TYPE));
	//memcpy(inArray1,sds_inArray1,height*width*(align_inp_planes)*batch_size*sizeof(IO_DATA_TYPE));


	float eps=1e-10;

	int batch_cnt1;
	int batch_cnt2;
	//int mm_range=pow(2,op_bw-1)-1;
	int max_data = scalar_norm_args[10];//mm_range;
	int min_data = -scalar_norm_args[10];//mm_range;

	//	if(op_bw==6)
	//		mm_range=31;
	//	else{
	//		if(op_bw==8)
	//			mm_range=127;
	//	}
	if(batch_size < BATCH_PACK_SIZE)
	{
		batch_cnt2 = 1;
		batch_cnt1 = batch_size;
	}
	else
	{
		batch_cnt2 = batch_size/BATCH_PACK_SIZE;
		if((batch_cnt2 * BATCH_PACK_SIZE) < batch_size)
			batch_cnt2 += 1;
		batch_cnt1 = BATCH_PACK_SIZE;
	}

	batch_cnt2 = 1;
	batch_cnt1 = 2;

	int batch_size_one_enable =  scalar_norm_args[8];
	batch_size = XBATCH_SIZE;
	int batch_size_loopcnt;
	if(batch_size_one_enable)
	{
		batch_size_loopcnt = 1;
	}
	else
	{
		batch_size_loopcnt = 2;//batch_cnt1;
	}

	batch_size_loopcnt = 2;
#if 0
	fprintf(stderr, "batch_cnt2 = %d\n", batch_cnt2);
	fprintf(stderr, "i_cnt = %d\n", height*width);
	fprintf(stderr, "batch_cnt1 = %d\n", batch_cnt1);
	fprintf(stderr, "channels_cnt = %d\n", channels/2);
	fprintf(stderr, "batch_size_loopcnt = %d\n", batch_size_loopcnt);
	fprintf(stderr, "pack_cnt = %d\n", PLANE_PACK);
#endif

#if PERFORMANCE_CHECK2
	clock_start_norm = sds_clock_counter();
#endif

	for(int batch_id2 = 0; batch_id2 < batch_cnt2; batch_id2++)
	{
		int batch_offset = channels*height*width*batch_id2*batch_cnt1;

		for(int i=0; i<height*width; i++)
		{
			for(int batch_id1=0; batch_id1<batch_cnt1; batch_id1++)
			{
				sumBuffer[i*batch_cnt1+batch_id1] = 0;
			}
		}
		int ch_cnt = channels/HCONV_OUT_PORTS;
		for(int ch=0; ch<ch_cnt; ch+=PLANE_PACK)
		{
			int channelOffset2 = batch_offset + ch*height*width*batch_cnt1;
			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
				{
					IO_DATA_TYPE val;
					IO_DATA_TYPE1 fval;
					SUM_FLOAT_DATA_TYPE sumval = sumBuffer[i*batch_cnt1+batch_id1];
					SUM_FLOAT_DATA_TYPE sumvalint;

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
#if 0
						fprintf(stderr, "batch_id2 = %d\n", batch_id2);
						fprintf(stderr, "ch = %d\n", ch);
						fprintf(stderr, "i = %d\n", i);
						fprintf(stderr, "batch_id1 = %d\n", batch_id1);
						fprintf(stderr, "packch = %d\n", packch);
#endif

						channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						//short *inptr = &inArray[channelOffset];
						//val = *inptr++;
						val = inArray[channelOffset];

						//if(quant_scheme_flag == 1)
						{
							fval = ((IO_DATA_TYPE1)(val*sf_in));
							sumvalint = fval * fval;
							//sumvalint = sumvalint >> fbits_input;
						}

						sumval += sumvalint;
					}
					sumBuffer[i*batch_cnt1+batch_id1] = sumval;
				}
			}

#if !SINGLE_IO_PORT
			//#if PERFORMANCE_CHECK2
			//	clock_start_norm = sds_clock_counter();
			//#endif
			//ch+=PLANE_PACK;
			//channelOffset = ch * height * width;
			//inptr = &inArray1[channelOffset];
			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
				{
					IO_DATA_TYPE val;
					IO_DATA_TYPE1 fval;
					SUM_FLOAT_DATA_TYPE sumval = sumBuffer[i*batch_cnt1+batch_id1];
					SUM_FLOAT_DATA_TYPE sumvalint;

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
						int channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						//val = *inptr++;
						val = inArray1[channelOffset];
						//sumvalint = val * val;
						//sumvalint = sumvalint >> fbits_input;

						{
							fval = ((IO_DATA_TYPE1)(val*sf_in));
							sumvalint = fval * fval;
							//sumvalint = sumvalint >> fbits_input;
						}

						sumval += sumvalint;
					}
					sumBuffer[i*batch_cnt1+batch_id1] = sumval;
				}
			}
#endif  //#if !SINGLE_IO_PORT
		}  //for(int ch=0; ch<ch_cnt; ch+=PLANE_PACK)


		//#if PERFORMANCE_CHECK2
		//	clock_start_norm = sds_clock_counter();
		//#endif
		// Compute square root of sumBuffer
		int fbits_input1  = 30;
		for(int i=0; i<height*width; i++)
		{
			for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
			{
				SUM_FLOAT_DATA_TYPE tmp_f = sumBuffer[i*batch_cnt1+batch_id1];
				//float tmp_f = (float)val/(float)(1 << (fbits_input));

				if(tmp_f == 0)
					tmp_f = 1e-10;
				float f_val = 1.0f/sqrtf(tmp_f);								// Optim : Store the reciprocal
				//fprintf(stderr, "sumBuffer[%d] = %f\n", i, f_val);
				//SUM_FLOAT_DATA_TYPE i_val=(SUM_FLOAT_DATA_TYPE)f_val;
				//SUM_FLOAT_DATA_TYPE fxval=ConvertToFP(f_val,i_val,fbits_input1);
				sumBuffer[i*batch_cnt1+batch_id1] = f_val;//(SUM_FLOAT_DATA_TYPE)fxval;
			}
		}

		//#if PERFORMANCE_CHECK2
		//		clock_start_norm = sds_clock_counter();
		//#endif
		int ch1 = 0;
		IO_DATA_TYPE mult2_16bit;
		IO_DATA_TYPE mult2_16bit_ref;
		// Normalize and Scale the values now.
		int fbits_input2  = fbits_input1 + fbits_gamma - (norm_out_fbits - norm_in_fbits);
		ch_cnt = channels/HCONV_OUT_PORTS;
		for(int ch=0; ch < ch_cnt; ch+=PLANE_PACK)
		{
			int channelOffset2 = batch_offset + ch*height*width*batch_cnt1;

			//int channelOffset = ch * height * width;
			//channelOffset = batch_offset + ch*height*width;// + packch*batch_id1;
			//short *inptr = &inArray[channelOffset];
			//short *outptr = &outArray[channelOffset];
			IO_DATA_TYPE1 gammaVal[PLANE_PACK];
			IO_DATA_TYPE val[PLANE_PACK];
			IO_DATA_TYPE1 fval[PLANE_PACK];

			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				gammaVal[packch] = gamma[ch1+packch];
			}
			//#if PERFORMANCE_CHECK2
			//			clock_start_norm_inter = sds_clock_counter();
			//#endif

			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
				{
					SUM_FLOAT_DATA_TYPE rmsVal = (SUM_FLOAT_DATA_TYPE)sumBuffer[i*batch_cnt1+batch_id1];
					int f_val=1;
					for(int packch=0; packch<PLANE_PACK; packch++)
					{
						channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						val[packch] = inArray[channelOffset];//*inptr++;
						fval[packch] = ((IO_DATA_TYPE1)(val[packch]*sf_in));
						MULT_FLOAT_DATA_TYPE mult1 = fval[packch] *  gammaVal[packch];
						MULT_FLOAT_DATA_TYPE mult2 = mult1 * rmsVal;
						//MULT_FLOAT_DATA_TYPE mult2 = fval[packch] *  gammaVal[packch];
						//mult2_16bit = mult2 >> fbits_input2;//40;
						//*outptr++ = mult2_16bit;
						//						IO_DATA_TYPE1 fresult = mult2/sf_out;
						IO_DATA_TYPE1 fresult = mult2*t_sf_out;
						int tmp = (int)fresult;
						//int max_data = mm_range;
						//int min_data = -mm_range;
						int s_val = std::max(std::min(tmp, max_data), min_data);

						//						if(tmp>127)
						//						{
						//							tmp = 127;
						//						}
						mult2_16bit = ((IO_DATA_TYPE)(s_val));
						//						mult2_16bit = ((IO_DATA_TYPE)(tmp));

						outArray[channelOffset] = mult2_16bit;

					}
				}
			}

			ch1+=PLANE_PACK;

#if !SINGLE_IO_PORT
			//#if PERFORMANCE_CHECK2
			//			clock_end_norm_inter = sds_clock_counter();
			//			Kerneltime1 = ((double)(clock_end_norm_inter-clock_start_norm_inter))/arm_clock_norm;
			//			fprintf(stderr,"\nAverage time for Norm function 1 in ms: %f ms\n", Kerneltime1);
			//#endif
			//channelOffset = ch * height * width;
			//inptr  = &inArray1[channelOffset];
			//outptr = &outArray1[channelOffset];

			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				gammaVal[packch] = gamma[ch1+packch];
			}
			//#if PERFORMANCE_CHECK2
			//			clock_start_norm_inter = sds_clock_counter();
			//#endif
			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
				{
					SUM_FLOAT_DATA_TYPE rmsVal = (SUM_FLOAT_DATA_TYPE)sumBuffer[i*batch_cnt1+batch_id1];

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
						channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						val[packch] = inArray1[channelOffset];//*inptr++;
						//val[packch] = *inptr++;
						fval[packch] = ((IO_DATA_TYPE1)(val[packch]*sf_in));
						MULT_FLOAT_DATA_TYPE mult1 = fval[packch] *  gammaVal[packch];
						MULT_FLOAT_DATA_TYPE mult2 = mult1 * rmsVal;

						//MULT_FLOAT_DATA_TYPE mult2 = fval[packch] *  gammaVal[packch];
						//mult2_16bit = mult2 >> fbits_input2;//40;
						//*outptr++ = mult2_16bit;
						//mult2_16bit = ((IO_DATA_TYPE)(mult2/sf_out));

						//						IO_DATA_TYPE1 fresult = mult2/sf_out;
						IO_DATA_TYPE1 fresult = mult2*t_sf_out;

						int tmp = (int)fresult;
						//int max_data = mm_range;
						//int min_data = -mm_range;

						int s_val = std::max(std::min(tmp, max_data),min_data);
						//						if(tmp > 127)
						//						{
						//							tmp = 127;
						//						}
						mult2_16bit = ((IO_DATA_TYPE)(s_val));
						//						mult2_16bit = ((IO_DATA_TYPE)(tmp));
						outArray1[channelOffset] = mult2_16bit;
					}
				}
			}
			ch1+=PLANE_PACK;
			//#if PERFORMANCE_CHECK2
			//			clock_end_norm_inter = sds_clock_counter();
			//			Kerneltime1 = ((double)(clock_end_norm_inter-clock_start_norm_inter))/arm_clock_norm;
			//			fprintf(stderr,"\nAverage time for Norm function 2 in ms: %f ms\n", Kerneltime1);
			//#endif

#endif  //#if !SINGLE_IO_PORT
		}  //for(int ch=0; ch<ch_cnt; ch+=PLANE_PACK)


	}  //for(int batch_id2 = 0; batch_id2 < batch_cnt2; batch_id2++)
	//scalar_norm_args[5] = 0;    //across_spatial

#if PERFORMANCE_CHECK2
	clock_end_norm = sds_clock_counter();
	Kerneltime1 = ((double)(clock_end_norm-clock_start_norm))/arm_clock_norm;
	fprintf(stderr,"\nAverage time for Norm function in ms: %f ms\n", Kerneltime1);
#endif
}


void NormalizationWrapper_fixfloat(IO_DATA_TYPE *inArray, IO_DATA_TYPE *inArray1, IO_DATA_TYPE1 *gamma,
		IO_DATA_TYPE *outArray, IO_DATA_TYPE *outArray1, IO_DATA_TYPE1 *sumBuffer1, float sf_in, float sf_out, int *scalar_norm_args
)
{
	SUM_FLOAT_DATA_TYPE *sumBuffer_float = (SUM_FLOAT_DATA_TYPE *)sumBuffer1;
	SUM_DATA_TYPE *sumBuffer = (SUM_DATA_TYPE *)sumBuffer1;
	int channels = scalar_norm_args[0];
	int width    = scalar_norm_args[1];
	int height   = scalar_norm_args[2];
	int fbits_input = scalar_norm_args[3];
	int fbits_gamma = scalar_norm_args[4];
	//scalar_norm_args[6] = 1;  //across_spatial

	int norm_in_fbits = fbits_input;//1;
	int norm_out_fbits = scalar_norm_args[5];//8;

	int batch_size  = scalar_norm_args[7];
	int channelOffset;

	//memcpy(inArray,sds_inArray,height*width*(align_inp_planes)*batch_size*sizeof(IO_DATA_TYPE));
	//memcpy(inArray1,sds_inArray1,height*width*(align_inp_planes)*batch_size*sizeof(IO_DATA_TYPE));


	float eps=1e-10;

	int batch_cnt1;
	int batch_cnt2;

	if(batch_size < BATCH_PACK_SIZE)
	{
		batch_cnt2 = 1;
		batch_cnt1 = batch_size;
	}
	else
	{
		batch_cnt2 = batch_size/BATCH_PACK_SIZE;
		if((batch_cnt2 * BATCH_PACK_SIZE) < batch_size)
			batch_cnt2 += 1;
		batch_cnt1 = BATCH_PACK_SIZE;
	}

	batch_cnt2 = 1;
	batch_cnt1 = 2;

	int batch_size_one_enable =  scalar_norm_args[8];
	batch_size = XBATCH_SIZE;
	int batch_size_loopcnt;
	if(batch_size_one_enable)
	{
		batch_size_loopcnt = 1;
	}
	else
	{
		batch_size_loopcnt = 2;//batch_cnt1;
	}

	batch_size_loopcnt = 2;
#if 0
	fprintf(stderr, "batch_cnt2 = %d\n", batch_cnt2);
	fprintf(stderr, "i_cnt = %d\n", height*width);
	fprintf(stderr, "batch_cnt1 = %d\n", batch_cnt1);
	fprintf(stderr, "channels_cnt = %d\n", channels/2);
	fprintf(stderr, "batch_size_loopcnt = %d\n", batch_size_loopcnt);
	fprintf(stderr, "pack_cnt = %d\n", PLANE_PACK);
#endif

#if PERFORMANCE_CHECK2
	clock_start_norm = sds_clock_counter();
#endif

	for(int batch_id2 = 0; batch_id2 < batch_cnt2; batch_id2++)
	{
		int batch_offset = channels*height*width*batch_id2*batch_cnt1;

		for(int i=0; i<height*width; i++)
		{
			for(int batch_id1=0; batch_id1<batch_cnt1; batch_id1++)
			{
				sumBuffer[i*batch_cnt1+batch_id1] = 0;
			}
		}

		for(int ch=0; ch<channels/2; ch+=PLANE_PACK)
		{
			int channelOffset2 = batch_offset + ch*height*width*batch_cnt1;
			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
				{
					IO_DATA_TYPE val;
					IO_DATA_TYPE1 fval;
					SUM_DATA_TYPE sumval = sumBuffer[i*batch_cnt1+batch_id1];
					SUM_DATA_TYPE sumvalint;

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
#if 0
						fprintf(stderr, "batch_id2 = %d\n", batch_id2);
						fprintf(stderr, "ch = %d\n", ch);
						fprintf(stderr, "i = %d\n", i);
						fprintf(stderr, "batch_id1 = %d\n", batch_id1);
						fprintf(stderr, "packch = %d\n", packch);
#endif

						channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						//short *inptr = &inArray[channelOffset];
						//val = *inptr++;
						val = inArray[channelOffset];

						//if(quant_scheme_flag == 1)
						{
							//fval = ((IO_DATA_TYPE1)(val*sf_in));
							sumvalint = val * val;
							//sumvalint = sumvalint >> fbits_input;
						}



						sumval += sumvalint;
					}
					sumBuffer[i*batch_cnt1+batch_id1] = sumval;
				}
			}

			//ch+=PLANE_PACK;
			//channelOffset = ch * height * width;
			//inptr = &inArray1[channelOffset];
			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
				{
					IO_DATA_TYPE val;
					IO_DATA_TYPE1 fval;
					SUM_DATA_TYPE sumval = sumBuffer[i*batch_cnt1+batch_id1];
					SUM_DATA_TYPE sumvalint;

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
						int channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						//val = *inptr++;
						val = inArray1[channelOffset];
						//sumvalint = val * val;
						//sumvalint = sumvalint >> fbits_input;

						{
							//fval = ((IO_DATA_TYPE1)(val*sf_in));
							sumvalint = val * val;
							//sumvalint = sumvalint >> fbits_input;
						}

						sumval += sumvalint;
					}
					sumBuffer[i*batch_cnt1+batch_id1] = sumval;
				}
			}
		}

		// Compute square root of sumBuffer
		int fbits_input1  = 30;
		for(int i=0; i<height*width; i++)
		{
			for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
			{
				SUM_DATA_TYPE tmp = sumBuffer[i*batch_cnt1+batch_id1];
				float tmp_f = (float)tmp;///(float)(1 << (fbits_input));

				if(tmp_f == 0)
					tmp_f = 1e-10;
				float f_val = 1.0f/sqrtf(tmp_f);								// Optim : Store the reciprocal
				//fprintf(stderr, "sumBuffer[%d] = %f\n", i, f_val);
				SUM_DATA_TYPE i_val=(SUM_DATA_TYPE)f_val;
				SUM_DATA_TYPE fxval=ConvertToFP(f_val,i_val,fbits_input1);
				sumBuffer_float[i*batch_cnt1+batch_id1] = f_val;//(SUM_FLOAT_DATA_TYPE)fxval;
				sumBuffer[i*batch_cnt1+batch_id1] = (SUM_DATA_TYPE)fxval;
			}
		}

		int ch1 = 0;
		IO_DATA_TYPE mult2_16bit;
		IO_DATA_TYPE mult2_16bit_ref;
		// Normalize and Scale the values now.
		int fbits_input2  = fbits_input1 + fbits_gamma - (norm_out_fbits - norm_in_fbits);
		for(int ch=0; ch < channels/2; ch+=PLANE_PACK)
		{
			int channelOffset2 = batch_offset + ch*height*width*batch_cnt1;

			//int channelOffset = ch * height * width;
			//channelOffset = batch_offset + ch*height*width;// + packch*batch_id1;
			//short *inptr = &inArray[channelOffset];
			//short *outptr = &outArray[channelOffset];
			IO_DATA_TYPE1 gammaVal[PLANE_PACK];
			IO_DATA_TYPE val[PLANE_PACK];
			IO_DATA_TYPE1 fval[PLANE_PACK];

			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				gammaVal[packch] = gamma[ch1+packch];
			}

			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
				{
					SUM_FLOAT_DATA_TYPE rmsVal = (SUM_FLOAT_DATA_TYPE)sumBuffer_float[i*batch_cnt1+batch_id1];
					//SUM_DATA_TYPE rmsVal = (SUM_DATA_TYPE)sumBuffer[i*batch_cnt1+batch_id1];

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
						channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						val[packch] = inArray[channelOffset];//*inptr++;
						fval[packch] = ((IO_DATA_TYPE1)(val[packch]*sf_in));
						MULT_FLOAT_DATA_TYPE mult1 = fval[packch] *  gammaVal[packch];
						MULT_FLOAT_DATA_TYPE fresult = mult1 * rmsVal;
						//mult2_16bit = mult2 >> fbits_input2;//40;
						//*outptr++ = mult2_16bit;
						//IO_DATA_TYPE1 fresult = mult2/sf_out;
						//MULT_FLOAT_DATA_TYPE fresult = val[packch];
						if(fresult > 127)
						{
							fresult = 127;
						}
						mult2_16bit = ((IO_DATA_TYPE)(fresult));
						outArray[channelOffset] = mult2_16bit;

					}
				}
			}

			ch1+=PLANE_PACK;

			//channelOffset = ch * height * width;
			//inptr  = &inArray1[channelOffset];
			//outptr = &outArray1[channelOffset];

			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				gammaVal[packch] = gamma[ch1+packch];
			}

			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_size_loopcnt; batch_id1++)
				{
					SUM_FLOAT_DATA_TYPE rmsVal = (SUM_FLOAT_DATA_TYPE)sumBuffer[i*batch_cnt1+batch_id1];

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
						channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						val[packch] = inArray1[channelOffset];//*inptr++;
						//val[packch] = *inptr++;
						//fval[packch] = ((IO_DATA_TYPE1)(val[packch]*sf_in));
						//MULT_FLOAT_DATA_TYPE mult1 = val[packch] *  gammaVal[packch];
						//MULT_FLOAT_DATA_TYPE fresult = mult1 * rmsVal;
						//mult2_16bit = mult2 >> fbits_input2;//40;
						//*outptr++ = mult2_16bit;
						//mult2_16bit = ((IO_DATA_TYPE)(mult2/sf_out));

						//IO_DATA_TYPE1 fresult = mult2/sf_out;
						MULT_FLOAT_DATA_TYPE fresult = val[packch];
						if(fresult > 127)
						{
							fresult = 127;
						}
						mult2_16bit = ((IO_DATA_TYPE)(fresult));

						outArray1[channelOffset] = mult2_16bit;
					}
				}
			}
			ch1+=PLANE_PACK;
		}

	}  //for(int batch_id2 = 0; batch_id2 < batch_cnt2; batch_id2++)
	//scalar_norm_args[5] = 0;    //across_spatial

#if PERFORMANCE_CHECK2
	clock_end_norm = sds_clock_counter();
	Kerneltime1 = ((double)(clock_end_norm-clock_start_norm))/arm_clock_norm;
	fprintf(stderr,"\nAverage time for Norm function in ms: %f ms\n", Kerneltime1);
#endif
}

#endif


#if 0
void NormalizationWrapper(IO_DATA_TYPE *inArray, IO_DATA_TYPE *inArray1, IO_DATA_TYPE *gamma,
		IO_DATA_TYPE *outArray, IO_DATA_TYPE *outArray1, int *sumBuffer1, int *scalar_norm_args
)
{
	SUM_DATA_TYPE *sumBuffer = (SUM_DATA_TYPE *)sumBuffer1;
	int channels = scalar_norm_args[0];
	int width    = scalar_norm_args[1];
	int height   = scalar_norm_args[2];
	int fbits_input = scalar_norm_args[3];
	int fbits_gamma = scalar_norm_args[4];
	//scalar_norm_args[6] = 1;  //across_spatial

	int norm_in_fbits = fbits_input;//1;
	int norm_out_fbits = scalar_norm_args[5];//8;

	int batch_size  = scalar_norm_args[7];
	int channelOffset;

	//memcpy(inArray,sds_inArray,height*width*(align_inp_planes)*batch_size*sizeof(IO_DATA_TYPE));
	//memcpy(inArray1,sds_inArray1,height*width*(align_inp_planes)*batch_size*sizeof(IO_DATA_TYPE));


	float eps=1e-10;

	int batch_cnt1;
	int batch_cnt2;

	if(batch_size < BATCH_PACK_SIZE)
	{
		batch_cnt2 = 1;
		batch_cnt1 = batch_size;
	}
	else
	{
		batch_cnt2 = batch_size/BATCH_PACK_SIZE;
		if((batch_cnt2 * BATCH_PACK_SIZE) < batch_size)
			batch_cnt2 += 1;
		batch_cnt1 = BATCH_PACK_SIZE;
	}

	for(int batch_id2 = 0; batch_id2 < batch_cnt2; batch_id2++)
	{
		int batch_offset = channels*height*width*batch_id2*batch_cnt1;

		for(int i=0; i<height*width; i++)
		{
			for(int batch_id1=0; batch_id1<batch_cnt1; batch_id1++)
			{
				sumBuffer[i*batch_cnt1+batch_id1] = 0;
			}
		}

		for(int ch=0; ch<channels/2; ch+=PLANE_PACK)
		{
			int channelOffset2 = batch_offset + ch*height*width*batch_cnt1;
			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_cnt1; batch_id1++)
				{
					IO_DATA_TYPE val;
					SUM_DATA_TYPE sumval = sumBuffer[i*batch_cnt1+batch_id1];
					SUM_DATA_TYPE sumvalint;

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
						channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						//short *inptr = &inArray[channelOffset];
						//val = *inptr++;
						val = inArray[channelOffset];
						sumvalint = val * val;
						sumvalint = sumvalint >> fbits_input;
						sumval += sumvalint;
					}
					sumBuffer[i*batch_cnt1+batch_id1] = sumval;
				}
			}

			//ch+=PLANE_PACK;
			//channelOffset = ch * height * width;
			//inptr = &inArray1[channelOffset];
			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_cnt1; batch_id1++)
				{
					IO_DATA_TYPE val;
					SUM_DATA_TYPE sumval = sumBuffer[i*batch_cnt1+batch_id1];
					SUM_DATA_TYPE sumvalint;

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
						int channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						//val = *inptr++;
						val = inArray1[channelOffset];
						sumvalint = val * val;
						sumvalint = sumvalint >> fbits_input;
						sumval += sumvalint;
					}
					sumBuffer[i*batch_cnt1+batch_id1] = sumval;
				}
			}
		}

		// Compute square root of sumBuffer
		int fbits_input1  = 30;
		for(int i=0; i<height*width; i++)
		{
			for(int batch_id1=0; batch_id1<batch_cnt1; batch_id1++)
			{
				SUM_DATA_TYPE val = sumBuffer[i*batch_cnt1+batch_id1];
				float tmp_f = (float)val/(float)(1 << (fbits_input));

				if(tmp_f == 0)
					tmp_f = 1e-10;
				float f_val = 1.0f/sqrt(tmp_f);								// Optim : Store the reciprocal
				//fprintf(stderr, "sumBuffer[%d] = %f\n", i, f_val);
				SUM_DATA_TYPE i_val=(SUM_DATA_TYPE)f_val;
				SUM_DATA_TYPE fxval=ConvertToFP(f_val,i_val,fbits_input1);
				sumBuffer[i*batch_cnt1+batch_id1] = (SUM_DATA_TYPE)fxval;
			}
		}

		int ch1 = 0;
		IO_DATA_TYPE mult2_16bit;
		IO_DATA_TYPE mult2_16bit_ref;
		// Normalize and Scale the values now.
		int fbits_input2  = fbits_input1 + fbits_gamma - (norm_out_fbits - norm_in_fbits);
		for(int ch=0; ch < channels/2; ch+=PLANE_PACK)
		{
			int channelOffset2 = batch_offset + ch*height*width*batch_cnt1;

			//int channelOffset = ch * height * width;
			//channelOffset = batch_offset + ch*height*width;// + packch*batch_id1;
			//short *inptr = &inArray[channelOffset];
			//short *outptr = &outArray[channelOffset];
			IO_DATA_TYPE gammaVal[PLANE_PACK];
			IO_DATA_TYPE val[PLANE_PACK];

			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				gammaVal[packch] = gamma[ch1+packch];
			}

			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_cnt1; batch_id1++)
				{
					SUM_DATA_TYPE rmsVal = (SUM_DATA_TYPE)sumBuffer[i*batch_cnt1+batch_id1];

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
						channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						val[packch] = inArray[channelOffset];//*inptr++;
						MULT_DATA_TYPE mult1 = val[packch] *  gammaVal[packch];
						MULT_DATA_TYPE mult2 = mult1 * rmsVal;
						mult2_16bit = mult2 >> fbits_input2;//40;
						//*outptr++ = mult2_16bit;
						outArray[channelOffset] = mult2_16bit;
					}
				}
			}

			ch1+=PLANE_PACK;

			//channelOffset = ch * height * width;
			//inptr  = &inArray1[channelOffset];
			//outptr = &outArray1[channelOffset];

			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				gammaVal[packch] = gamma[ch1+packch];
			}

			for(int i=0; i<height*width; i++)
			{
				int channelOffset1 = channelOffset2 + i*batch_cnt1*PLANE_PACK;
				for(int batch_id1=0; batch_id1<batch_cnt1; batch_id1++)
				{
					SUM_DATA_TYPE rmsVal = (SUM_DATA_TYPE)sumBuffer[i*batch_cnt1+batch_id1];

					for(int packch=0; packch<PLANE_PACK; packch++)
					{
						channelOffset = channelOffset1 + packch*batch_cnt1 + batch_id1;
						val[packch] = inArray1[channelOffset];//*inptr++;
						//val[packch] = *inptr++;
						MULT_DATA_TYPE mult1 = val[packch] *  gammaVal[packch];
						MULT_DATA_TYPE mult2 = mult1 * rmsVal;
						mult2_16bit = mult2 >> fbits_input2;//40;
						//*outptr++ = mult2_16bit;
						outArray1[channelOffset] = mult2_16bit;
					}
				}
			}
			ch1+=PLANE_PACK;
		}

	}  //for(int batch_id2 = 0; batch_id2 < batch_cnt2; batch_id2++)
	//scalar_norm_args[5] = 0;    //across_spatial
}
#endif

#if 0
void NormalizationWrapper(short *inArray, short *inArray1, short* gamma,
		short *outArray, short *outArray1, int *sumBuffer, int *scalar_norm_args
)
{
	int channels = scalar_norm_args[0];
	int width    = scalar_norm_args[1];
	int height   = scalar_norm_args[2];
	int fbits_input = scalar_norm_args[3];
	int fbits_gamma = scalar_norm_args[4];
	//scalar_norm_args[6] = 1;  //across_spatial

	int norm_in_fbits = fbits_input;//1;
	int norm_out_fbits = scalar_norm_args[5];//8;


	float eps=1e-10;

	for(int i=0; i<height*width; i++)
	{
		sumBuffer[i] = 0;
	}

	for(int ch=0; ch<channels/2; ch+=PLANE_PACK)
	{
		int channelOffset = ch * height * width;
		short *inptr = &inArray[channelOffset];
		for(int i=0; i<height*width; i++)
		{
			short val;
			int sumval = sumBuffer[i];
			int sumvalint;

			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				val = *inptr++;
				sumvalint = val * val;
				sumvalint = sumvalint >> fbits_input;
				sumval += sumvalint;
			}
			sumBuffer[i] = sumval;
		}

		//ch+=PLANE_PACK;
		channelOffset = ch * height * width;
		inptr = &inArray1[channelOffset];
		for(int i=0; i<height*width; i++)
		{
			short val;
			int sumval = sumBuffer[i];
			int sumvalint;

			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				val = *inptr++;
				sumvalint = val * val;
				sumvalint = sumvalint >> fbits_input;
				sumval += sumvalint;
			}
			sumBuffer[i] = sumval;
		}
	}

	// Compute square root of sumBuffer
	int fbits_input1  = 30;
	for(int i=0; i<height*width; i++)
	{
		int val = sumBuffer[i];
		float tmp_f = (float)val/(float)(1 << (fbits_input));

		if(tmp_f == 0)
			tmp_f = 1e-10;
		float f_val = 1.0f/sqrt(tmp_f);								// Optim : Store the reciprocal
		//fprintf(stderr, "sumBuffer[%d] = %f\n", i, f_val);
		int i_val=(int)f_val;
		int fxval=ConvertToFP(f_val,i_val,fbits_input1);
		sumBuffer[i] = (int)fxval;
	}

	int ch1 = 0;
	short mult2_16bit;
	short mult2_16bit_ref;
	// Normalize and Scale the values now.
	int fbits_input2  = fbits_input1 + fbits_gamma - (norm_out_fbits - norm_in_fbits);
	for(int ch=0; ch < channels/2; ch+=PLANE_PACK)
	{
		int channelOffset = ch * height * width;
		short *inptr = &inArray[channelOffset];
		short *outptr = &outArray[channelOffset];
		short gammaVal[PLANE_PACK];
		short val[PLANE_PACK];

		for(int packch=0; packch<PLANE_PACK; packch++)
		{
			gammaVal[packch] = gamma[ch1+packch];
		}

		for(int i=0; i<height*width; i++)
		{
			int rmsVal = (int)sumBuffer[i];
			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				val[packch] = *inptr++;
				long mult1 = val[packch] *  gammaVal[packch];
				long mult2 = mult1 * rmsVal;
				mult2_16bit = mult2 >> fbits_input2;//40;
				*outptr++ = mult2_16bit;
			}
		}

		ch1+=PLANE_PACK;

		channelOffset = ch * height * width;
		inptr  = &inArray1[channelOffset];
		outptr = &outArray1[channelOffset];

		for(int packch=0; packch<PLANE_PACK; packch++)
		{
			gammaVal[packch] = gamma[ch1+packch];
		}

		for(int i=0; i<height*width; i++)
		{
			int rmsVal = (int)sumBuffer[i];
			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				val[packch] = *inptr++;
				long mult1 = val[packch] *  gammaVal[packch];
				long mult2 = mult1 * rmsVal;
				mult2_16bit = mult2 >> fbits_input2;//40;
				*outptr++ = mult2_16bit;
			}
		}
		ch1+=PLANE_PACK;
	}
	//scalar_norm_args[5] = 0;    //across_spatial
}
#endif

#if 0
// Function : normalizeLayer_fixp
// This function calculates the L2-Norm of a feature output
// inArray : input feature map of shape [channel, height, width]
// gamma : Scaling factor in normalize layer with shape [channels]
// width, height : width & height of the feature map
// outArray : output result, same size as that of inArray
void NormalizationWrapper1(short *inArray, short* gamma,
		short *outArray, int *sumBuffer, int *scalar_norm_args
)
{
	int norm_in_fbits = 1;
	int norm_out_fbits = 8;

	int channels = scalar_norm_args[0];
	int width    = scalar_norm_args[1];
	int height   = scalar_norm_args[2];
	int fbits_input = scalar_norm_args[3];
	int fbits_gamma = scalar_norm_args[4];

	float eps=1e-10;

	for(int i=0; i<height*width; i++)
	{
		sumBuffer[i] = 0;
	}

	for(int ch=0; ch<channels; ch+=PLANE_PACK)
	{
		int channelOffset = ch * height * width;
		short *inptr = &inArray[channelOffset];
		for(int i=0; i<height*width; i++)
		{
			short val;
			int sumval = sumBuffer[i];
			int sumvalint;

			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				val = *inptr++;
				sumvalint = val * val;
				sumvalint = sumvalint >> fbits_input;
				sumval += sumvalint;
			}
			sumBuffer[i] = sumval;
		}
	}

	// Compute square root of sumBuffer
	int fbits_input1  = 30;
	for(int i=0; i<height*width; i++)
	{
		int val = sumBuffer[i];
		float tmp_f = (float)val/(float)(1 << (fbits_input));

		if(tmp_f == 0)
			tmp_f = 1e-10;
		float f_val = 1.0f/sqrt(tmp_f);								// Optim : Store the reciprocal
		//fprintf(stderr, "sumBuffer[%d] = %f\n", i, f_val);
		int i_val=(int)f_val;
		int fxval=ConvertToFP(f_val,i_val,fbits_input1);
		sumBuffer[i] = (int)fxval;
	}

	// Normalize and Scale the values now.
	int fbits_input2  = fbits_input1 + fbits_gamma - (norm_out_fbits - norm_in_fbits);
	for(int ch=0; ch < channels; ch+=PLANE_PACK)
	{
		int channelOffset = ch * height * width;
		short *inptr = &inArray[channelOffset];
		short *outptr = &outArray[channelOffset];
		short gammaVal[PLANE_PACK];
		short val[PLANE_PACK];

		for(int packch=0; packch<PLANE_PACK; packch++)
		{
			gammaVal[packch] = gamma[ch+packch];
		}

		for(int i=0; i<height*width; i++)
		{
			int rmsVal = (int)sumBuffer[i];
			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				val[packch] = *inptr++;
				long mult1 = val[packch] *  gammaVal[packch];
				long mult2 = mult1 * rmsVal;
				short mult2_16bit = mult2 >> fbits_input2;//40;
				*outArray++ = mult2_16bit;
			}

		}
	}

}
#endif

#if 0
// Function : normalizeLayer
// This function calculates the L2-Norm of a feature output
// inArray : input feature map of shape [channel, height, width]
// gamma : Scaling factor in normalize layer with shape [channels]
// width, height : width & height of the feature map
// outArray : output result, same size as that of inArray
void normalizeLayer(float *inArray, float* gamma, int channels,
		int width, int height, float *outArray)
{
	// Buffer to store the sum, initialize it to zero.

	// printf("Normalize Layer : Time for malloc : ");
	// TIME_STAMP_INIT
	float eps=1e-10;
	float* sumBuffer = (float *) malloc (height * width * sizeof(float));
	// TIME_STAMP

	// printf("Normalize Layer : Time for memset : ");
	// TIME_STAMP_INIT
	//memset(sumBuffer, eps, height * width * sizeof(float));

	for(int i=0; i<height*width; i++)
	{
		sumBuffer[i] = eps;
	}

	// TIME_STAMP

	// Normalization is done across axis = 0
	// TODO : ABID : Use a flat pointer to optimize index calculation
	// printf("Normalize Layer : Time for squared sum : ");
	// TIME_STAMP_INIT
	for(int c=0; c < channels; c++)
	{
		int channelOffset = c * height * width;
		for(int i=0; i<height; i++)
		{
			// TODO : ABID : Precompute height offset to optimize index calculation
			for(int j=0; j<width; j++)
			{
				float val = inArray[channelOffset + i*width + j];
				sumBuffer[i*width + j] += val * val;
			}
		}
	}
	// TIME_STAMP

	// Compute square root of sumBuffer
	// TODO : ABID : Optimize indexing with a flat iterator
	// printf("Normalize Layer : Time for square root : ");
	// TIME_STAMP_INIT
	for(int i=0; i<height; i++)
	{
		for(int j=0; j<width; j++)
		{
			float val = sumBuffer[i*width + j];
			sumBuffer[i*width + j] = 1.0f/sqrt(val);								// Optim : Store the reciprocal
		}
	}
	// TIME_STAMP

	// Normalize and Scale the values now.
	// printf("Normalize Layer : Time for Normalize : ");
	// TIME_STAMP_INIT
	for(int c=0; c < channels; c++)
	{
		int channelOffset = c * height * width;
		float gammaVal = gamma[c];
		for(int i=0; i<height; i++)
		{
			// TODO : ABID : Precompute height offset to optimize index calculation
			for(int j=0; j<width; j++)
			{
				float val = inArray[channelOffset + i*width + j];
				float rmsVal = sumBuffer[i*width + j];
				float result = (val * rmsVal) * gammaVal;							// Optim : Multiply vs Divide
				outArray[channelOffset + i*width + j] = result;
			}
		}
	}
	// TIME_STAMP
	// free the arrays
	free(sumBuffer);
}
#endif
#if 0
// Function : normalizeLayer
// This function calculates the L2-Norm of a feature output
// inArray : input feature map of shape [channel, height, width]
// gamma : Scaling factor in normalize layer with shape [channels]
// width, height : width & height of the feature map
// outArray : output result, same size as that of inArray
void normalizeLayer(float *inArray, float* gamma, int channels,
		int width, int height, float *outArray, float eps=1e-10)
{
	// Buffer to store the sum, initialize it to zero.

	// printf("Normalize Layer : Time for malloc : ");
	// TIME_STAMP_INIT
	float* sumBuffer = (float *) malloc (height * width * sizeof(float));
	// TIME_STAMP

	// printf("Normalize Layer : Time for memset : ");
	// TIME_STAMP_INIT
	memset(sumBuffer, eps, height * width * sizeof(float));
	// TIME_STAMP

	// Normalization is done across axis = 0
	// TODO : ABID : Use a flat pointer to optimize index calculation
	// printf("Normalize Layer : Time for squared sum : ");
	// TIME_STAMP_INIT

	for(int c=0; c < channels; c++)
	{
		int channelOffset = c * height * width;
		for(int i=0; i<height; i++)
		{
			// TODO : ABID : Precompute height offset to optimize index calculation
			for(int j=0; j<width; j++)
			{
				float val = inArray[channelOffset + i*width + j];
				sumBuffer[i*width + j] += val * val;
			}
		}
	}
	// TIME_STAMP

	// Compute square root of sumBuffer
	// TODO : ABID : Optimize indexing with a flat iterator
	// printf("Normalize Layer : Time for square root : ");
	// TIME_STAMP_INIT
	for(int i=0; i<height; i++)
	{
		for(int j=0; j<width; j++)
		{
			float val = sumBuffer[i*width + j];
			sumBuffer[i*width + j] = 1.0f/sqrt(val);								// Optim : Store the reciprocal
		}
	}
	// TIME_STAMP

	// Normalize and Scale the values now.
	// printf("Normalize Layer : Time for Normalize : ");
	// TIME_STAMP_INIT
	for(int c=0; c < channels; c++)
	{
		int channelOffset = c * height * width;
		float gammaVal = gamma[c];
		for(int i=0; i<height; i++)
		{
			// TODO : ABID : Precompute height offset to optimize index calculation
			for(int j=0; j<width; j++)
			{
				float val = inArray[channelOffset + i*width + j];
				float rmsVal = sumBuffer[i*width + j];
				float result = (val * rmsVal) * gammaVal;							// Optim : Multiply vs Divide
				outArray[channelOffset + i*width + j] = result;
			}
		}
	}
	// TIME_STAMP
	// free the arrays
	free(sumBuffer);
}
#endif

