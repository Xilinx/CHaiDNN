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

#include "../include/hw_settings.h"

#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)
#define ConvertToFP(fVal, iPart, fbits)	((int)((iPart<<fbits) + ((fVal-(float)iPart))*(1<<fbits)))

#if !SINGLE_IO_PORT
void PermuteWrapper(IO_DATA_TYPE *input1, IO_DATA_TYPE *input2, IO_DATA_TYPE *sds_input1, IO_DATA_TYPE *sds_input2, IO_DATA_TYPE *output, int *scalar_permute_args)
{

	int inp_planes  = scalar_permute_args[0];
	int height      = scalar_permute_args[1];
	int width       = scalar_permute_args[2];
	int fbits_input = scalar_permute_args[3];
	int batch_size  = scalar_permute_args[4];

	int align_inp_planes = AlignSize(inp_planes, PLANE_PACK);

	int output_off = 0;
	int input_off = 0;

	memcpy(input1,sds_input1,height*width*(align_inp_planes)*batch_size*sizeof(IO_DATA_TYPE));
	memcpy(input2,sds_input2,height*width*(align_inp_planes)*batch_size*sizeof(IO_DATA_TYPE));


	int pl_count =  align_inp_planes/PLANE_PACK;
	if( (pl_count > 1) && (( (pl_count/2)*2 ) < pl_count) )
	{
		pl_count += 1;
	}

	pl_count = pl_count * PLANE_PACK;

	int batch_cnt1;
	int batch_cnt2;
	int batch_cnt3;

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

	int pl1 = 0;

	int batch_size_one_enable =  scalar_permute_args[5];
	int batch_size_loopcnt;
	if(batch_size_one_enable)
	{
		batch_size_loopcnt = 1;
	}
	else
	{
		batch_size_loopcnt = batch_cnt2;
	}


#if 0
	//Unpack and rearrange pixel interleave:[height*width][pl]
	for(int pl=0; pl< pl_count/2; pl+=PLANE_PACK)
	{
		for(int packch=0; packch<PLANE_PACK; packch++)
		{
			input_off = pl*height*width+packch;

			for(int size=0; size< height*width; size++)
			{
				IO_DATA_TYPE temp = input1[input_off];

				if((pl1+packch) < inp_planes)
				{
					//float tmp_f = (float)temp/(float)(1 << (fbits_input));
					//output_unpack[output_off] = tmp_f;
					//output_off++;
					output_off = (pl1+packch)+size*inp_planes;
					output[output_off] = temp;//tmp_f;
				}
				input_off+=PLANE_PACK;
			}
		}

		pl1+=PLANE_PACK;

		if(pl1 < align_inp_planes)
		{
			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				input_off = pl*height*width+packch;

				for(int size=0; size< height*width; size++)
				{
					IO_DATA_TYPE temp = input2[input_off];
					if((pl1+packch) < inp_planes)
					{
						//float tmp_f = (float)temp/(float)(1 << (fbits_input));
						//output_unpack[output_off] = tmp_f;
						//output_off++;

						output_off = (pl1+packch)+size*inp_planes;
						output[output_off] = temp;//tmp_f;
					}
					input_off+=PLANE_PACK;
				}
			}
		}

		pl1+=PLANE_PACK;

	}
#endif

#if 1
	output_off = 0;
	//IO_DATA_TYPE* output1 = (IO_DATA_TYPE*) malloc (inp_planes*height*width * sizeof(IO_DATA_TYPE));
	//Unpack and rearrange pixel interleave:[height*width][pl]
	for(int size=0; size<(height*width); size++)
	{

		for(int batch_id2=0; batch_id2<batch_size_loopcnt; batch_id2++)
		{
			pl1 = 0;
			int batch_offset = pl_count*height*width*batch_id2*batch_cnt1;
			for(int pl=0; pl<(pl_count/2); pl+=PLANE_PACK)
			{
				input_off = batch_offset+(pl*height*width+size*PLANE_PACK)*batch_cnt1;

				for(int packch=0; packch<PLANE_PACK*batch_cnt1; packch++)
				{
					//fprintf(stderr,"input_off=%d\n", input_off);
					IO_DATA_TYPE temp = input1[input_off++];
					if((pl1+packch/batch_cnt1) < inp_planes)
					{
						output[output_off++] = temp;
					}
				}

				pl1+=PLANE_PACK;

				input_off = batch_offset+(pl*height*width+size*PLANE_PACK)*batch_cnt1;

				if(pl1 < align_inp_planes)
				{
					for(int packch=0; packch<PLANE_PACK*batch_cnt1; packch++)
					{
						//fprintf(stderr,"input_off=%d\n", input_off);
						IO_DATA_TYPE temp = input2[input_off++];
						if((pl1+packch/batch_cnt1) < inp_planes)
						{
							output[output_off++] = temp;
						}
					}
				}
				pl1+=PLANE_PACK;
			}  //for(int pl=0; pl<(pl_count/2); pl+=PACK_ELEMS)
		} //for(int batch_id2=0; batch_id2<batch_size_loopcnt; batch_id2++)

	}

