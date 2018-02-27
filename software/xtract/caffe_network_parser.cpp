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


#include "caffe_network_parser.hpp"
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <unistd.h>
#include "caffe.pb.h"
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "xgraph.hpp"
#include "caffe_layer_map.hpp"
#include "xi_funcs.hpp"


//-------------------------------------------------------------------------------------------------------------//
//------------------------------------- FUNCION DEFINTIONS ----------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------//

XGraph* ParseCaffeNetwork(const string& deployFile, const string& caffemodelFile, const string& caffeMeanFile, FileMode file_mode, const string& rootFolder)
{

    // ------------------------  GET CAFFE LAYER MAP ---------------------------- //
    fillCaffeLayerMap();

    // ------------------------  LOAD DEPLOY FILE ------------------------------- //

    // Create the Net for prototxt and load the deploy file to it
    caffe::NetParameter *Net = new caffe::NetParameter;
    readDeployFile(deployFile, Net);

    // ------------------------ START EXTRACTING TO XGRAPH ------------------------- //

    // Create a XGraph
    XGraph* graph  = new XGraph();  
    string uniqname, username;

    // XXX : ARK : remove this section once caffe read is ready
    string tmp_saveDir = rootFolder + "/data/";
    graph->saveDir = tmp_saveDir;

    // Handle the input layer separately as it can be initialized multiple ways
    // TODO : @ARK : Move this input parsing to a separate function for cleaner look
    graph->name = Net->has_name() ? Net->name(): ""; 
    int layer_start_index = 0;

    if(Net->layer(0).type() != "Input")                                      // If first layer is not Input Data Layer
    {                                                                       // that means no input layer defined.
        if(Net->input_size() != 1)
        {
            cerr << "[EP003] Current version supports one & only-one top blob" << endl;
            exit(-1);
        }
            
        if((Net->input_dim_size() > 0) && (Net->input_shape_size() > 0))
        {
            cerr << "[EP026] Both input_dim and input_shape can't be specified together. Use only one of them" << endl;
            exit(-1);
        }

        vector<int> tmpShape;
        if((Net->input_dim_size() > 0) && 
            ((Net->input_dim_size() != 4) || (Net->input_dim(0) != 1)))
        {
            cerr << "[EP004] Current version requires batch size = 1 & input dimension to be in the form (1 x Channels x Height x Width)" << endl;
            exit(-1);
        }
        else
        {
            tmpShape.resize(Net->input_dim_size());
            copy(Net->input_dim().begin(), Net->input_dim().end(), tmpShape.begin());
        } 

        if((Net->input_shape_size() > 0))
        {
            caffe::BlobShape shape = Net->input_shape(0);
            if((shape.dim_size() != 4) || (shape.dim(0) != 1))
            {
                cerr << "[EP027] Current version requires batch size = 1 & input shape to be in the form (1 x Channels x Height x Width)" << endl;
                exit(-1);
            }
            tmpShape.resize(shape.dim_size());
            copy(shape.dim().begin(), shape.dim().end(), tmpShape.begin());
        }

        username = Net->input(0);
        uniqname = graph->getUniqueBlobName();
        XBlob* tmp = new XBlob(username, uniqname);
        tmp->shape = tmpShape;
        graph->blobs[username] = tmp;
        graph->input_blob = nameIndex(tmp);
    }
    else
    {
        caffe::LayerParameter iLayer = Net->layer(0);
        if(iLayer.top_size() != 1)
        {
            cerr << "[EP006] <Parser> Current version supports one & only-one top blob" << endl;
            exit(-1);
        }
            
        caffe::InputParameter input_param = iLayer.input_param();
        caffe::BlobShape blobshape = input_param.shape(0);
        
        if(blobshape.dim_size() != 4)
        {
            cerr << "[EP007] <Parser> Current version requires input dimension to be in the form (1 x Channels x Height x Width)" << endl;
            exit(-1);
        }
        
        if(blobshape.dim(0) != 1)
        {
            cerr << "[EP008] <Parser> Current version supports batch size = 1. So input dimension should be (1 x Channels x Height x Width)" << endl;
            exit(-1);
        }

        username = iLayer.top(0);
        uniqname = graph->getUniqueBlobName();
        XBlob* tmp = new XBlob(username, uniqname);
        for(int i=0; i<blobshape.dim_size(); i++)
        {
            tmp->shape.push_back(blobshape.dim(i));
        }
        graph->blobs[username] = tmp;
        graph->input_blob = nameIndex(tmp);

        // We already finished with first layer. So increment the counter
        layer_start_index++; 
    }
        

    for(int i=layer_start_index; i<Net->layer_size(); i++)
    {
        caffe::LayerParameter cLayer = Net->layer(i);
        
        cout << "[PARSE] Parsing " << cLayer.name() << endl;

        // #. Check if the Caffe Layer is supported or not
        mapStrStr::const_iterator it = CaffeLayerMap.find(cLayer.type());
        if(it == CaffeLayerMap.end())
        {
            cerr << "[EP009] Unregistered Layer Type : " << cLayer.type() << endl;
            exit(-1);
        }

        // #. Check if any other layer with same name already present in the XGraph
        if(graph->layers.find(cLayer.name()) != graph->layers.end())
        {
            cerr << "[EP010] Duplicate layer name detected : " << cLayer.name() << endl;
            exit(-1);
        }
    
        // #. If it is supported, parse all the params from Caffe Layer to XLayer in XGraph
        ExtractParameters(cLayer, *graph);
    } 
    
    // -----------------------  LOAD MEAN FILE -------------------------------------- //

    // Check if mean file path is provided
    if(caffeMeanFile.empty())
    {
        cerr << "[EP028] Mean file is mandatory. If the mean values are zeros, "
             << "please keep zeros in a txt file (equal to number of planes in image) and provide the path" << endl;
        exit(-1);
    }

    if(file_mode == FILE_BIN)
    {
        // Create the Blob for Mean Data
        caffe::BlobProto *blob = new caffe::BlobProto();
        readMeanBinaryFile(caffeMeanFile, blob);

        // Extract all the trained parameters and fill the fields in xgraph
        ExtractCaffeMeanData(blob, graph);
        delete blob;
    }
    else
    {
        vector<float> mean = readtxt(caffeMeanFile);
        if(mean.size() == 0)
        {
            cerr << "[EP029] Atleast 1 mean value should be provided (for grayscale image). Parser couldn't read any data from the file. "
                 << "Make sure you provided correct TXT file (not binaryproto file) and file_mode = FILE_TXT in the Parser function call. " << endl;
            exit(-1);
        }
        graph->meanData = mean;
        graph->meanShape.resize(4, 1);
        graph->meanShape.at(3) = graph->meanData.size();
    }

    graph->meanFile = graph->saveDir + "L_mean";

#if GENERATE_CONSTDATA
    SAVEDATA(graph->meanData, graph->saveDir + "L_mean");
#endif

    // ------------------------  LOAD CAFFEMODEL FILE ------------------------------- //

    // Create the Net for caffemodel file 
    caffe::NetParameter *binNet = new caffe::NetParameter();
    readModelFile(caffemodelFile, binNet);

    // Extract all the trained parameters and fill the fields in xgraph
    ExtractTrainedParameters(binNet, graph);


    // -----------------------  CLEAR ALL MEMORY -------------------------------------- //
    google::protobuf::ShutdownProtobufLibrary();
    delete Net;
    delete binNet;

    return graph;
}

// Function that extract all the trained parameters from CaffeModel file
void ExtractTrainedParameters(const caffe::NetParameter* Net, XGraph* graph)
{
    // Create a map of layer_names -> index in caffemodel file for faster lookup
    map<string, int>* modelLayerIndex = new map<string, int>();
    for(int i=0; i<Net->layer_size(); i++)
    {
        caffe::LayerParameter cLayer = Net->layer(i);
        (*modelLayerIndex)[cLayer.name()] = i;
    }

    // Now iterate through each layer searching for layers with params
    for(map<string, XLayer*>::iterator xlayer_it = graph->layers.begin(); xlayer_it != graph->layers.end(); xlayer_it++)
    {
        if(xlayer_it->second->type == "Convolution")
        {
            extractConvolutionTrainedData(graph, xlayer_it->second->name, Net, modelLayerIndex);
        }
        else if(xlayer_it->second->type == "Deconvolution")
        {
            extractDeconvolutionTrainedData(graph, xlayer_it->second->name, Net, modelLayerIndex);
        }
        else if(xlayer_it->second->type == "InnerProduct")
        {
            extractFCTrainedData(graph, xlayer_it->second->name, Net, modelLayerIndex);
        }
        else if(xlayer_it->second->type == "L2Normalize")
        {
            extractL2NormalizeTrainedData(graph, xlayer_it->second->name, Net, modelLayerIndex);
        }
        else if(xlayer_it->second->type == "BatchNorm")
        {
            extractBatchNormTrainedData(graph, xlayer_it->second->name, Net, modelLayerIndex);
        }
        else if(xlayer_it->second->type == "Scale")
        {
            extractScaleTrainedData(graph, xlayer_it->second->name, Net, modelLayerIndex);
        }
        
    }

    // Free the map memory
    delete modelLayerIndex;
}

