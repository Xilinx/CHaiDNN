/*----------------------------------------------------
 * Copyright 2017 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ----------------------------------------------------*/

#include "xgraph.hpp"

int XBlob::UID = 0;
int XLayer::UID = 0;

// Helper function to return a string representation of XLayer
// [bottom1, bottom2 ....] --> {Layer Info} --> [top1, top2, ... ]
string XLayer::str(bool use_user_names)
{
	string printName = use_user_names ? name : uname;
	string tmp = "";

	// "[ bottom1, bottom2, ... ]"
	tmp     +=  "[";
	for(int i = 0; i < bottom.size(); i++)
	{
		tmp += bottom.at(i).str(use_user_names) ;
		if(i<bottom.size()-1)
			tmp += ", ";
	}
	tmp     +=  "]";

	//  " --> { Name - Layer Info } --> "
	tmp     +=  " --> { ";
	tmp     += printName + " - " ;
	tmp     +=  _getLayerStr() + " } --> ";

	// "[ top1, top2, ... ]"
	tmp     +=  "[";
	for(int i = 0; i < top.size(); i++)
	{
		tmp += top.at(i).str(use_user_names) ;
		if(i<top.size()-1)
			tmp += ", ";
	}
	tmp     +=  "]";

	return tmp;
}

XLayer::XLayer() 
:uname("L"+to_string(UID++)),
 opcode(-1),
 inPlace(false),
 conv_params(NULL), pool_params(NULL)
{
	init();
}

// TODO : @ARK : Change this if-else-if chain to switch-case somehow.
// TODO : ARK : Initialize all params pointers to NULL
XLayer::XLayer(string _username, string _type, string _top_name) 
: name(_username), type(_type), output_file(generateOutputFilename(_username)),
  uname("L"+to_string(UID++)),
  opcode(-1),
  inPlace(false),
  conv_params(NULL), pool_params(NULL)
{
	init(); 

	// Now create the parameters
	if(type == "Convolution")
		conv_params = new ConvolutionParameter();
	else if(type == "ReLU")
		relu_params = new ReLUParameter();
	else if(type == "Pooling")
		pool_params = new PoolingParameter();
	else if(type == "Softmax")
		softmax_params = new SoftmaxParameter();
	else if(type == "InnerProduct")
		fc_params = new FCLayerParameter();
	else if(type == "Argmax")
		argmax_params = new ArgmaxParameter();
	else if(type == "Dropout")
		dropout_params = new DropoutParameter();
	else if(type == "Input")
		input_params = new InputParameter();
	else if(type == "LRN")
		lrn_params = new LRNParameter();
	else if(type == "Concat")
		concat_params = new ConcatParameter();
	else if(type == "Deconvolution")
		deconv_params = new DeconvolutionParameter();
	else if(type == "Crop")
		crop_params = new CropParameter();
	else if(type == "Flatten")
		flatten_params = new FlattenParameter();
	else if(type == "Permute")
		permute_params = new PermuteParameter();
	else if(type == "L2Normalize")
		l2norm_params = new L2NormalizeParameter();
	else if(type == "PriorBox")
		priorbox_params = new PriorBoxParameter();
	else if(type == "Reshape")
		reshape_params = new ReshapeParameter();
	else if(type == "NMS")
		nms_params = new NMSParameter();
	else if(type == "Eltwise")
		eltwise_params = new EltwiseParameter();
	else if(type == "BatchNorm")
		batchnorm_params = new BatchNormParameter();
	else if(type == "Scale")
		scale_params = new ScaleParameter();
	else if(type == "Power")
		power_params = new PowerParameter();
	else if(type == "XCustom")
		xcustom_params = new XCustomParameter();
	else if(type == "XPack")
		xpack_params = new XPackParameter();
	else
	{
		cerr    << "[EX001] Layer type <" << type << "> doesn't match with any of the supported layers. " << endl;
		exit(-1);
	}
}

// TODO : @ARK : Change this if-else-if chain to switch-case somehow.
// OR if possible, get rid of those switch-case also
string XLayer::_getLayerStr()
{
	string tmp;
	if(type == "Convolution")
		tmp = conv_params->str();
	else if(type == "ReLU")
		tmp = relu_params->str();
	else if(type == "Pooling")
		tmp = pool_params->str();
	else if(type == "Softmax")
		tmp = softmax_params->str();
	else if(type == "InnerProduct")
		tmp = fc_params->str();
	else if(type == "Argmax")
		tmp = argmax_params->str();
	else if(type == "Dropout")
		tmp = dropout_params->str();
	else if(type == "Input")
		tmp = input_params->str();
	else if(type == "LRN")
		tmp = lrn_params->str();
	else if(type == "Concat")
		tmp = concat_params->str();
	else if(type == "Deconvolution")
		tmp = deconv_params->str();
	else if(type == "Crop")
		tmp = crop_params->str();
	else if(type == "Flatten")
		tmp = flatten_params->str();
	else if(type == "Permute")
		tmp = permute_params->str();
	else if(type == "L2Normalize")
		tmp = l2norm_params->str();
	else if(type == "PriorBox")
		tmp = priorbox_params->str();
	else if(type == "Reshape")
		tmp = reshape_params->str();
	else if(type == "NMS")
		tmp = nms_params->str();
	else if(type == "Eltwise")
		tmp = eltwise_params->str();
	else if(type == "BatchNorm")
		tmp = batchnorm_params->str();
	else if(type == "Scale")
		tmp = scale_params->str();
	else if(type == "Power")
		tmp = power_params->str();
	else if(type == "XCustom")
		tmp = xcustom_params->str();
	else if(type == "XPack")
		tmp = xpack_params->str();
	else
		tmp = "Not yet defined";

	return tmp;
}

