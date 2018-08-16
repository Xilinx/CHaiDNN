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

#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <numeric>
#include <cfloat>
#include <cmath>
#include <ctime>
#include <cstring>

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "../include/hw_settings.h"

#ifdef __SDSOC
#include <arm_neon.h>
void eltwise_add_i8_8bit(IO_DATA_TYPE *pi8_a1, IO_DATA_TYPE *pi8_a2, IO_DATA_TYPE *pi8_b1, IO_DATA_TYPE *pi8_b2, IO_DATA_TYPE *pi8_s1, IO_DATA_TYPE *pi8_s2, int i4_n, int relu_en)
{
	int8x16_t *pi8x16_a1 = (int8x16_t *)pi8_a1;
	int8x16_t *pi8x16_a2 = (int8x16_t *)pi8_a2;
	int8x16_t *pi8x16_b1 = (int8x16_t *)pi8_b1;
	int8x16_t *pi8x16_b2 = (int8x16_t *)pi8_b2;
	int8x16_t *pi8x16_s1 = (int8x16_t *)pi8_s1;
	int8x16_t *pi8x16_s2 = (int8x16_t *)pi8_s2;

	IO_DATA_TYPE max_val[16];
	IO_DATA_TYPE min_val[16];

	for( int i=0; i< 16; i++)
	{
		max_val[i] = 127;
		if(relu_en)
			min_val[i] = 0;
		else
			min_val[i] = -128;
	}

	int8x16_t *pi8x16_max_val = (int8x16_t *)max_val;
	int8x16_t *pi8x16_min_val = (int8x16_t *)min_val;
	int8x16_t i8x16_max_val = *pi8x16_max_val;
	int8x16_t i8x16_min_val = *pi8x16_min_val;

	int loop_count = i4_n >> 4;
	int i4_loop = 0;

	for(i4_loop = 0; i4_loop < loop_count; i4_loop++)
	{
		int8x16_t i8x16_a1 = *pi8x16_a1; pi8x16_a1++;
		int8x16_t i8x16_a2 = *pi8x16_a2; pi8x16_a2++;
		int8x16_t i8x16_b1 = *pi8x16_b1; pi8x16_b1++;
		int8x16_t i8x16_b2 = *pi8x16_b2; pi8x16_b2++;

		//*pi8x16_s1 = vqaddq_s8(i8x16_a1, i8x16_b1);
		//*pi8x16_s2 = vqaddq_s8(i8x16_a2, i8x16_b2);

		int8x16_t i8x16_s1 = vqaddq_s8(i8x16_a1, i8x16_b1);
		int8x16_t i8x16_s2 = vqaddq_s8(i8x16_a2, i8x16_b2);

		i8x16_s1   = vmaxq_s8(i8x16_s1, i8x16_min_val);
		*pi8x16_s1 = vminq_s8(i8x16_s1, i8x16_max_val);

		i8x16_s2   = vmaxq_s8(i8x16_s2, i8x16_min_val);
		*pi8x16_s2 = vminq_s8(i8x16_s2, i8x16_max_val);

		pi8x16_s1++;
		pi8x16_s2++;

	}

	pi8_a1 = (IO_DATA_TYPE *)pi8x16_a1;
	pi8_b1 = (IO_DATA_TYPE *)pi8x16_b1;
	pi8_a2 = (IO_DATA_TYPE *)pi8x16_a2;
	pi8_b2 = (IO_DATA_TYPE *)pi8x16_b2;
	pi8_s1 = (IO_DATA_TYPE *)pi8x16_s1;
	pi8_s2 = (IO_DATA_TYPE *)pi8x16_s2;

	for(i4_loop = (loop_count << 4); i4_loop < i4_n; i4_loop++)
	{
		IO_DATA_TYPE i8_a1 = *pi8_a1; pi8_a1++;
		IO_DATA_TYPE i8_a2 = *pi8_a2; pi8_a2++;
		IO_DATA_TYPE i8_b1 = *pi8_b1; pi8_b1++;
		IO_DATA_TYPE i8_b2 = *pi8_b2; pi8_b2++;

		short i2_s1 = i8_a1 + i8_b1;
		short i2_s2 = i8_a2 + i8_b2;

		*pi8_s1 = (i2_s1 > max_val[0]) ? max_val[0] : ((i2_s1 < min_val[0]) ? min_val[0]: i2_s1);
		*pi8_s2 = (i2_s2 > max_val[0]) ? max_val[0] : ((i2_s2 < min_val[0]) ? min_val[0]: i2_s2);

		pi8_s1++;
		pi8_s2++;

	}

	return;
}


