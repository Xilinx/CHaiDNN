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

#define PACK_ELEMS 8
#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)
#define ConvertToFP(fVal, iPart, fbits)	((int)((iPart<<fbits) + ((fVal-(float)iPart))*(1<<fbits)))



// Function : normalizeLayer_fixp
// This function calculates the L2-Norm of a feature output
// inArray : input feature map of shape [channel, height, width]
// gamma : Scaling factor in normalize layer with shape [channels]
// width, height : width & height of the feature map
// outArray : output result, same size as that of inArray
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

	for(int ch=0; ch<channels/2; ch+=PACK_ELEMS)
	{
		int channelOffset = ch * height * width;
		short *inptr = &inArray[channelOffset];
		for(int i=0; i<height*width; i++)
		{
			short val;
			int sumval = sumBuffer[i];
			int sumvalint;

			for(int packch=0; packch<PACK_ELEMS; packch++)
			{
				val = *inptr++;
				sumvalint = val * val;
				sumvalint = sumvalint >> fbits_input;
				sumval += sumvalint;
			}
			sumBuffer[i] = sumval;
		}

		//ch+=PACK_ELEMS;
		channelOffset = ch * height * width;
		inptr = &inArray1[channelOffset];
		for(int i=0; i<height*width; i++)
		{
			short val;
			int sumval = sumBuffer[i];
			int sumvalint;

			for(int packch=0; packch<PACK_ELEMS; packch++)
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
	for(int ch=0; ch < channels/2; ch+=PACK_ELEMS)
	{
		int channelOffset = ch * height * width;
		short *inptr = &inArray[channelOffset];
		short *outptr = &outArray[channelOffset];
		short gammaVal[PACK_ELEMS];
		short val[PACK_ELEMS];

		for(int packch=0; packch<PACK_ELEMS; packch++)
		{
			gammaVal[packch] = gamma[ch1+packch];
		}

		for(int i=0; i<height*width; i++)
		{
			int rmsVal = (int)sumBuffer[i];
			for(int packch=0; packch<PACK_ELEMS; packch++)
			{
				val[packch] = *inptr++;
				long mult1 = val[packch] *  gammaVal[packch];
				long mult2 = mult1 * rmsVal;
				mult2_16bit = mult2 >> fbits_input2;//40;
				*outptr++ = mult2_16bit;
			}
		}

		ch1+=PACK_ELEMS;

		channelOffset = ch * height * width;
		inptr  = &inArray1[channelOffset];
		outptr = &outArray1[channelOffset];

		for(int packch=0; packch<PACK_ELEMS; packch++)
		{
			gammaVal[packch] = gamma[ch1+packch];
		}

		for(int i=0; i<height*width; i++)
		{
			int rmsVal = (int)sumBuffer[i];
			for(int packch=0; packch<PACK_ELEMS; packch++)
			{
				val[packch] = *inptr++;
				long mult1 = val[packch] *  gammaVal[packch];
				long mult2 = mult1 * rmsVal;
				mult2_16bit = mult2 >> fbits_input2;//40;
				*outptr++ = mult2_16bit;
			}
		}
		ch1+=PACK_ELEMS;
	}
	//scalar_norm_args[5] = 0;    //across_spatial
}

#if 1
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

	for(int ch=0; ch<channels; ch+=PACK_ELEMS)
	{
		int channelOffset = ch * height * width;
		short *inptr = &inArray[channelOffset];
		for(int i=0; i<height*width; i++)
		{
			short val;
			int sumval = sumBuffer[i];
			int sumvalint;

			for(int packch=0; packch<PACK_ELEMS; packch++)
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
	for(int ch=0; ch < channels; ch+=PACK_ELEMS)
	{
		int channelOffset = ch * height * width;
		short *inptr = &inArray[channelOffset];
		short *outptr = &outArray[channelOffset];
		short gammaVal[PACK_ELEMS];
		short val[PACK_ELEMS];

		for(int packch=0; packch<PACK_ELEMS; packch++)
		{
			gammaVal[packch] = gamma[ch+packch];
		}

		for(int i=0; i<height*width; i++)
		{
			int rmsVal = (int)sumBuffer[i];
			for(int packch=0; packch<PACK_ELEMS; packch++)
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

#if 1
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

