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
#include "../xtract/xtract_utility.hpp"

#include "xi_interface.hpp"

#include "../bufmgmt/xi_buf_mgmt.h"
#include "../init/xi_init.hpp"
#include "../scheduler/xi_scheduler.hpp"
#include "xi_readwrite_util.hpp"

//# Just for display
const char* getLayerType(kernel_type_e type)
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
	  case XCUSTOM:	return "Custom";
	  case XPACK: 	return "Pack";
   }
   return "Invalid Layer";
}

void *xiInit(char *dirpath,  char* prototxt,  char* caffemodel,
		  _io_layer_info *io_layer_info_ptr, int numImg_to_process, bool is_first_layer_in, std::string start_layer, std::string end_layer)
{

	chaihandle_t *chaihandle_info = new chaihandle_t;
	if(chaihandle_info == NULL)
	{
		fprintf(stderr, "Failed to read handle\n");
	}
	
	int layer1_or_not = is_first_layer_in;
	int en_batch_size_one;

	if(numImg_to_process == 2)
		en_batch_size_one = 0;
	else
		en_batch_size_one = 1;

	char* meanfile;
	int mean_type;
	char model_path[500];
	sprintf(model_path, "%s/%s", dirpath, caffemodel);

	char deploy_path[500];
	sprintf(deploy_path, "%s/%s", dirpath, prototxt);

	char mean_path[500];
	//sprintf(mean_path, "%s/%s", dirpath, meanfile);

	//# Graph 
	XGraph* graph = ParseCaffeNetwork(deploy_path, model_path, start_layer, end_layer, dirpath);//"", "pool5");
	//graph->setResizeShape(resize_h, resize_w);

	map < string, XLayer* >::iterator it_layer=graph->layers.begin();
	string quant_schem = it_layer->second->quantization_scheme;
	kernelInfo opt_descriptor(quant_schem);
	std::cout << "[INFOx] Graph generated"  << std::endl;
	std::cout << "[INFOx] Generating JobQueue" << std::endl;

	vector <XlayerData> xlayer_seq;
	xlayer_sequence_generator(xlayer_seq, graph,opt_descriptor, layer1_or_not);
	//xlayer_print(xlayer_seq);

	std::cout << "[INFOx] JobQueue generated"  << std::endl;
	std::cout << "[INFOx] Creating Memory" << std::endl;
	//graph->drawGraph("/tmp/optimized_graph.dot");
	//# Call buffer management block
	xiBuffCreate(xlayer_seq, chaihandle_info->JobQueue, chaihandle_info->ptrsList);

	//bufMgmt_print(xlayer_seq, chaihandle_info->JobQueue);

	std::cout << "\n[INFOx] Memory created" << std::endl;

	std :: cout << "[INFOx] Network Path : " << dirpath << std :: endl;

	std::cout << "\n[INFOx] Init Start : This may take a while ...";

	//# xChange Init
	initXChangeHost(dirpath, xlayer_seq, chaihandle_info->JobQueue, en_batch_size_one);

	uint16_t totalLayers = chaihandle_info->JobQueue[0].size();
	uint16_t lastLayerIdx = totalLayers - 1; 	//# Last layer index

	io_layer_info_ptr->inlayer_exectype   = chaihandle_info->JobQueue[0][0].kernexecType;
	io_layer_info_ptr->inlayer_sizebytes  = chaihandle_info->JobQueue[0][0].input_sizebytes;
	io_layer_info_ptr->outlayer_exectype  = chaihandle_info->JobQueue[0][lastLayerIdx].kernexecType;
	io_layer_info_ptr->outlayer_sizebytes = chaihandle_info->JobQueue[0][lastLayerIdx].output_sizebytes;

	if((io_layer_info_ptr->inlayer_exectype.compare("hardware") == 0) && (layer1_or_not != 1))
		io_layer_info_ptr->num_in_bufs = 2;
	else
		io_layer_info_ptr->num_in_bufs = 1;


	if(io_layer_info_ptr->outlayer_exectype.compare("hardware") == 0)
		io_layer_info_ptr->num_out_bufs = 2;
	else
		io_layer_info_ptr->num_out_bufs = 1;


	int *in_params = (int *)chaihandle_info->JobQueue[0][0].params;
	int *out_params = (int *)chaihandle_info->JobQueue[0][lastLayerIdx].params;

	if(layer1_or_not == 1)
	{
		io_layer_info_ptr->in_height = in_params[0];
		io_layer_info_ptr->in_width = in_params[1];
		io_layer_info_ptr->in_bw = chaihandle_info->JobQueue[0][0].qf_format.ip_bw;;
		io_layer_info_ptr->in_fbits =  chaihandle_info->JobQueue[0][0].qf_format.ip_fbits;;
		io_layer_info_ptr->in_channel = 3;
		io_layer_info_ptr->in_depth = 4;
		io_layer_info_ptr->th_in = chaihandle_info->JobQueue[0][0].float_params[0];
		io_layer_info_ptr->quant_scheme_flag = in_params[127];
	}

	kernel_type_e kerType = chaihandle_info->JobQueue[0][lastLayerIdx].kernType;
	io_layer_info_ptr->out_kerType = chaihandle_info->JobQueue[0][lastLayerIdx].kernType;
	io_layer_info_ptr->out_size = chaihandle_info->JobQueue[0][lastLayerIdx].output_size;

#if 0
	fprintf(stderr, "[INFOx] IO Layer Params\n");
	std :: cout << "inlayer_exectype : " <<  io_layer_info_ptr->inlayer_exectype << endl;
	std :: cout << "inlayer_sizebytes : " <<  io_layer_info_ptr->inlayer_sizebytes << endl;
	std :: cout << "outlayer_exectype : " <<  io_layer_info_ptr->outlayer_exectype << endl;
	std :: cout << "outlayer_sizebytes : " <<  io_layer_info_ptr->outlayer_sizebytes << endl;
	std :: cout << "num_in_bufs : " <<  io_layer_info_ptr->num_in_bufs << endl;
	std :: cout << "num_out_bufs : " <<  io_layer_info_ptr->num_out_bufs << endl;

	if(layer1_or_not == 1)
	{
		std :: cout << "in_height : " <<  io_layer_info_ptr->in_height << endl;
		std :: cout << "in_width : " <<  io_layer_info_ptr->in_width << endl;
		std :: cout << "in_channel : " <<  io_layer_info_ptr->in_channel << endl;
		std :: cout << "in_depth : " <<  io_layer_info_ptr->in_depth << endl;
		std :: cout << "in_fbits : " <<  io_layer_info_ptr->in_fbits << endl;
		std :: cout << "in_bw : " <<  io_layer_info_ptr->in_bw << endl;
		std :: cout << "th_in : " <<  io_layer_info_ptr->th_in << endl;
		std :: cout << "quant_scheme_flag : " <<  io_layer_info_ptr->quant_scheme_flag << endl;
	}

	std :: cout << "out_kerType : " <<  getLayerType(io_layer_info_ptr->out_kerType) << endl;
	std :: cout << "out_size : " <<  io_layer_info_ptr->out_size << endl;
#endif  //#if 0

	std::cout << "\n[INFOx] Init Done" << std::endl;

	return (void*)chaihandle_info;
}

