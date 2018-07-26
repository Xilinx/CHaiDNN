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
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sds_lib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>

#define MEM_ALLOC   sds_alloc_non_cacheable
#define MEM_FREE    sds_free

FILE * outCsv = NULL;

#include "xi_perf_eval.hpp"
#include "../common/xi_kernels.h"

//# For DietChai, number of ports will be less
#define NUM_PORTS_IN            5
#define NUM_PORTS_OUT           4
#define NUM_PORTS_WEIGHTS       4
#define NUM_PORTS_BIAS          1

//# Assign some constant size for all, might not work in all cases
#define INPUT_BLOCK_SIZE        8
#define OUTPUT_BLOCK_SIZE       8
#define WEIGHTS_BLOCK_SIZE      4
#define BIAS_BLOCK_SIZE         4

int getNumberOfLayers(std::string &layerspec_csv)
{
    //# Open CSV and count number of layers
    std::string line;
    int number_of_lines = 0;
    FILE * csvFp = fopen(layerspec_csv.c_str(), "r");
    if (csvFp == NULL)
    {
        std::cout << "[ERROR] Couldn't open file ! " << std::endl;
        return -1;
    }
    uint32_t lines = 0;
    int32_t c;
    int32_t last = '\n';
    while (EOF != (c = fgetc(csvFp))) 
    {
        if ((c == '\n') && (last != '\n'))
        {
            ++lines;
        }
        last = c;
    }
    fclose(csvFp);
    return lines;
}

enum _opcode
{
    /*
    OPCODE_CONV=0,          // This opcode works for both standalone conv or conv+relu
    OPCODE_BN = 10,         // This opcode works as BatchNorm+ Scale
    OPCODE_POOL2CONV=20,    // This opcode mapped to max pooling 
    OPCODE_AVRPOOL2CONV=21,
    OPCODE_3D_CONV = 19,
    OPCODE_ELTWISE = 11,
    */
    OPCODE_SW_LAYER = -1,
    OPCODE_ZERO_LAYER = -2
};

void initParams(Parameter_struct &inParams, std::vector<int> &params, int &opcode)
{
    if (opcode == OPCODE_CONV || opcode == OPCODE_3D_CONV)
    {
        inParams.N = params[2];
        inParams.M = params[11];
        
        inParams.filter_h = params[3];
        inParams.filter_w = params[4];
        inParams.stride_h = params[5];
        inParams.stride_w = params[6];
        
        inParams.pad_h = params[7];
        inParams.pad_w = params[8];

        inParams.dilation = params[13];
        inParams.group    = params[12];
        
        inParams.inp_height = params[0];
        inParams.inp_width  = params[1];
        inParams.out_height = params[9];
        inParams.out_width  = params[10];

        inParams.opcode     = opcode;
        inParams.reluflag   = 0;
    }
    else if (opcode == OPCODE_BN)
    {
        inParams.N = params[2];
        inParams.M = params[2];

        inParams.filter_h = 1;
        inParams.filter_w = 1;
        inParams.stride_h = 1;
        inParams.stride_w = 1;

        inParams.pad_h = 1;
        inParams.pad_w = 1;

        inParams.dilation = 1;
        inParams.group    = 1;

        inParams.inp_height = params[0];
        inParams.inp_width  = params[1];
        inParams.out_height = params[0];
        inParams.out_width  = params[1];

        inParams.opcode     = opcode;
        inParams.reluflag   = 0;
    }
    else if (opcode == OPCODE_POOL2CONV || opcode == OPCODE_AVRPOOL2CONV)
    {
        inParams.N = params[2];
        inParams.M = params[11];
        
        inParams.filter_h = params[3];
        inParams.filter_w = params[4];
        inParams.stride_h = params[5];
        inParams.stride_w = params[6];
        
        inParams.pad_h = params[7];
        inParams.pad_w = params[8];

        inParams.dilation = 0;
        inParams.group    = 0;
        
        inParams.inp_height = params[0];
        inParams.inp_width  = params[1];
        inParams.out_height = params[9];
        inParams.out_width  = params[10];

        inParams.opcode     = opcode;
        inParams.reluflag   = 0;
    }
    else if (opcode == OPCODE_ELTWISE)
    {
        inParams.N = params[2];
        inParams.M = params[2];

        inParams.filter_h = 1;
        inParams.filter_w = 1;
        inParams.stride_h = 1;
        inParams.stride_w = 1;

        inParams.pad_h = 1;
        inParams.pad_w = 1;

        inParams.dilation = 1;
        inParams.group    = 1;

        inParams.inp_height = params[0];
        inParams.inp_width  = params[1];
        inParams.out_height = params[0];
        inParams.out_width  = params[1];

        inParams.opcode     = opcode;
        inParams.reluflag   = 0;
    }
}

//# Layer map
std::map<std::string, int> layerMap;

