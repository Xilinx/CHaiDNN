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

#include "kernelinfo_class.h"
#include <stdio.h>
#include <string.h>


/*int opcode_arr[NUM_OPCODES] = {
		HW_CONV,
		SW_SOFTMAX
};*/

/*struct packinfo
{
	enum dimesion_type dim;
	enum separation sep;
	int mod_div_factor;
	bool mem_contiguous;
};*/

_quant_scheme_e g_quant_value;

void update_conv_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){

	Module_info module_default;
	module_default.module_target_info = FPGA;
	module_default.module_type = HW_CONV;
	module_default.module_name = "Convolution"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** conv mega module ports & these port order should be same as deploy file*********/

	Port_info port_defualt;
	port_defualt.port_width=XI_128BIT_WIDTH;
	port_defualt.data_type = XI_INT8;
	port_defualt.size_datatype = XI_INT8_S;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __SDS_NON_CACHEABLE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = g_quant_value;

	packinfo input_pack;

	input_pack.dim = XI_CHANNEL;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_pack.sep=XI_DIVISION;

	port_defualt.pack_info_vec.push_back(input_pack);



	input_pack.dim = XI_HEIGHT;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_WIDTH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_INTERLEAVED;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_pack.sep=XI_MODULO;
	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_BATCH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	obj_module_opcode.port_vec.push_back(port_defualt);

	/************ other input needed for eltwise ***************/
	Port_info input_port;
	input_port.port_width=XI_128BIT_WIDTH;
	input_port.data_type = XI_INT8;
	input_port.size_datatype = XI_INT8_S;
	input_port.memory_align_required=32; // TODO need to check with buffer management & init module
	input_port.memory_map_info = __SDS_NON_CACHEABLE;
	input_port.port_enum=XI_INPUT;
	input_port.qant_scheme_type = g_quant_value;
	packinfo input_pack0;


	input_pack0.dim = XI_CHANNEL;
	input_pack0.mem_contiguous = true;
	input_pack0.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_pack0.sep=XI_DIVISION;

	input_port.pack_info_vec.push_back(input_pack0);

	input_pack0.dim = XI_HEIGHT;
	input_pack0.mem_contiguous = true;
	input_pack0.mod_div_factor = 0;
	input_pack0.sep=XI_NONE;

	input_port.pack_info_vec.push_back(input_pack0);

	input_pack0.dim = XI_WIDTH;
	input_pack0.mem_contiguous = true;
	input_pack0.mod_div_factor = 0;
	input_pack0.sep=XI_NONE;

	input_port.pack_info_vec.push_back(input_pack0);

	input_pack0.dim = XI_INTERLEAVED;
	input_pack0.mem_contiguous = true;
	input_pack0.mod_div_factor = (input_port.port_width/input_port.size_datatype)/batch_size;
	input_pack0.sep=XI_MODULO;

	input_port.pack_info_vec.push_back(input_pack0);

	input_pack0.dim = XI_BATCH;
	input_pack0.mem_contiguous = true;
	input_pack0.mod_div_factor = 0;
	input_pack0.sep=XI_NONE;

	input_port.pack_info_vec.push_back(input_pack0);

	obj_module_opcode.port_vec.push_back(input_port);


	Port_info port_defualt1;

	port_defualt1.data_type = XI_INT8;
	port_defualt1.size_datatype = XI_INT8_S;
	port_defualt1.port_width=XI_128BIT_WIDTH;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __SDS_NON_CACHEABLE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;
	packinfo output_pack0;

	output_pack0.dim = XI_CHANNEL;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = (port_defualt1.port_width/port_defualt1.size_datatype)/batch_size;
	output_pack0.sep=XI_DIVISION;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_HEIGHT;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 0;
	output_pack0.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_WIDTH;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 0;
	output_pack0.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_INTERLEAVED;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = (port_defualt1.port_width/port_defualt1.size_datatype)/batch_size;
	output_pack0.sep=XI_MODULO;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_BATCH;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 0;
	output_pack0.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	obj_module_opcode.port_vec.push_back(port_defualt1);

	/****** OPCODE_CONV  *******/
	obj_module_opcode.op_code = OPCODE_CONV;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
	/****** OPCODE_CRELU ***********/
	obj_module_opcode.op_code = OPCODE_CRELU;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
	/****** OPCODE_BN + SCALE***********/
	obj_module_opcode.op_code = OPCODE_BN;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
	/****** OPCODE_ELTWISE ***********/
#if XI_DIET_CHAI_Z==0
	obj_module_opcode.op_code = OPCODE_ELTWISE;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
#endif
	/****** OPCODE_FUSE_BN_CONV ***********/
	obj_module_opcode.op_code = OPCODE_FUSE_BN_CONV;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
	/****** OPCODE_3D_CONV ***********/
	obj_module_opcode.op_code = OPCODE_3D_CONV;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
	/****** OPCODE_SEPARABLE_CONV ***********/
	obj_module_opcode.op_code = OPCODE_SEPARABLE_CONV;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
	/****** OPCODE_PARALLEL_CONV_FUSE ***********/
	obj_module_opcode.op_code = OPCODE_PARALLEL_CONV_FUSE;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;

	/****** OPCODE_AVRPOOL2CONV ***********/
	obj_module_opcode.op_code = OPCODE_AVRPOOL2CONV;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;

	/****** OPCODE_POOL_CONV_FUSE ***********/
	obj_module_opcode.op_code = OPCODE_POOL_CONV2CONV;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;

#if POOL_KERNEL_EXIST==0
	/****** OPCODE_MAXPOOL2CONV ***********/
	obj_module_opcode.op_code = OPCODE_POOL2CONV;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
#endif

}
void update_fc_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){

	Module_info module_default;
#if FC_KERNEL_EXIST
	module_default.module_target_info = FPGA;
	module_default.module_type = HW_FC;
#else
	module_default.module_target_info = CPU;
	module_default.module_type = SW_FC;
#endif

	module_default.module_name = "InnerProduct"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** fc mega module ports  & these port order should be same as deploy file*********/

	Port_info port_defualt;

#if FC_KERNEL_EXIST
	port_defualt.data_type = XI_INT8;
	port_defualt.size_datatype = XI_INT8_S;
	port_defualt.port_width=XI_128BIT_WIDTH;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __SDS_NON_CACHEABLE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = QUNT_FIXED;
#else
	port_defualt.data_type = XI_FLOAT;
	port_defualt.size_datatype = XI_FLOAT_S;
	port_defualt.port_width=XI_32BIT_WIDTH;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __MALLOC_TYPE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = g_quant_value;
#endif

	packinfo input_pack;

	input_pack.dim = XI_CHANNEL;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_HEIGHT;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;
	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_WIDTH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_INTERLEAVED;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_BATCH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	obj_module_opcode.port_vec.push_back(port_defualt);

	Port_info port_defualt1;
#if FC_KERNEL_EXIST
	port_defualt1.data_type = XI_INT8;
	port_defualt1.size_datatype = XI_INT8_S;
	port_defualt1.port_width=XI_128BIT_WIDTH;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __SDS_NON_CACHEABLE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = QUNT_FIXED;
#else
	port_defualt1.data_type = XI_FLOAT;
	port_defualt1.size_datatype = XI_FLOAT_S;
	port_defualt1.port_width=XI_32BIT_WIDTH;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __MALLOC_TYPE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;
#endif
	packinfo  output_pack;
	output_pack.dim = XI_CHANNEL;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = 0;
	output_pack.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack);

	output_pack.dim = XI_HEIGHT;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = 0;
	output_pack.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack);

	output_pack.dim = XI_WIDTH;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = 0;
	output_pack.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack);

	output_pack.dim = XI_INTERLEAVED;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = 0;
	output_pack.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack);

	//packinfo output_pack;
	output_pack.dim = XI_BATCH;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = 0;
	output_pack.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack);

	obj_module_opcode.port_vec.push_back(port_defualt1);
	/****** OPCODE_FC  *******/
	obj_module_opcode.op_code = OPCODE_FC;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;

}
void update_deconv_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){

	Module_info module_default;
	module_default.module_target_info = FPGA;
	module_default.module_type = HW_DECONV;
	module_default.module_name = "Deconvolution"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** deconv mega module ports  & these port order should be same as deploy file *********/

	Port_info port_defualt;
	port_defualt.port_width=XI_16BIT_WIDTH;
	port_defualt.data_type = XI_INT16;
	port_defualt.size_datatype = XI_INT16_S;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __SDS_NON_CACHEABLE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = g_quant_value;
	packinfo input_pack;

	input_pack.dim = XI_CHANNEL;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);


	input_pack.dim = XI_HEIGHT;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;
	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_WIDTH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_INTERLEAVED;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_BATCH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	obj_module_opcode.port_vec.push_back(port_defualt);

	Port_info port_defualt1;
	port_defualt1.data_type = XI_INT32;
	port_defualt1.size_datatype = XI_INT32_S;
	port_defualt1.port_width=XI_64BIT_WIDTH;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __SDS_NON_CACHEABLE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;
	packinfo output_pack;

	output_pack.dim = XI_CHANNEL;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = 0;
	output_pack.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack);

	output_pack.dim = XI_HEIGHT;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = 0;
	output_pack.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack);

	output_pack.dim = XI_WIDTH;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = 0;
	output_pack.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack);

	output_pack.dim = XI_INTERLEAVED;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = 0;
	output_pack.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack);

	output_pack.dim = XI_BATCH;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = 0;
	output_pack.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack);

	obj_module_opcode.port_vec.push_back(port_defualt1);
	/****** OPCODE_FC  *******/
	obj_module_opcode.op_code = OPCODE_DECONV;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
}
void update_pooling_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){

	Module_info module_default;
	module_default.module_target_info = FPGA;
	module_default.module_type = HW_POOL;
	module_default.module_name = "Pooling"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** pooling mega module ports  & these port order should be same as deploy file *********/

	Port_info port_defualt;
	port_defualt.data_type = XI_INT8;
	port_defualt.size_datatype = XI_INT8_S;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __SDS_NON_CACHEABLE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.port_width=XI_128BIT_WIDTH;
	port_defualt.qant_scheme_type = g_quant_value;
	packinfo input_pack;

	input_pack.dim = XI_CHANNEL;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_pack.sep=XI_DIVISION;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_HEIGHT;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_WIDTH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);
	input_pack.dim = XI_INTERLEAVED;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_pack.sep=XI_MODULO;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_BATCH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	obj_module_opcode.port_vec.push_back(port_defualt);

	Port_info port_defualt1;
	port_defualt1.data_type = XI_INT8;
	port_defualt1.size_datatype = XI_INT8_S;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __SDS_NON_CACHEABLE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.port_width=XI_128BIT_WIDTH;
	port_defualt1.qant_scheme_type = g_quant_value;
	packinfo output_pack;

	output_pack.dim = XI_CHANNEL;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = (port_defualt1.port_width/port_defualt1.size_datatype)/batch_size;
	output_pack.sep=XI_DIVISION;

	port_defualt1.pack_info_vec.push_back(output_pack);

	output_pack.dim = XI_HEIGHT;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = 0;
	output_pack.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack);

	output_pack.dim = XI_WIDTH;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = 0;
	output_pack.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack);

	output_pack.dim = XI_INTERLEAVED;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = (port_defualt1.port_width/port_defualt1.size_datatype)/batch_size;
	output_pack.sep=XI_MODULO;

	port_defualt1.pack_info_vec.push_back(output_pack);

	output_pack.dim =  XI_BATCH;
	output_pack.mem_contiguous = true;
	output_pack.mod_div_factor = 0;
	output_pack.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack);

	obj_module_opcode.port_vec.push_back(port_defualt1);

	/****** OPCODE_POOL  *******/
	obj_module_opcode.op_code = OPCODE_POOL;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
}

