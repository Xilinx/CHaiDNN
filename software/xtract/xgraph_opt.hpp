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

#ifndef XTRACT_XGRAPH_OPT_HPP_
#define XTRACT_XGRAPH_OPT_HPP_
#include <sstream>
#include <queue>
#include "../common/kernelinfo_class.h"
#include "xgraph.hpp"

typedef map<string, XLayer*> mapStrXLayer;
typedef map<string, XLayer*>::iterator mapStrXLayer_it;

/*enum xtype_cust { _XTYPE_CHAR=1,_XTYPE_SHORT,_XTYPE_INT,_XTYPE_FLOAT};
typedef xtype_cust xtype_cust_e;*/

struct io_type{
	vector<_datatype_e> input_type;
	vector<_datatype_e> output_type;
};
typedef struct io_type io_type_e;

struct io_mem_data_type{
	string io_mem_type;
	_datatype_e io_data_type;
};
typedef struct io_mem_data_type io_mem_data_type_e;

// void checkGraphInsanity(XGraph* graph);

/// class structure to hold the io blob position,handle name, handle shape and its memory allocation type based on the layer type

class HBlob{

public:

	string handle;				///< unique name for IO blob
	int pos; 				///< blob partition position(usefull in the concat)
	string mem_type;			///< memory type is non_cacheable or cacheable or malloc
	_datatype_e io_data_type; /// io buffer data type
	vector <int> handle_shape; 		///< max blob shape, same for all HBlob in a network
};

class Handle_depth_info{

public:
	string free_handle;
	int degree_num;
	int degree_blob_num;
};

/// class structure to hold the io blob position and belonging layer index from exe sequence

class io_buffer_pos{

public:
	int pos;
	int layerindex;	

};


/// class structure to hold the io peers blob position and its shape

class io_peers{

public:
	int pos;
	vector<int> shape;

};

class io_peers_name{

public:
	int pos;
	string name;
};

/// XlayerData class is placeholder for capturing the network layer information and also capture some layer dependent info for scheduling

class XlayerData{

public:

	XLayer *hw_ops;				// pointer to the original XLayer in graph to access parameters
	vector < io_buffer_pos > prev_layers_ID;		// previous layers index in the sequence vector
	vector< io_buffer_pos > next_layer_ID;		// next layers index in the sequence vector
	vector < io_peers > ip_peers_shapes; // stores the shape of input of peer layers for offset calculations
	vector < io_peers > op_peers_shapes; // stores the shape of output of peer layers for offset calculations
	vector < io_peers > ip_peers_shapes_total; // stores the shape of input of peer layers for offset calculations
	vector < io_peers > op_peers_shapes_total; // stores the shape of output of peer layers for offset calculations
	vector <HBlob> ip_blob;			// vector of input blob handle
	vector <HBlob> op_blob;			// vector of output blob handle
	//void xlayer_print(vector<XlayerData> & xlayer_seq);
	int cur_seq_id;
	vector<int> meanShape;
	string meanFile;
	string layer_type;
	int resize_height;
	int resize_width;
	int opcode;
	int last_layer_id;
};

/// class structure to hold the index of blobs and its name from network graph
class BDegree{

public:
	int blobs_degree;
	vector < string > blobs_name;
};

/// XGraphOpt class has hardware dependent / independent optimization member functions
class XGraphOpt {

public:

	void opt_data(XGraph* graph);

	/// relu optimization and update xgraph & opcode value
	void opt_relu(XGraph *xgraph_opt);

	/// dropout optimization and update xgraph & opcode value
	void opt_dropout(XGraph *xgraph_opt);

	/// concat optimization and update xgraph & opcode value
	void opt_concat(XGraph *xgraph_opt);

	/// deconv optimization and update xgraph & opcode value
	void opt_deconv(XGraph *xgraph_opt);

	/// priorbox optimization and update xgraph & opcode value
	void opt_priorbox(XGraph* xgraph_opt);

	/// flatten optimization and update xgraph
	void opt_flatten(XGraph* xgraph_opt);

	/// reshape optimization and update xgraph
	void opt_reshape(XGraph* xgraph_opt);

	/// lrn optimization and update xgraph & opcode value
	void opt_lrn(XGraph* xgraph_opt);

	/// batchNorm optimization and update xgraph & opcode value
	void opt_batchNorm(XGraph* xgraph_opt,vector <string> &bn_opt_table);

	/// fuse_bn_conv optimization and update xgraph & opcode value
	void opt_fuse_bn_conv(XGraph* xgraph_opt,vector <string> &bn_opt_table);

	/// scale optimization and update xgraph & opcode value
	void opt_scale(XGraph* xgraph_opt);

	/// eltwise optimization and update xgraph & opcode value
	void opt_eltwise(XGraph* xgraph_opt);
	void opt_pool2conv(XGraph* xgraph_opt);
	void opt_fc2conv(XGraph* xgraph_opt);
	void opt_pool_conv_fuse(XGraph* xgraph_opt);

	/// custom_layer optimization and update xgraph & opcode value
	void custom_layer(XGraph* xgraph_opt);

	/// update_xgraph_pack_unpack_layer optimization and update xgraph with pack and unpack layers

	void update_xgraph_pack_unpack_layer(XGraph* xgraph_opt,vector < BDegree >& Bdepth,kernelInfo& opt_descriptor,int &in_put_layer);

	/// conv_bn_scale optimization and update xgraph & opcode value
	void opt_conv_bn_scale(XGraph* xgraph_opt);

	/// crelu optimization and update xgraph & opcode value
	void opt_crelu(XGraph* xgraph_opt);

	/// separable_conv optimization and update xgraph & opcode value
	void opt_separable_conv(XGraph* xgraph_opt);
	void opt_separable_conv_only(XGraph* xgraph_opt);
	void opt_sw_eltwise(XGraph* xgraph_opt);

	/// parallel_conv_fuse optimization and update xgraph & opcode value
	void opt_parallel_conv_fuse(XGraph* xgraph_opt);

	/// separable_conv_no_bias optimization and update xgraph & opcode value
	void opt_separable_conv_no_bias(XGraph* xgraph_opt,vector<string> &bn_opt_table);

};

#endif /* XGRAPH_OPT_HPP_ */