#if 0
	for(int pl=0; pl< inp_planes*height*width; pl++)
	{
		if(output1[pl] != output[pl])
		{
			short hls_val1 = output[pl];
			float float_val1 = ((float)hls_val1)/(1 << fbits_input);
			short hls_val2 = output1[pl];
			float float_val2 = ((float)hls_val2)/(1 << fbits_input);
			fprintf(stderr,"pl=%d, output = %f, output1 = %f\n", pl, float_val1, float_val2);
		}
	}
#endif

#endif

#if 0
	//Unpack and rearrange plane interleave:[pl][height*width]
	for(int pl=0; pl< pl_count/2; pl+=PLANE_PACK)
	{
		for(int packch=0; packch<PLANE_PACK; packch++)
		{
			input_off = pl*height*width+packch;

			for(int size=0; size< height*width; size++)
			{
				IO_DATA_TYPE temp = input1[input_off];

				if((pl1+packch) < inp_planes)
				{
					float tmp_f = (float)temp/(float)(1 << (fbits_input));
					output[output_off++] = tmp_f;
				}
				input_off+=PLANE_PACK;
			}
		}

		pl1+=PLANE_PACK;

		if(pl1 < align_inp_planes)
		{
			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				input_off = pl*height*width+packch;

				for(int size=0; size< height*width; size++)
				{
					IO_DATA_TYPE temp = input2[input_off];
					if((pl1+packch) < inp_planes)
					{
						float tmp_f = (float)temp/(float)(1 << (fbits_input));
						output[output_off++] = tmp_f;
					}
					input_off+=PLANE_PACK;
				}
			}
		}

		pl1+=PLANE_PACK;

	}
#endif

#if 0
	int permute_off = 0;
	output_off = 0;
	int output_offset = height*width;

	for(int ht=0; ht< height; ht++)
	{
		for(int wt=0; wt< width; wt++)
		{
			output_off  = ht*width+wt;
			for(int pl=0; pl< inp_planes; pl++)
			{
				float float_val = output_unpack[output_off];
				output1[permute_off] = float_val;
				permute_off++;
				output_off+=output_offset;

			}
		}
	}


#endif

}
#else
void PermuteWrapper(IO_DATA_TYPE *input1, IO_DATA_TYPE *sds_input1, IO_DATA_TYPE *output, int *scalar_permute_args)
{

	int inp_planes  = scalar_permute_args[0];
	int height      = scalar_permute_args[1];
	int width       = scalar_permute_args[2];
	int fbits_input = scalar_permute_args[3];
	int batch_size  = scalar_permute_args[4];

	int align_inp_planes = AlignSize(inp_planes, PLANE_PACK);

	int output_off = 0;
	int input_off = 0;

	memcpy(input1,sds_input1,height*width*(align_inp_planes)*batch_size*sizeof(IO_DATA_TYPE));

	int pl_count =  align_inp_planes/PLANE_PACK;
	if( (pl_count > 1) && (( (pl_count/2)*2 ) < pl_count) )
	{
		pl_count += 1;
	}

	pl_count = pl_count * PLANE_PACK;

	int batch_cnt1;
	int batch_cnt2;
	int batch_cnt3;

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

	int pl1 = 0;

	int batch_size_one_enable =  scalar_permute_args[5];
	int batch_size_loopcnt;
	if(batch_size_one_enable)
	{
		batch_size_loopcnt = 1;
	}
	else
	{
		batch_size_loopcnt = batch_cnt2;
	}


	output_off = 0;
	//IO_DATA_TYPE* output1 = (IO_DATA_TYPE*) malloc (inp_planes*height*width * sizeof(IO_DATA_TYPE));
	//Unpack and rearrange pixel interleave:[height*width][pl]
	for(int size=0; size<(height*width); size++)
	{

		for(int batch_id2=0; batch_id2<batch_size_loopcnt; batch_id2++)
		{
			pl1 = 0;
			int batch_offset = pl_count*height*width*batch_id2*batch_cnt1;
			for(int pl=0; pl<(pl_count/2); pl+=PLANE_PACK)
			{
				input_off = batch_offset+(pl*height*width+size*PLANE_PACK)*batch_cnt1;

				for(int packch=0; packch<PLANE_PACK*batch_cnt1; packch++)
				{
					//fprintf(stderr,"input_off=%d\n", input_off);
					IO_DATA_TYPE temp = input1[input_off++];
					if((pl1+packch/batch_cnt1) < inp_planes)
					{
						output[output_off++] = temp;
					}
				}

				pl1+=PLANE_PACK;

			}  //for(int pl=0; pl<(pl_count/2); pl+=PACK_ELEMS)
		} //for(int batch_id2=0; batch_id2<batch_size_loopcnt; batch_id2++)

	}

}
#endif  //#if !SINGLE_IO_PORT


