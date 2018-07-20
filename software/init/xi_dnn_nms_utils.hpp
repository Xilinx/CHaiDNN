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

#ifndef _XCHANGE_NMS_UTILS_HPP_
#define _XCHANGE_NMS_UTILS_HPP_

void nms_layerinputdatawrite(float *input_tb, int inputsize, vector< float >box_buf)
{
#if 0
#if DATA_IN_BINARY
	FILE *fp = fopen(path,"rb");
#else
	FILE *fp = fopen(path,"r");
#endif

	if(fp==NULL)
	{
		fprintf(stderr, "%s : File not found ******\n", path);
	}
#endif

#if DATA_IN_BINARY
	//fread(input_tb, sizeof(float)*inputsize, 1, fp);
#else
	float float_val;
	for(int i = 0; i < inputsize; i++)
	{
		//fscanf(fp, "%f ", &float_val);
		float_val = box_buf[i];
		input_tb[i] = float_val;
	}
#endif

	//fclose(fp);

}

#endif //_XCHANGE_NMS_UTILS_HPP_
