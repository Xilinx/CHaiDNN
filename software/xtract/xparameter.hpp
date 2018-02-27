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


#ifndef __XPARAMETER_HPP__
#define __XPARAMETER_HPP__

#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

#include "xi_funcs.hpp"

using namespace std;

class XBlob
{
public:
    // fields
    string name;                                // User provided name in network file
    string uname;                               // Unique name assigned by the program
    vector< int > shape;                        // Overall Dim of the tensor
    int size;                                   // Total size of this tensor (size = # elems)
    vector< vector<int> > producerDim;          // Dim of data written by producer to this array
    vector< vector<int> > consumerDim;          // Dim of data read by consumer from this array
    vector< string > producers;                 // List of layers writing to this tensor
    vector< string > consumers;                 // List of layers reading from this tensor

    // methods

    // Constructor 1
    XBlob() { }

    // Constructor 2
    XBlob(string _name)
    :name(_name)
    {}
    
    // Constructor 3
    XBlob(string _name, string _uniqname)
    :name(_name), uname(_uniqname)
    {}

    // Destructor 2
    ~XBlob() { }
    
    // Helper function to return a string representation of XBlob
    string str(bool use_user_names=true) 
    { 
        string printName = use_user_names ? name : uname; 

        string tmp = ""; 
        tmp     +=  printName + " : " + TensorDimToString(shape);
        return tmp;
    }

};

class nameIndex
{
public:
    XBlob* blob;
    int id;

    nameIndex()
    : blob(NULL), id(0)
    {}

    nameIndex(XBlob* _blob)
    : blob(_blob), id(0)
    {}

    nameIndex(XBlob* _blob, int _i)
    : blob(_blob), id(_i)
    {}
    
    ~nameIndex() {}; 

    // Return a nameIndex as a string --> "blob:id"
    string str(bool use_user_names = true)
    {
        string printName = use_user_names ? blob->name : blob->uname; 
        string tmp = ""; 
        tmp     +=  printName + ":" + to_string(id);
        return tmp;
    }
};

// This function simply checks the number of tops and bottoms of a particular layer
// A negative number denotes it can have any number of tops/bottoms
// Zero means, it doesn't have top/bottom. 
// TODO : @ARK: How to enable one-or-more blobs? (Eg: Concat layer). Negative number can have 0 blobs also
template<typename bottomType, typename topType>
void checkNumberOfTopAndBottom(string layerType, const bottomType& bottom, const topType& top, int nB, int nT)
{
    if((nB >= 0) && (bottom.size() != nB))
    {
        cerr << "[EG001] " << layerType << " accepts strictly " << nB << " bottom blobs." << " "
             << "Bottom vector contains " << bottom.size() << " bottom blobs." << endl;
        exit(-1);
    }

    if((nT >= 0) && (top.size() != nT))
    {
        cerr << "[EG002] " << layerType << " accepts strictly " << nT << " top blobs." << " "
             << "Top vector contains " << top.size() << " top blobs." << endl;
        exit(-1);
    }
}

enum _PoolingType
{
    MAX=0,
    AVE=1
};

enum _engine
{
    HW = 0,
    SW = 1
};

enum _PriorBoxType
{
    CORNER = 1,
    CENTER_SIZE = 2,
    CORNER_SIZE = 3
};

enum _LRNType
{
    ACROSS_CHANNELS = 0,
    WITHIN_CHANNEL = 1
};

enum _EltOpType
{
    ELT_PROD = 0,
    ELT_SUM = 1,
    ELT_MAX = 2,
};

enum _LayerType
{
    Convolution  ,  
    ReLU         ,  
    Pooling      ,  
    InnerProduct ,  
    Softmax      ,  
    Input        ,  
    LRN          ,
    Dropout      ,
    Concat       ,
    Argmax
};

typedef enum _PoolingType PoolingType;
typedef enum _LayerType LayerType;
typedef enum _engine Engine;
typedef enum _PriorBoxType PriorBoxType;
typedef enum _LRNType LRNType;
typedef enum _EltOpType EltOpType;

            /* ------------------------------ All Layer Parameters ------------------------- */

class ConvolutionParameter
{
public:
    // fields
    int N;                              // Input Feature Maps, it doesn't comes from user, we have to compute from input blob
    int M;                              // Output Feature Maps
    int filter_h, filter_w;           // Filter size - filter_h x filter_w
    int stride_h, stride_w;           // Stride 
    int pad_h, pad_w;                 // Padding
    int dilation;                       // dilation factor
    int reluflag;                       // ReLU enabled/disabled
    int extra_reluflag;
    int group;                        // 
    int has_bias;                     // If this convolution has bias_term or not
    vector<string> weightsPath;           // All weight file names are stored in this.
    vector<string> biasPath;              // All bias file paths are stored in this
    vector< vector<int> > weightsDim;
    vector< vector<int> > biasDim;

    // lrn_params
    int lrn_lsize;
    float lrn_alpha;
    float lrn_beta;
    float lrn_k;
    LRNType lrn_type;

    int has_software_fuse;