// function to compute the output dimension of a layer
void XLayer::computeOutputDim()
{
	if(type == "Convolution")
		conv_params->computeOutputDim(bottom, top);
	else if(type == "Dropout")
		dropout_params->computeOutputDim(bottom, top);
	else if(type == "ReLU")
		relu_params->computeOutputDim(bottom, top);
	else if(type == "Pooling")
		pool_params->computeOutputDim(bottom, top);
	else if(type == "InnerProduct")
		fc_params->computeOutputDim(bottom, top);
	else if(type == "Argmax")
		argmax_params->computeOutputDim(bottom, top);
	else if(type == "Softmax")
		softmax_params->computeOutputDim(bottom, top);
	else if(type == "Deconvolution")
		deconv_params->computeOutputDim(bottom, top);
	else if(type == "Concat")
		concat_params->computeOutputDim(bottom, top);
	else if(type == "Crop")
		crop_params->computeOutputDim(bottom, top);
	else if(type == "Flatten")
		flatten_params->computeOutputDim(bottom, top);
	else if(type == "Permute")
		permute_params->computeOutputDim(bottom, top);
	else if(type == "L2Normalize")
		l2norm_params->computeOutputDim(bottom, top);
	else if(type == "PriorBox")
		priorbox_params->computeOutputDim(bottom, top);
	else if(type == "Reshape")
		reshape_params->computeOutputDim(bottom, top);
	else if(type == "NMS")
		nms_params->computeOutputDim(bottom, top);
	else if(type == "LRN")
		lrn_params->computeOutputDim(bottom, top);
	else if(type == "Eltwise")
		eltwise_params->computeOutputDim(bottom, top);
	else if(type == "BatchNorm")
		batchnorm_params->computeOutputDim(bottom, top);
	else if(type == "Scale")
		scale_params->computeOutputDim(bottom, top);
	else if(type == "Power")
		power_params->computeOutputDim(bottom, top);
	else if(type == "XCustom")
		xcustom_params->computeOutputDim(bottom, top);
	else if(type == "XPack")
		xpack_params->computeOutputDim(bottom, top);
	else
		cerr << type << " Layer output dimension computation is not implemented";
}

XLayer::~XLayer()
{
	if(type == "Convolution")
	{
		if(conv_params != NULL)
		{
			delete conv_params;
			conv_params = NULL;
		}
		if(pool_params != NULL)
		{
			delete pool_params;
			pool_params = NULL;
		}
	}
	else if(type == "ReLU")
	{
		delete relu_params;
		relu_params = NULL;
	}
	else if(type == "Dropout")
	{
		delete dropout_params;
		dropout_params = NULL;
	}
	else if(type == "InnerProduct")
	{
		delete fc_params;
		fc_params = NULL;
	}
	else if(type == "Argmax")
	{
		delete argmax_params;
		argmax_params = NULL;
	}
	else if(type == "Softmax")
	{
		delete softmax_params;
		softmax_params = NULL;
	}
	else if(type == "Pooling")
	{
		if(conv_params != NULL)
		{
			delete conv_params;
			conv_params = NULL;
		}
		if(pool_params != NULL)
		{
			delete pool_params;
			pool_params = NULL;
		}
	}
	else if(type == "Deconvolution")
	{
		delete deconv_params;
		deconv_params = NULL;
	}
	else if(type == "Concat")
	{
		delete concat_params;
		concat_params = NULL;
	}
	else if(type == "Crop")
	{
		delete crop_params;
		crop_params = NULL;
	}
	else if(type == "Flatten")
	{
		delete flatten_params;
		flatten_params = NULL;
	}
	else if(type == "Permute")
	{
		delete permute_params;
		permute_params = NULL;
	}
	else if(type == "L2Normalize")
	{
		delete l2norm_params;
		l2norm_params = NULL;
	}
	else if(type == "PriorBox")
	{
		delete priorbox_params;
		priorbox_params = NULL;
	}
	else if(type == "Reshape")
	{
		delete reshape_params;
		reshape_params = NULL;
	}
	else if(type == "NMS")
	{
		delete nms_params;
		nms_params = NULL;
	}
	else if(type == "LRN")
	{
		delete lrn_params;
		lrn_params = NULL;
	}
	else if(type == "Eltwise")
	{
		delete eltwise_params;
		eltwise_params = NULL;
	}
	else if(type == "BatchNorm")
	{
		delete batchnorm_params;
		batchnorm_params = NULL;
	}
	else if(type == "Scale")
	{
		delete scale_params;
		scale_params = NULL;
	}
	else if(type == "Power")
	{
		delete power_params;
		power_params = NULL;
	}
	else if(type == "XCustom")
	{
		delete xcustom_params;
		xcustom_params = NULL;
	}
	else if(type == "XPack")
	{
		delete xpack_params;
		xpack_params = NULL;
	}
	else
	{}
}

string XLayer::generateOutputFilename(const string& top_name, const string& dirname)
{
	string retName;
	if(top_name.empty()) {
		retName = "";
	}
	else {
		string tmpName(top_name);
		replace(tmpName.begin(), tmpName.end(), '/', '_');
		retName = dirname.empty() ? tmpName + "_out.txt" : dirname + "/" + tmpName + "_out.txt";	
	}
	return retName;
}

void XLayer::init()
{
	ip_bw = wt_bw = op_bw = 0;
	ip_fl = wt_fl = op_fl = 0;
	bn_mean_bw = bn_variance_bw = 0;
	bn_mean_fl = bn_variance_fl = 0;
	scale_gamma_bw = scale_beta_bw = 0;
	scale_gamma_fl = scale_beta_fl = 0;
	scale_gamma_by_std_fl = scale_gamma_by_std_bw = 0;

	th_layer_in = th_layer_out = 0.0f;

}

