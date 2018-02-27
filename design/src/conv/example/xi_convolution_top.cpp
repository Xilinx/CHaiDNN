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

#include "xi_conv_config.h"
#include "xi_conv_desc.h"
#include "../src/xi_convolution.hpp"



void XiConvolutionTop(gmem_weighttype *weights1,
			 gmem_weighttype *weights2,
			 gmem_outputtype *output1,
		     gmem_outputtype *output2,
		     gmem_inputtype_layerx *input_other1,
		     gmem_inputtype_layerx *input_other2,
		     gmem_inputtype_layer1 *input_1st,
		     gmem_biastype *bias,
			 gmem_inputtype_layer1 *inp_norm_2,
			 gmem_inputtype_layer1 *inp_norm_3,
			 gmem_inputtype_layer1 *istg_out1,
			 gmem_inputtype_layer1 *istg_out2,
		     int *scalar_conv_args)

{
#if 0
#pragma HLS interface m_axi port=weights1 		offset=slave bundle=gmem0 depth=20 latency=100 num_write_outstanding=32 num_read_outstanding=32 max_write_burst_length=64 max_read_burst_length=64
#pragma HLS interface m_axi port=weights2 		offset=slave bundle=gmem0 depth=20 latency=100 num_write_outstanding=32 num_read_outstanding=32 max_write_burst_length=64 max_read_burst_length=64
	#pragma HLS interface m_axi port=output1 		offset=slave bundle=gmem1 depth=460800 latency=100 num_write_outstanding=32 num_read_outstanding=32 max_write_burst_length=64 max_read_burst_length=64
#pragma HLS interface m_axi port=output2 		offset=slave bundle=gmem1 depth=460800 latency=100 num_write_outstanding=32 num_read_outstanding=32 max_write_burst_length=64 max_read_burst_length=64
#pragma HLS interface m_axi port=input_other1 	offset=slave bundle=gmem2 depth=6272  latency=100 num_write_outstanding=32 num_read_outstanding=32 max_write_burst_length=64 max_read_burst_length=64
#pragma HLS interface m_axi port=input_other2 	offset=slave bundle=gmem2 depth=6272  latency=100 num_write_outstanding=32 num_read_outstanding=32 max_write_burst_length=64 max_read_burst_length=64
#pragma HLS interface m_axi port=input_1st 	offset=slave bundle=gmem3 depth=230400  latency=100 num_write_outstanding=32 num_read_outstanding=32 max_write_burst_length=64 max_read_burst_length=64
#pragma HLS interface m_axi port=bias 			offset=slave bundle=gmem4 depth=4 latency=100 num_write_outstanding=32 num_read_outstanding=32 max_write_burst_length=64 max_read_burst_length=64
#pragma HLS interface m_axi port=scalar_conv_args 			offset=slave bundle=gmem4 depth=4 latency=100 num_write_outstanding=32 num_read_outstanding=32 max_write_burst_length=64 max_read_burst_length=64

#pragma HLS interface s_axilite port=weights1 bundle=control
#pragma HLS interface s_axilite port=weights2 bundle=control

#pragma HLS interface s_axilite port=output1 bundle=control
#pragma HLS interface s_axilite port=output2 bundle=control
#pragma HLS interface s_axilite port=input_other1 bundle=control
#pragma HLS interface s_axilite port=input_other2 bundle=control
#pragma HLS interface s_axilite port=input_1st bundle=control
#pragma HLS interface s_axilite port=bias bundle=control
#pragma HLS interface s_axilite port=scalar_conv_args bundle=control
#pragma HLS interface s_axilite port=return bundle=control
#endif 

	Convolution<XI_IN_W,XI_IN_H,XI_OUT_W,XI_OUT_H,XI_NUM_KERNELS,XI_FILTER_SIZE,XI_CONV_STRIDE,XI_POOL_STRIDE,XI_POOL_SIZE,XI_INPUT_PLANES,XI_NKPF>
	(weights1,weights2,output1,output2,input_other1,input_other2,input_1st,bias,inp_norm_2,inp_norm_3,istg_out1,istg_out2,scalar_conv_args);

	//return 0;
}