	// batchnorm params
    float batchnorm_eps;
    vector<string> batchnorm_meanPath;          
    vector<string> batchnorm_variancePath;              
    vector< vector<int> > batchnorm_meanDim;
    vector< vector<int> > batchnorm_varianceDim;
	
	// scale params
    vector<string> scale_gammaPath;          
    vector<string> scale_betaPath;           
    vector< vector<int> > scale_gammaDim;
    vector< vector<int> > scale_betaDim;
	
	// eltwise params
	
    // Constructor 1
    ConvolutionParameter()
    :reluflag(0), extra_reluflag(0), has_software_fuse(0)
    {}

    // Destructor
    ~ConvolutionParameter() 
    {
    //    cout << "deleting Convolution Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str() 
    {  
        string tmp  =  "";
        tmp         += "Convolution: F = " + filterDimToString() + "; ";
        tmp         += "S = " + to_string(stride_h) + "; ";
        tmp         += "P = " + to_string(pad_h)    + "; ";
        tmp         += "D = " + to_string(dilation) + "; ";
        tmp         += "Group = " + to_string(group) + "; ";
        tmp         += "ReLU = " + to_string(reluflag) + "; ";
        tmp         += "HasBias = " + to_string(has_bias) + "; ";
        tmp         += "Weights = " + stringVectorToString(weightsPath) + "; ";
        tmp         += "Bias = " + stringVectorToString(biasPath) + "; ";
        tmp         += "WDim = " + dimVectorToString(weightsDim) + "; ";
        tmp         += "BDim = " + dimVectorToString(biasDim) + "; ";
        return tmp; 
    }
    
    // compute output dims
    // TODO : @ARK : The constraint on number of bottoms & tops wouldn't work 
    // when consecutive layer fusion comes. So get rid of it in future
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("Convolution", bottom, top, 1, 1);

        // Compute
        vector<int> bottomShape =   bottom.at(0).blob->shape;

        int num_input_channels  =   bottomShape.at(1);
        int input_height        =   bottomShape.at(2);
        int input_width         =   bottomShape.at(3);

        int num_output_channels =   M;
        int filter_extent_h     =   dilation * (filter_h - 1) + 1;
        int filter_extent_w     =   dilation * (filter_w - 1) + 1;
        int output_height       =   (input_height - filter_extent_h + 2 * pad_h) / stride_h + 1;
        int output_width        =   (input_width  - filter_extent_w + 2 * pad_w) / stride_w + 1;

        top.at(0).blob->shape.resize(4);
        top.at(0).blob->shape.at(0) = 1;                top.at(0).blob->shape.at(1) = num_output_channels;
        top.at(0).blob->shape.at(2) = output_height;    top.at(0).blob->shape.at(3) = output_width;

    }

    string filterDimToString()
    {
        return to_string(M) + "x" + to_string(N/group) + "x" + 
                to_string(filter_h) + "x" + to_string(filter_w);
    }

}; 

class DeconvolutionParameter
{
public:
    // fields
    int N;                              // Input Feature Maps, it doesn't come from user, we have to compute from input blob
    int M;                              // Output Feature Maps
    int filter_h, filter_w;           // Filter size - filter_h x filter_w
    int stride_h, stride_w;           // Stride 
    int pad_h, pad_w;                 // Padding
    int dilation;                       // dilation factor
    int reluflag;                       // ReLU enabled/disabled
    int group;
    int has_bias;                     // if it has the bias_term
    vector<string> weightsPath;           // All weight file names are stored in this.
    vector<string> biasPath;              // All bias file paths are stored in this
    vector< vector<int> > weightsDim;
    vector< vector<int> > biasDim;

    // Constructor 1
    DeconvolutionParameter() {}

    // Destructor
    ~DeconvolutionParameter() 
    {
    //    cout << "deleting Deconvolution Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str() 
    {  
        string tmp  =  "";
        tmp         += "Deconvolution: F = " + filterDimToString() + "; ";
        tmp         += "S = " + to_string(stride_h) + "; ";
        tmp         += "P = " + to_string(pad_h)    + "; ";
        tmp         += "D = " + to_string(dilation) + "; ";
        tmp         += "ReLU = " + to_string(reluflag) + "; ";
        tmp         += "HasBias = " + to_string(has_bias) + " ";
        tmp         += "Weights = " + stringVectorToString(weightsPath) + "; ";
        tmp         += "Bias = " + stringVectorToString(biasPath) + "; ";
        tmp         += "WDim = " + dimVectorToString(weightsDim) + "; ";
        tmp         += "BDim = " + dimVectorToString(biasDim) + "; ";
        return tmp; 
    }
    
    // compute output dims
    // TODO : @ARK : The constraint on number of bottoms & tops wouldn't work 
    // when consecutive layer fusion comes. So get rid of it in future
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("Deconvolution", bottom, top, 1, 1);

        // Compute
        vector<int> bottomShape =   bottom.at(0).blob->shape;

        int num_input_channels  =   bottomShape.at(1);
        int input_height        =   bottomShape.at(2);
        int input_width         =   bottomShape.at(3);

        int num_output_channels =   M;
        int filter_extent_h     =   dilation * (filter_h - 1) + 1;
        int filter_extent_w     =   dilation * (filter_w - 1) + 1;
        int output_height       =   stride_h * (input_height - 1) + filter_h - 2 * pad_h;
        int output_width        =   stride_w * (input_width - 1) + filter_w - 2 * pad_w;

        top.at(0).blob->shape.resize(4);
        top.at(0).blob->shape.at(0) = 1;                top.at(0).blob->shape.at(1) = num_output_channels;
        top.at(0).blob->shape.at(2) = output_height;    top.at(0).blob->shape.at(3) = output_width;
    }

