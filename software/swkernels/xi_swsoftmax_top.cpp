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
#include <string.h>


// function to return the maximum of an array
template<typename inType>
inType getMaxVal(inType* start, int size)
{
	// TODO : Abid : Need a better value for maxVal
	inType maxVal = -9999999;

	for(int i=0; i<size; i++)
	{
		maxVal = start[i] < maxVal ? maxVal : start[i];
	}

	return maxVal;
}


// Function : SwSoftmax Layer
// inArray : Input array of size [nboxes x nclasses] eg : [7308 x 21] for SSD-300-PascalVOC
// outArray : Result array with same size
// nclasses : Number of classes in training (eg : 21 for PascalVOC, 1000 for ImageNet)
// nboxes : Number of boxes detected
// TODO : ABID : Better subtract the maxVal for numerical stability
//template <typename inType, typename outType>
void SwSoftmaxWrapper(float* inArray, float* sumBuffer, float* outArray, int *scalar_softmax_args)
{

	int nclasses  = scalar_softmax_args[0];
	int nboxes    = scalar_softmax_args[1];

#if SOFTMAX_OPT
	const float SOFTMAX_THRESHOLD = -6.0;
#endif

	// Buffer to keep the sum of exponents for each box
	//outType* sumBuffer = (outType*) malloc ( nboxes * sizeof(outType) );

	// printf("SwSoftmax Layer : Time for exp sum : ");
	// TIME_STAMP_INIT
	for(int i=0; i<nboxes; i++)
	{
		float maxVal = getMaxVal<float>(inArray + i*nclasses, nclasses);

		float sumVal = 0.0f;
		for(int j=0; j<nclasses; j++)
		{
			int tmpIdx = i*nclasses + j;
			float fval = inArray[tmpIdx] - maxVal;
#if SOFTMAX_OPT
			float outVal = fval < SOFTMAX_THRESHOLD ? 0.0 : expf(fval);
#else
			float outVal = expf(fval);
#endif
			outArray[tmpIdx] = outVal;
			sumVal += outVal;
		}
		sumBuffer[i] = 1.0f/sumVal;											// Optim : Take reciprocal
	}
	// TIME_STAMP

	// Now outArray contains element-wise exponent. Now normalize them
	// printf("SwSoftmax Layer : Time for Normalization : ");
	// TIME_STAMP_INIT
	for(int i=0; i<nboxes; i++)
	{
		float normParam = sumBuffer[i];
		for(int j=0; j<nclasses; j++)
		{
			outArray[i*nclasses + j] *= normParam;							// Optim : Multplify vs Divide
		}
	}
	// TIME_STAMP

	//free(sumBuffer);
}