void eltwise_add_i8_6bit(IO_DATA_TYPE *pi8_a1, IO_DATA_TYPE *pi8_a2, IO_DATA_TYPE *pi8_b1, IO_DATA_TYPE *pi8_b2, IO_DATA_TYPE *pi8_s1, IO_DATA_TYPE *pi8_s2, int i4_n, int relu_en)
{
	int8x16_t *pi8x16_a1 = (int8x16_t *)pi8_a1;
	int8x16_t *pi8x16_a2 = (int8x16_t *)pi8_a2;
	int8x16_t *pi8x16_b1 = (int8x16_t *)pi8_b1;
	int8x16_t *pi8x16_b2 = (int8x16_t *)pi8_b2;
	int8x16_t *pi8x16_s1 = (int8x16_t *)pi8_s1;
	int8x16_t *pi8x16_s2 = (int8x16_t *)pi8_s2;

	IO_DATA_TYPE max_val[16];
	IO_DATA_TYPE min_val[16];

	for( int i=0; i< 16; i++)
	{
		max_val[i] = 31;
		if(relu_en)
			min_val[i] = 0;
		else
			min_val[i] = -32;
	}

	int8x16_t *pi8x16_max_val = (int8x16_t *)max_val;
	int8x16_t *pi8x16_min_val = (int8x16_t *)min_val;
	int8x16_t i8x16_max_val = *pi8x16_max_val;
	int8x16_t i8x16_min_val = *pi8x16_min_val;

	int loop_count = i4_n >> 4;
	int i4_loop = 0;

	for(i4_loop = 0; i4_loop < loop_count; i4_loop++)
	{
		int8x16_t i8x16_a1 = *pi8x16_a1; pi8x16_a1++;
		int8x16_t i8x16_a2 = *pi8x16_a2; pi8x16_a2++;
		int8x16_t i8x16_b1 = *pi8x16_b1; pi8x16_b1++;
		int8x16_t i8x16_b2 = *pi8x16_b2; pi8x16_b2++;

		int8x16_t i8x16_s1 = vqaddq_s8(i8x16_a1, i8x16_b1);
		int8x16_t i8x16_s2 = vqaddq_s8(i8x16_a2, i8x16_b2);

		i8x16_s1   = vmaxq_s8(i8x16_s1, i8x16_min_val);
		i8x16_s2   = vmaxq_s8(i8x16_s2, i8x16_min_val);

		*pi8x16_s1 = vminq_s8(i8x16_s1, i8x16_max_val);		
		*pi8x16_s2 = vminq_s8(i8x16_s2, i8x16_max_val);		

		pi8x16_s1++;
		pi8x16_s2++;
	}

	pi8_a1 = (IO_DATA_TYPE *)pi8x16_a1;
	pi8_b1 = (IO_DATA_TYPE *)pi8x16_b1;
	pi8_a2 = (IO_DATA_TYPE *)pi8x16_a2;
	pi8_b2 = (IO_DATA_TYPE *)pi8x16_b2;
	pi8_s1 = (IO_DATA_TYPE *)pi8x16_s1;
	pi8_s2 = (IO_DATA_TYPE *)pi8x16_s2;

	for(i4_loop = (loop_count << 4); i4_loop < i4_n; i4_loop++)
	{
		IO_DATA_TYPE i8_a1 = *pi8_a1; pi8_a1++;
		IO_DATA_TYPE i8_a2 = *pi8_a2; pi8_a2++;
		IO_DATA_TYPE i8_b1 = *pi8_b1; pi8_b1++;
		IO_DATA_TYPE i8_b2 = *pi8_b2; pi8_b2++;

		short i2_s1 = i8_a1 + i8_b1;
		short i2_s2 = i8_a2 + i8_b2;

		*pi8_s1 = (i2_s1 > max_val[0]) ? max_val[0] : ((i2_s1 < min_val[0]) ? min_val[0]: i2_s1);
		*pi8_s2 = (i2_s2 > max_val[0]) ? max_val[0] : ((i2_s2 < min_val[0]) ? min_val[0]: i2_s2);

		pi8_s1++;
		pi8_s2++;

	}

	return;
}