    string filterDimToString()
    {
        return to_string(M) + "x" + to_string(N/group) + "x" + 
                to_string(filter_h) + "x" + to_string(filter_w);
    }
};


class ReLUParameter 
{

public:
    // fields
    bool inPlace;

    // Constructor 1
    ReLUParameter() {}

    // Constructor 2
    ReLUParameter(bool _inPlace)
        : inPlace(_inPlace)
        {}
    
    // Destructor
    ~ReLUParameter() 
    {
    //    cout << "deleting ReLU Parameter" << endl;}
    }

    // Helper function to return a string representation of XLayer
    string str()
    {
        string tmp  =  "";
        tmp         += "ReLU: ";
        tmp         += "inPlace = " + to_string(inPlace);
        return tmp; 
    }
        

    // Compute the output blob dimension
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top)
    {
        // Check number of top and bottom
        checkNumberOfTopAndBottom("ReLU", bottom, top, 1, 1);        

        if(inPlace == false)
            // Copy bottom blob dimensions as such
            top.at(0).blob->shape = bottom.at(0).blob->shape;
    }
}; 


class FCLayerParameter
{
public:
    // fields
    int N;                              // Number of input elements
    int M;                              // Number of output elements
    int reluflag;                       // ReLU enabled/disabled
    int has_bias;                   // if it has the bias_term
    vector<string> weightsPath;           // All weight file names are stored in this.
    vector<string> biasPath;              // All bias file paths are stored in this
    vector< vector<int> > weightsDim;
    vector< vector<int> > biasDim;

    // Constructor 1
    FCLayerParameter()
    :reluflag(0)
    {}

    // Destructor
    ~FCLayerParameter() 
    {
    //    cout << "deleting FC Layer Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str()
    {
        string tmp  =  "";
        tmp         += "InnerProduct: ";
        tmp         += "W = " + filterDimToString() + "; ";
        tmp         += "ReLU = " + to_string(reluflag) + "; ";
        tmp         += "HasBias = " + to_string(has_bias) + "; ";
        tmp         += "Weights = " + stringVectorToString(weightsPath) + "; ";
        tmp         += "Bias = " + stringVectorToString(biasPath) + "; ";
        tmp         += "WDim = " + dimVectorToString(weightsDim) + "; ";
        tmp         += "BDim = " + dimVectorToString(biasDim) + "; ";
        return tmp; 
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("InnerProduct", bottom, top, 1, 1);

        // Compute
        vector<int> bottomShape =   bottom.at(0).blob->shape;

        // But whatever comes, FC Layer output is 1x1x1xM
        top.at(0).blob->shape.resize(2);
        top.at(0).blob->shape.at(0) = 1;    top.at(0).blob->shape.at(1) = M;
    }
    // TODO
    // NB : Input to FC layer might be coming from a convolution layer. 
    // Convolution top blob has shape like this : 1 x C x H x W
    // But FC layer expects input blob of shape like this : 1 x 1 x 1 x C*H*W
    // There is no problem here, but while passing to host-code remember this.

    string filterDimToString()
    {
        return to_string(M) + "x" + to_string(N); 
    }
}; 


class SoftmaxParameter
{
public:
    // fields
    Engine device ;
    int axis;
    int nclasses;
    int nboxes;

    // Constructor 1
    SoftmaxParameter() {}

    // Destructor
    ~SoftmaxParameter() 
    {
    //    cout << "deleting Softmax Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str()
    {
        string tmp  =  "";
        tmp         += "Softmax: ";
        tmp         += "axis = " + to_string(axis) + ", ";
        tmp         += "nclasses= " + to_string(nclasses) + ", ";
        tmp         += "nboxes = " + to_string(nboxes) + ", ";
        return tmp; 
    }

    // Compute the output blob dimension
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top)
    {
        // Check number of top and bottom
        checkNumberOfTopAndBottom("Softmax", bottom, top, 1, 1);        

        vector<int> bottomShape = bottom.at(0).blob->shape;
        if(axis >= bottomShape.size())
        {
            cerr << "[EX010] axis < bottomDim for Softmax Layer : " << axis << " v/s " << bottomShape.size() << endl;
            exit(-1);
        }

        // Copy bottom blob dimensions as such
        top.at(0).blob->shape = bottomShape;
    }
}; 


class ArgmaxParameter
{
public:
    // fields
    int top_k;                              // output top k Classes
    int axis;
    int nclasses;
    int nboxes;