int runLayer(const char *layerType, std::vector<int> &params)
{

    int* scalar_conv_args = (int*) MEM_ALLOC (256 * sizeof (int));
    if(scalar_conv_args == NULL)
    {
        std::cerr << "[ERROR] Couldn't allocate memory for : scalar_conv_args" << std::endl;
        return -1;
    }
    //# Check for Op-code
    std::string layer_t(layerType);
    int opcode;
    if (layerMap.count(layer_t) > 0)
        opcode = layerMap[layer_t];
    else
        opcode = 0x00C0FFEE;

    std::cout << "[EVALx] Layer Type : " << layer_t << " Opcode : " << opcode << std::endl;

    if (opcode == 0x00C0FFEE)
    {
        std::cout << "[EVALx] Layer " <<  layerType << " is not supported !" << std::endl << std::endl;
        fprintf (outCsv, "%s, %lf\n", layerType, 0.0f);
        return 0;
    }
    else if (opcode == OPCODE_SW_LAYER)
    {
        std::cout << "[EVALx] Layer " <<  layerType << " runs on ARM !" << std::endl << std::endl;
        fprintf (outCsv, "%s, %lf\n", layerType, 0.0f);
        return 0;
    }
    else if (opcode == OPCODE_ZERO_LAYER)
    {
        std::cout << "[PERFM] Estimated time : " <<  "0.0 ms" << std::endl;
        fprintf (outCsv, "%s, %lf\n", layerType, 0.0f);
        return 0;
    }

    //# Allocate memory
    std::vector<void*> input;
    std::vector<void*> output;
    std::vector<void*> weights;
    std::vector<void*> bias;
 
    //# Input buffers
    for (int ibuff = 0; ibuff < NUM_PORTS_IN; ++ibuff)
    {
        void * ptr = (void*) MEM_ALLOC (INPUT_BLOCK_SIZE * 1024);
        if (ptr == NULL)
        {
            std::cerr << "[ERROR] Couldn't allocate memory for : input port " << ibuff << std::endl;
            return -1;
        }
        input.push_back(ptr);
        ptr = NULL;
    }
    //# Output buffers
    for (int obuff = 0; obuff < NUM_PORTS_OUT; ++obuff)
    {
        void * ptr = (void*) MEM_ALLOC (OUTPUT_BLOCK_SIZE * 1024);
        if (ptr == NULL)
        {
            std::cerr << "[ERROR] Couldn't allocate memory for : output port " << obuff << std::endl;
            return -1;
        }
        output.push_back(ptr);
        ptr = NULL;
    }
    //# Weights buffers
    for (int wbuff = 0; wbuff < NUM_PORTS_WEIGHTS; ++wbuff)
    {
        void * ptr = (void*) MEM_ALLOC (WEIGHTS_BLOCK_SIZE * 1024);
        if (ptr == NULL)
        {
            std::cerr << "[ERROR] Couldn't allocate memory for : weights port " << wbuff << std::endl;
            return -1;
        }
        weights.push_back(ptr);
        ptr = NULL;
    }
    //# Bias buffers
    for (int bbuff = 0; bbuff < NUM_PORTS_BIAS; ++bbuff)
    {
        void * ptr = (void*) MEM_ALLOC (BIAS_BLOCK_SIZE * 1024);
        if (ptr == NULL)
        {
            std::cerr << "[ERROR] Couldn't allocate memory for : bias port " << bbuff << std::endl;
            return -1;
        }
        bias.push_back(ptr);
        ptr = NULL;
    }

    //# Init for different opcode
    Parameter_struct inParams;

    initParams(inParams, params, opcode);

    std::cout << "[EVALx] Running init ............... " ;
    hardwareParamInit (&inParams, scalar_conv_args);
    std::cout << "Done !" << std::endl;

#if 0
    for (int i = 0; i < 128; ++i)
        std::cout << scalar_conv_args[i] << std::endl;
#endif

    unsigned long long start, end;
    unsigned long long frequency = sds_clock_frequency();
    start = sds_clock_counter();
    //# Call Conv
    ConvolutionForward(
        (char*)weights[0], (char*)weights[1],
    #if (KER_PROC==16 || (PORT_BITWIDTH_64BIT==1 && KER_PROC==8)) 
        (char*)weights[2], (char*)weights[3], 
    #endif
        (char*)output[0], 
    #if !SINGLE_IO_PORT
        (char*)output[1],
    #endif
        (char*)input[0], 
    #if !SINGLE_IO_PORT
        (char*)input[1],
    #endif
        (char*)input[2], (short*)bias[0], 
    #if !DISABLE_BN
        (char*)input[3], (char*)input[4], 
    #endif
        (char*)output[2],
    #if !SINGLE_IO_PORT 
        (char*)output[3], 
    #endif
        scalar_conv_args);
        
    sds_wait(1);
    end = sds_clock_counter();
  	double tot_time = ((double)(end - start) / ((double)frequency)) * 1000.0;
	fprintf(stderr, "[PERFM] Estimated time : %lf ms\n\n", tot_time);
    fprintf (outCsv, "%s, %lf\n", layerType, tot_time);
    
    //# Release Memory
    for (int ibuff = 0; ibuff < NUM_PORTS_IN; ++ibuff)      MEM_FREE(input[ibuff]);
    for (int obuff = 0; obuff < NUM_PORTS_OUT; ++obuff)     MEM_FREE(output[obuff]);
    for (int wbuff = 0; wbuff < NUM_PORTS_WEIGHTS; ++wbuff) MEM_FREE(weights[wbuff]);
    for (int bbuff = 0; bbuff < NUM_PORTS_BIAS; ++bbuff)    MEM_FREE(bias[bbuff]);

    return 0;
}