void update_softmax_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){

	Module_info module_default;
	module_default.module_target_info = CPU;
	module_default.module_type = SW_SOFTMAX;
	module_default.module_name = "Softmax"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** softmax mega module ports  & these port order should be same as deploy file*********/

	Port_info port_defualt;
	port_defualt.data_type = XI_FLOAT;
	port_defualt.size_datatype = XI_FLOAT_S;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __MALLOC_TYPE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = g_quant_value;
	obj_module_opcode.port_vec.push_back(port_defualt);

	packinfo input_p_info;

	input_p_info.dim = XI_CHANNEL;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_p_info.sep=XI_DIVISION;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_HEIGHT;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_WIDTH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_INTERLEAVED;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_p_info.sep=XI_MODULO;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim =  XI_BATCH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	Port_info port_defualt1;
	port_defualt1.data_type = XI_FLOAT;
	port_defualt1.size_datatype = XI_FLOAT_S;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __MALLOC_TYPE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;

	packinfo output_p_info;

	output_p_info.dim = XI_CHANNEL;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_HEIGHT;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_WIDTH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_INTERLEAVED;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim =  XI_BATCH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt1);
	/****** OPCODE_SOFTMAX  *******/
	obj_module_opcode.op_code = OPCODE_SOFTMAX;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;

}
void update_crop_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){

	Module_info module_default;
	module_default.module_target_info = CPU;
	module_default.module_type = SW_CROP;
	module_default.module_name = "Crop"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** crop mega module ports  & these port order should be same as deploy file*********/

	Port_info port_defualt;
	port_defualt.data_type = XI_INT32;
	port_defualt.size_datatype = XI_INT32_S;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __SDS_CACHEABLE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = g_quant_value;

	packinfo input_p_info;

	input_p_info.dim = XI_CHANNEL;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_HEIGHT;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_WIDTH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_INTERLEAVED;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim =  XI_BATCH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt);

	Port_info port_defualt1;
	port_defualt1.data_type = XI_INT32;
	port_defualt1.size_datatype = XI_INT32_S;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __MALLOC_TYPE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;

	packinfo output_p_info;

	output_p_info.dim = XI_CHANNEL;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_HEIGHT;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_WIDTH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_INTERLEAVED;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim =  XI_BATCH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt1);
	/****** OPCODE_CROP  *******/
	obj_module_opcode.op_code = OPCODE_CROP;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
}
void update_nms_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){


	Module_info module_default;
	module_default.module_target_info = CPU;
	module_default.module_type = SW_NMS;
	module_default.module_name = "NMS"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** nms mega module ports  & these port order should be same as deploy file *********/

	Port_info in_port0;
	in_port0.data_type = XI_INT8;
	in_port0.size_datatype = XI_INT8_S;
	in_port0.memory_align_required=32; // TODO need to check with buffer management & init module
	in_port0.memory_map_info = __SDS_CACHEABLE;
	in_port0.port_enum=XI_INPUT;
	in_port0.qant_scheme_type = g_quant_value;

	packinfo input_p_info;

	input_p_info.dim = XI_CHANNEL;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 1; // need to discuss with team
	input_p_info.sep=XI_DIVISION;

	in_port0.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_HEIGHT;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	in_port0.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_WIDTH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	in_port0.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_INTERLEAVED;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 1; // need to discuss with team
	input_p_info.sep=XI_MODULO;

	in_port0.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_BATCH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	in_port0.pack_info_vec.push_back(input_p_info);

	obj_module_opcode.port_vec.push_back(in_port0);
	Port_info in_port1;

	in_port1.data_type = XI_FLOAT;
	in_port1.size_datatype = XI_FLOAT_S;
	in_port1.memory_align_required=32; // TODO need to check with buffer management & init module
	in_port1.memory_map_info = __SDS_CACHEABLE;
	in_port1.port_enum=XI_INPUT;
	in_port1.qant_scheme_type = g_quant_value;


	input_p_info.dim = XI_CHANNEL;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	in_port1.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_HEIGHT;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	in_port1.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_WIDTH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	in_port1.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_INTERLEAVED;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	in_port1.pack_info_vec.push_back(input_p_info);

	input_p_info.dim =  XI_BATCH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	in_port1.pack_info_vec.push_back(input_p_info);
	obj_module_opcode.port_vec.push_back(in_port1);

	Port_info port_defualt1;
	port_defualt1.data_type = XI_FLOAT;
	port_defualt1.size_datatype = XI_FLOAT_S;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __MALLOC_TYPE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;

	packinfo output_p_info;

	output_p_info.dim = XI_CHANNEL;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_HEIGHT;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_WIDTH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_INTERLEAVED;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim =  XI_BATCH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt1);
	/****** OPCODE_NMS  *******/
	obj_module_opcode.op_code = OPCODE_NMS;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
}
void update_xcustom_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){


	Module_info module_default;
	module_default.module_target_info = CPU;
	module_default.module_type = SW_XCUSTOM;
	module_default.module_name = "XCustom"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** xcustom mega module ports  & these port order should be same as deploy file*********/

	Port_info port_defualt;
	port_defualt.data_type = XI_FLOAT;
	port_defualt.size_datatype = XI_FLOAT_S;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __MALLOC_TYPE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = g_quant_value;

	packinfo input_p_info;
	input_p_info.dim = XI_CHANNEL;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_HEIGHT;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_WIDTH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_INTERLEAVED;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim =  XI_BATCH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt);

	Port_info port_defualt1;
	port_defualt1.data_type = XI_FLOAT;
	port_defualt1.size_datatype = XI_FLOAT_S;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __MALLOC_TYPE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;

	packinfo output_p_info;

	output_p_info.dim = XI_CHANNEL;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_HEIGHT;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_WIDTH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_INTERLEAVED;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim =  XI_BATCH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt1);
	/****** OPCODE_XCUSTOM  *******/
	obj_module_opcode.op_code = OPCODE_XCUSTOM;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
}
void update_conv_xpack_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){

	Module_info module_default;
	module_default.module_target_info = CPU;
	module_default.module_type = SW_XPACK;
	module_default.module_name = "XPack"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** xpack mega module ports  & these port order should be same as deploy file*********/

	Port_info port_defualt;
	port_defualt.data_type = XI_FLOAT;
	port_defualt.size_datatype = XI_FLOAT_S;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __MALLOC_TYPE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = g_quant_value;

	packinfo input_p_info;

	input_p_info.dim = XI_CHANNEL;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_HEIGHT;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_WIDTH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_INTERLEAVED;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim =  XI_BATCH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt);

	Port_info port_defualt1;
	port_defualt1.data_type = XI_INT8;
	port_defualt1.port_width=XI_128BIT_WIDTH;
	port_defualt1.size_datatype = XI_INT8_S;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __SDS_NON_CACHEABLE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;

	packinfo output_p_info;

	output_p_info.dim = XI_CHANNEL;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = (port_defualt1.port_width/port_defualt1.size_datatype)/batch_size;
	output_p_info.sep=XI_DIVISION;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_HEIGHT;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_WIDTH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_INTERLEAVED;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = (port_defualt1.port_width/port_defualt1.size_datatype)/batch_size;
	output_p_info.sep=XI_MODULO;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_BATCH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt1);

	/****** OPCODE_XPACK  *******/
	obj_module_opcode.op_code = OPCODE_XPACK;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;

}
void update_permute_xunpack_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){

	Module_info module_default;
	module_default.module_target_info = CPU;
	module_default.module_type = SW_XPACK;
	module_default.module_name = "XUnPack_permute"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** xpack mega module ports  & these port order should be same as deploy file*********/

	Port_info port_defualt;
	port_defualt.data_type = XI_INT8;
	port_defualt.size_datatype = XI_INT8_S;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __MALLOC_TYPE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = g_quant_value;

	packinfo input_p_info;

	input_p_info.dim = XI_CHANNEL;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 1;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_HEIGHT;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_WIDTH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_INTERLEAVED;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 1;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim =  XI_BATCH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt);

	Port_info port_defualt1;
	port_defualt1.data_type = XI_FLOAT;
	port_defualt1.size_datatype =XI_FLOAT_S;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __MALLOC_TYPE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;

	packinfo output_p_info;

	output_p_info.dim = XI_CHANNEL;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_HEIGHT;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_WIDTH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_INTERLEAVED;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_BATCH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt1);
	/****** OPCODE_XPACK  *******/
	obj_module_opcode.op_code = OPCODE_PERMUTE_XUNPACK;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;

}
void update_opcde_sw_eltwise_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){

	Module_info module_default;
	module_default.module_target_info = CPU;
	module_default.module_type = SW_ELTWISE_ADD;
	module_default.module_name = "Eltwise"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** xcustom mega module ports  & these port order should be same as deploy file*********/

	Port_info port_defualt;
	port_defualt.port_width=XI_128BIT_WIDTH;
	port_defualt.data_type = XI_INT8;
	port_defualt.size_datatype = XI_INT8_S;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __MALLOC_TYPE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = g_quant_value;


	packinfo input_pack;

	input_pack.dim = XI_CHANNEL;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_pack.sep=XI_DIVISION;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_HEIGHT;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_WIDTH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_INTERLEAVED;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_pack.sep=XI_MODULO;
	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_BATCH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	obj_module_opcode.port_vec.push_back(port_defualt);


	packinfo input_pack1;
	Port_info port_defualt2;
	port_defualt2.port_width=XI_128BIT_WIDTH;
	port_defualt2.data_type = XI_INT8;
	port_defualt2.size_datatype = XI_INT8_S;
	port_defualt2.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt2.memory_map_info = __MALLOC_TYPE;
	port_defualt2.port_enum=XI_INPUT;
	port_defualt2.qant_scheme_type = g_quant_value;

	input_pack1.dim = XI_CHANNEL;
	input_pack1.mem_contiguous = true;
	input_pack1.mod_div_factor = (port_defualt2.port_width/port_defualt2.size_datatype)/batch_size;
	input_pack1.sep=XI_DIVISION;

	port_defualt2.pack_info_vec.push_back(input_pack1);

	input_pack1.dim = XI_HEIGHT;
	input_pack1.mem_contiguous = true;
	input_pack1.mod_div_factor = 0;
	input_pack1.sep=XI_NONE;

	port_defualt2.pack_info_vec.push_back(input_pack1);

	input_pack1.dim = XI_WIDTH;
	input_pack1.mem_contiguous = true;
	input_pack1.mod_div_factor = 0;
	input_pack1.sep=XI_NONE;

	port_defualt2.pack_info_vec.push_back(input_pack1);

	input_pack1.dim = XI_INTERLEAVED;
	input_pack1.mem_contiguous = true;
	input_pack1.mod_div_factor = (port_defualt2.port_width/port_defualt2.size_datatype)/batch_size;
	input_pack1.sep=XI_MODULO;
	port_defualt2.pack_info_vec.push_back(input_pack1);

	input_pack1.dim = XI_BATCH;
	input_pack1.mem_contiguous = true;
	input_pack1.mod_div_factor = 0;
	input_pack1.sep=XI_NONE;

	port_defualt2.pack_info_vec.push_back(input_pack1);
	obj_module_opcode.port_vec.push_back(port_defualt2);

	Port_info port_defualt1;
	port_defualt1.port_width=XI_128BIT_WIDTH;
	port_defualt1.data_type = XI_INT8;
	port_defualt1.size_datatype = XI_INT8_S;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __MALLOC_TYPE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;

	packinfo output_pack0;

	output_pack0.dim = XI_CHANNEL;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = (port_defualt1.port_width/port_defualt1.size_datatype)/batch_size;
	output_pack0.sep=XI_DIVISION;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_HEIGHT;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 0;
	output_pack0.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_WIDTH;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 0;
	output_pack0.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_INTERLEAVED;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = (port_defualt1.port_width/port_defualt1.size_datatype)/batch_size;
	output_pack0.sep=XI_MODULO;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_BATCH;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 0;
	output_pack0.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack0);
	obj_module_opcode.port_vec.push_back(port_defualt1);


	/****** OPCODE_SW_ELTWISE_ADD  *******/
	obj_module_opcode.op_code = OPCODE_SW_ELTWISE_ADD;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;
}
void update_sf_xpack_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){

	Module_info module_default;
	module_default.module_target_info = CPU;
	module_default.module_type = SW_XPACK;
	module_default.module_name = "XPack_sf"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** xpack mega module ports  & these port order should be same as deploy file*********/

	Port_info port_defualt;
	port_defualt.data_type = XI_FLOAT;
	port_defualt.size_datatype = XI_FLOAT_S;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __MALLOC_TYPE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = g_quant_value;

	packinfo input_p_info;

	input_p_info.dim = XI_CHANNEL;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_HEIGHT;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_WIDTH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_INTERLEAVED;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim =  XI_BATCH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt);

	Port_info port_defualt1;
	port_defualt1.data_type = XI_INT8;
	port_defualt1.size_datatype = XI_INT8_S;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __MALLOC_TYPE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;

	packinfo output_p_info;

	output_p_info.dim = XI_CHANNEL;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 1;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_HEIGHT;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_WIDTH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_INTERLEAVED;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 1;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_BATCH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt1);
	/****** OPCODE_XPACK  *******/
	obj_module_opcode.op_code = OPCODE_SOFTMAX_XPACK;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;

}