    // Constructor 1
    ArgmaxParameter() {}

    // Destructor
    ~ArgmaxParameter() 
    {
    //    cout << "deleting Argmax Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str()
    {
        string tmp  =  "";
        tmp         += "Argmax: ";
        tmp         += "top_k = " + to_string(top_k) + ", ";
        tmp         += "axis = " + to_string(axis) + ", ";
        tmp         += "nclasses= " + to_string(nclasses) + ", ";
        tmp         += "nboxes = " + to_string(nboxes) + ", ";
        return tmp; 
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("Argmax", bottom, top, 1, 1);
        vector<int> topShape = bottom.at(0).blob->shape;
        int tmp_top_k = top_k;
        
        // axis should be less than bottom blob shape dim
        if(axis >= topShape.size())
        {
            cerr << "[EX011] Argmax axis should be smaller than the bottom blob dims : " << axis << " v/s " << topShape.size() << endl;
            exit(-1);
        }

        // top_k should be less than bottom blob shape dim
        if(top_k >= topShape.at(axis))
        {
            cerr << "[EX012] Argmax top_k should be smaller than the number of elements : " << top_k << " v/s " << topShape.size() << endl;
            exit(-1);
        }

        topShape.at(axis) = tmp_top_k;
        top.at(0).blob->shape = topShape;
    }
}; 


class PoolingParameter
{
public:
    // fields
    int kernel_h, kernel_w;
    int stride_h, stride_w;
    int pad_h, pad_w;
    PoolingType PoolType;
    int N;                          // number of I/O planes
    

    // Constructor 1
    PoolingParameter() {}

    // Destructor
    ~PoolingParameter() 
    {
    //    cout << "deleting Pooling Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str() 
    {
        string tmp = "";
        string type = PoolType == 0 ? "MAXPOOL" : "AVGPOOL";
        tmp         += "Pooling : ";
        tmp         += "PoolType = " + type + "; "; 
        tmp         += "N = " + to_string(N) + "; ";
        tmp         += "K = " + to_string(kernel_h) + "; ";
        tmp         += "S = " + to_string(stride_h) + "; ";
        tmp         += "P = " + to_string(pad_h)   ;
        return tmp; 
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("Pooling", bottom, top, 1, 1);

        // Compute
        vector<int> bottomShape =   bottom.at(0).blob->shape;

        int num_input_channels  =   bottomShape.at(1);
        int input_height        =   bottomShape.at(2);
        int input_width         =   bottomShape.at(3);

        int num_output_channels =   num_input_channels;
        int output_height       =   int(ceil((input_height - kernel_h + 2 * pad_h) / float(stride_h))) + 1;
        int output_width        =   int(ceil((input_width  - kernel_w + 2 * pad_w) / float(stride_w))) + 1;

        top.at(0).blob->shape.resize(4);
        top.at(0).blob->shape.at(0) = 1;                top.at(0).blob->shape.at(1) = num_output_channels;
        top.at(0).blob->shape.at(2) = output_height;    top.at(0).blob->shape.at(3) = output_width;
    }
}; 


class DropoutParameter
{
public:
    // fields
    bool inPlace;
    float dropout_ratio;
    
    // Constructor 1
    DropoutParameter() {}

    // Destructor
    ~DropoutParameter() 
    {
    //     cout << "deleting Dropout Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str()
    {
        string tmp = "";
        tmp     +=  "DropOut : ratio = " + to_string(dropout_ratio);
        return tmp;
    }

    // Compute the output blob dimension
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top)
    {
        // Check number of bottom and top blobs
        checkNumberOfTopAndBottom("Dropout", bottom, top, 1, 1);

        if(inPlace == false)
            // Copy bottom blob dimensions as such
            top.at(0).blob->shape = bottom.at(0).blob->shape;
    }

}; 


class InputParameter
{
public:
    // fields
    vector<int> dim;
    
    // Constructor 1
    InputParameter() {}

    // Destructor
    ~InputParameter() 
    {
    //    cout << "deleting Input Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str()
    {
        string tmp = "";
        tmp     +=  "Input Layer: dim = " + TensorDimToString(dim);
        return tmp;
    }

}; 


class LRNParameter
{
public:
    // fields
    int lsize;
    float alpha;
    float beta;
    float k;
    LRNType type;
    
    // Constructor 1
    LRNParameter() {}

    // Destructor
    ~LRNParameter() 
    {
    //    cout << "deleting LRN Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str()
    {
        string tmp = "";
        string tmptype = (type == ACROSS_CHANNELS) ? "ACROSS_CHANNEL" : "WITHIN_CHANNEL" ;
        tmp     +=  "LRN Layer: ";
        tmp     +=  "size = " + to_string(lsize) + " ";
        tmp     +=  "alpha = " + to_string(alpha) + " ";
        tmp     +=  "beta = " + to_string(beta) + " ";
        tmp     +=  "k = " + to_string(k) + " ";
        tmp     +=  "type = " + tmptype + " ";
        return tmp;
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("LRN Layer", bottom, top, 1, 1);

        // Compute
        top.at(0).blob->shape =   bottom.at(0).blob->shape;
    }
}; 


class ConcatParameter
{
public:
    // fields
    int axis;                               // axis along concatenation, default = 1 (channels)
    
