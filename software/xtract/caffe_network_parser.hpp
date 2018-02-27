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


#ifndef __CAFFENETWORKPARSER_HPP__
#define __CAFFENETWORKPARSER_HPP__

// #include <fcntl.h>
// #include <iostream>
// #include <fstream>
#include <string>
#include <vector>
#include <map>
//#include <memory>
//#include <unistd.h>
#include "caffe.pb.h"
//#include <google/protobuf/text_format.h>
//#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "xgraph.hpp"
// #include "caffeLayerMap.hpp"
// #include "xi_funcs.hpp"

#define INPUT_BW 16
#define WEIGHT_BW 8
#define OUTPUT_BW 16
#define BATCHNORM_BW 16
#define SCALE_BW 16

void checkNumberOfTopAndBottom(const caffe::LayerParameter& src, int nB, int nT);
void readDeployFile(const string& deployFile, caffe::NetParameter* Net); 
void readModelFile(const string& modelFile, caffe::NetParameter* Net);
void readMeanBinaryFile(const string& meanFile, caffe::BlobProto* Net);
void saveBlob(const caffe::BlobProto& blob, const string& filepath);
vector<float> extractBlobToVector(const caffe::BlobProto& blob);

vector<int> getBlobDim(const caffe::BlobProto& blob);

// Function to extract values from Caffe Binary files
// Net : Binary Caffemodel files
// layerIndex : (layer Name -> layer Index) Map
void ExtractCaffeMeanData(const caffe::BlobProto* blob, XGraph* graph);
void ExtractTrainedParameters(const caffe::NetParameter* Net, XGraph* graph);
void extractConvolutionTrainedData(XGraph* graph, const string& layerName, const caffe::NetParameter* Net,
                                    const map<string, int>* layerIndex);
void extractDeconvolutionTrainedData(XGraph* graph, const string& layerName, const caffe::NetParameter* Net,
                                    const map<string, int>* layerIndex);
void extractFCTrainedData(XGraph* graph, const string& layerName, const caffe::NetParameter* Net,
                                    const map<string, int>* layerIndex);
void extractL2NormalizeTrainedData(XGraph* graph, const string& layerName, const caffe::NetParameter* Net,
                                    const map<string, int>* layerIndex);
void extractBatchNormTrainedData(XGraph* graph, const string& layerName, const caffe::NetParameter* Net,
                                    const map<string, int>* layerIndex);
void extractScaleTrainedData(XGraph* graph, const string& layerName, const caffe::NetParameter* Net,
                                    const map<string, int>* layerIndex);

// Function to extract values from Caffe Prototxt files
void ExtractParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractConvolutionParameters(const caffe::LayerParameter& src, XGraph& graph); 
void ExtractDeconvolutionParameters(const caffe::LayerParameter& src, XGraph& graph); 
void ExtractDropoutParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractReLUParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractPoolingParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractFCParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractArgmaxParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractSoftmaxParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractConcatParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractCropParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractFlattenParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractPermuteParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractL2NormalizeParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractLRNParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractPriorBoxParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractReshapeParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractNMSParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractEltwiseParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractBatchNormParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractScaleParameters(const caffe::LayerParameter& src, XGraph& graph);
void ExtractPowerParameters(const caffe::LayerParameter& src, XGraph& graph);

// Function to extract precision parameters
void ExtractPrecisionParameters(const caffe::LayerParameter& src, XLayer& layer, bool isParamsPrecisionMandatory = true);

// Miscellaneous functions
vector<float> computePriorBoxes(int layerWidth, int layerHeight, int imageWidth, int imageHeight,
                            const vector<float>& aspect_ratios_, const vector<float>& min_size, const vector<float>& max_size, const vector<float>& var,
                            bool flip = 1, bool clip = 1, float step_h_ = 0.0, float step_w_ = 0.0, float offset_ = 0.5);
void Trim2FixedPoint(vector<float>& data, const int bw, const int fl, RoundingMethod rounding = ROUND_NEAREST );

// IO_format is derived differently for I/O feature maps and weights. So specify IO_format = true for I/O feature maps
void DynamicFPToHWFormat(const int BW, int r_bw, int r_fl, int& hw_Q, int& hw_F, bool IO_format = true);

XGraph* ParseCaffeNetwork(const string& deployFile, const string& caffemodelFile, const string& caffeMeanFile, FileMode file_mode = FILE_TXT, const string& root_folder="");


#endif          // __CAFFENETWORKPARSER_HPP__