#if 0
void PermuteWrapper(IO_DATA_TYPE *input1, IO_DATA_TYPE *input2, IO_DATA_TYPE *sds_input1, IO_DATA_TYPE *sds_input2, float *output, float *output_unpack, int *scalar_permute_args)
{

	int inp_planes  = scalar_permute_args[0];
	int height      = scalar_permute_args[1];
	int width       = scalar_permute_args[2];

	int align_inp_planes = AlignSize(inp_planes, PLANE_PACK);
	int fbits_input =  scalar_permute_args[3];//8;//5;
	int output_off = 0;

	memcpy(input1,sds_input1,height*width*(align_inp_planes)*sizeof(IO_DATA_TYPE));
	memcpy(input2,sds_input2,height*width*(align_inp_planes)*sizeof(IO_DATA_TYPE));
	int output_off1;


	int pl_count =  align_inp_planes/PLANE_PACK;
	if( ( (pl_count/2)*2 ) < pl_count)
	{
		pl_count += 1;
	}

	pl_count = pl_count * PLANE_PACK;

	int pl1 = 0;

	for(int pl=0; pl< pl_count/2; pl+=PLANE_PACK)
	{
		for(int packch=0; packch<PLANE_PACK; packch++)
		{
			int input_off = pl*height*width+packch;

			for(int size=0; size< height*width; size++)
			{
				IO_DATA_TYPE temp = input1[input_off];
				//fprintf(stderr,"input_off=%d\n", input_off);
				if((pl1+packch) < inp_planes)
				{
					float tmp_f = (float)temp/(float)(1 << (fbits_input));
					//output_unpack[output_off] = tmp_f;
					//output_off++;
					output_off1 = (pl1+packch)+size*inp_planes;
					output[output_off1] = tmp_f;
					//output_unpack1[output_off1] = tmp_f;
					//fprintf(stderr,"output_off1=%d\n", output_off1);
				}
				input_off+=PLANE_PACK;
			}
		}

		pl1+=PLANE_PACK;

		if(pl1 < align_inp_planes)
		{
			for(int packch=0; packch<PLANE_PACK; packch++)
			{
				int input_off = pl*height*width+packch;

				for(int size=0; size< height*width; size++)
				{
					IO_DATA_TYPE temp = input2[input_off];
					//fprintf(stderr,"input_off=%d\n", input_off);
					if((pl1+packch) < inp_planes)
					{
						float tmp_f = (float)temp/(float)(1 << (fbits_input));
						//output_unpack[output_off] = tmp_f;
						//output_off++;

						//output_off1 = (pl1+packch)*size;
						output_off1 = (pl1+packch)+size*inp_planes;
						//output_unpack1[output_off1] = tmp_f;
						output[output_off1] = tmp_f;
						//fprintf(stderr,"output_off1=%d\n", output_off1);
					}
					input_off+=PLANE_PACK;
				}
			}
		}

		pl1+=PLANE_PACK;

	}
}
#endif