    // Constructor 1
    ConcatParameter() {}

    // Destructor
    ~ConcatParameter() 
    {
    //    cout << "deleting Concat Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str()
    {
        string tmp = "";
        tmp     +=  "Concat Layer: ";
        tmp     +=  "Axis = " + to_string(axis) + "; " ;
        return tmp;
    }

    // Compute the output blob dimension
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top)
    {
        // Check number of top and bottom
        checkNumberOfTopAndBottom("Concat", bottom, top, -1, 1);        

        // XXX : @ARK : Concat layer needs atleast one bottom, which is not ensured in above check
        if(bottom.size() == 0)
        {
            cerr << "[EX004] Concat Layer doesn't have a blob in bottom" << endl;
            exit(-1);
        }


        // Make sure all bottoms have same spatical dimension, i.e same height x width
        // Take first bottom as reference
        // int h = bottom.at(0).blob->shape[2];
        // int w = bottom.at(0).blob->shape[3];
        // int c = 0;

        // for(int i=0; i<bottom.size(); i++)
        // {
        //     if((bottom.at(i).blob->shape[2] != h) || (bottom.at(i).blob->shape[3] != w))
        //     {
        //         cerr << "[EX005] Height & Width of bottom blobs of Concat layer doesn't match" << endl;
        //         exit(-1);
        //     }
        //     else
        //     {
        //         c += bottom.at(i).blob->shape[1];             // Accumulate all channels
        //     }
        // }

        vector<int> shape = bottom.at(0).blob->shape;

        // Axis shouldn't be more than the blob size
        if(axis >= shape.size())
        {
            cerr << "[EX013] axis should be smaller than the blob dims : " << axis << " v/s " << shape.size() << endl;
            exit(-1);
        }

        for(int i=1; i<bottom.size(); i++)
        {
            // To concatenate along an axis, remaining dimension(if any) should be same for both blobs
            for(int j=axis+1; j<shape.size(); j++)
            {
                if(bottom.at(i).blob->shape.at(j) != shape.at(j))
                {
                    cerr << "[EX005] Shape Mismatch for bottom blobs of Concat Layer. " 
                         << TensorDimToString(shape) << " v/s " << TensorDimToString(bottom.at(i).blob->shape) << endl;
                    exit(-1);
                }
            }
            shape.at(axis) += bottom.at(i).blob->shape.at(axis);
        }

        top.at(0).blob->shape = shape;
        //top.at(0).blob->shape.resize(4);
        //top.at(0).blob->shape.at(0) = 1;    top.at(0).blob->shape.at(1) = c;
        //top.at(0).blob->shape.at(2) = h;    top.at(0).blob->shape.at(3) = w;
    }
}; 

class CropParameter
{
public:
    // fields
    // TODO : @ARK : Supporting only spatial cropping with same offset on X & Y direction
    int axis;
    int offset;
    int crop_height, crop_width;
    
    // Constructor 1
    CropParameter() {}

    // Destructor
    ~CropParameter() 
    {
    //    cout << "deleting Crop Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str() 
    {
        string tmp = "";
        tmp         += "Crop : ";
        tmp         += "axis = " + to_string(axis) + "; ";
        tmp         += "offset = " + to_string(offset) + "; ";
        tmp         += "crop_HxW = " + to_string(crop_height) + "x" + to_string(crop_width) + "; ";
        return tmp; 
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("Crop", bottom, top, 2, 1);

        // Compute
        // first bottom is to be cropped, second bottom is the reference.
        vector<int> refBottomShape =   bottom.at(1).blob->shape;
        vector<int> testBottomShape =   bottom.at(0).blob->shape;

        // Basically, crop needs a lot of checking.
        // #1. Spatial size of test should be more than the reference by atleast "offset" pixels.
        if((testBottomShape[2] < refBottomShape[2] + offset) || 
            testBottomShape[3] < refBottomShape[3] + offset)
        {
            cerr << "[EX006] Bottom to be cropped is not large enough." << endl;
            exit(-1);
        }

        vector<int> topShape = refBottomShape;
        topShape.at(1) = testBottomShape.at(1);
        top.at(0).blob->shape   =   topShape;
    }
}; 

class FlattenParameter
{
public:
    // fields
    // TODO : @ARK : Flatten in Caffe supports multiple varieties of flatten
    // based on the axis & end_axis provided values. 
    // Since the i/p dimension & o/p dimension will vary depending upon the axis & end_axis
    // we need to support atleast axis (because SSD uses it)
    int axis;
    int end_axis;
    
    // Constructor 1
    FlattenParameter() {}