void eltwise_add(IO_DATA_TYPE *pi8_a1, IO_DATA_TYPE *pi8_a2, IO_DATA_TYPE *pi8_b1, IO_DATA_TYPE *pi8_b2, IO_DATA_TYPE *pi8_s1, IO_DATA_TYPE *pi8_s2, int i4_n, int relu_en, int io_6bit)
{

	void *ptr_a1 = malloc(i4_n*sizeof(IO_DATA_TYPE));
	void *ptr_a2 = malloc(i4_n*sizeof(IO_DATA_TYPE));
	void *ptr_b1 = malloc(i4_n*sizeof(IO_DATA_TYPE));
	void *ptr_b2 = malloc(i4_n*sizeof(IO_DATA_TYPE));
	void *ptr_s1 = malloc(i4_n*sizeof(IO_DATA_TYPE));
	void *ptr_s2 = malloc(i4_n*sizeof(IO_DATA_TYPE));

	memcpy(ptr_a1,pi8_a1,i4_n*sizeof(IO_DATA_TYPE));
	memcpy(ptr_a2,pi8_a2,i4_n*sizeof(IO_DATA_TYPE));
	memcpy(ptr_b1,pi8_b1,i4_n*sizeof(IO_DATA_TYPE));
	memcpy(ptr_b2,pi8_b2,i4_n*sizeof(IO_DATA_TYPE));

	int8x16_t *pi8x16_a1 = (int8x16_t *)ptr_a1;
	int8x16_t *pi8x16_a2 = (int8x16_t *)ptr_a2;
	int8x16_t *pi8x16_b1 = (int8x16_t *)ptr_b1;
	int8x16_t *pi8x16_b2 = (int8x16_t *)ptr_b2;
	int8x16_t *pi8x16_s1 = (int8x16_t *)ptr_s1;
	int8x16_t *pi8x16_s2 = (int8x16_t *)ptr_s2;

	IO_DATA_TYPE max_val[16];
	IO_DATA_TYPE min_val[16];

	if(io_6bit == 1)
	{
		for( int i=0; i< 16; i++)
		{
			max_val[i] = 31;
			if(relu_en)
				min_val[i] = 0;
			else
				min_val[i] = -32;
		}
	}
	else
	{	
		for( int i=0; i< 16; i++)
		{
			max_val[i] = 127;
			if(relu_en)
				min_val[i] = 0;
			else
				min_val[i] = -128;
		}
	}

	int8x16_t *pi8x16_max_val = (int8x16_t *)max_val;
	int8x16_t *pi8x16_min_val = (int8x16_t *)min_val;
	int8x16_t i8x16_max_val = *pi8x16_max_val;
	int8x16_t i8x16_min_val = *pi8x16_min_val;

	int loop_count = i4_n >> 4;
	int i4_loop = 0;

	for(i4_loop = 0; i4_loop < loop_count; i4_loop++)
	{
		int8x16_t i8x16_a1 = *pi8x16_a1; pi8x16_a1++;
		int8x16_t i8x16_a2 = *pi8x16_a2; pi8x16_a2++;
		int8x16_t i8x16_b1 = *pi8x16_b1; pi8x16_b1++;
		int8x16_t i8x16_b2 = *pi8x16_b2; pi8x16_b2++;

		int8x16_t i8x16_s1 = vqaddq_s8(i8x16_a1, i8x16_b1);
		int8x16_t i8x16_s2 = vqaddq_s8(i8x16_a2, i8x16_b2);

		i8x16_s1   = vmaxq_s8(i8x16_s1, i8x16_min_val);
		i8x16_s2   = vmaxq_s8(i8x16_s2, i8x16_min_val);

		*pi8x16_s1 = vminq_s8(i8x16_s1, i8x16_max_val);		
		*pi8x16_s2 = vminq_s8(i8x16_s2, i8x16_max_val);		

		pi8x16_s1++;
		pi8x16_s2++;
	}

	IO_DATA_TYPE *pi8_aa1 = (IO_DATA_TYPE *)pi8x16_a1;
	IO_DATA_TYPE *pi8_bb1 = (IO_DATA_TYPE *)pi8x16_b1;
	IO_DATA_TYPE *pi8_aa2 = (IO_DATA_TYPE *)pi8x16_a2;
	IO_DATA_TYPE *pi8_bb2 = (IO_DATA_TYPE *)pi8x16_b2;
	IO_DATA_TYPE *pi8_ss1 = (IO_DATA_TYPE *)pi8x16_s1;
	IO_DATA_TYPE *pi8_ss2 = (IO_DATA_TYPE *)pi8x16_s2;

	for(i4_loop = (loop_count << 4); i4_loop < i4_n; i4_loop++)
	{
		IO_DATA_TYPE i8_a1 = *pi8_aa1; pi8_aa1++;
		IO_DATA_TYPE i8_a2 = *pi8_aa2; pi8_aa2++;
		IO_DATA_TYPE i8_b1 = *pi8_bb1; pi8_bb1++;
		IO_DATA_TYPE i8_b2 = *pi8_bb2; pi8_bb2++;

		short i2_s1 = i8_a1 + i8_b1;
		short i2_s2 = i8_a2 + i8_b2;

		*pi8_ss1 = (i2_s1 > max_val[0]) ? max_val[0] : ((i2_s1 < min_val[0]) ? min_val[0]: i2_s1);
		*pi8_ss2 = (i2_s2 > max_val[0]) ? max_val[0] : ((i2_s2 < min_val[0]) ? min_val[0]: i2_s2);

		pi8_ss1++;
		pi8_ss2++;
	}

	memcpy(pi8_s1,ptr_s1,i4_n*sizeof(IO_DATA_TYPE));
	memcpy(pi8_s2,ptr_s2,i4_n*sizeof(IO_DATA_TYPE));

	free(ptr_a1);
	free(ptr_a2);
	free(ptr_b1);
	free(ptr_b2);
	free(ptr_s1);
	free(ptr_s2);

	return;
}

