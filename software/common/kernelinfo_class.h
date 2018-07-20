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

#ifndef __KERNELINFO_CLASS_H__
#define __KERNELINFO_CLASS_H__

#include <iostream>
#include <string>
#include <map>
#include <cmath>
#include <vector>
#include <cfloat>
#include <stdlib.h>
#include <algorithm>
#include <iterator>
#include "../include/xchange_structs.hpp"
#include "../include/hw_settings.h"

using namespace std;

enum _opcode_num{

	OPCODE_CONV=0,
	OPCODE_ReLU=0,
	OPCODE_CONV_ReLU=1,
	//OPCODE_Concat=0,
	OPCODE_TMP_BN = 0,
	OPCODE_TMP_POWER = 0,
	OPCODE_LRN_INTER_SOS=1,
	OPCODE_LRN_INTER_PNS=2,
	OPCODE_MAX_POOL=100,
	OPCODE_AVR_POOL=101,
	OPCODE_POOL=100,
	OPCODE_TMP_LRN=0,
	OPCODE_DECONV=103,
	OPCODE_SOFTMAX=200,
	OPCODE_ARGMAX=0,
	OPCODE_FC=102,
	OPCODE_PERM=206,
	OPCODE_CROP=205,
	OPCODE_NMS=201,
	OPCODE_L2NORM=0,
	OPCODE_BN = 10,
	OPCODE_ELTWISE = 11,
	OPCODE_FUSE_BN_CONV = 12,
	OPCODE_CRELU = 18,
	OPCODE_3D_CONV = 19,
	OPCODE_SEPARABLE_CONV=40,
	OPCODE_SW_ELTWISE_ADD=41,
	OPCODE_XCUSTOM = 204,
	OPCODE_XPACK = 202,
	OPCODE_XUNPACK=203,
	OPCODE_PARALLEL_CONV_FUSE=42,
	OPCODE_SW_SOFTMAX=200,
	OPCODE_SW_NMS=201,
	OPCODE_SW_PERMUTE=207,
	OPCODE_SW_L2NORM=208,
	OPCODE_AVRPOOL2CONV=21,
	OPCODE_FC2CONV=22,
	OPCODE_POOL_CONV2CONV=23,
	OPCODE_POOL2CONV=20,
	OPCODE_SOFTMAX_XPACK=209,
	OPCODE_PERMUTE_XUNPACK=210

};

typedef _opcode_num opcode_num_e;



enum moudle_type {
	HW_CONV = 0,
	HW_POOL = 1,
	HW_FC = 2,
	HW_DECONV = 3,
	SW_FC = 4,
	SW_SOFTMAX = 100,
	SW_NMS = 101,
	SW_CROP = 102,
	SW_XCUSTOM=103,
	SW_XPACK=104,
	SW_XUNPACK=105,
	SW_PERMUTE=106,
	SW_L2NORMALIZE=107
};
typedef enum moudle_type _moudle_type_e;


enum datatype
{

	XI_INT8=0,
	XI_UINT8=1,
	XI_INT16=2,
	XI_UINT16=3,
	XI_INT32=4,
	XI_UINT32=5,
	XI_FLOAT=6
};

typedef enum datatype  _datatype_e;

enum datatype_size
{
	XI_FLOAT_S=5,
	XI_INT8_S=1,
	XI_INT16_S=2,
	XI_INT32_S=4,

};

typedef enum datatype_size  _datatype_size_e;

enum dimesion_type
{
	XI_BATCH = 0,
	XI_CHANNEL = 1,
	XI_WIDTH = 2,
	XI_HEIGHT = 3,
	XI_INTERLEAVED=4
};

enum separation
{
	XI_NONE = 0,
	XI_MODULO,
	XI_DIVISION
};
enum quant_scheme{
	QUNT_NONE=0,
	QUNT_FIXED,
	QUNT_INT
};
typedef quant_scheme _quant_scheme_e;
struct packinfo
{
	enum dimesion_type dim;
	enum separation sep;
	int mod_div_factor;
	bool mem_contiguous;
};

enum portwidth
{
	XI_16BIT_WIDTH = 16,
	XI_32BIT_WIDTH = 32,
	XI_64BIT_WIDTH = 64,
	XI_128BIT_WIDTH = 128   ///change
};

enum module_target
{
	FPGA =0,
	CPU,
	GPU
};
typedef module_target _module_target_e;


struct Module_info
{
	enum module_target module_target_info;
	_moudle_type_e module_type;
	string 	module_name; /// TODO abid needs to update the layer information based on the enum instead of layer name
	int num_total_ports;

};

enum port_type {
	XI_UNUSED=0,
	XI_INPUT,
	XI_OUTPUT,
	XI_WEIGHTS,
	XI_BIAS,
	XICUSTOM
};
enum memory_mapping{
	__MALLOC_TYPE=0,
	__SDS_CACHEABLE,
	__SDS_NON_CACHEABLE
};
struct Port_info
{
	//bool used;         // port sued for this opcode   //remove
	int port_idx;  // this is to signal number of unique pointers
	enum portwidth port_width;    //int portwidth  give value
	enum datatype data_type;
	_datatype_size_e size_datatype;
	vector<packinfo> pack_info_vec;
	enum port_type port_enum;  //unused, input, output, weight, bias, custom, etc.
	int memory_align_required;
	enum memory_mapping memory_map_info;
	_quant_scheme_e qant_scheme_type;

};
typedef struct Port_info Port_info_s;

struct Opcode_info
{
	Module_info      mega_module;
	opcode_num_e          op_code;

	//int              op_code;
	vector<Port_info> port_vec;   //single vec


};


/*struct Opcode_info
{
	Module_info      mega_module;
	int              op_code;
	int 			 mem_align_info;
	vector<Port_info> in_port_vec;   //single vec
	vector<Port_info> out_port_vec;
	vector<Port_info> wt_port_vec;
	vector<Port_info> bias_port_vec;
};*/


//#define NUM_OPCODES 2

struct datainfo{
	_datatype_e		t_s_data;
	_datatype_e		t_data;
};

typedef struct datainfo _datainfo_s;

class kernelInfo;
typedef void (kernelInfo::*kernelfunction)(int op_code, Opcode_info *arg_opcode);


class kernelInfo
{

public:

	kernelInfo();
	kernelInfo(string &quant_schem);

	int batch_size;

	map<opcode_num_e, Opcode_info>   opcode_info_map;
	map<_datatype_size_e,_datainfo_s> datainfo_map;
	void display_opcode_info(Opcode_info *arg_opcode);
	int get_num_ports(Opcode_info *arg_opcode, enum port_type port_tp);


	~kernelInfo();

};

/*class kernelInfo
{

public:
	struct Opcode_info opcode_info_obj;
	void kernelinfo_function(int op_code, Opcode_info *arg_opcode);

	void kernelinfo(int opcode, Opcode_info *arg_opcode);

	map<int, kernelfunction>   kernelinfo_func_keys;
	map<int, int>   opcode_idx_pair;

	kernelInfo();
	~kernelInfo();

};*/

//void kernelinfo_generator(vector<kernelInfo> kernelinfo_obj);

void kernelinfo_generator(vector<kernelInfo> kernelinfo_out, map<int, kernelInfo> kernelinfo_map);
#endif ///__KERNELINFO_CLASS_H__
