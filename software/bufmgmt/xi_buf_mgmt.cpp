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

#include "xi_buf_mgmt.h"
#include "xi_buf_mgmt_utils.hpp"

//# Creates the memory of given size and returns the pointer
void* xiMemCreate(
		int size,
		mem_type_e mem_flag,
		bufPtrs &ptrList
)
{
	void *ptr;
#ifdef __SDSOC
	if(mem_flag == CACHEABLE)
	{
#if EN_ADDR_PRINT
		cout << "creating memory with sds_alloc_cacheable" << endl;
#endif
		ptr = (void*)sds_alloc_cacheable(size);
		ptrList.hwBufs.push_back(ptr);
	}
	else if(mem_flag == NON_CACHEABLE)
	{
#if EN_ADDR_PRINT
		cout << "creating memory with sds_alloc_non_cacheable" << endl;
#endif
		ptr = (void*)sds_alloc_non_cacheable(size);
		ptrList.hwBufs.push_back(ptr);
	}
	else
	{
#if EN_ADDR_PRINT
		cout << "creating memory with malloc" << endl;
#endif
		ptr = (void*)malloc(size);
		ptrList.swBufs.push_back(ptr);
	}
#else
	ptr = (void *)malloc(size);
	ptrList.swBufs.push_back(ptr);
#endif

	if(ptr == NULL)
	{
		cout << "Failed to create memory!" << endl;
		return NULL;
	}

	//# Initialize the buffer with zero
	//memset(ptr, 0, size);

	//cout << "accu size : " << ptrList.size << endl;
	ptrList.size += size;
#if EN_ADDR_PRINT
	cout << "xiMemCreate (base_addr, size) : " << ptr << " " << size << endl;
#endif
	return ptr;
}

//# check available stack and creates memory
void checkStack(
		int size,
		int& size_acc,
		void* &base_ptr,
		bufPtrs &ptrList,
		mem_type_e mem_flag
)
{
#if EN_STACK_PRINT
	cout << "size : " << size << endl;;
#endif
	int rem_space = STACK_SIZE - size_acc;
	if(size > rem_space)
	{
		if(size > STACK_SIZE)
		{
			base_ptr = (void*)xiMemCreate(size, mem_flag, ptrList);
		}
		else
		{
			base_ptr = (void*)xiMemCreate(STACK_SIZE, mem_flag, ptrList);
		}
		size_acc = 0;
#if EN_STACK_PRINT
		cout << "stack upadate - new base_addr : " << base_ptr << endl;
#endif
	}
}

//# Add offset to the address
template<typename T>
void *addOffset(
		void *base_addr,
		int offset
)
{
	void *ptr = (T*)base_addr + offset;
#if EN_OFF_PRINT
	cout << "addr (base_addr, off, fin_addr) : " << base_addr << " " << offset << " " << ptr << endl;
#endif
	return ptr;
}

