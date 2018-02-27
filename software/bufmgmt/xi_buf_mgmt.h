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

#ifndef __XI_BUF_MGMT_HPP__
#define __XI_BUF_MGMT_HPP__

#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <iterator>

#ifdef __SDSOC
#include "sds_lib.h"
#endif

//# Include from xTract
#include "../xtract/xgraph_opt.hpp"

//# Including xChage_structs
#include "../include/hw_settings.h"
#include "../include/xchange_structs.hpp"

using namespace std;

#define EN_PRINT 0

//# Enable prints in conctant buffer module
#define EN_CONST_PRINT			0
//# Enable prints in IO module
#define EN_IO_PRINT				0
//# Enable prints in offset addition block
#define EN_OFF_PRINT			0
//# Enable prints in memory allocation block
#define EN_ADDR_PRINT			0
//# Enable prints in shapetosize module
#define EN_SHAPE2SIZE_PRINT		0
//# Enable prints in shapetosize module
#define EN_PEER2SIZE_PRINT		0
//# Enable prints in checkstack module
#define EN_STACK_PRINT			0

//# Enable prints in free buffer module
#define EN_FREE_PRINT			0

//# Enable "." prints on terminal
#define EN_CONSOLE_TXT_BUFMGMT_DBG 0

//# Cacheable memory limit 512KB
#define CACHE_LIMIT				524288

struct _ptr_pair
{
	void *ptr[INPUT_PORTS];
};
typedef _ptr_pair ptr_pair;

//# Type of memory (sds/malloc)
enum _mem_type
{
	NON_CACHEABLE,
	CACHEABLE,
	MALLOC
};
typedef _mem_type mem_type_e;

//# Creates the memory of given size and returns the pointer
void* xiMemCreate(
		unsigned int size,
		mem_type_e mem_flag,
		bufPtrs &ptrList
);

//# check available stack and creates memory
void checkStack(
		int size,
		int& size_acc,
		void* &base_ptr,
		bufPtrs &ptrList,
		mem_type_e mem_flag
);

//# Add offset to the address
template<typename T>
void *addOffset(
		void *base_addr,
		int offset
);

//# Constant buffer Creation
void xiConstBufCreate(
		vector<XlayerData> xlayer_seq,
		vector<xChangeLayer> lay_obj[NUM_IMG],
		bufPtrs &ptrList
);

//# IO buffer Creation
void xiIOBuffCreate(
		vector<XlayerData> xlayer_seq,
		vector<xChangeLayer> lay_obj[NUM_IMG],
		bufPtrs &ptrList
);
// xiIOBuffCreate

//# Freeing Memory pointers
void freeMemory(bufPtrs &ptrList);
// bufMgmt_print

//# Prints all address
void bufMgmt_print(
		vector<XlayerData> xlayer_seq,
		vector<xChangeLayer> lay_obj[NUM_IMG]
);

//# Main Buffer creation function
void xiBuffCreate(
		vector<XlayerData> xlayer_seq,
		vector<xChangeLayer> (&lay_obj)[NUM_IMG],
		bufPtrs &ptrList
);
// xiBuffCreate

#endif      // __XI_BUF_MGMT_HPP__