//------------------------- XGRAPH -----------------------------//

// Constructor 1
XGraph::XGraph() 
: layers(), blobs(),
  input_blob(""), output_blob("")
{}

// Destructor 
XGraph::~XGraph()
{ 
	for (map < string, XLayer* >::iterator it=layers.begin(); it != layers.end(); it++)
		delete it->second;
	for (map < string, XBlob* >::iterator it=blobs.begin(); it != blobs.end(); it++)
		delete it->second;

	cout << "deleted Xgraph " << endl;
}

void XGraph::print(bool use_user_names)
{
	cout << endl;
	cout << "Network : " << name <<  endl;
	// cout << "Mean Shape : " << TensorDimToString(meanShape) << endl;
	// cout << "Reshape Dim: " << transform_params.resize_height << "x" << transform_params.resize_width << endl;
	cout << "Input Blob : " << input_blob << endl;
	cout << "Output Blob : " << output_blob << endl;
	cout << "Start Layer : " << start_layer << endl;
	cout << "End Layer : " << end_layer << endl;
	cout << endl;
	cout << "# -------------------- Layers ------------------------ #" << endl;
	for(map<string, XLayer*>::iterator it = layers.begin(); it != layers.end(); it++)
	{
		cout << it->second->str(use_user_names) << endl;
		cout << it->second->quantization_scheme << endl;
		// cout << it->second->ip_bw << ":" << it->second->ip_fl << endl;
	}

	cout << endl;
	cout << "# -------------------- Blobs ------------------------ #" << endl;
	for(map<string, XBlob*>::iterator it = blobs.begin(); it != blobs.end(); it++)
	{
		cout << it->second->str(use_user_names) << endl;
		// cout << stringVectorToString(it->second->producers) << " >>> " << it->second->name << " >>> " << stringVectorToString(it->second->consumers) << endl;
	}
	cout << endl;
}

map<string, XBlob*>::iterator XGraph::checkIfBlobExists(const string& name, bool exit = false, bool exit_if = false)
{
	map<string, XBlob*>::iterator it = blobs.find(name);

	if(exit == true)
	{
		// If object not found and user wants to exit if object not found (exit_if == false)
		if((it == blobs.end()) && (exit_if == false))
		{
			cerr << "[EX002] Unrecognized Blob : " << name << endl;
			std::exit(-1);
		}

		// If object is found and user wants to exit if object is found (exit_if == true)
		if((it != blobs.end()) && (exit_if == true))
		{
			cerr << "[EX003] Already blob existing with same name: " << name << endl;
			std::exit(-1);
		}

	}
	return it;
}

vector<string> XGraph::getParentLayers(const string& layerName)
{
	vector<string> res;
	map<string, XLayer*>::iterator uut_it = layers.find(layerName);
	if(uut_it == layers.end())
	{
		cerr << "[EX007] LayerName not found in the graph layers. " << endl;
		exit(-1);
	}

	XLayer* uut = uut_it->second;

	// If layer is an in-place op, simply return previous op
	if(uut->inPlace) {
		XBlob* bottom_blob = uut->bottom[0].blob;
		vector<string>& producers = bottom_blob->producers;
		if(producers.size() > 1) {
			vector<string>::iterator it = std::find(producers.begin(), producers.end(), layerName);
			res.push_back(*(it-1));
		}
		return res;
	}

	// Go through all bottom Blobs of UUT, then through each producer of each bottom-blob
	// NB: A blob can have only one producer. If there is multiple, all of them except first one are inplace ops.
	// So if there is any in-place op in producer list, it is the only parent layer
	for(int i = 0; i < uut->bottom.size(); i++)
	{
		XBlob* bottom_blob = uut->bottom[i].blob;
		const vector<string>& producers = bottom_blob->producers;

		if(producers.size() == 0) continue;

		// If last producer is an inline op, it is the parent
		if(layers[producers.back()]->inPlace) {
			res.push_back(producers.back());
		}
		else {
			for(int j=0; j<bottom_blob->producers.size(); j++)
			{
				res.push_back(bottom_blob->producers[j]);
			}
		}
	}

	return res;


}


vector<string> XGraph::getChildLayers(const string& layerName)
{
	vector<string> res;
	map<string, XLayer*>::iterator uut_it = layers.find(layerName);
	if(uut_it == layers.end())
	{
		cerr << "[EX008] LayerName not found in the graph layers. " << endl;
		exit(-1);
	}

	XLayer* uut = uut_it->second;

	// If layer is an in-place op, simply return next op
	if(uut->inPlace) {
		XBlob* top_blob = uut->top[0].blob;
		vector<string>& consumers = top_blob->consumers;
		if(consumers.size() > 1) {
			vector<string>::iterator it = std::find(consumers.begin(), consumers.end(), layerName);
			// if next layer is also inplace, it is the child layer
			// else, every layer following this layer is a child layer
			vector<string>::iterator next = it + 1;
			if(layers[*next]->inPlace) {
				res.push_back(*next);
			}
			else {
				res.insert(res.end(), next, consumers.end());
			}
		}
		return res;
	}

	// Go through all top Blobs of UUT, then through each consumer of each top-blob
	// NB: A blob can have multiple consumers, but all inline consumers should come in front.
	// So if there is any in-place op in consumer list, it is the only child layer
	for(int i = 0; i < uut->top.size(); i++)
	{
		XBlob* top_blob = uut->top[i].blob;
		const vector<string>& consumers = top_blob->consumers;

		if(consumers.size() == 0) continue;

		// If first consumer is an inline op, it is the child
		if(layers[consumers.front()]->inPlace) {
			res.push_back(consumers.front());
		}
		else {
			for(int j=0; j<top_blob->consumers.size(); j++)
			{
				res.push_back(top_blob->consumers[j]);
			}
		}
	}

	return res;
}