//# Constant buffer Creation
void xiConstBufCreate(
		vector<XlayerData> xlayer_seq,
		vector<xChangeLayer> lay_obj[NUM_IMG],
		bufPtrs &ptrList
)
{
	unsigned int seq_len = xlayer_seq.size();

#if EN_PRINT
	cout << endl;
	cout << "start : xiConstBufCreate() " << endl;
	cout << "seq_len : " << seq_len << endl;
#endif

	mem_type_e mem_flag;

	//# Pool of memory
	void *base_ptr;
	base_ptr = (void*)xiMemCreate(STACK_SIZE, NON_CACHEABLE, ptrList);

	int n_size, size_acc = 0;

	//# offset storage
	vector<int> offset;

	//# Mean data shape
	//int mean_size = maxshapeToSize<float, 1>(xlayer_seq[0].meanShape);

	int mean_size = sizeof(float);

#if EN_CONST_PRINT
	cout << "mean shape size : " << xlayer_seq[0].meanShape.size() << endl;
	cout << "mean shape : ";
#endif
	for(unsigned int i = 0; i < xlayer_seq[0].meanShape.size(); i++)
	{
		mean_size *= xlayer_seq[0].meanShape[i];
#if EN_CONST_PRINT
		cout << xlayer_seq[0].meanShape[i] << " ";
#endif
	}
#if EN_CONST_PRINT
	cout << endl;
	cout << "Mean size in bytes : " << mean_size << endl;
#endif

	//# Total size for params and mean
	int total_size = mean_size + (seq_len*MAX_PARAM_SIZE*sizeof(int));

	//# Create memory for parameters
	void *param_buf;
	param_buf = (void*)xiMemCreate(total_size, NON_CACHEABLE, ptrList);
	int param_off = 0;

	void *ptr_mean = addOffset<BUF_ADDR_TYPE>(param_buf, 0);
	param_off += mean_size;

	int scalar_size = MAX_PARAM_SIZE*sizeof(HPARAM_TYPE);


	void *t_ptr;

	cout << endl;
	//# Travel through all layers
	for(unsigned int iter = 0; iter < seq_len; iter++)
	{
		xChangeLayer curLayer;
		string lay_mode = xlayer_seq[iter].layer_type;

		if(lay_mode.compare("hardware") == 0)
		{

			mem_flag = NON_CACHEABLE;
		}
		else
		{
			//cout << xlayer_seq[iter].layer_type << endl;
			mem_flag = MALLOC;
		}

#if EN_PRINT
		cout << "(Layer, Type) : " << xlayer_seq[iter].hw_ops -> name  << " , " << xlayer_seq[iter].hw_ops -> type << endl;
#endif

		//# initializing mean pointer
		curLayer.mean = ptr_mean;

#if EN_CONST_PRINT
		cout << "Params buffer creation" <<endl;
#endif

		//# initializing buffers for scalar parameters
		curLayer.params = addOffset<BUF_ADDR_TYPE>(param_buf, param_off);
		param_off += scalar_size;

#if EN_CONST_PRINT
		cout << "[DEBUG] Params buffer creation done!" << endl;
#endif

		int opCode =  xlayer_seq[iter].opcode;

#if EN_CONST_PRINT
		cout << "[DEBUG] opCode : " << opCode << endl;//" enum : " << OPCODE_BN << endl;
#endif


		if( xlayer_seq[iter].hw_ops -> type.compare("Convolution") == 0)
		{

			//# Checking for opcode
			if( (opCode == OPCODE_CRELU) || (opCode == OPCODE_CONV) || (opCode == OPCODE_FUSE_BN_CONV) || (opCode == OPCODE_3D_CONV))
			{
				//cout << "Layer Type : " << xlayer_seq[iter].hw_ops -> type << endl;
				int out_depth = xlayer_seq[iter].hw_ops -> conv_params -> M;
				int in_depth = xlayer_seq[iter].hw_ops -> conv_params -> N;
				int kernel_h = xlayer_seq[iter].hw_ops -> conv_params -> filter_h;
				int kernel_w = xlayer_seq[iter].hw_ops -> conv_params -> filter_w;
				int group = xlayer_seq[iter].hw_ops -> conv_params -> group;

				//# compute weights1 size
				n_size = convWeightsSize(out_depth, in_depth, kernel_h, kernel_w, group);
				n_size = n_size / WEIGHT_PORTS;
				//cout << "wts_size : " << n_size << endl;

				for(int l = 0; l < WEIGHT_PORTS; l++)
				{
					checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
					curLayer.wts_ptrs[l] = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);
					size_acc += n_size;
					offset.push_back(n_size);
				}

				//# Allocate space for scaling factors in case of offline quant mode
				//if(xlayer_seq[iter].hw_ops->quantization_scheme.compare("Xilinx") == 0)
				{
					//cout << "Inside offline block : " << endl;
					//# align_factor is 4, derived from port_width/data type (64-bit / 16-bit)
					int align_factor = ALIGN_FACTOR(HCONV_BIAS_PORT_WIDTH, HCONV_BIAS_DATA_WIDTH);
					n_size = convBiasSize<HCONV_BIAS_TYPE>(out_depth, align_factor);
					//# Space for scaling factor
					n_size = n_size * 4;
				}
#if 0
				else
				{
					//# compute bias size
					n_size = convBiasSize<HCONV_BIAS_TYPE>(out_depth, KER_PROC);
				}
#endif
				checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
				curLayer.bias_ptr = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);
				size_acc += n_size;
				offset.push_back(n_size);
			}

			//# Batchnorm Scale and shift
			if(( opCode == OPCODE_BN) || (opCode == OPCODE_FUSE_BN_CONV) )
			{
#if EN_CONST_PRINT
				cout << "[DEBUG] creating bias port for BN SnS" << endl;
#endif
				int in_depth = xlayer_seq[iter].hw_ops -> conv_params -> N;

				//# compute mean, variance and gamma size
				//TODO : Size check
				//# compute bias size
				n_size = convBiasSize<HMEAN_TYPE>(3*in_depth, KER_PROC);
				checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
				void *bias_ptr = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);
				curLayer.xtra_ptrs.push_back(bias_ptr);
				size_acc += n_size;
				offset.push_back(n_size);
			}

			if(opCode == OPCODE_3D_CONV)
			{
				//cout << "Layer Type : " << xlayer_seq[iter].hw_ops -> type << endl;
				int out_depth = xlayer_seq[iter].hw_ops -> conv_params -> M_3d;
				out_depth = out_depth/2;
				out_depth = AlignSize(out_depth, 32);

				int in_depth = xlayer_seq[iter].hw_ops -> conv_params -> N_3d;
				//in_depth = AlignSize(in_depth, 32);

				int kernel_h = xlayer_seq[iter].hw_ops -> conv_params -> filter_h_3d;
				int kernel_w = xlayer_seq[iter].hw_ops -> conv_params -> filter_w_3d;

				n_size = out_depth*in_depth*kernel_h*kernel_w;
				//cout << "3D-conv wts_size : " << n_size << endl;

				for(int l = 0; l < 2; l++)
				{
					checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
					void *wts_ptr = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);
					curLayer.xtra_ptrs.push_back(wts_ptr);
					size_acc += n_size;
					offset.push_back(n_size);
				}

				n_size = out_depth;
				//cout << "3D-conv bias_size : " << n_size << endl;

				for(int l = 0; l < 2; l++)
				{
					checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
					void *bias_ptr = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);
					curLayer.xtra_ptrs.push_back(bias_ptr);
					size_acc += n_size;
					offset.push_back(n_size);

				}
			}

			if(opCode == OPCODE_FC2CONV)
			{
				int num_output = xlayer_seq[iter].hw_ops -> conv_params -> M_3d;
				int num_input  = xlayer_seq[iter].hw_ops -> conv_params -> N_3d;

				//cout << "M :" << num_output << endl;
				//cout << "N :" << num_input << endl;

				//# compute weights1 size
				n_size = fcWgtsSize(num_output, num_input);

				//cout << "FC wgt size :" << n_size << endl;

				n_size = (n_size) / 2;
				for(int l = 0; l < 2; l++)
				{
					checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
					curLayer.in_ptrs[l] = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);
					size_acc += n_size;
					offset.push_back(n_size);
				}

				//# align_factor is 4, derived from port_width/data type (64-bit / 16-bit)
				int align_factor = ALIGN_FACTOR(HFC_BIAS_PORT_WIDTH, HFC_BIAS_DATA_WIDTH);
				n_size = fcBiasSize<HFC_BIAS_TYPE>(num_output, align_factor);

				//# Allocate extra Space for multiplication factors in case of offline quant mode
				if(xlayer_seq[iter].hw_ops->quantization_scheme.compare("Xilinx") == 0)
				{
					//cout << "Inside offline block : " << endl;
					n_size = n_size * 4;
				}

				//cout << "FC bias size :" << n_size << endl;
				checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
				curLayer.bias_ptr = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);
				size_acc += n_size;
			}

			//# Memory for bias and Scale factors for average pool
			if(opCode == OPCODE_AVRPOOL2CONV)
			{
				int num_output = xlayer_seq[iter].hw_ops -> conv_params -> M_3d;
				int num_input  = xlayer_seq[iter].hw_ops -> conv_params -> N_3d;

				//fprintf(stderr, "num_op : %d num_ip : %d\n", num_output, num_output);

				int align_factor = ALIGN_FACTOR(HCONV_BIAS_PORT_WIDTH, HCONV_BIAS_DATA_WIDTH);
				//# Include scale factor + Bias and there are two ptrs used(2P/2)
				n_size = AlignSize(num_output, align_factor);
				n_size = n_size*sizeof(HCONV_BIAS_TYPE);

				checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
				void *sf_ptr = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);
				curLayer.xtra_ptrs.push_back(sf_ptr);//addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc));
				size_acc += n_size;

				checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
				sf_ptr = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);
				curLayer.xtra_ptrs.push_back(sf_ptr);
				size_acc += n_size;

			}
		}
		else if( xlayer_seq[iter].hw_ops -> type.compare("Pooling") == 0)
		{

			//# Checking for opcode
			if(opCode == OPCODE_SEPARABLE_CONV)
			{
				//cout << "Layer Type : " << xlayer_seq[iter].hw_ops -> type << endl;
				int out_depth = xlayer_seq[iter].hw_ops -> conv_params -> M;
				int in_depth = xlayer_seq[iter].hw_ops -> conv_params -> N;
				int kernel_h = xlayer_seq[iter].hw_ops -> conv_params -> filter_h;
				int kernel_w = xlayer_seq[iter].hw_ops -> conv_params -> filter_w;
				int group = xlayer_seq[iter].hw_ops -> conv_params -> group;

				//# compute weights1 size
				n_size = convWeightsSize(out_depth, in_depth, kernel_h, kernel_w, group);
				//cout << "wts_size : " << n_size << endl;

				//for(int l = 0; l < WEIGHT_PORTS; l++)
				{
					checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
					curLayer.wts_ptrs[0] = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);
					size_acc += n_size;
					offset.push_back(n_size);
				}

			}
		}
		else if(xlayer_seq[iter].hw_ops -> type.compare("InnerProduct") == 0)
		{
			//cout << "Layer Type : " << xlayer_seq[iter].hw_ops -> type << endl;

			int num_output = xlayer_seq[iter].hw_ops -> fc_params -> M;
			int num_input  = xlayer_seq[iter].hw_ops -> fc_params -> N;

			//	cout << "M :" << num_output << endl;
			//	cout << "N :" << num_input << endl;

			if(lay_mode.compare("hardware") == 0)
			{
				//# compute weights1 size
				n_size = fcWgtsSize(num_output, num_input);

				n_size = n_size / 2;

				for(int l = 0; l < 2; l++)
				{
					checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
					curLayer.wts_ptrs[l] = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);
					size_acc += n_size;
					offset.push_back(n_size);
				}

				int align_factor = ALIGN_FACTOR(HFC_BIAS_PORT_WIDTH, HFC_BIAS_DATA_WIDTH);
				//# compute bias size
				n_size = fcBiasSize<HFC_BIAS_TYPE>(num_output, align_factor);
				//cout << "bias size :" << n_size << endl;
				checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
				curLayer.bias_ptr = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);
				size_acc += n_size;
			}
			else
			{
				//# compute weights size
				n_size = num_output*num_input*sizeof(SW_FC_DATA_TYPE);
				void *ptr = xiMemCreate(n_size, mem_flag, ptrList);
				curLayer.wts_ptrs[0] = ptr;

				//# compute bias size
				n_size = num_output*sizeof(SW_FC_DATA_TYPE);
				ptr = xiMemCreate(n_size, mem_flag, ptrList);
				curLayer.bias_ptr = ptr;
			}
		}
		else if( xlayer_seq[iter].hw_ops -> type.compare("Deconvolution") == 0)
		{
			//cout << "Layer Type : " << xlayer_seq[iter].hw_ops -> type << endl;

			int out_depth = xlayer_seq[iter].hw_ops -> deconv_params -> M;
			int in_depth  = xlayer_seq[iter].hw_ops -> deconv_params -> N;
			int fil_h	= xlayer_seq[iter].hw_ops -> deconv_params -> filter_h;
			int fil_w	= xlayer_seq[iter].hw_ops -> deconv_params -> filter_w;

			//# compute weights size
			n_size = out_depth*in_depth*fil_h*fil_w*sizeof(HDECONV_WGT_TYPE);
			checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
			curLayer.wts_ptrs[0] = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);//(HDECONV_WGT_TYPE*)base_ptr + size_acc;
			size_acc += n_size;
			offset.push_back(n_size);

			//# compute bias size
			n_size = in_depth*sizeof(HDECONV_BIAS_TYPE);
			checkStack(n_size, size_acc, base_ptr, ptrList, mem_flag);
			curLayer.bias_ptr = addOffset<BUF_ADDR_TYPE>(base_ptr, size_acc);
			size_acc += n_size;
			offset.push_back(n_size);
		}
		else if (xlayer_seq[iter].hw_ops -> type.compare("L2Normalize") == 0)
		{
			//# Create malloc buffers because norm is Software module
			//# Gamma buffer creation
			//TODO : compute size
			//cout << "norm : " << mem_flag << endl;
			if(xlayer_seq[iter].hw_ops->quantization_scheme.compare("Xilinx") == 0)
			{
				n_size = shapeToSize<HNORM_GAMMA_FLOAT_TYPE, KER_PROC>(xlayer_seq[iter].hw_ops -> bottomShape);
			}
			else
			{
				n_size = shapeToSize<HNORM_GAMMA_TYPE, KER_PROC>(xlayer_seq[iter].hw_ops -> bottomShape);
			}
			t_ptr = xiMemCreate(n_size, mem_flag, ptrList);
			curLayer.wts_ptrs[0] = addOffset<BUF_ADDR_TYPE>(t_ptr, 0);

		}
		else if (xlayer_seq[iter].hw_ops -> type.compare("Softmax") == 0)
		{

			//fprintf(stderr,"softmax");
			//TODO : check sw/hw
			//if(xlayer_seq[iter].hw_ops -> device == SW)
			{

			}
		}
		else if (xlayer_seq[iter].hw_ops -> type.compare("NMS") == 0)
		{
			//cout << "NMS : " << mem_flag << endl;
			//# Memory for Prior box
			n_size = maxshapeToSize<HNMS_PBOX_TYPE>(xlayer_seq[iter].hw_ops -> nms_params -> pboxShape);
			//cout << "nms size : " << n_size << endl;
			t_ptr = xiMemCreate(n_size, mem_flag, ptrList);
			curLayer.wts_ptrs[0] = addOffset<BUF_ADDR_TYPE>(t_ptr, 0);

			//# Memory for variance
			n_size = maxshapeToSize<HNMS_VAR_TYPE>(xlayer_seq[iter].hw_ops -> nms_params -> varShape);
			t_ptr = xiMemCreate(n_size, mem_flag, ptrList);
			curLayer.wts_ptrs[1] = addOffset<BUF_ADDR_TYPE>(t_ptr, 0);
		}

		lay_obj[0].push_back(curLayer);