#else

void eltwise_add(IO_DATA_TYPE *pi8_a1, IO_DATA_TYPE *pi8_a2, IO_DATA_TYPE *pi8_b1, IO_DATA_TYPE *pi8_b2, IO_DATA_TYPE *pi8_s1, IO_DATA_TYPE *pi8_s2, int i4_n, int relu_en, int io_6bit)
{

	void *ptr_a1 = malloc(i4_n*sizeof(IO_DATA_TYPE));
	void *ptr_a2 = malloc(i4_n*sizeof(IO_DATA_TYPE));
	void *ptr_b1 = malloc(i4_n*sizeof(IO_DATA_TYPE));
	void *ptr_b2 = malloc(i4_n*sizeof(IO_DATA_TYPE));
	void *ptr_s1 = malloc(i4_n*sizeof(IO_DATA_TYPE));
	void *ptr_s2 = malloc(i4_n*sizeof(IO_DATA_TYPE));

	memcpy(ptr_a1,pi8_a1,i4_n*sizeof(IO_DATA_TYPE));
	memcpy(ptr_a2,pi8_a2,i4_n*sizeof(IO_DATA_TYPE));
	memcpy(ptr_b1,pi8_b1,i4_n*sizeof(IO_DATA_TYPE));
	memcpy(ptr_b2,pi8_b2,i4_n*sizeof(IO_DATA_TYPE));

	IO_DATA_TYPE *p_a1 = (IO_DATA_TYPE *)ptr_a1;
	IO_DATA_TYPE *p_a2 = (IO_DATA_TYPE *)ptr_a2;
	IO_DATA_TYPE *p_b1 = (IO_DATA_TYPE *)ptr_b1;
	IO_DATA_TYPE *p_b2 = (IO_DATA_TYPE *)ptr_b2;
	IO_DATA_TYPE *p_s1 = (IO_DATA_TYPE *)ptr_s1;
	IO_DATA_TYPE *p_s2 = (IO_DATA_TYPE *)ptr_s2;

	int loop_count = i4_n;
	IO_DATA_TYPE max_val, min_val;

	if(io_6bit == 1)
	{
		max_val = 31;
		if(relu_en)
			min_val = 0;
		else
			min_val = -32;
	}
	else
	{	
		max_val = 127;
		if(relu_en)
			min_val = 0;
		else
			min_val = -128;
	}	

	for(int loop_id = 0; loop_id < loop_count; loop_id++)
	{
		IO_DATA_TYPE a1 = *p_a1; p_a1++;
		IO_DATA_TYPE a2 = *p_a2; p_a2++;
		IO_DATA_TYPE b1 = *p_b1; p_b1++;
		IO_DATA_TYPE b2 = *p_b2; p_b2++;

		short s1 = a1 + b1;
		short s2 = a2 + b2;

		IO_DATA_TYPE sum1 = (s1 > max_val) ? max_val : ((s1 < min_val) ? min_val: s1);
		IO_DATA_TYPE sum2 = (s2 > max_val) ? max_val : ((s2 < min_val) ? min_val: s2);

		*p_s1 = sum1; p_s1++;
		*p_s2 = sum2; p_s2++;
	}

	memcpy(pi8_s1,ptr_s1,i4_n*sizeof(IO_DATA_TYPE));
	memcpy(pi8_s2,ptr_s2,i4_n*sizeof(IO_DATA_TYPE));

	free(ptr_a1);
	free(ptr_a2);
	free(ptr_b1);
	free(ptr_b2);
	free(ptr_s1);
	free(ptr_s2);

	return;
}


