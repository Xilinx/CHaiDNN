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
//------------------------------------- FUNCTION DEFINTIONS ----------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------//

XGraph* ParseCaffeNetwork(const string& deployFile, const string& caffemodelFile, const string& start_layer, const string& end_layer,
			    const string& rootFolder, const string& caffeMeanFile, FileMode file_mode)
{

    // ------------------------  GET CAFFE LAYER MAP ---------------------------- //
    fillCaffeLayerMap();
    fillSWLayerSet();

    // ------------------------  LOAD DEPLOY FILE ------------------------------- //

    // Create the Net for prototxt and load the deploy file to it
    caffe::NetParameter *Net = new caffe::NetParameter;
    readDeployFile(deployFile, Net);

    // #. Check if atleast one layer is there in the prototxt
    ASSERT( Net->layer_size() > 0, EP106, 
            "Network should contain atleast one layer. No layer is found in " << deployFile )

    // ------------------------ START EXTRACTING TO XGRAPH ------------------------- //

    // Create a XGraph
    XGraph* graph  = new XGraph();  
    string uniqname, username;

    // Setting up the location to save data
    if(rootFolder.empty())
        graph->saveDir = "data/";
    else
    {
        bool ends_with_slash = *(rootFolder.end()-1) == '/';
        string tmp_saveDir = ends_with_slash ? rootFolder + "data/" : rootFolder + "/data/";
        graph->saveDir = tmp_saveDir;
    }

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
        XBlob* tmp = new XBlob(username);
        tmp->shape = tmpShape;
        graph->blobs[username] = tmp;
        // graph->input_blob = nameIndex(tmp);
        graph->input_blob = username;
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

	XLayer* dst = new XLayer(iLayer.name(), iLayer.type());
        username = iLayer.top(0);
        XBlob* tmp = new XBlob(username);
        for(int i=0; i<blobshape.dim_size(); i++)
        {
            tmp->shape.push_back(blobshape.dim(i));
        }
	dst->input_params->dim = tmp->shape;
	graph->layers[dst->name] = dst;
    
	dst->top.push_back(nameIndex(tmp));			// Add top to layer top list
	tmp->producers.push_back(dst->name);                     // Add current layer to top Producers list
	tmp->producerDim.push_back(tmp->shape);           // Add output blob shape to producerDim
	dst->topShape.push_back(tmp->shape);                // Replicate top shape in layer also

        graph->blobs[username] = tmp;
        graph->input_blob = username;

        // We already finished with first layer. So increment the counter
        layer_start_index++; 
    }

    graph->start_layer = Net->layer(0).name();
    graph->end_layer = Net->layer(Net->layer_size()-1).name();
    if((end_layer.compare("")==0) || end_layer.compare(graph->end_layer)==0) 
	    graph->model_last_layer=true;
    else
	    graph->model_last_layer=false;
    

    for(int i=layer_start_index; i<Net->layer_size(); i++)
    {
        const caffe::LayerParameter& cLayer = Net->layer(i);
        
        // #. Check if the layer is a train-only layer
        bool is_layer_train_only = false;
        for(int j=0; j < cLayer.include_size(); ++j)
        {
            if(cLayer.include(j).has_phase() && cLayer.include(j).phase() == caffe::TRAIN)
            {
                is_layer_train_only = true;
            }
        }

        // #. Ignore a train-only layer and parse a test-layer
        if(is_layer_train_only)
        {
            cout << "[PARSE] Ignoring TRAIN-only layer : " << cLayer.name() << endl;
            continue;
        }
        else
        {
            cout << "[PARSE] Parsing " << cLayer.name() << endl;
        }

        // #. Check if it is an user-defined layer
        bool user_defined_layer = cLayer.user_defined_layer();

        // #. Check if the Layer is supported or not
        mapStrStr::const_iterator it = CaffeLayerMap.find(cLayer.type());

        // #. If the layer is not an user-defined layer and layer type doesn't 
        // match with any of the supported layers, EXIT.
        ELOG(   (!user_defined_layer) && (it == CaffeLayerMap.end()), EP009,
                "Unregistered Layer Type : " << cLayer.type() )

        // #. If the layer is an user-defined layer, but layer type is already
        // a supported layer type, EXIT.
        ELOG(   (user_defined_layer && (it != CaffeLayerMap.end())), EP067,
                "User-defined layer type: " << cLayer.type() << " is already a registered layer in CHaiDNN. "
                << "Please change the type of the layer : " << cLayer.name() )

        // #. Check if any other layer with same name already present in the XGraph
        ELOG(   graph->layers.find(cLayer.name()) != graph->layers.end(), EP010,
                "[EP010] Duplicate layer name detected : " << cLayer.name() )
    
        // #. If it is supported, parse all the params from Caffe Layer to XLayer in XGraph
        ExtractParameters(cLayer, *graph);

	if(cLayer.top_size() > 0)
	    graph->output_blob = cLayer.top()[0];
    } 
    
    // -----------------------  LOAD MEAN FILE -------------------------------------- //

    // Check if mean file path is provided
    // if(caffeMeanFile.empty())
    // {
    //     cerr << "[EP028] Mean file is mandatory. If the mean values are zeros, "
    //          << "please keep zeros in a txt file (equal to number of planes in image) and provide the path" << endl;
    //     exit(-1);
    // }

    if(!caffeMeanFile.empty())
    {
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
#if DEBUG_WEIGHT_EXTRACTION
        SAVEDATA(graph->meanData, graph->saveDir + "L_mean");
#endif
    }

    // ------------------------  PRUNE THE XGRAPH ------------------------------- //
    graph->prune(start_layer, end_layer);


    // ------------------------  LOAD CAFFEMODEL FILE ------------------------------- //

    // Create the Net for caffemodel file 
    caffe::NetParameter *binNet = new caffe::NetParameter();
    readModelFile(caffemodelFile, binNet);

    // DEBUG : Print all layer names and types in binary file
    // for(int i=0; i<binNet->layer_size(); ++i)
    // {
    //     cerr << binNet->layer(i).name() << " : " << binNet->layer(i).type() << endl;
    // }

    // Extract all the trained parameters and fill the fields in xgraph
    ExtractTrainedParameters(binNet, graph);

    // ----------------------- PRINT LAYERS WITHOUT PROPER PRECISION ------------------ //
    if(graph->precMissLayers.size() > 0)
    {
        cerr    << endl;
        cerr    << "[WARNING] Precision Parameters are not specified correctly for some of the layers." << '\n'
                << "So CHaiDNN assumes default values for those parameters. It might affect the accuracy of result." << '\n'
                << "For accurate results, please provide the correct parameters for following layers: " << '\n';

        for(int i=0; i<graph->precMissLayers.size(); ++i)
        {
            cerr << "\'" << graph->precMissLayers[i] << "\'" << "\t";
            if((i+1)%5 == 0)
                cerr << endl;
        }
        cerr << endl;
    }

    // -----------------------  CLEAR ALL MEMORY -------------------------------------- //
    // google::protobuf::ShutdownProtobufLibrary();
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
        const caffe::LayerParameter& cLayer = Net->layer(i);
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
        else if(xlayer_it->second->type == "XCustom")
        {
            extractXCustomTrainedData(graph, xlayer_it->second->name, Net, modelLayerIndex);
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
    const caffe::LayerParameter& binLayer  = Net->layer(loc);
    XLayer* tmpXlayer               = graph->layers[layerName];
    string txtFileName;

    // Extract the weights, do trimming & rounding before saving to TXT file
    cerr << "[IG001] Extracting " << tmpXlayer->name << " weights ... " << endl;
    vector<int> weightsShape = getBlobDim(binLayer.blobs(0));
    ELOG(  ((weightsShape.at(0) != tmpXlayer->conv_params->M) || 
            (weightsShape.at(1) != (tmpXlayer->conv_params->N/tmpXlayer->conv_params->group)) ||
            (weightsShape.at(2) != tmpXlayer->conv_params->filter_h) ||
            (weightsShape.at(3) != tmpXlayer->conv_params->filter_w)),
            EP056,
            "Convolution Layer: " << tmpXlayer->name << " - mismatch in filter shape. " 
            << TensorDimToString(weightsShape) << " v/s " << tmpXlayer->conv_params->filterDimToString()
        )

    vector<float> weights = extractBlobToVector(binLayer.blobs(0));
    tmpXlayer->conv_params->weights.push_back(weights);
    tmpXlayer->conv_params->weightsDim.push_back(getBlobDim(binLayer.blobs(0)));

#if DEBUG_WEIGHT_EXTRACTION
    string tmpName(tmpXlayer->name); 
    replace(tmpName.begin(), tmpName.end(), '/', '_');
    txtFileName = graph->saveDir + tmpName + "_weights";
    tmpXlayer->conv_params->weightsPath.push_back(txtFileName);
    int sizeInBytes = getSize(weightsShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << "\t";
	if(sizeInBytes > (14*1024*1024))
	{
		cerr << "Parsing large data, this may take a while ...";
	}
	cerr << endl;
    SAVEDATA(weights, txtFileName);
#endif
    
    // Extract the bias if bias is present, else save a vector filled with zeros
    cerr << "[IG001] Extracting " << tmpXlayer->name << " bias ... " << endl;
    vector<int> biasShape;
    vector<float> bias;
    if(tmpXlayer->conv_params->has_bias)
    {
        biasShape = getBlobDim(binLayer.blobs(1));
        ELOG( (biasShape.at(0) != tmpXlayer->conv_params->M) , EP031,
              "Deconv Layer: " << tmpXlayer->name << " - mismatch in bias shape. "
               << TensorDimToString(biasShape) << " v/s " << tmpXlayer->conv_params->M )

        bias = extractBlobToVector(binLayer.blobs(1));
    }
    else
    {
        int OFM = tmpXlayer->conv_params->M;
        bias = vector<float>(OFM, 0.0);
        biasShape.push_back(OFM);
    }

    tmpXlayer->conv_params->bias.push_back(bias);
    tmpXlayer->conv_params->biasDim.push_back(biasShape);

#if DEBUG_WEIGHT_EXTRACTION
    txtFileName = graph->saveDir + tmpName + "_bias";
    tmpXlayer->conv_params->biasPath.push_back(txtFileName);
    sizeInBytes = getSize(biasShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
    SAVEDATA(bias, txtFileName);
#endif

    // TODO : Move this to different loc
    // Assign default params
    if(tmpXlayer->th_params.size() == 0)
	tmpXlayer->th_params = getAbsMaxPerFilter<float>(tmpXlayer->conv_params->weights[0], tmpXlayer->conv_params->weightsDim[0]);
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
    const caffe::LayerParameter& binLayer  = Net->layer(loc);
    XLayer* tmpXlayer               = graph->layers[layerName];
    string txtFileName;

    // Extract the weights
    cerr << "[IG001] Extracting " << tmpXlayer->name << " weights ... " << endl;
    vector<int> weightsShape = getBlobDim(binLayer.blobs(0));
    ELOG(  ((weightsShape.at(0) != tmpXlayer->deconv_params->M) || 
            (weightsShape.at(1) != (tmpXlayer->deconv_params->N/tmpXlayer->deconv_params->group)) ||
            (weightsShape.at(2) != tmpXlayer->deconv_params->filter_h) ||
            (weightsShape.at(3) != tmpXlayer->deconv_params->filter_w)),
            EP057,
            "Deconvolution Layer: " << tmpXlayer->name << " - mismatch in filter shape. " 
            << TensorDimToString(weightsShape) << " v/s " << tmpXlayer->deconv_params->filterDimToString()
        )

    vector<float> weights = extractBlobToVector(binLayer.blobs(0));
    tmpXlayer->deconv_params->weights.push_back(weights);
    tmpXlayer->deconv_params->weightsDim.push_back(weightsShape);

#if DEBUG_WEIGHT_EXTRACTION
    string tmpName(tmpXlayer->name); 
    replace(tmpName.begin(), tmpName.end(), '/', '_');
    txtFileName = graph->saveDir + tmpName + "_weights";
    tmpXlayer->deconv_params->weightsPath.push_back(txtFileName);
    int sizeInBytes = getSize(weightsShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << "\t";
	if(sizeInBytes > (14*1024*1024))
	{
		cerr << "Parsing large data, this may take a while ...";
	}
	cerr << endl;
    SAVEDATA(weights, txtFileName);
#endif
    
    // Extract the bias if bias is present, else save a vector filled with zeros
    cerr << "[IG001] Extracting " << tmpXlayer->name << " weights ... " << endl;
    vector<float> bias;
    vector<int> biasShape;

    if(tmpXlayer->deconv_params->has_bias)
    {
        biasShape = getBlobDim(binLayer.blobs(1));
        ELOG( (biasShape.at(0) != tmpXlayer->deconv_params->M) , EP034,
              "Deconv Layer: " << tmpXlayer->name << " - mismatch in bias shape. "
               << TensorDimToString(biasShape) << " v/s " << tmpXlayer->deconv_params->M )

        bias = extractBlobToVector(binLayer.blobs(1));
    }
    else
    {
        int OFM = tmpXlayer->deconv_params->M;
        bias = vector<float>(OFM, 0.0);
        biasShape.push_back(OFM);
    }
    tmpXlayer->deconv_params->bias.push_back(bias);
    tmpXlayer->deconv_params->biasDim.push_back(biasShape);

#if DEBUG_WEIGHT_EXTRACTION
    txtFileName = graph->saveDir + tmpName + "_bias";
    tmpXlayer->deconv_params->biasPath.push_back(txtFileName);
    sizeInBytes = getSize(biasShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
    SAVEDATA(bias, txtFileName);
#endif
    
    // Assign default params
    if(tmpXlayer->quantization_scheme == "Xilinx" &&  tmpXlayer->th_params.size() == 0)
	tmpXlayer->th_params = getAbsMaxPerFilter<float>(tmpXlayer->deconv_params->weights[0], tmpXlayer->deconv_params->weightsDim[0]);
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
    const caffe::LayerParameter& binLayer  = Net->layer(loc);
    XLayer* tmpXlayer               = graph->layers[layerName];
    string txtFileName;

    // Extract the weights

    // Check if filter shape is matching
    cerr << "[IG001] Extracting " << tmpXlayer->name << " weights ... " << endl;
    vector<int> weightsShape = getBlobDim(binLayer.blobs(0));
    ELOG( (weightsShape.at(0) != tmpXlayer->fc_params->M) || (weightsShape.at(1) != tmpXlayer->fc_params->N), EP035,
          "FC Layer: " << tmpXlayer->name << " - mismatch in filter shape. " 
           << TensorDimToString(weightsShape) << " v/s " << tmpXlayer->fc_params->M << "x" << tmpXlayer->fc_params->N);

    vector<float> weights = extractBlobToVector(binLayer.blobs(0));
    tmpXlayer->fc_params->weights.push_back(weights);
    tmpXlayer->fc_params->weightsDim.push_back(weightsShape);

#if DEBUG_WEIGHT_EXTRACTION
    string tmpName(tmpXlayer->name); 
    replace(tmpName.begin(), tmpName.end(), '/', '_');
    txtFileName = graph->saveDir + tmpName + "_weights";
    tmpXlayer->fc_params->weightsPath.push_back(txtFileName);
    int sizeInBytes = getSize(weightsShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << "\t";
	if(sizeInBytes > (14*1024*1024))
	{
		cerr << "Parsing large data, this may take a while ...";
	}
	cerr << endl;
    SAVEDATA(weights, txtFileName);
#endif
    
    // Extract the bias if bias is present, else save a vector filled with zeros
    cerr << "[IG001] Extracting " << tmpXlayer->name << " bias ... " << endl;
    vector<int> biasShape;
    vector<float> bias;
    if(tmpXlayer->fc_params->has_bias)
    {
        // Check if bias shape is matching
        biasShape = getBlobDim(binLayer.blobs(1));
        ELOG( (biasShape.at(0) != tmpXlayer->fc_params->M) , EP036,
              "FC Layer: " << tmpXlayer->name << " - mismatch in bias shape. "
               << TensorDimToString(biasShape) << " v/s " << tmpXlayer->fc_params->M );

        bias = extractBlobToVector(binLayer.blobs(1));
    }
    else
    {
        int OFM = tmpXlayer->fc_params->M;
        bias = vector<float>(OFM, 0.0);
        biasShape.push_back(OFM);
    }
    tmpXlayer->fc_params->bias.push_back(bias);
    tmpXlayer->fc_params->biasDim.push_back(biasShape);

#if DEBUG_WEIGHT_EXTRACTION
    txtFileName = graph->saveDir + tmpName + "_bias";
    tmpXlayer->fc_params->biasPath.push_back(txtFileName);
    sizeInBytes = getSize(biasShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
    SAVEDATA(bias, txtFileName);
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
    const caffe::LayerParameter& binLayer  = Net->layer(loc);
    XLayer* tmpXlayer               = graph->layers[layerName];
    string txtFileName;

    // Extract the weights
    cerr << "[IG001] Extracting " << tmpXlayer->name << " weights ... " << endl;
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

#if DEBUG_WEIGHT_EXTRACTION
    string tmpName(tmpXlayer->name); 
    replace(tmpName.begin(), tmpName.end(), '/', '_');
    txtFileName = graph->saveDir + tmpName + "_weights";
    tmpXlayer->l2norm_params->gammaFile = txtFileName;
    int sizeInBytes = tmpXlayer->l2norm_params->gamma.size() * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
    SAVEDATA(tmpXlayer->l2norm_params->gamma, txtFileName);
#endif

    // If th_l2n_gamma not provided set default values
    if (tmpXlayer->quantization_scheme == "Xilinx" && tmpXlayer->th_l2n_gamma.empty()) {
	tmpXlayer->th_params = tmpXlayer->l2norm_params->gamma;
    }

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
    const caffe::LayerParameter& binLayer  = Net->layer(loc);
    XLayer* tmpXlayer               = graph->layers[layerName];
    string txtFileName;

    // Extract MWA
    cerr << "[IG001] Extracting " << tmpXlayer->name << " weights ... " << endl;
    vector<float> mwa = extractBlobToVector(binLayer.blobs(2));
    float scaling_factor = mwa[0] == 0 ? 0 : 1.0f/mwa[0];

    // Extract the mean, do trimming & rounding before saving to TXT file
    vector<int> weightsShape = getBlobDim(binLayer.blobs(0));
    int channels = tmpXlayer->topShape.at(0).at(1);               // Number of feature maps
    ASSERT( (weightsShape.at(0) == channels), EP056,
            "BatchNorm Layer: " << tmpXlayer->name << "mismatch in Mean shape : " 
            << TensorDimToString(weightsShape) << " != " << channels)
    vector<float> weights = extractBlobToVector(binLayer.blobs(0));
    for(int i=0; i<weights.size(); ++i)
        weights[i] *= scaling_factor;
    tmpXlayer->batchnorm_params->mean.push_back(weights);
    tmpXlayer->batchnorm_params->meanDim.push_back(weightsShape);

#if DEBUG_WEIGHT_EXTRACTION
    string tmpName(tmpXlayer->name); 
    replace(tmpName.begin(), tmpName.end(), '/', '_');
    txtFileName = graph->saveDir + tmpName + "_mean";
    tmpXlayer->batchnorm_params->meanPath.push_back(txtFileName);
    int sizeInBytes = getSize(weightsShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << "\t";
	if(sizeInBytes > (14*1024*1024))
	{
		cerr << "Parsing large data, this may take a while ...";
	}
	cerr << endl;
    SAVEDATA(weights, txtFileName);
#endif
    
    // Extract the variance, do trimming & rounding before saving to TXT file
    vector<int> biasShape = getBlobDim(binLayer.blobs(1));
    ASSERT( (biasShape.at(0) == channels), EP056,
            "BatchNorm Layer: " << tmpXlayer->name << "mismatch in variance shape : " 
            << TensorDimToString(biasShape) << " != " << channels)
    vector<float> bias = extractBlobToVector(binLayer.blobs(1));
    for(int i=0; i<bias.size(); ++i)
        bias[i] *= scaling_factor;
    tmpXlayer->batchnorm_params->variance.push_back(bias);
    tmpXlayer->batchnorm_params->varianceDim.push_back(biasShape);

#if DEBUG_WEIGHT_EXTRACTION
    txtFileName = graph->saveDir + tmpName + "_variance";
    tmpXlayer->batchnorm_params->variancePath.push_back(txtFileName);
    sizeInBytes = getSize(biasShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << "\t";
	if(sizeInBytes > (14*1024*1024))
	{
		cerr << "Parsing large data, this may take a while ...";
	}
	cerr << endl;
    SAVEDATA(bias, txtFileName);
#endif

    // If th_bn_mean not provided set default values
    if (tmpXlayer->quantization_scheme == "Xilinx" && tmpXlayer->th_bn_mean.empty()) {
	tmpXlayer->th_bn_mean = tmpXlayer->batchnorm_params->mean.at(0);
    }
    // If th_bn_variance not provided set default values
    if (tmpXlayer->quantization_scheme == "Xilinx" && tmpXlayer->th_bn_variance.empty()) {
	tmpXlayer->th_bn_variance = tmpXlayer->batchnorm_params->variance.at(0);
    }
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
    const caffe::LayerParameter& binLayer  = Net->layer(loc);
    XLayer* tmpXlayer               = graph->layers[layerName];
    string txtFileName;

    // Extract gamma 
    cerr << "[IG001] Extracting " << tmpXlayer->name << " weights ... " << endl;

    // Check if filter shape is matching
    vector<int> weightsShape = getBlobDim(binLayer.blobs(0));
    int channels = tmpXlayer->topShape.at(0).at(1);               // Number of feature maps
    ASSERT( (weightsShape.at(0) == channels), EP057,
            "Scale Layer: " << tmpXlayer->name << "mismatch in Scale shape : " 
            << TensorDimToString(weightsShape) << " != " << channels)
    vector<float> weights = extractBlobToVector(binLayer.blobs(0));
    tmpXlayer->scale_params->gamma.push_back(weights);
    tmpXlayer->scale_params->gammaDim.push_back(weightsShape);

#if DEBUG_WEIGHT_EXTRACTION
    string tmpName(tmpXlayer->name); 
    replace(tmpName.begin(), tmpName.end(), '/', '_');
    txtFileName = graph->saveDir + tmpName + "_gamma";
    tmpXlayer->scale_params->gammaPath.push_back(txtFileName);
    int sizeInBytes = getSize(weightsShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << "\t";
	if(sizeInBytes > (14*1024*1024))
	{
		cerr << "Parsing large data, this may take a while ...";
	}
	cerr << endl;
    SAVEDATA(weights, txtFileName);
#endif
    
    // Extract the beta if beta is present, else save a vector filled with zeros
    vector<float> bias;
    vector<int> biasShape;

    if(tmpXlayer->scale_params->has_bias)
    {
        // Check if bias shape is matching
        vector<int> biasShape = getBlobDim(binLayer.blobs(1));
        ASSERT( (biasShape.at(0) == channels), EP060,
                "Scale Layer: " << tmpXlayer->name << "mismatch in Bias shape : " 
                << TensorDimToString(weightsShape) << " != " << channels)

        bias = extractBlobToVector(binLayer.blobs(1));
    }
    else
    {
        int OFM = channels;
        bias = vector<float>(OFM, 0.0);
        biasShape.push_back(OFM);
    }
    tmpXlayer->scale_params->beta.push_back(bias);
    tmpXlayer->scale_params->betaDim.push_back(biasShape);

#if DEBUG_WEIGHT_EXTRACTION
    txtFileName = graph->saveDir + tmpName + "_beta";
    tmpXlayer->scale_params->betaPath.push_back(txtFileName);
    sizeInBytes = getSize(biasShape) * sizeof(float);
    cerr << "[IG001] Saving " << txtFileName << " (" << humanReadableSize(sizeInBytes) << ")" << endl;
    SAVEDATA(bias, txtFileName);
#endif

    // If th_scale_gamma not provided set default values
    if (tmpXlayer->quantization_scheme == "Xilinx" && tmpXlayer->th_scale_gamma.empty()) {
	tmpXlayer->th_scale_gamma = tmpXlayer->scale_params->gamma.at(0);
    }
    // If th_bn_variance not provided set default values
    if (tmpXlayer->quantization_scheme == "Xilinx" && tmpXlayer->th_scale_beta.empty()) {
	tmpXlayer->th_scale_beta = tmpXlayer->scale_params->beta.at(0);
    }
}

void extractXCustomTrainedData(XGraph* graph, const string& layerName, const caffe::NetParameter* Net,
                                    const map<string, int>* layerIndex)
{
    XLayer* tmpXlayer               = graph->layers[layerName];
    map<string, int>::const_iterator modelLayer_it = layerIndex->find(layerName);

    // If the layer name not found in binFile, 
    //     If it is a supported layer, throw error
    //     Else if it is a custom layer, return

    if(modelLayer_it == layerIndex->end())
    {
        if(tmpXlayer->type != "XCustom")
        {
            cerr << "[EP035] Layer " << layerName << " is not found in the caffemodel file. " << endl;
            exit(-1);
        }
        else
        {
            return;
        }
    }
    
    // All is well, Start extracting
    // Get the layer from caffemodel
    int loc = modelLayer_it->second;
    const caffe::LayerParameter& binLayer  = Net->layer(loc);
    LOG(   binLayer.type() != tmpXlayer->xcustom_params->type, EP074,
            "Layer types not matching in deploy file and caffemodel file for user_defined_layer: " 
            << layerName << " : " << binLayer.type() << " v/s " << tmpXlayer->xcustom_params->type )

    for(int i=0; i<binLayer.blobs_size(); ++i)
    {
        tmpXlayer->xcustom_params->params.push_back(extractBlobToVector(binLayer.blobs(i)));
        tmpXlayer->xcustom_params->params_dims.push_back(getBlobDim(binLayer.blobs(i)));
    }
}

// A function to return the dimension of a caffe blob
vector<int> getBlobDim(const caffe::BlobProto& blob)
{
    vector<int> dim;                                                                        // To store the dimension
    caffe::BlobShape blobshape = blob.shape();
    // dim.reserve(blobshape.dim_size());
    // copy(blobshape.dim().begin(), blobshape.dim().end(), std::back_inserter(dim));
    ASSERT(blobshape.dim_size() > 0, EP167, "Blob Shape can't be 0")
    dim.insert(dim.end(), blobshape.dim().begin(), blobshape.dim().end());
    return dim;
}

// A function to dump the the caffe blob to a txt file
void saveBlob(const caffe::BlobProto& blob, const string& filepath)
{
    vector<float> weights;                                                                      // To store the weights/bias
    weights.reserve(blob.data_size());

    // copy the data.
    copy(blob.data().begin(), blob.data().end(), std::back_inserter(weights));    

    // Save data
    SAVEDATA(weights, filepath);
}

// A function to dump the the caffe blob to a vector
vector<float> extractBlobToVector(const caffe::BlobProto& blob)
{
    vector<float> weights;                                                                      // To store the weights/bias
    // weights.reserve(blob.data_size());
    // copy(blob.data().begin(), blob.data().end(), std::back_inserter(weights));    

    // copy the  data itself.
    weights.insert(weights.end(), blob.data().begin(), blob.data().end());

    return weights;
}

// Function that extract all the parameter from Caffe Layer,
// put it in a XLayer and Register it to XGraph
// TODO : @ARK : Use switch-case to get rid of long if-else-if chain
void ExtractParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    mapStrStr::const_iterator map_it = CaffeLayerMap.find(src.type());
    bool user_defined_layer = src.user_defined_layer();

    if (user_defined_layer)
    {
        ExtractXCustomParameters(src, graph);
    }
    else if (map_it->second == "Convolution")
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

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
    if (src_parameter.has_kernel_h() || src_parameter.has_kernel_w())
    {
        ELOG ( (src_parameter.kernel_size_size() > 0),
                EP151,
                "Mention either kernel_size or kernel_h/kernel_w for layer " << src.name() << ". Not both. !!")
        ASSERT((src_parameter.has_kernel_h() && src_parameter.has_kernel_w()),
                EP152,
                "Mention both kernel_h and kernel_w for layer " << src.name() << ". Or use kernel_size.")
        dst->conv_params->filter_h = src_parameter.kernel_h();
        dst->conv_params->filter_w = src_parameter.kernel_w();
    }
    else
    {
        ASSERT((src_parameter.kernel_size_size() > 0),
                EP153, "kernel_size is not specified in the convolution layer : " << src.name()) 
        dst->conv_params->filter_h = src_parameter.kernel_size(0);
        dst->conv_params->filter_w = src_parameter.kernel_size(0);
    }

    // Check if filters are not rectangular
    ASSERT( (dst->conv_params->filter_h == dst->conv_params->filter_w),
            EP154, "This version supports only square filters for convolution layer: " << src.name() )
    
    // Get PAD [DEFAULT = 0]
    if (src_parameter.has_pad_h() || src_parameter.has_pad_w())
    {
        ELOG ( (src_parameter.pad_size() > 0),
                EP155,
                "Mention either pad or pad_h/pad_w for layer " << src.name() << ". Not both. !!")
        ASSERT((src_parameter.has_pad_h() && src_parameter.has_pad_w()),
                EP156,
                "Mention both pad_h and pad_w for layer " << src.name() << ". Or use pad.")
        dst->conv_params->pad_h = src_parameter.pad_h();
        dst->conv_params->pad_w = src_parameter.pad_w();
    }
    else
    {
        bool pad_specified = src_parameter.pad_size() > 0;
        dst->conv_params->pad_h = pad_specified ? src_parameter.pad(0) : 0;
        dst->conv_params->pad_w = pad_specified ? src_parameter.pad(0) : 0;
    }

    // Check if pad is same in all directions
    ASSERT( (dst->conv_params->pad_h == dst->conv_params->pad_w),
            EP157, "This version supports only same pad in all directions for convolution layer: " << src.name() )

    // Get STRIDE [DEFAULT = 1]
    // TODO : Currently "stride" is supported. Support "stride_h/w" in future
    dst->conv_params->stride_h = src_parameter.stride_size() > 0 ? src_parameter.stride(0) : 1;
    dst->conv_params->stride_w = src_parameter.stride_size() > 0 ? src_parameter.stride(0) : 1;

    if (src_parameter.has_stride_h() || src_parameter.has_stride_w())
    {
        ELOG ( (src_parameter.stride_size() > 0),
                EP158,
                "Mention either stride or stride_h/stride_w for layer " << src.name() << ". Not both. !!")
        ASSERT((src_parameter.has_stride_h() && src_parameter.has_stride_w()),
                EP159,
                "Mention both stride_h and stride_w for layer " << src.name() << ". Or use stride.")
        dst->conv_params->stride_h = src_parameter.stride_h();
        dst->conv_params->stride_w = src_parameter.stride_w();
    }
    else
    {
        bool stride_specified = src_parameter.stride_size() > 0;
        dst->conv_params->stride_h = stride_specified ? src_parameter.stride(0) : 1;
        dst->conv_params->stride_w = stride_specified ? src_parameter.stride(0) : 1;
    }

    // Check if stride is same in all directions
    ASSERT( (dst->conv_params->stride_h == dst->conv_params->stride_w),
            EP160, "This version supports only same stride in all directions for convolution layer: " << src.name() )

    // Get DILATION [DEFAULT = 1]
    // TODO : Currently same dilation is done on all dimensions. But caffe support different dilation in different axis
    dst->conv_params->dilation = src_parameter.dilation_size() > 0 ? src_parameter.dilation(0) : 1;

    // Get GROUP[DEFAULT = 1]
    dst->conv_params->group = src_parameter.has_group() ? src_parameter.group() : 1;

    // Get HAS_BIAS[DEFAULT = 1]
    dst->conv_params->has_bias = src_parameter.has_bias_term() ? src_parameter.bias_term() : 1;

    // Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst);
    if(!success)
        graph.precMissLayers.push_back(src.name());

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
    XBlob* tmpTop = new XBlob(src.top(0));
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

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
    if (src_parameter.has_kernel_h() || src_parameter.has_kernel_w())
    {
        ELOG ( (src_parameter.kernel_size_size() > 0),
                EP151,
                "Mention either kernel_size or kernel_h/kernel_w for layer " << src.name() << ". Not both. !!")
        ASSERT((src_parameter.has_kernel_h() && src_parameter.has_kernel_w()),
                EP152,
                "Mention both kernel_h and kernel_w for layer " << src.name() << ". Or use kernel_size.")
        dst->deconv_params->filter_h = src_parameter.kernel_h();
        dst->deconv_params->filter_w = src_parameter.kernel_w();
    }
    else
    {
        ASSERT((src_parameter.kernel_size_size() > 0),
                EP153, "kernel_size is not specified in the Deconvolution layer : " << src.name()) 
        dst->deconv_params->filter_h = src_parameter.kernel_size(0);
        dst->deconv_params->filter_w = src_parameter.kernel_size(0);
    }

    // Check if filters are not rectangular
    ASSERT( (dst->deconv_params->filter_h == dst->deconv_params->filter_w),
            EP154, "This version supports only square filters for Deconvolution layer: " << src.name() )
    
    // Get PAD [DEFAULT = 0]
    if (src_parameter.has_pad_h() || src_parameter.has_pad_w())
    {
        ELOG ( (src_parameter.pad_size() > 0),
                EP155,
                "Mention either pad or pad_h/pad_w for layer " << src.name() << ". Not both. !!")
        ASSERT((src_parameter.has_pad_h() && src_parameter.has_pad_w()),
                EP156,
                "Mention both pad_h and pad_w for layer " << src.name() << ". Or use pad.")
        dst->deconv_params->pad_h = src_parameter.pad_h();
        dst->deconv_params->pad_w = src_parameter.pad_w();
    }
    else
    {
        bool pad_specified = src_parameter.pad_size() > 0;
        dst->deconv_params->pad_h = pad_specified ? src_parameter.pad(0) : 0;
        dst->deconv_params->pad_w = pad_specified ? src_parameter.pad(0) : 0;
    }

    // Check if pad is same in all directions
    ASSERT( (dst->deconv_params->pad_h == dst->deconv_params->pad_w),
            EP157, "This version supports only same pad in all directions for Deconvolution layer: " << src.name() )

    // Get STRIDE [DEFAULT = 1]
    if (src_parameter.has_stride_h() || src_parameter.has_stride_w())
    {
        ELOG ( (src_parameter.stride_size() > 0),
                EP158,
                "Mention either stride or stride_h/stride_w for layer " << src.name() << ". Not both. !!")
        ASSERT((src_parameter.has_stride_h() && src_parameter.has_stride_w()),
                EP159,
                "Mention both stride_h and stride_w for layer " << src.name() << ". Or use stride.")
        dst->deconv_params->stride_h = src_parameter.stride_h();
        dst->deconv_params->stride_w = src_parameter.stride_w();
    }
    else
    {
        bool stride_specified = src_parameter.stride_size() > 0;
        dst->deconv_params->stride_h = stride_specified ? src_parameter.stride(0) : 1;
        dst->deconv_params->stride_w = stride_specified ? src_parameter.stride(0) : 1;
    }

    // Check if stride is same in all directions
    ASSERT( (dst->deconv_params->stride_h == dst->deconv_params->stride_w),
            EP160, "This version supports only same stride in all directions for Deconvolution layer: " << src.name() )

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
    XBlob* tmpTop = new XBlob(src.top(0));
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
    
    // Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst);
    if(!success)
        graph.precMissLayers.push_back(src.name());
}


void ExtractDropoutParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::DropoutParameter src_parameter = src.dropout_param();
    mapStrStr::const_iterator type_it = CaffeLayerMap.find(src.type());
    string xlayerType = type_it->second;

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Check if it is inPlace.
    dst->dropout_params->inPlace = src.bottom(0) == src.top(0) ? true : false; 
    dst->inPlace = src.bottom(0) == src.top(0) ? true : false; 

    // Get Dropout Ratio
    dst->dropout_params->dropout_ratio           =  src_parameter.dropout_ratio();

    // Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst);
    if(!success)
        graph.precMissLayers.push_back(src.name());

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
        XBlob* tmpTop = new XBlob(src.top(0));
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Check if it is inPlace.
    dst->relu_params->inPlace = src.bottom(0) == src.top(0) ? true : false; 
    dst->inPlace = src.bottom(0) == src.top(0) ? true : false; 

    // Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst);
    if(!success)
        graph.precMissLayers.push_back(src.name());

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
        XBlob* tmpTop = new XBlob(src.top(0));
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

    // Get POOLING_TYPE [OPTIONAL, caffe default = MAX]
    dst->pool_params->PoolType = (src_parameter.has_pool()) ? (PoolingType)src_parameter.pool() : MAX;    

    // Check GLOBAL POOLING
    bool global_pooling = (src_parameter.has_global_pooling() && src_parameter.global_pooling() == true) ? true : false;

    // Get FILTER SIZE [MANDATORY if it is not a GLOBAL POOLING]
    if(global_pooling == false)
    {
        if (src_parameter.has_kernel_h() || src_parameter.has_kernel_w())
        {
            ELOG ( (src_parameter.has_kernel_size()),
                    EP161,
                    "Mention either kernel_size or kernel_h/kernel_w for layer " << src.name() << ". Not both. !!")
            ASSERT((src_parameter.has_kernel_h() && src_parameter.has_kernel_w()),
                    EP162,
                    "Mention both kernel_h and kernel_w for layer " << src.name() << ". Or use kernel_size.")
            dst->pool_params->kernel_h = src_parameter.kernel_h();
            dst->pool_params->kernel_w = src_parameter.kernel_w();
        }
        else
        {
            ASSERT((src_parameter.has_kernel_size()),
                    EP163, "kernel_size is not specified in the Pooling layer : " << src.name()) 
            dst->pool_params->kernel_h = src_parameter.kernel_size();
            dst->pool_params->kernel_w = src_parameter.kernel_size();
        }
    }
    else
    {
        dst->pool_params->kernel_h = 0;
        dst->pool_params->kernel_w = 0;
    }

    // Get PAD [DEFAULT = 0]
    if (src_parameter.has_pad_h() || src_parameter.has_pad_w())
    {
        ELOG ( (src_parameter.has_pad()),
                EP165,
                "Mention either pad or pad_h/pad_w for layer " << src.name() << ". Not both. !!")
        ASSERT((src_parameter.has_pad_h() && src_parameter.has_pad_w()),
                EP166,
                "Mention both pad_h and pad_w for layer " << src.name() << ". Or use pad.")
        dst->pool_params->pad_h = src_parameter.pad_h();
        dst->pool_params->pad_w = src_parameter.pad_w();
    }
    else
    {
        bool pad_specified = src_parameter.has_pad();
        dst->pool_params->pad_h = pad_specified ? src_parameter.pad() : 0;
        dst->pool_params->pad_w = pad_specified ? src_parameter.pad() : 0;
    }

    // Check if pad is same in all directions
    ASSERT( (dst->pool_params->pad_h == dst->pool_params->pad_w),
            EP167, "This version supports only same pad in all directions for Pooling layer: " << src.name() )

    // Get STRIDE [DEFAULT = 1]
    if (src_parameter.has_stride_h() || src_parameter.has_stride_w())
    {
        ELOG ( (src_parameter.has_stride()),
                EP168,
                "Mention either stride or stride_h/stride_w for layer " << src.name() << ". Not both. !!")
        ASSERT((src_parameter.has_stride_h() && src_parameter.has_stride_w()),
                EP169,
                "Mention both stride_h and stride_w for layer " << src.name() << ". Or use stride.")
        dst->pool_params->stride_h = src_parameter.stride_h();
        dst->pool_params->stride_w = src_parameter.stride_w();
    }
    else
    {
        bool stride_specified = src_parameter.has_stride();
        dst->pool_params->stride_h = stride_specified ? src_parameter.stride() : 1;
        dst->pool_params->stride_w = stride_specified ? src_parameter.stride() : 1;
    }

    // Check if stride is same in all directions
    ASSERT( (dst->pool_params->stride_h == dst->pool_params->stride_w),
            EP170, "This version supports only same stride in all directions for Pooling layer: " << src.name() )

    // Extract Precision Parameters
	bool success = ExtractPrecisionParameters(src, *dst, false);
	if(!success)
		graph.precMissLayers.push_back(src.name());

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
    if(global_pooling == true)
    {
        dst->pool_params->kernel_h           =  tmpBottom->shape.at(2);
        dst->pool_params->kernel_w           =  tmpBottom->shape.at(3);
    }

    //TODO:Anitha Commented to test new SSD
    // Check if filters are not rectangular
    //ASSERT( (dst->pool_params->kernel_h == dst->pool_params->kernel_w),
    //        EP164, "This version supports only square filters for Pooling layer: " << src.name() )

    dst->pool_params->N                     =   tmpBottom->shape.at(1);
     
    // Check top. Make sure it is not registered in the Graph.blobs
    it = graph.checkIfBlobExists(src.top(0), true, true);

    // Execution reached here means, top blob doesn't exist, so create.
    XBlob* tmpTop = new XBlob(src.top(0));
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

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
    bool success = ExtractPrecisionParameters(src, *dst);
    if(!success)
        graph.precMissLayers.push_back(src.name());

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
    XBlob* tmpTop = new XBlob(src.top(0));

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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

    // get TOP_K [OPTIONAL : default = 1]
    dst->argmax_params->top_k = src_parameter.has_top_k() ? src_parameter.top_k() : 1;
    
    // get AXIS [OPTIONAL : default = 1]
    dst->argmax_params->axis = src_parameter.has_axis() ? src_parameter.axis() : 1;

    // Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst);
    if(!success)
        graph.precMissLayers.push_back(src.name());


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
    XBlob* tmpTop = new XBlob(src.top(0));
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

    // get AXIS [OPTIONAL : default = 1]
    dst->softmax_params->axis = src_parameter.has_axis() ? src_parameter.axis() : 1;

    //  Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());
	
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
    XBlob* tmpTop = new XBlob(src.top(0));
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

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

    // Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst);
    if(!success)
        graph.precMissLayers.push_back(src.name());

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
    XBlob* tmpTop = new XBlob(src.top(0));
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

    // axis [OPTIONAL : default = 1]
    dst->concat_params->axis = src_parameter.has_axis() ? src_parameter.axis() : 1 ;    

	// Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());
	
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
    XBlob* tmpTop = new XBlob(src.top(0));
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

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

    //  Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());

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
    XBlob* tmpTop = new XBlob(src.top(0));
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

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
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());

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
    XBlob* tmpTop = new XBlob(src.top(0));
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

    dst->lrn_params->lsize              = src_parameter.has_local_size()? src_parameter.local_size(): 5;
    dst->lrn_params->alpha              = src_parameter.has_alpha()     ? src_parameter.alpha()     : 1.0;
    dst->lrn_params->beta               = src_parameter.has_beta()      ? src_parameter.beta()      : 0.75;
    dst->lrn_params->k                  = src_parameter.has_k()         ? src_parameter.k()         : 1.0;
    dst->lrn_params->type               = src_parameter.has_norm_region()? LRNType(src_parameter.norm_region()): ACROSS_CHANNELS;

    //  Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());

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
    XBlob* tmpTop = new XBlob(src.top(0));
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

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
    XBlob* tmpTop = new XBlob(src.top(0));
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;
    
    // Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

    // min_size [MANDATORY] TODO : Currently one & only-one value of min_size is allowed
    ASSERT( src_parameter.min_size_size() > 0, EP043, "min_size is not specified for PriorBox Layer: " << src.name())
    std::copy(src_parameter.min_size().begin(), src_parameter.min_size().end(), std::back_inserter(dst->priorbox_params->min_size));

    // max_size [OPIONAL as per SSDv3. MANDATORY as per SSDv4]. What TODO? 
    // ASSERT( src_parameter.max_size_size() > 0, EP044, "max_size is not specified for PriorBox Layer: " << src.name())
    std::copy(src_parameter.max_size().begin(), src_parameter.max_size().end(), std::back_inserter(dst->priorbox_params->max_size));
    
    // aspect_ratio [OPTIONAL]
    if(src_parameter.aspect_ratio_size() > 0)
    {  
        std::copy(src_parameter.aspect_ratio().begin(), src_parameter.aspect_ratio().end(), std::back_inserter(dst->priorbox_params->aspect_ratio));
    }

    // flip [OPTIONAL, default = true]
    dst->priorbox_params->flip = src_parameter.has_flip() ? src_parameter.flip() : true;
    
    // clip [OPTIONAL, default = false]
    dst->priorbox_params->clip = src_parameter.has_clip() ? src_parameter.clip() : false;

    // step [OPTIONAL as per SSDv3]
    ELOG(   src_parameter.has_step() && (src_parameter.has_step_h() || src_parameter.has_step_w()), EP103,
            "Please specify either step or step_h/step_w for PriorBox Layer: " << src.name() << ". Not both.")
    // #. Check if only one of step_h and step_w is provided.
    ELOG(   (src_parameter.has_step_h() !=  src_parameter.has_step_w()), EP104,
            "Please specify both step_h and step_w for PriorBox Layer: " << src.name())
    ELOG(   src_parameter.has_step() && src_parameter.step() <= 0, EP108, "step should be >0 for PriorBox Layer: " << src.name())
    ELOG(   src_parameter.has_step_h() && src_parameter.step_h() <= 0, EP109, "step_h should be >0 for PriorBox Layer: " << src.name())
    ELOG(   src_parameter.has_step_w() && src_parameter.step_w() <= 0, EP110, "step_w should be >0 for PriorBox Layer: " << src.name())

    if(src_parameter.has_step())
    {
        dst->priorbox_params->step = src_parameter.step();
        dst->priorbox_params->step_h = src_parameter.step();
        dst->priorbox_params->step_w = src_parameter.step();
    }
    else if(src_parameter.has_step_h() && src_parameter.has_step_w())
    {
        dst->priorbox_params->step = 0;
        dst->priorbox_params->step_h = src_parameter.step_h();
        dst->priorbox_params->step_w = src_parameter.step_w();
    }
    else
    {
        dst->priorbox_params->step = 0;
        dst->priorbox_params->step_h = 0;
        dst->priorbox_params->step_w = 0;
    }

    // img_size [OPTIONAL]
    ELOG(   src_parameter.has_img_size() && (src_parameter.has_img_h() || src_parameter.has_img_w()), EP103,
            "Please specify either img or img_h/img_w for PriorBox Layer: " << src.name() << ". Not both.")
    ELOG(   src_parameter.has_img_h() !=  src_parameter.has_img_w(), EP104,
            "Please specify both img_h and img_w for PriorBox Layer: " << src.name())
    ELOG(   src_parameter.has_img_size() && src_parameter.img_size() <= 0, EP105, "img_size should be >0 for PriorBox Layer: " << src.name())
    ELOG(   src_parameter.has_img_h() && src_parameter.img_h() <= 0, EP106, "img_h should be >0 for PriorBox Layer: " << src.name())
    ELOG(   src_parameter.has_img_w() && src_parameter.img_w() <= 0, EP107, "img_w should be >0 for PriorBox Layer: " << src.name())

    if(src_parameter.has_img_size())
    {
        dst->priorbox_params->img_size = src_parameter.img_size();
        dst->priorbox_params->img_h = src_parameter.img_h();
        dst->priorbox_params->img_w = src_parameter.img_w();
    }
    else if(src_parameter.has_img_h() && src_parameter.has_img_w())
    {
        dst->priorbox_params->img_size = 0;
        dst->priorbox_params->img_h = src_parameter.img_h();
        dst->priorbox_params->img_w = src_parameter.img_w();
    }
    else
    {
        dst->priorbox_params->img_size = 0;
        dst->priorbox_params->img_h = 0;
        dst->priorbox_params->img_w = 0;
    }


    // offset [OPTIONAL, default = 0.5]
    dst->priorbox_params->offset = src_parameter.has_offset() ? src_parameter.offset() : 0.5;
    
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
    XBlob* tmpTop = new XBlob(src.top(0));
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;

    // ---------------------------------  Pre-compute priorbox and variance -------------------- //

    int layerheight= dst->priorbox_params->img_h > 0 ? dst->priorbox_params->img_h : dst->bottomShape.at(0).at(2);
    int layerwidth = dst->priorbox_params->img_w > 0 ? dst->priorbox_params->img_w :dst->bottomShape.at(0).at(3);
    int imageheight= dst->bottomShape.at(1).at(2);
    int imagewidth = dst->bottomShape.at(1).at(3);
    vector<float> ar = dst->priorbox_params->aspect_ratio;
    vector<float> min_size = dst->priorbox_params->min_size;
    vector<float> max_size = dst->priorbox_params->max_size;
    vector<float> var = dst->priorbox_params->variance;
    bool flip = dst->priorbox_params->flip;
    bool clip = dst->priorbox_params->clip;
    float offset = dst->priorbox_params->offset;
    float step_h = dst->priorbox_params->step_h;
    float step_w = dst->priorbox_params->step_w;

    dst->priorbox_params->pbox =  computePriorBoxes(layerwidth, layerheight, imagewidth, imageheight, 
                                                  ar, min_size, max_size, var, flip, clip, step_h, step_w, offset);
    dst->priorbox_params->pboxShape = dst->topShape.at(0);

    // Keep priorboxes in txt files for debug purposes
    // string filename = dst->uname + "_pboxes";
    // savetxt(priorboxes, filename);

    //  Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

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
    XBlob* tmpTop = new XBlob(src.top(0));
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;

    //  Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

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
    XBlob* tmpTop = new XBlob(src.top(0));
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;

    //  Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());
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
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

    // Get Op Type [OPTIONAL, DEFAULT=SUM]
    dst->eltwise_params->type = (src_parameter.has_operation()) ? (EltOpType)src_parameter.operation() : (EltOpType)ELT_SUM;    

    // Currently only SUM/PROD is supported
    ASSERT  ((dst->eltwise_params->type == ELT_SUM) || (dst->eltwise_params->type == ELT_PROD), EP061,
            "For Eltwise layer, only SUM and PROD operation is allowed")

    // Get coeff. TODO : ARK: Currently coeff is not supported
    ASSERT(src_parameter.coeff().size() == 0, EP062, "Coeff for Eltwise layer is not currently supported") 


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
    XBlob* tmpTop = new XBlob(src.top(0));
    dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

    // Update layer "top" info with top name and shape
    tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
    dst->computeOutputDim();                                    // Calculate the output blob shape
    tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
    dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also

    // Finally Register the top blob to graph
    graph.blobs[tmpTop->name] = tmpTop;

    //  Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());
}

void ExtractBatchNormParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::BatchNormParameter src_parameter = src.batch_norm_param();
    mapStrStr::const_iterator type_it = CaffeLayerMap.find(src.type());
    string xlayerType = type_it->second;

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Check if it is inPlace.
    dst->batchnorm_params->inPlace = src.bottom(0) == src.top(0) ? true : false; 
    dst->inPlace = src.bottom(0) == src.top(0) ? true : false; 

    // Get use_global_stats [Optional : default : true]
    dst->batchnorm_params->global_stats = src_parameter.has_use_global_stats() ? src_parameter.use_global_stats() : true;
    ASSERT( dst->batchnorm_params->global_stats == true, EP163, 
            "use_global_stats should be true for the batch_norm layer: " << src.name())

    // Get eps [default: 1e-5]
    dst->batchnorm_params->eps = src_parameter.has_eps() ? src_parameter.eps() : 0.00001;


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
        XBlob* tmpTop = new XBlob(src.top(0));
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
    
    // Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());
}

void ExtractScaleParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::ScaleParameter src_parameter = src.scale_param();
    mapStrStr::const_iterator type_it = CaffeLayerMap.find(src.type());
    string xlayerType = type_it->second;

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Check if it is inPlace.
    dst->scale_params->inPlace = src.bottom(0) == src.top(0) ? true : false; 
    dst->inPlace = src.bottom(0) == src.top(0) ? true : false; 

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
        XBlob* tmpTop = new XBlob(src.top(0));
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
    
    // Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());

}


void ExtractPowerParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::PowerParameter src_parameter = src.power_param();
    mapStrStr::const_iterator type_it = CaffeLayerMap.find(src.type());
    string xlayerType = type_it->second;

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));

    // Parse the input/output connections
    checkNumberOfTopAndBottom(src, 1, 1);

    // Check if it is inPlace.
    dst->power_params->inPlace = src.bottom(0) == src.top(0) ? true : false; 
    dst->inPlace = src.bottom(0) == src.top(0) ? true : false; 

    // Get power [default: 1.0]
    // TODO : Ark : Currently only power=1.0 is allowed
    dst->power_params->power = src_parameter.has_power() ? src_parameter.power() : 1.0;
    ASSERT(dst->power_params->power == 1.0, EP072, "Current version of CHaiDNN supports only power = 1.0 in Power Layer")

    // Get scale [default: 1]
    // TODO : Ark : Currently only scale=1 is allowed
    dst->power_params->scale = src_parameter.has_scale() ? src_parameter.scale() : 1.0;
    ASSERT(dst->power_params->scale == -1.0, EP073, "Current version of CHaiDNN supports only scale = -1.0 in Power Layer")

    // Get shift [default: 1]
    // TODO : Ark : Currently only shift = 0.0 is allowed
    dst->power_params->shift = src_parameter.has_shift() ? src_parameter.shift() : 0.0;
    ASSERT(dst->power_params->shift == 0.0, EP074, "Current version of CHaiDNN supports only shift = 0.0 in Power Layer")

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
        XBlob* tmpTop = new XBlob(src.top(0));
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

    //  Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());
}


void ExtractXCustomParameters(const caffe::LayerParameter& src, XGraph& graph)
{
    // Map the Caffe layer type to anonymoX layer type
    caffe::XCustomParameter src_parameter = src.xcustom_param();
    string xlayerType = "XCustom";

    // Create new XLayer
    XLayer* dst = new XLayer(src.name(), xlayerType, src.top(0));
    
    // Assign actual layer type
    dst->xcustom_params->type = src.type();

    // Check if it is inPlace.
    int ntops = src.top_size();
    int nbottoms = src.bottom_size();
    ASSERT( (ntops > 0) && (nbottoms > 0), EP070, 
            "There should be minimum one top & bottom blobs for layer " << src.name())

    ASSERT( (ntops == 1), EP075, 
            "Currently only one top blob is supported for user_defined_layer: " << src.name())

    ELOG( (ntops == 1 && nbottoms == 1 && src.bottom(0) == src.top(0)), EP073,
            "bottom and top blobs should be separate for user_defined_layer : " << src.name() )                        

    // Extract output dimensions [MANDATORY : Atleast one dimension should be given and it should be more than 0]
    ASSERT(src_parameter.top_dim_size() > 0, EP071,
            "Output dimension is not mentioned for user_defined_layer : " << src.name())

    dst->xcustom_params->output_dim.insert( dst->xcustom_params->output_dim.end(), 
                                            src_parameter.top_dim().begin(), src_parameter.top_dim().end());

    ASSERT(getSize(dst->xcustom_params->output_dim) > 0, EP072,
            "Total dimesion should be greater than 0 for user_defined_layer : " << src.name())

    // Extract all float arguments
    dst->xcustom_params->float_args.insert( dst->xcustom_params->float_args.end(), 
                                            src_parameter.float_args().begin(), src_parameter.float_args().end());

    // Extract all string arguments
    dst->xcustom_params->string_args.insert(dst->xcustom_params->string_args.end(), 
                                            src_parameter.string_args().begin(), src_parameter.string_args().end());

    // Finally add the XLayer to graph
    graph.layers[src.name()] = dst;

    // -----------------------------   Setup Blobs and Connections ----------------------- //


    // Check bottom first. Make sure it is already registered in the Graph.blobs
    map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(0), true, false);

    for(int i = 0; i<src.bottom_size(); i++)
    {
        // Check bottom first. Make sure it is already registered in the Graph.blobs
        map<string, XBlob*>::iterator it = graph.checkIfBlobExists(src.bottom(i), true, false);

        // Execution here reached means, bottom blob exists in graph. So update its fields
        XBlob* tmpBottom = it->second;
        dst->bottom.push_back(nameIndex(tmpBottom));                           // Add bottom to XLayer.bottom
        dst->bottomShape.push_back(tmpBottom->shape);                           // Replicate bottom shape in Layer also
        dst->xcustom_params->input_dims.push_back(tmpBottom->shape);
        tmpBottom->consumers.push_back(dst->name);                                   // Add convolution layer to bottom consumers
        tmpBottom->consumerDim.push_back(tmpBottom->shape);                     // and consumer uses the full bottom data.
    }
     
    // Here, things are a little different because of the inplace operation
    for(int i=0; i<src.top_size(); ++i)
    {
        // Check top. Make sure it is not registered in the Graph.blobs
        it = graph.checkIfBlobExists(src.top(0), true, true);

        // Execution reached here means, top blob doesn't exist, so create.
        XBlob* tmpTop = new XBlob(src.top(0));
        dst->top.push_back(nameIndex(tmpTop));                // Add top blob to current layer top list

        // Update layer "top" info with top name and shape
        tmpTop->producers.push_back(dst->name);                     // Add current layer to top Producers list
        dst->computeOutputDim();                                    // Calculate the output blob shape
        tmpTop->producerDim.push_back(tmpTop->shape);           // Add output blob shape to producerDim
        dst->topShape.push_back(tmpTop->shape);                           // Replicate bottom shape in Layer also
        dst->xcustom_params->output_dims.push_back(tmpTop->shape);

        // Finally Register the top blob to graph
        graph.blobs[tmpTop->name] = tmpTop;
    }

    //  Extract Precision Parameters
    bool success = ExtractPrecisionParameters(src, *dst, false);
    if(!success)
        graph.precMissLayers.push_back(src.name());
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

#define CHECK_PRECISION(condn, variable, true_value, default_value) \
    if(condn)                                                       \
    {                                                               \
        variable = true_value;                                      \
    }                                                               \
    else                                                            \
    {                                                               \
        variable = default_value;                                   \
        parsedSuccessfully = false;                                 \
    }   

// Extract Dynamic Fixed Point Precision parameters
// Return true if successfully parsed
template<typename PrecisionType>
bool ExtractDynamicFixedPointParameters(const PrecisionType& src_params, XLayer& layer, bool isParamsPrecisionMandatory, bool parsedSuccessfully_)
{
	bool parsedSuccessfully = parsedSuccessfully_;
	
    // input and output precision parameters are mandatory for every layer.
    CHECK_PRECISION(src_params.has_bw_layer_in(), layer.ip_bw, src_params.bw_layer_in(), DEFAULT_BW)
    CHECK_PRECISION(src_params.has_bw_layer_out(), layer.op_bw, src_params.bw_layer_out(), DEFAULT_BW)
    CHECK_PRECISION(src_params.has_fl_layer_in(), layer.ip_fl, src_params.fl_layer_in(), DEFAULT_FL)
    CHECK_PRECISION(src_params.has_fl_layer_out(), layer.op_fl, src_params.fl_layer_out(), DEFAULT_FL)

    // weights precision is mandatory for [Convolution, FC] layers.
    if(layer.type == "Convolution" || layer.type == "InnerProduct")
    {
        CHECK_PRECISION(src_params.has_bw_params(), layer.wt_bw, src_params.bw_params(), DEFAULT_BW)
        CHECK_PRECISION(src_params.has_fl_params(), layer.wt_fl, src_params.fl_params(), DEFAULT_BW-1)
    }

    // BatchNorm precision parameters
    if(layer.type == "BatchNorm")
    {
        CHECK_PRECISION(src_params.has_batchnorm_mean_bw(), layer.bn_mean_bw, src_params.batchnorm_mean_bw(), DEFAULT_BW)
        CHECK_PRECISION(src_params.has_batchnorm_mean_fl(), layer.bn_mean_fl, src_params.batchnorm_mean_fl(), DEFAULT_FL)
        CHECK_PRECISION(src_params.has_batchnorm_variance_bw(), layer.bn_variance_bw, src_params.batchnorm_variance_bw(), DEFAULT_BW)
        CHECK_PRECISION(src_params.has_batchnorm_variance_fl(), layer.bn_variance_fl, src_params.batchnorm_variance_fl(), DEFAULT_FL)
    }

    // Scale layer precision parameters
    if(layer.type == "Scale")
    {
        CHECK_PRECISION(src_params.has_scale_gamma_bw(), layer.scale_gamma_bw, src_params.scale_gamma_bw(), DEFAULT_BW)
        CHECK_PRECISION(src_params.has_scale_gamma_fl(), layer.scale_gamma_fl, src_params.scale_gamma_fl(), DEFAULT_FL)
        CHECK_PRECISION(src_params.has_scale_beta_bw(), layer.scale_beta_bw, src_params.scale_beta_bw(), DEFAULT_BW)
        CHECK_PRECISION(src_params.has_scale_beta_fl(), layer.scale_beta_fl, src_params.scale_beta_fl(), DEFAULT_FL)
        CHECK_PRECISION(src_params.has_scale_gamma_by_std_bw(), layer.scale_gamma_by_std_bw, src_params.scale_gamma_by_std_bw(), DEFAULT_BW)
        CHECK_PRECISION(src_params.has_scale_gamma_by_std_fl(), layer.scale_gamma_by_std_fl, src_params.scale_gamma_by_std_fl(), DEFAULT_FL)
	}

    return parsedSuccessfully;
}

template
bool ExtractDynamicFixedPointParameters(const caffe::PrecisionParameter& src_params, XLayer& layer, bool isParamsPrecisionMandatory, bool parsedSuccessfully_);
template
bool ExtractDynamicFixedPointParameters(const caffe::QuantizationParameter& src_params, XLayer& layer, bool isParamsPrecisionMandatory, bool parsedSuccessfully_);

template<typename PrecisionType>
bool ExtractOfflineQuantizationParameters(const PrecisionType& src_params, XLayer& layer, bool isParamsPrecisionMandatory, bool parsedSuccessfully_) {
    bool parsedSuccessfully = parsedSuccessfully_;

    // input precision parameters are mandatory for every layer.
    CHECK_PRECISION(src_params.has_bw_layer_in(), layer.ip_bw, src_params.bw_layer_in(), DEFAULT_BW);
    CHECK_PRECISION(src_params.has_th_layer_in(), layer.th_layer_in, src_params.th_layer_in(), DEFAULT_TH);

    // Output precision is not mandatory for Softmax, so skip it. (TODO : Have a better scheme)
    if(layer.type != "Softmax") {
	CHECK_PRECISION(src_params.has_bw_layer_out(), layer.op_bw, src_params.bw_layer_out(), DEFAULT_BW);
	CHECK_PRECISION(src_params.has_th_layer_out(), layer.th_layer_out, src_params.th_layer_out(), DEFAULT_TH);
    }

    // weights precision is mandatory for [Convolution, FC] layers.
    if(layer.type == "Convolution" || layer.type == "InnerProduct" || layer.type == "Deconvolution")
    {
	CHECK_PRECISION(src_params.has_bw_params(), layer.wt_bw, src_params.bw_params(), DEFAULT_BW);
	if(src_params.th_params_size() > 0) {
	    layer.th_params.insert(layer.th_params.begin(), src_params.th_params().begin(), src_params.th_params().end());
	}
	else {
	    // layer.th_params = getAbsMaxPerFilter<float>(layer.params[0], layer.paramDims[0]);
	    parsedSuccessfully = false;
	}
    }

    // Batchnorm params [TODO : Assigining default vals now]
    if(layer.type == "BatchNorm") {
        // layer.th_bn_mean 
        // layer.th_bn_variance 
        parsedSuccessfully = false;
    }
	
    // Scale params [TODO : Assigining default vals now]
    if(layer.type == "Scale") {
        // layer.th_scale_gamma
        // layer.th_scale_beta
        parsedSuccessfully = false;
    }

    // Scale params [TODO : Assigining default vals now]
    if(layer.type == "L2Normalize") {
	CHECK_PRECISION(src_params.has_bw_params(), layer.wt_bw, src_params.bw_params(), DEFAULT_BW);
	if(src_params.th_params_size() > 0) {
	    layer.th_params.insert(layer.th_params.begin(), src_params.th_params().begin(), src_params.th_params().end());
	}
	else {
	    parsedSuccessfully = false;
	}
    }

    return parsedSuccessfully;
}

template
bool ExtractOfflineQuantizationParameters(const caffe::PrecisionParameter& src_params, XLayer& layer, bool isParamsPrecisionMandatory, bool parsedSuccessfully_);
template
bool ExtractOfflineQuantizationParameters(const caffe::QuantizationParameter& src_params, XLayer& layer, bool isParamsPrecisionMandatory, bool parsedSuccessfully_);

// Extract FP Precision Parameters
bool ExtractPrecisionParameters(const caffe::LayerParameter& src, XLayer& layer, bool isParamsPrecisionMandatory) {
    bool parsedSuccessfully = true;

    // Check if it is a single precision layer
    if(SWLayerSet.find(layer.type) != SWLayerSet.end()) {
	layer.quantization_scheme = "SinglePrecision";
	return true;
    }

    // precision_param() or quantization_param() ?
    string field_type = "";

    if(src.has_precision_param()) {
        // src_params = &(src.precision_param());
	layer.quantization_scheme = src.precision_param().quant_type();
	field_type = "precision"; 
    }
    else if(src.has_quantization_param()) {
	// src_params = &(src.quantization_param());
	layer.quantization_scheme = src.quantization_param().quant_type();
	field_type = "quantization"; 
    }
    else {
	parsedSuccessfully = false;
    }

    layer.quantization_scheme = layer.quantization_scheme.empty() ? "DynamicFixed" : layer.quantization_scheme; 

    // quant_type should be either "DynamicFixed" or "Xilinx" (if empty, by default it is Dynamic Fixed)
    ASSERT((layer.quantization_scheme == "DynamicFixed" || layer.quantization_scheme == "Xilinx"), EP501,
    		"Quantization Scheme should be either \"DynamicFixed\" or \"Xilinx\". "
    		"Quantization Scheme of layer " << layer.name << " is " << "\"" << layer.quantization_scheme << "\""
			<< " which is not supported" )

    // INFO : Special case : Don't run precision extraction for Maxpool DynamicFixed format
    // TODO : Maxpool has prec param only in Offline mode, not in DynamicFixed mode. Better keep same pattern everywhere.
    if(layer.quantization_scheme == "DynamicFixed") {
	if ((layer.type == "Pooling" && layer.pool_params->PoolType == MAX) ||
		(layer.type == "Concat") || (layer.type == "Softmax"))
	{
	    return true;
	}
    }

    // Parse based on the quantization_scheme
    if(layer.quantization_scheme == "DynamicFixed") {
	if(field_type == "precision") {
	    parsedSuccessfully = ExtractDynamicFixedPointParameters(src.precision_param(), layer, isParamsPrecisionMandatory, parsedSuccessfully);
	}
	else { // quantization
	    parsedSuccessfully = ExtractDynamicFixedPointParameters(src.quantization_param(), layer, isParamsPrecisionMandatory, parsedSuccessfully);
	}
    }
    else if(layer.quantization_scheme == "Xilinx") {
	if(field_type == "precision") {
	    parsedSuccessfully = ExtractOfflineQuantizationParameters(src.precision_param(), layer, isParamsPrecisionMandatory, parsedSuccessfully);
	}
	else { // quantization
	    parsedSuccessfully = ExtractOfflineQuantizationParameters(src.quantization_param(), layer, isParamsPrecisionMandatory, parsedSuccessfully);
	}
    }
    else {
	cerr << "[WP205] Quantization scheme is not assigned for layer : " << layer.name << endl;
    }
#if 0
    cout << "[DPxxx] " << src.name() << " DFP : [ " 
	<<  layer.ip_bw << ":" << layer.ip_fl << " "
	<<  layer.op_bw << ":" << layer.op_fl << " "
	<<  layer.wt_bw << ":" << layer.wt_fl << " ]" << endl;
    cout << "[DPxxx] " << src.name() << " OFF : [ " 
	<<  layer.ip_bw << ":" << layer.th_layer_in << " "
	<<  layer.op_bw << ":" << layer.th_layer_out << " "
	<<  layer.wt_bw << ":" ;
    if(layer.th_params.size() > 0)
	std::copy(layer.th_params.begin(), layer.th_params.begin() + 10, std::ostream_iterator<float>(cerr, " ")); cerr << endl;
#endif

    // cout << "[DPxxx] " << src.name() << " HW BN: [ " << layer.bn_mean_bw << ":" << layer.bn_mean_fl << " " 
    //                                                  << layer.bn_variance_bw << ":" << layer.bn_variance_fl << endl;
    // cout << "[DPxxx] " << src.name() << " HW SCALE: [ " << layer.scale_gamma_bw << ":" << layer.scale_gamma_fl << " " 
    //                                                  << layer.scale_beta_bw << ":" << layer.scale_beta_fl << endl;

    return parsedSuccessfully;
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
    vector<float> aspect_ratios;
    aspect_ratios.push_back(1.);

    for (int i=0; i<aspect_ratios_.size(); i++)
    {
        float ar = aspect_ratios_[i];
        bool processed = false;
        for(int j=0; j<aspect_ratios.size(); ++j)
        {
            // #. Check for duplicate values
            if(fabs(ar - aspect_ratios[j]) < 1e-6)
            {
                processed = true;
                break;
            }
        }
        if(!processed)
        {
            aspect_ratios.push_back(ar);
            if (flip)
            {
                aspect_ratios.push_back(1.0 / ar);
            }
        }
    }

    int num_priors = aspect_ratios.size() * min_size.size();

    if(max_size.size() > 0)
    {
        for(int i=0; i<max_size.size(); ++i)
        {
            ASSERT( max_size[i] > min_size[i], EP100,
                    "max_size shoule be larger than min_size for PriorBox Layer.")
            num_priors+=1;
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
            for(int s = 0; s<min_size.size(); ++s)
            {
                // first prior: aspect_ratio = 1, size = min_size
                box_width = box_height = (float)min_size[s];
                priorboxes[idx++] = (center_x - box_width / 2.) / img_width;    // xmin
                priorboxes[idx++] = (center_y - box_height / 2.) / img_height;  // ymin
                priorboxes[idx++] = (center_x + box_width / 2.) / img_width;    // xmax
                priorboxes[idx++] = (center_y + box_height / 2.) / img_height;  // ymax

                // second prior: aspect_ratio = 1, size = sqrt(min_size * max_size)     
                if((max_size.size() > 0) && (max_size.size() == min_size.size()) && (max_size[s] > min_size[s]))
                {
                    box_width = box_height = sqrt(min_size[s] * max_size[s]);           
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
                    box_width     = min_size[s] * sqrt(ar);
                    box_height    = min_size[s] / sqrt(ar);
                    priorboxes[idx++] = (center_x - box_width / 2.) / img_width;  // xmin
                    priorboxes[idx++] = (center_y - box_height / 2.) / img_height;// ymin
                    priorboxes[idx++] = (center_x + box_width / 2.) / img_width;  // xmax
                    priorboxes[idx++] = (center_y + box_height / 2.) / img_height;// ymax
                }
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

// An utility function to convert Ristretto precision format to HW precision format
void RistrettoToHWFormat(const int BW, int r_bw, int r_fl, int& hw_bw, int& hw_fl, bool IO_format)
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
    // If Q.F is the format obtained from Ristretto, I/O needs Q+1:F-1, but weights still needs Q:F itself
    //hw_fl = IO_format ? tmp_fl - 1: tmp_fl ;
    //TODO:: Anusha
    hw_fl = IO_format ? tmp_fl : tmp_fl ;
    hw_fl = std::max(0, hw_fl);
    hw_fl += diff;
    hw_bw = BW;
}

// An utility function to convert Ristretto precision format to HW precision format
void RistrettoToHWFormat_fixedFL(const int BW, int r_bw, int r_fl, int& hw_bw, int& hw_fl, bool IO_format)
{
    string type = IO_format ? "I/O feature maps" : "parameters"; 
    int tmp_Q = r_bw - r_fl;
    int tmp_fl = std::max(0, r_fl);                        // if fl is negative, clip it to zero.

    // Q+F < BW
    ELOG( tmp_Q + tmp_fl > BW, EP055,
          "Total bit-width required for " << type << " is more than what is supported in the HW : " << tmp_Q+tmp_fl << " v/s " << BW);

    // Behaviour is different for weights and I/O formats
    // Special care should be taken care for I/O formats
    // If Q.F is the format obtained from Ristretto, I/O needs Q+1:F-1, but weights still needs Q:F itself
    // FIXME : If user gives 16:0, this will become 17:-1
    hw_fl = IO_format ? tmp_fl - 1: tmp_fl ;
    hw_bw = BW;
}

// An utility function to convert Ristretto precision format to HW precision format
void RistrettoToHWFormat_deprecated(const int BW, int r_bw, int r_fl, int& hw_bw, int& hw_fl, bool IO_format)
{
    // Behaviour is different for weights and I/O formats
    // Special care should be taken care for I/O formats
    // If Q.F is the format obtained from Ristretto, I/O needs Q+1:F-1, but weights still needs Q:F itself
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
