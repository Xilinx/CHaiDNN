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


#ifndef __CAFFELAYERMAP_HPP__
#define __CAFFELAYERMAP_HPP__

#include <map>

typedef map<string, string> mapStrStr;
// A Mapping from Caffe layer names to anonymoX layer names
//  { "Caffe Layer Name ,   "Xlayer name"   }

map<string, string> CaffeLayerMap; 

void fillCaffeLayerMap()
{
    CaffeLayerMap["Convolution"  ]  = "Convolution"  ;
    CaffeLayerMap["Deconvolution"]  = "Deconvolution";
    CaffeLayerMap["ReLU"         ]  = "ReLU"         ;
    CaffeLayerMap["Pooling"      ]  = "Pooling"      ;
    CaffeLayerMap["InnerProduct" ]  = "InnerProduct" ;
    CaffeLayerMap["Softmax"      ]  = "Softmax"      ;
    CaffeLayerMap["Input"        ]  = "Input"        ;
    CaffeLayerMap["Dropout"      ]  = "Dropout"      ;
    CaffeLayerMap["Concat"       ]  = "Concat"       ;
    CaffeLayerMap["Crop"         ]  = "Crop"         ;
    CaffeLayerMap["ArgMax"       ]  = "Argmax"       ;
    CaffeLayerMap["Flatten"      ]  = "Flatten"      ;
    CaffeLayerMap["Permute"      ]  = "Permute"      ;
    CaffeLayerMap["Normalize"    ]  = "L2Normalize"  ;
    CaffeLayerMap["PriorBox"     ]  = "PriorBox"     ;
    CaffeLayerMap["Reshape"      ]  = "Reshape"      ;
    CaffeLayerMap["LRN"          ]  = "LRN"          ;
    CaffeLayerMap["DetectionOutput"]= "NMS"          ;
    CaffeLayerMap["ConvolutionRistretto"  ]  = "Convolution"  ;
    CaffeLayerMap["FcRistretto"  ]  = "InnerProduct";
    CaffeLayerMap["Eltwise"  ]      = "Eltwise"     ;
    CaffeLayerMap["BatchNorm"  ]    = "BatchNorm"   ;
    CaffeLayerMap["Scale"  ]        = "Scale"       ;
    CaffeLayerMap["Power"  ]        = "Power"       ;
}

#endif          //  __CAFFELAYERMAP_HPP__