    // Destructor
    ~FlattenParameter() 
    {
    //    cout << "deleting Flatten Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str() 
    {
        string tmp = "";
        tmp         += "Flatten : ";
        tmp         += "axis = " + to_string(axis) + "; ";
        tmp         += "end_axis = " + to_string(end_axis) + "; ";
        return tmp; 
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("Flatten", bottom, top, 1, 1);

        // Compute
        // first bottom is to be cropped, second bottom is the reference.
        vector<int> bottomShape =   bottom.at(0).blob->shape;
        vector<int> shape;
        if(axis >= bottomShape.size())
        {
            cerr << "[EX014] Flatten Layer: axis < number of axes in bottom blob. "
                 << axis << "v/s" << bottomShape.size() << endl;
            exit(-1);
        }

        // copy the dimension before specified axis as such
        for(int i=0; i<axis; i++)
        {
            shape.push_back(bottomShape.at(i));
        }

        // accumulate the dimension after specified axis
        int prod = 1;
        for(int i=axis; i<bottomShape.size(); i++)
        {
            prod *= bottomShape.at(i);
        }
        shape.push_back(prod); 

        top.at(0).blob->shape   =   shape;
    }
}; 


class PermuteParameter
{
public:
    // fields
    // TODO : @ARK : Permute Layer is introduced in SSD.
    // It basically rearranges the data based on the order of axis given
    // If I am correct, currently we support only permute for order (0, 2, 3, 1) which is in SSD
    // But I think we need a generic Permute kernel, but need to see if any HW specific optimization is provided.
    vector<int> order;
    
    // Constructor 1
    PermuteParameter() {}

    // Destructor
    ~PermuteParameter() 
    {
    //    cout << "deleting Permute Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str() 
    {
        string tmp = "";
        tmp         += "Permute: ";
        tmp         += "order : " + TensorDimToString(order);
        return tmp; 
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("Permute", bottom, top, 1, 1);

        // Compute
        // first bottom is to be cropped, second bottom is the reference.
        vector<int> bottomShape =   bottom.at(0).blob->shape;
        vector<int>topShape(4);
        for(int i=0; i<topShape.size(); i++)
        {
            topShape.at(i) = bottomShape.at(order.at(i));
        }
        top.at(0).blob->shape   =   topShape;
    }
}; 

class L2NormalizeParameter
{
public:
    // fields
    // across_spatial
    // If True, squared sum is taken over all the pixels. (Output = 1 value)
    // If False, squared sum is taken across channels only.(Output = HxW values)
    bool across_spatial;
    // channel_shared - If it is true, gamma will have one value.
    // If it is false, each channel will have its own gamma value (like bias values)
    bool channel_shared;
    // A small addition constant to avoid Zero-SquareRoot
    float eps;
    // Trained scale values
    vector<float> gamma;
    string gammaFile;

    // Constructor 1
    L2NormalizeParameter() {}

    // Destructor
    ~L2NormalizeParameter() 
    {
    //    cout << "deleting Normalize Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str() 
    {
        string tmp = "";
        tmp         += "L2-Normalize: ";
        tmp         += "gamma_size : " + to_string(gamma.size()) + " ";
        tmp         += "gamma file : " + gammaFile;
        return tmp; 
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("L2Normalize", bottom, top, 1, 1);

        // Compute
        top.at(0).blob->shape =   bottom.at(0).blob->shape;
    }
}; 


class PriorBoxParameter
{
public:
    // fields
    // string codeType;                    // = CORNER/CENTER_SIZE/CORNER_SIZE
    vector<float> min_size;             // Min. box size in px
    vector<float> max_size;             // Max. box size in px
    vector<float> aspect_ratio;         // Aspect ratios, default = 1
    bool flip;                          // If true, reciprocal of aspect ratio is also considered
    bool clip;                          // If true, prior box is clipped b/w [0, 1]
    vector<float> variance;             // It will be a 4D vector
    int img_size;                       // Either mention [img_size] or [img_h, img_w]. Not both !!
    int img_h;
    int img_w;

    float step;                         // Either mention [step] or [step_h, step_w]. Not both !!
    float step_h;
    float step_w;

    float offset;                       // Offset to the top left corner of each cell.

    vector<float> pbox;                 // pbox is the precomputed priorBoxes and Variances. Shape [1 x 2 x nboxes*4]
    vector<int> pboxShape;              // pboxShape is same as top.shape, but last dimension gives number of elements in each file

    // Constructor 1
    PriorBoxParameter() {}

