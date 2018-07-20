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

#ifndef _XI_INTERFACE_HPP_
#define _XI_INTERFACE_HPP_

//#include <cstdint>

#include "../include/xchange_structs.hpp" 

struct _io_layer_info
{
	std::string inlayer_exectype;
	int inlayer_sizebytes;
	std::string outlayer_exectype;
	int outlayer_sizebytes;
	int num_in_bufs;
	int num_out_bufs;

	int in_height;
	int in_width;
	int in_bw;
	int in_fbits;
	int in_channel;
	int in_depth;
	float th_in;
	int quant_scheme_flag;
	int out_size;

	kernel_type_e out_kerType;

};
typedef struct _io_layer_info io_layer_info;

void *xiInit(char *dirpath,  char* prototxt,  char* caffemodel,
		  _io_layer_info *io_layer_info_ptr, int numImg_to_process, bool layer1_or_not, std::string start_layer, std::string end_layer);

void xiExec(void *handle, std::vector<void *> input, std::vector<void *> output);

void xiRelease(void *chaihandle);

void getPerfInfo(void *handle);

#endif//_XI_INTERFACE_HPP_
