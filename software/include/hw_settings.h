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

#ifndef _HW_SETTINGS_H_
#define _HW_SETTINGS_H_

#include <string>
#include <vector>

#define GENERATE_CONSTDATA 1
#define DATA_IN_BINARY 1

#define PACKED_INOUT 0
#define FILE_WRITE 	 0

//# Number of Ports
#define INPUT_PORTS		5
#define OUTPUT_PORTS	2
#define WEIGHT_PORTS	2

//# Scalar parameter size
#define MAX_PARAM_SIZE	48

//# Number of images in the Queue
#define NUM_IMG			1

//# Stack size for constant buffers (in bytes)
#define STACK_SIZE	(64*1024*1024)

//# Convolution kernel parameters
#define KER_PROC 8

#define HPARAM_TYPE 	int

#define BUF_ADDR_TYPE 	char

//# Convolution data types
#define HCONV_WGT_TYPE	char
#define HCONV_BIAS_TYPE	short
#define HCONV_OUT_TYPE	short
#define HCONV_IN_TYPE1	short
#define HCONV_IN_TYPE2	char

//# FC data types
#define HFC_WGT_TYPE 	char
#define HFC_BIAS_TYPE 	short
#define HFC_OUT_TYPE	short
#define HFC_IN_TYPE 	short

//# Softmax Data types
#define HSOFTMAX_IN_TYPE    float
#define HSOFTMAX_OUT_TYPE   int

//# Deconvolution data types
#define HDECONV_WGT_TYPE 	short
#define HDECONV_BIAS_TYPE 	short
#define HDECONV_IN_TYPE     short
#define HDECONV_OUT_TYPE	int

//# Normalization data type
#define HNORM_GAMMA_TYPE			short
#define HNORM_SUM_TYPE				int

//# Permute data types
#define HPERMUTE_OUT_TYPE			float
#define HPERMUTE_IN_TYPE			short

//# Software softmax data types
#define HSWSX_OUT_TYPE				float
#define HSWSX_IN_TYPE				float

//# PriorBox data types
#define HNMS_PBOX_TYPE				float
#define HNMS_VAR_TYPE				float
#define HNMS_OUT_TYPE				int


//# Type of layer (HW/SW)
enum _layer_type{
	EXE_ON_HW,
	EXE_ON_SW
};
typedef _layer_type layer_type_e;


struct xtract_layer_type
{
	std::string layer_name;
	layer_type_e layer_type;
};

enum _opcode_num{

	OPCODE_CONV=0,
	OPCODE_ReLU=0,
	OPCODE_Concat=0,
	OPCODE_TMP_BN = 0,
	OPCODE_TMP_POWER = 0,
	OPCODE_LRN_INTER_SOS=1,
	OPCODE_LRN_INTER_PNS=2,
	OPCODE_POOL=0,
	OPCODE_TMP_LRN=0,
	OPCODE_DECONV=0,
	OPCODE_SOFTMAX=0,
	OPCODE_ARGMAX=0,
	OPCODE_FC=0,
	OPCODE_PERM=0,
	OPCODE_CROP=0,
	OPCODE_NMS=0,
	OPCODE_L2NORM=0,
	OPCODE_BN = 10,
	OPCODE_ELTWISE = 11,
	OPCODE_FUSE_BN_CONV = 12,
	OPCODE_CRELU = 18
};

typedef _opcode_num opcode_num_e;

struct xtract_opcode_num
{
	std::string layer_name;
	opcode_num_e opcode;
};

//void get_hw_and_sw_layer(std::vector <struct xtract_layer_type> &xtract_layer);
//void get_opcode_xlayer(std::vector < struct xtract_opcode_num> &xlayers_opcode);

#endif // _HW_SETTINGS_H_