void extractConvolutionTrainedData(XGraph* graph, const string& layerName, const caffe::NetParameter* Net,
                                    const map<string, int>* layerIndex)
{
    map<string, int>::const_iterator modelLayer_it = layerIndex->find(layerName);
    if(modelLayer_it == layerIndex->end())
    {
        cerr << "[EP030] Layer " << layerName << " is not found in the caffemodel file. " << endl;
        exit(-1);
    }
    
    // Get the layer from caffemodel
    int loc = modelLayer_it->second;
    caffe::LayerParameter binLayer  = Net->layer(loc);
    XLayer* tmpXlayer               = graph->layers[layerName];
    string txtFileName;

    // Extract the weights, do trimming & rounding before saving to TXT file
    vector<int> weightsShape = getBlobDim(binLayer.blobs(0));
    ELOG(  ((weightsShape.at(0) != tmpXlayer->conv_params->M) || 
            (weightsShape.at(1) != (tmpXlayer->conv_params->N/tmpXlayer->conv_params->group)) ||
            (weightsShape.at(2) != tmpXlayer->conv_params->filter_h) ||
            (weightsShape.at(3) != tmpXlayer->conv_params->filter_w)),
            EP056,
            "Convolution Layer: " << tmpXlayer->name << " - mismatch in filter shape. " 
            << TensorDimToString(weightsShape) << " v/s " << tmpXlayer->conv_params->filterDimToString()
        )

    string tmpName(tmpXlayer->name); 
    replace(tmpName.begin(), tmpName.end(), '/', '_');
    txtFileName = graph->saveDir + tmpName + "_weights";
    tmpXlayer->conv_params->weightsPath.push_back(txtFileName);

#if GENERATE_CONSTDATA
    int sizeInBytes = getSize(weightsShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << "\t";
	if(sizeInBytes > (14*1024*1024))
	{
		cerr << "Parsing large data, this may take a while ...";
	}
	cerr << endl;
    vector<float> weights = extractBlobToVector(binLayer.blobs(0));
    //Trim2FixedPoint(weights, tmpXlayer->user_wt_bw, tmpXlayer->user_wt_fl, ROUND_NEAREST);
    SAVEDATA(weights, txtFileName);

    tmpXlayer->conv_params->weightsDim.push_back(getBlobDim(binLayer.blobs(0)));
#endif
    
    // Extract the bias if bias is present, else save a vector filled with zeros
    txtFileName = graph->saveDir + tmpName + "_bias";
    tmpXlayer->conv_params->biasPath.push_back(txtFileName);

#if GENERATE_CONSTDATA
    if(tmpXlayer->conv_params->has_bias)
    {
        vector<int> biasShape = getBlobDim(binLayer.blobs(1));
        ELOG( (biasShape.at(0) != tmpXlayer->conv_params->M) , EP031,
              "Deconv Layer: " << tmpXlayer->name << " - mismatch in bias shape. "
               << TensorDimToString(biasShape) << " v/s " << tmpXlayer->conv_params->M )

        int sizeInBytes = getSize(biasShape) * sizeof(float);
        cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
        vector<float> bias = extractBlobToVector(binLayer.blobs(1));
        //Trim2FixedPoint(bias, tmpXlayer->user_wt_bw, tmpXlayer->user_wt_fl, ROUND_NEAREST);
        SAVEDATA(bias, txtFileName);
        tmpXlayer->conv_params->biasDim.push_back(biasShape);
    }
    else
    {
        int OFM = tmpXlayer->conv_params->M;
        vector<float> bias(OFM, 0.0);
        vector<int> dim; dim.push_back(OFM);
        int sizeInBytes = getSize(dim) * sizeof(float);
        cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
        SAVEDATA(bias, txtFileName);
        tmpXlayer->conv_params->biasDim.push_back(dim);
    }
#endif
}

void extractDeconvolutionTrainedData(XGraph* graph, const string& layerName, const caffe::NetParameter* Net,
                                    const map<string, int>* layerIndex)
{
    map<string, int>::const_iterator modelLayer_it = layerIndex->find(layerName);
    if(modelLayer_it == layerIndex->end())
    {
        cerr << "[EP032] Layer " << layerName << " is not found in the caffemodel file. " << endl;
        exit(-1);
    }
    
    // Get the layer from caffemodel
    int loc = modelLayer_it->second;
    caffe::LayerParameter binLayer  = Net->layer(loc);
    XLayer* tmpXlayer               = graph->layers[layerName];
    string txtFileName;

    // Extract the weights
    vector<int> weightsShape = getBlobDim(binLayer.blobs(0));
    ELOG(  ((weightsShape.at(0) != tmpXlayer->deconv_params->M) || 
            (weightsShape.at(1) != (tmpXlayer->deconv_params->N/tmpXlayer->deconv_params->group)) ||
            (weightsShape.at(2) != tmpXlayer->deconv_params->filter_h) ||
            (weightsShape.at(3) != tmpXlayer->deconv_params->filter_w)),
            EP057,
            "Deconvolution Layer: " << tmpXlayer->name << " - mismatch in filter shape. " 
            << TensorDimToString(weightsShape) << " v/s " << tmpXlayer->deconv_params->filterDimToString()
        )

    string tmpName(tmpXlayer->name); 
    replace(tmpName.begin(), tmpName.end(), '/', '_');
    txtFileName = graph->saveDir + tmpName + "_weights";
    tmpXlayer->deconv_params->weightsPath.push_back(txtFileName);

#if GENERATE_CONSTDATA
    int sizeInBytes = getSize(weightsShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << "\t";
	if(sizeInBytes > (14*1024*1024))
	{
		cerr << "Parsing large data, this may take a while ...";
	}
	cerr << endl;

    saveBlob(binLayer.blobs(0), txtFileName);
    tmpXlayer->deconv_params->weightsDim.push_back(weightsShape);
#endif
    
    // Extract the bias if bias is present, else save a vector filled with zeros
    txtFileName = graph->saveDir + tmpName + "_bias";
    tmpXlayer->deconv_params->biasPath.push_back(txtFileName);

#if GENERATE_CONSTDATA
    if(tmpXlayer->deconv_params->has_bias)
    {
        vector<int> biasShape = getBlobDim(binLayer.blobs(1));
        int sizeInBytes = getSize(biasShape) * sizeof(float);
        ELOG( (biasShape.at(0) != tmpXlayer->deconv_params->M) , EP034,
              "Deconv Layer: " << tmpXlayer->name << " - mismatch in bias shape. "
               << TensorDimToString(biasShape) << " v/s " << tmpXlayer->deconv_params->M )
    
        cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
        saveBlob(binLayer.blobs(1), txtFileName);
        tmpXlayer->deconv_params->biasDim.push_back(biasShape);
    }
    else
    {
        int OFM = tmpXlayer->deconv_params->M;
        vector<float> bias(OFM, 0.0);
        vector<int> dim; dim.push_back(OFM);
        int sizeInBytes = getSize(dim) * sizeof(float);
        cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
        SAVEDATA(bias, txtFileName);
        tmpXlayer->deconv_params->biasDim.push_back(dim);
    }
#endif
}
void extractFCTrainedData(XGraph* graph, const string& layerName, const caffe::NetParameter* Net,
                                    const map<string, int>* layerIndex)
{
    map<string, int>::const_iterator modelLayer_it = layerIndex->find(layerName);
    if(modelLayer_it == layerIndex->end())
    {
        cerr << "[EP035] Layer " << layerName << " is not found in the caffemodel file. " << endl;
        exit(-1);
    }
    
    // Get the layer from caffemodel
    int loc = modelLayer_it->second;
    caffe::LayerParameter binLayer  = Net->layer(loc);
    XLayer* tmpXlayer               = graph->layers[layerName];
    string txtFileName;

    // Extract the weights

    // Check if filter shape is matching
    vector<int> weightsShape = getBlobDim(binLayer.blobs(0));
    ELOG( (weightsShape.at(0) != tmpXlayer->fc_params->M) || (weightsShape.at(1) != tmpXlayer->fc_params->N), EP035,
          "FC Layer: " << tmpXlayer->name << " - mismatch in filter shape. " 
           << TensorDimToString(weightsShape) << " v/s " << tmpXlayer->fc_params->M << "x" << tmpXlayer->fc_params->N);

    string tmpName(tmpXlayer->name); 
    replace(tmpName.begin(), tmpName.end(), '/', '_');
    txtFileName = graph->saveDir + tmpName + "_weights";
    tmpXlayer->fc_params->weightsPath.push_back(txtFileName);

#if GENERATE_CONSTDATA
    int sizeInBytes = getSize(weightsShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << "\t";
	if(sizeInBytes > (14*1024*1024))
	{
		cerr << "Parsing large data, this may take a while ...";
	}
	cerr << endl;

    vector<float> weights = extractBlobToVector(binLayer.blobs(0));
    //Trim2FixedPoint(weights, tmpXlayer->user_wt_bw, tmpXlayer->user_wt_fl, ROUND_NEAREST);
    SAVEDATA(weights, txtFileName);
    tmpXlayer->fc_params->weightsDim.push_back(weightsShape);
#endif
    
    // Extract the bias if bias is present, else save a vector filled with zeros
    txtFileName = graph->saveDir + tmpName + "_bias";
    tmpXlayer->fc_params->biasPath.push_back(txtFileName);

#if GENERATE_CONSTDATA
    if(tmpXlayer->fc_params->has_bias)
    {
        // Check if bias shape is matching
        vector<int> biasShape = getBlobDim(binLayer.blobs(1));
        int sizeInBytes = getSize(biasShape) * sizeof(float);
        ELOG( (biasShape.at(0) != tmpXlayer->fc_params->M) , EP036,
              "FC Layer: " << tmpXlayer->name << " - mismatch in bias shape. "
               << TensorDimToString(biasShape) << " v/s " << tmpXlayer->fc_params->M );

        cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
        vector<float> bias = extractBlobToVector(binLayer.blobs(1));
        //Trim2FixedPoint(bias, tmpXlayer->user_wt_bw, tmpXlayer->user_wt_fl, ROUND_NEAREST);
        SAVEDATA(bias, txtFileName);
        tmpXlayer->fc_params->biasDim.push_back(biasShape);
    }
    else
    {
        int OFM = tmpXlayer->fc_params->M;
        vector<float> bias(OFM, 0.0);
        vector<int> dim; dim.push_back(OFM);
        int sizeInBytes = getSize(dim) * sizeof(float);
        cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
        SAVEDATA(bias, txtFileName);
        tmpXlayer->fc_params->biasDim.push_back(dim);
    }
#endif
}

void extractL2NormalizeTrainedData(XGraph* graph, const string& layerName, const caffe::NetParameter* Net,
                                    const map<string, int>* layerIndex)
{
    map<string, int>::const_iterator modelLayer_it = layerIndex->find(layerName);
    if(modelLayer_it == layerIndex->end())
    {
        cerr << "[EP037] Layer " << layerName << " is not found in the caffemodel file. " << endl;
        exit(-1);
    }
    
    // Get the layer from caffemodel
    int loc = modelLayer_it->second;
    caffe::LayerParameter binLayer  = Net->layer(loc);
    XLayer* tmpXlayer               = graph->layers[layerName];
    string txtFileName;

    // Extract the weights
    string tmpName(tmpXlayer->name); 
    replace(tmpName.begin(), tmpName.end(), '/', '_');
    txtFileName = graph->saveDir + tmpName + "_weights";
    tmpXlayer->l2norm_params->gammaFile = txtFileName;

#if GENERATE_CONSTDATA
    int channels = tmpXlayer->topShape.at(0).at(1);               // Number of feature maps
    caffe::BlobProto blob = binLayer.blobs(0);
    
    // If it is just one value for all channels, replicate it #channels times
    if(tmpXlayer->l2norm_params->channel_shared)
    {
        float val = blob.data(0);
        tmpXlayer->l2norm_params->gamma.resize(channels, val);  // Replicate same value #channels times
    }
    else
    {
        // Check if bias shape is matching
        vector<int> gammaShape = getBlobDim(binLayer.blobs(0));
        ELOG( (gammaShape.at(0) != channels) , EP038,
              "L2 Normalization Layer: " << tmpXlayer->name << " - mismatch in gamma shape. "
               << TensorDimToString(gammaShape) << " v/s " << channels );

        tmpXlayer->l2norm_params->gamma.reserve(channels);
        std::copy(blob.data().begin(), blob.data().end(), std::back_inserter(tmpXlayer->l2norm_params->gamma));
    }
    int sizeInBytes = tmpXlayer->l2norm_params->gamma.size() * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
    SAVEDATA(tmpXlayer->l2norm_params->gamma, txtFileName);
#endif

}

void extractBatchNormTrainedData(XGraph* graph, const string& layerName, const caffe::NetParameter* Net,
                                    const map<string, int>* layerIndex)
{
    map<string, int>::const_iterator modelLayer_it = layerIndex->find(layerName);
    if(modelLayer_it == layerIndex->end())
    {
        cerr << "[EP030] Layer " << layerName << " is not found in the caffemodel file. " << endl;
        exit(-1);
    }
    
    // Get the layer from caffemodel
    int loc = modelLayer_it->second;
    caffe::LayerParameter binLayer  = Net->layer(loc);
    XLayer* tmpXlayer               = graph->layers[layerName];
    string txtFileName;
#if 1  //TODO:ANITHA
    // Extract MWA
    vector<float> mwa = extractBlobToVector(binLayer.blobs(2));
    float scaling_factor = mwa[0] == 0 ? 0 : 1.0f/mwa[0];
    // cout << "Scaling Factor: " << scaling_factor << endl;
#endif

    // Extract the mean, do trimming & rounding before saving to TXT file
    vector<int> weightsShape = getBlobDim(binLayer.blobs(0));
    int channels = tmpXlayer->topShape.at(0).at(1);               // Number of feature maps
    ASSERT( (weightsShape.at(0) == channels), EP056,
            "BatchNorm Layer: " << tmpXlayer->name << "mismatch in Mean shape : " 
            << TensorDimToString(weightsShape) << " != " << channels)

    string tmpName(tmpXlayer->name); 
    replace(tmpName.begin(), tmpName.end(), '/', '_');
    txtFileName = graph->saveDir + tmpName + "_mean";
    tmpXlayer->batchnorm_params->meanPath.push_back(txtFileName);

#if GENERATE_CONSTDATA
    int sizeInBytes = getSize(weightsShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << "\t";
	if(sizeInBytes > (14*1024*1024))
	{
		cerr << "Parsing large data, this may take a while ...";
	}
	cerr << endl;
    vector<float> weights = extractBlobToVector(binLayer.blobs(0));
#if 1 //TODO:ANITHA
    for(int i=0; i<weights.size(); ++i)
        weights[i] *= scaling_factor;
#endif
    //Trim2FixedPoint(weights, BATCHNORM_BW, tmpXlayer->user_bn_mean_fl, ROUND_NEAREST);
    SAVEDATA(weights, txtFileName);
    tmpXlayer->batchnorm_params->meanDim.push_back(getBlobDim(binLayer.blobs(0)));
#endif
    
    // Extract the variance, do trimming & rounding before saving to TXT file
    vector<int> biasShape = getBlobDim(binLayer.blobs(1));
    ASSERT( (biasShape.at(0) == channels), EP056,
            "BatchNorm Layer: " << tmpXlayer->name << "mismatch in variance shape : " 
            << TensorDimToString(biasShape) << " != " << channels)

    txtFileName = graph->saveDir + tmpName + "_variance";
    tmpXlayer->batchnorm_params->variancePath.push_back(txtFileName);

#if GENERATE_CONSTDATA
    sizeInBytes = getSize(biasShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << "\t";
	if(sizeInBytes > (14*1024*1024))
	{
		cerr << "Parsing large data, this may take a while ...";
	}
	cerr << endl;
    vector<float> bias = extractBlobToVector(binLayer.blobs(1));
    //Trim2FixedPoint(bias, BATCHNORM_BW, tmpXlayer->user_bn_variance_fl, ROUND_NEAREST);
    SAVEDATA(bias, txtFileName);
    tmpXlayer->batchnorm_params->varianceDim.push_back(biasShape);
#endif
}

void extractScaleTrainedData(XGraph* graph, const string& layerName, const caffe::NetParameter* Net,
                                    const map<string, int>* layerIndex)
{
    map<string, int>::const_iterator modelLayer_it = layerIndex->find(layerName);
    if(modelLayer_it == layerIndex->end())
    {
        cerr << "[EP035] Layer " << layerName << " is not found in the caffemodel file. " << endl;
        exit(-1);
    }
    
    // Get the layer from caffemodel
    int loc = modelLayer_it->second;
    caffe::LayerParameter binLayer  = Net->layer(loc);
    XLayer* tmpXlayer               = graph->layers[layerName];
    string txtFileName;

    // Extract gamma 

    // Check if filter shape is matching
    vector<int> weightsShape = getBlobDim(binLayer.blobs(0));
    int channels = tmpXlayer->topShape.at(0).at(1);               // Number of feature maps
    ASSERT( (weightsShape.at(0) == channels), EP057,
            "Scale Layer: " << tmpXlayer->name << "mismatch in Scale shape : " 
            << TensorDimToString(weightsShape) << " != " << channels)

    string tmpName(tmpXlayer->name); 
    replace(tmpName.begin(), tmpName.end(), '/', '_');
    txtFileName = graph->saveDir + tmpName + "_gamma";
    tmpXlayer->scale_params->gammaPath.push_back(txtFileName);

#if GENERATE_CONSTDATA
    int sizeInBytes = getSize(weightsShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << "\t";
	if(sizeInBytes > (14*1024*1024))
	{
		cerr << "Parsing large data, this may take a while ...";
	}
	cerr << endl;

    vector<float> weights = extractBlobToVector(binLayer.blobs(0));
    //Trim2FixedPoint(weights, SCALE_BW, tmpXlayer->user_scale_gamma_fl, ROUND_NEAREST);
    SAVEDATA(weights, txtFileName);
    tmpXlayer->scale_params->gammaDim.push_back(weightsShape);
#endif
    
    // Extract the beta if beta is present, else save a vector filled with zeros
    txtFileName = graph->saveDir + tmpName + "_beta";
    tmpXlayer->scale_params->betaPath.push_back(txtFileName);

#if GENERATE_CONSTDATA
    if(tmpXlayer->scale_params->has_bias)
    {
        // Check if bias shape is matching
        vector<int> biasShape = getBlobDim(binLayer.blobs(1));
        int sizeInBytes = getSize(biasShape) * sizeof(float);
        ASSERT( (biasShape.at(0) == channels), EP060,
                "Scale Layer: " << tmpXlayer->name << "mismatch in Bias shape : " 
                << TensorDimToString(weightsShape) << " != " << channels)

        cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
        vector<float> bias = extractBlobToVector(binLayer.blobs(1));
        //Trim2FixedPoint(bias, SCALE_BW, tmpXlayer->user_scale_beta_fl, ROUND_NEAREST);
        SAVEDATA(bias, txtFileName);
        tmpXlayer->scale_params->betaDim.push_back(biasShape);
    }
    else
    {
        int OFM = channels;
        vector<float> bias(OFM, 0.0);
        vector<int> dim; dim.push_back(OFM);
        int sizeInBytes = getSize(dim) * sizeof(float);
        cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
        SAVEDATA(bias, txtFileName);
        tmpXlayer->scale_params->betaDim.push_back(dim);
    }
#endif
}


// A function to return the dimension of a caffe blob
vector<int> getBlobDim(const caffe::BlobProto& blob)
{
    vector<int> dim;                                                                        // To store the dimension
    caffe::BlobShape blobshape = blob.shape();
    dim.reserve(blobshape.dim_size());
    copy(blobshape.dim().begin(), blobshape.dim().end(), std::back_inserter(dim));
    return dim;
}

// A function to dump the the caffe blob to a txt file
void saveBlob(const caffe::BlobProto& blob, const string& filepath)
{
    vector<float> weights;                                                                      // To store the weights/bias
    weights.reserve(blob.data_size());

    // copy the dimension of the array and the data itself.
    copy(blob.data().begin(), blob.data().end(), std::back_inserter(weights));    

    // Setup the filename
    SAVEDATA(weights, filepath);
}

// A function to dump the the caffe blob to a txt file
vector<float> extractBlobToVector(const caffe::BlobProto& blob)
{
    vector<float> weights;                                                                      // To store the weights/bias
    weights.reserve(blob.data_size());

    // copy the  data itself.
    copy(blob.data().begin(), blob.data().end(), std::back_inserter(weights));    

    return weights;
}

// Function that extract all the parameter from Caffe Layer,
// put it in a XLayer and Register it to XGraph
// TODO : @ARK : Use switch-case to get rid of long if-else-if chain
void ExtractParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    mapStrStr::const_iterator map_it = CaffeLayerMap.find(src.type());

    if (map_it->second == "Convolution")
    {
        ExtractConvolutionParameters(src, graph);
    }
    else if (map_it->second == "Dropout")
    {
        ExtractDropoutParameters(src, graph);
    }
    else if (map_it->second == "Pooling")
    {
        ExtractPoolingParameters(src, graph);
    }
    else if (map_it->second == "ReLU")
    {
        ExtractReLUParameters(src, graph);
    }
    else if (map_it->second == "InnerProduct")
    {
        ExtractFCParameters(src, graph);
    }
    else if (map_it->second == "Argmax")
    {
        ExtractArgmaxParameters(src, graph);
    }
    else if (map_it->second == "Softmax")
    {
        ExtractSoftmaxParameters(src, graph);
    }
    else if (map_it->second == "Deconvolution")
    {
        ExtractDeconvolutionParameters(src, graph);
    }
    else if (map_it->second == "Concat")
    {
        ExtractConcatParameters(src, graph);
    }
    else if (map_it->second == "Crop")
    {
        ExtractCropParameters(src, graph);
    }
    else if (map_it->second == "Flatten")
    {
        ExtractFlattenParameters(src, graph);
    }
    else if (map_it->second == "Permute")
    {
        ExtractPermuteParameters(src, graph);
    }
    else if (map_it->second == "L2Normalize")
    {
        ExtractL2NormalizeParameters(src, graph);
    }
    else if (map_it->second == "LRN")
    {
        ExtractLRNParameters(src, graph);
    }
    else if (map_it->second == "PriorBox")
    {
        ExtractPriorBoxParameters(src, graph);
    }
    else if (map_it->second == "Reshape")
    {
        ExtractReshapeParameters(src, graph);
    }
    else if (map_it->second == "NMS")
    {
        ExtractNMSParameters(src, graph);
    }
    else if (map_it->second == "Eltwise")
    {
        ExtractEltwiseParameters(src, graph);
    }
    else if (map_it->second == "BatchNorm")
    {
        ExtractBatchNormParameters(src, graph);
    }
    else if (map_it->second == "Scale")
    {
        ExtractScaleParameters(src, graph);
    }
    else if (map_it->second == "Power")
    {
        ExtractPowerParameters(src, graph);
    }
    else
    {
        cout << "[EP011] " <<  src.type() << " Layer : <" << src.name() << "> not supported in this version " << endl;
        exit(-1);
    }
}

void ExtractConvolutionParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    caffe::ConvolutionParameter src_parameter = src.convolution_param();
    mapStrStr::const_iterator map_it = CaffeLayerMap.find(src.type());
    string xlayerType = map_it->second;
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // Ensure number of input/output blobs
    checkNumberOfTopAndBottom(src, 1, 1);

    // Get OUTPUT_FEATURE_MAPS [MANDATORY]
    if(src_parameter.has_num_output())
    {
        dst->conv_params->M           =  src_parameter.num_output();
    }
    else
    {
        cerr << "[EP012] \"num_output\" is not specified in the convolution layer : " << src.name() << endl;
        exit(-1);
    }

    // Get FILTER SIZE [MANDATORY]
    // TODO : Currently "kernel_size" is supported. Support "kernel_h/w" in future
    if (src_parameter.kernel_size_size()>0)
    {
        dst->conv_params->filter_h = src_parameter.kernel_size(0);
        dst->conv_params->filter_w = src_parameter.kernel_size(0);
    }
    else
    {
        cerr << "[EP013] \"kernel_size\" is not specified in the convolution layer : " << src.name() << endl;
        exit(-1);
    }
    
    // Get PAD [DEFAULT = 0]
    // TODO : Currently "pad" is supported. Support "pad_h/w" in future
    dst->conv_params->pad_h = src_parameter.pad_size() > 0 ? src_parameter.pad(0) : 0;
    dst->conv_params->pad_w = src_parameter.pad_size() > 0 ? src_parameter.pad(0) : 0;

    // Get STRIDE [DEFAULT = 1]
    // TODO : Currently "stride" is supported. Support "stride_h/w" in future
    dst->conv_params->stride_h = src_parameter.stride_size() > 0 ? src_parameter.stride(0) : 1;
    dst->conv_params->stride_w = src_parameter.stride_size() > 0 ? src_parameter.stride(0) : 1;

    // Get DILATION [DEFAULT = 1]
    // TODO : Currently same dilation is done on all dimensions. But caffe support different dilation in different axis
    dst->conv_params->dilation = src_parameter.dilation_size() > 0 ? src_parameter.dilation(0) : 1;

    // Get GROUP[DEFAULT = 1]
    if (src_parameter.has_group() && (src_parameter.group() > 2))
    {
        cerr << "[EP039] Current version doesn't support group>2 for Convolution Layer" << endl;
        exit(-1);
    }
    dst->conv_params->group = src_parameter.has_group() ? src_parameter.group() : 1;

    // Get HAS_BIAS[DEFAULT = 1]
    dst->conv_params->has_bias = src_parameter.has_bias_term() ? src_parameter.bias_term() : 1;

    // Extract Precision Parameters
    ExtractPrecisionParameters(src, *dst);

    // Register the XLayer to graph
    graph.layers[src.name()] = dst;

    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also

    // Some sanity checks
    dst->conv_params->N     =   tmpBottom->shape.at(1);                         // XXX : Find any better place to do this
    if(dst->conv_params->N % dst->conv_params->group != 0)
    {
        cerr << "[EP014] The input channels should be a multiple of group for layer " << dst->name << endl;
        exit(-1);
    }

    tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                         // and consumer uses the full bottom data.
     
    // Check top. Make sure it is not registered in the Graph.blobs
    it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                // Replicate top shape in layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
    
}

void ExtractDeconvolutionParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    caffe::ConvolutionParameter src_parameter = src.convolution_param();
    mapStrStr::const_iterator map_it= CaffeLayerMap.find(src.type());
    string xlayerType = map_it->second;
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // Ensure number of input/output blobs
    checkNumberOfTopAndBottom(src, 1, 1);

    // Get OUTPUT_FEATURE_MAPS [MANDATORY]
    if(src_parameter.has_num_output())
    {
        dst->deconv_params->M           =  src_parameter.num_output();
    }
    else
    {
        cerr << "[EP015] \"num_output\" is not specified in the Deconvolution layer : " << src.name() << endl;
        exit(-1);
    }

    // Get FILTER SIZE [MANDATORY]
    // TODO : Currently "kernel_size" is supported. Support "kernel_h/w" in future
    if (src_parameter.kernel_size_size()>0)
    {
        dst->deconv_params->filter_h = src_parameter.kernel_size(0);
        dst->deconv_params->filter_w = src_parameter.kernel_size(0);
    }
    else
    {
        cerr << "[EP016] \"kernel_size\" is not specified in the Deconvolution layer : " << src.name() << endl;
        exit(-1);
    }
    
    // Get PAD [DEFAULT = 0]
    // TODO : Currently "pad" is supported. Support "pad_h/w" in future
    dst->deconv_params->pad_h = src_parameter.pad_size() > 0 ? src_parameter.pad(0) : 0;
    dst->deconv_params->pad_w = src_parameter.pad_size() > 0 ? src_parameter.pad(0) : 0;

    // Get STRIDE [DEFAULT = 1]
    // TODO : Currently "stride" is supported. Support "stride_h/w" in future
    dst->deconv_params->stride_h = src_parameter.stride_size() > 0 ? src_parameter.stride(0) : 1;
    dst->deconv_params->stride_w = src_parameter.stride_size() > 0 ? src_parameter.stride(0) : 1;

    // Get DILATION [DEFAULT = 1]
    dst->deconv_params->dilation = src_parameter.dilation_size() > 0 ? src_parameter.dilation(0) : 1;

    // Get GROUP[DEFAULT = 1]
    dst->deconv_params->group = src_parameter.has_group() ? src_parameter.group() : 1;
    
    // Get HAS_BIAS[DEFAULT = 1]
    dst->deconv_params->has_bias = src_parameter.has_bias_term() ? src_parameter.bias_term() : 1;


    // Register the XLayer to graph
    graph.layers[src.name()] = dst;

    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also

    dst->deconv_params->N     =   tmpBottom->shape.at(1);                         // XXX : Find any better place to do this

    // TODO : Following set of restrictions are kept for Deconvolution right now.
    ASSERT( (dst->deconv_params->pad_h == 0) && (dst->deconv_params->pad_w == 0), EP058,
            "Current version of Deconvolution supports only pad = 0"    );
    ASSERT( (dst->deconv_params->filter_h == 2 * dst->deconv_params->stride_h) ||
            (dst->deconv_params->filter_h == 2 * dst->deconv_params->stride_h - 1), EP059,
            "Current version of Deconvolution supports only filter_size = 2xStride [OR] filter_size = 2xStride - 1" );
    ASSERT( (dst->deconv_params->M == dst->deconv_params->N) &&
            (dst->deconv_params->M == dst->deconv_params->group) &&
            (dst->deconv_params->N == dst->deconv_params->group), EP060,
            "In the current deconvolution, number of input feature maps, number of output feature maps and group should be same");
    if(dst->deconv_params->N % dst->deconv_params->group != 0)
    {
        cerr << "[EP024] The input channels should be a multiple of group for layer " << dst->name << endl;
        exit(-1);
    }

    tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                         // and consumer uses the full bottom data.
     
    // Check top. Make sure it is not registered in the Graph.blobs
    it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
    
}


void ExtractDropoutParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::DropoutParameter src_parameter = src.dropout_param();
    mapStrStr::const_iterator type_it = CaffeLayerMap.find(src.type());
    string xlayerType = type_it->second;

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Check if it is inPlace.
    dst->dropout_params->inPlace = src.bottom(0) == src.top(0) ? true : false; 

    // Get Dropout Ratio
    dst->dropout_params->dropout_ratio           =  src_parameter.dropout_ratio();

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //


    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also

    tmpBottom->consumers.push_back(dst->name);                                   // Add layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                         // and consumer uses the full bottom data.
     
    // Here, things are a little different because of the inplace operation
    if(dst->dropout_params->inPlace == false)
    {
        // Check top. Make sure it is not registered in the Graph.blobs
        it = graph.checkIfBlobExists(src.top(0), true, true);

        // Execution reached here means, top blob doesn't exist, so create.
        XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
        dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

        // Update layer "top" info with top name and shape
        tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
        dst->computeOutputDim();                                    // Calculate the output blob shape
        tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
        dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

        // Finally Register the top blob to graph
        graph.blobs[tmpTop->name] = tmpTop;
    }
    else
    {
        XBlob* tmpTop = tmpBottom;                                // In in-place operation, top is same as bottom 
        dst->top.push_back(nameIndex(tmpTop));                   // Add it to current layer top list

        // Update layer "top" info with top name and shape
        tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
        dst->computeOutputDim();                                    // Calculate the output blob shape
        tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
        dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also
    }
}

void ExtractReLUParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::ReLUParameter src_parameter = src.relu_param();
    mapStrStr::const_iterator type_it = CaffeLayerMap.find(src.type());
    string xlayerType = type_it->second;

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Check if it is inPlace.
    dst->relu_params->inPlace = src.bottom(0) == src.top(0) ? true : false; 

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //


    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    tmpBottom->consumers.push_back(dst->name);                                   // Add layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                         // and consumer uses the full bottom data.
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also
     
    // Here, things are a little different because of the inplace operation
    if(dst->relu_params->inPlace == false)
    {
        // Check top. Make sure it is not registered in the Graph.blobs
        it = graph.checkIfBlobExists(src.top(0), true, true);

        // Execution reached here means, top blob doesn't exist, so create.
        XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
        dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

        // Update layer "top" info with top name and shape
        tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
        dst->computeOutputDim();                                    // Calculate the output blob shape
        tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
        dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

        // Finally Register the top blob to graph
        graph.blobs[tmpTop->name] = tmpTop;
    }
    else
    {
        XBlob* tmpTop = tmpBottom;                                // In in-place operation, top is same as bottom 
        dst->top.push_back(nameIndex(tmpTop));                   // Add it to current layer top list

        // Update layer "top" info with top name and shape
        tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
        dst->computeOutputDim();                                    // Calculate the output blob shape
        tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
        dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also
    }
}

void ExtractPoolingParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::PoolingParameter src_parameter = src.pooling_param();
    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // Get POOLING_TYPE [OPTIONAL, caffe default = MAX]
    dst->pool_params->PoolType = (src_parameter.has_pool()) ? (PoolingType)src_parameter.pool() : MAX;    

    // Get KERNEL_SIZE [MANDATORY]
    if(src_parameter.has_kernel_size())
    {
        dst->pool_params->kernel_h           =  src_parameter.kernel_size();
        dst->pool_params->kernel_w           =  src_parameter.kernel_size();
    }
    else
    {
        // if(!(src_parameter.has_global_pooling() || (src_parameter.global_pooling() == false)))
        if(src_parameter.global_pooling() == false)
        {
            cout << "[EP017] Kernel Size is not mentioned for layer : " << src.name() << endl;
            exit(-1);
        }
    }

    // Get PAD [OPTIONAL; Caffe default = 0]
    dst->pool_params->pad_h           =  (src_parameter.has_pad()) ? src_parameter.pad() : 0;
    dst->pool_params->pad_w           =  dst->pool_params->pad_h; 
    
    // Get STRIDE [OPTIONAL; Caffe default = 1]
    dst->pool_params->stride_h           =  src_parameter.has_stride() ? src_parameter.stride() : 1;
    dst->pool_params->stride_w           =  dst->pool_params->stride_h;

    // Extract Precision Parameters
    if(dst->pool_params->PoolType == AVE)
        ExtractPrecisionParameters(src, *dst, false);

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also

    tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                         // and consumer uses the full bottom data.

    // If global_pooling = true, kernel size is same as the feature map size
    if(src_parameter.global_pooling() == true)
    {
        dst->pool_params->kernel_h           =  tmpBottom->shape.at(2);
        dst->pool_params->kernel_w           =  tmpBottom->shape.at(3);
    }

    dst->pool_params->N                     =   tmpBottom->shape.at(1);
     
    // Check top. Make sure it is not registered in the Graph.blobs
    it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
}

void ExtractFCParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::InnerProductParameter src_parameter = src.inner_product_param();
    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // Get NUM_OF_OUTPUT [MANDATORY]
    if(src_parameter.has_num_output())
    {
        dst->fc_params->M = src_parameter.num_output();
    }
    else
    {
        cout << "[EP018] num_output is not mentioned for layer : " << src.name() << endl;
        exit(-1);
    }

    // Get HAS_BIAS [DEFAULT = 1]
    dst->fc_params->has_bias = src_parameter.has_bias_term() ? src_parameter.bias_term() : 1;

    // TODO : There are other parameters like 'transpose', 'axis' etc. Currently not supported

    // Extract Precision Parameters
    ExtractPrecisionParameters(src, *dst);

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also

    tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.
    dst->fc_params->N     =   flattenTensorDim(tmpBottom->shape).at(3);                         // XXX : It is a bad location to do this
     
    // Check top. Make sure it is not registered in the Graph.blobs
    it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());

    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
}

void ExtractArgmaxParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::ArgMaxParameter src_parameter = src.argmax_param();
    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // get TOP_K [OPTIONAL : default = 1]
    dst->argmax_params->top_k = src_parameter.has_top_k() ? src_parameter.top_k() : 1;
    
    // get AXIS [OPTIONAL : default = 1]
    dst->argmax_params->axis = src_parameter.has_axis() ? src_parameter.axis() : 1;


    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also

    tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.
     
    // Fill out nclasses & nboxes
    // nboxes is the dimension along the axis. nclasses is the rest.
    dst->argmax_params->nclasses = tmpBottom->shape.at(dst->argmax_params->axis);
    dst->argmax_params->nboxes = getSize(tmpBottom->shape)/tmpBottom->shape.at(dst->argmax_params->axis);

    // Check top. Make sure it is not registered in the Graph.blobs
    it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
}

void ExtractSoftmaxParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::SoftmaxParameter src_parameter = src.softmax_param();
    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // get AXIS [OPTIONAL : default = 1]
    dst->softmax_params->axis = src_parameter.has_axis() ? src_parameter.axis() : 1;

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also

    tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.

    // Fill out nclasses & nboxes
    // nboxes is the dimension along the axis. nclasses is the rest.
    dst->softmax_params->nclasses = tmpBottom->shape.at(dst->softmax_params->axis);
    dst->softmax_params->nboxes = getSize(tmpBottom->shape)/tmpBottom->shape.at(dst->softmax_params->axis);
     
    // Check top. Make sure it is not registered in the Graph.blobs
    it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
}

void ExtractCropParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::CropParameter src_parameter = src.crop_param();
    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 2, 1);

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // Get AXIS [OPTIONAL, DEFAULT=2]
    // TODO : @ARK : Right now only axis=2 is supported
    if(src_parameter.has_axis() && src_parameter.axis() != 2)
    {
        cerr << "[EP019] In crop layer, only axis = 2 is supported currently" << endl;
        exit(-1);
    }
    
    dst->crop_params->axis = src_parameter.has_axis() ? src_parameter.axis() : 2;

    // Get OFFSET [MANDATORY]
    // TODO : @ARK : Only spatial cropping with same offset in both X & Y direction is supported now.
    if(src_parameter.offset_size() == 0)
    {
        cerr << "[EP020] In crop layer, offset should be mentioned" << endl;
        exit(-1);
    }
    
    dst->crop_params->offset = src_parameter.offset(0);

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Crop has two bottom, so iterate through them

    for(int i = 0; i<src.bottom_size(); i++)
    {
        // Check bottom first. Make sure it is already registered in the Graph.blobs
        map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(i), true, false);

        // Execution here reached means, bottom blob exists in graph. So update its fields
        XBlob* tmpBottom = it->second;
        dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
        dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also
        tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
        tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.
    }
     
    // Once bottom shapes are ready, assign the crop_height & crop_width since we do only spatial cropping
    vector<int> refBottomShape = dst->bottomShape.at(1);
    dst->crop_params->crop_height = refBottomShape.at(2);
    dst->crop_params->crop_width = refBottomShape.at(3);

    // Check top. Make sure it is not registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
}

void ExtractConcatParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::ConcatParameter src_parameter = src.concat_param();
    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, -1, 1);

    // Check if atleast one bottom is present
    if(src.bottom_size() <= 0)
    {
        cerr << "[EP021] There should be atleast one bottom for concat layer: " << src.name() << endl;
        exit(-1);
    }

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // axis [OPTIONAL : default = 1]
    dst->concat_params->axis = src_parameter.has_axis() ? src_parameter.axis() : 1 ;    

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Concat has multiple bottom, so iterate through them

    for(int i = 0; i<src.bottom_size(); i++)
    {
        // Check bottom first. Make sure it is already registered in the Graph.blobs
        map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(i), true, false);

        // Execution here reached means, bottom blob exists in graph. So update its fields
        XBlob* tmpBottom = it->second;
        dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
        dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also
        tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
        tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.
    }
     
    // Check top. Make sure it is not registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
}

void ExtractFlattenParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::FlattenParameter src_parameter = src.flatten_param();
    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // TODO : @ARK : Currently we support flatten with end_axis = -1, but any axis (for support in SSD)
    // And flatten layer will be removed from the graph by BE.
    // We need to see possible varieties of Flatten. Until then we support only end_axis=-1
    if((src_parameter.has_end_axis()   && (src_parameter.end_axis() != -1)) )
    {
        cerr << "[EP040] Current version supports flatten layer with end_axis=-1 only" << endl;
        exit(-1);
    }

    dst->flatten_params->axis = src_parameter.has_axis() ? src_parameter.axis() : 1;
    dst->flatten_params->end_axis = src_parameter.has_end_axis() ? src_parameter.end_axis() : -1;

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also

    tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.
     
    // Check top. Make sure it is not registered in the Graph.blobs
    it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
}

void ExtractPermuteParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::PermuteParameter src_parameter = src.permute_param();
    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // Get ORDER [REPEATED]
    vector<int> tmp_order;
    tmp_order.resize(src_parameter.order_size());
    copy(src_parameter.order().begin(), src_parameter.order().end(), tmp_order.begin());    
    // TODO : @ARK : Currently we support an order (0, 2, 3, 1) only. This kernel should be generic for any order.
    if( (tmp_order.size() != 4)     || 
        (tmp_order.at(0)  != 0)     || 
        (tmp_order.at(1)  != 2)     || 
        (tmp_order.at(2)  != 3)     || 
        (tmp_order.at(3)  != 1)     ) 
    {
        cerr << "[EP041] Current version support permute layer with order (0, 2, 3, 1) only. Given order : " << TensorDimToString(tmp_order, ", ") << endl;
        exit(-1);
    }
    else
    {
        dst->permute_params->order = tmp_order;
    }

    // Extract Precision Parameters
    ExtractPrecisionParameters(src, *dst, false);

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also

    tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.
     
    // Check top. Make sure it is not registered in the Graph.blobs
    it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
}


void ExtractLRNParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::LRNParameter src_parameter = src.lrn_param();
    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    dst->lrn_params->lsize              = src_parameter.has_local_size()? src_parameter.local_size(): 5;
    dst->lrn_params->alpha              = src_parameter.has_alpha()     ? src_parameter.alpha()     : 1.0;
    dst->lrn_params->beta               = src_parameter.has_beta()      ? src_parameter.beta()      : 0.75;
    dst->lrn_params->k                  = src_parameter.has_k()         ? src_parameter.k()         : 1.0;
    dst->lrn_params->type               = src_parameter.has_norm_region()? LRNType(src_parameter.norm_region()): ACROSS_CHANNELS;

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also

    tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.
     
    // Check top. Make sure it is not registered in the Graph.blobs
    it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
}


void ExtractL2NormalizeParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::NormalizeParameter src_parameter = src.norm_param();
    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // TODO : @ARK : Currently, we support only across_spatial=false right now.
    if(src_parameter.has_across_spatial() && (src_parameter.across_spatial() == true))
    {
        cerr << "[EP042] Current version supports only L2-Normalization with across_spatial = false" << endl;
        exit(-1);
    }

    dst->l2norm_params->across_spatial  = src_parameter.has_across_spatial() ? src_parameter.across_spatial() : true;
    dst->l2norm_params->channel_shared  = src_parameter.has_channel_shared() ? src_parameter.channel_shared() : true;
    dst->l2norm_params->eps             = src_parameter.has_eps() ? src_parameter.eps() : 1e-10;

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also

    tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.
     
    // Check top. Make sure it is not registered in the Graph.blobs
    it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
}

void ExtractPriorBoxParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::PriorBoxParameter src_parameter = src.prior_box_param();
    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 2, 1);

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // min_size [MANDATORY] TODO : Currently one & only-one value of min_size is allowed
    if(src_parameter.min_size_size() == 1)
    {
        std::copy(src_parameter.min_size().begin(), src_parameter.min_size().end(), std::back_inserter(dst->priorbox_params->min_size));
    }
    else
    {
        cerr << "[EP043] current version supports exactly one min_size for PriorBox Layer : " << src.name() << endl;
        exit(-1);
    }

    // max_size [OPIONAL as per SSDv3. MANDATORY as per SSDv4]. What TODO? : Currently maximum one value of max_size is allowed
    if(src_parameter.max_size_size() <= 1)
    {
        std::copy(src_parameter.max_size().begin(), src_parameter.max_size().end(), std::back_inserter(dst->priorbox_params->max_size));
    }
    else
    {
        cerr << "[EP044] Current version supports maximum one max_size for PriorBox Layer : " << src.name() << endl;
        exit(-1);
    }
    
    // aspect_ratio [OPTIONAL]
    if(src_parameter.aspect_ratio_size() > 0)
    {  
        std::copy(src_parameter.aspect_ratio().begin(), src_parameter.aspect_ratio().end(), std::back_inserter(dst->priorbox_params->aspect_ratio));
    }

    // flip [OPTIONAL, default = true]
    dst->priorbox_params->flip = src_parameter.has_flip() ? src_parameter.flip() : true;
    
    // clip [OPTIONAL, default = false]
    dst->priorbox_params->clip = src_parameter.has_clip() ? src_parameter.clip() : false;

    // variance
    int varSize = src_parameter.variance_size();

    // If 4 variance values are provided, copy them
    if(varSize == 4)
    {
        for(int i=0; i<varSize; i++)
        {
            if(src_parameter.variance(i) > 0)
            {
                dst->priorbox_params->variance.push_back(src_parameter.variance(i));
            }
            else
            {
                cerr << "[EP045] variance should be greater than 0 in PriorBox Layer : " << src.name() << endl;
                exit(-1);
            }
        }
    }
    // If only 1 variance is given, replicate them
    else if(varSize == 1)
    {
        if(src_parameter.variance(0) > 0)
        {
            dst->priorbox_params->variance.resize(4, src_parameter.variance(0));
        }
        else
        {
            cerr << "[EP046] variance should be greater than 0 in PriorBox Layer : " << src.name() << endl;
            exit(-1);
        }
    }
    // If nothing is given, replicate default value = 0.1
    else
    {
        dst->priorbox_params->variance.resize(4, 0.1);
    }

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // it has two bottom, so iterate through them
    // first bottom is the intermediate layer
    // second bottom is the main input

    for(int i = 0; i<src.bottom_size(); i++)
    {
        // Check bottom first. Make sure it is already registered in the Graph.blobs
        map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(i), true, false);

        // Execution here reached means, bottom blob exists in graph. So update its fields
        XBlob* tmpBottom = it->second;
        dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
        dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also
        tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
        tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.
    }
     
    // Check top. Make sure it is not registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;

    // ---------------------------------  Pre-compute priorbox and variance -------------------- //

    int layerheight= dst->bottomShape.at(0).at(2);
    int layerwidth = dst->bottomShape.at(0).at(3);
    int imageheight= dst->bottomShape.at(1).at(2);
    int imagewidth = dst->bottomShape.at(1).at(3);
    vector<float> ar = dst->priorbox_params->aspect_ratio;
    vector<float> min_size = dst->priorbox_params->min_size;
    vector<float> max_size = dst->priorbox_params->max_size;
    vector<float> var = dst->priorbox_params->variance;
    bool flip = dst->priorbox_params->flip;
    bool clip = dst->priorbox_params->clip;

    dst->priorbox_params->pbox =  computePriorBoxes(layerwidth, layerheight, imagewidth, imageheight, 
                                                  ar, min_size, max_size, var, flip, clip, 0.0, 0.0, 0.5);
    dst->priorbox_params->pboxShape = dst->topShape.at(0);

    // Keep priorboxes in txt files for debug purposes
    // string filename = dst->uname + "_pboxes";
    // savetxt(priorboxes, filename);
}


void ExtractReshapeParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::ReshapeParameter src_parameter = src.reshape_param();
    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // shape [MANDATORY]
    if(src_parameter.shape().dim_size() == 0)
    {
        cerr << "[EP047] Shape is not mentioned for Reshape Layer : " << src.name() << endl;
        exit(-1);
    }
    else
    {
        caffe::BlobShape shape = src_parameter.shape();
        std::copy(shape.dim().begin(), shape.dim().end(), std::back_inserter(dst->reshape_params->shape));
    }

    // TODO : @ARK : Current version support only number_axes = -1
    if(src_parameter.has_num_axes() && src_parameter.num_axes() != -1)
    {
        cerr << "[EP048] Current version supports only num_axes = -1 in Reshape Layer : " << src.name() << endl;
        exit(-1);
    }

    dst->reshape_params->axis           = src_parameter.has_axis()       ? src_parameter.axis()     : 0;
    dst->reshape_params->num_axes       = src_parameter.has_num_axes()   ? src_parameter.num_axes() : -1;

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also

    tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.
     
    // Check top. Make sure it is not registered in the Graph.blobs
    it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
}


void ExtractNMSParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::DetectionOutputParameter src_parameter = src.detection_output_param();
    caffe::NonMaximumSuppressionParameter nms_parameter = src_parameter.nms_param();

    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 3, 1);

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // Extract num_classes [MANDATORY]
    if(src_parameter.has_num_classes())
    {
        dst->nms_params->num_classes = src_parameter.num_classes();
    }
    else
    {
        cerr << "[EP049] num_classes is not specified for layer DetectionOutputLayer : " << src.name() << endl;
        exit(-1);
    }

    // Extract share_location [OPTIONAL : true]
    if(src_parameter.share_location())
    {
        dst->nms_params->share_location = true;
    }
    else
    {
        cerr << "[EP050] Current version supports only share_location = true in DetectionOutputLayer : " << src.name() << endl;
        exit(-1);
    }

    // Extract background_label_id [OPTIONAL : 0]
    if(src_parameter.background_label_id() != 0)
    {
        cerr << "[EP051] Current version supports only background_label_id = 0 in DetectionOutputLayer : " << src.name() << endl;
        exit(-1);
    }
    dst->nms_params->background_label_id = src_parameter.background_label_id();

    // Extract nms_threshold[OPTIONAL : 0.3]
    dst->nms_params->nms_threshold = nms_parameter.nms_threshold();

    // Extract nms_top_k [OPTIONAL ]
    dst->nms_params->nms_top_k = nms_parameter.has_top_k() ? nms_parameter.top_k() : -1; 
    
    // Extract nms_eta [OPTIONAL : 1.0 ]
    dst->nms_params->nms_eta = nms_parameter.has_eta() ? nms_parameter.eta() : 1.0; 

    // Extract code_type [OPTIONAL : CORNER]
    if(src_parameter.code_type() != caffe::PriorBoxParameter_CodeType(CENTER_SIZE))
    {
        cerr << "[EP052] Current version supports only code_type = CENTER_SIZE in DetectionOutputLayer : " << src.name() << endl;
        exit(-1);
    }
    dst->nms_params->code_type = PriorBoxType(src_parameter.code_type());

    // Extract keep_top_k [OPTIONAL : -1 ]
    dst->nms_params->keep_top_k = src_parameter.has_keep_top_k() ? src_parameter.keep_top_k() : -1; 

    // Extract confidence_threshold [OPTIONAL : 0.01 ]
    dst->nms_params->confidence_threshold = src_parameter.has_confidence_threshold() ? src_parameter.confidence_threshold() : 0.01; 

    // Extract variance_encoded_in_target [OPTIONAL : false]
    if(src_parameter.variance_encoded_in_target() == true)
    {
        cerr << "[EP053] Current version supports only 'variance_encoded_in_target = false' in DetectionOutputLayer : " << src.name() << endl;
        exit(-1);
    }
    dst->nms_params->variance_encoded_in_target = src_parameter.variance_encoded_in_target();

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Crop has two bottom, so iterate through them

    for(int i = 0; i<src.bottom_size(); i++)
    {
        // Check bottom first. Make sure it is already registered in the Graph.blobs
        map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(i), true, false);

        // Execution here reached means, bottom blob exists in graph. So update its fields
        XBlob* tmpBottom = it->second;
        dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
        dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also
        tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
        tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.
    }
     
    // Check top. Make sure it is not registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
}

void ExtractEltwiseParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::EltwiseParameter src_parameter = src.eltwise_param();
    mapStrStr::const_iterator it_type = CaffeLayerMap.find(src.type());
    string xlayerType = it_type->second;

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 2, 1);

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // Get Op Type [OPTIONAL, DEFAULT=SUM]
    dst->eltwise_params->type = (src_parameter.has_operation()) ? (EltOpType)src_parameter.operation() : (EltOpType)ELT_SUM;    

    // Currently only SUM/PROD is supported
    ASSERT  ((dst->eltwise_params->type == ELT_SUM) || (dst->eltwise_params->type == ELT_PROD), EP061,
            "For Eltwise layer, only SUM and PROD operation is allowed")

    // Get coeff. TODO : ARK: Currently coeff is not supported
    ASSERT(src_parameter.coeff().size() == 0, EP062, "Coeff for Eltwise layer is not currently supported") 

    // Extract Precision Parameters
    ExtractPrecisionParameters(src, *dst, false);

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //

    // Crop has two bottom, so iterate through them

    for(int i = 0; i<src.bottom_size(); i++)
    {
        // Check bottom first. Make sure it is already registered in the Graph.blobs
        map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(i), true, false);

        // Execution here reached means, bottom blob exists in graph. So update its fields
        XBlob* tmpBottom = it->second;
        dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
        dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also
        tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
        tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.
    }
     
    // Check top. Make sure it is not registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
}

void ExtractBatchNormParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::BatchNormParameter src_parameter = src.batch_norm_param();
    mapStrStr::const_iterator type_it = CaffeLayerMap.find(src.type());
    string xlayerType = type_it->second;

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Check if it is inPlace.
    dst->batchnorm_params->inPlace = src.bottom(0) == src.top(0) ? true : false; 

    // Get use_global_stats [Mandatory]
    // TODO : Ark : Currently only axis=1 is allowed
    ASSERT(src_parameter.has_use_global_stats(), EP063, "use_global_stats is not mentioned in BatchNorm Layer : " + src.name())
    dst->batchnorm_params->global_stats = src_parameter.use_global_stats();

    // Get eps [default: 1e-5]
    dst->batchnorm_params->eps = src_parameter.has_eps() ? src_parameter.eps() : 0.00001;

    // Extract Precision Parameters
    ExtractPrecisionParameters(src, *dst, false);

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //


    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    tmpBottom->consumers.push_back(dst->name);                                   // Add layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                         // and consumer uses the full bottom data.
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also
     
    // Here, things are a little different because of the inplace operation
    if(dst->batchnorm_params->inPlace == false)
    {
        // Check top. Make sure it is not registered in the Graph.blobs
        it = graph.checkIfBlobExists(src.top(0), true, true);

        // Execution reached here means, top blob doesn't exist, so create.
        XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
        dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

        // Update layer "top" info with top name and shape
        tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
        dst->computeOutputDim();                                    // Calculate the output blob shape
        tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
        dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

        // Finally Register the top blob to graph
        graph.blobs[tmpTop->name] = tmpTop;
    }
    else
    {
        XBlob* tmpTop = tmpBottom;                                // In in-place operation, top is same as bottom 
        dst->top.push_back(nameIndex(tmpTop));                   // Add it to current layer top list

        // Update layer "top" info with top name and shape
        tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
        dst->computeOutputDim();                                    // Calculate the output blob shape
        tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
        dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also
    }
}

void ExtractScaleParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::ScaleParameter src_parameter = src.scale_param();
    mapStrStr::const_iterator type_it = CaffeLayerMap.find(src.type());
    string xlayerType = type_it->second;

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Check if it is inPlace.
    dst->scale_params->inPlace = src.bottom(0) == src.top(0) ? true : false; 

    // Check if bias there or not
    dst->scale_params->has_bias = src_parameter.has_bias_term() ? src_parameter.bias_term() : false;

    // Get axis [default: 1]
    // TODO : Ark : Currently only axis=1 is allowed
    dst->scale_params->axis = src_parameter.has_axis() ? src_parameter.axis() : 1;
    ASSERT(dst->scale_params->axis == 1, EP062, "Currently, axis should be 1 in Scale Layer")

    // Get num_axes [default: 1]
    // TODO : Ark : Currently only num_axes=1 is allowed
    dst->scale_params->num_axes = src_parameter.has_num_axes() ? src_parameter.num_axes() : 1;
    ASSERT(dst->scale_params->num_axes == 1, EP063, "Currently, num_axes should be 1 in Scale Layer")

    // Extract Precision Parameters
    ExtractPrecisionParameters(src, *dst, false);

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //


    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    tmpBottom->consumers.push_back(dst->name);                                   // Add layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                         // and consumer uses the full bottom data.
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also
     
    // Here, things are a little different because of the inplace operation
    if(dst->scale_params->inPlace == false)
    {
        // Check top. Make sure it is not registered in the Graph.blobs
        it = graph.checkIfBlobExists(src.top(0), true, true);

        // Execution reached here means, top blob doesn't exist, so create.
        XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
        dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

        // Update layer "top" info with top name and shape
        tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
        dst->computeOutputDim();                                    // Calculate the output blob shape
        tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
        dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

        // Finally Register the top blob to graph
        graph.blobs[tmpTop->name] = tmpTop;
    }
    else
    {
        XBlob* tmpTop = tmpBottom;                                // In in-place operation, top is same as bottom 
        dst->top.push_back(nameIndex(tmpTop));                   // Add it to current layer top list

        // Update layer "top" info with top name and shape
        tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
        dst->computeOutputDim();                                    // Calculate the output blob shape
        tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
        dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also
    }
}


void ExtractPowerParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::PowerParameter src_parameter = src.power_param();
    mapStrStr::const_iterator type_it = CaffeLayerMap.find(src.type());
    string xlayerType = type_it->second;

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType);
    dst->uname = graph.getUniqueLayerName();

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Check if it is inPlace.
    dst->power_params->inPlace = src.bottom(0) == src.top(0) ? true : false; 

    // Get power [default: 1.0]
    // TODO : Ark : Currently only power=1.0 is allowed
    dst->power_params->power = src_parameter.has_power() ? src_parameter.power() : 1.0;
    ASSERT(dst->power_params->power == 1.0, EP072, "Current version of xfDNN supports only power = 1.0 in Power Layer")

    // Get scale [default: 1]
    // TODO : Ark : Currently only scale=1 is allowed
    dst->power_params->scale = src_parameter.has_scale() ? src_parameter.scale() : 1.0;
    ASSERT(dst->power_params->scale == -1.0, EP073, "Current version of xfDNN supports only scale = -1.0 in Power Layer")

    // Get shift [default: 1]
    // TODO : Ark : Currently only shift = 0.0 is allowed
    dst->power_params->shift = src_parameter.has_shift() ? src_parameter.shift() : 0.0;
    ASSERT(dst->power_params->shift == 0.0, EP074, "Current version of xfDNN supports only shift = 0.0 in Power Layer")

    // Extract Precision Parameters
    // ExtractPrecisionParameters(src, *dst, false);

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;
    
    // -----------------------------   Setup Blobs and Connections ----------------------- //


    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    // Execution here reached means, bottom blob exists in graph. So update its fields
    XBlob* tmpBottom = it->second;
    dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
    tmpBottom->consumers.push_back(dst->name);                                   // Add layer to bottom consumers
    tmpBottom->consumerDim.push_back(tmpBottom->shape);                         // and consumer uses the full bottom data.
    dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also
     
    // Here, things are a little different because of the inplace operation
    if(dst->power_params->inPlace == false)
    {
        // Check top. Make sure it is not registered in the Graph.blobs
        it = graph.checkIfBlobExists(src.top(0), true, true);

        // Execution reached here means, top blob doesn't exist, so create.
        XBlob* tmpTop = new XBlob(src.top(0), graph.getUniqueBlobName());
        dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

        // Update layer "top" info with top name and shape
        tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
        dst->computeOutputDim();                                    // Calculate the output blob shape
        tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
        dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

        // Finally Register the top blob to graph
        graph.blobs[tmpTop->name] = tmpTop;
    }
    else
    {
        XBlob* tmpTop = tmpBottom;                                // In in-place operation, top is same as bottom 
        dst->top.push_back(nameIndex(tmpTop));                   // Add it to current layer top list

        // Update layer "top" info with top name and shape
        tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
        dst->computeOutputDim();                                    // Calculate the output blob shape
        tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
        dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also
    }
}

// This function simply checks the number of tops and bottoms and exit if they don't match with prototxt
// A negative number denotes it can have any number of tops/bottoms
// Zero means, it doesn't have top/bottom. TODO : @ARK: Does such layer exist? Don't know.
void checkNumberOfTopAndBottom(const caffe::LayerParameter& src, int nB=-1, int nT=-1)
{
    if((nB >= 0) && (src.bottom_size() != nB))
    {
        cerr << "[EP022] " << src.type() << " accepts strictly " << nB << " bottom blobs." << " "
             << "You have provided " << src.bottom_size() << " bottom blobs. Check deploy file again" << endl;
        exit(-1);
    }

    if((nT >= 0) && (src.top_size() != nT))
    {
        cerr << "[EP023] " << src.type() << " accepts strictly " << nT << " top blobs." << " "
             << "You have provided " << src.top_size() << " top blobs. Check deploy file again" << endl;
        exit(-1);
    }
}

void readDeployFile(const string& deployFile, caffe::NetParameter* Net)
{
    int fileDescriptor = open(deployFile.c_str(), O_RDONLY);
    if(fileDescriptor < 0)
    {
        cerr << "[EP001] Couldn't open the deploy file : " << deployFile << endl;
        exit(-1);
    }

    // Merge the deploy file to caffe Net
    google::protobuf::io::FileInputStream* fileInput = new google::protobuf::io::FileInputStream(fileDescriptor);
    bool status = google::protobuf::TextFormat::Parse(fileInput, Net);
    if(!status)
    {
        cerr << "[EP002] Couldn't Parse the deploy file : " << deployFile << endl;
        exit(-1);
    } 

    delete fileInput;
    close(fileDescriptor);
}


void readModelFile(const string& caffemodelFile, caffe::NetParameter* Net)
{
    fstream infile(caffemodelFile.c_str(), ios::in | ios::binary);
    if(!infile)
    {
        cerr << "[EP024] Couldn't open the caffemodel file : " << caffemodelFile << endl;
        exit(-1);
    }

    // Merge the deploy file to caffe Net
    bool binMergeStatus = Net->ParseFromIstream(&infile);
    if(!binMergeStatus)
    {
        cerr << "[EP025] Couldn't Parse the CaffeModel file : " << caffemodelFile << endl;
        exit(-1);
    }
    infile.close();
}

void readMeanBinaryFile(const string& caffeMeanFile, caffe::BlobProto* blob)
{
    fstream infile(caffeMeanFile.c_str(), ios::in | ios::binary);
    if(!infile)
    {
        cerr << "[EP024] Couldn't open the Caffe Mean file : " << caffeMeanFile << endl;
        exit(-1);
    }

    // Merge the deploy file to caffe Net
    bool binMergeStatus = blob->ParseFromIstream(&infile);
    if(!binMergeStatus)
    {
        cerr << "[EP025] Couldn't Parse the Caffe Mean file : " << caffeMeanFile << endl;
        exit(-1);
    }
    infile.close();
}

// Extract the mean data
void ExtractCaffeMeanData(const caffe::BlobProto* blob, XGraph* graph)
{
    // Extract the mean data itself
    graph->meanData.reserve(blob->data_size());
    copy(blob->data().begin(), blob->data().end(), std::back_inserter(graph->meanData));    

    // Extract the mean shape
    // Caffe compute_mean() still uses deprecated API. So be ready for both
    caffe::BlobShape shape = blob->shape();
    if(shape.dim_size() > 0)
    {
        graph->meanShape.reserve(shape.dim_size());
        copy(shape.dim().begin(), shape.dim().end(), std::back_inserter(graph->meanShape));
    }
    else
    {
        // Use vector.reserve only if you do push_back() else use vector.resize() only
        graph->meanShape.resize(4);
        graph->meanShape.at(0) = 1;
        graph->meanShape.at(1) = blob->has_channels() ? blob->channels() : 1;
        graph->meanShape.at(2) = blob->has_height() ? blob->height() : 1;
        graph->meanShape.at(3) = blob->has_width() ? blob->width() : graph->meanData.size();
    }
}

// Extract FP Precision Parameters
void ExtractPrecisionParameters(const caffe::LayerParameter& src, XLayer& layer, bool isParamsPrecisionMandatory)
{
    caffe::QuantizationParameter src_params;

    // XXX : @ARK : This warning should be a fatal error. For release, just uncomment "exit(-1)" line.
    if(src.has_quantization_param())
        src_params = src.quantization_param();
    else
    {
        cerr << "[EP061] Precision parameters are not defined for the layer " << src.name() << ". " << endl;
        exit(-1);
    }    

    // input and output precision parameters are mandatory for every layer.
    ASSERT((src_params.has_bw_layer_in() && src_params.has_bw_layer_out() && 
            src_params.has_fl_layer_in() && src_params.has_fl_layer_out()), EP065,
            "Input and output precision parameters are not specified for the layer " << src.name() << "." )

    layer.user_ip_bw = src_params.bw_layer_in();
    layer.user_op_bw = src_params.bw_layer_out();
    layer.user_ip_fl = src_params.fl_layer_in();
    layer.user_op_fl = src_params.fl_layer_out();

    // Now convert user precision format to HW precision format 
    DynamicFPToHWFormat(INPUT_BW, layer.user_ip_bw, layer.user_ip_fl, layer.ip_bw, layer.ip_fl, true);
    DynamicFPToHWFormat(OUTPUT_BW, layer.user_op_bw, layer.user_op_fl, layer.op_bw, layer.op_fl, true);

    // weights precision is mandatory for [Convolution, FC] layers.
    ELOG( (isParamsPrecisionMandatory) && ((!src_params.has_bw_params()) || (!src_params.has_fl_params())),
            EP066, "Parameter Precisions should be mentioned for the layer " << src.name() << "." )

    layer.user_wt_bw = src_params.bw_params();
    layer.user_wt_fl = src_params.fl_params();

    // weights precision should be always bw=8, fl=7
    /*ELOG((isParamsPrecisionMandatory) && ((layer.user_wt_bw != WEIGHT_BW) || (layer.user_wt_fl != 7)), EP054,
            "Parameter Precision should be always: bw_params = 8, fl_params = 7." )*/

    DynamicFPToHWFormat(WEIGHT_BW, layer.user_wt_bw, layer.user_wt_fl, layer.wt_bw, layer.wt_fl, false);

    // BatchNorm precision parameters
    if(layer.type == "BatchNorm")
    {
        ELOG( (layer.type == "BatchNorm") && ( !src_params.has_batchnorm_mean_bw() || !src_params.has_batchnorm_variance_bw() ||
                    !src_params.has_batchnorm_mean_fl() || !src_params.has_batchnorm_variance_fl() ), 
                EP067, "Precision Parameters should be mentioned for mean and variance in BatchNorm Layer : " << src.name() )

        layer.user_bn_mean_bw = src_params.batchnorm_mean_bw();
        layer.user_bn_variance_bw = src_params.batchnorm_variance_bw();
        layer.user_bn_mean_fl = src_params.batchnorm_mean_fl();
        layer.user_bn_variance_fl = src_params.batchnorm_variance_fl();

        DynamicFPToHWFormat(BATCHNORM_BW, layer.user_bn_mean_bw, layer.user_bn_mean_fl, layer.bn_mean_bw, layer.bn_mean_fl, false);
        DynamicFPToHWFormat(BATCHNORM_BW, layer.user_bn_variance_bw, layer.user_bn_variance_fl, layer.bn_variance_bw, layer.bn_variance_fl, false);
    }

    // Scale layer precision parameters
    if(layer.type == "Scale")
    {
        ELOG( (layer.type == "Scale") && (  !src_params.has_scale_gamma_bw() || !src_params.has_scale_beta_bw() ||
                                            !src_params.has_scale_gamma_fl() || !src_params.has_scale_beta_fl() ||
                                            !src_params.has_scale_gamma_by_std_fl() || !src_params.has_scale_gamma_by_std_bw() ),
                EP067, "Precision Parameters should be mentioned for gamma, beta and gamma_by_std in Scale Layer : " << src.name() )

        layer.user_scale_gamma_bw = src_params.scale_gamma_bw();
        layer.user_scale_beta_bw = src_params.scale_beta_bw();
        layer.user_scale_gamma_by_std_bw = src_params.scale_gamma_by_std_bw();
        layer.user_scale_gamma_fl = src_params.scale_gamma_fl();
        layer.user_scale_beta_fl = src_params.scale_beta_fl();
        layer.user_scale_gamma_by_std_fl = src_params.scale_gamma_by_std_fl();

        DynamicFPToHWFormat(SCALE_BW, layer.user_scale_gamma_bw, layer.user_scale_gamma_fl, layer.scale_gamma_bw, layer.scale_gamma_fl, false);
        DynamicFPToHWFormat(SCALE_BW, layer.user_scale_beta_bw, layer.user_scale_beta_fl, layer.scale_beta_bw, layer.scale_beta_fl, false);
        DynamicFPToHWFormat(SCALE_BW, layer.user_scale_gamma_by_std_bw, layer.user_scale_gamma_by_std_fl, layer.scale_gamma_by_std_bw, layer.scale_gamma_by_std_fl, false);
    }

/*    cout << "[DPxxx] " << src.name() << " user : [ " <<  layer.user_ip_bw << ":" << layer.user_ip_fl << " "
                                            <<  layer.user_op_bw << ":" << layer.user_op_fl << " "
                                            <<  layer.user_wt_bw << ":" << layer.user_wt_fl << " ]" << endl;;
    cout << "[DPxxx] " << src.name() << " HW : [ " <<  layer.ip_bw << ":" << layer.ip_fl << " "
                                            <<  layer.op_bw << ":" << layer.op_fl << " "
                                            <<  layer.wt_bw << ":" << layer.wt_fl << " ]" << endl;
    cout << "[DPxxx] " << src.name() << " user BN : [ " << layer.user_bn_mean_bw << ":" << layer.user_bn_mean_fl << " " 
                                                     << layer.user_bn_variance_bw << ":" << layer.user_bn_variance_fl << endl;
    cout << "[DPxxx] " << src.name() << " HW BN: [ " << layer.bn_mean_bw << ":" << layer.bn_mean_fl << " " 
                                                     << layer.bn_variance_bw << ":" << layer.bn_variance_fl << endl;
    cout << "[DPxxx] " << src.name() << " user SCALE: [ " << layer.user_scale_gamma_bw << ":" << layer.user_scale_gamma_fl << " " 
                                                     << layer.user_scale_beta_bw << ":" << layer.user_scale_beta_fl << endl;
    cout << "[DPxxx] " << src.name() << " HW SCALE: [ " << layer.scale_gamma_bw << ":" << layer.scale_gamma_fl << " " 
                                                     << layer.scale_beta_bw << ":" << layer.scale_beta_fl << endl;
    cout << "[DPxxx] " << src.name() << " HW STD: [ " << layer.user_scale_gamma_by_std_bw << ":" << layer.user_scale_gamma_by_std_fl << " " 
                                                     << layer.scale_gamma_by_std_bw << ":" << layer.scale_gamma_by_std_fl<< endl;
*/
}