#endif //#ifdef __SDSOC

#define ELTWADD_PTHREAD 1

#if ELTWADD_PTHREAD

#define NUM_ELTW_THREADS 4
struct _eltw_layer
{
	void *in_ptr1;
	void *in_ptr2;
	void *in_ptr3;
	void *in_ptr4;
	void *out_ptr1;
	void *out_ptr2;
	int size;
	int relu_en;
	int io_6bit;
	int layer_done;
};
typedef struct _eltw_layer eltwLayer;

//# ELTWISEADD Thread Routine
void * eltwaddRoutine_multiplethread(void *arguments)
{
	//	std::cout << "\n** NMS Thread " << std::endl;
	eltwLayer *layerArgs = (eltwLayer *)arguments;
	layerArgs->layer_done = 0;

	//# Call NMS wrapper
	eltwise_add(
			(IO_DATA_TYPE*)layerArgs->in_ptr1,
			(IO_DATA_TYPE*)layerArgs->in_ptr2,
			(IO_DATA_TYPE*)layerArgs->in_ptr3,
			(IO_DATA_TYPE*)layerArgs->in_ptr4,
			(IO_DATA_TYPE*)layerArgs->out_ptr1,
			(IO_DATA_TYPE*)layerArgs->out_ptr2,
			layerArgs->size,
			layerArgs->relu_en,
			layerArgs->io_6bit
	);

	layerArgs->layer_done = 1;

#if ELTWADD_PTHREAD
	pthread_exit(NULL);
#endif

	return NULL;
}