void update_conv_xunpack_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){

	Module_info module_default;
	module_default.module_target_info = CPU;
	module_default.module_type = SW_XUNPACK;
	module_default.module_name = "XUnPack_conv"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** xunpack mega module ports  & these port order should be same as deploy file*********/

	Port_info port_defualt;
	port_defualt.data_type = XI_INT8;
	port_defualt.size_datatype = XI_INT8_S;
	port_defualt.port_width=XI_128BIT_WIDTH;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __SDS_NON_CACHEABLE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = g_quant_value;

	packinfo input_p_info;

	input_p_info.dim = XI_CHANNEL;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_p_info.sep=XI_DIVISION;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_HEIGHT;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_WIDTH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_INTERLEAVED;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_p_info.sep=XI_MODULO;

	port_defualt.pack_info_vec.push_back(input_p_info);

	input_p_info.dim = XI_BATCH;
	input_p_info.mem_contiguous = true;
	input_p_info.mod_div_factor = 0;
	input_p_info.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt);

	Port_info port_defualt1;
	port_defualt1.data_type = XI_FLOAT;
	port_defualt1.size_datatype = XI_FLOAT_S;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __MALLOC_TYPE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;

	packinfo output_p_info;

	output_p_info.dim = XI_CHANNEL;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_HEIGHT;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_WIDTH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim = XI_INTERLEAVED;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	output_p_info.dim =  XI_BATCH;
	output_p_info.mem_contiguous = true;
	output_p_info.mod_div_factor = 0;
	output_p_info.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_p_info);

	obj_module_opcode.port_vec.push_back(port_defualt1);
	/****** OPCODE_XUNPACK  *******/
	obj_module_opcode.op_code = OPCODE_XUNPACK;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;

}
void update_permute_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){

	Module_info module_default;
	module_default.module_target_info = CPU;
	module_default.module_type = SW_PERMUTE;
	module_default.module_name = "Permute"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** xcustom mega module ports  & these port order should be same as deploy file******/

	Port_info port_defualt;
	port_defualt.data_type = XI_INT8;
	port_defualt.port_width=XI_128BIT_WIDTH;
	port_defualt.size_datatype = XI_INT8_S;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __SDS_NON_CACHEABLE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = g_quant_value;

	packinfo input_pack;

	input_pack.dim = XI_CHANNEL;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_pack.sep=XI_DIVISION;

	port_defualt.pack_info_vec.push_back(input_pack);



	input_pack.dim = XI_HEIGHT;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_WIDTH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_INTERLEAVED;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_pack.sep=XI_MODULO;
	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_BATCH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	obj_module_opcode.port_vec.push_back(port_defualt);

	Port_info port_defualt1;
	port_defualt1.data_type = XI_INT8;
	port_defualt1.size_datatype = XI_INT8_S;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __MALLOC_TYPE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;

	packinfo output_pack0;

	output_pack0.dim = XI_CHANNEL;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 1;
	output_pack0.sep=XI_DIVISION;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_HEIGHT;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 0;
	output_pack0.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_WIDTH;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 0;
	output_pack0.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_INTERLEAVED;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 1;
	output_pack0.sep=XI_MODULO;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_BATCH;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 0;
	output_pack0.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	obj_module_opcode.port_vec.push_back(port_defualt1);
	/****** OPCODE_SW_PERMUTE  *******/
	obj_module_opcode.op_code = OPCODE_SW_PERMUTE;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;

}


