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

#ifndef __HW_SETTINGS_H__
#define __HW_SETTINGS_H__

#include <string>
#include <vector>

#if 0//ndef __SDSOC
#define XI_DIET_CHAI_Z 		0
#define XI_DIET_CHAI_ZUPLUS	0
#define __POOL_ENABLE__		1
#define __DECONV_ENABLE__	0
#endif

//**** CONFIGURABLE MACROS
#if XI_DIET_CHAI_Z || XI_DIET_CHAI_ZUPLUS
#define XI_KER_PROC       	8
#define XI_PIX_PROC       	8
#define XI_ISTAGEBUFF_DEPTH 1024
#define XI_OSTAGEBUFF_DEPTH 1024
#define XI_WEIGHTBUFF_DEPTH 1024
#else
#define XI_KER_PROC       	16
#define XI_PIX_PROC       	32
#define XI_ISTAGEBUFF_DEPTH 8192
#define XI_OSTAGEBUFF_DEPTH 2048
#define XI_WEIGHTBUFF_DEPTH 2048
#endif

//**DON'T MODIFY THE BELOW MACROS**//
#if XI_DIET_CHAI_Z
#define XI_64BIT_PORT_EN    	1
#define XI_DISABLE_BN 			1
#define XI_OSTG_BUFFER_SET 		4
#define XI_SINGLE_IO_PORT_EN 	0
#elif XI_DIET_CHAI_ZUPLUS
#define XI_64BIT_PORT_EN    	0
#define XI_DISABLE_BN 			1
#define XI_OSTG_BUFFER_SET 		4
#define XI_SINGLE_IO_PORT_EN 	1
#else
#define XI_64BIT_PORT_EN    	0
#define XI_SINGLE_IO_PORT_EN 	0
#define XI_DISABLE_BN 			0
#define XI_OSTG_BUFFER_SET 		8
#endif

#define DISABLE_BN          XI_DISABLE_BN
#define SINGLE_IO_PORT      XI_SINGLE_IO_PORT_EN
#define PORT_BITWIDTH_64BIT XI_64BIT_PORT_EN
#define KER_PROC            XI_KER_PROC

//# if 1 use standalone pool otherwise convolution
#if __POOL_ENABLE__
#define POOL_KERNEL_EXIST   1
#else
#define POOL_KERNEL_EXIST   0
#endif

#define FC_KERNEL_EXIST     0

//# Enable to dump layerwise output 
#define LAYERWISE_OUTPUT_WRITE	0

#define DEBUG_WEIGHT_EXTRACTION 0

//#define POOL_KERNEL_EXIST 0
//#define MERGED_FC_KERNEL_EXIST 0
//# Set to 0 for Caffe ref / Set to 1 for HLS ref comparison
#define PACKED_INOUT 0
#define FILE_WRITE 	 0

//# I/O data type - For 8bit specify 8 / For 16bit specify 16
#define IO_TYPE  8

#if IO_TYPE==16

#define XBATCH_SIZE		1
#define IO_DATA_TYPE	short
#define WEIGHT_PORTS	2
//#define KER_PROC        8  //Make sure for IO_TYPE 16, it is 8

#else

#define XBATCH_SIZE		2
#define IO_DATA_TYPE	signed char

//# Weight ports
#if KER_PROC==16
#define WEIGHT_PORTS	4
#else
#if PORT_BITWIDTH_64BIT
#define WEIGHT_PORTS	4
#else
#define WEIGHT_PORTS	2
#endif  //#if PORT_BITWIDTH_64BIT
#endif  //#if KER_PROC==16

#define MAX_PORTS			32

//# Number of Ports
#if SINGLE_IO_PORT==0
#define INPUT_PORTS			5
#define OUTPUT_PORTS		4
#define HCONV_OUT_PORTS		2
#else
#define INPUT_PORTS			3
#define OUTPUT_PORTS		2
#define HCONV_OUT_PORTS 	1
#endif  //#if SINGLE_IO_PORT==0

#endif  //#if IO_TYPE==16

#define QUANT_PREC_SHIFT	22