// TODO : @ARK : Need to improve this algorithm for recursive deletion of layer/blob
vector<string> XGraph::deleteBlobFromGraph(const string& blobName)
{
	map<string, XBlob*>::iterator blob_it = blobs.find(blobName);
	vector<string> dummy;

	if(blob_it == blobs.end())
	{
		cerr << "[WXxxx] Trying to delete unknown blob name : " << blobName << endl;
		return dummy;
	}

	XBlob* blob = blob_it->second;

	// #. First handle the producers
	// #. Go through each layer in the producers list
	for(int i=0; i<blob->producers.size(); i++)
	{
		string layerName = blob->producers.at(i);
		map<string, XLayer*>::iterator layer_it = layers.find(layerName);

		// #. If a producer layer is available, remove blob from its top list
		if(layer_it != layers.end())
		{
			XLayer* tmp_layer = layer_it->second;
			vector<int> indexToDelete;

			// Don't erase in this loop because you will alter the iterator
			for(int i = 0; i < tmp_layer->top.size(); i++)
			{
				if(tmp_layer->top.at(i).blob->name == blobName)
				{
					indexToDelete.push_back(i);
				}
			}

			// Now delete the blobs
			for(int i=0; i<indexToDelete.size(); i++)
			{
				tmp_layer->top.erase(tmp_layer->top.begin()+indexToDelete[i]);
			}
		}
	}

	// #. Next handle the consumers
	// #. Go through each layer in the consumers list
	for(int i=0; i<blob->consumers.size(); i++)
	{
		string layerName = blob->consumers.at(i);
		map<string, XLayer*>::iterator layer_it = layers.find(layerName);

		// #. If a consumer layer is available, remove blob from its bottom list
		if(layer_it != layers.end())
		{
			XLayer* tmp_layer = layer_it->second;
			vector<int> indexToDelete;

			// Don't erase in this loop because you will alter the iterator
			for(int i = 0; i < tmp_layer->bottom.size(); i++)
			{
				if(tmp_layer->bottom.at(i).blob->name == blobName)
				{
					indexToDelete.push_back(i);
				}
			}

			// Now delete the blobs
			for(int i=0; i<indexToDelete.size(); i++)
			{
				tmp_layer->bottom.erase(tmp_layer->bottom.begin()+indexToDelete[i]);
			}
		}
	}

	// Finally delete the blob itself
	blobs.erase(blobName);
	delete blob;
	return dummy;
}

vector<string> XGraph::deleteLayerFromGraph(const string& layerName)
{
	vector<string> blobsToBeDeleted;
	map<string, XLayer*>::iterator layer_it = layers.find(layerName);

	if(layer_it == layers.end())
	{
		cerr << "[WXxxx] Trying to delete unknown layer name : " << layerName << endl;
		return blobsToBeDeleted;
	}

	XLayer* layer = layer_it->second;

	// #. First handle the top blobs
	for(vector<nameIndex>::iterator it = layer->top.begin(); it != layer->top.end(); it++)
	{
		int id = it->id;
		XBlob* blob = it->blob;

		// #. If this layer is the only producer of this blob, mark this blob for deletion
		if((blob->producers.size() == 1) && (blob->producers.at(0) == layerName))
			blobsToBeDeleted.push_back(blob->name);

		// #. Find the index of layer in the blob producer list
		vector<string>::iterator tmp_it = std::find(blob->producers.begin(), blob->producers.end(), layerName);
		if(tmp_it != blob->producers.end())
		{
			ptrdiff_t index = tmp_it - blob->producers.begin();

			// #. Remove the layer name from blob producers list
			blob->producers.erase(blob->producers.begin() + index);
			blob->producerDim.erase(blob->producerDim.begin() + index);
		}
	}

	// #. Handle the bottom blobs
	for(vector<nameIndex>::iterator it = layer->bottom.begin(); it != layer->bottom.end(); it++)
	{
		int id = it->id;
		XBlob* blob = it->blob;

		// #. Find the index of layer in the blob consumer list
		vector<string>::iterator tmp_it = std::find(blob->consumers.begin(), blob->consumers.end(), layerName);
		if(tmp_it != blob->consumers.end())
		{
			ptrdiff_t index = tmp_it - blob->consumers.begin();

			// #. Remove the layer name from blob consumers list
			blob->consumers.erase(blob->consumers.begin() + index);
			blob->consumerDim.erase(blob->consumerDim.begin() + index);
		}
	}

	// #. Delete the layer itself.
	layers.erase(layerName);
	return blobsToBeDeleted;
}

// void XGraph::setResizeShape(int resize_height, int resize_width)
// {
//     int num_channels = input_blob.blob->shape.at(1);
//     int input_height = input_blob.blob->shape.at(2);
//     int input_width = input_blob.blob->shape.at(3);
// 
//     // Resize_shape should be greater/equal to input_shape
//     ASSERT( (resize_height >= input_height) && (resize_width >= input_width),
//             EX009, "(resize_height >= input_height) && (resize_width >= input_width)" );
// 
//     transform_params.resize_height = resize_height;
//     transform_params.resize_width = resize_width;
//     
// }

// Routine to representational layer type based on actual layer type and opcode
string generateGraphLayerType(const string& layerType, int opcode)
{
	string graphLayerType = "";
	if(layerType == "Convolution") {
		switch (opcode) {
		case 10: graphLayerType = "BN [+ Scale]"; break;
		case 11: graphLayerType = "Eltwise"; break;
		case 12: graphLayerType = "BN + Conv"; break;
		case 18: graphLayerType = "CReLU"; break;
		case 19: graphLayerType = "3DS-Conv + Conv"; break;
		default: graphLayerType = "Convolution"; break;
		}
	}
	else if(layerType == "Pooling") {
		graphLayerType = opcode == 40 ? graphLayerType = "3DS-Conv" : "Pooling";
	}
	else {
		graphLayerType = layerType;
	}

	return graphLayerType;
}