void update_l2normalize_module_opcode_table(Opcode_info & obj_module_opcode,map<opcode_num_e, Opcode_info>  & opcode_info_map,int batch_size){

	Module_info module_default;
	module_default.module_target_info = CPU;
	module_default.module_type = SW_L2NORMALIZE;
	module_default.module_name = "L2Normalize"; /// TODO abid needs to update the layer information based on the enum instead of layer name
	module_default.num_total_ports=11; // TODO need to check with buffer management & init module
	obj_module_opcode.mega_module = module_default;

	/*** xcustom mega module ports  & these port order should be same as deploy file*********/

	Port_info port_defualt;
	port_defualt.port_width=XI_128BIT_WIDTH;
	port_defualt.data_type = XI_INT8;
	port_defualt.size_datatype = XI_INT8_S;
	port_defualt.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt.memory_map_info = __MALLOC_TYPE;
	port_defualt.port_enum=XI_INPUT;
	port_defualt.qant_scheme_type = g_quant_value;


	packinfo input_pack;

	input_pack.dim = XI_CHANNEL;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_pack.sep=XI_DIVISION;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_HEIGHT;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_WIDTH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_INTERLEAVED;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = (port_defualt.port_width/port_defualt.size_datatype)/batch_size;
	input_pack.sep=XI_MODULO;
	port_defualt.pack_info_vec.push_back(input_pack);

	input_pack.dim = XI_BATCH;
	input_pack.mem_contiguous = true;
	input_pack.mod_div_factor = 0;
	input_pack.sep=XI_NONE;

	port_defualt.pack_info_vec.push_back(input_pack);

	obj_module_opcode.port_vec.push_back(port_defualt);

	Port_info port_defualt1;
	port_defualt1.port_width=XI_128BIT_WIDTH;
	port_defualt1.data_type = XI_INT8;
	port_defualt1.size_datatype = XI_INT8_S;
	port_defualt1.memory_align_required=32; // TODO need to check with buffer management & init module
	port_defualt1.memory_map_info = __MALLOC_TYPE;
	port_defualt1.port_enum=XI_OUTPUT;
	port_defualt1.qant_scheme_type = g_quant_value;

	packinfo output_pack0;

	output_pack0.dim = XI_CHANNEL;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = (port_defualt1.port_width/port_defualt1.size_datatype)/batch_size;
	output_pack0.sep=XI_DIVISION;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_HEIGHT;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 0;
	output_pack0.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_WIDTH;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 0;
	output_pack0.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_INTERLEAVED;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = (port_defualt1.port_width/port_defualt1.size_datatype)/batch_size;
	output_pack0.sep=XI_MODULO;

	port_defualt1.pack_info_vec.push_back(output_pack0);

	output_pack0.dim = XI_BATCH;
	output_pack0.mem_contiguous = true;
	output_pack0.mod_div_factor = 0;
	output_pack0.sep=XI_NONE;

	port_defualt1.pack_info_vec.push_back(output_pack0);
	obj_module_opcode.port_vec.push_back(port_defualt1);


	/****** OPCODE_SW_L2NORM  *******/
	obj_module_opcode.op_code = OPCODE_SW_L2NORM;
	opcode_info_map[obj_module_opcode.op_code]=obj_module_opcode;

}
void update_datainfo(map<_datatype_size_e,_datainfo_s> &datainfo_map){

	_datainfo_s tmp;

	tmp.t_data=XI_UINT8;
	tmp.t_s_data=XI_INT8;

	datainfo_map[XI_INT8_S]=tmp;

	tmp.t_data=XI_UINT16;
	tmp.t_s_data=XI_INT16;

	datainfo_map[XI_INT16_S]=tmp;

	tmp.t_data=XI_UINT32;
	tmp.t_s_data=XI_INT32;

	datainfo_map[XI_INT32_S]=tmp;

	tmp.t_data = XI_FLOAT;
	tmp.t_s_data = XI_FLOAT;

	datainfo_map[XI_FLOAT_S]=tmp;
}

