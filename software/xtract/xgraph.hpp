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


#ifndef __XGRAPH_HPP__
#define __XGRAPH_HPP__

#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>

#include "xparameter.hpp"


using namespace std;

class XLayer
{
public:
    // fields
    string name;                    // User-provided name in network file
    string uname;                   // Unique name assigned by the program
    string type;
    vector<nameIndex> bottom;
    vector<nameIndex> top;

    string output_file;             // Reference layerwise output filename for debugging
    
    // Replicate bottom & top shapes here for easy-access from xChange
    // TODO : @ARK : Think how you are going to handle the Channels in case of Layer Fusion
    vector< vector<int> > bottomShape;
    vector< vector<int> > topShape;
    
    // device denote where this layer operation takes place
    // Right now, it directly comes from HW_CONFIG file
    Engine device;

    // Keep a pointer to all type of Parameters here
    // TODO : @ARK : Simplify this part, this long list is head-ache on long run
    // Polymorphism doesn't work here, object slicing is the issue.
    // TODO : @ARK : Keep them in sorted order. Otherwise it is going to be head-ache later (use vim : sort)
    ConvolutionParameter *conv_params       ;
    ReLUParameter* relu_params              ;
    PoolingParameter* pool_params           ;
    SoftmaxParameter* softmax_params        ;
    FCLayerParameter* fc_params             ;
    ArgmaxParameter* argmax_params          ;
    DropoutParameter* dropout_params        ;
    InputParameter* input_params            ;
    LRNParameter* lrn_params                ;
    ConcatParameter* concat_params          ;
    CropParameter* crop_params              ;
    DeconvolutionParameter *deconv_params   ;
    FlattenParameter *flatten_params        ;
    PermuteParameter *permute_params        ;
    L2NormalizeParameter *l2norm_params     ;
    PriorBoxParameter* priorbox_params      ;
    ReshapeParameter* reshape_params        ;
    NMSParameter* nms_params                ;
    EltwiseParameter* eltwise_params        ;
    BatchNormParameter* batchnorm_params    ;
    ScaleParameter* scale_params            ;
    PowerParameter* power_params            ;

    // Precision Parameters that should be passed to Host
    int ip_bw, wt_bw, op_bw;
    int ip_fl, wt_fl, op_fl;

    int bn_mean_fl, bn_variance_fl;
    int bn_mean_bw, bn_variance_bw;

    int scale_gamma_fl, scale_beta_fl;
    int scale_gamma_bw, scale_beta_bw;

    int scale_gamma_by_std_fl, scale_gamma_by_std_bw;

    // Precision Parameters that comes from user
    int user_ip_bw, user_wt_bw, user_op_bw;
    int user_ip_fl, user_wt_fl, user_op_fl;

    int user_bn_mean_fl, user_bn_variance_fl;
    int user_bn_mean_bw, user_bn_variance_bw;

    int user_scale_gamma_fl, user_scale_beta_fl;
    int user_scale_gamma_bw, user_scale_beta_bw;

    int user_scale_gamma_by_std_fl, user_scale_gamma_by_std_bw;

    // HW Params
    int opcode;

    // methods

    // Constructor 1
    XLayer() 
    :ip_bw(0), wt_bw(0), op_bw(0),
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
     user_scale_gamma_by_std_fl(0), user_scale_gamma_by_std_bw(0),
     scale_gamma_by_std_fl(0), scale_gamma_by_std_bw(0)
    {}
    

    // Constructor 2
    XLayer(string _name, string _type); 
    
    // Compute the output dimension of this layer and fill it in top->shape
    void computeOutputDim();

    // Destructor
    ~XLayer();

    // Helper function to return a string representation of XLayer
    // [bottom1, bottom2 ....] --> {Layer Info} --> [top1, top2, ... ]
    string str(bool use_user_names = true);

    // Function to generate output layer name
    string generateOutputFilename(const string& dirname = "");

private:
    string _getLayerStr();
}; 


class XGraph 
{
public:
    // FIELDS
 
    map < string, XLayer* > layers;
    map < string, XBlob*> blobs;
    string name;                       // Just name of network
    nameIndex input_blob;                   // The whole network starts from this blob
    nameIndex output_blob;                  // and ends with this blob

    // XXX : ARK : Get rid of this path  once direct read from caffefile is implemented
    string saveDir;

    // mean data
    vector<int> meanShape;
    vector<float> meanData;
    string meanFile;
    
    // input transformation params
    TransformationParameter transform_params;

    // counters to create some unique names for blobs/layers
    int _blobcounter, _layercounter;
    
    // METHODS

    // Constructor 1
    XGraph();

    // Destructor 
    ~XGraph();

    // TODO : @ARK : Add utility function to get number of specific layers in graph

    // print string representation of XGraph
    // use_true_names : if true, print the original names provided by the user, else program-assigned names
    void print(bool use_true_names = true);
    
    // Check if a blob exists in blobs and return an iterator to it
    // If the exit = true, it forces the execution to stop based on "exit_if" status, else it return an iterator.
    // if exit_if = true = OBJECT FOUND,  it forces the execution to stop if the blob is already present. 
    // If it is false = OBJECT NOT FOUND, it forces the execution to stop if blob is absent.
    // This function is useful to check if bottom is present or top is already defined etc. 
    map<string, XBlob*>::iterator checkIfBlobExists(const string& name, bool exit, bool exit_if);

    // An utility function to generate an unique name for a blob/layer
    // Use these functions only if you are creating a blob/layer and registering to XGraph
    string getUniqueBlobName();
    string getUniqueLayerName();

    // Utility functions to get a list of parent/child layers of another layer
    vector<string> getParentLayers(const string& layerName);
    vector<string> getChildLayers(const string& layerName);

    // Utility functions to delete a layer/blob from graph
    // It only deletes the reference, any new connections to be made should be done by the caller
    // It returns a vector of names of blobs/layers which are no longer required because its parents are deleted.
    vector<string> deleteBlobFromGraph(const string& blobName);
    vector<string> deleteLayerFromGraph(const string& layerName);

    // API function to specify the resize_height & resize_width
    void setResizeShape(int resize_height, int resize_width);
}; 


#endif