void XGraph::drawGraph(const string& filename, const string& rankdir)
{
	std::ofstream output_file(filename.c_str());
	if(!output_file)
	{
		cerr << "[ERROR] Couldn't create " << filename << endl;
		exit(-1);
	}

	output_file << "digraph {" << '\n';
	output_file << "rankdir=" << rankdir << ";" << '\n';

	// Create a Set to keep unique edges
	vector<string> edges;
	map<string, string> errorUnits;
	string errorEdge = "[color=\"red\", style=\"bold\"]";

	// Generate all blobs
	for (map < string, XBlob* >::iterator it=blobs.begin(); it != blobs.end(); it++)
	{
		XBlob* blob = it->second;
		output_file << blob->uname
				<< "[label=\"" << blob->reuse_name << "\","
				<< "shape=ellipse" << ","
				<< "color=\"black\"" << ","
				<< "style=filled" << ","
				<< "fillcolor=\"#BBBBBB\"" << ","
				<< "];" << '\n';

		// keep all Producer->B connections in edgeSet
		for(int i=0; i<blob->producers.size(); ++i)
		{
			// Check if the producer is in the graph.Layers
			map<string, XLayer*>::iterator layer_it = layers.find(blob->producers[i]);

			// If it is there, add the connection to edge_set
			if(layer_it != layers.end())
			{
				XLayer* layer = layer_it->second;
				string connection = layer->uname + " -> " + blob->uname;
				edges.push_back(connection);
			}
			// If it is not there, create a dummy UID and add it to errorUnits
			else
			{
				// search if this wrong layer is already reported
				map<string, string>::iterator err_it = errorUnits.find(blob->producers[i]);
				string layerUName;

				// If it is not reported, create a UID and add it to the errorUnits
				if(err_it == errorUnits.end())
				{
					layerUName = "EL_" + to_string(errorUnits.size());
					errorUnits[blob->producers[i]] = layerUName;
					output_file << "// Non-existing producer layer: " << blob->producers[i] << " for blob: " << blob->name << '\n';
					output_file << layerUName
							<< "[label=\"" << blob->producers[i]  << "\","
							<< "shape=box" << ","
							<< "color=\"black\"" << ","
							<< "style=filled" << ","
							<< "fillcolor=\"red\"" << ","
							<< "];" << '\n';
				}
				else
				{
					layerUName = err_it->second;
				}
				string connection = layerUName + " -> " + blob->uname;
				edges.push_back(connection + "[color=\"red\", style=\"bold\"]"/*, label=\"chk_PR\"]"*/);
			}
		}

		// keep all B->Consumers to connections in edgeSet
		for(int i=0; i<blob->consumers.size(); ++i)
		{
			map<string, XLayer*>::iterator layer_it = layers.find(blob->consumers[i]);
			if(layer_it != layers.end())
			{
				XLayer* layer = layer_it->second;
				string connection = blob->uname + " -> " + layer->uname;
				edges.push_back(connection);
			}
			else
			{
				map<string, string>::iterator err_it = errorUnits.find(blob->consumers[i]);
				string layerUName;
				if(err_it == errorUnits.end())
				{
					layerUName = "EL_" + to_string(errorUnits.size());
					errorUnits[blob->consumers[i]] = layerUName;
					output_file << "// Non-existing consumer layer: " << blob->consumers[i] << " for blob: " << blob->name << '\n';
					output_file << layerUName
							<< "[label=\"" << blob->consumers[i]  << "\","
							<< "shape=box" << ","
							<< "color=\"black\"" << ","
							<< "style=filled" << ","
							<< "fillcolor=\"red\"" << ","
							<< "];" << '\n';
				}
				else
				{
					layerUName = err_it->second;
				}
				string connection = blob->uname + " -> " + layerUName;
				edges.push_back(connection + "[color=\"red\", style=\"bold\"]"/*,label=\"chk_CR\"]"*/);
			}
		}
	}

	std::sort(edges.begin(), edges.end());
	const int nEdges = edges.size();

	// Generate all Layers
	for (map < string, XLayer* >::iterator it=layers.begin(); it != layers.end(); it++)
	{
		XLayer* layer = it->second;
		output_file << layer->uname
				<< "[label=" << "\"" << layer->name << "\\n"
				<< "( " << generateGraphLayerType(layer->type, layer->opcode) << " )" << "\\n"
				<< "OpCode: " << layer->opcode << "\"" << ","
				<< "shape=box" << ","
				<< "color=\"black\"" << ","
				<< "style=filled" << ","
				<< "fillcolor=\"#44FF44\"" << ","
				<< "];" << '\n';

		// Keep all B->L connections in edgeSet
		for(int i=0; i<layer->bottom.size(); ++i)
		{
			string bottomName = layer->bottom[i].blob->name;
			map<string, XBlob*>::iterator blob_it = blobs.find(layer->bottom[i].blob->name);
			if(blob_it != blobs.end())
			{
				string tmpEdge = blob_it->second->uname + " -> " + layer->uname;
				if(!(std::binary_search(edges.begin(), edges.begin()+nEdges, tmpEdge)))
					edges.push_back(tmpEdge);
			}
			else
			{
				map<string, string>::iterator err_it = errorUnits.find(bottomName);
				string blobUName;
				if(err_it == errorUnits.end())
				{
					blobUName = "EB_" + to_string(errorUnits.size());
					errorUnits[bottomName] = blobUName;
					output_file << "// Non-existing bottom blob: " << bottomName << " for layer: " << layer->name << '\n';
					output_file << blobUName
							<< "[label=\"" << layer->bottom[i].blob->name  << "\","
							<< "shape=ellipse" << ","
							<< "color=\"black\"" << ","
							<< "style=filled" << ","
							<< "fillcolor=\"red\"" << ","
							<< "];" << '\n';
				}
				else
				{
					blobUName = err_it->second;
				}
				string connection = blobUName + " -> " + layer->uname;
				string tmpEdge = connection + "[color=\"red\", style=\"bold\"]";
				if(!(std::binary_search(edges.begin(), edges.begin()+nEdges, tmpEdge)))
					edges.push_back(tmpEdge);
			}
		}

		// Keep all L->B connections in edgeSet
		for(int i=0; i<layer->top.size(); ++i)
		{
			string topName = layer->top[i].blob->name;
			map<string, XBlob*>::iterator blob_it = blobs.find(layer->top[i].blob->name);
			if(blob_it != blobs.end())
			{
				string tmpEdge = layer->uname + " -> " + blob_it->second->uname;
				if(!(std::binary_search(edges.begin(), edges.begin()+nEdges, tmpEdge)))
					edges.push_back(tmpEdge);
			}
			else
			{
				map<string, string>::iterator err_it = errorUnits.find(topName);
				string blobUName;
				if(err_it == errorUnits.end())
				{
					blobUName = "EB_" + to_string(errorUnits.size());
					errorUnits[topName] = blobUName;
					output_file << "// Non-existing top blob: " << topName  << " for layer: " << layer->name << '\n';
					output_file << blobUName
							<< "[label=\"" << layer->top[i].blob->name  << "\","
							<< "shape=ellipse" << ","
							<< "color=\"black\"" << ","
							<< "style=filled" << ","
							<< "fillcolor=\"red\"" << ","
							<< "];" << '\n';
				}
				else
				{
					blobUName = err_it->second;
				}
				string connection = layer->uname + " -> " + blobUName;
				string tmpEdge = connection + "[color=\"red\", style=\"bold\"]";
				if(!(std::binary_search(edges.begin(), edges.begin()+nEdges, tmpEdge)))
					edges.push_back(tmpEdge);
			}
		}
	}       // end: all layers

	for(vector<string>::iterator it=edges.begin(); it != edges.end(); ++it)
	{
		output_file << *it << '\n';
	}

	output_file << "}";
	output_file.close();

}