    // Destructor
    ~PriorBoxParameter() 
    {
    //    cout << "deleting PriorBox Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str() 
    {
        string tmp = "";
        tmp         += "PriorBox : ";
        tmp         += "ar: " + TensorDimToString(aspect_ratio, ", ") + " ";
        tmp         += "var: " + TensorDimToString(variance, ", ") + " ";
        return tmp; 
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("PriorBox", bottom, top, 2, 1);

        // First bottom is the intermediate feature map, second bottom is the 'data'
        // This way, if 'step' is not mentioned in deploy, they can calculate it by comparing sizes
        // TODO : @ARK : There is lot of confusion within diff version of SSD, especially number of ARs.
        int layer_width = bottom.at(0).blob->shape.at(3);
        int layer_height= bottom.at(0).blob->shape.at(2);

        vector<int> topShape(3);
        topShape.at(0) = 1;
        topShape.at(1) = 2;

        int totalar = 1;
        if(flip)
            totalar += 2*aspect_ratio.size();
        else
            totalar += aspect_ratio.size();

        if(max_size.size() > 0)
        {
            totalar += 1;
        }

        // cerr << "PriorBox : " << totalar << endl;
        topShape.at(2) = totalar * 4 * layer_width * layer_height;
        top.at(0).blob->shape = topShape;
    }
}; 


class ReshapeParameter
{
public:
    // fields
    vector<int> shape;             // Output Shape
    int axis;                      // TODO : @ARK : Currently, only axis = 0 is supported
    int num_axes;                  // TODO : @ARK : Currently, only num_axis = -1 is supported

    // Constructor 1
    ReshapeParameter() {}

    // Destructor
    ~ReshapeParameter() 
    {
    //    cout << "deleting Reshape Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str() 
    {
        string tmp = "";
        tmp         += "Reshape : ";
        tmp         += "shape : " + TensorDimToString(shape, ", ") + " ";
        return tmp; 
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("Reshape", bottom, top, 1, 1);
        
        vector<int> bottomShape = bottom.at(0).blob->shape;
        int totalElems = getSize(bottomShape);
        vector<int> topShape(shape.size());
        
        // Simply copy direct shapes
        int minusOneIndex = -1;
        for(int i=0; i<shape.size(); i++)
        {
            if(shape.at(i) != -1)
            {
                // if shape[i] = 0, simply copy bottom[i] to top[i], else top[i] = shape[i]
                topShape.at(i) = (shape.at(i) == 0) ? bottomShape.at(i) : shape.at(i);
            }
            else
            {
                minusOneIndex = i;
            }
        }
    
        // Special care for -1 case
        if(minusOneIndex != -1)
        {
            if(getSize(bottomShape) % getSizeExcludingAxis(topShape, minusOneIndex) == 0)
            {
                topShape.at(minusOneIndex) = getSize(bottomShape) / getSizeExcludingAxis(topShape, minusOneIndex);
            }
            else
            {
                cerr << "[EX015] Total number of elements should be maintained during Reshape. "
                     << TensorDimToString(bottomShape) << " vs " << TensorDimToString(shape) << endl;
                exit(-1);
            }
        }
        // cerr << "Reshape output shape : " << TensorDimToString(topShape) << endl;
        top.at(0).blob->shape = topShape;
    
    }
}; 


class NMSParameter
{
public:
    // fields
    int num_classes;                                    // Number of classes to be predicted. Required!
    bool share_location;                                // [true] If true, bounding box are shared among different classes, [true]
    int background_label_id;                            // [0] Background label id. If there is no background class,set it as -1. [0]
    float nms_threshold;                                // [0.3] Threshold to be used in nms
    int nms_top_k;                                      // Maximum number of NMS results to be kept
    float nms_eta;                                      // [1.0] Parameter for adaptive nms
    PriorBoxType code_type;                             // [CORNER] Type of coding method for bbox. 
    bool variance_encoded_in_target;                    // [false] If true, variance is encoded in target; otherwise we need to adjust the predicted offset accordingly.
    int keep_top_k;                                     // [-1] Number of top-scoring boxes to keep after NMS. -1 means keep all
    float confidence_threshold;                         // only consider detection with confidence more than this. If not provided, consider all.

    // File details for priorbox & variance
    string pboxFile;
    vector<int> pboxShape;
    string varFile;
    vector<int> varShape;

    // Constructor 1
    NMSParameter() {}

    // Destructor
    ~NMSParameter() 
    {
    //    cout << "deleting NMS Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str() 
    {
        string tmp = "";
        string tmp_code_type  = (code_type == CORNER) ? "CORNER" : 
                                (code_type == CENTER_SIZE) ? "CENTER_SIZE" :
                                (code_type == CORNER_SIZE) ? "CORNER_SIZE" : "Unidentified";

        tmp         += "NMS : ";
        tmp         += "classes: " + to_string(num_classes) + ", ";
        tmp         += "nms_thresh: " + to_string(nms_threshold) + ", ";
        tmp         += "nms_top_k: " + to_string(nms_top_k) + ", ";
        tmp         += "code_type: " + tmp_code_type + ", ";
        tmp         += "keep_top_k: " + to_string(keep_top_k) + ", ";
        tmp         += "conf_thresh: " + to_string(confidence_threshold) + ", ";
        tmp         += "pboxFile: " + pboxFile + ", ";
        tmp         += "pboxShape: " + TensorDimToString(pboxShape) + ", ";
        tmp         += "varFile: " + varFile + ", ";
        tmp         += "varShape: " + TensorDimToString(varShape) + ", ";
        return tmp; 
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("NMS", bottom, top, 3, 1);

        // bottoms in order - [loc, conf, prior_box]
        // We can't really pre-compute the number of output boxes detected. 
        // But we can calculate the worst-case size, which is total number of boxes = loc[1]/4
        int total_boxes = keep_top_k < bottom.at(0).blob->shape.at(1) ? keep_top_k : bottom.at(0).blob->shape.at(1);
        vector<int> topShape(4, 1);
        topShape.at(2) = total_boxes;
        topShape.at(3) = 7;
        top.at(0).blob->shape = topShape;
    }
}; 

class TransformationParameter
{
public:
    // fields
    int resize_height;
    int resize_width;

