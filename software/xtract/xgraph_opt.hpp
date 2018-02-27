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


#ifndef XGRAPH_OPT_HPP_
#define XGRAPH_OPT_HPP_
#include <sstream>
#include <queue>
#include "xgraph.hpp"
#include "../include/hw_settings.h"
#include "lib_types.hpp"

void checkGraphInsanity(XGraph* graph);

class HBlob{

public:

	string handle;				// unique name for IO blob
	int pos; 				// blob partition position(usefull in the concat)
	string mem_type;			// memory type is non_cacheable or cacheable or malloc
	vector <int> handle_shape; 		// max blob shape, same for all HBlob in a network
};

class Handle_depth_info{

public:
	string free_handle;
	int degree_num;
	int degree_blob_num;
};

class io_buffer_pos{

public:
	int pos;
	int layerindex;	

};

/*class int_type{
public:

int layer_num;
};*/

class io_peers{

public:
	int pos;
	vector<int> shape;

};

class XlayerData{

public:

	XLayer *hw_ops;				// pointer to the original XLayer in graph to access parameters
	vector < io_buffer_pos > prev_layers_ID;		// previous layers index in the sequence vector
	vector< io_buffer_pos > next_layer_ID;		// next layers index in the sequence vector
	vector < io_peers > ip_peers_shapes; // stores the shape of input of peer layers for offset calculations
	vector < io_peers > op_peers_shapes; // stores the shape of output of peer layers for offset calculations
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
};

class BDegree{

public:
	int blobs_degree;
	vector < string > blobs_name;
};

class XGraphOpt {

public:

	void xi_opt_relu(XGraph *xgraph_opt);
	void xi_opt_dropout(XGraph *xgraph_opt);
	void xi_opt_concat(XGraph *xgraph_opt);
	void xi_opt_deconv(XGraph *xgraph_opt);
	void xi_opt_priorbox(XGraph* xgraph_opt);
	void xi_opt_flatten(XGraph* xgraph_opt);
	void xi_opt_reshape(XGraph* xgraph_opt);
	void xi_opt_lrn(XGraph* xgraph_opt);
	void xi_opt_batchNorm(XGraph* xgraph_opt,vector <string> &bn_opt_table);
	void xi_opt_fuse_bn_conv(XGraph* xgraph_opt,vector <string> &bn_opt_table);
	void xi_opt_scale(XGraph* xgraph_opt);
	void xi_opt_eltwise(XGraph* xgraph_opt);
	void xi_opt_find_max_q_factor(XGraph* xgraph_opt);
	void xi_opt_conv_bn_scale(XGraph* xgraph_opt);
	void xi_opt_crelu(XGraph* xgraph_opt);

};
void xi_find_blobs_degree(XGraph *xgraph,vector<BDegree> &Bdepth);
void xlayer_print(vector<XlayerData> & xlayer_seq);
void xi_print_blobs_degre(vector < BDegree > &Bdepth);
void xlayer_sequence_generator(vector < XlayerData > & xlayer_seq,XGraph *xgraph);
void xi_get_hw_and_sw_layer(vector <struct xtract_layer_type> &xtract_layer);
void xi_get_opcode_xlayer(vector < struct xtract_opcode_num> &xlayers_opcode);

#endif /* XGRAPH_OPT_HPP_ */