void xiRelease(void *chaihandle)
{
	chaihandle_t *ptr = (chaihandle_t*) chaihandle;
	freeMemory(ptr->ptrsList);
	delete ptr;
}

//# Display Custom layer latency breakup
void getPerfInfo(void *handle)
{
	if(handle == NULL)
	{
		fprintf(stderr, "Failed to read handle\n");
	}

	chaihandle_t *chaihandle_info = (chaihandle*)handle;
	std::vector<xChangeLayer> *hwQueue = chaihandle_info->JobQueue;

	//# Number of layers to be scheduled
	uint16_t totalLayers = hwQueue[0].size();

	//# Number of layers to be scheduled
	// uint16_t totalLayers = hwQueue[0].size();
	if(totalLayers <= 0)
	{
		std::cerr << "\n[ERROR] Invalid Queue size !" << std::endl;
		return;
	}
	else
	{
#if ENABLE_ERROR_CHECKS
		std::cerr << "\n[INFOx] Total Layers : " << totalLayers << std::endl << std::endl;
#endif
	}

	long long int clock_start, clock_end, frequency;

#ifdef __SDSOC
	frequency = sds_clock_frequency();
#endif

	long long strat_clock = 0, end_clock = 0;
	double tot_time, cust_lat_acc = 0.0, pack_lat_acc = 0.0, unpack_lat_acc = 0.0;
	double acc = 0.0;

	std::vector<int> xcustomSeq;
	
	//# Initialize layer-wise sequence
	for(uint16_t idx = 0; idx < totalLayers; ++idx)
	{
		kernel_type_e layerType = hwQueue[0][idx].kernType;
		switch (layerType)
		{
		case XCUSTOM:	xcustomSeq.push_back(idx);	break;
		}
	}

	uint16_t txCustomLayers	= xcustomSeq.size();
	fprintf(stderr, "[INFOx] Total Number of Custom layers : %d\n\n", txCustomLayers);
		
	short last_cust_idx =  xcustomSeq[txCustomLayers-1];

	//# Flag used to accumulate pack/unpack layers used for custom layers
	bool accu_flag = true;

	//# loop through Num of images
	for(int j = 0; j < NUM_IMG; j++)
	{
		//# Compute latency custom layer and corresponding pack/unpack layers
		for(int i = 0; i < txCustomLayers; i++)
		{
			int idx =  xcustomSeq[i];
			fprintf(stderr, "[INFOx] Custom Layer - %d : %s\n", i+1, hwQueue[j][idx].layername.c_str());
			
			clock_start = hwQueue[j][idx].startclk;
			clock_end = hwQueue[j][idx].endclk;
			tot_time = (((double)(clock_end-clock_start)/(double)frequency*1000));

			cust_lat_acc = tot_time;

			if(hwQueue[j][idx-1].kernType == XPACK)
			{
				clock_start = hwQueue[j][idx-1].startclk;
				clock_end = hwQueue[j][idx-1].endclk;
				tot_time = (((double)(clock_end-clock_start)/(double)frequency*1000));

				unpack_lat_acc = tot_time;
			}

			if(hwQueue[j][idx+1].kernType == XPACK)
			{
				clock_start = hwQueue[j][idx+1].startclk;
				clock_end = hwQueue[j][idx+1].endclk;
				tot_time = (((double)(clock_end-clock_start)/(double)frequency*1000));

				pack_lat_acc = tot_time;
			}
			
			
			fprintf(stderr, "[PERFM] Unpack layer Latency : %f ms\n", unpack_lat_acc);
			fprintf(stderr, "[PERFM] Custom layer Latency : %f ms\n", cust_lat_acc);
			fprintf(stderr, "[PERFM] Pack   layer Latency : %f ms\n\n", pack_lat_acc);
		}

#if LayerWise_Latency

		FILE *fp = fopen("layerwise_latency.csv", "w");
		if(fp == NULL)
		{
			fprintf(stderr, "Failed to create file\n");
			return;
		}

		fprintf(fp, "layer_name, latency\n");

		//# Compute latency custom layer and corresponding pack/unpack layers
		for(int i = 0; i < totalLayers; i++)
		{
			clock_start = hwQueue[j][i].startclk;
			clock_end = hwQueue[j][i].endclk;
			tot_time = (((double)(clock_end-clock_start)/(double)frequency*1000));

			acc += tot_time;
			fprintf(fp, "%s, %f\n", hwQueue[j][i].layername.c_str(), tot_time);
			//fprintf(stderr, "%s : %f\n", hwQueue[j][i].layername.c_str(), tot_time);
		}
		fclose(fp);
#endif
	} // NUM_IMG

#if LayerWise_Latency
	fprintf(stderr, "Total Latency : %f\n", acc);
#endif

} // getPerfInfo