kernelInfo::kernelInfo()
{
	//cout<<"Mapping opcode and kernel function"<<endl;

	batch_size =2;

	/*** conv mega module*********/

	update_datainfo(datainfo_map);

	Opcode_info opcde_conv;

	update_conv_module_opcode_table(opcde_conv,opcode_info_map,batch_size);

	/*** fc mega module*********/

	Opcode_info opcde_fc;

	update_fc_module_opcode_table(opcde_fc,opcode_info_map,batch_size);

	/*** deconv mega module*********/

	Opcode_info opcde_deconv;
	update_deconv_module_opcode_table(opcde_deconv,opcode_info_map,batch_size);

	/*** pooling mega module*********/

	Opcode_info opcde_pooling;
	update_pooling_module_opcode_table(opcde_pooling,opcode_info_map,batch_size);

	/*** softmax mega module*********/

	Opcode_info opcde_softmax;
	update_softmax_module_opcode_table(opcde_softmax,opcode_info_map,batch_size);

	/*** crop mega module*********/

	Opcode_info opcde_crop;
	update_crop_module_opcode_table(opcde_crop,opcode_info_map,batch_size);

	/*** nms mega module*********/

	Opcode_info opcde_nms;
	update_nms_module_opcode_table(opcde_nms,opcode_info_map,batch_size);

	/*** xcustom mega module*********/

	Opcode_info opcde_xcustom;
	update_xcustom_module_opcode_table(opcde_xcustom,opcode_info_map,batch_size);

	/*** xpack mega module*********/

	Opcode_info opcde_xpack;
	update_conv_xpack_module_opcode_table(opcde_xpack,opcode_info_map,batch_size);

	/*** xunpack mega module*********/

	Opcode_info opcde_xunpack;
	update_conv_xunpack_module_opcode_table(opcde_xunpack,opcode_info_map,batch_size);

	/*** permute mega module*********/

	Opcode_info opcde_permute;
	update_permute_module_opcode_table(opcde_permute,opcode_info_map,batch_size);

	/*** l2normalize mega module*********/
	Opcode_info opcde_l2normalize;
	update_l2normalize_module_opcode_table(opcde_l2normalize,opcode_info_map,batch_size);



	Opcode_info opcde_sf_xpack;
	update_sf_xpack_module_opcode_table(opcde_sf_xpack,opcode_info_map,batch_size);

	Opcode_info permute_xunpack;
	update_permute_xunpack_module_opcode_table(permute_xunpack,opcode_info_map,batch_size);
#if (XI_DIET_CHAI_Z || XI_DIET_CHAI_ZUPLUS)
	Opcode_info opcde_sw_eltwise;
	update_opcde_sw_eltwise_module_opcode_table(opcde_sw_eltwise,opcode_info_map,batch_size);
#endif
	//	opcode_info_info.insert()
	//
	//	kernelinfo_func_keys.insert(pair<int, kernelfunction>(int(HW_CONV), &kernelInfo::kernelinfo_function));
	//	kernelinfo_func_keys.insert(pair<int, kernelfunction>(int(SW_SOFTMAX), &kernelInfo::kernelinfo_function));
}

