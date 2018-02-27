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


#include "xgraph.hpp"

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

// TODO : @ARK : Change this if-else-if chain to switch-case somehow.
XLayer::XLayer(string _username, string _type) 
: name(_username), type(_type), output_file(generateOutputFilename()),
  ip_bw(0), wt_bw(0), op_bw(0),
  ip_fl(0), wt_fl(0), op_fl(0),
  user_ip_bw(0), user_wt_bw(0), user_op_bw(0),
  user_ip_fl(0), user_wt_fl(0), user_op_fl(0),
  bn_mean_bw(0), bn_variance_bw(0),
  user_bn_mean_bw(0), user_bn_variance_bw(0),
  bn_mean_fl(0), bn_variance_fl(0),
  user_bn_mean_fl(0), user_bn_variance_fl(0),
  scale_gamma_bw(0), scale_beta_bw(0),
  user_scale_gamma_bw(0), user_scale_beta_bw(0),
  scale_gamma_fl(0), scale_beta_fl(0),
  user_scale_gamma_fl(0), user_scale_beta_fl(0),
  scale_gamma_by_std_fl(0), scale_gamma_by_std_bw(0),
  user_scale_gamma_by_std_fl(0), user_scale_gamma_by_std_bw(0)
{
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
    else
        cerr << type << " Layer output dimension computation is not implemented";
}

XLayer::~XLayer()
{
    if(type == "Convolution")
    {
        delete conv_params;
        conv_params = NULL;
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
        delete pool_params;
        pool_params = NULL;
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
    else
    {}
}

string XLayer::generateOutputFilename(const string& dirname)
{
    string tmpName(name); 
    replace(tmpName.begin(), tmpName.end(), '/', '_');
    string retName = dirname.empty() ? tmpName + "_out.txt" : dirname + "/" + tmpName + "_out.txt";
    return retName;
}

//------------------------- XGRAPH -----------------------------//

// Constructor 1
XGraph::XGraph() 
: layers(), blobs(), _blobcounter(0), _layercounter(0)
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
    cout <<  "Network : " << name <<  endl;
    cout << "Mean Shape : " << TensorDimToString(meanShape) << endl;
    cout << "Mean File: " << meanFile << endl;
    cout << "Reshape Dim: " << transform_params.resize_height << "x" << transform_params.resize_width << endl;
    cout << endl;
    cout << "# -------------------- Layers ------------------------ #" << endl;
    for(map<string, XLayer*>::iterator it = layers.begin(); it != layers.end(); it++)
    {
        cout << it->second->str(use_user_names) << endl;
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

string XGraph::getUniqueBlobName()
{
    string tmp = "B" + to_string(_blobcounter);
    _blobcounter++;
    return tmp;
}

string XGraph::getUniqueLayerName()
{
    string tmp = "L" + to_string(_layercounter);
    _layercounter++;
    return tmp;
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

    // Go through all bottom Blobs of UUT, then through each producer of each bottom-blob
    for(int i = 0; i < uut->bottom.size(); i++)
    {
        XBlob* bottom_blob = uut->bottom[i].blob;
        for(int j=0; j<bottom_blob->producers.size(); j++)
        {
            res.push_back(bottom_blob->producers[j]);
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

    // Go through all top Blobs of UUT, then through each consumer of each top-blob
    for(int i = 0; i < uut->top.size(); i++)
    {
        XBlob* top_blob = uut->top[i].blob;
        for(int j=0; j<top_blob->consumers.size(); j++)
        {
            res.push_back(top_blob->consumers[j]);
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

void XGraph::setResizeShape(int resize_height, int resize_width)
{
    int num_channels = input_blob.blob->shape.at(1);
    int input_height = input_blob.blob->shape.at(2);
    int input_width = input_blob.blob->shape.at(3);

    // Resize_shape should be greater/equal to input_shape
    ASSERT( (resize_height >= input_height) && (resize_width >= input_width),
            EX009, "(resize_height >= input_height) && (resize_width >= input_width)" );

    transform_params.resize_height = resize_height;
    transform_params.resize_width = resize_width;
    
}