void XGraph::prune(const string& start_layer_, const string& end_layer_) {

	// If nothing no start or end blob mentioned, simply return;
	string s = start_layer_.empty() ? start_layer : start_layer_;
	string e = end_layer_.empty() ? end_layer : end_layer_;

	if(s == start_layer && e == end_layer) return;

	// Check if start_layer and end_layer are in XGraph
	ELOG(!start_layer_.empty() && layers.find(start_layer_) == layers.end(), EP301, "Couldn't find specified start layer : " << start_layer_)
	ELOG(!end_layer_.empty() && layers.find(end_layer_) == layers.end(), EP301, "Couldn't find specified end layer: " << end_layer_)

	// Update input_blob and output_blob
	if(!start_layer_.empty()) {
		XLayer* layer = layers[start_layer_];
		vector<nameIndex> bottom_blobs  = layer->bottom;
		vector<nameIndex> top_blobs  = layer->top;
		ASSERT(bottom_blobs.size() <= 1, EP302, "First layer shouldn't have more than one input : " << start_layer_)
		input_blob = layer->type == "Input" ? top_blobs.at(0).blob->name : bottom_blobs.at(0).blob->name;
		start_layer = start_layer_;
	}

	if(!end_layer_.empty()) {
		XLayer* layer = layers[end_layer_];
		vector<nameIndex> bottom_blobs  = layer->bottom;
		vector<nameIndex> top_blobs  = layer->top;
		ASSERT(top_blobs.size() <= 1, EP303, "Last layer shouldn't have more than one output: " << end_layer_)
		output_blob = top_blobs.at(0).blob->name;
		end_layer = end_layer_;
	}

	// List of blobs and layers to be kept
	set<string> blobsToBeKept;
	set<string> layersToBeKept;

	list<string> stack;
	set<string> visitedLayers;
	map<string, int> layerScore;
	for(map<string, XLayer*>::iterator it=layers.begin(); it != layers.end(); ++it) {
		layerScore[it->first] = 0;
	}

	XLayer* slayer = layers[start_layer];
	XLayer* elayer = layers[end_layer];

	// Push the start layer to STACK & SET
	stack.push_back(start_layer);

//	cerr << "BFS START " << endl;
//	cerr << "Start : " << start_layer << endl;
//	cerr << "End : " << end_layer << endl;

	bool path_found = false;
	while(!stack.empty()) {
		// pop top of stack
//		cerr << "STACK : "; copy(stack.begin(), stack.end(), ostream_iterator<string>(cerr, " ")); cerr << endl;
		string cur_layer = stack.back();
//		cerr << "CUR_LAYER : " << cur_layer << endl;
		stack.pop_back();

//		cerr << "SCORE : " << endl;
//		for(map<string, int>::iterator it=layerScore.begin(); it!=layerScore.end(); ++it) {
//			if(it->second > 0) {
//				cerr << it->first << " : " << it->second << endl;
//			}
//		}

		// If it is the end_layer, backtrack
		// Don't include it in visited layers, because we will come back through different path.
		if(cur_layer == end_layer || layersToBeKept.find(cur_layer) != layersToBeKept.end()) {
//			cerr << "FOUND END LAYER " << endl;
			if(cur_layer == end_layer) path_found = true;

			string bt_layer = cur_layer;
			string layer = "";

			bool proceed = true;
			while(proceed) {
//				cerr << "TRACE BACK : " << bt_layer << endl;
				// Get cur_layer parent layers
				const vector<string>& parent_layers = getParentLayers(bt_layer);

				// In parent layers, check for visited layer, but not yet added to the PATH(layersToBeKept)
				bool all_already_processed = true;
				for(int i=0; i<parent_layers.size(); ++i) {
					layer = parent_layers[i];
					// Proceed if it is a visited layer
					if(layerScore[layer] > 0) {
							layersToBeKept.insert(layer);
							bt_layer = layer;
							all_already_processed = false;
							layerScore[layer]--;
							break; // out of for loop
					}
				} // end for

				// If all the parent layers are already processed, nowhere else to go
				// break out of back_track while loop
				if(all_already_processed) {
					proceed = false;
					break;	// out of while loop
				}

			}	// end while
			continue; // the next iteration in main while loop
		}	// end if

		// If layer is not marked as visited,
		if(layerScore[cur_layer] == 0) {

			// Mark it as visited by incrementing its score
			layerScore[cur_layer]++;

			// Get child layers of cur_layer
			const vector<string>& child_layers = getChildLayers(cur_layer);
//			cerr << "CHILD LAYERS: "; copy(child_layers.begin(), child_layers.end(), ostream_iterator<string>(cerr, " ")); cerr << endl;


			// And push them to stack
			stack.insert(stack.end(), child_layers.begin(), child_layers.end());
//			cerr << "STACK AT END: "; copy(stack.begin(), stack.end(), ostream_iterator<string>(cerr, " ")); cerr << endl;
		}
//		cerr << endl;


	}	// while loop

	// Finally add the end_layer
	if(path_found) {
		layersToBeKept.insert(end_layer);
	}
	else {
		cerr << "[EP308] No path found from "<< start_layer << " to "<< end_layer << endl;
		exit(1);
	}




	// EPILOG : Push blobs of all layers in the SET
	for(set<string>::iterator layer_it=layersToBeKept.begin(); layer_it != layersToBeKept.end(); ++layer_it) {
		const string& layer_name = *layer_it;
		const XLayer* layer = layers[layer_name];

		//. Push bottom
		for(int i=0; i<layer->bottom.size(); ++i) {
			blobsToBeKept.insert(layer->bottom[i].blob->name);
		}

		//. Push top
		for(int i=0; i<layer->top.size(); ++i) {
			blobsToBeKept.insert(layer->top[i].blob->name);
		}
	}

	//. ------------  HANDLE INPUT BLOB ------------------------ //
	//. First remove the reference to the producer (if any) from input_blob producer list
	//. But if the producer is a Input Layer and if it is the src layer, then don't
	//. Search for slayer in producer list, and delete upto it.
	if(slayer->bottom.size() > 0) {
		XBlob* b = slayer->bottom[0].blob;
		vector<string>::iterator pit = std::find(b->producers.begin(), b->producers.end(), slayer->name);
		int distance = std::distance(b->producers.begin(), pit);
		b->producers.erase(b->producers.begin(), b->producers.begin() + distance);
		b->producerDim.erase(b->producerDim.begin(), b->producerDim.begin() + distance);
	}

	//. Similarly get rid of any unwanted consumers of input blob
	//. Search for slayer in producer list, and delete upto it.
	if(slayer->bottom.size() > 0) {
		XBlob* b = slayer->bottom[0].blob;
		vector<string>::iterator pit = std::find(b->consumers.begin(), b->consumers.end(), slayer->name);
		int distance = std::distance(b->consumers.begin(), pit);
		b->consumers.erase(b->consumers.begin(), b->consumers.begin() + distance);
		b->consumerDim.erase(b->consumerDim.begin(), b->consumerDim.begin() + distance);
	}


	//. ------------  HANDLE OUTPUT BLOB ------------------------ //
	//. Next remove references to consumers (if any) from output_blob consumer list
	//. Network should have one & only one output blob
	XBlob* bb = elayer->top[0].blob;
	vector<string>::iterator cit = std::find(bb->consumers.begin(), bb->consumers.end(), elayer->name);
	if(cit == bb->consumers.end()) {
		bb->consumers.clear();
		bb->consumerDim.clear();
	}
	else {
		int distance = std::distance(bb->consumers.begin(), cit);
		bb->consumers.erase(bb->consumers.begin()+distance+1, bb->consumers.end() + distance);
		bb->consumerDim.erase(bb->consumerDim.begin()+distance+1, bb->consumerDim.end() + distance);
	}

	//. Next remove references to producers(if any) from output_blob producer list
	cit = std::find(bb->producers.begin(), bb->producers.end(), elayer->name);
	int distance = std::distance(bb->producers.begin(), cit);
	bb->producers.erase(bb->producers.begin()+distance+1, bb->producers.end() + distance);
	bb->producerDim.erase(bb->producerDim.begin()+distance+1, bb->producerDim.end() + distance);

	//. Now we have all layers and blobs to be kept. So delete rest of the guys
	for(map<string, XBlob*>::iterator it=blobs.begin(); it!=blobs.end(); ) {
		const string& name = it->first;
		if(blobsToBeKept.find(name) == blobsToBeKept.end()) {
			delete it->second;
			blobs.erase(it++);
		}
		else {
			++it;
		}
	}

	for(map<string, XLayer*>::iterator it=layers.begin(); it!=layers.end(); ) {
		const string& name = it->first;
		if(layersToBeKept.find(name) == layersToBeKept.end()) {
			delete it->second;
			layers.erase(it++);
		}
		else {
			++it;
		}
	}

	// Clear any hanging nodes
	clearNodes();

	// Check number of inputs/outputs to the subgraph
	vector<string> input_blobs = listOfInputBlobs();
	vector<string> output_blobs = listOfOutputBlobs();
	drawGraph("/tmp/maingraph.dot");

	if(input_blobs.size() != 1) {
		cerr << "[EP306] Network or sub-graph can have one & only one input. Current Inputs : ";
		std::copy(input_blobs.begin(), input_blobs.end(), ostream_iterator<string>(cerr, " ")); cerr << endl;
		exit(1);
	}

	if(output_blobs.size() != 1) {
		cerr << "[EP307] Network or sub-graph can have one & only one output. Current Outputs : ";
		std::copy(output_blobs.begin(), output_blobs.end(), ostream_iterator<string>(cerr, " ")); cerr << endl;
		exit(1);
	}

}

