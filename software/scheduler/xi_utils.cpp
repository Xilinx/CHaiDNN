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

//# Re-arranges data for HW FC
void fc_inputdatawrite(short *conv_out, short *conv_out1, int height, int width, int indepth, bool relu,
		short *kernel_output, char *output_file_path)
{
	int ofms  = height*width;
	int packs = ofms*PACK_ELEMS;
	int depth = indepth/(PACK_ELEMS<<1);

#if XI_UTILS_CONSOLE_TEXT
	std::cout << ofms << " x " << packs << " x " << depth << std::endl;
#endif
	
	int counter = 0;
	for(int t = 0; t < depth; ++t)
	{
		for(int pack = 0; pack < PACK_ELEMS; ++pack)
		{
			for(int i = 0; i < ofms; ++i)
			{
				kernel_output[counter] = conv_out[t*packs + i*PACK_ELEMS + pack];
				counter++;
			}
		}
		for(int pack = 0; pack < PACK_ELEMS; ++pack)
		{
			for(int i = 0; i < ofms; ++i)
			{
				kernel_output[counter] = conv_out1[t*packs + i*PACK_ELEMS + pack];
				counter++;
			}
		}
	}
	
#if XI_UTILS_CONSOLE_TEXT
	std::cout << "\n** Re-arrange : Done " << std::endl;
#endif

}//# fc_inputdatawrite

//# Re-arranges data for HW deconv
void DeconvInReArrange(short *input1, short *input2, short *output, int height, int width, int inp_planes)
{
	
#if XI_UTILS_CONSOLE_TEXT
	std::cout << "\n[DECON] Height : %d Width : %d In Planes : %d\n", height, width, inp_planes);
#endif

	int hxw = height*width;
	int idx1 = 0;
	int idx2 = 0;
	//# Re-arrange input data for Deconv
	for(int pl16 = 0; pl16 < inp_planes; pl16 += 16)
	{
		for(int size = 0; size < hxw; size++)
		{
			for(int pl = 0; pl < 16; pl++)
			{
				short a;
				if(pl < 8)
				{
					a = input1[idx1++];
				}
				else
				{
					a = input2[idx2++];
				}

				int outidx = size + (hxw*pl) + (hxw*pl16);
				output[outidx] = a;
			}
		}
	}
}//# DeconvInReArrange
