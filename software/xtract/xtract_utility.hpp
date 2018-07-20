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

#ifndef XTRACT_XTRACT_UTILITY_HPP_
#define XTRACT_XTRACT_UTILITY_HPP_


#include "xgraph_opt.hpp"
#include "xgraph.hpp"
//#include "../common/kernelinfo_class.h"
//#include "../include/hw_settings.h"
/// find the alignment size with multiple of y
#define AlignSize(x,y) (x%y==0)? x : ((x/y+1)*y)

struct xtract_opcode_num
{
	std::string layer_name;
	opcode_num_e opcode;
};

struct xtract_layer_type
{
	std::string layer_name;
	_module_target_e layer_type;
};

struct opc_accl_s{
	_module_target_e accel_type;
	opcode_num_e opc_e;
};
struct blob_cons_prod_names{
	std::string cons_name;
	std::string prod_name;
};
typedef struct blob_cons_prod_names blob_cons_prod_names_s;



typedef	struct opc_accl_s opc_accl_s_e;

void update_layer_io_type(map<string,struct io_type> &LayerIOType,kernelInfo& opt_descriptor);

/// its take input as layer name and xgraph and delete layer from xgraph
void xgrap_layer_delete(XGraph *xgraph, string &delate_layer_name);

/// For debug purpose print mem_type for eache blob

void mem_type_print(map<string, string> &mem_type_table);

/// For debug purpose print xlayerdata structure variables, it takes  xlayer exe sequence

void xlayer_print(vector<XlayerData> & xlayer_seq);

/// Utility function to find the size of the given vector

int vectDim (vector<int>& src);

/// Utility function to find max blob size from xgraph and return it

void maxsize_blob(XGraph *xgraph_opt,vector <int> &max_shape);

/// Find the blobs degree based on BFS algo with blob & layers. It takes input as xgraph and output is blobs degreee vector(Bdegree)

void find_blobs_degree(XGraph *xgraph,vector<BDegree> &Bdepth);

/// For debug print the variables from Bdegree vector

void print_blobs_degre(vector < BDegree > &Bdepth);

/// Create sequence table from xgraph using Bdegree details. It also generate layer_seq_table &  layer_type_table. Where layer_seq_table index based on the degree,layer_type_table has sw/hw based on the layer type.

void create_seq_num_table(vector < XlayerData > &xlayer_seq,map<string, int> &layer_seq_table,map<string, string> &layer_type_table,vector < BDegree >&Bdepth,XGraph *xgraph);

/// Update the next & previous layer index based on the layer_seq_table map. Populate details into xlayer_sequency vector.

void update_prev_next_layer_info(vector < XlayerData > &xlayer_seq, map<string, int> &layer_seq_table,XGraph *xgraph);

/// Utility function to find which layer is sw or hw

void find_hw_and_sw_layer_type(vector <struct xtract_layer_type> &xtract_layer_type_e,string &layer_type,int &hs_ops_type);

/// Utility function to find which blob is allocate which mem type (NON_CACHEABLE,CACHEABLE,MALLOC)

void find_mem_type_io_buffers(XGraph *xgraph,vector < BDegree > &Bdepth,map<string, string> &layer_type_table,map<string, string> &mem_type_table,map<string,struct io_type> &LayerIOType,kernelInfo& opt_descriptor);

/// Update mem type info into xlayer_sequency vector

void update_io_mem_type(vector < XlayerData > & xlayer_seq,XGraph *xgraph,vector < BDegree > &Bdepth,map<string, string> &layer_type_table,map<string, io_mem_data_type_e> &mem_type_table, map<string,io_type_e> &LayerIOType,kernelInfo& opt_descriptor);


/// Find the reuse IO buffers and update into exe xlayer sequence

void update_buffer_reuse_xlayer_seq(vector < XlayerData > & xlayer_seq,vector < BDegree > &Bdepth,map<string, int> &layer_seq_table,map<string, io_mem_data_type_e> &mem_type_table,XGraph *xgraph,vector < BDegree > &reUseBdepth);

/// Update layer opcode into exe sequence vector
void update_xlayer_opcode(XGraph *xgraph,kernelInfo& opt_descriptor);

///  Utility function to find layer opcode based on the layer execution
void find_xlayer_opcode(vector < struct xtract_opcode_num> xlayers_opcode_e,string &layer_type,int &hs_ops_type);

/// Utility function to check network input height & width

void checkGraphInsanity(XGraph* graph);

/// Exe sequence generator function ,its generate sequence of layer with hw dependent and independent optimizations also it does and reuse opt
/// XlayerData class is placeholder for capturing the network layer information
/// xgraph class is placeholder for capturing the network graph as a layers & blobs
void xlayer_sequence_generator(vector < XlayerData > & xlayer_seq,XGraph *xgraph,kernelInfo& opt_descriptor,int in_put_layer);


/// Utility function to check current blob index with its dependents, used in buffer reuse model

void check_free_handle_layers_indx_with_curr_blob_layers_indx(vector < BDegree > &Bdepth,vector < XlayerData > & xlayer_seq,XGraph *xgraph,map<string, int> &layer_seq_table,int free_handle_degree_num,int free_handle_blob_num,int curr_degree_num,int curr_blob_num,int *res);

/// Read layers information from hardware configuration file to map its hardware or software

void get_hw_and_sw_layer(vector <struct xtract_layer_type> &xtract_layer,kernelInfo& opt_descriptor);

/// Read opcode list from hardware configuration file

void get_opcode_xlayer(vector < struct xtract_opcode_num> &xlayers_opcode,kernelInfo& opt_descriptor);

/// Utility function to find the max Q factor within the consumers & produces in the current id buffer(blob) and populate in the xgraph xlayer

void find_max_q_factor(XGraph* xgraph_opt);

/// Utility function to find the total output planes and update in the xgraph layers

void find_total_output_plane(XGraph *xgraph);

/// Find the common port format for consumers & produces.

void find_input_port_type(XGraph *xgraph,kernelInfo& opt_descriptor,map < string, XBlob*>::iterator &it_blob,Port_info_s &pack_port,vector<Port_info_s> &cons_in_pack,int &change_status);
void find_output_port_type(XGraph *xgraph,kernelInfo& opt_descriptor,map < string, XBlob*>::iterator &it_blob,Port_info_s &pack_port,vector<Port_info_s> &prod_in_pack,int &change_status);

void prod_port_comp(Port_info_s &pack_port,vector<Port_info_s> &layer_port,vector<int> &io_match);
void prod_port_comp_out(Port_info_s &pack_port,vector<Port_info_s> &out_port,vector<int> &io_match);
void prod_port_comp_input(Port_info_s &pack_port,vector<Port_info_s> &in_port,vector<int> &io_match);
bool check_producers_port(vector<Port_info_s> &prod_port_format);
bool check_consumers_port(vector<Port_info_s> &cons_port_format);
bool check_cons_prod_port_same(Port_info_s &prod_port,Port_info_s &cons_port);
void inser_format_conversion_layer_below_it(XGraph *xgraph,map < string, XBlob*>::iterator &it_blob,kernelInfo& opt_descriptor,Port_info_s &output_port,Port_info_s &input_port,int &custom_layer_cnt);
void inser_format_conversion_layer_above_it(XGraph *xgraph,map < string, XBlob*>::iterator &it_blob,kernelInfo& opt_descriptor,Port_info_s &output_port,Port_info_s &input_port,int &custom_layer_cnt);
#endif /* XTRACT_XTRACT_UTILITY_HPP_ */