kernelInfo::kernelInfo(string &quant_schem)
{
	//cout<<"Mapping opcode and kernel function"<<endl;

	batch_size =2;


	/*** conv mega module*********/
	if(quant_schem.compare("Xilinx")==0)
		g_quant_value =QUNT_INT;
	else
		g_quant_value=QUNT_FIXED;
	update_datainfo(datainfo_map);

	Opcode_info opcde_conv;

	update_conv_module_opcode_table(opcde_conv,opcode_info_map,batch_size);

	/*** fc mega module*********/

	Opcode_info opcde_fc;

	update_fc_module_opcode_table(opcde_fc,opcode_info_map,batch_size);

	/*** deconv mega module*********/

	Opcode_info opcde_deconv;
	update_deconv_module_opcode_table(opcde_deconv,opcode_info_map,batch_size);

	/*** pooling mega module*********/

	Opcode_info opcde_pooling;
	update_pooling_module_opcode_table(opcde_pooling,opcode_info_map,batch_size);

	/*** softmax mega module*********/

	Opcode_info opcde_softmax;
	update_softmax_module_opcode_table(opcde_softmax,opcode_info_map,batch_size);

	/*** crop mega module*********/

	Opcode_info opcde_crop;
	update_crop_module_opcode_table(opcde_crop,opcode_info_map,batch_size);

	/*** nms mega module*********/

	Opcode_info opcde_nms;
	update_nms_module_opcode_table(opcde_nms,opcode_info_map,batch_size);

	/*** xcustom mega module*********/

	Opcode_info opcde_xcustom;
	update_xcustom_module_opcode_table(opcde_xcustom,opcode_info_map,batch_size);

	/*** xpack mega module*********/

	Opcode_info opcde_xpack;
	update_conv_xpack_module_opcode_table(opcde_xpack,opcode_info_map,batch_size);

	/*** xunpack mega module*********/

	Opcode_info opcde_xunpack;
	update_conv_xunpack_module_opcode_table(opcde_xunpack,opcode_info_map,batch_size);

	/*** permute mega module*********/

	Opcode_info opcde_permute;
	update_permute_module_opcode_table(opcde_permute,opcode_info_map,batch_size);

	/*** l2normalize mega module*********/
	Opcode_info opcde_l2normalize;
	update_l2normalize_module_opcode_table(opcde_l2normalize,opcode_info_map,batch_size);
	//Opcode_info opcde_batch;
        Opcode_info opcde_sf_xpack;
	update_sf_xpack_module_opcode_table(opcde_sf_xpack,opcode_info_map,batch_size);

	Opcode_info permute_xunpack;
	update_permute_xunpack_module_opcode_table(permute_xunpack,opcode_info_map,batch_size);
#if (XI_DIET_CHAI_Z || XI_DIET_CHAI_ZUPLUS)
	Opcode_info opcde_sw_eltwise;
	update_opcde_sw_eltwise_module_opcode_table(opcde_sw_eltwise,opcode_info_map,batch_size);
#endif
	//	opcode_info_info.insert()
	//
	//	kernelinfo_func_keys.insert(pair<int, kernelfunction>(int(HW_CONV), &kernelInfo::kernelinfo_function));
	//	kernelinfo_func_keys.insert(pair<int, kernelfunction>(int(SW_SOFTMAX), &kernelInfo::kernelinfo_function));
}
kernelInfo::~kernelInfo()
{

}