void XGraph::clearNodes() {
	// Clear hanging blob references from layers
	for(map<string, XLayer*>::iterator it=layers.begin(); it != layers.end(); ++it) {
		XLayer* layer = it->second;
		vector<nameIndex>::iterator blob_it;
		vector< vector<int> >::iterator shape_it;

		// Check Bottom first
		vector<nameIndex>& bottoms = layer->bottom;
		vector< vector<int> >& bottomShape = layer->bottomShape;
		for(blob_it=bottoms.begin(), shape_it=bottomShape.begin();
				blob_it != bottoms.end() && shape_it != bottomShape.end(); ) 	{

			const string& blob_name = blob_it->blob->name;
			// Check if this blob in XGraph.blobs, if not delete the reference
			map<string, XBlob*>::iterator b_it = blobs.find(blob_name);
			if(b_it == blobs.end()) {
				blob_it = bottoms.erase(blob_it);
				shape_it = bottomShape.erase(shape_it);
			}
			else {
				++blob_it;
				++shape_it;
			}
		}

		// Now Check Top
		vector<nameIndex>& tops = layer->top;
		vector< vector<int> >& topShape = layer->topShape;
		for(blob_it=tops.begin(), shape_it=topShape.begin();
				blob_it != tops.end() && shape_it != topShape.end(); ) {

			const string& blob_name = blob_it->blob->name;
			// Check if this blob in XGraph.blobs, if not delete the reference
			map<string, XBlob*>::iterator b_it = blobs.find(blob_name);
			if(b_it == blobs.end()) {
				blob_it = tops.erase(blob_it);
				shape_it = topShape.erase(shape_it);
			}
			else {
				++blob_it;
				++shape_it;
			}
		}
	}

	// Clear hanging layer references from blobs
	for(map<string, XBlob*>::iterator it=blobs.begin(); it != blobs.end(); ++it) {
		XBlob* blob = it->second;
		vector<string>::iterator layer_it;
		vector< vector<int> >::iterator dim_it;

		// Check Producers first
		vector<string>& producers = blob->producers;
		vector< vector<int> >& producerDim = blob->producerDim;
		for(layer_it=producers.begin(), dim_it=producerDim.begin();
				layer_it != producers.end() && dim_it != producerDim.end(); ) {

			const string& layer_name = *layer_it;
			map<string, XLayer*>::iterator l_it = layers.find(layer_name);
			if(l_it == layers.end()) {
				layer_it = producers.erase(layer_it);
				dim_it = producerDim.erase(dim_it);
			}
			else {
				++layer_it;
				++dim_it;
			}
		}

		// Check Consumers Next
		vector<string>& consumers = blob->consumers;
		vector< vector<int> >& consumerDim = blob->consumerDim;
		for(layer_it=consumers.begin(), dim_it=consumerDim.begin();
				layer_it != consumers.end() && dim_it != consumerDim.end(); ) {

			const string& layer_name = *layer_it;
			map<string, XLayer*>::iterator l_it = layers.find(layer_name);
			if(l_it == layers.end()) {
				layer_it = consumers.erase(layer_it);
				dim_it = consumerDim.erase(dim_it);
			}
			else {
				++layer_it;
				++dim_it;
			}
		}
	}
}

