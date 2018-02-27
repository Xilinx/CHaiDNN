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

#include <iostream>
#include <vector>

#include "xchange_structs.hpp"

typedef unsigned char uint8_t;

//# Just for display
const char* getLayerName(kernel_type_e type) 
{
   switch (type) 
   {
      case CONV: return "Convolution";
      case POOL: return "Pool";
	  case FC_LAYER: return "FC";
	  case SOFTMAX: return "SoftMax";
	  case DECONV: return "Deconv";
	  case NORM: return "Normalization";
	  case NMS: return "NMS";
	  case PERMUTE: return "Permute";
	  case CROP: return "Crop";
   }
   return "Invalid Layer";
}

//# Method to display all the scalar parameters for a layer
void xChangeLayer::DisplayParams(void)
{
	std::cout << "Layer Type : " << getLayerName(this->kernType) << std::endl;
	//# TODO: Some other prints can be added later
	for(uint8_t idx = 0; idx < MAX_PARAM_SIZE; ++idx)	
		std::cout << ((int*)params)[idx] << " ";
	std::cout << std::endl;
}

void xChangeLayer::DisplayParams(int layerIdx)
{
	std::cout << "** Layer " <<  layerIdx << " : " << getLayerName(this->kernType) << std::endl;
//	std::cout << "Previous layer id : " << previous[0].seqidx << std::endl;
	//# TODO: Some other prints can be added later
	for(uint8_t idx = 0; idx < MAX_PARAM_SIZE; ++idx)	
		std::cout << ((int*)params)[idx] << " ";
	std::cout << std::endl << std::endl;
}