    // Constructor 1
    TransformationParameter()
    :resize_height(0), resize_width(0)
    {}

    // Destructor
    ~TransformationParameter() 
    {
    //    cout << "deleting Transformation Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str() 
    {
        string tmp = "";
        tmp     += "resize_height = " + to_string(resize_height) + ", ";
        tmp     += "resize_width = " + to_string(resize_width) + ", ";
        return tmp; 
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) {}
}; 

class EltwiseParameter
{
public:
    // fields
    EltOpType type;
    vector<float> coeff;
    bool reluflag;

    // Constructor 1
    EltwiseParameter()
    :reluflag(false)
    {}

    // Destructor
    ~EltwiseParameter() 
    {
    //    cout << "deleting Eltwise Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str() 
    {
        string tmp = "";
        string type_str;
        switch (type)
        {
            case ELT_PROD:
                type_str = "ELT_PROD";
                break;
            case ELT_SUM:
                type_str = "ELT_SUM";
                break;
            case ELT_MAX:
                type_str = "ELT_MAX";
                break;
            //case MIN:
            //    type_str = "MIN";
            //    break;
            default:
                cerr << "[EX061] Unsupported operation type for Eltwise" << endl;
                exit(0);
        }

        tmp     +=  "Eltwise Layer: ";
        tmp     += "Type of Operation = " + type_str ;
        return tmp; 
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top)
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("Eltwise", bottom, top, 2, 1);

        top.at(0).blob->shape = bottom.at(0).blob->shape;
    }
}; 

class BatchNormParameter
{
public:
    // fields
    bool global_stats;
    float eps;
    bool inPlace;
    bool reluflag;
    vector<string> meanPath;           // All weight file names are stored in this.
    vector<string> variancePath;              // All bias file paths are stored in this
    vector< vector<int> > meanDim;
    vector< vector<int> > varianceDim;
    
    // Constructor 1
    BatchNormParameter() {}

    // Destructor
    ~BatchNormParameter() 
    {
    //    cout << "deleting BatchNorm Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str()
    {
        string tmp = "";
        tmp     +=  "BatchNorm Layer: ";
        tmp     +=  "global_stats: " + to_string(global_stats) + ", ";
        tmp     +=  "eps: " + to_string(eps) + ", ";
        tmp     +=  "inPlace: " + to_string(inPlace);

        return tmp;
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("BatchNorm Layer", bottom, top, 1, 1);

        // Compute
        if(!inPlace)
            top.at(0).blob->shape =   bottom.at(0).blob->shape;
    }
}; 

class ScaleParameter
{
public:
    // fields
    int axis;
    int num_axes;
    bool inPlace;
    bool reluflag;
    int has_bias;                   // if it has the bias_term
    vector<string> gammaPath;           // All weight file names are stored in this.
    vector<string> betaPath;              // All bias file paths are stored in this
    vector< vector<int> > gammaDim;
    vector< vector<int> > betaDim;
    
    // Constructor 1
    ScaleParameter() {}

    // Destructor
    ~ScaleParameter() 
    {
    //    cout << "deleting Scale Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str()
    {
        string tmp = "";
        tmp     +=  "Scale Layer: ";
        tmp     +=  "axis: " + to_string(axis) + ", ";
        tmp     +=  "num_axes: " + to_string(num_axes) + ", ";
        tmp     +=  "inPlace: " + to_string(inPlace);
        return tmp;
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("Scale Layer", bottom, top, 1, 1);

        // Compute
        if(!inPlace)
            top.at(0).blob->shape =   bottom.at(0).blob->shape;
    }
}; 

class PowerParameter
{
public:
    // fields
    float power;
    float scale;
    float shift;
    bool inPlace;
    
    // Constructor 1
    PowerParameter() {}

    // Destructor
    ~PowerParameter() 
    {
    //    cout << "deleting Power Parameter" << endl;
    }

    // Helper function to return a string representation of XLayer
    string str()
    {
        string tmp = "";
        tmp     +=  "Power Layer: ";
        tmp     +=  "power: " + to_string(power) + ", ";
        tmp     +=  "scale: " + to_string(scale) + ", ";
        tmp     +=  "shift: " + to_string(shift);
        return tmp;
    }

    // compute output dims
    void computeOutputDim(vector<nameIndex>& bottom, vector<nameIndex>& top) 
    {
        // Check if number of bottom and top blobs are correct.
        checkNumberOfTopAndBottom("Power Layer", bottom, top, 1, 1);

        // Compute
        top.at(0).blob->shape =   bottom.at(0).blob->shape;
    }
}; 

#endif     // __XPARAMETER_HPP__