//void kernelInfo::kernelinfo(int opcode, Opcode_info *arg_opcode)
//{
//
//
//	map<int, kernelfunction>::iterator result = kernelinfo_func_keys.find(opcode);
//	if(result != kernelinfo_func_keys.end()){
//
//		(this->*(result->second))(opcode, arg_opcode);
//	}
//
//}
#if 0
void display_port_info(Port_info *port_obj)
{

	fprintf(stderr, "\n *** port info start *** \n");

	if(port_obj->used == 0)
	{
		fprintf(stderr, "port used: No\n");
	}
	else
	{
		fprintf(stderr, "port used: Yes\n");


		fprintf(stderr, "in port port_idx: %d\n", port_obj->port_idx);

		switch(port_obj->data_type)
		{
		case XI_FLOAT:
			fprintf(stderr, "port data_type: float\n");
			break;

		case XI_INT8:
			fprintf(stderr, "port data_type: int8\n");
			break;

		case XI_UINT8:
			fprintf(stderr, "port data_type: uint8\n");
			break;
		}

		switch(port_obj->pack_info)
		{
		case XI_CHANNEL_INTERLEAVED_2:
			fprintf(stderr, "port pack_info: channel interleaved and split into two ports\n");
			break;

		}

		switch(port_obj->port_width)
		{
		case XI_128BIT_WIDTH:
			fprintf(stderr, "port pack_width: 128 bit port width\n");
			break;

		}
	}

	fprintf(stderr, "\n *** port info end *** \n");

}

