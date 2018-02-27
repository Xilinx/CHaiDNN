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

void xiInit( char *dirpath,  char* prototxt,  char* caffemodel,  char* meanfile, int mean_type, 
			   std::vector<xChangeLayer> (&jobQueue)[NUM_IMG], bufPtrs ptrsList, int resize_h, int resize_w);
			   
unsigned short int xiExec(std::vector<xChangeLayer> hwQueue[NUM_IMG], void *inptr, void *outptr, int inp_bytes);

void xiRelease(bufPtrs ptrsList);


#endif//_XI_INTERFACE_HPP_
