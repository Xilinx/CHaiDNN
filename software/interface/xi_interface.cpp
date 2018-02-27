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

#include "../xtract/xgraph.hpp"
#include "../xtract/caffe_network_parser.hpp"
#include "../xtract/xgraph_opt.hpp"

#include "../bufmgmt/xi_buf_mgmt.h"
#include "../init/xi_init.hpp"
#include "../scheduler/xi_scheduler.hpp"

#include "xi_interface.hpp"

void xiInit( char *dirpath,  char* prototxt,  char* caffemodel,  char* meanfile, int mean_type, 
			   std::vector<xChangeLayer> (&jobQueue)[NUM_IMG], bufPtrs ptrsList, int resize_h, int resize_w)
{
	char model_path[500];
	sprintf(model_path, "%s/%s", dirpath, caffemodel);

	char deploy_path[500];
	sprintf(deploy_path, "%s/%s", dirpath, prototxt);

	char mean_path[500];
	sprintf(mean_path, "%s/%s", dirpath, meanfile);
	
	//# Graph 
	XGraph* graph = ParseCaffeNetwork(deploy_path, model_path, mean_path, FileMode(mean_type), dirpath);
    graph->setResizeShape(resize_h, resize_w);

	std::cout << "[INFOx] Graph generated"  << std::endl;
	std::cout << "[INFOx] Generating JobQueue" << std::endl;

	vector <XlayerData> xlayer_seq;
	xlayer_sequence_generator(xlayer_seq, graph);
	
	std::cout << "[INFOx] JobQueue generated"  << std::endl;
	std::cout << "[INFOx] Creating Memory" << std::endl;
	
	//# Call buffer management block
	xiBuffCreate(xlayer_seq, jobQueue, ptrsList);
	
	std::cout << "\n[INFOx] Memory created" << std::endl;
	std::cout << "\n[INFOx] Init Start : This may take a while ...";

	//# xChange Init
	initXChangeHost(dirpath, xlayer_seq, jobQueue);

	std::cout << "\n[INFOx] Init Done" << std::endl;
	
	return;
}

void xiRelease(bufPtrs ptrsList)
{
	freeMemory(ptrsList);
}
