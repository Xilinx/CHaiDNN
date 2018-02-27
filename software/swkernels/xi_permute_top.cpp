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

#define PACK_ELEMS 8
#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)
#define ConvertToFP(fVal, iPart, fbits)	((int)((iPart<<fbits) + ((fVal-(float)iPart))*(1<<fbits)))

void PermuteWrapper(short *input1, short *input2, short *sds_input1, short *sds_input2, float *output, float *output_unpack, int *scalar_permute_args)
{

	int inp_planes  = scalar_permute_args[0];
	int height      = scalar_permute_args[1];
	int width       = scalar_permute_args[2];

	int align_inp_planes = AlignSize(inp_planes, PACK_ELEMS);
	int fbits_input =  scalar_permute_args[3];//8;//5;
	int output_off = 0;

	memcpy(input1,sds_input1,height*width*(align_inp_planes)*sizeof(short));
	memcpy(input2,sds_input2,height*width*(align_inp_planes)*sizeof(short));

	//float *output_unpack = (float *)malloc(align_inp_planes*height*width*sizeof(float));

	int pl_count =  align_inp_planes/PACK_ELEMS;
	if( ( (pl_count/2)*2 ) < pl_count)
	{
		pl_count += 1;
	}

	pl_count = pl_count * PACK_ELEMS;

	int pl1 = 0;

	for(int pl=0; pl< pl_count/2; pl+=PACK_ELEMS)
	{
		for(int packch=0; packch<PACK_ELEMS; packch++)
		{
			int input_off = pl*height*width+packch;

			for(int size=0; size< height*width; size++)
			{
				short temp = input1[input_off];
				if((pl1+packch) < inp_planes)
				{
					float tmp_f = (float)temp/(float)(1 << (fbits_input));
					output_unpack[output_off] = tmp_f;
					output_off++;
				}
				input_off+=PACK_ELEMS;
			}
		}

		pl1+=PACK_ELEMS;

		if(pl1 < align_inp_planes)
		{
			for(int packch=0; packch<PACK_ELEMS; packch++)
			{
				int input_off = pl*height*width+packch;

				for(int size=0; size< height*width; size++)
				{
					short temp = input2[input_off];
					if((pl1+packch) < inp_planes)
					{
						float tmp_f = (float)temp/(float)(1 << (fbits_input));
						output_unpack[output_off] = tmp_f;
						output_off++;
					}
					input_off+=PACK_ELEMS;
				}
			}
		}

		pl1+=PACK_ELEMS;

	}


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
				output[permute_off] = float_val;
				permute_off++;
				output_off+=output_offset;
			}
		}
	}

}