vector<string> XGraph::listOfInputBlobs() {
	vector<string> inputs;
	for(map<string, XBlob*>::iterator it=blobs.begin(); it != blobs.end(); ++it) {
		XBlob* blob = it->second;
		if(blob->producers.size() == 0) {
			inputs.push_back(blob->name);
		}
		else {	// Special check if all producers are inPlace ops
			bool add_this_layer = true;
			// if atleast one of them is not an inplace producer, then it is not an input blob
			for(int i=0; i<blob->producers.size(); ++i) {
				if(layers[blob->producers[i]]->inPlace == false) {
					add_this_layer = false;
					break;
				}
			}
			if(add_this_layer) inputs.push_back(blob->name);
		}
	}

	// Put special case for number of input layers
	for(map<string, XLayer*>::iterator it=layers.begin(); it != layers.end(); ++it) {
		XLayer* layer = it->second;
		if(layer->type == "Input") {
			inputs.push_back(layer->top[0].blob->name);
		}
	}

	return inputs;
}

vector<string> XGraph::listOfOutputBlobs() {
	vector<string> outputs;
	for(map<string, XBlob*>::iterator it=blobs.begin(); it != blobs.end(); ++it) {
		XBlob* blob = it->second;
		if(blob->consumers.size() == 0) {
			outputs.push_back(blob->name);
		}
		else {	// Special check if all consumers are inPlace ops
			bool add_this_layer = true;
			// if atleast one of them is not an inplace consumer, then it is not an output blob
			for(int i=0; i<blob->consumers.size(); ++i) {
				if(layers[blob->consumers[i]]->inPlace == false) {
					add_this_layer = false;
					break;
				}
			}
			if(add_this_layer) outputs.push_back(blob->name);
		}
	}
	return outputs;
}