#if EN_CONSOLE_TXT_BUFMGMT_DBG
		fprintf(stderr,".");
#endif
	} // for seg_layers

#if EN_PRINT
	cout << "Constant buffers allocated for image-0" << endl;
#endif

	//# Copying the pointers for other images
	for (int i = 1; i < NUM_IMG; i++)
	{
		//# Travel through all layers
		for(unsigned int iter = 0; iter < seq_len; iter++)
		{
			//curLayer = lay_obj[0][iter];
			lay_obj[i].push_back(lay_obj[0][iter]);
#if EN_CONSOLE_TXT_BUFMGMT_DBG
			fprintf(stderr,".");
#endif
		}
	}
#if EN_PRINT
	cout << "Constant buffers allocated for image-1" << endl;
#endif
}

//# IO buffer Creation
void xiIOBuffCreate(
		vector<XlayerData> xlayer_seq,
		vector<xChangeLayer> lay_obj[NUM_IMG],
		bufPtrs &ptrList
)
{
#if EN_PRINT
	cout << endl;
	cout << "start : xiIOBuffCreate () " << endl;
	cout << "Number of layers : " << xlayer_seq.size() << endl;
#endif

	//# number of layers in the network
	unsigned int seq_len = xlayer_seq.size();

	string cur_handle;
	void *base_ptr;

	//# compute size of maximum handle shape in the network
	int max_size = maxshapeToSize<HCONV_OUT_TYPE>(xlayer_seq[0].ip_blob[0].handle_shape);
	//cout << "input max_size : " << max_size << endl;

	//# Offset & Position for concatination
	vector <io_peers> offPair;

	int out_offset, in_offset;

	//# dummy pointer to hold address of input image - in_ptrs[2]
	void *ptr_l1 = NULL;

	//# flags for enabling only first layers
	bool fc_flag = 0, sx_flag = 0, deconv_flag = 0;

	//# Memory allocation across images
	for(int img_id = 0; img_id < NUM_IMG; img_id++)
	{
#if EN_PRINT
		cout << endl;
		cout << "img : " << img_id << endl;
#endif

		fc_flag = 0; sx_flag = 0; deconv_flag = 0;

		//# map used to pair handle and i/o pointers
		//# hardwarehandle is used as key & ptr_pair is the mapped type
		map<string, ptr_pair> io_map;

		//# dummy object
		xChangeLayer curLayer;


#if EN_PRINT
		cout << endl;
#endif
		//# Travel through all layers
		for(unsigned int iter = 0; iter < seq_len; iter++)
		{
#if EN_PRINT
			cout << endl << "/////////////////////////////// layer : " << iter << endl;
			cout << "(Layer, Type, HW/SW) " << xlayer_seq[iter].hw_ops -> name << ", " << xlayer_seq[iter].hw_ops -> type << ", " << xlayer_seq[iter].layer_type << endl;
			//cout << "handle size (ip, op): " << xlayer_seq[iter].ip_blob.size() << " " << xlayer_seq[iter].op_blob.size() << endl;
			for(unsigned int i = 0; i < xlayer_seq[iter].ip_blob.size(); i++)
			{
				cout << xlayer_seq[iter].ip_blob[i].handle << " - " << xlayer_seq[iter].ip_blob[i].mem_type << " , ";
			}
			for(unsigned int i = 0; i < xlayer_seq[iter].op_blob.size(); i++)
			{
				cout << xlayer_seq[iter].op_blob[i].handle << " - " << xlayer_seq[iter].op_blob[i].mem_type;
			}
			cout << endl << endl;
#endif

			////////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////		Input Blobs	///////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////

			//# Travel across input blobs
			for(unsigned int l_ib = 0; l_ib  < xlayer_seq[iter].ip_blob.size(); l_ib ++)
			{

				HBlob in_blob =  xlayer_seq[iter].ip_blob[l_ib];

				//# input handle from xTract
				cur_handle = in_blob.handle;

				//# Type of memory (SDS/Malloc)
				mem_type_e mem_flag;
				_datatype_e xdata_type_flag=in_blob.io_data_type;
				if(in_blob.mem_type.compare("non_cacheable") == 0)
				{
					mem_flag = NON_CACHEABLE;
				}
				else if(in_blob.mem_type.compare("cacheable") == 0)
				{
					mem_flag = CACHEABLE;
				}
				else
					mem_flag = MALLOC;

				//# input peers shape
				//TODO :: Enable after support
				in_offset = peersToShape<HCONV_OUT_TYPE, KER_PROC>(xlayer_seq[iter].ip_peers_shapes);
				//cout << "in_offset : " << in_offset << endl;

				//# Checking the existence of input handle in the map
				//# For every new handle, creates new memory
				//# If already exists no need to create
				if(io_map.count(cur_handle) == false)
				{
#if EN_IO_PRINT
					cout << "ip_handle : " << cur_handle << endl;
#endif

					//# Creating unique memory for input image data
					if(cur_handle == "data")
					{
						//# TODO : Using 3 Ports for first layer of convolution
						int num_inports = 3;
						for(int i = 0; i < num_inports; i++)
						{
							base_ptr = (void*)xiMemCreate(max_size*XBATCH_SIZE, mem_flag, ptrList);
							io_map[cur_handle].ptr[i] = addOffset<BUF_ADDR_TYPE>(base_ptr, 0);

							if(i == 2)
								ptr_l1 = io_map[cur_handle].ptr[i];
#if EN_IO_PRINT
							cout << "io_map[cur_handle].ptr[i] : " << io_map[cur_handle].ptr[i] << endl;
#endif
						}
					}
					else
					{
						//						int h_size = XBATCH_SIZE*halfshapeToSize<HCONV_OUT_TYPE>(xlayer_seq[0].ip_blob[0].handle_shape); //TODO kvraju
						int h_size;

						if(xdata_type_flag ==XI_INT8 || xdata_type_flag == XI_UINT8){
							h_size= XBATCH_SIZE*halfshapeToSize<char>(in_blob.handle_shape);
						}else if(xdata_type_flag ==XI_INT16 || xdata_type_flag == XI_UINT16) {
							h_size= XBATCH_SIZE*halfshapeToSize<short>(in_blob.handle_shape);
						}else if(xdata_type_flag ==XI_INT32|| xdata_type_flag == XI_UINT32){
							h_size= XBATCH_SIZE*halfshapeToSize<int>(in_blob.handle_shape);
						}else{
							if(xdata_type_flag ==XI_FLOAT){

								h_size= XBATCH_SIZE*halfshapeToSize<float>(in_blob.handle_shape);
							}
						}
						base_ptr = (void*)xiMemCreate(h_size*HCONV_OUT_PORTS, mem_flag, ptrList);

						int t_off = 0;

						//# Each layer is having maximum 2 inputs
						for(int i = 0; i < HCONV_OUT_PORTS; i++)
						{
							io_map[cur_handle].ptr[i] = addOffset<BUF_ADDR_TYPE>(base_ptr, t_off);
							t_off += h_size;
						}
					}
				}

				vector<vector<int> > inLay_shape = xlayer_seq[iter].hw_ops-> bottomShape;

#if EN_IO_PRINT
				cout << "in_ptr : ";
#endif

				int opCode = xlayer_seq[iter].opcode;

				for(int j = 0; (j < INPUT_PORTS) && (l_ib == 0); j++)
				{
					//# Filling Dummy pointers & these are not being used by kernel
					//# This is to avoid the sds_alloc error
					//# Required for convolution only
					if (xlayer_seq[iter].hw_ops -> type == "Convolution")
					{
						if(opCode == OPCODE_FC2CONV)
						{
							if(j > 1)
								lay_obj[img_id][iter].in_ptrs[j] = ptr_l1;
						}
						else
							lay_obj[img_id][iter].in_ptrs[j] = ptr_l1;
					}
				}



#if EN_IO_PRINT
				cout << "opcode : " << opCode << endl;
#endif

				//# initializing pointers
				//# Each layer is having maximum 2 inputs
				//# Fixed the loop cnt, prev is INPUT_PORTS-2
				for(int j = 0; j < 3; j++)
				{
					int idx = 0;

					//# assigning other inputs pointers if blob size is > 1
					if(j < HCONV_OUT_PORTS)
					{
						if(l_ib == 0)
						{
							//# Input ptrs for SOS opcode
							if( opCode == OPCODE_ELTWISE )
							{
								if(j == 0)
									lay_obj[img_id][iter].bias_ptr = addOffset<BUF_ADDR_TYPE>(io_map[cur_handle].ptr[j], in_offset); //io_map[cur_handle].ptr[j];
								else
									lay_obj[img_id][iter].in_ptrs[j+3] = addOffset<BUF_ADDR_TYPE>(io_map[cur_handle].ptr[j], in_offset); //io_map[cur_handle].ptr[j];
							}
							else if(opCode == OPCODE_FC2CONV)
							{
								//# Assign input to weights pointers
								lay_obj[img_id][iter].wts_ptrs[j] = addOffset<BUF_ADDR_TYPE>(io_map[cur_handle].ptr[j], in_offset);

								if(j == 1)
								{
									//# 16 is batch size and is fixed for FC
									int in_size = 16*shapeToSize<HFC_IN_TYPE, KER_PROC>(inLay_shape)/2;
									//cout << "FC in buf size : " << in_size << endl;

									//# Intermediate buffers created for FC layer
									lay_obj[img_id][iter].in_ptrs[2]  = (void*)xiMemCreate(in_size, NON_CACHEABLE, ptrList);
									lay_obj[img_id][iter].in_ptrs[3]  = lay_obj[img_id][iter].in_ptrs[2];

									//# For support of KER_PROC=16
									//# loading the dummy addresses for the interface
									for(int l = 2; l < WEIGHT_PORTS; l++)
									{
										lay_obj[img_id][iter].wts_ptrs[l] = lay_obj[img_id][iter].in_ptrs[2];
									}

									//lay_obj[img_id][iter].wts_ptrs[2] = (void*)xiMemCreate(in_size, NON_CACHEABLE, ptrList);
									//lay_obj[img_id][iter].wts_ptrs[3] = (void*)xiMemCreate(in_size, NON_CACHEABLE, ptrList);
								}
							}
							else
							{
								if ( (opCode == OPCODE_LRN_INTER_PNS) || (opCode == OPCODE_BN) || (opCode == OPCODE_FUSE_BN_CONV) ) {
									idx = j+2;
								} else {
									idx = j;
								}
								lay_obj[img_id][iter].in_ptrs[idx] = addOffset<BUF_ADDR_TYPE>(io_map[cur_handle].ptr[j], in_offset); //io_map[cur_handle].ptr[j];
							}
						}
						else
						{
							if( (opCode == OPCODE_LRN_INTER_PNS))
							{
								if(j == 0)
									lay_obj[img_id][iter].bias_ptr = addOffset<BUF_ADDR_TYPE>(io_map[cur_handle].ptr[j], in_offset); //io_map[cur_handle].ptr[j];
								else
									lay_obj[img_id][iter].in_ptrs[j+3] = addOffset<BUF_ADDR_TYPE>(io_map[cur_handle].ptr[j], in_offset); //io_map[cur_handle].ptr[j];
							}
							else
							{
								lay_obj[img_id][iter].in_ptrs[j+(l_ib*2)] = addOffset<BUF_ADDR_TYPE>(io_map[cur_handle].ptr[j], in_offset); //io_map[cur_handle].ptr[j];
							}
						}
					}

					//# Creating intermediate buffers required by host
					//# position : in_ptrs[2]
					if( (l_ib == 0) && (j == 2))
					{
						if( (xlayer_seq[iter].hw_ops -> type == "InnerProduct") && (fc_flag == false) )
						{
							fc_flag = true;

							if(xlayer_seq[iter].layer_type.compare("hardware") == 0)
							{
								int in_size = XBATCH_SIZE*shapeToSize<HFC_IN_TYPE, KER_PROC>(inLay_shape);

								//# Creating sds_cachable memory
								base_ptr = (void*)xiMemCreate(in_size, NON_CACHEABLE, ptrList);
							}
							else
							{
								int in_size = XBATCH_SIZE*shapeToSize<SW_FC_DATA_TYPE, KER_PROC>(inLay_shape);

								//# Creating sds_cachable memory
								base_ptr = (void*)xiMemCreate(in_size, MALLOC, ptrList);
							}

							lay_obj[img_id][iter].in_ptrs[j] = base_ptr;
						}
						else if( (xlayer_seq[iter].hw_ops -> type == "Softmax") && (sx_flag == false) )
						{
							sx_flag = true;

							int in_size = XBATCH_SIZE*shapeToSize<HSOFTMAX_IN_TYPE, 1>(inLay_shape);
							base_ptr = (void*)xiMemCreate(in_size, NON_CACHEABLE, ptrList);
							lay_obj[img_id][iter].in_ptrs[j] = base_ptr;
						}
						else if( (xlayer_seq[iter].hw_ops -> type == "Deconvolution") && (deconv_flag == false) )
						{
							deconv_flag = true;

							//TODO :: check the shape once
							//cout << " deconv : " << endl;
							int in_size = XBATCH_SIZE*shapeToSize<HDECONV_IN_TYPE, 16>(inLay_shape);
							base_ptr = (void*)xiMemCreate(in_size, NON_CACHEABLE, ptrList);
							lay_obj[img_id][iter].in_ptrs[j] = base_ptr;
						}
					}
#if EN_IO_PRINT
					cout << lay_obj[img_id][iter].in_ptrs[j] << " ";
#endif
				}
#if EN_IO_PRINT
				cout << endl;
#endif
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////		Output Blobs	///////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////

			//# Travel across output blobs
			for(unsigned int l_ib = 0; l_ib  < xlayer_seq[iter].op_blob.size(); l_ib ++)
			{
				HBlob op_blob = xlayer_seq[iter].op_blob[l_ib];
				int offset[2];
				offset[0] = 0;
				offset[1] = 0;

				//# checking for output blob
				cur_handle = op_blob.handle;

				//# Type of memory (SDS/Malloc)
				mem_type_e mem_flag;

				_datatype_e xdata_type_flag=op_blob.io_data_type;

				if(op_blob.mem_type.compare("non_cacheable") == 0)
				{
					mem_flag = NON_CACHEABLE;
				}
				else if(op_blob.mem_type.compare("cacheable") == 0)
				{
					//# Checking for size 512KB //4MB
					if(max_size < CACHE_LIMIT)//4194304)
					{
						mem_flag = CACHEABLE;
						//cout << "cache enter ::::::::::::::::" << endl;
					}
					else
					{
						mem_flag = NON_CACHEABLE;//CACHEABLE;
					}
				}
				else
					mem_flag = MALLOC;

				int opCode = xlayer_seq[iter].opcode;

				//# ouput peers shape
				//# this is the offset required for output buffer in concatenation buffer
				if (xlayer_seq[iter].hw_ops -> type.compare("Permute") == 0)
				{
					out_offset = peersToShapeNoalign<HPERMUTE_OUT_TYPE, KER_PROC>(xlayer_seq[iter].op_peers_shapes);
				}
				else
				{
					if(XBATCH_SIZE==2)
					{
						out_offset = peersToShape<HCONV_OUT_TYPE, KER_PROC>(xlayer_seq[iter].op_peers_shapes);
					}
					else  //if(XBATCH_SIZE==1)
					{
						out_offset = peersToShapeAlign16<HCONV_OUT_TYPE, KER_PROC>(xlayer_seq[iter].op_peers_shapes, offset);
					}
				}

				//fprintf(stderr,"out_offset = %d\n", out_offset);

				//# Checking the existence of input handle in the map
				//# For every new handle, creates new memory
				//# If already exists no need to create
				//# TODO : Reuse
				if(io_map.count(cur_handle) == false)
				{
#if EN_IO_PRINT
					cout << "op_handle : " << cur_handle << endl;
#endif
					//TODO : deconv data type condideration
					//					int h_size = halfshapeToSize<HCONV_OUT_TYPE>(xlayer_seq[0].ip_blob[0].handle_shape); //TODO kvraju

					//					int h_size = halfshapeToSize<float>(xlayer_seq[0].ip_blob[0].handle_shape); // TODO kvraju
					int h_size;
					if((xdata_type_flag ==XI_INT8) ||( xdata_type_flag == XI_UINT8)){
						h_size= halfshapeToSize<char>(op_blob.handle_shape);
					}else if((xdata_type_flag ==XI_INT16) || (xdata_type_flag ==XI_UINT16) ) {
						h_size= halfshapeToSize<short>(op_blob.handle_shape);
					}else if((xdata_type_flag ==XI_INT32) || (xdata_type_flag ==XI_UINT32)){
						h_size= halfshapeToSize<int>(op_blob.handle_shape);
					}else{
						if(xdata_type_flag ==XI_FLOAT){

							h_size= halfshapeToSize<float>(op_blob.handle_shape);
						}
					}
					int tot_size = h_size * XBATCH_SIZE;
					//fprintf(stderr, "out size:%d\n", tot_size);

					base_ptr = (void*)xiMemCreate(tot_size*HCONV_OUT_PORTS, mem_flag, ptrList);

					int t_off = 0;
					//# OUTPUT_PORTS are fixed to 2
					for(int i = 0; i < HCONV_OUT_PORTS; i++)
					{
						io_map[cur_handle].ptr[i] = addOffset<BUF_ADDR_TYPE>(base_ptr, t_off);
						t_off += (tot_size);
					}
				}

				//# initializing pointers
				//# OUTPUT_PORTS are fixed to 2
				if(offset[0]==offset[1])
				{
					for(int j = 0; j < HCONV_OUT_PORTS; j++)
					{
						lay_obj[img_id][iter].out_ptrs[j] = addOffset<BUF_ADDR_TYPE>(io_map[cur_handle].ptr[j], out_offset);
#if EN_IO_PRINT
						cout << "(base ptr, updated_ptr, offset) : " << io_map[cur_handle].ptr[j] << " " << lay_obj[img_id][iter].out_ptrs[j] << " " << out_offset <<  endl;
#endif
					}

					for(int j = HCONV_OUT_PORTS, k = 0; j < OUTPUT_PORTS; j++, k++)
					{
						lay_obj[img_id][iter].out_ptrs[j] = lay_obj[img_id][iter].out_ptrs[k];
#if EN_IO_PRINT
						cout << "(base ptr, updated_ptr, offset) : " << io_map[cur_handle].ptr[j] << " " << lay_obj[img_id][iter].out_ptrs[j] << " " << out_offset <<  endl;
#endif
					}
				}
				else
				{
					lay_obj[img_id][iter].out_ptrs[0] = addOffset<BUF_ADDR_TYPE>(io_map[cur_handle].ptr[1], offset[0]);
					lay_obj[img_id][iter].out_ptrs[1] = addOffset<BUF_ADDR_TYPE>(io_map[cur_handle].ptr[0], offset[1]);
#if EN_IO_PRINT
					cout << "(base ptr, updated_ptr, offset) : " << io_map[cur_handle].ptr[1] << " " << lay_obj[img_id][iter].out_ptrs[0] << " " << offset[1] <<  endl;
					cout << "(base ptr, updated_ptr, offset) : " << io_map[cur_handle].ptr[0] << " " << lay_obj[img_id][iter].out_ptrs[1] << " " << offset[0] <<  endl;
#endif
				}

				////////////////////////////////////////////////////////////////////////////////////////////////////
				///////////////////		Extra Buffers	///////////////////////////////////////////////////////////////
				////////////////////////////////////////////////////////////////////////////////////////////////////

				int n_size = 0;

				//# Extra pointers need for individual kernel
				//# Decided to create extra ponter and not to integrate with others
				if (xlayer_seq[iter].hw_ops -> type.compare("L2Normalize") == 0)
				{
					//# sumBuffer creation
					//TODO : compute size
					n_size = XBATCH_SIZE*shapeToSize<HNORM_SUM_TYPE, KER_PROC>(xlayer_seq[iter].hw_ops -> bottomShape);
					base_ptr = xiMemCreate(n_size, MALLOC, ptrList);
					lay_obj[img_id][iter].xtra_ptrs.push_back(addOffset<BUF_ADDR_TYPE>(base_ptr, 0));
				}
				else if (xlayer_seq[iter].hw_ops -> type.compare("Permute") == 0)
				{
					//# Create malloc buffers because permute is Software module
					//# Extra Output buffers
					//TODO : compute size
					n_size = XBATCH_SIZE*shapeToSize<HPERMUTE_IN_TYPE, KER_PROC>(xlayer_seq[iter].hw_ops -> bottomShape);
					base_ptr = xiMemCreate(n_size, mem_flag, ptrList);
					lay_obj[img_id][iter].xtra_ptrs.push_back(addOffset<BUF_ADDR_TYPE>(base_ptr, 0));

					//# sumBuffer creation
					//# Extra Output buffers
					//TODO : compute size
					n_size = XBATCH_SIZE*shapeToSize<HPERMUTE_IN_TYPE, KER_PROC>(xlayer_seq[iter].hw_ops -> bottomShape);
					base_ptr = xiMemCreate(n_size, mem_flag, ptrList);
					lay_obj[img_id][iter].xtra_ptrs.push_back( addOffset<BUF_ADDR_TYPE>(base_ptr, 0) );
				}
				else if( xlayer_seq[iter].hw_ops -> type == "Softmax" )
				{
					//# Softmax sum buffer creation
					n_size = XBATCH_SIZE*shapeToSize<HSOFTMAX_IN_TYPE, 1>(xlayer_seq[iter].hw_ops -> bottomShape);

					base_ptr = xiMemCreate(n_size, mem_flag, ptrList);
					lay_obj[img_id][iter].xtra_ptrs.push_back(addOffset<BUF_ADDR_TYPE>(base_ptr, 0));
				}
				else if (xlayer_seq[iter].hw_ops -> type.compare("NMS") == 0)
				{
					n_size = XBATCH_SIZE*maxshapeToSize<HNMS_OUT_TYPE>(xlayer_seq[iter].hw_ops -> nms_params -> pboxShape);

					//# Memory for nms_label
					base_ptr = xiMemCreate(n_size, mem_flag, ptrList);
					lay_obj[img_id][iter].xtra_ptrs.push_back( addOffset<BUF_ADDR_TYPE>(base_ptr, 0) );

					//# Memory for nms_box
					base_ptr = xiMemCreate(n_size, mem_flag, ptrList);
					lay_obj[img_id][iter].xtra_ptrs.push_back( addOffset<BUF_ADDR_TYPE>(base_ptr, 0) );

					//# Memory for nms_score
					base_ptr = xiMemCreate(n_size, mem_flag, ptrList);
					lay_obj[img_id][iter].xtra_ptrs.push_back( addOffset<BUF_ADDR_TYPE>(base_ptr, 0) );
				}

			} // op_blob loop

#if EN_PRINT
			cout << "/////////////////////////////// end " << endl;
#endif

		} // iter : seq_len
	} // NUM_IMG
}
// xiIOBuffCreate

//# Freeing Memory pointers
void freeMemory(bufPtrs &ptrList)
{
#if EN_FREE_PRINT
	cout << endl;
	cout << "start : freeMemory() " << endl;

	cout << "swbufs count : " << ptrList.swBufs.size() << endl;
	cout << "hwbufs count : " << ptrList.hwBufs.size() << endl;

	cout << "Total allocated memory : " << ptrList.size/(1024.0f*1024.0f) << endl;

	cout << "Freeing software memory ... " << endl;
#endif
	for(unsigned int i = 0; i < ptrList.swBufs.size(); i++)
	{
		//cout << "cnt :" << i << endl;
		free(ptrList.swBufs[i]);
		ptrList.swBufs[i] = NULL;
	}
#if EN_FREE_PRINT
	cout << "Memory Released" << endl;
#endif

#ifdef __SDSOC
#if EN_FREE_PRINT
	cout << "Freeing hardware memory ... " << endl;
#endif
	for(unsigned int i = 0; i < ptrList.hwBufs.size(); i++)
	{
		sds_free(ptrList.hwBufs[i]);
		ptrList.hwBufs[i] = NULL;
	}
#if EN_FREE_PRINT
	cout << "Memory Released" << endl;
#endif
#endif
}
// freeMemory

//# Prints all address
void bufMgmt_print(
		vector<XlayerData> xlayer_seq,
		vector<xChangeLayer> lay_obj[NUM_IMG]
)
{
	unsigned int seq_len = xlayer_seq.size();

	for(int img_id = 0; img_id < NUM_IMG; img_id++)
	{
		cout << endl;
		vector<xChangeLayer> cur_obj;
		cur_obj = lay_obj[img_id];

		cout << "*** image : " << img_id << endl;
		xChangeLayer curLayer;

		for(unsigned int iter = 0; iter < seq_len; iter++)
		{
			curLayer = cur_obj[iter];

			cout << endl;
			cout << "[INFOx] Layer " << iter << " : "  << xlayer_seq[iter].hw_ops -> name << " , " << "Type : " << xlayer_seq[iter].hw_ops -> type << endl;
			cout << "params : " << curLayer.params << endl;

			//# input pointers
			for(int i = 0; i < INPUT_PORTS; i++)
			{
				cout << "ip_ptrs[" << i << "] : " << curLayer.in_ptrs[i] << endl;
			}

			//# output pointers
			for(int i = 0; i < OUTPUT_PORTS; i++)
			{
				cout << "out_ptrs[" << i << "] : " << curLayer.out_ptrs[i] << endl;
			}

			//# Extra pointers
			for(unsigned int i = 0; i < curLayer.xtra_ptrs.size(); i++)
			{
				cout << "xtra_ptrs[" << i << "] : " << curLayer.xtra_ptrs[i] << endl;
			}

			if( 	(xlayer_seq[iter].hw_ops -> type == "Convolution")  ||
					(xlayer_seq[iter].hw_ops -> type == "InnerProduct") ||
					(xlayer_seq[iter].hw_ops -> type == "Deconvolution")
			)
			{
				//# weight pointers
				for(int i = 0; i < WEIGHT_PORTS; i++)
				{
					cout << "wgt_ptrs[" << i << "] : " << curLayer.wts_ptrs[i] << endl;
				}

				//# bias pointer
				cout << "bias_ptrs : " << curLayer.bias_ptr << endl;
			}
#if EN_CONSOLE_TXT_BUFMGMT_DBG
			fprintf(stderr,".");
#endif
		}
	}
}
// bufMgmt_print

//# Main Buffer creation function
void xiBuffCreate(
		vector<XlayerData> xlayer_seq,
		vector<xChangeLayer> (&lay_obj)[NUM_IMG],
		bufPtrs &ptrList
)
{
	ptrList.size = 0;

#if EN_PRINT
	cout << "creating memoery for constant buffers ... " << endl;
#endif
	xiConstBufCreate(xlayer_seq, lay_obj, ptrList);
#if EN_PRINT
	cout << "constant buffer creation Done!" << endl;
	cout << "creating memoery for IO buffers ... " << endl;
#endif	
	xiIOBuffCreate(xlayer_seq, lay_obj, ptrList);
#if EN_PRINT
	cout << "IO buffer creation Done!" << endl;
#endif
}
// xiBuffCreate