void EltwiseaddWrapper(IO_DATA_TYPE* inArray1, IO_DATA_TYPE* inArray2, IO_DATA_TYPE* inArray3, IO_DATA_TYPE* inArray4, IO_DATA_TYPE* outArray1, IO_DATA_TYPE* outArray2, int *scalar_eltwiseadd_args)
{

	int nsize   = scalar_eltwiseadd_args[0];
	int io_6bit = scalar_eltwiseadd_args[1];
	int relu_en = scalar_eltwiseadd_args[2];
	int eltwadd_done = 0;

	pthread_t eltwaddThread[NUM_ELTW_THREADS];

	eltwLayer layerArgs[NUM_ELTW_THREADS];

	nsize = nsize/NUM_ELTW_THREADS;

	for(int th=0;th<NUM_ELTW_THREADS;th++)
	{
		layerArgs[th].in_ptr1  = (void *)inArray1;
		layerArgs[th].in_ptr2  = (void *)inArray2;
		layerArgs[th].in_ptr3  = (void *)inArray3;
		layerArgs[th].in_ptr4  = (void *)inArray4;
		layerArgs[th].out_ptr1 = (void *)outArray1;
		layerArgs[th].out_ptr2 = (void *)outArray2;
		layerArgs[th].size     = nsize;
		layerArgs[th].io_6bit  = io_6bit;
		layerArgs[th].relu_en  = relu_en;

		//# Call Eltwadd wrapper
		pthread_create(&eltwaddThread[th], NULL, eltwaddRoutine_multiplethread, (void *)&layerArgs[th]);

		inArray1  = inArray1 + nsize;
		inArray2  = inArray2 + nsize;
		inArray3  = inArray3 + nsize;
		inArray4  = inArray4 + nsize;
		outArray1 = outArray1 + nsize;
		outArray2 = outArray2 + nsize;
	}

	while(1)
	{
		for(int th=0;th<NUM_ELTW_THREADS;th++)
		{
			//# Check for thread completion
			if(layerArgs[th].layer_done)
			{
				int eltwaddRet = pthread_join(eltwaddThread[th], NULL);
				eltwadd_done = layerArgs[th].layer_done;
			}
		}


		for(int th=0;th<NUM_ELTW_THREADS;th++)
		{
			eltwadd_done = eltwadd_done & layerArgs[th].layer_done;
		}

		if(eltwadd_done)
		{
			break;
		}
	}

	//fprintf(stderr,"eltwadd thread done");
}

#else

void EltwiseaddWrapper(IO_DATA_TYPE* inArray1, IO_DATA_TYPE* inArray2, IO_DATA_TYPE* inArray3, IO_DATA_TYPE* inArray4, IO_DATA_TYPE* outArray1, IO_DATA_TYPE* outArray2, int *scalar_eltwiseadd_args)
{

	int nsize   = scalar_eltwiseadd_args[0];
	int io_6bit = scalar_eltwiseadd_args[1];
	int relu_en = scalar_eltwiseadd_args[2];

	eltwise_add(inArray1, inArray2, inArray3, inArray4, outArray1, outArray2, nsize, relu_en, io_6bit);

#if 0
#ifdef __SDSOC
	if(io_6bit == 1)
	{
		eltwise_add_i8_6bit(inArray1, inArray2, inArray3, inArray4, outArray1, outArray2, nsize, relu_en);
	}
	else
	{
		eltwise_add_i8_8bit(inArray1, inArray2, inArray3, inArray4, outArray1, outArray2, nsize, relu_en);
	}
#else
	if(io_6bit == 1)
	{
		eltwise_add_6bit(inArray1, inArray2, inArray3, inArray4, outArray1, outArray2, nsize, relu_en);
	}
	else
	{
		eltwise_add_8bit(inArray1, inArray2, inArray3, inArray4, outArray1, outArray2, nsize, relu_en);
	}
#endif
#endif

}
#endif  //#if ELTWADD_PTHREAD

#if 0
int main(int argc, char* argv[])
{
	if(argc < 2) {
		std::cout << "./run.elf Number " << std::endl;
		exit(0);
	}
	const int N = atoi(argv[1]);
	char *pi8_a1 = new char[N];
	char *pi8_a2 = new char[N];
	char *pi8_b1 = new char[N];
	char *pi8_b2 = new char[N];
	char *pi8_s1 = new char[N];
	char *pi8_s2 = new char[N];

	clock_t start, end, accum;
	bool pass;
	int runs;

	start = clock();
	eletwise_add_i8_8bit(pi8_a1, pi8_a2, pi8_b1, pi8_b2, pi8_s1, pi8_s2, N);
	end = clock();
	std::cout << "Execution time for eletwise_add_i8: " << (end - start)  << " " << argv[1] << " elements " <<  std::endl;

	start = clock();
	eletwise_add_i8_6bit(pi8_a1, pi8_a2, pi8_b1, pi8_b2, pi8_s1, pi8_s2, N);
	end = clock();
	std::cout << "Execution time for eletwise_add_i8: " << (end - start)  << " " << argv[1] << " elements " <<  std::endl;	
}
#endif