void display_kernel_info(Opcode_info *arg_opcode)
{

	//printing kernel info
	fprintf(stderr, "\n ***** op_code info start ***** \n");
	fprintf(stderr, "op_code:%d\n", arg_opcode->op_code);

	fprintf(stderr, "\ninput_ports info \n");
	fprintf(stderr, "num_input_ports:%d\n", arg_opcode->mega_module.num_input_ports);

	for(int portid=0; portid<arg_opcode->mega_module.num_input_ports; portid++)
	{
		display_port_info(&arg_opcode->in_port_vec[portid]);
	}

	fprintf(stderr, "\noutput_ports info \n");
	fprintf(stderr, "num_output_ports:%d\n", arg_opcode->mega_module.num_output_ports);

	for(int portid=0; portid<arg_opcode->mega_module.num_output_ports; portid++)
	{
		display_port_info(&arg_opcode->out_port_vec[portid]);
	}

	fprintf(stderr, "\nweight_ports info \n");
	fprintf(stderr, "num_weight_ports:%d\n", arg_opcode->mega_module.num_weight_ports);

	for(int portid=0; portid<arg_opcode->mega_module.num_weight_ports; portid++)
	{
		display_port_info(&arg_opcode->wt_port_vec[portid]);
	}

	fprintf(stderr, "\nbias_ports info \n");
	fprintf(stderr, "num_bias_ports:%d\n", arg_opcode->mega_module.num_bias_ports);

	for(int portid=0; portid<arg_opcode->mega_module.num_bias_ports; portid++)
	{
		display_port_info(&arg_opcode->bias_port_vec[portid]);
	}

	fprintf(stderr, "\n ***** op_code info end ***** \n");

}

void kernelInfo::kernelinfo_function(int opcode, Opcode_info *arg_opcode)
{

	cout << "enters kernel info function"<<endl;
	Port_info port_obj;
	int num_ports;

	switch(opcode)
	{
	case HW_CONV:
		arg_opcode->op_code                      = opcode;

		arg_opcode->mega_module.module_type      = HW_CONV;

		/* input port info filling */
		num_ports = 2;
		arg_opcode->mega_module.num_input_ports  = num_ports;

		for(int portid=0; portid<num_ports; portid++)
		{
			port_obj.used = 1;
			port_obj.data_type = XI_INT8;
			port_obj.pack_info = XI_CHANNEL_INTERLEAVED_2;
			port_obj.port_width = XI_128BIT_WIDTH;
			port_obj.port_idx = portid;
			arg_opcode->in_port_vec.push_back(port_obj);
		}

		/* output port info filling */
		num_ports = 2;
		arg_opcode->mega_module.num_output_ports  = num_ports;

		for(int portid=0; portid<num_ports; portid++)
		{
			port_obj.used = 1;
			port_obj.data_type = XI_INT8;
			port_obj.pack_info = XI_CHANNEL_INTERLEAVED_2;
			port_obj.port_width = XI_128BIT_WIDTH;
			port_obj.port_idx = portid;
			arg_opcode->out_port_vec.push_back(port_obj);
		}


		/* weight port info filling */
		num_ports = 4;
		arg_opcode->mega_module.num_weight_ports  = num_ports;

		for(int portid=0; portid<num_ports; portid++)
		{
			port_obj.used = 1;
			port_obj.data_type = XI_INT8;
			port_obj.pack_info = XI_CHANNEL_INTERLEAVED_2;
			port_obj.port_width = XI_128BIT_WIDTH;
			port_obj.port_idx = portid;
			arg_opcode->wt_port_vec.push_back(port_obj);
		}

		arg_opcode->mega_module.num_bias_ports   = 1;

		/* bias port info filling */
		num_ports = 1;
		arg_opcode->mega_module.num_bias_ports  = num_ports;

		for(int portid=0; portid<num_ports; portid++)
		{
			port_obj.used = 1;
			port_obj.data_type = XI_INT8;
			port_obj.pack_info = XI_CHANNEL_INTERLEAVED_2;
			port_obj.port_width = XI_128BIT_WIDTH;
			port_obj.port_idx = portid;
			arg_opcode->bias_port_vec.push_back(port_obj);
		}

		display_kernel_info(arg_opcode);
		break;

	case SW_SOFTMAX:
		arg_opcode->op_code = opcode;
		fprintf(stderr, "opcode:%d\n", opcode);
		break;

	default:
		fprintf(stderr, "invalid opcode\n");
		break;
	}

	//printing kernel info
	//fprintf(stderr, "opcode:%d\n", opcode);
	//display_kernel_info((Opcode_info)arg_opcode->Opcode_info);


}



void kernelinfo_generator(vector<kernelInfo> kernelinfo_out, map<int, kernelInfo> kernelinfo_map)
{

	//std::vector<kernelInfo> kernelinfo_obj;
	for(int i=0;i<NUM_OPCODES;i++)
	{
		kernelInfo kernel_info_obj;
		kernel_info_obj.kernelinfo_function(opcode_arr[i],&kernel_info_obj.opcode_info_obj);

		kernelinfo_out.push_back(kernel_info_obj);

		//Mapping opcode and index id
		//kernelinfo_obj[i].opcode_idx_pair.insert(pair<int,int>(int(opcode_arr[i]),i));

		//display_kernel_info(&kernelinfo_obj[i].opcode_info_obj);
		kernelinfo_map[opcode_arr[i]] = kernelinfo_out[i];
	}

	kernelInfo kernel_info_obj = kernelinfo_map[opcode_arr[0]];
	display_kernel_info(&kernel_info_obj.opcode_info_obj);


#if 0
	std::map<int,int> example = {{HW_CONV,0},{SW_SOFTMAX,1}};
	auto search = example.find(HW_CONV);

	if(search != example.end()) {
		//std::cout << "Found " << search->first << " " << search->second << '\n';
	}
	else {
		std::cout << "Not found\n";
	}
#endif
}
#endif