void initLayerMap(void)
{
    //# Initialize layer map
    layerMap["CONV"]        = OPCODE_CONV;
    layerMap["BN"]          = OPCODE_BN;
    layerMap["MAXPOOL"]     = OPCODE_POOL2CONV;
    layerMap["AVEPOOL"]     = OPCODE_AVRPOOL2CONV;
    layerMap["ELTWISE"]     = OPCODE_ELTWISE;
    layerMap["CONV3D"]      = OPCODE_3D_CONV;
    
    layerMap["NMS"]         = OPCODE_SW_LAYER;
    layerMap["PERMUTE"]     = OPCODE_SW_LAYER;
    layerMap["SOFTMAX"]     = OPCODE_SW_LAYER;
    layerMap["L2NORM"]      = OPCODE_SW_LAYER;
    layerMap["INNERPROD"]   = OPCODE_SW_LAYER;
    layerMap["PRIORBOX"]    = OPCODE_SW_LAYER;
    layerMap["CROP"]        = OPCODE_SW_LAYER;

    layerMap["DECONV"]      = OPCODE_ZERO_LAYER;
    layerMap["DROPOUT"]     = OPCODE_ZERO_LAYER;
    layerMap["CRELU"]       = OPCODE_ZERO_LAYER;
    layerMap["RELU"]        = OPCODE_ZERO_LAYER;
    layerMap["RESHAPE"]     = OPCODE_ZERO_LAYER;
    layerMap["POWER"]       = OPCODE_ZERO_LAYER;
    layerMap["ARGMAX"]      = OPCODE_ZERO_LAYER;
    layerMap["SCALE"]       = OPCODE_ZERO_LAYER;
    layerMap["CONCAT"]      = OPCODE_ZERO_LAYER;
    layerMap["FLATTEN"]     = OPCODE_ZERO_LAYER;
}

int xiEval (std::string &layerspec_csv)
{
    //# Run layer by layer for performance estimate
    int nb_layers = getNumberOfLayers(layerspec_csv);
    if (nb_layers <= 0)
    {
        std::cout << std::endl << "[ERROR] No layers found ! " << std::endl;
        return -1;
    }

    FILE * csvFp = fopen(layerspec_csv.c_str(), "r");
    if (csvFp == NULL)
    {
        std::cout << "[ERROR] Couldn't open file ! " << std::endl;
        return -1;
    }

    outCsv = fopen ("neteval.csv", "w");
    if (outCsv == NULL)
    {
        std::cout << "[ERROR] Couldn't create file ! " << std::endl;
        return -1;
    }

    //# Initialize layer to opcode mapping
    initLayerMap();

    char layerType[64];

    std::cout << std::endl;
    //# Run for all layers
    for (int layer = 0; layer < nb_layers; ++layer)
    {
        //# Read CSV
        char buffer[1024];
        //# Get one line
        fgets(buffer, 1024, csvFp);
        //# Read layer type
        char *tok = strtok(buffer, ",");
        strcpy(layerType, tok);
        std::cout << "[EVALx] " << layerType << " ";

        //# Read Layer params
        std::vector<int> scalarParams;
        tok = strtok (NULL, ",");
        while (tok != NULL)
        {
            int val = atoi(tok);
            scalarParams.push_back(val);
            std::cout << scalarParams.back() << " ";
            tok = strtok (NULL, ",");
        }
        std::cout << std::endl;

        //# Run Network
        int err = runLayer(layerType, scalarParams);
        if(err != 0)
        {
            std::cerr << "[ERROR] Performance Eval Failed : Layer Index : " << layer << " Layer Type : " << layerType << std::endl;
            return -1;
        }
    }
    std::cout << std::endl;
    
    return 0;
}

/*
int main(void)
{   
    
    //    "/home/vkjain/Desktop/PoolingLayerSpecifications.tsv"
    
    std::string csvfile("/home/vkjain/Desktop/LayerSpecs.csv");
    int ret = fooFunc(csvfile);

    return 0;
}*/