//# Scalar parameter size
#define MAX_PARAM_SIZE	128

//# Number of images in the Queue
#define NUM_IMG			1

//# Batch size
#define BATCH_PACK_SIZE		16

//# Stack size for constant buffers (in bytes)
#define STACK_SIZE	(64*1024*1024)

#define IO_DATA_TYPE1 float

//# Convolution IO port width
#if PORT_BITWIDTH_64BIT
#define PORT_WIDTH 128//64
#else
#define PORT_WIDTH 128
#endif

//# Number of IO planes packed
#define PLANE_PACK ((PORT_WIDTH/IO_TYPE)/XBATCH_SIZE)

#define PACK_ELEMS PLANE_PACK

//# Number of IO planes packed
#define ALIGN_FACTOR(port_width, data_type) (port_width/data_type)

//#define WEIGHT_PACK_ELEMS 16
#define CONV3D_WEIGHT_PACK_ELEMS 8

#define HPARAM_TYPE 	int

#define BUF_ADDR_TYPE 	char

//# Configurable params depend on quant mode
//#ifdef QUANT_MODE
#define HCONV_BIAS_TYPE			short
#define HCONV_BIAS_DATA_WIDTH 	16
#define HFC_BIAS_TYPE 			short
#define HFC_BIAS_DATA_WIDTH 	16
/*
#else
#define HCONV_BIAS_TYPE			IO_DATA_TYPE
#define HCONV_BIAS_DATA_WIDTH 	8
#define HFC_BIAS_TYPE 			IO_DATA_TYPE
#define HFC_BIAS_DATA_WIDTH 	16
#endif
*/

/************ Convolution ********************/
//# Convolution data types
#define HCONV_WGT_TYPE	IO_DATA_TYPE
#define HCONV_IN_TYPE2	IO_DATA_TYPE
#define HCONV_OUT_TYPE	IO_DATA_TYPE
#define HCONV_IN_TYPE1	IO_DATA_TYPE
#define HCONV_BIAS_PORT_WIDTH	64

/************ FC Layer ********************/
#define HFC_WGT_TYPE 		short
#define HFC_WGT_DATA_WIDTH 	16
#define HFC_WGT_PORT_WIDTH	128

#define HFC_BIAS_PORT_WIDTH	64

#define HFC_OUT_PORT_WIDTH	128
#define HFC_IN_PORT_WIDTH	128
#define HFC_IN_DATA_WIDTH	8

//# FC I/O data types
#define HFC_OUT_TYPE	IO_DATA_TYPE
#define HFC_IN_TYPE 	IO_DATA_TYPE

//# Softmax Data types
#define HSOFTMAX_IN_TYPE    float
#define HSOFTMAX_OUT_TYPE   int

//# Deconvolution data types
#define HDECONV_WGT_TYPE 	short
#define HDECONV_BIAS_TYPE 	short
#define HDECONV_IN_TYPE     short
#define HDECONV_OUT_TYPE	int

//# Normalization data type
//#if QUANT_MODE
#define HNORM_GAMMA_FLOAT_TYPE		float
#define HNORM_SUM_FLOAT_TYPE		float
//#else
#define HNORM_GAMMA_TYPE			short
#define HNORM_SUM_TYPE				int
//#endif

//# Permute data types
#define HPERMUTE_IN_TYPE			IO_DATA_TYPE
#define HPERMUTE_OUT_TYPE			IO_DATA_TYPE

//# Batch norm mean data type
#define HMEAN_TYPE	short

//# Software softmax data types
#define HSWSX_OUT_TYPE				float
#define HSWSX_IN_TYPE				IO_DATA_TYPE

//# PriorBox data types
#define HNMS_PBOX_TYPE				float
#define HNMS_VAR_TYPE				float
#define HNMS_OUT_TYPE				int

#define SW_FC_DATA_TYPE float

//# Type of layer (HW/SW)
enum _layer_type{
	EXE_ON_HW,
	EXE_ON_SW
};
typedef _layer_type layer_type_e;

#endif // __HW_SETTINGS_H__