// TODO : @ARK : This function will have some changes when we support the latest SSD
// layerWidth, layerHeight : Width and Height of input feature map
// imageWidth, imageHeight : Width and Height of original input image
// aspect_ratios : Aspect ratios to be considered (float vector)
// min_size : minimum size to be specified (float vector)
// max_size : maximum size to be specified (float vector : optional )
// flip : Whether aspect_ratio to be flipped (bool : optional : default = True)
// clip : Whether output to be clipped [0, 1] (bool : optional : default = True)
// step_h, step_w : in simple words, ratio of imageWidth/layerWidth (float : optional : default = -1.0)
// offset : Used in priorbox calculation (float : optional : default = 0.0)
vector<float> computePriorBoxes(int layerWidth, int layerHeight, int imageWidth, int imageHeight, 
                            const vector<float>& aspect_ratios_, const vector<float>& min_size, const vector<float>& max_size, const vector<float>& var,
                            bool flip, bool clip, float step_h_, float step_w_, float offset_)
{
    const int layer_width   = layerWidth;
    const int layer_height  = layerHeight;
    const int img_width     = imageWidth;
    const int img_height    = imageHeight;
    const float step_w      = (step_w_ == 0) ? static_cast<float>(img_width)  / layer_width  : step_w_ ;
    const float step_h      = (step_h_ == 0) ? static_cast<float>(img_height) / layer_height : step_h_ ;

    // Update aspect ratios with their reciprocals
    // No check for duplicate value
    vector<float> aspect_ratios;
    aspect_ratios.push_back(1.);
    int num_priors = 1;

    for (int i=0; i<aspect_ratios_.size(); i++)
    {
        float ar = aspect_ratios_[i];
        aspect_ratios.push_back(ar);
        num_priors++;
        if (flip)
        {
            aspect_ratios.push_back(1.0 / ar);
            num_priors++;
        }
    }

    if((max_size.size() > 0) && (max_size[0] > 0))
    {
        if(max_size[0] > min_size[0])
        {
            num_priors++;
        }
        else
        {
            cerr << "[EP054] max_size should be larger than min_size for PriorBox layer : " << max_size[0] << " v/s " << min_size[0] << endl;
            exit(-1);
        }
    }
    

    // Now we start creating the boxes

    int nboxes  = layer_height * layer_width * num_priors;      // Total number of boxes
    int dim     = nboxes * 4;                                   // Total size for boxes
    vector<float> priorboxes(2 * dim);                          // 2 * dim for [prior_box & variance]
    int idx = 0;

    
    for (int h = 0; h < layer_height; ++h) 
    {
        for (int w = 0; w < layer_width; ++w) 
        {
            float center_x = (w + offset_) * step_w;
            float center_y = (h + offset_) * step_h;
            float box_width, box_height;

            // first prior: aspect_ratio = 1, size = min_size
            box_width = box_height = (float)min_size[0];
            priorboxes[idx++] = (center_x - box_width / 2.) / img_width;    // xmin
            priorboxes[idx++] = (center_y - box_height / 2.) / img_height;  // ymin
            priorboxes[idx++] = (center_x + box_width / 2.) / img_width;    // xmax
            priorboxes[idx++] = (center_y + box_height / 2.) / img_height;  // ymax

            // second prior: aspect_ratio = 1, size = sqrt(min_size * max_size)     
            if((max_size.size() > 0) && (max_size[0] > 0))
            {
                box_width = box_height = sqrt(min_size[0] * max_size[0]);           
                priorboxes[idx++] = (center_x - box_width / 2.) / img_width;    // xmin
                priorboxes[idx++] = (center_y - box_height / 2.) / img_height;  // ymin
                priorboxes[idx++] = (center_x + box_width / 2.) / img_width;    // xmax    
                priorboxes[idx++] = (center_y + box_height / 2.) / img_height;  // ymax
            }

            // rest of priors
            for (int r = 0; r < aspect_ratios.size(); ++r) 
            {
                float ar = aspect_ratios[r];
                // Check if selected ar is 1.0 which is already processed
                if (fabs(ar - 1.) < 1e-6) 
                {
                    continue;
                }
                box_width     = min_size[0] * sqrt(ar);
                box_height    = min_size[0] / sqrt(ar);
                priorboxes[idx++] = (center_x - box_width / 2.) / img_width;  // xmin
                priorboxes[idx++] = (center_y - box_height / 2.) / img_height;// ymin
                priorboxes[idx++] = (center_x + box_width / 2.) / img_width;  // xmax
                priorboxes[idx++] = (center_y + box_height / 2.) / img_height;// ymax
            }
        }
    }

    // Clip the boxes if required
    if(clip) 
    {
        for (int d = 0; d < priorboxes.size()/2; ++d) 
        {
            priorboxes[d] = std::min<float>(std::max<float>(priorboxes[d], 0.), 1.);
        }
    }

    // Fill the variance
    for(int i=0; i<nboxes; i++)
    {
        std::copy(var.begin(), var.end(), priorboxes.begin() + idx);
        idx += 4;
    }

    return priorboxes;
}

// An utility function to saturate & round the data to specified format
// bw = total bitwidth (including sign-bit)
// fl = fractional bits
// rounding = Rounding Method (TODO : @ARK : Only NEAREST is supported, STOCHASTIC is suppressed)
void Trim2FixedPoint(vector<float>& data, const int bw, const int fl, RoundingMethod rounding )
{
    float pow_bw_minus_1 = pow(2, bw-1);
    float pow_minus_fl = pow(2, -fl);
    float pow_plus_fl = pow(2, fl);
    float max_data = (pow_bw_minus_1 - 1) * pow_minus_fl;
    float min_data = -pow_bw_minus_1 * pow_minus_fl;

    for (int index = 0; index < data.size(); ++index)
    {
        // Saturate data
        data[index] = std::max(std::min(data[index], max_data), min_data);

        // Round data
        data[index] /= pow_minus_fl;
        switch (rounding) {
            case ROUND_NEAREST:
                data[index] = round(data[index]);
                break;
            // case ROUND_STOCHASTIC:
            //     data[index] = floor(data[index] + RandUniform_cpu());
            //     break;
            default:
                break;
        }
        data[index] *= pow_minus_fl;
    }
}

// An utility function to convert DynamicFP precision format to HW precision format
void DynamicFPToHWFormat(const int BW, int r_bw, int r_fl, int& hw_bw, int& hw_fl, bool IO_format)
{
    string type = IO_format ? "I/O feature maps" : "parameters"; 
    int tmp_Q = r_bw - r_fl;
    int tmp_fl = std::max(0, r_fl);                        // if fl is negative, clip it to zero.

    // Q+F < BW
    ELOG( tmp_Q + tmp_fl > BW, EP055,
          "Total bit-width required for " << type << " is more than what is supported in the HW : " << tmp_Q+tmp_fl << " v/s " << BW);
    
    int diff = BW - (tmp_Q + tmp_fl);

    // Behaviour is different for weights and I/O formats
    // Special care should be taken care for I/O formats
    // If Q.F is the format obtained from DynamicFP, I/O needs Q+1:F-1, but weights still needs Q:F itself
    hw_fl = IO_format ? tmp_fl - 1: tmp_fl ;
    //hw_fl = IO_format ? tmp_fl: tmp_fl ;
    hw_fl = std::max(0, hw_fl);
    hw_fl += diff;
    hw_bw = BW;
}

// An utility function to convert DynamicFP precision format to HW precision format
void DynamicFPToHWFormat_fixedFL(const int BW, int r_bw, int r_fl, int& hw_bw, int& hw_fl, bool IO_format)
{
    string type = IO_format ? "I/O feature maps" : "parameters"; 
    int tmp_Q = r_bw - r_fl;
    int tmp_fl = std::max(0, r_fl);                        // if fl is negative, clip it to zero.

    // Q+F < BW
    ELOG( tmp_Q + tmp_fl > BW, EP055,
          "Total bit-width required for " << type << " is more than what is supported in the HW : " << tmp_Q+tmp_fl << " v/s " << BW);

    // Behaviour is different for weights and I/O formats
    // Special care should be taken care for I/O formats
    // If Q.F is the format obtained from DynamicFP, I/O needs Q+1:F-1, but weights still needs Q:F itself
    // FIXME : If user gives 16:0, this will become 17:-1
    hw_fl = IO_format ? tmp_fl - 1: tmp_fl ;
    hw_bw = BW;
}

// An utility function to convert DynamicFP precision format to HW precision format
void DynamicFPToHWFormat_deprecated(const int BW, int r_bw, int r_fl, int& hw_bw, int& hw_fl, bool IO_format)
{
    // Behaviour is different for weights and I/O formats
    // Special care should be taken care for I/O formats
    // If Q.F is the format obtained from DynamicFP, I/O needs Q+1:F-1, but weights still needs Q:F itself
    string type = IO_format ? "I/O feature maps" : "parameters"; 
    int hw_Q = IO_format ? r_bw - r_fl + 1 : r_bw - r_fl ;
    if(hw_Q > BW)
    {
        cerr << "[EP055] Total bit-width required for " << type << " is more than what is supported in the HW : " << hw_Q << " v/s " << BW << endl;
        exit(-1);
    }

    hw_bw = BW;
    hw_fl = BW - hw_Q;
}

