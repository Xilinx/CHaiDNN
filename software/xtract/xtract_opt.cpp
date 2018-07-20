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

#include <math.h>
#include "xi_funcs.hpp"
#include "xgraph_opt.hpp"
#include "xtract_utility.hpp"



void XGraphOpt::opt_lrn(XGraph* xgraph_opt){

	string *lrn_layer_type = new string("LRN");
	int lrn_cnt=0;
	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();it++){

		if (it->second->type.compare(*lrn_layer_type)==0){

			std::stringstream ss1;
			std::stringstream ss2;

			string lrn_name = "conv_lrn";
			ss1<<lrn_name<<lrn_cnt<<"1";
			string lrn1_name = ss1.str();//"conv_lrn1";
			ss2<<lrn_name<<lrn_cnt<<"2";
			string lrn2_name = ss2.str();


			XLayer* dst0 = new XLayer(lrn1_name, "Convolution");
			xgraph_opt->layers[lrn1_name] = dst0;
			XLayer* dst1 = new XLayer(lrn2_name, "Convolution");
			xgraph_opt->layers[lrn2_name] = dst1;

			//string uniqname = xgraph_opt->getUniqueBlobName();
			string username = lrn1_name;
			XBlob* tmp_blob = new XBlob(username);
			xgraph_opt->blobs[username] = tmp_blob;

			tmp_blob->shape=it->second->top[0].blob->shape;

			tmp_blob->producerDim.push_back(tmp_blob->shape);
			tmp_blob->consumerDim.push_back(tmp_blob->shape);

			nameIndex new_blob(tmp_blob, 0);

			dst0->top.push_back(new_blob);

			dst1->bottom.push_back(new_blob);

			// copy lrn params to first conv params
			dst0->conv_params->lrn_alpha = it->second->lrn_params->alpha;
			dst0->conv_params->lrn_beta = it->second->lrn_params->beta;
			dst0->conv_params->lrn_lsize = it->second->lrn_params->lsize;
			dst0->conv_params->lrn_k = it->second->lrn_params->k;
			dst0->conv_params->lrn_type = it->second->lrn_params->type;
			dst0->conv_params->N = tmp_blob->shape.at(1);
			dst0->conv_params->M = tmp_blob->shape.at(1);
			dst0->conv_params->filter_h = dst0->conv_params->lrn_lsize;
			dst0->conv_params->filter_w = dst0->conv_params->lrn_lsize;
			dst0->conv_params->pad_h=0;
			dst0->conv_params->pad_w=0;
			dst0->conv_params->dilation=1;
			dst0->conv_params->group =1;
			dst0->conv_params->has_bias = 0;
			dst0->conv_params->reluflag = 0;

			dst0->opcode = OPCODE_LRN_INTER_SOS;

			// copy lrn params to second conv
			dst1->conv_params->lrn_alpha = it->second->lrn_params->alpha;
			dst1->conv_params->lrn_beta = it->second->lrn_params->beta;
			dst1->conv_params->lrn_lsize = it->second->lrn_params->lsize;
			dst1->conv_params->lrn_k = it->second->lrn_params->k;
			dst1->conv_params->lrn_type = it->second->lrn_params->type;
			dst1->conv_params->N = tmp_blob->shape.at(1);
			dst1->conv_params->M = tmp_blob->shape.at(1);
			dst1->conv_params->filter_h = dst0->conv_params->lrn_lsize;
			dst1->conv_params->filter_w = dst0->conv_params->lrn_lsize;
			dst1->conv_params->pad_h=0;
			dst1->conv_params->pad_w=0;
			dst1->conv_params->dilation=1;
			dst1->conv_params->group =1;
			dst1->conv_params->has_bias = 0;
			dst1->conv_params->reluflag = 0;

			dst1->opcode = OPCODE_LRN_INTER_PNS;

			tmp_blob->consumers.push_back(dst1->name);
			tmp_blob->producers.push_back(dst0->name);

			for(int iter_l_b_c=0;iter_l_b_c<it->second->bottom.size();iter_l_b_c++){

				XBlob * cur_b_blob = it->second->bottom[iter_l_b_c].blob;

				for(int iter_l_b_c_c=0;iter_l_b_c_c<cur_b_blob->consumers.size();iter_l_b_c_c++){

					if(cur_b_blob->consumers[iter_l_b_c_c].compare(it->second->name)==0){

						cur_b_blob->consumers[iter_l_b_c_c]=lrn1_name;
						cur_b_blob->consumers.push_back(lrn2_name);

						dst0->bottom.push_back(it->second->bottom[iter_l_b_c]);
						dst1->bottom.push_back(it->second->bottom[iter_l_b_c]);
					}
				}
			}

			for(int iter_l_t_c=0;iter_l_t_c<it->second->top.size();iter_l_t_c++){

				XBlob * cur_t_blob = it->second->top[iter_l_t_c].blob;

				for(int iter_l_b_c_c=0;iter_l_b_c_c<cur_t_blob->producers.size();iter_l_b_c_c++){

					if(cur_t_blob->producers[iter_l_b_c_c].compare(it->second->name)==0){
						cur_t_blob->producers[iter_l_b_c_c]=lrn2_name;

						dst1->top.push_back(it->second->top[iter_l_t_c]);
					}
				}
			}
			xgraph_opt->layers.erase(it);
			lrn_cnt++;
		}
	}
	delete lrn_layer_type;
}

void XGraphOpt::opt_reshape(XGraph* xgraph_opt){

	string *reshape_layer_type = new string("Reshape");

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();){

		int cont=0;
		if (it->second->type.compare(*reshape_layer_type)==0){
			mapStrXLayer::iterator cur_it = it;
			it++;
			xgrap_layer_delete(xgraph_opt,cur_it->second->name);
			cont=1;

		}
		if(cont==0)
			it++;
		else{
			cont=0;
		}
	}
	delete reshape_layer_type;
}



void XGraphOpt::opt_flatten(XGraph* xgraph_opt){

	string *flatten_layer_type = new string("Flatten");

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();){

		int cont=0;
		if (it->second->type.compare(*flatten_layer_type)==0){

			mapStrXLayer::iterator cur_it = it;
			it++;
			xgrap_layer_delete(xgraph_opt,cur_it->second->name);
			cont=1;

		}
		if(cont==0)
			it++;
		else{
			cont=0;
		}
	}
	delete flatten_layer_type;
}

void XGraphOpt::opt_fuse_bn_conv(XGraph* xgraph_opt,vector <string> &bn_opt_table){

	int fuse_conv_cnt=0;
	for(int iter_bn_v=0;iter_bn_v<bn_opt_table.size();iter_bn_v++){

		map < string, XLayer*>::iterator it_layer = xgraph_opt->layers.find(bn_opt_table[iter_bn_v]);

		if(it_layer->second->top.size()==1){

			map < string, XBlob*>::iterator it_blob = xgraph_opt->blobs.find(it_layer->second->top[0].blob->name);
			string top_blob_consumer;

			if(it_blob->second->consumers.size()==1){

				top_blob_consumer = it_blob->second->consumers[0];
				map < string, XLayer*>::iterator it_conv_layer = xgraph_opt->layers.find(top_blob_consumer);

				if((it_conv_layer->second->type.compare("Convolution")==0) && (it_conv_layer->second->opcode==OPCODE_CONV)){

					//cout<<"it_conv_layer->second->type.compare(Convolution==0) is true" << fuse_conv_cnt++ <<" name "<<it_conv_layer->second->name<<endl;

					int conv_top_indx=0,conv_top_prod_indx=0;

					it_layer->second->conv_params->N = it_blob->second->shape.at(1);

					for(int iter_top_c=0;iter_top_c<it_conv_layer->second->top.size();iter_top_c++){

						for(int iter_t_p=0;iter_t_p<it_conv_layer->second->top[iter_top_c].blob->producers.size();iter_t_p++){

							if(it_conv_layer->second->top[iter_top_c].blob->producers[iter_t_p].compare(it_conv_layer->second->name)==0){

								it_layer->second->conv_params->M = it_conv_layer->second->top[iter_top_c].blob->shape.at(1);
								conv_top_indx= iter_top_c;
								conv_top_prod_indx= iter_t_p;
							}
						}
					}

					it_layer->second->conv_params->filter_h = it_conv_layer->second->conv_params->filter_h;
					it_layer->second->conv_params->filter_w = it_conv_layer->second->conv_params->filter_w;
					it_layer->second->conv_params->stride_h = it_conv_layer->second->conv_params->stride_h;
					it_layer->second->conv_params->stride_w = it_conv_layer->second->conv_params->stride_w;
					it_layer->second->conv_params->pad_h=it_conv_layer->second->conv_params->pad_h;
					it_layer->second->conv_params->pad_w=it_conv_layer->second->conv_params->pad_w;
					it_layer->second->conv_params->dilation=it_conv_layer->second->conv_params->dilation;
					it_layer->second->conv_params->group =it_conv_layer->second->conv_params->group;
					it_layer->second->conv_params->has_bias = it_conv_layer->second->conv_params->has_bias;
					//it_layer->second->conv_params->biasPath = it_conv_layer->second->conv_params->biasPath;
					it_layer->second->conv_params->bias = it_conv_layer->second->conv_params->bias;
					it_layer->second->conv_params->biasDim = it_conv_layer->second->conv_params->biasDim;
					it_layer->second->conv_params->weightsDim = it_conv_layer->second->conv_params->weightsDim;
					//	it_layer->second->conv_params->weightsPath = it_conv_layer->second->conv_params->weightsPath;
					it_layer->second->conv_params->weights = it_conv_layer->second->conv_params->weights;
					it_layer->second->wt_bw = it_conv_layer->second->wt_bw;
					it_layer->second->wt_fl = it_conv_layer->second->wt_fl;

					it_layer->second->output_file = it_conv_layer->second->output_file;

					it_layer->second->conv_params->M = it_conv_layer->second->top[conv_top_indx].blob->shape.at(1);

					if(it_conv_layer->second->conv_params->reluflag)
						it_layer->second->conv_params->reluflag = true;

					it_layer->second->op_bw = it_conv_layer->second->op_bw;
					it_layer->second->op_fl = it_conv_layer->second->op_fl;

					it_layer->second->th_layer_in = it_conv_layer->second->th_layer_in;
					it_layer->second->th_layer_out = it_conv_layer->second->th_layer_out;
					it_layer->second->th_params = it_conv_layer->second->th_params;
					// it_layer->second->user_op_bw = it_conv_layer->second->user_op_bw;
					// it_layer->second->user_op_fl = it_conv_layer->second->user_op_fl;

					it_layer->second->top[0]=it_conv_layer->second->top[conv_top_indx]; // TODO assuming only one top blob for each layer
					it_layer->second->top[0].blob->producers[conv_top_prod_indx]=it_layer->second->name; // TODO assuming only one top blob for each layer
					it_layer->second->opcode = OPCODE_FUSE_BN_CONV;
					xgraph_opt->layers.erase(it_conv_layer);
					xgraph_opt->blobs.erase(it_blob);

				}
			}

		}else{
			if(it_layer->second->top.size()>0){

				ELOG((it_layer->second->top.size()==1),"opt_fuse_bn_conv", "bn Layer has more than one top blobs,its not support right now : "<< it_layer->second->top.size());
			}
		}
	}
}

// Function to compare absolute values.
// Used to find absolute max_value in an array using std::max_element
// TODO : It is a generic function. It should go to some other file, probably xdnnfuncs.cpp
inline bool compareAbsValues(float a, float b)
{
	return (std::abs(a) < std::abs(b));
}

// This function is same as Trim2FixedPoint() in caffeNetworkParser.cpp
// TODO : It is a generic function. It should go to some other file, probably xdnnfuncs.cpp
void Trim2FixedPoint2(vector<float>& data, const int bw, const int fl, RoundingMethod rounding )
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

void fuseBNScaleConvParams(XLayer* convLayer, XLayer* bnLayer, XLayer* scaleLayer, int mode)
{
	// Load Convolution Data
	std::vector<float>& conv_weights = convLayer->conv_params->weights.at(0);
	std::vector<float>& conv_bias = convLayer->conv_params->bias.at(0);
	std::vector<int> conv_weights_dim = convLayer->conv_params->weightsDim.at(0);
	std::vector<int> conv_bias_dim = convLayer->conv_params->biasDim.at(0);

	int final_op_bw = convLayer->op_bw;
	int final_op_fl = convLayer->op_fl;
	int final_op_th = convLayer->th_layer_out;

	std::vector<float> mean, variance, gamma, beta;
	float eps = 1e-10;

	switch(mode)
	{
	// Convolution -> [BatchNorm] -> [Scale]
	case 0:
	{

		// Load BatchNorm Data
		if (bnLayer != NULL)
		{
			mean = bnLayer->batchnorm_params->mean.at(0);
			variance = bnLayer->batchnorm_params->variance.at(0);
			eps = bnLayer->batchnorm_params->eps;
			final_op_bw = bnLayer->op_bw;
			final_op_fl = bnLayer->op_fl;
			final_op_th = bnLayer->th_layer_out;
		}
		else
		{
			mean.resize(conv_bias_dim.at(0), 0.0);
			variance.resize(conv_bias_dim.at(0), 1.0);
			eps = 0;
		}


		// Load Scale Layer Data
		if (scaleLayer != NULL)
		{
			gamma = scaleLayer->scale_params->gamma.at(0);
			beta = scaleLayer->scale_params->beta.at(0);
			final_op_bw = scaleLayer->op_bw;
			final_op_fl = scaleLayer->op_fl;
			final_op_th = scaleLayer->th_layer_out;
		}
		else
		{
			gamma.resize(conv_bias_dim.at(0), 1.0);
			beta.resize(conv_bias_dim.at(0), 0.0);
		}

		// Real Operation Starts here
		int num = conv_bias_dim.at(0);
		ASSERT( conv_weights.size() % num == 0, EX021, "conv_weights.size() % num != 0 in Layer: " << convLayer->name << " " << num << " " << conv_weights.size())
		int count = conv_weights.size()/num;

		// Calculate the new weights and bias
		for(int n=0; n<num; ++n)
		{
			int W_offset = n * count;
			float M = mean[n];
			float V = variance[n];
			float G = gamma[n];
			float B = beta[n];
			float K = std::sqrt(V + eps);
			float S = G / std::sqrt(V + eps);

			conv_bias[n] = S * (conv_bias[n] - M) + B;

			for(int i=0; i<count; ++i)
			{
				conv_weights[W_offset + i] *= S;
			}
		}

		// Now calculate the Precision Params for weights

		if(convLayer->quantization_scheme == "DynamicFixed") {
			const int CONV_BW = convLayer->wt_bw;
			int MAX_VALUE = std::pow(2, CONV_BW-1);
			int int_bits, fl_bits;
			float log2_val=0;
			vector<float>::iterator abs_max_iter = std::max_element(conv_weights.begin(), conv_weights.end(), compareAbsValues);
			float abs_max_val = std::abs(*abs_max_iter);
			/*ASSERT( abs_max_val <= MAX_VALUE, EO99,
                            "Insufficient bit-precision for weights in " << convLayer->name << ": " << abs_max_val << " v/s " << MAX_VALUE)*/

			// Special case for max<1
			if(abs_max_val < 1)
			{
				int_bits = 1;                   // Sign bit
			}
			else if(abs_max_val == 1)
			{
				int_bits = 2;                   // Sign bit + 1-bit to represent "1"
			}
			else
			{
				log2_val = log2f(abs_max_val);
				int_bits = static_cast<int>(std::ceil(std::abs(log2_val))) + 1;     // +1 for sign-bit
			}

			fl_bits = CONV_BW - int_bits;

			convLayer->op_bw = final_op_bw;
			convLayer->op_fl = final_op_fl;
			convLayer->wt_fl = fl_bits;
		}

		else if(convLayer->quantization_scheme == "Xilinx") {
			convLayer->th_params = getAbsMaxPerFilter<float>(conv_weights, conv_weights_dim);
			convLayer->op_bw = final_op_bw;
			convLayer->th_layer_out = final_op_th;
		}

		// cerr << "DEBUG : " << convLayer->name << " : " << abs_max_val << " , "
		//     << log2_val << " " << std::abs(log2_val) << " " << std::abs(*abs_max_iter) << " " << fl_bits << endl;

		break;
	}
	default:
	{
		ELOG(true, EX022, "Unrecognized mode in Fusion")
	}
	}

#if DEBUG_WEIGHT_EXTRACTION
	SAVEDATA(conv_weights, convLayer->conv_params->weightsPath.at(0));
	SAVEDATA(conv_bias, convLayer->conv_params->biasPath.at(0));
#endif

}

void XGraphOpt::opt_conv_bn_scale(XGraph* xgraph_opt){

	string *conv_layer_type = new string("Convolution");
	string *scale_layer_type = new string("Scale");
	string *bn_layer_type = new string("BatchNorm");
	string *relu_layer_type = new string("ReLU");


	/*layer_in_place.push_back(*scale_layer_type);
	layer_in_place.push_back(*bn_layer_type);*/
	XLayer* bn_layer;
	XLayer* scale_layer;
	XLayer* relu_layer;

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();it++){
		vector<string> layer_in_place;
		int xtract_fuse_flag=0;
		if(it->second->type.compare(*conv_layer_type)==0){

			if(it->second->top.size()==1)
			{

				map < string, XBlob*>::iterator it_t_blob = xgraph_opt->blobs.find(it->second->top[0].blob->name);

				if(it_t_blob->second->producers.size()>1){

					int bn_flag=0,scale_flag=0,rel_flag=0;

					for(int iter_t_p=0;iter_t_p<it_t_blob->second->producers.size();iter_t_p++){

						map < string, XLayer*>::iterator it_p_layer = xgraph_opt->layers.find(it_t_blob->second->producers[iter_t_p]);

						if(it_p_layer->second->type.compare(*bn_layer_type)==0){

							if(it_p_layer->second->batchnorm_params->inPlace){

								layer_in_place.push_back(it_p_layer->second->name);
								bn_layer=it_p_layer->second;
								bn_flag=1;
							}
						}
						else if(it_p_layer->second->type.compare(*scale_layer_type)==0){

							if(it_p_layer->second->scale_params->inPlace){

								layer_in_place.push_back(it_p_layer->second->name);
								scale_layer=it_p_layer->second;
								scale_flag=1;
							}
						}
						else{

							if(it_p_layer->second->type.compare(*relu_layer_type)==0){

								if(it_p_layer->second->relu_params->inPlace){
									layer_in_place.push_back(it_p_layer->second->name);
									relu_layer=it_p_layer->second;
									rel_flag=1;

								}
							}

						}
					}

					if((it_t_blob->second->producers.size()-1)==layer_in_place.size())
					{

						if(rel_flag==1 && scale_flag==1 && bn_flag==1){

							fuseBNScaleConvParams(it->second,bn_layer,scale_layer,0);

							it->second->conv_params->reluflag = 1;
							it->second->conv_params->has_software_fuse=1;

							xtract_fuse_flag=1;
							for(int iter_t_p_l=0;iter_t_p_l<layer_in_place.size();iter_t_p_l++){


								std::vector<string>::iterator it_str=std::find(it_t_blob->second->producers.begin(),it_t_blob->second->producers.end(),layer_in_place[iter_t_p_l]);

								if(it_str!=it_t_blob->second->producers.end())
								{
									it_t_blob->second->producers.erase(it_str);
								}

							}
						}else{

							if(rel_flag==0 && scale_flag==1 && bn_flag==1){

								fuseBNScaleConvParams(it->second,bn_layer,scale_layer,0);
								xtract_fuse_flag=1;

								it->second->conv_params->has_software_fuse=1;

								for(int iter_t_p_l=0;iter_t_p_l<layer_in_place.size();iter_t_p_l++){


									std::vector<string>::iterator it_str=std::find(it_t_blob->second->producers.begin(),it_t_blob->second->producers.end(),layer_in_place[iter_t_p_l]);

									if(it_str!=it_t_blob->second->producers.end())
									{
										it_t_blob->second->producers.erase(it_str);
									}

								}
							}
						}
						if(xtract_fuse_flag==1){

							for(int iter_t_c_l=0;iter_t_c_l<layer_in_place.size();iter_t_c_l++){


								std::vector<string>::iterator it_str=std::find(it_t_blob->second->consumers.begin(),it_t_blob->second->consumers.end(),layer_in_place[iter_t_c_l]);

								if(it_str!=it_t_blob->second->consumers.end())
								{
									it_t_blob->second->consumers.erase(it_str);
								}

							}

							for(int iter_inc=0;iter_inc<layer_in_place.size();iter_inc++){

								map < string, XLayer*>::iterator it_d_layer = xgraph_opt->layers.find(layer_in_place[iter_inc]);
								xgraph_opt->layers.erase(it_d_layer);
							}

						}
					}
				}
				else{

					if(it_t_blob->second->consumers.size()==1){

						map < string, XLayer*>::iterator it_c_layer = xgraph_opt->layers.find(it_t_blob->second->consumers[0]);

						if(it_c_layer->second->type.compare(*bn_layer_type)==0){

							layer_in_place.push_back(it_c_layer->second->name);

							bn_layer=it_c_layer->second;

							if(it_c_layer->second->top.size()==1){

								map < string, XBlob*>::iterator it_t_bn_blob = xgraph_opt->blobs.find(it_c_layer->second->top[0].blob->name);

								if(it_t_bn_blob->second->consumers.size()==1){

									map < string, XLayer*>::iterator it_bn_c_layer = xgraph_opt->layers.find(it_t_bn_blob->second->consumers[0]);

									if(it_bn_c_layer->second->type.compare(*scale_layer_type)==0){

										layer_in_place.push_back(it_bn_c_layer->second->name);

										scale_layer=it_bn_c_layer->second;

										map < string, XBlob*>::iterator it_t_s_blob = xgraph_opt->blobs.find(it_bn_c_layer->second->top[0].blob->name);

										//if(it_t_s_blob->second->consumers.size()==1){
										int rel_cout_flag=0;
										for(int iter_rel_c=0;iter_rel_c<it_t_s_blob->second->consumers.size();iter_rel_c++){

											map < string, XLayer*>::iterator it_sc_c_layer = xgraph_opt->layers.find(it_t_s_blob->second->consumers[iter_rel_c]);

											if(it_sc_c_layer->second->type.compare(*relu_layer_type)==0){
												rel_cout_flag=1;
												relu_layer=it_sc_c_layer->second;
												layer_in_place.push_back(it_sc_c_layer->second->name);

												//Abid_software_fuse(bn_layer,scale_layer,it->second);
												fuseBNScaleConvParams(it->second,bn_layer,scale_layer,0);

												it->second->conv_params->reluflag =1;
												it->second->conv_params->has_software_fuse=1;

												if(!it_sc_c_layer->second->relu_params->inPlace){


													it->second->top[0]=it_sc_c_layer->second->top[0];
													it->second->top[0].blob->producers[0]=it->second->name;
													it->second->conv_params->reluflag=1;

													it->second->output_file = it_sc_c_layer->second->output_file;

													for(int iter_inc=0;iter_inc<layer_in_place.size();iter_inc++){

														map < string, XLayer*>::iterator it_d_layer = xgraph_opt->layers.find(layer_in_place[iter_inc]);
														xgraph_opt->layers.erase(it_d_layer);
													}

													xgraph_opt->blobs.erase(it_t_blob);
													xgraph_opt->blobs.erase(it_t_bn_blob);
													xgraph_opt->blobs.erase(it_t_s_blob);
													if(rel_cout_flag==1)
														break;
												}else{

													it->second->top[0]=it_sc_c_layer->second->top[0];
													it->second->top[0].blob->producers[0]=it->second->name;
													it->second->conv_params->reluflag=1;

													it->second->output_file = it_sc_c_layer->second->output_file;

													for(int iter_inc=0;iter_inc<layer_in_place.size();iter_inc++){

														map < string, XLayer*>::iterator it_d_layer = xgraph_opt->layers.find(layer_in_place[iter_inc]);
														xgraph_opt->layers.erase(it_d_layer);
													}
													std::vector<string>::iterator it_str=std::find(it_t_s_blob->second->producers.begin(),it_t_s_blob->second->producers.end(),it_sc_c_layer->second->name);

													if(it_str!=it_t_s_blob->second->producers.end())
													{
														it_t_s_blob->second->producers.erase(it_str);
													}

													it_str=std::find(it_t_s_blob->second->consumers.begin(),it_t_s_blob->second->consumers.end(),it_sc_c_layer->second->name);

													if(it_str!=it_t_s_blob->second->consumers.end())
													{
														it_t_s_blob->second->consumers.erase(it_str);
													}
													xgraph_opt->blobs.erase(it_t_blob);
													xgraph_opt->blobs.erase(it_t_bn_blob);

													if(rel_cout_flag==1){

														break;
													}
													//xgraph_opt->blobs.erase(it_t_s_blob);
												}
											}
											else{

												fuseBNScaleConvParams(it->second,bn_layer,scale_layer,0);

												it->second->top[0]=scale_layer->top[0];
												it->second->top[0].blob->producers[0]=it->second->name;
												//it->second->conv_params->reluflag=1;

												it->second->conv_params->has_software_fuse=1;

												it->second->output_file = scale_layer->output_file;

												for(int iter_inc=0;iter_inc<layer_in_place.size();iter_inc++){

													map < string, XLayer*>::iterator it_d_layer = xgraph_opt->layers.find(layer_in_place[iter_inc]);
													xgraph_opt->layers.erase(it_d_layer);
												}

												xgraph_opt->blobs.erase(it_t_blob);
												xgraph_opt->blobs.erase(it_t_bn_blob);
												//xgraph_opt->blobs.erase(it_t_s_blob);
											}
										}
									}
								}
							}
						}
					}
				}

			}
		}
	}

	delete conv_layer_type;
	delete scale_layer_type;
	delete bn_layer_type;
	delete relu_layer_type;
}

void XGraphOpt::opt_crelu(XGraph *xgraph_opt){

	string *conv_layer_type = new string("Convolution");
	string *relu_layer_type = new string("ReLU");
	string *concat_layer_type = new string("Concat");
	string *power_layer_type = new string("Power");

	//vector<string> crelu_layers_pattern;
	int power_break=0;
	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();it++){


		if(it->second->type.compare(*conv_layer_type)==0){

			map < string, XBlob*>::iterator it_conv_t_blob = xgraph_opt->blobs.find(it->second->top[0].blob->name);
			map < string, XBlob*>::iterator it_pw_t_blob;
			map < string, XBlob*>::iterator it_concat_t_blob;
			XBlob* top_blob = it->second->top[0].blob; // TODO as per the caffe we alwayes expect only one top[0] blob for each layer


			XLayer* pw_layer;
			XLayer* concat_layer;
			XLayer* relu_layer;

			int power_flag=-1,concat_flag=-1,rel_flag=-1;

			if(top_blob->consumers.size()==2){

				vector<string> crelu_layers_pattern;

				for(int iter_c=0;iter_c<2;iter_c++){

					string c_layer_name = top_blob->consumers[iter_c];

					map < string, XLayer*>::iterator it_c_layer= xgraph_opt->layers.find(c_layer_name);

					if(it_c_layer->second->type.compare(*power_layer_type)==0){

						// checking the params to meet the following equation (scale*x+shift)^power

						if(!((it_c_layer->second->power_params->power==1) && (it_c_layer->second->power_params->scale==-1) && (it_c_layer->second->power_params->shift==0))){

							power_break=1;

							break;

						}
						power_flag=iter_c;

						pw_layer = it_c_layer->second;
						it_pw_t_blob = xgraph_opt->blobs.find(pw_layer->top[0].blob->name);
						crelu_layers_pattern.push_back(pw_layer->name);
					}else if(it_c_layer->second->type.compare(*concat_layer_type)==0){
						concat_flag=iter_c;
						concat_layer=it_c_layer->second;
						crelu_layers_pattern.push_back(concat_layer->name);
						it_concat_t_blob = xgraph_opt->blobs.find(concat_layer->top[0].blob->name);
					}
				}
				if(power_break==1){
					power_break=0;
					continue;

				}

				/*map < string, XBlob*>::iterator it_pw_t_blob = xgraph_opt->blobs.find(pw_layer->top[0].blob->name);

				map < string, XBlob*>::iterator it_concat_t_blob = xgraph_opt->blobs.find(concat_layer->top[0].blob->name);*/

				if((power_flag!=-1) && (concat_flag!=-1)){
					string relu_name;
					XBlob* concat_top_blob = concat_layer->top[0].blob; // TODO as per the caffe we always expect only one top[0] blob for each layer
					concat_flag=-1;

					if(concat_top_blob->producers.size()==2){

						for(int iter_p_c=0;iter_p_c<2;iter_p_c++){

							string p_layer_name = concat_top_blob->producers[iter_p_c];

							map < string, XLayer*>::iterator it_p_layer=xgraph_opt->layers.find(p_layer_name);

							if(it_p_layer->second->type.compare(*relu_layer_type)==0){

								rel_flag=iter_p_c;
								relu_layer = it_p_layer->second;

								crelu_layers_pattern.push_back(relu_layer->name);

							}else if(it_p_layer->second->type.compare(*concat_layer_type)==0){

								concat_flag=iter_p_c;
								//concat_layer=it_p_layer->second;
							}
						}
						if((rel_flag!=-1) && (concat_flag!=-1)){

							it->second->top[0]=concat_layer->top[0];
							it->second->topShape[0]=concat_layer->topShape[0];
							it->second->top[0].blob->producers[0]=it->second->name;
							it->second->opcode=OPCODE_CRELU;

							for(int iter_v=0;iter_v<crelu_layers_pattern.size();iter_v++){

								map < string, XLayer*>::iterator it_layer=xgraph_opt->layers.find(crelu_layers_pattern[iter_v]);
								xgraph_opt->layers.erase(it_layer);
							}
							xgraph_opt->blobs.erase(it_pw_t_blob);
							xgraph_opt->blobs.erase(it_conv_t_blob);

							concat_top_blob->producers.erase(concat_top_blob->producers.begin() + rel_flag);

							std::vector<string>::iterator it_str=std::find(concat_top_blob->consumers.begin(),concat_top_blob->consumers.end(),relu_layer->name);

							if(it_str!=concat_top_blob->consumers.end())
							{
								concat_top_blob->consumers.erase(it_str);
							}

						}
					}else{

						if(concat_top_blob->producers.size()==1){

							if(concat_top_blob->consumers.size()==1){

								map < string, XLayer*>::iterator it_concat_c_layer=xgraph_opt->layers.find(concat_top_blob->consumers[0]);

								if(it_concat_c_layer->second->type.compare(*relu_layer_type)==0){

									crelu_layers_pattern.push_back(it_concat_c_layer->second->name);

									it->second->top[0]=it_concat_c_layer->second->top[0];
									it->second->top[0].blob->producers[0]=it->second->name;
									//it->second->top[0].blob->producers[0]=it->second->name;

									it->second->topShape[0] = it_concat_c_layer->second->topShape[0];
									it->second->opcode=OPCODE_CRELU;

									for(int iter_v=0;iter_v<crelu_layers_pattern.size();iter_v++){

										map < string, XLayer*>::iterator it_layer=xgraph_opt->layers.find(crelu_layers_pattern[iter_v]);
										xgraph_opt->layers.erase(it_layer);
									}
									xgraph_opt->blobs.erase(it_pw_t_blob);
									xgraph_opt->blobs.erase(it_conv_t_blob);
									xgraph_opt->blobs.erase(it_concat_t_blob);
								}
							}
						}
					}
				}
			}
		}
	}
	delete conv_layer_type;
	delete relu_layer_type;
	delete concat_layer_type;
	delete power_layer_type ;
}

void XGraphOpt::custom_layer(XGraph *xgraph_opt){

	vector<string> in_packed,in_unpacked,out_packed,out_unpacked;

	in_packed.push_back("Convolution");
	in_packed.push_back("Permute");
	in_packed.push_back("Pooling");
	in_packed.push_back("L2Normalize");
	in_packed.push_back("Softmax");

	out_packed.push_back("Convolution");
	out_packed.push_back("Pooling");
	out_packed.push_back("L2Normalize");

	in_unpacked.push_back("Deconvolution");
	in_unpacked.push_back("InnerProduct");
	in_unpacked.push_back("XCustom");
	//in_unpacked.push_back("Softmax");
	in_unpacked.push_back("NMS");
	in_unpacked.push_back("Crop");

	out_unpacked.push_back("InnerProduct");
	out_unpacked.push_back("Softmax");
	out_unpacked.push_back("XCustom");
	out_unpacked.push_back("Permute");
	out_unpacked.push_back("NMS");
	out_unpacked.push_back("Crop");
	out_unpacked.push_back("Deconvolution");


	string *custom_layer_type =   new string("XCustom");
	int custom_layer_cnt=0;
	int unpack_flag=0,unpack_flag_fbit=0,pack_flag=0,pack_flag_fbit=0;
	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();it++){

		if(it->second->type.compare(*custom_layer_type)==0){

			for(int iter_bottom_b=0;iter_bottom_b<it->second->bottom.size();iter_bottom_b++){

				XBlob* b_blob = it->second->bottom[iter_bottom_b].blob;

				for(int iter_p=0;iter_p<b_blob->producers.size();iter_p++){

					map < string, XLayer*>::iterator prod_layer_it = xgraph_opt->layers.find(b_blob->producers[iter_p]);

					std::vector<string>::iterator out_packed_type_it=std::find(out_packed.begin(),out_packed.end(),prod_layer_it->second->type);

					if(out_packed_type_it!=out_packed.end()){ // one of the custom layer producer is generates packed output, so that here need a unpacked layers.

						string layer_name = "unpack_XCustom";
						std::stringstream ss1;

						ss1<<layer_name<<custom_layer_cnt;
						string packed_layer_name = ss1.str();//"conv_lrn1";

						XLayer* dst0 = new XLayer(packed_layer_name, "XPack");
						xgraph_opt->layers[packed_layer_name] = dst0;

						dst0->xpack_params->pack=false;
						dst0->opcode=OPCODE_XPACK;
						//string uniqname = xgraph_opt->getUniqueBlobName();
						string username = packed_layer_name;
						XBlob* tmp_blob = new XBlob(username);
						xgraph_opt->blobs[username] = tmp_blob;

						nameIndex new_blob(tmp_blob, 0);

						dst0->bottom.push_back(it->second->bottom[iter_bottom_b]);
						dst0->bottomShape.push_back(b_blob->shape);

						tmp_blob->shape = prod_layer_it->second->topShape[0];

						dst0->top.push_back(new_blob);
						dst0->topShape.push_back(tmp_blob->shape);
						dst0->quantization_scheme = prod_layer_it->second->quantization_scheme;

						tmp_blob->producers.push_back(packed_layer_name);
						tmp_blob->consumers.push_back(it->second->name);

						for(int iter_b_cons=0;iter_b_cons<b_blob->consumers.size();iter_b_cons++){

							if(b_blob->consumers[iter_b_cons].compare(it->second->name)==0){

								b_blob->consumers[iter_b_cons]=dst0->name;

							}
						}

						it->second->bottom[iter_bottom_b]=new_blob;
						//prod_layer_it->second->top[0]=tmp_blob;
						//dst0->xpack_params->fbits=	(int)it->second->xcustom_params->float_args[0];// need to fix as below
						//dst0->xpack_params->fbits=prod_layer_it->second->op_fl;

						dst0->xpack_params->fbits=it->second->ip_fl;

						custom_layer_cnt++;
						unpack_flag=1;
						unpack_flag_fbit=dst0->xpack_params->fbits;
						continue;
					}

					std::vector<string>::iterator out_unpacked_type_it=std::find(out_unpacked.begin(),out_unpacked.end(),prod_layer_it->second->type);

					if(out_unpacked_type_it!=out_unpacked.end()){ // one of the custom layer producer is generates packed output, so that here need a unpacked layers.

						continue;
					}
				}
			}

			for(int iter_top_b=0;iter_top_b<it->second->top.size();iter_top_b++){

				XBlob* t_blob = it->second->top[iter_top_b].blob;

				for(int iter_c=0;iter_c<t_blob->consumers.size();iter_c++){

					map < string, XLayer*>::iterator cons_layer_it = xgraph_opt->layers.find(t_blob->consumers[iter_c]);
					std::vector<string>::iterator in_packed_type_it=std::find(in_packed.begin(),in_packed.end(),cons_layer_it->second->type);

					if(in_packed_type_it!=in_packed.end()){ // one of the custom layer producer is generates packed output, so that here need a unpacked layers.

						string layer_name = "pack_XCustom";
						std::stringstream ss1;

						ss1<<layer_name<<custom_layer_cnt;
						string packed_layer_name = ss1.str();//"conv_lrn1";

						XLayer* dst0 = new XLayer(packed_layer_name, "XPack");
						xgraph_opt->layers[packed_layer_name] = dst0;

						dst0->xpack_params->pack=true;
						dst0->opcode=OPCODE_XPACK;

						//string uniqname = xgraph_opt->getUniqueBlobName();
						string username = packed_layer_name;
						XBlob* tmp_blob = new XBlob(username);
						xgraph_opt->blobs[username] = tmp_blob;

						nameIndex new_blob(tmp_blob, 0);

						dst0->bottom.push_back(it->second->top[iter_top_b]);

						dst0->bottomShape.push_back(t_blob->shape);

						dst0->top.push_back(new_blob);
						tmp_blob->shape = cons_layer_it->second->topShape[0];
						dst0->topShape.push_back(tmp_blob->shape);

						tmp_blob->producers.push_back(dst0->name);

						tmp_blob->consumers.push_back(cons_layer_it->second->name);
						dst0->quantization_scheme = cons_layer_it->second->quantization_scheme;
						for(int iter_c_b_b=0;iter_c_b_b<cons_layer_it->second->bottom.size();iter_c_b_b++){

							for(int iter_c_b_p=0;iter_c_b_p<cons_layer_it->second->bottom[iter_c_b_b].blob->producers.size();iter_c_b_p++){

								if(cons_layer_it->second->bottom[iter_c_b_b].blob->producers[iter_c_b_p].compare(it->second->name)==0){

									cons_layer_it->second->bottom[iter_c_b_b]=new_blob;

								}
							}
						}

						for(int iter_t_b_prod=0;iter_t_b_prod<t_blob->producers.size();iter_t_b_prod++){

							if(t_blob->producers[iter_t_b_prod].compare(it->second->name)==0){

								t_blob->consumers[iter_t_b_prod]=dst0->name;

							}
						}
						//it->second->top[iter_c]=tmp_blob;
						//dst0->xpack_params->fbits=(int)it->second->xcustom_params->float_args[1];// this needs to fix as below
						//dst0->xpack_params->fbits=cons_layer_it->second->ip_fl;
						dst0->xpack_params->fbits=it->second->op_fl;
						custom_layer_cnt++;
						if(!dst0->xpack_params->fbits)
							dst0->xpack_params->fbits=unpack_flag_fbit;
						continue;
					}

					std::vector<string>::iterator in_unpacked_type_it=std::find(in_unpacked.begin(),in_unpacked.end(),cons_layer_it->second->type);


					if(in_unpacked_type_it!=in_unpacked.end()){ // one of the custom layer producer is generates packed output, so that here need a unpacked layers.

						continue;
					}

				}
			}
		}
	}
	delete custom_layer_type;
}

void XGraphOpt::opt_separable_conv(XGraph *xgraph_opt){
	string *relu_layer_type = new string("ReLU");

	string *conv_layer_type = new string("Convolution");

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();it++){

		if(it->second->type.compare(*conv_layer_type)==0){

			if(it->second->conv_params->group>2){


				if(it->second->conv_params->M==it->second->conv_params->group){

					if(it->second->top.size()==1){
						int tem_3d_relu_flag = 0;

						map < string, XBlob*>::iterator it_top_b = xgraph_opt->blobs.find(it->second->top[0].blob->name);

						if(it_top_b->second->consumers.size()==1){

							map < string, XLayer*>::iterator it_cun_layer = xgraph_opt->layers.find(it_top_b->second->consumers[0]);

							if(it_cun_layer->second->type.compare(*relu_layer_type)==0){


								map < string, XBlob*>::iterator it_top_rb = xgraph_opt->blobs.find(it_cun_layer->second->top[0].blob->name);

								if(it_top_rb->second->consumers.size()==1){

									map < string, XLayer*>::iterator it_relu_cun_layer = xgraph_opt->layers.find(it_top_rb->second->consumers[0]);

									if(it_relu_cun_layer->second->type.compare(*conv_layer_type)==0){

										it_relu_cun_layer->second->conv_params->reluflag_3d=1;

										xgrap_layer_delete(xgraph_opt,it_cun_layer->second->name);

										it_relu_cun_layer->second->conv_params->M_3d = it->second->conv_params->M;
										it_relu_cun_layer->second->conv_params->N_3d = it->second->conv_params->N;
										it_relu_cun_layer->second->conv_params->filter_h_3d = it->second->conv_params->filter_h;
										it_relu_cun_layer->second->conv_params->filter_w_3d = it->second->conv_params->filter_w;

										it_relu_cun_layer->second->conv_params->stride_h_3d = it->second->conv_params->stride_h;
										it_relu_cun_layer->second->conv_params->stride_w_3d = it->second->conv_params->stride_w;
										it_relu_cun_layer->second->conv_params->pad_h_3d = it->second->conv_params->pad_h;
										it_relu_cun_layer->second->conv_params->pad_w_3d = it->second->conv_params->pad_w;
										it_relu_cun_layer->second->conv_params->dilation_3d = it->second->conv_params->dilation;
										it_relu_cun_layer->second->conv_params->group_3d = it->second->conv_params->group;
										it_relu_cun_layer->second->conv_params->has_bias_3d = it->second->conv_params->has_bias;
										it_relu_cun_layer->second->conv_params->weightsPath_3d = it->second->conv_params->weightsPath;

										it_relu_cun_layer->second->conv_params->biasPath_3d = it->second->conv_params->biasPath;
										it_relu_cun_layer->second->conv_params->weights_3d = it->second->conv_params->weights;
										it_relu_cun_layer->second->conv_params->bias_3d = it->second->conv_params->bias;
										it_relu_cun_layer->second->conv_params->weightsDim_3d = it->second->conv_params->weightsDim;
										it_relu_cun_layer->second->conv_params->biasDim_3d = it->second->conv_params->biasDim;

										it_relu_cun_layer->second->conv_params->ip_bw_3d = it->second->ip_bw;
										it_relu_cun_layer->second->conv_params->ip_fl_3d = it->second->ip_fl;

										it_relu_cun_layer->second->conv_params->op_bw_3d = it->second->op_bw;
										it_relu_cun_layer->second->conv_params->op_fl_3d = it->second->op_fl;
										it_relu_cun_layer->second->conv_params->wt_bw_3d = it->second->wt_bw;
										it_relu_cun_layer->second->conv_params->wt_fl_3d = it->second->wt_fl;
										it_relu_cun_layer->second->conv_params->th_layer_in_3d = it->second->th_layer_in;
										it_relu_cun_layer->second->conv_params->th_layer_out_3d = it->second->th_layer_out;
										it_relu_cun_layer->second->conv_params->th_params_3d = it->second->th_params;


										for(int iter_bs=0;iter_bs<it->second->bottomShape.size();iter_bs++){
											it_relu_cun_layer->second->bottomShape[iter_bs]=it->second->bottomShape[iter_bs];
										}
										//xgrap_layer_delete(xgraph_opt,it->second->name);
										it_relu_cun_layer->second->opcode=OPCODE_3D_CONV;

										map < string, XBlob*>::iterator bottom_blob = xgraph_opt->blobs.find(it_relu_cun_layer->second->bottom[0].blob->name);
										//for(int iter_it_b=0;iter_it_b<it->second->bottom.size();iter_it_b++){

										std::vector<string>::iterator it_str=std::find(it->second->bottom[0].blob->consumers.begin(),it->second->bottom[0].blob->consumers.end(),it->second->name);

										if(it_str!=it->second->bottom[0].blob->consumers.end())
										{
											it->second->bottom[0].blob->consumers.erase(it_str);
										}
										//it->second->bottom[0].blob->consumers.erase(it->second->bottom[0].blob->consumers.begin()+0);

										it_relu_cun_layer->second->bottom[0].blob=it->second->bottom[0].blob;
										it_relu_cun_layer->second->bottom[0].id = it->second->bottom[0].id;

										it->second->bottom[0].blob->consumers.push_back(it_relu_cun_layer->second->name);

										xgraph_opt->blobs.erase(bottom_blob);
										xgraph_opt->layers.erase(it);
									}
								}
							}else if(it_cun_layer->second->type.compare(*conv_layer_type)==0){

								it_cun_layer->second->conv_params->M_3d = it->second->conv_params->M;
								it_cun_layer->second->conv_params->N_3d = it->second->conv_params->N;
								it_cun_layer->second->conv_params->filter_h_3d = it->second->conv_params->filter_h;
								it_cun_layer->second->conv_params->filter_w_3d = it->second->conv_params->filter_w;

								it_cun_layer->second->conv_params->stride_h_3d = it->second->conv_params->stride_h;
								it_cun_layer->second->conv_params->stride_w_3d = it->second->conv_params->stride_w;
								it_cun_layer->second->conv_params->pad_h_3d = it->second->conv_params->pad_h;
								it_cun_layer->second->conv_params->pad_w_3d = it->second->conv_params->pad_w;
								it_cun_layer->second->conv_params->dilation_3d = it->second->conv_params->dilation;
								it_cun_layer->second->conv_params->group_3d = it->second->conv_params->group;
								it_cun_layer->second->conv_params->has_bias_3d = it->second->conv_params->has_bias;
								it_cun_layer->second->conv_params->weightsPath_3d = it->second->conv_params->weightsPath;

								it_cun_layer->second->conv_params->biasPath_3d = it->second->conv_params->biasPath;
								it_cun_layer->second->conv_params->weights_3d = it->second->conv_params->weights;
								it_cun_layer->second->conv_params->bias_3d = it->second->conv_params->bias;
								it_cun_layer->second->conv_params->weightsDim_3d = it->second->conv_params->weightsDim;
								it_cun_layer->second->conv_params->biasDim_3d = it->second->conv_params->biasDim;

								it_cun_layer->second->conv_params->ip_bw_3d = it->second->ip_bw;
								it_cun_layer->second->conv_params->ip_fl_3d = it->second->ip_fl;

								it_cun_layer->second->conv_params->op_bw_3d = it->second->op_bw;
								it_cun_layer->second->conv_params->op_fl_3d = it->second->op_fl;
								it_cun_layer->second->conv_params->wt_bw_3d = it->second->wt_bw;
								it_cun_layer->second->conv_params->wt_fl_3d = it->second->wt_fl;
								it_cun_layer->second->conv_params->th_layer_in_3d = it->second->th_layer_in;
								it_cun_layer->second->conv_params->th_layer_out_3d = it->second->th_layer_out;
								it_cun_layer->second->conv_params->th_params_3d = it->second->th_params;

								for(int iter_bs=0;iter_bs<it->second->bottomShape.size();iter_bs++){
									it_cun_layer->second->bottomShape[iter_bs]=it->second->bottomShape[iter_bs];
								}
								//xgrap_layer_delete(xgraph_opt,it->second->name);

								it_cun_layer->second->opcode=OPCODE_3D_CONV;
								map < string, XBlob*>::iterator bottom_blob = xgraph_opt->blobs.find(it_cun_layer->second->bottom[0].blob->name);
								//for(int iter_it_b=0;iter_it_b<it->second->bottom.size();iter_it_b++){

								std::vector<string>::iterator it_str=std::find(it->second->bottom[0].blob->consumers.begin(),it->second->bottom[0].blob->consumers.end(),it->second->name);

								if(it_str!=it->second->bottom[0].blob->consumers.end())
								{
									it->second->bottom[0].blob->consumers.erase(it_str);
								}
								//it->second->bottom[0].blob->consumers.erase(it->second->bottom[0].blob->consumers.begin()+0);

								it_cun_layer->second->bottom[0].blob=it->second->bottom[0].blob;
								it_cun_layer->second->bottom[0].id = it->second->bottom[0].id;

								//it->second->bottom[0].blob->consumers.erase(it->second->bottom[0].blob->consumers.begin()+0);
								//it->second->bottom[0].blob->producers.erase(it->second->bottom[0].blob->producers.begin()+0);

								it->second->bottom[0].blob->consumers.push_back(it_cun_layer->second->name);

								xgraph_opt->blobs.erase(bottom_blob);
								xgraph_opt->layers.erase(it);
								//it_cun_layer->second->opcode=OPCODE_3D_CONV;
							}
						}else{


							if(it_top_b->second->consumers.size()==2){

								int cnt_type =0;
								int conv_indx=-1;
								int relu_indx=-1;
								int l_conv_f=0;
								vector<string> name_layer_vect;
								string tmp_conv_name ;
								string tmp_relu_name ;
								vector<map < string, XLayer*>::iterator> vect_layer_it;

								for(int iter_c_t=0;iter_c_t<it_top_b->second->consumers.size();iter_c_t++){

									map < string, XLayer*>::iterator cuns_layer_t = xgraph_opt->layers.find(it_top_b->second->consumers[iter_c_t]);
									vect_layer_it.push_back(cuns_layer_t);
									if((cuns_layer_t->second->type.compare(*conv_layer_type)==0)&&(l_conv_f==0)){
										tmp_conv_name = cuns_layer_t->second->name;
										conv_indx=iter_c_t;
										cnt_type++;
										l_conv_f=1;

									}else{
										if(cuns_layer_t->second->type.compare(*relu_layer_type)==0){
											tmp_relu_name = cuns_layer_t->second->name;
											relu_indx=iter_c_t;
											cnt_type++;
										}
									}
								}
								if(cnt_type==2){

									vect_layer_it[conv_indx]->second->conv_params->M_3d = it->second->conv_params->M;
									vect_layer_it[conv_indx]->second->conv_params->N_3d = it->second->conv_params->N;
									vect_layer_it[conv_indx]->second->conv_params->filter_h_3d = it->second->conv_params->filter_h;
									vect_layer_it[conv_indx]->second->conv_params->filter_w_3d = it->second->conv_params->filter_w;

									vect_layer_it[conv_indx]->second->conv_params->stride_h_3d = it->second->conv_params->stride_h;
									vect_layer_it[conv_indx]->second->conv_params->stride_w_3d = it->second->conv_params->stride_w;
									vect_layer_it[conv_indx]->second->conv_params->pad_h_3d = it->second->conv_params->pad_h;
									vect_layer_it[conv_indx]->second->conv_params->pad_w_3d = it->second->conv_params->pad_w;
									vect_layer_it[conv_indx]->second->conv_params->dilation_3d = it->second->conv_params->dilation;
									vect_layer_it[conv_indx]->second->conv_params->group_3d = it->second->conv_params->group;
									vect_layer_it[conv_indx]->second->conv_params->has_bias_3d = it->second->conv_params->has_bias;
									vect_layer_it[conv_indx]->second->conv_params->weightsPath_3d = it->second->conv_params->weightsPath;

									vect_layer_it[conv_indx]->second->conv_params->biasPath_3d = it->second->conv_params->biasPath;
									vect_layer_it[conv_indx]->second->conv_params->weights_3d = it->second->conv_params->weights;
									vect_layer_it[conv_indx]->second->conv_params->bias_3d = it->second->conv_params->bias;
									vect_layer_it[conv_indx]->second->conv_params->weightsDim_3d = it->second->conv_params->weightsDim;
									vect_layer_it[conv_indx]->second->conv_params->biasDim_3d = it->second->conv_params->biasDim;
									vect_layer_it[conv_indx]->second->conv_params->reluflag_3d=1;
									vect_layer_it[conv_indx]->second->conv_params->ip_bw_3d = it->second->ip_bw;
									vect_layer_it[conv_indx]->second->conv_params->ip_fl_3d = it->second->ip_fl;

									vect_layer_it[conv_indx]->second->conv_params->op_bw_3d = it->second->op_bw;
									vect_layer_it[conv_indx]->second->conv_params->op_fl_3d = it->second->op_fl;
									vect_layer_it[conv_indx]->second->conv_params->wt_bw_3d = it->second->wt_bw;
									vect_layer_it[conv_indx]->second->conv_params->wt_fl_3d = it->second->wt_fl;

									vect_layer_it[conv_indx]->second->conv_params->th_layer_in_3d = it->second->th_layer_in;
									vect_layer_it[conv_indx]->second->conv_params->th_layer_out_3d = it->second->th_layer_out;
									vect_layer_it[conv_indx]->second->conv_params->th_params_3d = it->second->th_params;

									for(int iter_bs=0;iter_bs<it->second->bottomShape.size();iter_bs++){
										vect_layer_it[conv_indx]->second->bottomShape[iter_bs]=it->second->bottomShape[iter_bs];
									}

									xgrap_layer_delete(xgraph_opt,vect_layer_it[relu_indx]->second->name);
									//xgrap_layer_delete(xgraph_opt,it->second->name);

									vect_layer_it[conv_indx]->second->opcode=OPCODE_3D_CONV;
									map < string, XBlob*>::iterator bottom_blob = xgraph_opt->blobs.find(vect_layer_it[conv_indx]->second->bottom[0].blob->name);
									//for(int iter_it_b=0;iter_it_b<it->second->bottom.size();iter_it_b++){

									std::vector<string>::iterator it_str=std::find(it->second->bottom[0].blob->consumers.begin(),it->second->bottom[0].blob->consumers.end(),it->second->name);

									if(it_str!=it->second->bottom[0].blob->consumers.end())
									{
										it->second->bottom[0].blob->consumers.erase(it_str);
									}
									//it->second->bottom[0].blob->consumers.erase(it->second->bottom[0].blob->consumers.begin()+0);

									vect_layer_it[conv_indx]->second->bottom[0].blob=it->second->bottom[0].blob;
									vect_layer_it[conv_indx]->second->bottom[0].id = it->second->bottom[0].id;

									//it->second->bottom[0].blob->consumers.erase(it->second->bottom[0].blob->consumers.begin()+0);
									//it->second->bottom[0].blob->producers.erase(it->second->bottom[0].blob->producers.begin()+0);
									vect_layer_it[conv_indx]->second->intermediateShape.push_back(it->second->topShape[0]);
									it->second->bottom[0].blob->consumers.push_back(vect_layer_it[conv_indx]->second->name);

									xgraph_opt->blobs.erase(bottom_blob);
									xgraph_opt->layers.erase(it);
									//}
								}
							}
						}
					}else
					{
						cerr << "[EO001] Due to mismatch between layer group value and output dimension, Current version support layer group value should be <=2 or group == output dimension(3D separable convolution )" << endl;
						cerr<<"Current layer group = " <<it->second->conv_params->group <<" output dimension = "<<it->second->conv_params->M<<endl;									exit(-1);
					}
				}
			}
		}
	}

	delete conv_layer_type;
}
void XGraphOpt::opt_separable_conv_only(XGraph *xgraph_opt){

	string *conv_layer_type = new string("Convolution");
	int conv_3d_only=0;

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();it++){

		if(it->second->type.compare(*conv_layer_type)==0){

			if(it->second->conv_params->group>2){

				if(it->second->conv_params->M==it->second->conv_params->group){

					string layer_name = "conv_3d_only";
					std::stringstream ss1;

					ss1<<layer_name<<conv_3d_only;
					string conv_3d_layer_name = ss1.str();//"conv_lrn1";

					XLayer* dst0 = new XLayer(conv_3d_layer_name, "Convolution");

					xgraph_opt->layers[conv_3d_layer_name] = dst0;

					map < string, XBlob*>::iterator top_blob_it;
					map < string, XBlob*>::iterator bottom_blob_it;

					for(int iter_top=0;iter_top<it->second->top.size();iter_top++){

						dst0->top.push_back(it->second->top[iter_top]);
						dst0->topShape.push_back(it->second->topShape[iter_top]);
						string top_blob_name = it->second->top[iter_top].blob->name;

						top_blob_it = xgraph_opt->blobs.find(top_blob_name);

						for(int iter_t_p=0;iter_t_p<top_blob_it->second->producers.size();iter_t_p++){

							if(top_blob_it->second->producers[iter_t_p].compare(it->second->name)==0){
								top_blob_it->second->producers[iter_t_p] = conv_3d_layer_name;
							}
						}
					}

					for(int iter_bot=0;iter_bot<it->second->bottom.size();iter_bot++){

						dst0->bottom.push_back(it->second->bottom[iter_bot]);
						dst0->bottomShape.push_back(it->second->bottomShape[iter_bot]);

						string bottom_blob_name = it->second->bottom[iter_bot].blob->name;

						bottom_blob_it = xgraph_opt->blobs.find(bottom_blob_name);

						for(int iter_b_c=0;iter_b_c<bottom_blob_it->second->consumers.size();iter_b_c++){

							if(bottom_blob_it->second->consumers[iter_b_c].compare(it->second->name)==0){
								bottom_blob_it->second->consumers[iter_b_c] = conv_3d_layer_name;
							}
						}

					}

					dst0->conv_params->M_3d = it->second->conv_params->M;
					dst0->conv_params->N_3d = it->second->conv_params->N;
					dst0->conv_params->filter_h_3d = it->second->conv_params->filter_h;
					dst0->conv_params->filter_w_3d = it->second->conv_params->filter_w;

					dst0->conv_params->stride_h_3d = it->second->conv_params->stride_h;
					dst0->conv_params->stride_w_3d = it->second->conv_params->stride_w;
					dst0->conv_params->pad_h_3d = it->second->conv_params->pad_h;
					dst0->conv_params->pad_w_3d = it->second->conv_params->pad_w;
					dst0->conv_params->dilation_3d = it->second->conv_params->dilation;
					dst0->conv_params->group_3d = it->second->conv_params->group;
					dst0->conv_params->has_bias_3d = it->second->conv_params->has_bias;
					dst0->conv_params->weightsPath_3d = it->second->conv_params->weightsPath;

					dst0->conv_params->biasPath_3d = it->second->conv_params->biasPath;
					dst0->conv_params->weights_3d = it->second->conv_params->weights;
					dst0->conv_params->bias_3d = it->second->conv_params->bias;
					dst0->conv_params->weightsDim_3d = it->second->conv_params->weightsDim;
					dst0->conv_params->biasDim_3d = it->second->conv_params->biasDim;

					dst0->conv_params->ip_bw_3d = it->second->ip_bw;
					dst0->conv_params->ip_fl_3d = it->second->ip_fl;

					dst0->conv_params->op_bw_3d = it->second->op_bw;
					dst0->conv_params->op_fl_3d = it->second->op_fl;
					dst0->conv_params->wt_bw_3d = it->second->wt_bw;
					dst0->conv_params->wt_fl_3d = it->second->wt_fl;
					dst0->conv_params->th_layer_in_3d = it->second->th_layer_in;
					dst0->conv_params->th_layer_out_3d = it->second->th_layer_out;
					dst0->conv_params->th_params_3d = it->second->th_params;
					dst0->intermediateShape.push_back(it->second->topShape[0]);
					dst0->quantization_scheme = it->second->quantization_scheme;
					dst0->output_file=it->second->output_file;
					dst0->opcode=OPCODE_SEPARABLE_CONV;
					dst0->conv_params->reluflag=false;
					//map < string, XLayer*>::iterator it_cut = it;

					mapStrXLayer::iterator cur_it = it;
					it++;
					xgraph_opt->layers.erase(cur_it);
					conv_3d_only++;
				}else{
					it++;
					cerr << "[EO001] Due to mismatch between layer group value and output dimension, Current version support layer group value should be <=2 or group == output dimension(3D separable convolution )" << endl;
					cerr<<"Current layer group = " <<it->second->conv_params->group <<" output dimension = "<<it->second->conv_params->M<<endl;									exit(-1);
				}
			}else{
				it++;
			}
		}
	}

	delete conv_layer_type;
}
void XGraphOpt::opt_sw_eltwise(XGraph *xgraph_opt){

	string *eltwise_layer_type = new string("Eltwise");

	int eltwise_cnt=0;

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();it++){

		if (it->second->type.compare(*eltwise_layer_type)==0){

			it->second->opcode = OPCODE_SW_ELTWISE_ADD;

		}
	}
	delete eltwise_layer_type;
}
bool check_parallel_params(vector<XLayer*> &vec_layer,vector<XLayer*> &layer_list){

	int kernel_h;
	int kernel_w;
	int stride_h;
	int stride_w;
	int pad_h;
	int pad_w;
	int op_fl;
	int op_bw;
	int ip_fl;
	int ip_bw;
	int wt_fl;
	int wt_bw;
	int output_h;
	int output_w;
	vector<string> conv_k_list,conv_s_list,conv_p_list,conv_o_list,conv_out_fl,conv_out_bw,conv_relu_list,conv_wt_fl,conv_wt_bw,conv_in_fl,conv_in_bw;




	for(int iter_p_c=0;iter_p_c<vec_layer.size();iter_p_c++){

		kernel_h=vec_layer[iter_p_c]->conv_params->filter_h;
		kernel_w=vec_layer[iter_p_c]->conv_params->filter_w;

		for(int f_s=iter_p_c+1;f_s<vec_layer.size();f_s++){

			if((kernel_h==vec_layer[f_s]->conv_params->filter_h) && (kernel_w==vec_layer[f_s]->conv_params->filter_w)){

				std::vector<string>::iterator it_str1=std::find(conv_k_list.begin(),conv_k_list.end(),vec_layer[iter_p_c]->name);

				if(it_str1==conv_k_list.end())
				{
					conv_k_list.push_back(vec_layer[iter_p_c]->name);
					layer_list.push_back(vec_layer[iter_p_c]);

				}

				std::vector<string>::iterator it_str=std::find(conv_k_list.begin(),conv_k_list.end(),vec_layer[f_s]->name);
				if(it_str==conv_k_list.end())
				{
					conv_k_list.push_back(vec_layer[f_s]->name);
					layer_list.push_back(vec_layer[f_s]);

				}
			}
		}
	}


	if(layer_list.size()<=1){

		cout<<"layer_list.size()<=1"<<endl;
		return false;

	}else{

		if(!(layer_list.size()==conv_k_list.size())){
			return false;
		}

		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			op_fl=layer_list[iter_l]->op_fl;
			op_bw=layer_list[iter_l]->op_bw;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if(op_fl==layer_list[f_s]->op_fl){

					std::vector<string>::iterator it_str1=std::find(conv_out_fl.begin(),conv_out_fl.end(),layer_list[iter_l]->name);

					if(it_str1==conv_out_fl.end())
					{
						conv_out_fl.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_out_fl.begin(),conv_out_fl.end(),layer_list[f_s]->name);
					if(it_str==conv_out_fl.end())
					{
						conv_out_fl.push_back(layer_list[f_s]->name);

					}

				}
				if(op_bw==layer_list[f_s]->op_bw){

					std::vector<string>::iterator it_str=std::find(conv_out_bw.begin(),conv_out_bw.end(),layer_list[iter_l]->name);

					if(it_str==conv_out_bw.end())
					{
						conv_out_bw.push_back(layer_list[iter_l]->name);

					}
					it_str=std::find(conv_out_bw.begin(),conv_out_bw.end(),layer_list[f_s]->name);
					if(it_str==conv_out_bw.end())
					{
						conv_out_bw.push_back(layer_list[f_s]->name);

					}
				}
			}
		}
		if(!(layer_list.size()==conv_out_fl.size()))
			return false;

		if(!(layer_list.size()==conv_out_bw.size()))
			return false;

		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			ip_fl=layer_list[iter_l]->ip_fl;
			ip_bw=layer_list[iter_l]->ip_bw;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if(ip_fl==layer_list[f_s]->ip_fl){

					std::vector<string>::iterator it_str1=std::find(conv_in_fl.begin(),conv_in_fl.end(),layer_list[iter_l]->name);

					if(it_str1==conv_in_fl.end())
					{
						conv_in_fl.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_in_fl.begin(),conv_in_fl.end(),layer_list[f_s]->name);
					if(it_str==conv_in_fl.end())
					{
						conv_in_fl.push_back(layer_list[f_s]->name);

					}

				}
				if(ip_bw==layer_list[f_s]->ip_bw){

					std::vector<string>::iterator it_str=std::find(conv_in_bw.begin(),conv_in_bw.end(),layer_list[iter_l]->name);

					if(it_str==conv_in_bw.end())
					{
						conv_in_bw.push_back(layer_list[iter_l]->name);

					}
					it_str=std::find(conv_in_bw.begin(),conv_in_bw.end(),layer_list[f_s]->name);
					if(it_str==conv_in_bw.end())
					{
						conv_in_bw.push_back(layer_list[f_s]->name);

					}
				}
			}
		}
		if(!(layer_list.size()==conv_in_fl.size()))
			return false;
		if(!(layer_list.size()==conv_in_bw.size()))
			return false;

		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			wt_fl=layer_list[iter_l]->wt_fl;
			wt_bw=layer_list[iter_l]->wt_bw;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if(wt_fl==layer_list[f_s]->wt_fl){

					std::vector<string>::iterator it_str1=std::find(conv_wt_fl.begin(),conv_wt_fl.end(),layer_list[iter_l]->name);

					if(it_str1==conv_wt_fl.end())
					{
						conv_wt_fl.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_wt_fl.begin(),conv_wt_fl.end(),layer_list[f_s]->name);
					if(it_str==conv_wt_fl.end())
					{
						conv_wt_fl.push_back(layer_list[f_s]->name);

					}

				}
				if(wt_bw==layer_list[f_s]->wt_bw){

					std::vector<string>::iterator it_str=std::find(conv_wt_bw.begin(),conv_wt_bw.end(),layer_list[iter_l]->name);

					if(it_str==conv_wt_bw.end())
					{
						conv_wt_bw.push_back(layer_list[iter_l]->name);

					}
					it_str=std::find(conv_wt_bw.begin(),conv_wt_bw.end(),layer_list[f_s]->name);
					if(it_str==conv_wt_bw.end())
					{
						conv_wt_bw.push_back(layer_list[f_s]->name);

					}
				}
			}
		}

		if(!(layer_list.size()==conv_wt_fl.size()))
			return false;
		if(!(layer_list.size()==conv_wt_bw.size()))
			return false;
		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			pad_h=layer_list[iter_l]->conv_params->pad_h;
			pad_w=layer_list[iter_l]->conv_params->pad_w;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if((pad_h==layer_list[f_s]->conv_params->pad_h) && (pad_w==layer_list[f_s]->conv_params->pad_w)){

					std::vector<string>::iterator it_str1=std::find(conv_p_list.begin(),conv_p_list.end(),layer_list[iter_l]->name);

					if(it_str1==conv_p_list.end())
					{
						conv_p_list.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_p_list.begin(),conv_p_list.end(),layer_list[f_s]->name);
					if(it_str==conv_p_list.end())
					{
						conv_p_list.push_back(layer_list[f_s]->name);

					}
				}
			}
		}
		if(!(layer_list.size()==conv_p_list.size()))
			return false;
		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			output_h=layer_list[iter_l]->topShape[0].at(2);
			output_w=layer_list[iter_l]->topShape[0].at(3);

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if((output_h==layer_list[f_s]->topShape[0].at(2)) && (output_w==layer_list[f_s]->topShape[0].at(3))){

					std::vector<string>::iterator it_str1=std::find(conv_o_list.begin(),conv_o_list.end(),layer_list[iter_l]->name);

					if(it_str1==conv_o_list.end())
					{
						conv_o_list.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_o_list.begin(),conv_o_list.end(),layer_list[f_s]->name);
					if(it_str==conv_o_list.end())
					{
						conv_o_list.push_back(layer_list[f_s]->name);

					}
				}
			}
		}
		if(!(layer_list.size()==conv_o_list.size()))
			return false;
		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			stride_h=layer_list[iter_l]->conv_params->stride_h;
			stride_w=layer_list[iter_l]->conv_params->stride_w;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if((stride_h==layer_list[f_s]->conv_params->stride_h) && (stride_w==layer_list[f_s]->conv_params->stride_w)){

					std::vector<string>::iterator it_str1=std::find(conv_s_list.begin(),conv_s_list.end(),layer_list[iter_l]->name);

					if(it_str1==conv_s_list.end())
					{
						conv_s_list.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_s_list.begin(),conv_s_list.end(),layer_list[f_s]->name);
					if(it_str==conv_s_list.end())
					{
						conv_s_list.push_back(layer_list[f_s]->name);

					}
				}
			}
		}
		if(!(layer_list.size()==conv_s_list.size()))
			return false;
	}
	return true;
}
bool check_parallel_params_offline(vector<XLayer*> &vec_layer,vector<XLayer*> &layer_list){

	int kernel_h;
	int kernel_w;
	int stride_h;
	int stride_w;
	int pad_h;
	int pad_w;

	int op_bw;

	int ip_bw;

	int wt_bw;
	int output_h;
	int output_w;
	float th_layer_in;
	float th_layer_out;
	vector<string> conv_k_list,conv_s_list,conv_p_list,conv_o_list,conv_out_bw,conv_relu_list,conv_wt_fl,conv_wt_bw,conv_in_bw;
	vector<string>conv_out_th,conv_in_th;



	for(int iter_p_c=0;iter_p_c<vec_layer.size();iter_p_c++){

		kernel_h=vec_layer[iter_p_c]->conv_params->filter_h;
		kernel_w=vec_layer[iter_p_c]->conv_params->filter_w;

		for(int f_s=iter_p_c+1;f_s<vec_layer.size();f_s++){

			if((kernel_h==vec_layer[f_s]->conv_params->filter_h) && (kernel_w==vec_layer[f_s]->conv_params->filter_w)){

				std::vector<string>::iterator it_str1=std::find(conv_k_list.begin(),conv_k_list.end(),vec_layer[iter_p_c]->name);

				if(it_str1==conv_k_list.end())
				{
					conv_k_list.push_back(vec_layer[iter_p_c]->name);
					layer_list.push_back(vec_layer[iter_p_c]);

				}

				std::vector<string>::iterator it_str=std::find(conv_k_list.begin(),conv_k_list.end(),vec_layer[f_s]->name);
				if(it_str==conv_k_list.end())
				{
					conv_k_list.push_back(vec_layer[f_s]->name);
					layer_list.push_back(vec_layer[f_s]);

				}
			}
		}
	}


	if(layer_list.size()<=1){

		cout<<"layer_list.size()<=1"<<endl;
		return false;

	}else{

		if(!(layer_list.size()==conv_k_list.size())){
			return false;
		}

		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			op_bw=layer_list[iter_l]->op_bw;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){



				if(op_bw==layer_list[f_s]->op_bw){

					std::vector<string>::iterator it_str=std::find(conv_out_bw.begin(),conv_out_bw.end(),layer_list[iter_l]->name);

					if(it_str==conv_out_bw.end())
					{
						conv_out_bw.push_back(layer_list[iter_l]->name);

					}
					it_str=std::find(conv_out_bw.begin(),conv_out_bw.end(),layer_list[f_s]->name);
					if(it_str==conv_out_bw.end())
					{
						conv_out_bw.push_back(layer_list[f_s]->name);

					}
				}
			}
		}
		if(!(layer_list.size()==conv_out_bw.size()))
			return false;

		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			float out_th=layer_list[iter_l]->th_layer_out;
			float in_th=layer_list[iter_l]->th_layer_in;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if(out_th==layer_list[f_s]->th_layer_out){

					std::vector<string>::iterator it_str1=std::find(conv_out_th.begin(),conv_out_th.end(),layer_list[iter_l]->name);

					if(it_str1==conv_out_th.end())
					{
						conv_out_th.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_out_th.begin(),conv_out_th.end(),layer_list[f_s]->name);
					if(it_str==conv_out_th.end())
					{
						conv_out_th.push_back(layer_list[f_s]->name);

					}

				}
				if(in_th==layer_list[f_s]->th_layer_in){

					std::vector<string>::iterator it_str=std::find(conv_in_th.begin(),conv_in_th.end(),layer_list[iter_l]->name);

					if(it_str==conv_in_th.end())
					{
						conv_in_th.push_back(layer_list[iter_l]->name);

					}
					it_str=std::find(conv_in_th.begin(),conv_in_th.end(),layer_list[f_s]->name);
					if(it_str==conv_in_th.end())
					{
						conv_in_th.push_back(layer_list[f_s]->name);

					}
				}
			}
		}
		if(!(layer_list.size()==conv_out_th.size()))
			return false;
		if(!(layer_list.size()==conv_in_th.size()))
			return false;


		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			//			ip_fl=layer_list[iter_l]->ip_fl;
			ip_bw=layer_list[iter_l]->ip_bw;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){

				if(ip_bw==layer_list[f_s]->ip_bw){

					std::vector<string>::iterator it_str=std::find(conv_in_bw.begin(),conv_in_bw.end(),layer_list[iter_l]->name);

					if(it_str==conv_in_bw.end())
					{
						conv_in_bw.push_back(layer_list[iter_l]->name);

					}
					it_str=std::find(conv_in_bw.begin(),conv_in_bw.end(),layer_list[f_s]->name);
					if(it_str==conv_in_bw.end())
					{
						conv_in_bw.push_back(layer_list[f_s]->name);

					}
				}
			}
		}

		if(!(layer_list.size()==conv_in_bw.size()))
			return false;

		for(int iter_l=0;iter_l<layer_list.size();iter_l++){


			wt_bw=layer_list[iter_l]->wt_bw;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){



				if(wt_bw==layer_list[f_s]->wt_bw){

					std::vector<string>::iterator it_str=std::find(conv_wt_bw.begin(),conv_wt_bw.end(),layer_list[iter_l]->name);

					if(it_str==conv_wt_bw.end())
					{
						conv_wt_bw.push_back(layer_list[iter_l]->name);

					}
					it_str=std::find(conv_wt_bw.begin(),conv_wt_bw.end(),layer_list[f_s]->name);
					if(it_str==conv_wt_bw.end())
					{
						conv_wt_bw.push_back(layer_list[f_s]->name);

					}
				}
			}
		}


		if(!(layer_list.size()==conv_wt_bw.size()))
			return false;
		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			pad_h=layer_list[iter_l]->conv_params->pad_h;
			pad_w=layer_list[iter_l]->conv_params->pad_w;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if((pad_h==layer_list[f_s]->conv_params->pad_h) && (pad_w==layer_list[f_s]->conv_params->pad_w)){

					std::vector<string>::iterator it_str1=std::find(conv_p_list.begin(),conv_p_list.end(),layer_list[iter_l]->name);

					if(it_str1==conv_p_list.end())
					{
						conv_p_list.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_p_list.begin(),conv_p_list.end(),layer_list[f_s]->name);
					if(it_str==conv_p_list.end())
					{
						conv_p_list.push_back(layer_list[f_s]->name);

					}
				}
			}
		}
		if(!(layer_list.size()==conv_p_list.size()))
			return false;
		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			output_h=layer_list[iter_l]->topShape[0].at(2);
			output_w=layer_list[iter_l]->topShape[0].at(3);

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if((output_h==layer_list[f_s]->topShape[0].at(2)) && (output_w==layer_list[f_s]->topShape[0].at(3))){

					std::vector<string>::iterator it_str1=std::find(conv_o_list.begin(),conv_o_list.end(),layer_list[iter_l]->name);

					if(it_str1==conv_o_list.end())
					{
						conv_o_list.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_o_list.begin(),conv_o_list.end(),layer_list[f_s]->name);
					if(it_str==conv_o_list.end())
					{
						conv_o_list.push_back(layer_list[f_s]->name);

					}
				}
			}
		}
		if(!(layer_list.size()==conv_o_list.size()))
			return false;
		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			stride_h=layer_list[iter_l]->conv_params->stride_h;
			stride_w=layer_list[iter_l]->conv_params->stride_w;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if((stride_h==layer_list[f_s]->conv_params->stride_h) && (stride_w==layer_list[f_s]->conv_params->stride_w)){

					std::vector<string>::iterator it_str1=std::find(conv_s_list.begin(),conv_s_list.end(),layer_list[iter_l]->name);

					if(it_str1==conv_s_list.end())
					{
						conv_s_list.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_s_list.begin(),conv_s_list.end(),layer_list[f_s]->name);
					if(it_str==conv_s_list.end())
					{
						conv_s_list.push_back(layer_list[f_s]->name);

					}
				}
			}
		}

		if(!(layer_list.size()==conv_s_list.size()))
			return false;
	}
	return true;
}
bool parallel_conv_fuse(XGraph* xgraph_opt,map < string, XBlob*>::iterator &blob_it,vector<XLayer*> &vec_layer,int fuse_cnt){

	vector<XLayer*> layer_list;
	string quant_scheme=vec_layer[0]->quantization_scheme;
	bool status = false;

	if(quant_scheme.compare("Xilinx")!=0){

		status=check_parallel_params(vec_layer, layer_list);
	}else{
		status=check_parallel_params_offline(vec_layer,layer_list);

	}
	vector<int> blob_channel;

	vector<string> tmp_top_blob_name;
	vector<int> tmp_shape,tmp_custom;

	vector< vector<int> > tmp_custom_shape;

	if(status==true){
		int relu_false_cnt=0,relu_true_cnt=0;
		for(int iter_r=0;iter_r<layer_list.size();iter_r++){

			if(layer_list[iter_r]->conv_params->reluflag==true){
				relu_true_cnt++;
			}else{
				relu_false_cnt++;
			}
		}
		if(relu_true_cnt==0){

			if(relu_false_cnt!=layer_list.size())
				return false;
		}else{

			if(relu_true_cnt!=layer_list.size())
				return false;
		}

		for(int iter_ch=0;iter_ch<layer_list.size();iter_ch++){

			int tmp_channel=AlignSize(layer_list[iter_ch]->topShape[0].at(1),16);

			std::vector<string>::iterator it_str=std::find(tmp_top_blob_name.begin(),tmp_top_blob_name.end(),layer_list[iter_ch]->top[0].blob->name);

			if(it_str==tmp_top_blob_name.end())
			{
				tmp_top_blob_name.push_back(layer_list[iter_ch]->top[0].blob->name);

			}
			blob_channel.push_back(tmp_channel);
		}
		string layer_name = "conv_parallel_fuse";
		std::stringstream ss1;
		string tmp_batchnorm_name;
		ss1<<layer_name<<fuse_cnt;
		string fuse_layer_name = ss1.str();//"conv_lrn1";

		XLayer* dst0 = new XLayer(fuse_layer_name, "Convolution");

		xgraph_opt->layers[fuse_layer_name] = dst0;

		string username = fuse_layer_name;
		XBlob* tmp_blob = new XBlob(username);
		xgraph_opt->blobs[username] = tmp_blob;
		dst0->opcode=OPCODE_CONV;//OPCODE_PARALLEL_CONV_FUSE;
		if(relu_true_cnt)
			dst0->conv_params->reluflag=true;
		tmp_shape.push_back(blob_it->second->shape.at(0));
		int tmp_sum_c=0;
		int tmp_sum_w=0;
		int tmp_sum_h=0;
		tmp_sum_h=layer_list[0]->topShape[0].at(2);
		tmp_sum_w=layer_list[0]->topShape[0].at(3);

		for(int t_b_c=0;t_b_c<layer_list.size();t_b_c++){

			tmp_sum_c= tmp_sum_c+blob_channel[t_b_c];
			//tmp_sum_h= tmp_sum_h+layer_list[t_b_c]->topShape[0].at(2);
			//tmp_sum_w= tmp_sum_w+layer_list[t_b_c]->topShape[0].at(3);
		}

		tmp_shape.push_back(tmp_sum_c);
		tmp_shape.push_back(tmp_sum_h);
		tmp_shape.push_back(tmp_sum_w);
		vector<int> tmp_weightsDim;
		int n_out=0;
		int n_in=0;
		int n_fh=0;
		int n_fw=0;

		for(int iter_wd=0;iter_wd<layer_list.size();iter_wd++){

			n_out= n_out+AlignSize(layer_list[iter_wd]->conv_params->weightsDim[0].at(0),16);
			n_in = n_in+AlignSize(layer_list[iter_wd]->conv_params->weightsDim[0].at(1),16);
			n_fh = n_fh+layer_list[iter_wd]->conv_params->weightsDim[0].at(2);
			n_fw = n_fw+layer_list[iter_wd]->conv_params->weightsDim[0].at(3);

		}

		tmp_weightsDim.push_back(n_out);
		tmp_weightsDim.push_back(n_in);
		tmp_weightsDim.push_back(n_fh);
		tmp_weightsDim.push_back(n_fw);


		int cnt_weight=0;
		dst0->conv_params->weightsDim.push_back(tmp_weightsDim);
		vector<int> tmp_biasdim;
		tmp_biasdim.push_back(tmp_shape.at(1));
		dst0->conv_params->biasDim.push_back(tmp_biasdim);

		int o_n,i_n;
		float tmp_bias_val=0.0,tmp_th_val=0.0f;
		vector<float> tmp_weights,tmp_bias,th_params;

		for(int iter_bias_c=0;iter_bias_c<layer_list.size();iter_bias_c++){

			for(int iter_s_b=0;iter_s_b<blob_channel[iter_bias_c];iter_s_b++){

				if(layer_list[iter_bias_c]->conv_params->bias[0].size()>iter_s_b)

					tmp_bias.push_back(layer_list[iter_bias_c]->conv_params->bias[0][iter_s_b]);
				else
					tmp_bias.push_back(tmp_bias_val);
			}

		}
		if(quant_scheme.compare("Xilinx")==0){

			for(int iter_ll_c=0;iter_ll_c<layer_list.size();iter_ll_c++){

				for(int iter_th_c=0;iter_th_c<blob_channel[iter_ll_c];iter_th_c++){

					if(layer_list[iter_ll_c]->th_params.size()>iter_th_c)

						th_params.push_back(layer_list[iter_ll_c]->th_params[iter_th_c]);
					else
						th_params.push_back(tmp_th_val);
				}
			}
		}
		for(int t_b_c=0;t_b_c<layer_list.size();t_b_c++){

			o_n = layer_list[t_b_c]->conv_params->weightsDim[0].at(0);
			i_n = layer_list[t_b_c]->conv_params->weightsDim[0].at(1);

			n_out = AlignSize(layer_list[t_b_c]->conv_params->weightsDim[0].at(0),16);
			n_in = AlignSize(layer_list[t_b_c]->conv_params->weightsDim[0].at(1),16);

			for(int iter_n=0;iter_n<n_out;iter_n++){

				for(int iter_m=0;iter_m<n_in;iter_m++){

					int w_h=layer_list[t_b_c]->conv_params->weightsDim[0].at(2);

					for(int iter_fh=0;iter_fh<w_h;iter_fh++){

						int w_w=layer_list[t_b_c]->conv_params->weightsDim[0].at(3);

						for(int iter_fw=0;iter_fw<w_w;iter_fw++){

							int indx = iter_n*i_n*w_h*w_w+iter_m*w_h*w_w+iter_fh*w_w+iter_fw;
							cnt_weight=indx;

							if(o_n>iter_n ){

								if(i_n>iter_m){

									tmp_weights.push_back(layer_list[t_b_c]->conv_params->weights[0][indx]);
								}else{
									float w_dum=0.0;
									tmp_weights.push_back(w_dum);
								}
							}else{
								float w_dum=0.0;
								tmp_weights.push_back(w_dum);
							}
						}
					}
				}
			}
		}

		dst0->conv_params->weights.push_back(tmp_weights);
		dst0->conv_params->bias.push_back(tmp_bias);
		if(quant_scheme.compare("Xilinx")==0){
			dst0->th_params=th_params;
			dst0->th_layer_in = layer_list[0]->th_layer_in;
			dst0->th_layer_out = layer_list[0]->th_layer_out;
			dst0->quantization_scheme = quant_scheme;
		}
		//dst0->bottom=layer_list[0]->bottom;

		tmp_blob->shape=tmp_shape;

		tmp_blob->producerDim.push_back(tmp_blob->shape);
		if(tmp_top_blob_name.size()==1){
			tmp_blob->consumerDim.push_back(tmp_blob->shape);
		}
		else{

			if(layer_list.size()==tmp_top_blob_name.size()){
				for(int iter_b_s=0;iter_b_s<layer_list.size();iter_b_s++){
					tmp_blob->consumerDim.push_back(layer_list[iter_b_s]->top[0].blob->shape);
				}
			}else{
				cerr << "support need to added ,currenlty no support for this pattern" << endl;
				exit(-1);

			}
		}

		nameIndex new_blob(tmp_blob,0);
		nameIndex bottom_blob(blob_it->second,0);
		dst0->top.push_back(new_blob);
		dst0->bottom.push_back(bottom_blob);
		tmp_blob->producers.push_back(dst0->name);

		/*************************/
		string produce_name = blob_it->second->producers[0];
		map < string, XLayer*>::iterator it_pro_layer =xgraph_opt->layers.find(produce_name);

		dst0->conv_params->N = blob_it->second->shape.at(1);
		dst0->conv_params->M = tmp_blob->shape.at(1);
		dst0->conv_params->filter_h = layer_list[0]->conv_params->filter_h;
		dst0->conv_params->filter_w = layer_list[0]->conv_params->filter_w;
		dst0->conv_params->pad_h=layer_list[0]->conv_params->pad_h;
		dst0->conv_params->pad_w=layer_list[0]->conv_params->pad_w;
		dst0->conv_params->stride_h=layer_list[0]->conv_params->stride_h;
		dst0->conv_params->stride_w=layer_list[0]->conv_params->stride_w;
		dst0->conv_params->dilation=1;
		dst0->conv_params->group =1;

		dst0->op_bw = layer_list[0]->op_bw;
		if(quant_scheme.compare("Xilinx")!=0)
			dst0->op_fl = layer_list[0]->op_fl;
		// dst0->user_ip_bw = layer_list[0]->user_ip_bw;
		// dst0->user_ip_fl = layer_list[0]->user_ip_fl;
		dst0->ip_bw = layer_list[0]->ip_bw;
		if(quant_scheme.compare("Xilinx")!=0)
			dst0->ip_fl = layer_list[0]->ip_fl;
		dst0->wt_bw = layer_list[0]->wt_bw;
		if(quant_scheme.compare("Xilinx")!=0)
			dst0->wt_fl = layer_list[0]->wt_fl;

		dst0->output_file=fuse_layer_name;

		dst0->topShape.push_back(tmp_blob->shape);

		dst0->bottomShape.push_back(blob_it->second->shape);

		/**************************/

		for(int l_t_cus=0;l_t_cus<layer_list.size();l_t_cus++){

			for(int iter_t_cus=0;iter_t_cus<layer_list[l_t_cus]->top[0].blob->consumers.size();iter_t_cus++){

				std::vector<string>::iterator it_str=std::find(tmp_blob->consumers.begin(),tmp_blob->consumers.end(),layer_list[l_t_cus]->top[0].blob->consumers[iter_t_cus]);

				if(it_str==tmp_blob->consumers.end())
				{
					tmp_blob->consumers.push_back(layer_list[l_t_cus]->top[0].blob->consumers[iter_t_cus]);

				}

			}
		}
		std::vector<string>::iterator it_insert;
		int cnt_erase=0;
		for(int l_d=0;l_d<layer_list.size();l_d++){

			std::vector<string>::iterator it_str=std::find(blob_it->second->consumers.begin(),blob_it->second->consumers.end(),layer_list[l_d]->name);

			if(it_str!=blob_it->second->consumers.end())
			{
				if(cnt_erase==0){
					it_insert=it_str;
					cnt_erase++;
				}
				blob_it->second->consumers.erase(it_str);

			}
		}
		blob_it->second->consumers.insert(it_insert,dst0->name);


		for(int iter_l_p=0;iter_l_p<tmp_top_blob_name.size();iter_l_p++){

			map < string, XBlob* >::iterator top_blob_it = xgraph_opt->blobs.find(tmp_top_blob_name[iter_l_p]);

			for(int iter_tb_c=0;iter_tb_c<top_blob_it->second->consumers.size();iter_tb_c++){

				map < string, XLayer* >::iterator layer_it = xgraph_opt->layers.find(top_blob_it->second->consumers[iter_tb_c]);

				for(int iter_tb_cu_p=0;iter_tb_cu_p<layer_it->second->bottom.size();iter_tb_cu_p++){

					if(layer_it->second->bottom[iter_tb_cu_p].blob->name.compare(tmp_top_blob_name[iter_l_p])==0){

						layer_it->second->bottom[iter_tb_cu_p].blob =  new_blob.blob;
						layer_it->second->bottom[iter_tb_cu_p].id =  iter_l_p;
					}
				}
			}
		}
		for(int l_d=0;l_d<layer_list.size();l_d++){

			map < string, XBlob* >::iterator d_blob_it = xgraph_opt->blobs.find(layer_list[l_d]->top[0].blob->name);
			if(d_blob_it!=xgraph_opt->blobs.end())
				xgraph_opt->blobs.erase(d_blob_it);
			map < string, XLayer* >::iterator layer_it = xgraph_opt->layers.find(layer_list[l_d]->name);
			if(layer_it!=xgraph_opt->layers.end())
				xgraph_opt->layers.erase(layer_it);
		}
		return true;
	}
	return true;
}

#if 0 // this working parallel fuse.

bool parallel_conv_fuse(XGraph* xgraph_opt,map < string, XBlob*>::iterator &blob_it,vector<XLayer*> &vec_layer,int fuse_cnt){

	vector<XLayer*> layer_list;

	int kernel_h;
	int kernel_w;
	int stride_h;
	int stride_w;
	int pad_h;
	int pad_w;
	int op_fl;
	int op_bw;
	int ip_fl;
	int ip_bw;
	int wt_fl;
	int wt_bw;
	int output_h;
	int output_w;
	vector<string> conv_k_list,conv_s_list,conv_p_list,conv_o_list,conv_out_fl,conv_out_bw,conv_relu_list,conv_wt_fl,conv_wt_bw,conv_in_fl,conv_in_bw;


	vector<int> blob_channel;

	for(int iter_p_c=0;iter_p_c<vec_layer.size();iter_p_c++){

		kernel_h=vec_layer[iter_p_c]->conv_params->filter_h;
		kernel_w=vec_layer[iter_p_c]->conv_params->filter_w;

		for(int f_s=iter_p_c+1;f_s<vec_layer.size();f_s++){

			if((kernel_h==vec_layer[f_s]->conv_params->filter_h) && (kernel_w==vec_layer[f_s]->conv_params->filter_w)){

				std::vector<string>::iterator it_str1=std::find(conv_k_list.begin(),conv_k_list.end(),vec_layer[iter_p_c]->name);

				if(it_str1==conv_k_list.end())
				{
					conv_k_list.push_back(vec_layer[iter_p_c]->name);
					layer_list.push_back(vec_layer[iter_p_c]);

				}

				std::vector<string>::iterator it_str=std::find(conv_k_list.begin(),conv_k_list.end(),vec_layer[f_s]->name);
				if(it_str==conv_k_list.end())
				{
					conv_k_list.push_back(vec_layer[f_s]->name);
					layer_list.push_back(vec_layer[f_s]);

				}
			}
		}
	}

	if(layer_list.size()<=1){

		cout<<"layer_list.size()<=1"<<endl;
		return false;

	}else{
		int relu_false_cnt=0,relu_true_cnt=0;
		for(int iter_r=0;iter_r<layer_list.size();iter_r++){

			if(layer_list[iter_r]->conv_params->reluflag==true){
				relu_true_cnt++;
			}else{
				relu_false_cnt++;
			}
		}
		if(relu_true_cnt==0){

			if(relu_false_cnt!=layer_list.size())
				return false;
		}else{

			if(relu_true_cnt!=layer_list.size())
				return false;
		}

		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			op_fl=layer_list[iter_l]->op_fl;
			op_bw=layer_list[iter_l]->op_bw;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if(op_fl==layer_list[f_s]->op_fl){

					std::vector<string>::iterator it_str1=std::find(conv_out_fl.begin(),conv_out_fl.end(),layer_list[iter_l]->name);

					if(it_str1==conv_out_fl.end())
					{
						conv_out_fl.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_out_fl.begin(),conv_out_fl.end(),layer_list[f_s]->name);
					if(it_str==conv_out_fl.end())
					{
						conv_out_fl.push_back(layer_list[f_s]->name);

					}

				}
				if(op_bw==layer_list[f_s]->op_bw){

					std::vector<string>::iterator it_str=std::find(conv_out_bw.begin(),conv_out_bw.end(),layer_list[iter_l]->name);

					if(it_str==conv_out_bw.end())
					{
						conv_out_bw.push_back(layer_list[iter_l]->name);

					}
					it_str=std::find(conv_out_bw.begin(),conv_out_bw.end(),layer_list[f_s]->name);
					if(it_str==conv_out_bw.end())
					{
						conv_out_bw.push_back(layer_list[f_s]->name);

					}
				}
			}
		}

		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			ip_fl=layer_list[iter_l]->ip_fl;
			ip_bw=layer_list[iter_l]->ip_bw;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if(ip_fl==layer_list[f_s]->ip_fl){

					std::vector<string>::iterator it_str1=std::find(conv_in_fl.begin(),conv_in_fl.end(),layer_list[iter_l]->name);

					if(it_str1==conv_in_fl.end())
					{
						conv_in_fl.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_in_fl.begin(),conv_in_fl.end(),layer_list[f_s]->name);
					if(it_str==conv_in_fl.end())
					{
						conv_in_fl.push_back(layer_list[f_s]->name);

					}

				}
				if(ip_bw==layer_list[f_s]->ip_bw){

					std::vector<string>::iterator it_str=std::find(conv_in_bw.begin(),conv_in_bw.end(),layer_list[iter_l]->name);

					if(it_str==conv_in_bw.end())
					{
						conv_in_bw.push_back(layer_list[iter_l]->name);

					}
					it_str=std::find(conv_in_bw.begin(),conv_in_bw.end(),layer_list[f_s]->name);
					if(it_str==conv_in_bw.end())
					{
						conv_in_bw.push_back(layer_list[f_s]->name);

					}
				}
			}
		}

		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			wt_fl=layer_list[iter_l]->wt_fl;
			wt_bw=layer_list[iter_l]->wt_bw;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if(wt_fl==layer_list[f_s]->wt_fl){

					std::vector<string>::iterator it_str1=std::find(conv_wt_fl.begin(),conv_wt_fl.end(),layer_list[iter_l]->name);

					if(it_str1==conv_wt_fl.end())
					{
						conv_wt_fl.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_wt_fl.begin(),conv_wt_fl.end(),layer_list[f_s]->name);
					if(it_str==conv_wt_fl.end())
					{
						conv_wt_fl.push_back(layer_list[f_s]->name);

					}

				}
				if(wt_bw==layer_list[f_s]->wt_bw){

					std::vector<string>::iterator it_str=std::find(conv_wt_bw.begin(),conv_wt_bw.end(),layer_list[iter_l]->name);

					if(it_str==conv_wt_bw.end())
					{
						conv_wt_bw.push_back(layer_list[iter_l]->name);

					}
					it_str=std::find(conv_wt_bw.begin(),conv_wt_bw.end(),layer_list[f_s]->name);
					if(it_str==conv_wt_bw.end())
					{
						conv_wt_bw.push_back(layer_list[f_s]->name);

					}
				}
			}
		}

		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			pad_h=layer_list[iter_l]->conv_params->pad_h;
			pad_w=layer_list[iter_l]->conv_params->pad_w;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if((pad_h==layer_list[f_s]->conv_params->pad_h) && (pad_w==layer_list[f_s]->conv_params->pad_w)){

					std::vector<string>::iterator it_str1=std::find(conv_p_list.begin(),conv_p_list.end(),layer_list[iter_l]->name);

					if(it_str1==conv_p_list.end())
					{
						conv_p_list.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_p_list.begin(),conv_p_list.end(),layer_list[f_s]->name);
					if(it_str==conv_p_list.end())
					{
						conv_p_list.push_back(layer_list[f_s]->name);

					}
				}
			}
		}

		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			output_h=layer_list[iter_l]->topShape[0].at(2);
			output_w=layer_list[iter_l]->topShape[0].at(3);

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if((output_h==layer_list[f_s]->topShape[0].at(2)) && (output_w==layer_list[f_s]->topShape[0].at(3))){

					std::vector<string>::iterator it_str1=std::find(conv_o_list.begin(),conv_o_list.end(),layer_list[iter_l]->name);

					if(it_str1==conv_o_list.end())
					{
						conv_o_list.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_o_list.begin(),conv_o_list.end(),layer_list[f_s]->name);
					if(it_str==conv_o_list.end())
					{
						conv_o_list.push_back(layer_list[f_s]->name);

					}
				}
			}
		}

		for(int iter_l=0;iter_l<layer_list.size();iter_l++){

			stride_h=layer_list[iter_l]->conv_params->stride_h;
			stride_w=layer_list[iter_l]->conv_params->stride_w;

			for(int f_s=iter_l+1;f_s<layer_list.size();f_s++){


				if((stride_h==layer_list[f_s]->conv_params->stride_h) && (stride_w==layer_list[f_s]->conv_params->stride_w)){

					std::vector<string>::iterator it_str1=std::find(conv_s_list.begin(),conv_s_list.end(),layer_list[iter_l]->name);

					if(it_str1==conv_s_list.end())
					{
						conv_s_list.push_back(layer_list[iter_l]->name);

					}

					std::vector<string>::iterator it_str=std::find(conv_s_list.begin(),conv_s_list.end(),layer_list[f_s]->name);
					if(it_str==conv_s_list.end())
					{
						conv_s_list.push_back(layer_list[f_s]->name);

					}
				}
			}
		}

		if(!(layer_list.size()==conv_k_list.size())){
			return false;
		}
		else{

			if(!(layer_list.size()==conv_s_list.size())){
				return false;
			}else{

				if(!(layer_list.size()==conv_p_list.size()))
					return false;
				else{

					if(!(layer_list.size()==conv_o_list.size()))
						return false;
					else{

						if(!(layer_list.size()==conv_out_fl.size()))
							return false;
						else{
							if(!(layer_list.size()==conv_out_bw.size()))
								return false;
							else{
								if(!(layer_list.size()==conv_wt_bw.size()))
									return false;
								else{

									if(!(layer_list.size()==conv_wt_fl.size()))
										return false;
									else{
										if(!(layer_list.size()==conv_in_bw.size()))
											return false;
										else{

											if(!(layer_list.size()==conv_in_fl.size()))
												return false;
											else{

												vector<string> tmp_top_blob_name;
												vector<int> tmp_shape,tmp_custom;

												vector< vector<int> > tmp_custom_shape;

												for(int iter_ch=0;iter_ch<layer_list.size();iter_ch++){

													int tmp_channel=AlignSize(layer_list[iter_ch]->topShape[0].at(1),16);

													std::vector<string>::iterator it_str=std::find(tmp_top_blob_name.begin(),tmp_top_blob_name.end(),layer_list[iter_ch]->top[0].blob->name);

													if(it_str==tmp_top_blob_name.end())
													{
														tmp_top_blob_name.push_back(layer_list[iter_ch]->top[0].blob->name);

													}
													blob_channel.push_back(tmp_channel);
												}
												string layer_name = "conv_parallel_fuse";
												std::stringstream ss1;
												string tmp_batchnorm_name;
												ss1<<layer_name<<fuse_cnt;
												string fuse_layer_name = ss1.str();//"conv_lrn1";

												XLayer* dst0 = new XLayer(fuse_layer_name, "Convolution");

												xgraph_opt->layers[fuse_layer_name] = dst0;

												string username = fuse_layer_name;
												XBlob* tmp_blob = new XBlob(username);
												xgraph_opt->blobs[username] = tmp_blob;
												dst0->opcode=OPCODE_CONV;//OPCODE_PARALLEL_CONV_FUSE;
												if(relu_true_cnt)
													dst0->conv_params->reluflag=true;
												tmp_shape.push_back(blob_it->second->shape.at(0));
												int tmp_sum_c=0;
												int tmp_sum_w=0;
												int tmp_sum_h=0;
												tmp_sum_h=layer_list[0]->topShape[0].at(2);
												tmp_sum_w=layer_list[0]->topShape[0].at(3);

												for(int t_b_c=0;t_b_c<layer_list.size();t_b_c++){

													tmp_sum_c= tmp_sum_c+blob_channel[t_b_c];
													//tmp_sum_h= tmp_sum_h+layer_list[t_b_c]->topShape[0].at(2);
													//tmp_sum_w= tmp_sum_w+layer_list[t_b_c]->topShape[0].at(3);
												}

												tmp_shape.push_back(tmp_sum_c);
												tmp_shape.push_back(tmp_sum_h);
												tmp_shape.push_back(tmp_sum_w);
												vector<int> tmp_weightsDim;
												int n_out=0;
												int n_in=0;
												int n_fh=0;
												int n_fw=0;

												for(int iter_wd=0;iter_wd<layer_list.size();iter_wd++){

													n_out= n_out+AlignSize(layer_list[iter_wd]->conv_params->weightsDim[0].at(0),16);
													n_in = n_in+AlignSize(layer_list[iter_wd]->conv_params->weightsDim[0].at(1),16);
													n_fh = n_fh+layer_list[iter_wd]->conv_params->weightsDim[0].at(2);
													n_fw = n_fw+layer_list[iter_wd]->conv_params->weightsDim[0].at(3);

												}

												tmp_weightsDim.push_back(n_out);
												tmp_weightsDim.push_back(n_in);
												tmp_weightsDim.push_back(n_fh);
												tmp_weightsDim.push_back(n_fw);


												int cnt_weight=0;
												dst0->conv_params->weightsDim.push_back(tmp_weightsDim);
												vector<int> tmp_biasdim;
												tmp_biasdim.push_back(tmp_shape.at(1));
												dst0->conv_params->biasDim.push_back(tmp_biasdim);

												int o_n,i_n;
												float tmp_bias_val=0.0;
												vector<float> tmp_weights,tmp_bias;

												for(int iter_bias_c=0;iter_bias_c<layer_list.size();iter_bias_c++){

													for(int iter_s_b=0;iter_s_b<blob_channel[iter_bias_c];iter_s_b++){

														if(layer_list[iter_bias_c]->conv_params->bias[0].size()>iter_s_b)

															tmp_bias.push_back(layer_list[iter_bias_c]->conv_params->bias[0][iter_s_b]);
														else
															tmp_bias.push_back(tmp_bias_val);
													}

												}

												for(int t_b_c=0;t_b_c<layer_list.size();t_b_c++){

													o_n = layer_list[t_b_c]->conv_params->weightsDim[0].at(0);
													i_n = layer_list[t_b_c]->conv_params->weightsDim[0].at(1);

													n_out = AlignSize(layer_list[t_b_c]->conv_params->weightsDim[0].at(0),16);
													n_in = AlignSize(layer_list[t_b_c]->conv_params->weightsDim[0].at(1),16);

													for(int iter_n=0;iter_n<n_out;iter_n++){

														for(int iter_m=0;iter_m<n_in;iter_m++){

															int w_h=layer_list[t_b_c]->conv_params->weightsDim[0].at(2);

															for(int iter_fh=0;iter_fh<w_h;iter_fh++){

																int w_w=layer_list[t_b_c]->conv_params->weightsDim[0].at(3);

																for(int iter_fw=0;iter_fw<w_w;iter_fw++){

																	int indx = iter_n*i_n*w_h*w_w+iter_m*w_h*w_w+iter_fh*w_w+iter_fw;
																	cnt_weight=indx;

																	if(o_n>iter_n ){

																		if(i_n>iter_m){

																			tmp_weights.push_back(layer_list[t_b_c]->conv_params->weights[0][indx]);
																		}else{
																			float w_dum=0.0;
																			tmp_weights.push_back(w_dum);
																		}
																	}else{
																		float w_dum=0.0;
																		tmp_weights.push_back(w_dum);
																	}
																}
															}
														}
													}
												}

												dst0->conv_params->weights.push_back(tmp_weights);
												dst0->conv_params->bias.push_back(tmp_bias);
												//dst0->bottom=layer_list[0]->bottom;

												tmp_blob->shape=tmp_shape;

												tmp_blob->producerDim.push_back(tmp_blob->shape);
												if(tmp_top_blob_name.size()==1){
													tmp_blob->consumerDim.push_back(tmp_blob->shape);
												}
												else{

													if(layer_list.size()==tmp_top_blob_name.size()){
														for(int iter_b_s=0;iter_b_s<layer_list.size();iter_b_s++){
															tmp_blob->consumerDim.push_back(layer_list[iter_b_s]->top[0].blob->shape);
														}
													}else{
														cerr << "support need to added ,currenlty no support for this pattern" << endl;
														exit(-1);

													}
												}

												nameIndex new_blob(tmp_blob,0);
												nameIndex bottom_blob(blob_it->second,0);
												dst0->top.push_back(new_blob);
												dst0->bottom.push_back(bottom_blob);
												tmp_blob->producers.push_back(dst0->name);

												/*************************/
												string produce_name = blob_it->second->producers[0];
												map < string, XLayer*>::iterator it_pro_layer =xgraph_opt->layers.find(produce_name);

												dst0->conv_params->N = blob_it->second->shape.at(1);
												dst0->conv_params->M = tmp_blob->shape.at(1);
												dst0->conv_params->filter_h = layer_list[0]->conv_params->filter_h;
												dst0->conv_params->filter_w = layer_list[0]->conv_params->filter_w;
												dst0->conv_params->pad_h=layer_list[0]->conv_params->pad_h;
												dst0->conv_params->pad_w=layer_list[0]->conv_params->pad_w;
												dst0->conv_params->stride_h=layer_list[0]->conv_params->stride_h;
												dst0->conv_params->stride_w=layer_list[0]->conv_params->stride_w;
												dst0->conv_params->dilation=1;
												dst0->conv_params->group =1;

												dst0->op_bw = layer_list[0]->op_bw;
												dst0->op_fl = layer_list[0]->op_fl;
												// dst0->user_ip_bw = layer_list[0]->user_ip_bw;
												// dst0->user_ip_fl = layer_list[0]->user_ip_fl;
												dst0->ip_bw = layer_list[0]->ip_bw;
												dst0->ip_fl = layer_list[0]->ip_fl;
												dst0->wt_bw = layer_list[0]->wt_bw;
												dst0->wt_fl = layer_list[0]->wt_fl;

												dst0->output_file=fuse_layer_name;

												dst0->topShape.push_back(tmp_blob->shape);

												dst0->bottomShape.push_back(blob_it->second->shape);

												/**************************/

												for(int l_t_cus=0;l_t_cus<layer_list.size();l_t_cus++){

													for(int iter_t_cus=0;iter_t_cus<layer_list[l_t_cus]->top[0].blob->consumers.size();iter_t_cus++){

														std::vector<string>::iterator it_str=std::find(tmp_blob->consumers.begin(),tmp_blob->consumers.end(),layer_list[l_t_cus]->top[0].blob->consumers[iter_t_cus]);

														if(it_str==tmp_blob->consumers.end())
														{
															tmp_blob->consumers.push_back(layer_list[l_t_cus]->top[0].blob->consumers[iter_t_cus]);

														}

													}
												}
												std::vector<string>::iterator it_insert;
												int cnt_erase=0;
												for(int l_d=0;l_d<layer_list.size();l_d++){

													std::vector<string>::iterator it_str=std::find(blob_it->second->consumers.begin(),blob_it->second->consumers.end(),layer_list[l_d]->name);

													if(it_str!=blob_it->second->consumers.end())
													{
														if(cnt_erase==0){
															it_insert=it_str;
															cnt_erase++;
														}
														blob_it->second->consumers.erase(it_str);

													}
												}
												blob_it->second->consumers.insert(it_insert,dst0->name);

												/*for(int l_d=0;l_d<layer_list.size();l_d++){

													std::vector<string>::iterator it_str=std::find(blob_it->second->consumers.begin(),blob_it->second->consumers.end(),layer_list[l_d]->name);

													if(it_str!=blob_it->second->consumers.end())
													{
														blob_it->second->consumers.erase(it_str);

													}
												}*/
												for(int iter_l_p=0;iter_l_p<tmp_top_blob_name.size();iter_l_p++){

													map < string, XBlob* >::iterator top_blob_it = xgraph_opt->blobs.find(tmp_top_blob_name[iter_l_p]);

													for(int iter_tb_c=0;iter_tb_c<top_blob_it->second->consumers.size();iter_tb_c++){

														map < string, XLayer* >::iterator layer_it = xgraph_opt->layers.find(top_blob_it->second->consumers[iter_tb_c]);

														for(int iter_tb_cu_p=0;iter_tb_cu_p<layer_it->second->bottom.size();iter_tb_cu_p++){

															if(layer_it->second->bottom[iter_tb_cu_p].blob->name.compare(tmp_top_blob_name[iter_l_p])==0){

																layer_it->second->bottom[iter_tb_cu_p].blob =  new_blob.blob;
																layer_it->second->bottom[iter_tb_cu_p].id =  iter_l_p;
															}
														}
													}
												}
												for(int l_d=0;l_d<layer_list.size();l_d++){

													map < string, XBlob* >::iterator d_blob_it = xgraph_opt->blobs.find(layer_list[l_d]->top[0].blob->name);
													if(d_blob_it!=xgraph_opt->blobs.end())
														xgraph_opt->blobs.erase(d_blob_it);
													map < string, XLayer* >::iterator layer_it = xgraph_opt->layers.find(layer_list[l_d]->name);
													if(layer_it!=xgraph_opt->layers.end())
														xgraph_opt->layers.erase(layer_it);
												}
												return true;
											}
										}
									}
								}
							}
						}

					}
				}
			}
		}
	}
}
#endif
void XGraphOpt::opt_parallel_conv_fuse(XGraph* xgraph_opt){

	string *conv_layer_type = new string("Convolution");
	int fuse_cnt=0;
	for(map < string, XBlob*>::iterator it=xgraph_opt->blobs.begin();it!=xgraph_opt->blobs.end();it++){

		int conv_cnt=0;
		int match_pro_cnt=0;
		vector<string> no_match_pos;
		vector<string> layer_names;
		vector<XLayer*> vec_layer;
		int no_match=0;

		if(it->second->consumers.size()>1){

			for(int iter_b_c_c=0;iter_b_c_c<it->second->consumers.size();iter_b_c_c++){

				string tmp_name = it->second->consumers[iter_b_c_c];
				map < string, XLayer*>::iterator it_layer = xgraph_opt->layers.find(tmp_name);

				if(it_layer->second->type.compare(*conv_layer_type)==0){

					if(it_layer->second->opcode==OPCODE_CONV)
						layer_names.push_back(it->second->consumers[iter_b_c_c]);
				}
			}

			for(int iter_l_c=0;iter_l_c<layer_names.size();iter_l_c++){

				map < string, XLayer*>::iterator it_layer = xgraph_opt->layers.find(layer_names[iter_l_c]);

				int pro_size = it_layer->second->top[0].blob->producers.size();

				if(pro_size>1){

					for(int iter_t_p=0;iter_t_p<pro_size;iter_t_p++){

						std::vector<string>::iterator it_str1=std::find(layer_names.begin(),layer_names.end(),it_layer->second->top[0].blob->producers[iter_t_p]);

						if(it_str1==layer_names.end()){

							std::vector<string>::iterator it_str=std::find(no_match_pos.begin(),no_match_pos.end(),it_layer->second->top[0].blob->producers[iter_t_p]);

							if(it_str==no_match_pos.end())
							{
								no_match_pos.push_back(layer_names[iter_l_c]);
								no_match=1;
							}
						}
					}
				}
			}
			if(no_match==0){

				for(int iter_l_c=0;iter_l_c<layer_names.size();iter_l_c++){

					map < string, XLayer*>::iterator it_layer = xgraph_opt->layers.find(layer_names[iter_l_c]);
					vec_layer.push_back(it_layer->second);
					conv_cnt++;
				}
			}else{

				for(int iter_l_c=0;iter_l_c<layer_names.size();iter_l_c++){

					std::vector<string>::iterator it_str=std::find(no_match_pos.begin(),no_match_pos.end(),layer_names[iter_l_c]);

					if(it_str==no_match_pos.end())
					{
						map < string, XLayer*>::iterator it_layer = xgraph_opt->layers.find(layer_names[iter_l_c]);

						vec_layer.push_back(it_layer->second);

						conv_cnt++;
					}
				}

			}
			if(conv_cnt>1){

				bool flag_done=parallel_conv_fuse(xgraph_opt,it,vec_layer,fuse_cnt);
				if(flag_done)
					fuse_cnt++;

			}else{
				conv_cnt=0;
				continue;
			}
		}
	}
	delete conv_layer_type;
}
void find_max_q_factor_xlayer(vector < XlayerData > & xlayer_seq){


	for(int iter_xinx=0; iter_xinx<xlayer_seq.size();iter_xinx++){


		int Qfactor_consumer =  0;
		int Ffactor_consumer =  0;
		int Qfactor_producer =  0;
		int Ffactor_producer =  0;
		int Qfactor_max =  0;
		int Ffactor_max =  0;


		//if(it->second->consumers.size()!=0){

		if((xlayer_seq[iter_xinx].hw_ops->ip_bw == 0) && (xlayer_seq[iter_xinx].hw_ops->ip_fl==0)){

			for(int iter_pre_l=0;iter_pre_l<xlayer_seq[iter_xinx].prev_layers_ID.size();iter_pre_l++){

				int prev_layer_id1 = xlayer_seq[iter_xinx].prev_layers_ID[iter_pre_l].layerindex;

				int Qfactor_tmp =0;
				Qfactor_tmp = xlayer_seq[prev_layer_id1].hw_ops->op_bw;

				if(Qfactor_tmp>Qfactor_consumer){

					Ffactor_consumer = xlayer_seq[prev_layer_id1].hw_ops->op_fl;
					Qfactor_consumer=(Qfactor_tmp-Ffactor_consumer);

				}
			}
			Qfactor_max = Qfactor_consumer;
			Ffactor_max = Ffactor_consumer;
			xlayer_seq[iter_xinx].hw_ops->ip_bw=Qfactor_max+Ffactor_max;
			xlayer_seq[iter_xinx].hw_ops->ip_fl = Ffactor_max;

			for(int iter_pre_l=0;iter_pre_l<xlayer_seq[iter_xinx].prev_layers_ID.size();iter_pre_l++){

				int prev_layer_id1 = xlayer_seq[iter_xinx].prev_layers_ID[iter_pre_l].layerindex;

				xlayer_seq[prev_layer_id1].hw_ops->op_bw=Qfactor_max+Ffactor_max;
				xlayer_seq[prev_layer_id1].hw_ops->op_fl = Ffactor_max;

			}
		}


		if((xlayer_seq[iter_xinx].hw_ops->op_bw == 0) && (xlayer_seq[iter_xinx].hw_ops->op_fl==0)){

			for(int iter_next_l=0;iter_next_l<xlayer_seq[iter_xinx].next_layer_ID.size();iter_next_l++){

				int next_layer_id1 = xlayer_seq[iter_xinx].next_layer_ID[iter_next_l].layerindex;

				int Qfactor_tmp =0;
				Qfactor_tmp = xlayer_seq[next_layer_id1].hw_ops->ip_bw;

				if(Qfactor_tmp>Qfactor_producer){

					Ffactor_producer = xlayer_seq[next_layer_id1].hw_ops->ip_fl;
					Qfactor_producer=(Qfactor_tmp-Ffactor_producer);

				}
			}
			if(xlayer_seq[iter_xinx].next_layer_ID.size()!=0){

				Qfactor_max = Qfactor_producer;
				Ffactor_max = Ffactor_producer;
				xlayer_seq[iter_xinx].hw_ops->op_bw=Qfactor_max+Ffactor_max;
				xlayer_seq[iter_xinx].hw_ops->op_fl = Ffactor_max;

				for(int iter_next_l=0;iter_next_l<xlayer_seq[iter_xinx].next_layer_ID.size();iter_next_l++){

					int next_layer_id1 = xlayer_seq[iter_xinx].next_layer_ID[iter_next_l].layerindex;

					xlayer_seq[next_layer_id1].hw_ops->ip_bw=Qfactor_max+Ffactor_max;
					xlayer_seq[next_layer_id1].hw_ops->ip_fl = Ffactor_max;

				}
			}else{
				xlayer_seq[iter_xinx].hw_ops->op_bw=xlayer_seq[iter_xinx].hw_ops->ip_bw;
				xlayer_seq[iter_xinx].hw_ops->op_fl=xlayer_seq[iter_xinx].hw_ops->ip_fl;
			}
		}
	}
	//}
}

void XGraphOpt::opt_separable_conv_no_bias(XGraph *xgraph_opt,vector<string> &bn_opt_table){

	string *conv_layer_type = new string("Convolution");
	string *relu_layer_type = new string ("ReLU");
	int batchnorm_cnt=0;
	string relu_output_file;
	vector<int> tmp_val;
	string tmp_string = "NULL";
	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();it++){

		if(it->second->type.compare(*conv_layer_type)==0){
			int relu_layer_flage=0;
			if(it->second->conv_params->group>2){


				if(it->second->conv_params->M==it->second->conv_params->group){

					it->second->opcode=OPCODE_SEPARABLE_CONV;
					it->second->type="Pooling";


					if(it->second->conv_params->has_bias){

						map < string, XBlob*>::iterator top_3DS_iter = xgraph_opt->blobs.find(it->second->top[0].blob->name);

						if(top_3DS_iter->second->producers.size()>1){

							for(int iter_3d_p=0;iter_3d_p<top_3DS_iter->second->producers.size();iter_3d_p++){

								map < string, XLayer*>::iterator cons_layer_it =  xgraph_opt->layers.find(top_3DS_iter->second->producers[iter_3d_p]);

								if (cons_layer_it->second->type.compare(*relu_layer_type)==0){
									relu_layer_flage=1;
									xgrap_layer_delete(xgraph_opt,cons_layer_it->second->name);
									relu_output_file=cons_layer_it->second->output_file;
									break;
								}
							}
						}else{

							if(top_3DS_iter->second->producers.size()==1){

								for(int iter_3d_c=0;iter_3d_c<top_3DS_iter->second->consumers.size();iter_3d_c++){

									map < string, XLayer*>::iterator cons_layer_it =  xgraph_opt->layers.find(top_3DS_iter->second->consumers[iter_3d_c]);

									if (cons_layer_it->second->type.compare(*relu_layer_type)==0){
										relu_layer_flage=1;
										xgrap_layer_delete(xgraph_opt,cons_layer_it->second->name);
										relu_output_file=cons_layer_it->second->output_file;
										break;
									}
								}
							}
						}

						string layer_name = "conv_scale";
						std::stringstream ss1,outfile;
						string tmp_batchnorm_name;

						ss1<<layer_name<<batchnorm_cnt;

						string scale_layer_name = ss1.str();//"conv_lrn1";

						XLayer* dst0 = new XLayer(scale_layer_name, "Convolution");
						xgraph_opt->layers[scale_layer_name] = dst0;

						bn_opt_table.push_back(scale_layer_name);//]="Convolution";

						//string uniqname = xgraph_opt->getUniqueBlobName();
						string username = scale_layer_name;
						XBlob* tmp_blob = new XBlob(username);
						xgraph_opt->blobs[username] = tmp_blob;

						outfile<<scale_layer_name<<"_out.txt";

						tmp_blob->shape=it->second->top[0].blob->shape;

						tmp_blob->producerDim.push_back(tmp_blob->shape);
						tmp_blob->consumerDim.push_back(tmp_blob->shape);

						nameIndex new_blob(tmp_blob, 0);

						dst0->top.push_back(new_blob);

						// copy lrn params to first conv params

						dst0->conv_params->N = tmp_blob->shape.at(1);
						dst0->conv_params->M = tmp_blob->shape.at(1);
						dst0->conv_params->filter_h = 0;
						dst0->conv_params->filter_w = 0;
						dst0->conv_params->pad_h=0;
						dst0->conv_params->pad_w=0;
						dst0->conv_params->dilation=1;
						dst0->conv_params->group =1;
						dst0->conv_params->has_bias = 0;
						dst0->conv_params->extra_reluflag = false;
						tmp_val.push_back(0);
						dst0->conv_params->scale_gammaDim.push_back(tmp_val);
						dst0->conv_params->scale_gammaPath.push_back(tmp_string);
						dst0->conv_params->scale_betaDim = it->second->conv_params->biasDim;
						//dst0->conv_params->scale_betaPath = it->second->conv_params->biasPath;
						dst0->conv_params->scale_beta = it->second->conv_params->bias[0];
						//cout<<"Opcode 10"<< layer_it->second->scale_params->gammaPath[0]<<endl;
						//cout<<"Opcode 10"<< layer_it->second->scale_params->betaPath[0]<<endl;
						dst0->scale_beta_bw=0;
						dst0->scale_beta_fl=0;
						dst0->scale_gamma_bw= 0;
						dst0->scale_gamma_fl= 0;
						dst0->op_bw = it->second->op_bw;
						dst0->op_fl = it->second->op_fl;
						dst0->scale_gamma_by_std_bw = 0;
						dst0->scale_gamma_by_std_fl = 0;

						dst0->conv_params->batchnorm_meanDim.push_back(tmp_val);;

						dst0->conv_params->batchnorm_meanPath.push_back(tmp_string);
						dst0->conv_params->batchnorm_varianceDim.push_back(tmp_val);;
						dst0->conv_params->batchnorm_variancePath.push_back(tmp_string);
						dst0->conv_params->batchnorm_eps = 0;
						dst0->ip_bw = it->second->ip_bw;
						dst0->ip_fl = it->second->ip_fl;
						dst0->bn_mean_bw = 0;
						dst0->bn_mean_fl = 0;
						dst0->bn_variance_bw = 0;
						dst0->bn_variance_fl = 0;

						dst0->opcode = OPCODE_BN;

						dst0->bottom.push_back(top_3DS_iter->second);

						for(int iter_t_c=0;iter_t_c<top_3DS_iter->second->consumers.size();iter_t_c++){

							tmp_blob->consumers.push_back(top_3DS_iter->second->consumers[iter_t_c]);
						}

						top_3DS_iter->second->consumers[0]=dst0->name;

						tmp_blob->producers.push_back(dst0->name);

						if(relu_layer_flage){

							dst0->output_file=relu_output_file;

							relu_layer_flage=0;

						}else{
							//outfile<<tmp_blob<<batchnorm_cnt<<"_out.txt";
							dst0->output_file = outfile.str();//"conv_lrn1";


						}
						batchnorm_cnt++;
					}

				}else
				{
					cerr << "[EO001] Due to mismatch between layer group value and output dimension, Current version support layer group value should be <=2 or group == output dimension(3D separable convolution )" << endl;
					cerr<<"Current layer group = " <<it->second->conv_params->group <<" output dimension = "<<it->second->conv_params->M<<endl;									exit(-1);
				}
			}
		}
	}

	delete conv_layer_type;
	delete relu_layer_type;
}

void find_output_format_conversion_info(XGraph *xgraph_opt,map < string, XBlob*>::iterator &it_out_blob,kernelInfo& opt_descriptor,int &pack_fl,int &pack_format,string &quantization_scheme,int &custom_layer_cnt){

	if(it_out_blob->second->producers.size()==1){
		map<opcode_num_e, Opcode_info>::iterator it_opcode_tmp;
		int output_layer_flag=0;
		map < string, XLayer*>::iterator it_prod_layer= xgraph_opt->layers.find(it_out_blob->second->producers[0]);

		for(map<opcode_num_e, Opcode_info>::iterator it_opcode = opt_descriptor.opcode_info_map.begin();it_opcode!=opt_descriptor.opcode_info_map.end();it_opcode++){


			if(it_opcode->second.mega_module.module_name.compare(it_prod_layer->second->type)==0)
			{
				output_layer_flag=1;
				if((it_opcode->second.mega_module.module_name.compare("Convolution")==0) || (it_opcode->second.mega_module.module_name.compare("Pooling")==0) ){

					if(it_prod_layer->second->opcode==OPCODE_AVRPOOL2CONV){
						pack_fl=it_prod_layer->second->conv_params->op_fl_3d;
						quantization_scheme=it_prod_layer->second->quantization_scheme;
						it_opcode_tmp=it_opcode;
#if SINGLE_IO_PORT==0
						pack_format = 0;
#else
						pack_format = 5;

#endif
						break;
					}else{
						if(it_prod_layer->second->opcode==OPCODE_POOL2CONV){
							map < string, XLayer*>::iterator tmp_prods = xgraph_opt->layers.find(it_prod_layer->second->bottom[0].blob->producers[0]);//TODO Assumed its convolution
							pack_fl=tmp_prods->second->op_fl;
							quantization_scheme = tmp_prods->second->quantization_scheme;
#if SINGLE_IO_PORT==0
							pack_format = 0;
#else
							pack_format = 5;

#endif
							break;
						}
						else{
							if(it_prod_layer->second->opcode==OPCODE_POOL){
								map < string, XLayer*>::iterator it_p_c_layer= xgraph_opt->layers.find(it_prod_layer->second->bottom[0].blob->producers[0]);
								pack_fl=it_p_c_layer->second->op_fl;
								quantization_scheme=it_p_c_layer->second->quantization_scheme;
								it_opcode_tmp=it_opcode;
								pack_format=0;
								break;
							}else{
								pack_fl=it_prod_layer->second->op_fl;
								quantization_scheme=it_prod_layer->second->quantization_scheme;
								it_opcode_tmp=it_opcode;
#if SINGLE_IO_PORT==0
								pack_format = 0;
#else
								pack_format = 5;

#endif
								break;
							}

						}
					}
				}else if(it_opcode->second.mega_module.module_name.compare("Permute")==0) {
					map < string, XLayer*>::iterator it_p_c_layer= xgraph_opt->layers.find(it_prod_layer->second->bottom[0].blob->producers[0]);
					pack_fl=it_p_c_layer->second->op_fl;
					pack_format=3;
					quantization_scheme=it_p_c_layer->second->quantization_scheme;
					it_opcode_tmp=it_opcode;
					break;
				}

			}
		}
		if((output_layer_flag==0) && (pack_format==-1)){

			cout<<" CRITICAL WARNING: chAIDNN can't expect's "<<xgraph_opt->end_layer<<"output layer" <<endl;
			return;
		}
		if(pack_format!=-1){

			string layer_name;
			layer_name= "format_converter";

			std::stringstream ss1;

			ss1<<layer_name<<custom_layer_cnt;
			string packed_layer_name = ss1.str();

			XLayer* dst0= new XLayer(packed_layer_name, "XPack");

			int pack_fl=0;
			dst0->xpack_params->pack=pack_format;

			dst0->xpack_params->fbits = pack_fl;
			if(it_prod_layer->second->opcode==OPCODE_AVRPOOL2CONV || it_prod_layer->second->opcode==OPCODE_POOL2CONV){

				dst0->th_layer_in = it_prod_layer->second->conv_params->th_layer_out_3d;
				dst0->ip_bw = it_prod_layer->second->conv_params->op_bw_3d;
				dst0->op_bw = it_prod_layer->second->conv_params->op_bw_3d;
				dst0->ip_fl=it_prod_layer->second->conv_params->op_fl_3d;
				dst0->op_fl=it_prod_layer->second->conv_params->op_fl_3d;

			}else{
				if(it_prod_layer->second->opcode==OPCODE_POOL){
					map < string, XLayer*>::iterator it_p_c_layer= xgraph_opt->layers.find(it_prod_layer->second->bottom[0].blob->producers[0]); //TODO Assumed this is convloution layer
					dst0->th_layer_in = it_p_c_layer->second->th_layer_out;
					dst0->ip_bw = it_p_c_layer->second->op_bw;
					dst0->op_bw = it_p_c_layer->second->op_bw;
					dst0->ip_fl=it_p_c_layer->second->op_fl;
					dst0->op_fl=it_p_c_layer->second->op_fl;
				}else{
					dst0->th_layer_in = it_prod_layer->second->th_layer_out;
					dst0->ip_bw = it_prod_layer->second->op_bw;
					dst0->op_bw = it_prod_layer->second->op_bw;
					dst0->ip_fl=it_prod_layer->second->op_fl;
					dst0->op_fl=it_prod_layer->second->op_fl;
				}
			}

			xgraph_opt->layers[packed_layer_name] = dst0;
			dst0->opcode=OPCODE_XUNPACK;
			dst0->quantization_scheme = quantization_scheme; //TODO In feature we need to update this as layer based instead of xgraph

			string username = packed_layer_name;
			XBlob* tmp_blob = new XBlob(username);
			xgraph_opt->blobs[username] = tmp_blob;
			nameIndex new_blob(tmp_blob, 0);

			tmp_blob->shape=it_out_blob->second->shape;

			dst0->top.push_back(tmp_blob); //it means layer


			dst0->bottomShape.push_back(tmp_blob->shape);
			dst0->topShape.push_back(tmp_blob->shape);
			map<opcode_num_e, Opcode_info>::iterator it_pack_opcode;
			Port_info output_port,tmp_port;
			if((pack_format==0) || (pack_format==5))
				it_pack_opcode = opt_descriptor.opcode_info_map.find(OPCODE_XUNPACK);
			else if(pack_format==3)
				it_pack_opcode = opt_descriptor.opcode_info_map.find(OPCODE_PERMUTE_XUNPACK);

			for(int iter_pack_in=0;iter_pack_in<it_pack_opcode->second.port_vec.size();iter_pack_in++){

				if(it_pack_opcode->second.port_vec[iter_pack_in].port_enum==XI_OUTPUT){

					tmp_port=it_pack_opcode->second.port_vec[iter_pack_in];
				}
			}
			for(int iter_port=0;iter_port<it_pack_opcode->second.port_vec.size();iter_port++){

				if(it_pack_opcode->second.port_vec[iter_port].port_enum==XI_INPUT)
					output_port = it_pack_opcode->second.port_vec[iter_port];
			}
			dst0->xpack_params->in_port_vec.push_back(output_port);
			dst0->xpack_params->out_port_vec.push_back(tmp_port);

			it_prod_layer->second->top[0].blob->consumers.push_back(dst0->name);
			int in_fl=0,out_fl=0;
			dst0->bottom.push_back(it_prod_layer->second->top[0]);
			tmp_blob->producers.push_back(dst0->name);

			custom_layer_cnt++;

		}
	}

}

void find_in_format_conversion_info(XGraph *xgraph_opt,map < string, XBlob*>::iterator &it_in_blob,kernelInfo& opt_descriptor,int &pack_fl,int &pack_format,string &quantization_scheme,int &custom_layer_cnt){

	if(it_in_blob->second->consumers.size()==1){
		map<opcode_num_e, Opcode_info>::iterator it_opcode_tmp;
		map < string, XLayer*>::iterator it_conv_layer= xgraph_opt->layers.find(it_in_blob->second->consumers[0]);

		for(map<opcode_num_e, Opcode_info>::iterator it_opcode = opt_descriptor.opcode_info_map.begin();it_opcode!=opt_descriptor.opcode_info_map.end();it_opcode++){


			if(it_opcode->second.mega_module.module_name.compare(it_conv_layer->second->type)==0)
			{
				if((it_opcode->second.mega_module.module_name.compare("Convolution")==0) || (it_opcode->second.mega_module.module_name.compare("Pooling")==0)){

					if(it_conv_layer->second->opcode==OPCODE_AVRPOOL2CONV){
						pack_fl=it_conv_layer->second->conv_params->op_fl_3d;
						quantization_scheme=it_conv_layer->second->quantization_scheme;
#if SINGLE_IO_PORT==0
						pack_format = 1;
#else
						pack_format = 4;

#endif
						it_opcode_tmp=it_opcode;
						break;
					}
					else{
						if((it_conv_layer->second->opcode==OPCODE_POOL||it_conv_layer->second->opcode==OPCODE_POOL2CONV)&&(it_conv_layer->second->pool_params->PoolType==MAX)){
							if(it_conv_layer->second->quantization_scheme.compare("Xilinx")!=0){
								cout<<" CRITICAL WARNING: chAIDNN expect's fl bits for start layer" <<endl;
								return;
							}
						}else{
							pack_fl=it_conv_layer->second->ip_fl;
							quantization_scheme=it_conv_layer->second->quantization_scheme;
#if SINGLE_IO_PORT==0
							pack_format = 1;
#else
							pack_format = 4;

#endif
							it_opcode_tmp=it_opcode;
							break;
						}
					}


				}else if(it_opcode->second.mega_module.module_name.compare("Permute")==0) {

					pack_fl=it_conv_layer->second->op_fl;
					pack_format=1;
					it_opcode_tmp=it_opcode;
					quantization_scheme=it_conv_layer->second->quantization_scheme;
					break;
				}
			}
		}
		if(pack_format!=-1){
			//		map < string, XLayer*>::iterator cons_layer_it = xgraph_opt->layers.find(it_in_blob->second->consumers[0]);

			string tmp_cons_layer_name;
			string layer_name;
			layer_name= "format_converter";
			std::stringstream ss1;

			ss1<<layer_name<<custom_layer_cnt;
			string packed_layer_name = ss1.str();

			XLayer* dst0=new XLayer(packed_layer_name, "XPack");
			int pack_fl=0;
			tmp_cons_layer_name=it_conv_layer->second->name;

			dst0->xpack_params->pack=pack_format;

			dst0->xpack_params->fbits = pack_fl;
			xgraph_opt->layers[packed_layer_name] = dst0;
			dst0->opcode=OPCODE_XPACK;

			dst0->quantization_scheme = quantization_scheme; //TODO In feature we need to update this as layer based instead of xgraph


			XBlob* tmp_blob = new XBlob(packed_layer_name);
			xgraph_opt->blobs[packed_layer_name] = tmp_blob;
			nameIndex new_blob(tmp_blob, 0);

			tmp_blob->shape=it_conv_layer->second->topShape[0];

			dst0->top.push_back(tmp_blob); //it means layer
			//dst0->top[0].id = 0;

			dst0->bottomShape.push_back(tmp_blob->shape);
			dst0->topShape.push_back(tmp_blob->shape);

			//******** pack info **********/
			map<opcode_num_e, Opcode_info>::iterator it_pack_opcode;
			Port_info output_port,tmp_port;
			if(pack_format==1)
				it_pack_opcode = opt_descriptor.opcode_info_map.find(OPCODE_XPACK);

			for(int iter_pack_in=0;iter_pack_in<it_pack_opcode->second.port_vec.size();iter_pack_in++){

				if(it_pack_opcode->second.port_vec[iter_pack_in].port_enum==XI_OUTPUT){

					tmp_port=it_pack_opcode->second.port_vec[iter_pack_in];
				}
			}
			for(int iter_port=0;iter_port<it_opcode_tmp->second.port_vec.size();iter_port++){

				if(it_pack_opcode->second.port_vec[iter_port].port_enum==XI_INPUT)
					output_port = it_pack_opcode->second.port_vec[iter_port];
			}

			dst0->xpack_params->in_port_vec.push_back(output_port);
			dst0->xpack_params->out_port_vec.push_back(tmp_port);

			if(it_conv_layer->second->opcode==OPCODE_AVRPOOL2CONV||it_conv_layer->second->opcode==OPCODE_POOL2CONV){

				dst0->th_layer_in = it_conv_layer->second->conv_params->th_layer_in_3d;
				dst0->ip_bw = it_conv_layer->second->conv_params->ip_bw_3d;
				dst0->op_bw = it_conv_layer->second->conv_params->ip_bw_3d;
				dst0->ip_fl = it_conv_layer->second->conv_params->ip_fl_3d;
				dst0->op_fl = it_conv_layer->second->conv_params->ip_fl_3d;
			}else{
				if(it_conv_layer->second->opcode!=OPCODE_POOL){
					dst0->th_layer_in = it_conv_layer->second->th_layer_in;
					dst0->ip_bw = it_conv_layer->second->ip_bw;
					dst0->op_bw = it_conv_layer->second->ip_bw;
					dst0->ip_fl = it_conv_layer->second->ip_fl;
					dst0->op_fl = it_conv_layer->second->ip_fl;
				}else{
					if(it_conv_layer->second->opcode==OPCODE_POOL){
						map < string, XLayer*>::iterator it_layer_prod = xgraph_opt->layers.find(it_conv_layer->second->top[0].blob->consumers[0]); // TODO assumed this is convolution
						dst0->th_layer_in = it_layer_prod->second->th_layer_in;
						dst0->ip_bw = it_layer_prod->second->ip_bw;
						dst0->op_bw = it_layer_prod->second->ip_bw;
						dst0->ip_fl = it_layer_prod->second->ip_fl;
						dst0->op_fl = it_layer_prod->second->ip_fl;
					}
				}
			}

			it_conv_layer->second->bottom[0].blob->consumers[0]= dst0->name;

			dst0->bottom.push_back(it_conv_layer->second->bottom[0]);
			dst0->bottom[0].id=0;

			tmp_blob->consumers.push_back(it_conv_layer->second->name);
			tmp_blob->producers.push_back(dst0->name);

			it_conv_layer->second->bottom[0].blob=tmp_blob;
			it_conv_layer->second->bottom[0].id=0;


			custom_layer_cnt++;
		}
	}
}
void XGraphOpt::update_xgraph_pack_unpack_layer(XGraph* xgraph_opt,vector < BDegree >& Bdepth,kernelInfo& opt_descriptor,int &in_put_layer){

	int custom_layer_cnt=0;

	for(int iter_bdepth=0;iter_bdepth<Bdepth.size();iter_bdepth++){

		for(int iter_b_name=0;iter_b_name<Bdepth[iter_bdepth].blobs_name.size();iter_b_name++){

			map < string, XBlob*>::iterator it_blob = xgraph_opt->blobs.find(Bdepth[iter_bdepth].blobs_name[iter_b_name]);

			vector<Port_info_s> cons_port_format,prod_port_format;
			Port_info_s c_unique_port,p_unique_port;
			Port_info_s expected_port;
			packinfo prod_in_pack_info,prod_out_pack_info,cons_in_pack_info,cons_out_pack_info;

			int change_status=-1;

			/*find the concensus on consumer type*/

			find_input_port_type(xgraph_opt,opt_descriptor,it_blob,c_unique_port,cons_port_format,change_status); // TODO this implementation works one input per consumer layer

			if(change_status==-1)
				continue;

			change_status=-1;

			find_output_port_type(xgraph_opt,opt_descriptor,it_blob,p_unique_port,prod_port_format,change_status); // TODO this implementation works one output per producer layer

			if(change_status==-1)
				continue;
			///bool producers_same_type = check_producers();

			bool producers_same_type= check_producers_port(prod_port_format);

			///bool consumers_same_type = check_consumers();

			bool consumers_same_type = check_consumers_port(cons_port_format);

			///bool prod_con_same = check(prods, consumers);

			bool prod_con_same=check_cons_prod_port_same(p_unique_port,c_unique_port);


			if(prod_con_same == false)
			{

				if(producers_same_type == true && consumers_same_type == true)
				{
					/* insert one format conv below this blob */
					inser_format_conversion_layer_below_it(xgraph_opt,it_blob,opt_descriptor,p_unique_port,c_unique_port,custom_layer_cnt);
				}
				if(producers_same_type == false )
				{
					/* check and insert  format conv above this  blob for each producer  aim at final format same as pack_port deducded above*/
					//cerr << "[BEEO001] different produces format is not supported in the version " << endl;
					inser_format_conversion_layer_above_it(xgraph_opt,it_blob,opt_descriptor,p_unique_port,c_unique_port,custom_layer_cnt);
				}
				if(consumers_same_type == false )
				{
					/* insert on format conv below blob for each consumer, input format is pack_port and aim at the format that the consumer needs */

					inser_format_conversion_layer_below_it(xgraph_opt,it_blob,opt_descriptor,p_unique_port,c_unique_port,custom_layer_cnt);
				}
			}
		}
	}

	map < string, XBlob*>::iterator it_out_blob = xgraph_opt->blobs.find(xgraph_opt->output_blob);
	int out_pack_fl=-1,in_pack_fl=-1;
	int out_pack_format=-1,in_pack_format=-1;
	string out_quantization_scheme,in_quantization_scheme;
	if(in_put_layer==0){

		map < string, XBlob*>::iterator it_in_blob = xgraph_opt->blobs.find(xgraph_opt->input_blob);

		find_in_format_conversion_info(xgraph_opt,it_in_blob,opt_descriptor,in_pack_fl,in_pack_format,in_quantization_scheme,custom_layer_cnt);

	}
	if(xgraph_opt->model_last_layer==false)
	find_output_format_conversion_info(xgraph_opt,it_out_blob,opt_descriptor,out_pack_fl,out_pack_format,out_quantization_scheme,custom_layer_cnt);

}

void find_in_out_thresold(XGraph *xgraph){


	for(map < string, XBlob*>::iterator it=xgraph->blobs.begin();it!=xgraph->blobs.end();it++){

		vector<string> cons_layer_name,prod_layer_name;
		vector<float> cons_in_th;
		vector<float>  prod_out_th;
		string quant_scheme_name;
		//map < string, XBlob*>::iterator blob_it = xgraph->blobs.find(Bdepth[it_d].blobs_name[it_b]);

		for(int iter_b_c=0;iter_b_c<it->second->consumers.size();iter_b_c++){

			map < string, XLayer*>::iterator c_layer_it = xgraph->layers.find(it->second->consumers[iter_b_c]);
			if(c_layer_it->second->quantization_scheme.compare("Xilinx")==0){

				if((c_layer_it->second->opcode==OPCODE_AVRPOOL2CONV)||(c_layer_it->second->opcode==OPCODE_POOL2CONV)){
					cons_in_th.push_back(c_layer_it->second->conv_params->th_layer_in_3d);
				}else{
					cons_in_th.push_back(c_layer_it->second->th_layer_in);
				}
				cons_layer_name.push_back(it->second->consumers[iter_b_c]);
			}
		}
		for(int iter_b_p=0;iter_b_p<it->second->producers.size();iter_b_p++){

			map < string, XLayer*>::iterator p_layer_it = xgraph->layers.find(it->second->producers[iter_b_p]);
			quant_scheme_name = p_layer_it->second->quantization_scheme;
			if(p_layer_it->second->quantization_scheme.compare("Xilinx")==0){
				if((p_layer_it->second->opcode==OPCODE_AVRPOOL2CONV)||(p_layer_it->second->opcode==OPCODE_POOL2CONV)){

					prod_out_th.push_back(p_layer_it->second->conv_params->th_layer_out_3d);
				}else{
					prod_out_th.push_back(p_layer_it->second->th_layer_out);
				}
				prod_layer_name.push_back(it->second->producers[iter_b_p]);

			}
		}
		if(quant_scheme_name.compare("Xilinx")==0){
			for(int it_p_th=0;it_p_th<prod_out_th.size();it_p_th++){

				for(int it_c_th=0;it_c_th<cons_in_th.size();it_c_th++){

					if(prod_out_th[it_p_th]!=cons_in_th[it_c_th]){
						cout<<" CRITICAL WARNING: A drop in accuracy may be Observed" <<endl;
						cout<< "Following producer and consumer layes threshold values are not matching "<<endl;
						cout<<"Producer layer name:"<<prod_layer_name[it_p_th]<<endl;
						cout<<"Consumer layer name:"<<cons_layer_name[it_c_th]<<endl;

					}
				}
			}
		}
	}
}
/// This module is responsble for generate
void xlayer_sequence_generator(vector < XlayerData > & xlayer_seq,XGraph *xgraph,kernelInfo& opt_descriptor,int in_put_layer){

	XGraphOpt xgraphopt;
	map<string, int> layer_seq_table;
	map<string, string> layer_type_table;

	map<string, io_mem_data_type_e> mem_type_table;
	vector < BDegree > Bdepth_interm,Bdepth,reUseBdepth;
	vector<string> bn_opt_table;
	map<string,struct io_type> LayerIOType;

	xgraphopt.opt_data(xgraph);

	update_layer_io_type(LayerIOType,opt_descriptor);

	// checkGraphInsanity(xgraph);

	xgraphopt.opt_priorbox(xgraph);

#if O_DEBUG
	cout<<"xgraphopt.opt_priorbox(&graph) done" << endl;
#endif



#if O_DEBUG
	cout<<"xgraphopt.opt_relu(&graph) done" << endl;
#endif


	xgraphopt.opt_dropout(xgraph);
#if O_DEBUG
	cout<<"xgraphopt.opt_dropout(&graph) done" << endl;
#endif



	xgraphopt.opt_reshape(xgraph);
	xgraphopt.opt_flatten(xgraph);



#if O_DEBUG
	cout << " opt_concat optimization done" << endl;
#endif



	xgraphopt.opt_deconv(xgraph);
#if O_DEBUG
	cout << " opt_deconv optimization done" << endl;
#endif

	xgraphopt.opt_conv_bn_scale(xgraph);

	xgraphopt.opt_separable_conv(xgraph);

	//xgraph->drawGraph("/tmp/optimized_3d_graph.dot");

	xgraphopt.opt_separable_conv_only(xgraph);
	//xgraphopt.opt_separable_conv_no_bias(xgraph,bn_opt_table); only needed for separable convolution have bias

	xgraphopt.opt_crelu(xgraph);

	xgraphopt.opt_batchNorm(xgraph,bn_opt_table);


	//xgraphopt.opt_eltwise(xgraph);

	if(bn_opt_table.size()){
		xgraphopt.opt_fuse_bn_conv(xgraph,bn_opt_table);
	}

	//xgraphopt.opt_pool_conv_fuse(xgraph);

	xgraphopt.opt_pool2conv(xgraph);

	//#if FC_KERNEL_EXIST==0
	//	xgraphopt.opt_fc2conv(xgraph);
	//#endif

	xgraphopt.opt_eltwise(xgraph);

	xgraphopt.opt_relu(xgraph);

	xgraphopt.opt_lrn(xgraph); // TODO offline changes need to discussed with team and add changes

	xgraphopt.opt_concat(xgraph);
	xgraphopt.opt_parallel_conv_fuse(xgraph);  //TODO:Anitha for SSD

	//xgraphopt.custom_layer(xgraph); // after all optimization, custom layer support is added

	//find_max_q_factor(xgraph);
	//xgraph->drawGraph("/tmp/optimized_graph0.dot");


	update_xlayer_opcode(xgraph,opt_descriptor);
	find_total_output_plane(xgraph); // support is needed for extended batching
	find_blobs_degree(xgraph,Bdepth_interm);
	xgraphopt.update_xgraph_pack_unpack_layer(xgraph,Bdepth_interm,opt_descriptor,in_put_layer);

	find_blobs_degree(xgraph,Bdepth);

	//print_blobs_degre(Bdepth);
	find_in_out_thresold(xgraph);
#if O_DEBUG
	print_blobs_degre(Bdepth);

#endif

	create_seq_num_table(xlayer_seq,layer_seq_table,layer_type_table,Bdepth,xgraph);
	update_prev_next_layer_info(xlayer_seq,layer_seq_table,xgraph);
	update_io_mem_type(xlayer_seq,xgraph,Bdepth,layer_type_table,mem_type_table,LayerIOType,opt_descriptor);

#if O_DEBUG
	mem_type_print(mem_type_table);

#endif


	update_buffer_reuse_xlayer_seq(xlayer_seq,Bdepth,layer_seq_table,mem_type_table,xgraph,reUseBdepth);
	//	xgraph->drawGraph("/tmp/optimized_graph.dot");


}

void XGraphOpt::opt_data(XGraph* graph){

	string *input_layer_type = new string("Input");
	int input_out_flag=0;

	for(map < string, XLayer*>::iterator it = graph->layers.begin();it !=graph->layers.end();){

		if (it->second->type.compare(*input_layer_type)==0){
			input_out_flag=1;
			map < string, XBlob*>::iterator top_it = graph->blobs.find(it->second->top[0].blob->name);

			mapStrXLayer::iterator cur_it = it;
			it++;
			xgrap_layer_delete(graph,cur_it->second->name);

		}
		if(input_out_flag==0)
			it++;
		else{

			input_out_flag=0;
		}
	}
	delete input_layer_type;
}
void XGraphOpt::opt_priorbox(XGraph* graph)
{
	// Find NMS Layer
	mapStrXLayer_it nms_it = graph->layers.end();
	XLayer* nms_layer;
	for(mapStrXLayer_it it=graph->layers.begin(); it!=graph->layers.end(); it++)
	{
		if(it->second->type == "NMS")
		{
			if(nms_it == graph->layers.end())
			{
				nms_it = it;
				nms_layer = it->second;
			}
			else
			{
				cerr << "[EO001] Current version support only one NMS Layer in the network" << endl;
				exit(-1);
			}
		}
	}

	// If we found an NMS layer, then start optimizing it.
	// First setup list of layers and blobs to be deallocated
	vector<XLayer*> deallocateLayers;

	if(nms_it != graph->layers.end())
	{
		// Get Parent layer of NMS (It should be just 3 layers & 3rd layer should be priorbox concat layer)
		vector<string> nms_parent_layers = graph->getParentLayers(nms_layer->name);
		if(nms_parent_layers.size() != 3)
		{
			cerr << "[EO002] NMS Layer should have 3 child layers: conf, loc, priorBox" << endl;
			exit(-1);
		}

		mapStrXLayer_it priorbox_concat_layer_it = graph->layers.find(nms_parent_layers.at(2));
		if(priorbox_concat_layer_it->second->type != "Concat")
		{
			cerr << "[EO003] Third input to NMS is not coming from a Concat Layer" << endl;
			exit(-1);
		}
		XLayer* pb_concat_layer = priorbox_concat_layer_it->second;
		deallocateLayers.push_back(pb_concat_layer);

		// Vectors to store concatenated pbox and variances
		vector<float>& pboxVector = nms_layer->nms_params->pbox;
		vector<float>& varVector  = nms_layer->nms_params->var;

		// Get the parent layers of prior_box_concat layer.
		vector<string> pb_layers = graph->getParentLayers(nms_parent_layers.at(2));
		vector<int> shape(3, 1);        // shape = 1x1x1
		shape[2] = 0;                   // shape = 1x1x0; Initialize last dim for accumulation

		vector<string> blobsToBeDeleted;

		for(int i=0; i<pb_layers.size(); i++)
		{
			// #. Take each layer and check if it is a PriorBox Layer
			mapStrXLayer_it pb_layer_it = graph->layers.find(pb_layers[i]);
			XLayer *pb_layer = pb_layer_it->second;
			deallocateLayers.push_back(pb_layer);

			if(pb_layer->type != "PriorBox")
			{
				cerr << "[EO004] Input layers to PriorBox_Concat should be a PriorBox Layer : " << pb_layer->name << endl;
				exit(-1);
			}

			// #. Adjust the shapes
			vector<int> pboxShape = pb_layer->priorbox_params->pboxShape;
			int dim = pboxShape[2];
			shape[2] += dim;

			// #. Concatenate the each pboxes and vars
			pboxVector.insert(pboxVector.end(), pb_layer->priorbox_params->pbox.begin(), pb_layer->priorbox_params->pbox.begin() + dim);
			varVector.insert(varVector.end(), pb_layer->priorbox_params->pbox.begin()+dim, pb_layer->priorbox_params->pbox.end());

			// #. delete layer from the graph
			vector<string> blobsToDelete = graph->deleteLayerFromGraph(pb_layer->name);
			std::copy(blobsToDelete.begin(), blobsToDelete.end(), std::back_inserter(blobsToBeDeleted));
		}

		nms_layer->nms_params->pboxShape = shape;
		nms_layer->nms_params->varShape = shape;

#if DEBUG_WEIGHT_EXTRACTION
		string pboxFileName = graph->saveDir + nms_layer->name + "_weights";
		string varFileName = graph->saveDir + nms_layer->name + "_bias";
		nms_layer->nms_params->pboxFile = pboxFileName;
		nms_layer->nms_params->varFile = varFileName;
		int sizeInBytes1 = pboxVector.size() * sizeof(float);
		cerr << "[IG001] Saving " << pboxFileName << " (" << humanReadableSize(sizeInBytes1) << ")" << "\t";
		if(sizeInBytes1 > (14*1024*1024))
		{
			cerr << "Parsing large data, this may take a while ...";
		}
		cerr << endl;
		SAVEDATA(pboxVector, pboxFileName);

		int sizeInBytes2 = varVector.size() * sizeof(float);
		cerr << "[IG001] Saving " << varFileName << " (" << humanReadableSize(sizeInBytes2) << ")" << "\t";
		if(sizeInBytes2 > (14*1024*1024))
		{
			cerr << "Parsing large data, this may take a while ...";
		}
		cerr << endl;
		SAVEDATA(varVector, varFileName);
#endif

		// #. Now remove the concat layer
		vector<string> blobsToDelete = graph->deleteLayerFromGraph(pb_concat_layer->name);
		std::copy(blobsToDelete.begin(), blobsToDelete.end(), std::back_inserter(blobsToBeDeleted));

		// Finally remove all the obsolete blobs
		// XXX: Don't deallocate the layers before this, since blobs are referencing them inside
		for(int i=0; i<blobsToBeDeleted.size(); i++)
		{
			vector<string> dummy = graph->deleteBlobFromGraph(blobsToBeDeleted.at(i));
		}

		// TODO : ARK : This should be automated inside some function somehow.
		for(int i=0; i<deallocateLayers.size(); ++i)
		{
			delete deallocateLayers.at(i);
		}
	}
}

void XGraphOpt::opt_batchNorm(XGraph *xgraph_opt,vector<string> &bn_opt_table){

	string *batchNorm_layer_type = new string("BatchNorm");
	string *scale_layer_type = new string("Scale");
	string *relu_layer_type = new string("ReLU");
	vector<string> names_bt_norm;
	int batchnorm_cnt=0;
	vector<string> names_layers_del;

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();it++){

		if(it->second->type.compare(*batchNorm_layer_type)==0){

			string relu_output_file,scale_output_file;
			int relu_flag_enabled =0;
			if(it->second->batchnorm_params->global_stats==true){ //TODO currently resnet 50/34 we see global_stats always true... we may need to implement false case as well.

				if(it->second->batchnorm_params->inPlace){

					string blob_name = it->second->bottom[0].blob->name;

					map < string, XBlob* > ::iterator blob_it = xgraph_opt->blobs.find(blob_name);

					for(int iter_con=0;iter_con<blob_it->second->consumers.size();iter_con++){

						map < string, XLayer*>::iterator layer_it = xgraph_opt->layers.find(blob_it->second->consumers[iter_con]);

						if (layer_it->second->type.compare(*scale_layer_type)==0){

							if(layer_it->second->scale_params->inPlace){


								string layer_name = "conv_scale";
								std::stringstream ss1;
								string tmp_batchnorm_name;
								ss1<<layer_name<<batchnorm_cnt;
								string scale_layer_name = ss1.str();//"conv_lrn1";

								XLayer* dst0 = new XLayer(scale_layer_name, "Convolution");
								xgraph_opt->layers[scale_layer_name] = dst0;

								bn_opt_table.push_back(scale_layer_name);//]="Convolution";

								string username = scale_layer_name;
								XBlob* tmp_blob = new XBlob(username);
								xgraph_opt->blobs[username] = tmp_blob;

								tmp_blob->shape=it->second->top[0].blob->shape;

								tmp_blob->producerDim.push_back(tmp_blob->shape);
								tmp_blob->consumerDim.push_back(tmp_blob->shape);

								nameIndex new_blob(tmp_blob, 0);

								dst0->top.push_back(new_blob);
								dst0->topShape.push_back(new_blob.blob->shape);
								// copy lrn params to first conv params

								dst0->conv_params->N = tmp_blob->shape.at(1);
								dst0->conv_params->M = tmp_blob->shape.at(1);
								dst0->conv_params->filter_h = 0;
								dst0->conv_params->filter_w = 0;
								dst0->conv_params->pad_h=0;
								dst0->conv_params->pad_w=0;
								dst0->conv_params->dilation=1;
								dst0->conv_params->group =1;
								dst0->conv_params->has_bias = 0;
								dst0->conv_params->extra_reluflag = false;
								dst0->conv_params->scale_gammaDim = layer_it->second->scale_params->gammaDim;
								dst0->conv_params->scale_gamma = layer_it->second->scale_params->gamma[0];
								dst0->conv_params->scale_betaDim = layer_it->second->scale_params->betaDim;
								dst0->conv_params->scale_beta = layer_it->second->scale_params->beta[0];
								dst0->scale_beta_bw=layer_it->second->scale_beta_bw;
								dst0->scale_beta_fl=layer_it->second->scale_beta_fl;
								dst0->scale_gamma_bw= layer_it->second->scale_gamma_bw;
								dst0->scale_gamma_fl= layer_it->second->scale_gamma_fl;
								dst0->op_bw = layer_it->second->op_bw;
								dst0->op_fl = layer_it->second->op_fl;
								dst0->ip_fl = it->second->ip_fl;
								dst0->ip_bw = it->second->ip_bw;
								dst0->scale_gamma_by_std_bw = layer_it->second->scale_gamma_by_std_bw;
								dst0->scale_gamma_by_std_fl = layer_it->second->scale_gamma_by_std_fl;
								dst0->conv_params->th_layer_in_scale = layer_it->second->th_layer_in;
								dst0->conv_params->th_layer_out_scale = layer_it->second->th_layer_out;
								dst0->conv_params->th_params_beta_scale = layer_it->second->th_scale_beta;
								dst0->conv_params->th_params_gamma_scale = layer_it->second->th_scale_gamma;
								dst0->quantization_scheme = it->second->quantization_scheme;

								map < string, XLayer*>::iterator relu_layer_it;
								map < string, XLayer*>::iterator scale_layer_it;

								for(int iter_prod=0;iter_prod<blob_it->second->producers.size();iter_prod++){

									map < string, XLayer*>::iterator prod_layer_it = xgraph_opt->layers.find(blob_it->second->producers[iter_prod]);

									if(prod_layer_it->second->type.compare(*relu_layer_type)==0){

										if(prod_layer_it->second->relu_params->inPlace==true){

											relu_layer_it = prod_layer_it;
											relu_flag_enabled=1;
											relu_output_file=prod_layer_it->second->output_file;
											dst0->conv_params->extra_reluflag = true;
											blob_it->second->producers.erase(blob_it->second->producers.begin() + iter_prod);
											iter_prod=iter_prod-2;

											if(iter_prod==-2)
												iter_prod=-1;
										}
									}
									else if(prod_layer_it->second->type.compare(*scale_layer_type)==0){

										scale_layer_it = prod_layer_it;
										scale_output_file=prod_layer_it->second->output_file;
										blob_it->second->producers.erase(blob_it->second->producers.begin() + iter_prod);
										iter_prod=iter_prod-2;
										if(iter_prod==-2)
											iter_prod=-1;
									}

									else {

										if(prod_layer_it->second->type.compare(*batchNorm_layer_type)==0){

											tmp_batchnorm_name = prod_layer_it->second->name;
											blob_it->second->producers.erase(blob_it->second->producers.begin() + iter_prod);
											iter_prod=iter_prod-2;
											if(iter_prod==-2)
												iter_prod=-1;
										}
									}
								}

								map < string, XLayer*>::iterator bn_layer_it=xgraph_opt->layers.find(tmp_batchnorm_name);

								dst0->conv_params->batchnorm_meanDim = bn_layer_it->second->batchnorm_params->meanDim;
								dst0->conv_params->batchnorm_mean = bn_layer_it->second->batchnorm_params->mean[0];
								dst0->conv_params->batchnorm_varianceDim = bn_layer_it->second->batchnorm_params->varianceDim;
								dst0->conv_params->batchnorm_variance = bn_layer_it->second->batchnorm_params->variance[0];
								dst0->conv_params->batchnorm_eps = bn_layer_it->second->batchnorm_params->eps;
								dst0->ip_bw = bn_layer_it->second->ip_bw;
								dst0->ip_fl = bn_layer_it->second->ip_fl;
								dst0->bn_mean_bw = bn_layer_it->second->bn_mean_bw;
								dst0->bn_mean_fl = bn_layer_it->second->bn_mean_fl;
								dst0->bn_variance_bw = bn_layer_it->second->bn_variance_bw;
								dst0->bn_variance_fl = bn_layer_it->second->bn_variance_fl;

								dst0->conv_params->th_layer_in_batchnorm = bn_layer_it->second->th_layer_in;
								dst0->conv_params->th_layer_out_batchnorm = bn_layer_it->second->th_layer_out;
								dst0->conv_params->th_params_variance_batchnorm = bn_layer_it->second->th_bn_variance;
								dst0->conv_params->th_params_mean_batchnorm = bn_layer_it->second->th_bn_mean;

								for(int iter_con_c=0;iter_con_c<blob_it->second->consumers.size();iter_con_c++){

									map < string, XLayer*>::iterator cons_layer_it = xgraph_opt->layers.find(blob_it->second->consumers[iter_con_c]);

									if(cons_layer_it->second->type.compare(*relu_layer_type)==0){

										if(cons_layer_it->second->relu_params->inPlace==true){

											blob_it->second->consumers.erase(blob_it->second->consumers.begin() + iter_con_c);
											iter_con_c--;
										}
									}
									else if(cons_layer_it->second->type.compare(*batchNorm_layer_type)==0){

										blob_it->second->consumers.erase(blob_it->second->consumers.begin()+ iter_con_c);
										iter_con_c--;
									}
									else if(cons_layer_it->second->type.compare(*scale_layer_type)==0){
										blob_it->second->consumers[iter_con_c]=scale_layer_name;
									}

									else if(cons_layer_it->second->type.compare("Convolution")==0){

										blob_it->second->consumers.erase(blob_it->second->consumers.begin() + iter_con_c);

										tmp_blob->consumers.push_back(cons_layer_it->second->name);

										for(int iter_b_c=0;iter_b_c<cons_layer_it->second->bottom.size();iter_b_c++)
										{
											if(cons_layer_it->second->bottom[iter_b_c].blob->name.compare(blob_it->second->name)==0){

												cons_layer_it->second->bottom[iter_b_c].blob=tmp_blob;
											}
										}
									}
									else if(cons_layer_it->second->type.compare("Eltwise")==0){

										blob_it->second->consumers.erase(blob_it->second->consumers.begin() + iter_con_c);

										tmp_blob->consumers.push_back(cons_layer_it->second->name);

										for(int iter_b_c=0;iter_b_c<cons_layer_it->second->bottom.size();iter_b_c++)
										{
											if(cons_layer_it->second->bottom[iter_b_c].blob->name.compare(blob_it->second->name)==0){

												cons_layer_it->second->bottom[iter_b_c].blob=tmp_blob;
											}
										}
									}
									else
									{
										if(cons_layer_it->second->type.compare("Pooling")==0){

											blob_it->second->consumers.erase(blob_it->second->consumers.begin() + iter_con_c);

											tmp_blob->consumers.push_back(cons_layer_it->second->name);

											for(int iter_b_c=0;iter_b_c<cons_layer_it->second->bottom.size();iter_b_c++)
											{
												if(cons_layer_it->second->bottom[iter_b_c].blob->name.compare(blob_it->second->name)==0){

													cons_layer_it->second->bottom[iter_b_c].blob=tmp_blob;
												}
											}
										}
									}
								}

								dst0->opcode = OPCODE_BN;
								dst0->bottom.push_back(blob_it->second);
								dst0->bottomShape.push_back(blob_it->second->shape);
								tmp_blob->producers.push_back(dst0->name);

								if(relu_flag_enabled){

									dst0->output_file=relu_output_file;
									xgraph_opt->layers.erase(relu_layer_it);
									relu_flag_enabled=0;

								}else{

									dst0->output_file=scale_output_file;

								}
								xgraph_opt->layers.erase(scale_layer_it);
								xgraph_opt->layers.erase(it);
								batchnorm_cnt++;
							}

						}

					}
				}
				else{

					for(int iter_b_b=0;iter_b_b<it->second->top.size();iter_b_b++){

						map < string, XBlob*>::iterator it_top_blob = xgraph_opt->blobs.find(it->second->top[iter_b_b].blob->name);

						for(int iter_b_con=0;iter_b_con<it_top_blob->second->consumers.size();iter_b_con++){

							map < string, XLayer*>::iterator it_cons_layer = xgraph_opt->layers.find(it_top_blob->second->consumers[iter_b_con]);

							if (it_cons_layer->second->type.compare(*scale_layer_type)==0){

								if(!it_cons_layer->second->scale_params->inPlace){

									for(int iter_b_scale=0;iter_b_scale<it_cons_layer->second->top.size();iter_b_scale++){

										map < string, XBlob*>::iterator scale_top_blob = xgraph_opt->blobs.find(it_cons_layer->second->top[iter_b_scale].blob->name);

										for(int iter_s_b_con=0;iter_s_b_con<scale_top_blob->second->consumers.size();iter_s_b_con++){

											map < string, XLayer*>::iterator scale_top_cons_layer = xgraph_opt->layers.find(scale_top_blob->second->consumers[iter_s_b_con]);

											if (scale_top_cons_layer->second->type.compare(*relu_layer_type)==0){

												if(!scale_top_cons_layer->second->relu_params->inPlace){
													relu_flag_enabled=1;
													string layer_name = "conv_scale";
													std::stringstream ss1;
													string tmp_batchnorm_name;
													ss1<<layer_name<<batchnorm_cnt;
													string scale_layer_name = ss1.str();//"conv_lrn1";
													bn_opt_table.push_back(scale_layer_name);
													XLayer* dst0 = new XLayer(scale_layer_name, "Convolution");
													xgraph_opt->layers[scale_layer_name] = dst0;

													map < string, XBlob*>::iterator it_bottom_blob = xgraph_opt->blobs.find(it->second->bottom[0].blob->name); // TODO : assuming only one bottom blob is connected to BN layer
													dst0->top.push_back(scale_top_cons_layer->second->top[0]); // TODO : assuming only one top blob is connected to ReLU layer
													dst0->topShape.push_back(scale_top_cons_layer->second->topShape[0]);
													scale_top_cons_layer->second->top[0].blob->producers[0]=dst0->name;

													dst0->conv_params->N = it_bottom_blob->second->shape.at(1);
													dst0->conv_params->M = scale_top_cons_layer->second->top[0].blob->shape.at(1); // TODO : assuming only one top blob is connected to ReLU layer
													dst0->conv_params->filter_h = 0;
													dst0->conv_params->filter_w = 0;
													dst0->conv_params->pad_h=0;
													dst0->conv_params->pad_w=0;
													dst0->conv_params->dilation=1;
													dst0->conv_params->group =1;
													dst0->conv_params->has_bias = 0;
													dst0->conv_params->extra_reluflag = true;
													dst0->conv_params->scale_gammaDim = it_cons_layer->second->scale_params->gammaDim;
													dst0->conv_params->scale_gamma = it_cons_layer->second->scale_params->gamma[0];
													dst0->conv_params->scale_betaDim = it_cons_layer->second->scale_params->betaDim;
													dst0->conv_params->scale_beta = it_cons_layer->second->scale_params->beta[0];
													dst0->scale_beta_bw=it_cons_layer->second->scale_beta_bw;
													dst0->scale_beta_fl=it_cons_layer->second->scale_beta_fl;
													dst0->scale_gamma_bw= it_cons_layer->second->scale_gamma_bw;
													dst0->scale_gamma_fl= it_cons_layer->second->scale_gamma_fl;
													dst0->scale_gamma_by_std_bw = it_cons_layer->second->scale_gamma_by_std_bw;
													dst0->scale_gamma_by_std_fl = it_cons_layer->second->scale_gamma_by_std_fl;

													dst0->conv_params->th_layer_in_scale = it_cons_layer->second->th_layer_in;
													dst0->conv_params->th_layer_out_scale = it_cons_layer->second->th_layer_out;
													dst0->conv_params->th_params_beta_scale = it_cons_layer->second->th_scale_beta;
													dst0->conv_params->th_params_gamma_scale = it_cons_layer->second->th_scale_gamma;

													dst0->op_bw = it_cons_layer->second->op_bw;
													dst0->op_fl = it_cons_layer->second->op_fl;

													dst0->conv_params->batchnorm_meanDim = it->second->batchnorm_params->meanDim;
													dst0->conv_params->batchnorm_mean = it->second->batchnorm_params->mean[0];
													dst0->conv_params->batchnorm_varianceDim = it->second->batchnorm_params->varianceDim;
													dst0->conv_params->batchnorm_variance = it->second->batchnorm_params->variance[0];
													dst0->conv_params->batchnorm_eps = it->second->batchnorm_params->eps;
													dst0->ip_bw = it->second->ip_bw;
													dst0->ip_fl = it->second->ip_fl;
													dst0->bn_mean_bw = it->second->bn_mean_bw;
													dst0->bn_mean_fl = it->second->bn_mean_fl;
													dst0->bn_variance_bw = it->second->bn_variance_bw;
													dst0->bn_variance_fl = it->second->bn_variance_fl;

													dst0->conv_params->th_layer_in_batchnorm = it->second->th_layer_in;
													dst0->conv_params->th_layer_out_batchnorm = it->second->th_layer_out;
													dst0->conv_params->th_params_variance_batchnorm = it->second->th_bn_variance;
													dst0->conv_params->th_params_mean_batchnorm = it->second->th_bn_mean;

													dst0->output_file = scale_top_cons_layer->second->output_file;
													dst0->opcode = OPCODE_BN;
													dst0->bottom.push_back(it->second->bottom[0]);
													dst0->bottomShape.push_back(it->second->bottom[0].blob->shape);
													it->second->bottom[0].blob->consumers[0] = dst0->name;
													dst0->quantization_scheme = it->second->quantization_scheme;
													map < string, XBlob*>::iterator bn_top_blob_it = xgraph_opt->blobs.find(it->second->top[0].blob->name);
													xgraph_opt->blobs.erase(bn_top_blob_it);
													xgraph_opt->layers.erase(it);

													map < string, XBlob*>::iterator scale_top_blob_it = xgraph_opt->blobs.find(it_cons_layer->second->top[0].blob->name);
													xgraph_opt->blobs.erase(scale_top_blob_it);
													xgraph_opt->layers.erase(it_cons_layer);

													xgraph_opt->layers.erase(scale_top_cons_layer);
													batchnorm_cnt++;
												}
												else{

													string layer_name = "conv_scale";
													std::stringstream ss1;
													string tmp_batchnorm_name;
													ss1<<layer_name<<batchnorm_cnt;
													string scale_layer_name = ss1.str();//"conv_lrn1";


													bn_opt_table.push_back(scale_layer_name);
													XLayer* dst0 = new XLayer(scale_layer_name, "Convolution");
													xgraph_opt->layers[scale_layer_name] = dst0;

													map < string, XBlob*>::iterator it_bottom_blob = xgraph_opt->blobs.find(it->second->bottom[0].blob->name); // TODO : assuming only one bottom blob is connected to BN layer
													dst0->top.push_back(it_cons_layer->second->top[0]); // TODO : assuming only one top blob is connected to ReLU layer

													it_cons_layer->second->top[0].blob->producers[0]=dst0->name;

													dst0->conv_params->N = it_bottom_blob->second->shape.at(1);
													dst0->conv_params->M = it_cons_layer->second->top[0].blob->shape.at(1); // TODO : assuming only one top blob is connected to ReLU layer
													dst0->conv_params->filter_h = 0;
													dst0->conv_params->filter_w = 0;
													dst0->conv_params->pad_h=0;
													dst0->conv_params->pad_w=0;
													dst0->conv_params->dilation=1;
													dst0->conv_params->group =1;
													dst0->conv_params->has_bias = 0;
													dst0->conv_params->extra_reluflag = true;
													dst0->conv_params->scale_gammaDim = it_cons_layer->second->scale_params->gammaDim;
													dst0->conv_params->scale_gamma = it_cons_layer->second->scale_params->gamma[0];
													dst0->conv_params->scale_betaDim = it_cons_layer->second->scale_params->betaDim;
													dst0->conv_params->scale_beta = it_cons_layer->second->scale_params->beta[0];

													dst0->scale_beta_bw=it_cons_layer->second->scale_beta_bw;
													dst0->scale_beta_fl=it_cons_layer->second->scale_beta_fl;
													dst0->scale_gamma_bw= it_cons_layer->second->scale_gamma_bw;
													dst0->scale_gamma_fl= it_cons_layer->second->scale_gamma_fl;
													dst0->scale_gamma_by_std_bw = it_cons_layer->second->scale_gamma_by_std_bw;
													dst0->scale_gamma_by_std_fl = it_cons_layer->second->scale_gamma_by_std_fl;

													dst0->conv_params->th_layer_in_scale = it_cons_layer->second->th_layer_in;
													dst0->conv_params->th_layer_out_scale = it_cons_layer->second->th_layer_out;
													dst0->conv_params->th_params_beta_scale = it_cons_layer->second->th_scale_beta;
													dst0->conv_params->th_params_gamma_scale = it_cons_layer->second->th_scale_gamma;

													dst0->op_bw = it_cons_layer->second->op_bw;
													dst0->op_fl = it_cons_layer->second->op_fl;

													dst0->conv_params->batchnorm_meanDim = it->second->batchnorm_params->meanDim;
													dst0->conv_params->batchnorm_mean = it->second->batchnorm_params->mean[0];
													dst0->conv_params->batchnorm_varianceDim = it->second->batchnorm_params->varianceDim;
													dst0->conv_params->batchnorm_variance = it->second->batchnorm_params->variance[0];
													dst0->conv_params->batchnorm_eps = it->second->batchnorm_params->eps;

													dst0->conv_params->th_layer_in_batchnorm = it->second->th_layer_in;
													dst0->conv_params->th_layer_out_batchnorm = it->second->th_layer_out;
													dst0->conv_params->th_params_variance_batchnorm = it->second->th_bn_variance;
													dst0->conv_params->th_params_mean_batchnorm = it->second->th_bn_mean;

													dst0->ip_bw = it->second->ip_bw;
													dst0->ip_fl = it->second->ip_fl;
													dst0->bn_mean_bw = it->second->bn_mean_bw;
													dst0->bn_mean_fl = it->second->bn_mean_fl;
													dst0->bn_variance_bw = it->second->bn_variance_bw;
													dst0->bn_variance_fl = it->second->bn_variance_fl;
													dst0->output_file = it_cons_layer->second->output_file;
													dst0->opcode = OPCODE_BN;
													dst0->quantization_scheme = it->second->quantization_scheme;
													for(int iter_b_s=0;iter_b_s<it->second->bottom.size();iter_b_s++){

														dst0->bottom.push_back(it->second->bottom[iter_b_s]);
														dst0->bottomShape.push_back(it->second->bottom[iter_b_s].blob->shape);
														for(int iter_b_c=0;iter_b_c<it->second->bottom[iter_b_s].blob->consumers.size();iter_b_c++){

															if(it->second->bottom[iter_b_s].blob->consumers[iter_b_c].compare(it->second->name)==0){

																it->second->bottom[iter_b_s].blob->consumers[iter_b_c]=dst0->name;
															}
														}
													}



													map < string, XBlob*>::iterator bn_top_blob_it = xgraph_opt->blobs.find(it->second->top[0].blob->name);
													xgraph_opt->blobs.erase(bn_top_blob_it);
													xgraph_opt->layers.erase(it);

													map < string, XBlob*>::iterator scale_top_blob_it = xgraph_opt->blobs.find(it_cons_layer->second->top[0].blob->name);

													std::vector<string>::iterator it_str=std::find(scale_top_blob_it->second->producers.begin(),scale_top_blob_it->second->producers.end(),scale_top_cons_layer->second->name);
													if(it_str!=scale_top_blob_it->second->producers.end())
													{
														scale_top_blob_it->second->producers.erase(it_str);
													}
													it_str=std::find(scale_top_blob_it->second->consumers.begin(),scale_top_blob_it->second->consumers.end(),scale_top_cons_layer->second->name);
													if(it_str!=scale_top_blob_it->second->consumers.end())
													{
														scale_top_blob_it->second->consumers.erase(it_str);
													}

													xgraph_opt->layers.erase(it_cons_layer);
													xgraph_opt->layers.erase(scale_top_cons_layer);
													batchnorm_cnt++;
												}
											}else{

												string layer_name = "conv_scale";
												std::stringstream ss1;
												string tmp_batchnorm_name;
												ss1<<layer_name<<batchnorm_cnt;
												string scale_layer_name = ss1.str();//"conv_lrn1";


												bn_opt_table.push_back(scale_layer_name);
												XLayer* dst0 = new XLayer(scale_layer_name, "Convolution");
												xgraph_opt->layers[scale_layer_name] = dst0;

												map < string, XBlob*>::iterator it_bottom_blob = xgraph_opt->blobs.find(it->second->bottom[0].blob->name); // TODO : assuming only one bottom blob is connected to BN layer
												dst0->top.push_back(it_cons_layer->second->top[0]); // TODO : assuming only one top blob is connected to ReLU layer

												it_cons_layer->second->top[0].blob->producers[0]=dst0->name;

												dst0->conv_params->N = it_bottom_blob->second->shape.at(1);
												dst0->conv_params->M = it_cons_layer->second->top[0].blob->shape.at(1); // TODO : assuming only one top blob is connected to ReLU layer
												dst0->conv_params->filter_h = 0;
												dst0->conv_params->filter_w = 0;
												dst0->conv_params->pad_h=0;
												dst0->conv_params->pad_w=0;
												dst0->conv_params->dilation=1;
												dst0->conv_params->group =1;
												dst0->conv_params->has_bias = 0;
												dst0->conv_params->extra_reluflag = true;
												dst0->conv_params->scale_gammaDim = it_cons_layer->second->scale_params->gammaDim;
												dst0->conv_params->scale_gamma = it_cons_layer->second->scale_params->gamma[0];
												dst0->conv_params->scale_betaDim = it_cons_layer->second->scale_params->betaDim;
												dst0->conv_params->scale_beta = it_cons_layer->second->scale_params->beta[0];

												dst0->conv_params->th_layer_in_scale = it_cons_layer->second->th_layer_in;
												dst0->conv_params->th_layer_out_scale = it_cons_layer->second->th_layer_out;
												dst0->conv_params->th_params_beta_scale = it_cons_layer->second->th_scale_beta;
												dst0->conv_params->th_params_gamma_scale = it_cons_layer->second->th_scale_gamma;

												dst0->scale_beta_bw=it_cons_layer->second->scale_beta_bw;
												dst0->scale_beta_fl=it_cons_layer->second->scale_beta_fl;
												dst0->scale_gamma_bw= it_cons_layer->second->scale_gamma_bw;
												dst0->scale_gamma_fl= it_cons_layer->second->scale_gamma_fl;
												dst0->scale_gamma_by_std_bw = it_cons_layer->second->scale_gamma_by_std_bw;
												dst0->scale_gamma_by_std_fl = it_cons_layer->second->scale_gamma_by_std_fl;

												dst0->op_bw = it_cons_layer->second->op_bw;
												dst0->op_fl = it_cons_layer->second->op_fl;

												dst0->conv_params->batchnorm_meanDim = it->second->batchnorm_params->meanDim;
												dst0->conv_params->batchnorm_mean = it->second->batchnorm_params->mean[0];
												dst0->conv_params->batchnorm_varianceDim = it->second->batchnorm_params->varianceDim;
												dst0->conv_params->batchnorm_variance = it->second->batchnorm_params->variance[0];
												dst0->conv_params->batchnorm_eps = it->second->batchnorm_params->eps;

												dst0->conv_params->th_layer_in_batchnorm = it->second->th_layer_in;
												dst0->conv_params->th_layer_out_batchnorm = it->second->th_layer_out;
												dst0->conv_params->th_params_variance_batchnorm = it->second->th_bn_variance;
												dst0->conv_params->th_params_mean_batchnorm = it->second->th_bn_mean;

												dst0->ip_bw = it->second->ip_bw;
												dst0->ip_fl = it->second->ip_fl;
												dst0->bn_mean_bw = it->second->bn_mean_bw;
												dst0->bn_mean_fl = it->second->bn_mean_fl;
												dst0->bn_variance_bw = it->second->bn_variance_bw;
												dst0->bn_variance_fl = it->second->bn_variance_fl;
												dst0->output_file = it_cons_layer->second->output_file;
												dst0->opcode = OPCODE_BN;
												dst0->quantization_scheme = it->second->quantization_scheme;
												for(int iter_b_s=0;iter_b_s<it->second->bottom.size();iter_b_s++){

													dst0->bottom.push_back(it->second->bottom[iter_b_s]);
													dst0->bottomShape.push_back(it->second->bottom[iter_b_s].blob->shape);
													for(int iter_b_c=0;iter_b_c<it->second->bottom[iter_b_s].blob->consumers.size();iter_b_c++){

														if(it->second->bottom[iter_b_s].blob->consumers[iter_b_c].compare(it->second->name)==0){

															it->second->bottom[iter_b_s].blob->consumers[iter_b_c]=dst0->name;
														}
													}
												}

												map < string, XBlob*>::iterator bn_top_blob_it = xgraph_opt->blobs.find(it->second->top[0].blob->name);
												xgraph_opt->blobs.erase(bn_top_blob_it);
												xgraph_opt->layers.erase(it);
												xgraph_opt->layers.erase(it_cons_layer);

												batchnorm_cnt++;
											}
										}
									}
								}
							}
						}
					}
				}

			}
		}
	}
	delete batchNorm_layer_type;
	delete scale_layer_type;
	delete relu_layer_type;
}


void XGraphOpt::opt_scale(XGraph *xgraph_opt){

	string *scale_layer_type = new string("Scale");
	string *relu_layer_type = new string("ReLU");

	string *batchNorm_layer_type = new string("BatchNorm");

	vector<string> names_layers_del;

	int scale_cnt=0;

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();it++){

		if (it->second->type.compare(*scale_layer_type)==0){

			if(it->second->scale_params->inPlace){


				string layer_name = "conv_scale";
				std::stringstream ss1;
				string tmp_batchnorm_name;
				ss1<<layer_name<<scale_cnt;
				string scale_layer_name = ss1.str();//"conv_lrn1";

				XLayer* dst0 = new XLayer(scale_layer_name, "Convolution");
				xgraph_opt->layers[scale_layer_name] = dst0;

				//string uniqname = xgraph_opt->getUniqueBlobName();
				string username = scale_layer_name;
				XBlob* tmp_blob = new XBlob(username);
				xgraph_opt->blobs[username] = tmp_blob;

				tmp_blob->shape=it->second->top[0].blob->shape;

				tmp_blob->producerDim.push_back(tmp_blob->shape);
				tmp_blob->consumerDim.push_back(tmp_blob->shape);

				nameIndex new_blob(tmp_blob, 0);

				dst0->top.push_back(new_blob);

				// copy lrn params to first conv params

				dst0->conv_params->N = tmp_blob->shape.at(1);
				dst0->conv_params->M = tmp_blob->shape.at(1);
				dst0->conv_params->filter_h = 0;
				dst0->conv_params->filter_w = 0;
				dst0->conv_params->pad_h=0;
				dst0->conv_params->pad_w=0;
				dst0->conv_params->dilation=1;
				dst0->conv_params->group =1;
				dst0->conv_params->has_bias = 0;
				dst0->conv_params->reluflag = false;
				dst0->conv_params->scale_gammaDim = it->second->scale_params->gammaDim;
				//dst0->conv_params->scale_gammaPath = it->second->scale_params->gammaPath;
				dst0->conv_params->scale_gamma = it->second->scale_params->gamma[0];
				dst0->conv_params->scale_betaDim = it->second->scale_params->betaDim;
				//dst0->conv_params->scale_betaPath = it->second->scale_params->betaPath;
				dst0->conv_params->scale_beta = it->second->scale_params->beta[0];


				map < string, XBlob*>::iterator blob_it = xgraph_opt->blobs.find(it->second->top[0].blob->name);

				for(int iter_prod=0;iter_prod<blob_it->second->producers.size();iter_prod++){

					map < string, XLayer*>::iterator layer_it = xgraph_opt->layers.find(blob_it->second->producers[iter_prod]);

					if(layer_it->second->type.compare(*relu_layer_type)==0){

						if(layer_it->second->relu_params->inPlace==true){

							names_layers_del.push_back(layer_it->second->name);

							dst0->conv_params->reluflag = true;

							blob_it->second->producers.erase(blob_it->second->producers.begin() + iter_prod);
							iter_prod--;
						}
					}
					if(layer_it->second->type.compare(*scale_layer_type)==0){

						blob_it->second->producers.erase(blob_it->second->producers.begin() + iter_prod);
						iter_prod--;
					}

					if(layer_it->second->type.compare(*batchNorm_layer_type)==0){
						tmp_batchnorm_name = layer_it->second->name;
						names_layers_del.push_back(layer_it->second->name);
						blob_it->second->producers.erase(blob_it->second->producers.begin() + iter_prod);
						iter_prod--;
					}
				}
				for(int iter_con=0;iter_con<blob_it->second->consumers.size();iter_con++){

					map < string, XLayer*>::iterator layer_it = xgraph_opt->layers.find(blob_it->second->consumers[iter_con]);

					if(layer_it->second->type.compare(*relu_layer_type)==0){

						if(layer_it->second->relu_params->inPlace==true){

							blob_it->second->consumers.erase(blob_it->second->consumers.begin() + iter_con);
							iter_con--;
						}
					}
					if(layer_it->second->type.compare(*batchNorm_layer_type)==0){

						blob_it->second->consumers.erase(blob_it->second->consumers.begin()+ iter_con);
						iter_con--;
					}
					if(layer_it->second->type.compare(*scale_layer_type)==0){

						blob_it->second->consumers[iter_con]=scale_layer_name;
						//iter_con--;
					}

					if(layer_it->second->type.compare("Convolution")==0){

						blob_it->second->consumers.erase(blob_it->second->consumers.begin() + iter_con);
						tmp_blob->consumers.push_back(layer_it->second->name);
						for(int iter_b_c=0;iter_b_c<layer_it->second->bottom.size();iter_b_c++)
						{
							if(layer_it->second->bottom[iter_b_c].blob->name.compare(blob_it->second->name)==0){

								layer_it->second->bottom[iter_b_c].blob=tmp_blob;
							}
						}
					}
					else if(layer_it->second->type.compare("Eltwise")==0){

						blob_it->second->consumers.erase(blob_it->second->consumers.begin() + iter_con);

						tmp_blob->consumers.push_back(layer_it->second->name);

						for(int iter_b_c=0;iter_b_c<layer_it->second->bottom.size();iter_b_c++)
						{
							if(layer_it->second->bottom[iter_b_c].blob->name.compare(blob_it->second->name)==0){

								layer_it->second->bottom[iter_b_c].blob=tmp_blob;
							}
						}
					}
					else if(layer_it->second->type.compare("Pooling")==0){

						blob_it->second->consumers.erase(blob_it->second->consumers.begin() + iter_con);
						tmp_blob->consumers.push_back(layer_it->second->name);
						for(int iter_b_c=0;iter_b_c<layer_it->second->bottom.size();iter_b_c++)
						{
							if(layer_it->second->bottom[iter_b_c].blob->name.compare(blob_it->second->name)==0){

								layer_it->second->bottom[iter_b_c].blob=tmp_blob;
							}
						}
					}
				}
				map < string, XLayer*>::iterator bn_layer_it=xgraph_opt->layers.find(tmp_batchnorm_name);

				dst0->conv_params->batchnorm_meanDim = bn_layer_it->second->batchnorm_params->meanDim;
				//dst0->conv_params->batchnorm_meanPath = bn_layer_it->second->batchnorm_params->meanPath;
				dst0->conv_params->batchnorm_mean = bn_layer_it->second->batchnorm_params->mean[0];
				dst0->conv_params->batchnorm_varianceDim = bn_layer_it->second->batchnorm_params->varianceDim;
				//dst0->conv_params->batchnorm_variancePath = bn_layer_it->second->batchnorm_params->variancePath;
				dst0->conv_params->batchnorm_variance = bn_layer_it->second->batchnorm_params->variance[0];
				dst0->conv_params->batchnorm_eps = bn_layer_it->second->batchnorm_params->eps;

				dst0->opcode = OPCODE_BN;
				dst0->bottom.push_back(blob_it->second);
				tmp_blob->producers.push_back(dst0->name);
				xgraph_opt->layers.erase(it);
				scale_cnt++;
			}
		}
	}

	for(int iter_del=0;iter_del<names_layers_del.size();iter_del++){

		map < string, XLayer*>::iterator layer_it = xgraph_opt->layers.find(names_layers_del[iter_del]);
		xgraph_opt->layers.erase(layer_it);
	}
	delete scale_layer_type;
	delete relu_layer_type;
	delete batchNorm_layer_type;
}


/*void XGraphOpt::opt_pool2conv(XGraph *xgraph_opt){

	string *pool_layer_type = new string("Pooling");

	int pool_cnt=0;

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();){

		if (it->second->type.compare(*pool_layer_type)==0 && (it->second->pool_params->PoolType==AVE)){

#if POOL_KERNEL_EXIST

			string layer_name = "conv_pool";
			std::stringstream ss1;

			ss1<<layer_name<<pool_cnt;
			string pool_layer_name = ss1.str();//"conv_lrn1";

			XLayer* dst0 = new XLayer(pool_layer_name, "Convolution");

			xgraph_opt->layers[pool_layer_name] = dst0;

			map < string, XBlob*>::iterator top_blob_it;
			map < string, XBlob*>::iterator bottom_blob_it;

			for(int iter_top=0;iter_top<it->second->top.size();iter_top++){

				dst0->top.push_back(it->second->top[iter_top]);
				dst0->topShape.push_back(it->second->topShape[iter_top]);
				string top_blob_name = it->second->top[iter_top].blob->name;

				top_blob_it = xgraph_opt->blobs.find(top_blob_name);

				for(int iter_t_p=0;iter_t_p<top_blob_it->second->producers.size();iter_t_p++){

					if(top_blob_it->second->producers[iter_t_p].compare(it->second->name)==0){
						top_blob_it->second->producers[iter_t_p] = pool_layer_name;
					}
				}
			}

			for(int iter_bot=0;iter_bot<it->second->bottom.size();iter_bot++){

				dst0->bottom.push_back(it->second->bottom[iter_bot]);
				dst0->bottomShape.push_back(it->second->bottomShape[iter_bot]);

				string bottom_blob_name = it->second->bottom[iter_bot].blob->name;

				bottom_blob_it = xgraph_opt->blobs.find(bottom_blob_name);

				for(int iter_b_c=0;iter_b_c<bottom_blob_it->second->consumers.size();iter_b_c++){

					if(bottom_blob_it->second->consumers[iter_b_c].compare(it->second->name)==0){
						bottom_blob_it->second->consumers[iter_b_c] = pool_layer_name;
					}
				}

			}

			dst0->intermediateShape.push_back(it->second->topShape[0]);
			dst0->conv_params->M_3d = it->second->pool_params->N;
			dst0->conv_params->N_3d = it->second->pool_params->N;
			dst0->conv_params->filter_h_3d = it->second->pool_params->kernel_h;
			dst0->conv_params->filter_w_3d = it->second->pool_params->kernel_w;

			dst0->conv_params->stride_h_3d = it->second->pool_params->stride_h;
			dst0->conv_params->stride_w_3d = it->second->pool_params->stride_w;
			dst0->conv_params->pad_h_3d = it->second->pool_params->pad_h;
			dst0->conv_params->pad_w_3d = it->second->pool_params->pad_w;

			dst0->conv_params->ip_bw_3d = it->second->ip_bw;
			dst0->conv_params->ip_fl_3d = it->second->ip_fl;
			dst0->conv_params->op_bw_3d = it->second->op_bw;
			dst0->conv_params->op_fl_3d = it->second->op_fl;

			dst0->conv_params->th_layer_in_3d = it->second->th_layer_in;
			dst0->conv_params->th_layer_out_3d = it->second->th_layer_out;
			dst0->conv_params->th_params_3d = it->second->th_params;
			dst0->quantization_scheme = it->second->quantization_scheme;
			dst0->output_file=it->second->output_file;
			//			if(it->second->pool_params->PoolType==MAX)
			//				dst0->opcode = OPCODE_POOL2CONV;
			//			else
			dst0->opcode = OPCODE_AVRPOOL2CONV;

			dst0->conv_params->reluflag=false;

			mapStrXLayer::iterator cur_it = it;
			it++;
			xgraph_opt->layers.erase(cur_it);
			pool_cnt++;

#else
			it++;
#endif
		}else{
			it++;
		}
	}
	delete pool_layer_type;
}*/
void XGraphOpt::opt_pool2conv(XGraph *xgraph_opt){

	string *pool_layer_type = new string("Pooling");

	int pool_cnt=0;

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();){

		if (it->second->type.compare(*pool_layer_type)==0){

			if(it->second->pool_params->PoolType==AVE){

				string layer_name = "conv_pool";
				std::stringstream ss1;

				ss1<<layer_name<<pool_cnt;
				string pool_layer_name = ss1.str();//"conv_lrn1";

				XLayer* dst0 = new XLayer(pool_layer_name, "Convolution");

				xgraph_opt->layers[pool_layer_name] = dst0;

				map < string, XBlob*>::iterator top_blob_it;
				map < string, XBlob*>::iterator bottom_blob_it;

				for(int iter_top=0;iter_top<it->second->top.size();iter_top++){

					dst0->top.push_back(it->second->top[iter_top]);
					dst0->topShape.push_back(it->second->topShape[iter_top]);
					string top_blob_name = it->second->top[iter_top].blob->name;

					top_blob_it = xgraph_opt->blobs.find(top_blob_name);

					for(int iter_t_p=0;iter_t_p<top_blob_it->second->producers.size();iter_t_p++){

						if(top_blob_it->second->producers[iter_t_p].compare(it->second->name)==0){
							top_blob_it->second->producers[iter_t_p] = pool_layer_name;
						}
					}
				}

				for(int iter_bot=0;iter_bot<it->second->bottom.size();iter_bot++){

					dst0->bottom.push_back(it->second->bottom[iter_bot]);
					dst0->bottomShape.push_back(it->second->bottomShape[iter_bot]);

					string bottom_blob_name = it->second->bottom[iter_bot].blob->name;

					bottom_blob_it = xgraph_opt->blobs.find(bottom_blob_name);

					for(int iter_b_c=0;iter_b_c<bottom_blob_it->second->consumers.size();iter_b_c++){

						if(bottom_blob_it->second->consumers[iter_b_c].compare(it->second->name)==0){
							bottom_blob_it->second->consumers[iter_b_c] = pool_layer_name;
						}
					}

				}

				dst0->intermediateShape.push_back(it->second->topShape[0]);
				dst0->conv_params->M_3d = it->second->pool_params->N;
				dst0->conv_params->N_3d = it->second->pool_params->N;
				dst0->conv_params->filter_h_3d = it->second->pool_params->kernel_h;
				dst0->conv_params->filter_w_3d = it->second->pool_params->kernel_w;

				dst0->conv_params->stride_h_3d = it->second->pool_params->stride_h;
				dst0->conv_params->stride_w_3d = it->second->pool_params->stride_w;
				dst0->conv_params->pad_h_3d = it->second->pool_params->pad_h;
				dst0->conv_params->pad_w_3d = it->second->pool_params->pad_w;

				dst0->conv_params->ip_bw_3d = it->second->ip_bw;
				dst0->conv_params->ip_fl_3d = it->second->ip_fl;
				dst0->conv_params->op_bw_3d = it->second->op_bw;
				dst0->conv_params->op_fl_3d = it->second->op_fl;

				dst0->conv_params->th_layer_in_3d = it->second->th_layer_in;
				dst0->conv_params->th_layer_out_3d = it->second->th_layer_out;
				dst0->conv_params->th_params_3d = it->second->th_params;
				dst0->quantization_scheme = it->second->quantization_scheme;
				dst0->output_file=it->second->output_file;
				//			if(it->second->pool_params->PoolType==MAX)
				//				dst0->opcode = OPCODE_POOL2CONV;
				//			else
				dst0->opcode = OPCODE_AVRPOOL2CONV;

				dst0->conv_params->reluflag=false;

				mapStrXLayer::iterator cur_it = it;
				it++;
				xgraph_opt->layers.erase(cur_it);
				pool_cnt++;

			}else{
#if POOL_KERNEL_EXIST==0
				string layer_name = "conv_pool";
				std::stringstream ss1;

				ss1<<layer_name<<pool_cnt;
				string pool_layer_name = ss1.str();//"conv_lrn1";

				XLayer* dst0 = new XLayer(pool_layer_name, "Convolution");

				xgraph_opt->layers[pool_layer_name] = dst0;

				map < string, XBlob*>::iterator top_blob_it;
				map < string, XBlob*>::iterator bottom_blob_it;

				for(int iter_top=0;iter_top<it->second->top.size();iter_top++){

					dst0->top.push_back(it->second->top[iter_top]);
					dst0->topShape.push_back(it->second->topShape[iter_top]);
					string top_blob_name = it->second->top[iter_top].blob->name;

					top_blob_it = xgraph_opt->blobs.find(top_blob_name);

					for(int iter_t_p=0;iter_t_p<top_blob_it->second->producers.size();iter_t_p++){

						if(top_blob_it->second->producers[iter_t_p].compare(it->second->name)==0){
							top_blob_it->second->producers[iter_t_p] = pool_layer_name;
						}
					}
				}

				for(int iter_bot=0;iter_bot<it->second->bottom.size();iter_bot++){

					dst0->bottom.push_back(it->second->bottom[iter_bot]);
					dst0->bottomShape.push_back(it->second->bottomShape[iter_bot]);

					string bottom_blob_name = it->second->bottom[iter_bot].blob->name;

					bottom_blob_it = xgraph_opt->blobs.find(bottom_blob_name);

					for(int iter_b_c=0;iter_b_c<bottom_blob_it->second->consumers.size();iter_b_c++){

						if(bottom_blob_it->second->consumers[iter_b_c].compare(it->second->name)==0){
							bottom_blob_it->second->consumers[iter_b_c] = pool_layer_name;
						}
					}

				}

				dst0->intermediateShape.push_back(it->second->topShape[0]);
				dst0->conv_params->M_3d = it->second->pool_params->N;
				dst0->conv_params->N_3d = it->second->pool_params->N;
				dst0->conv_params->filter_h_3d = it->second->pool_params->kernel_h;
				dst0->conv_params->filter_w_3d = it->second->pool_params->kernel_w;

				dst0->conv_params->stride_h_3d = it->second->pool_params->stride_h;
				dst0->conv_params->stride_w_3d = it->second->pool_params->stride_w;
				dst0->conv_params->pad_h_3d = it->second->pool_params->pad_h;
				dst0->conv_params->pad_w_3d = it->second->pool_params->pad_w;

				dst0->conv_params->ip_bw_3d = it->second->ip_bw;
				dst0->conv_params->ip_fl_3d = it->second->ip_fl;
				dst0->conv_params->op_bw_3d = it->second->op_bw;
				dst0->conv_params->op_fl_3d = it->second->op_fl;

				dst0->conv_params->th_layer_in_3d = it->second->th_layer_in;
				dst0->conv_params->th_layer_out_3d = it->second->th_layer_out;
				dst0->conv_params->th_params_3d = it->second->th_params;
				dst0->quantization_scheme = it->second->quantization_scheme;
				dst0->output_file=it->second->output_file;
				//			if(it->second->pool_params->PoolType==MAX)
				//				dst0->opcode = OPCODE_POOL2CONV;
				//			else
				dst0->opcode = OPCODE_POOL2CONV;

				dst0->conv_params->reluflag=false;

				mapStrXLayer::iterator cur_it = it;
				it++;
				xgraph_opt->layers.erase(cur_it);
				pool_cnt++;
#else
				it++;
#endif
			}
		}else{
			it++;
		}
	}
	delete pool_layer_type;
}
void XGraphOpt::opt_fc2conv(XGraph *xgraph_opt){

	string *fc_layer_type = new string("InnerProduct");

	int fc_cnt=0;

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();){

		if (it->second->type.compare(*fc_layer_type)==0){


			string layer_name = "conv_fc";
			std::stringstream ss1;

			ss1<<layer_name<<fc_cnt;
			string fc_layer_name = ss1.str();//"conv_lrn1";

			XLayer* dst0 = new XLayer(fc_layer_name, "Convolution");
			xgraph_opt->layers[fc_layer_name] = dst0;

			map < string, XBlob*>::iterator top_blob_it;
			map < string, XBlob*>::iterator bottom_blob_it;

			for(int iter_top=0;iter_top<it->second->top.size();iter_top++){

				dst0->top.push_back(it->second->top[iter_top]);
				dst0->topShape.push_back(it->second->topShape[iter_top]);
				string top_blob_name = it->second->top[iter_top].blob->name;

				top_blob_it = xgraph_opt->blobs.find(top_blob_name);

				for(int iter_t_p=0;iter_t_p<top_blob_it->second->producers.size();iter_t_p++){

					if(top_blob_it->second->producers[iter_t_p].compare(it->second->name)==0){
						top_blob_it->second->producers[iter_t_p] = fc_layer_name;
					}
				}
			}

			for(int iter_bot=0;iter_bot<it->second->bottom.size();iter_bot++){

				dst0->bottom.push_back(it->second->bottom[iter_bot]);
				dst0->bottomShape.push_back(it->second->bottomShape[iter_bot]);

				string bottom_blob_name = it->second->bottom[iter_bot].blob->name;

				bottom_blob_it = xgraph_opt->blobs.find(bottom_blob_name);

				for(int iter_b_c=0;iter_b_c<bottom_blob_it->second->consumers.size();iter_b_c++){

					if(bottom_blob_it->second->consumers[iter_b_c].compare(it->second->name)==0){
						bottom_blob_it->second->consumers[iter_b_c] = fc_layer_name;
					}
				}

			}

			dst0->conv_params->M_3d = it->second->fc_params->M;
			dst0->conv_params->N_3d = it->second->fc_params->N;
			dst0->conv_params->has_bias_3d = it->second->fc_params->has_bias;
			dst0->conv_params->weightsPath_3d = it->second->fc_params->weightsPath;

			dst0->conv_params->biasPath_3d = it->second->fc_params->biasPath;
			dst0->conv_params->weights_3d = it->second->fc_params->weights;
			dst0->conv_params->bias_3d = it->second->fc_params->bias;
			dst0->conv_params->weightsDim_3d = it->second->fc_params->weightsDim;
			dst0->conv_params->biasDim_3d = it->second->fc_params->biasDim;

			dst0->conv_params->ip_bw_3d = it->second->ip_bw;
			dst0->conv_params->ip_fl_3d = it->second->ip_fl;

			dst0->conv_params->op_bw_3d = it->second->op_bw;
			dst0->conv_params->op_fl_3d = it->second->op_fl;
			dst0->conv_params->wt_bw_3d = it->second->wt_bw;
			dst0->conv_params->wt_fl_3d = it->second->wt_fl;

			dst0->conv_params->th_layer_in_3d = it->second->th_layer_in;
			dst0->conv_params->th_layer_out_3d = it->second->th_layer_out;
			dst0->conv_params->th_params_3d = it->second->th_params;

			dst0->intermediateShape.push_back(it->second->topShape[0]);
			dst0->opcode = OPCODE_FC2CONV;
			dst0->conv_params->reluflag=false;
			dst0->output_file=it->second->output_file;
			dst0->quantization_scheme = it->second->quantization_scheme;
			mapStrXLayer::iterator cur_it = it;
			it++;
			xgraph_opt->layers.erase(cur_it);
			fc_cnt++;
		}else{
			it++;
		}
	}
	delete fc_layer_type;
}

void XGraphOpt::opt_pool_conv_fuse(XGraph *xgraph_opt){

	string *relu_layer_type = new string("ReLU");

	string *conv_layer_type = new string("Convolution");
	string *pool_layer_type = new string("Pooling");

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();){

		if(it->second->type.compare(*pool_layer_type)==0){


			if(it->second->top.size()==1){

				int tem_3d_relu_flag = 0;

				map < string, XBlob*>::iterator it_top_b = xgraph_opt->blobs.find(it->second->top[0].blob->name);

				if(it_top_b->second->consumers.size()==1){

					map < string, XLayer*>::iterator it_cun_layer = xgraph_opt->layers.find(it_top_b->second->consumers[0]);

					if(it_cun_layer->second->type.compare(*conv_layer_type)==0){

						it_cun_layer->second->conv_params->M_3d = it->second->pool_params->N;
						it_cun_layer->second->conv_params->N_3d = it->second->pool_params->N;
						it_cun_layer->second->conv_params->filter_h_3d = it->second->pool_params->kernel_h;
						it_cun_layer->second->conv_params->filter_w_3d = it->second->pool_params->kernel_w;

						it_cun_layer->second->conv_params->stride_h_3d = it->second->pool_params->stride_h;
						it_cun_layer->second->conv_params->stride_w_3d = it->second->pool_params->stride_w;
						it_cun_layer->second->conv_params->pad_h_3d = it->second->pool_params->pad_h;
						it_cun_layer->second->conv_params->pad_w_3d = it->second->pool_params->pad_w;

						it_cun_layer->second->conv_params->ip_bw_3d = it->second->ip_bw;
						it_cun_layer->second->conv_params->ip_fl_3d = it->second->ip_fl;
						it_cun_layer->second->conv_params->op_bw_3d = it->second->op_bw;
						it_cun_layer->second->conv_params->op_fl_3d = it->second->op_fl;

						it_cun_layer->second->conv_params->th_layer_in_3d = it->second->th_layer_in;
						it_cun_layer->second->conv_params->th_layer_out_3d = it->second->th_layer_out;
						it_cun_layer->second->conv_params->th_params_3d = it->second->th_params;

						it_cun_layer->second->opcode=OPCODE_POOL_CONV2CONV;



						for(int iter_bs=0;iter_bs<it->second->bottomShape.size();iter_bs++){
							it_cun_layer->second->bottomShape[iter_bs]=it->second->bottomShape[iter_bs];
						}

						map < string, XBlob*>::iterator bottom_blob = xgraph_opt->blobs.find(it_cun_layer->second->bottom[0].blob->name);

						std::vector<string>::iterator it_str=std::find(it->second->bottom[0].blob->consumers.begin(),it->second->bottom[0].blob->consumers.end(),it->second->name);

						if(it_str!=it->second->bottom[0].blob->consumers.end())
						{
							it->second->bottom[0].blob->consumers.erase(it_str);
						}

						it_cun_layer->second->bottom[0].blob=it->second->bottom[0].blob;
						it_cun_layer->second->bottom[0].id = it->second->bottom[0].id;
						it_cun_layer->second->intermediateShape.push_back(it->second->topShape[0]);
						it->second->bottom[0].blob->consumers.push_back(it_cun_layer->second->name);

						xgraph_opt->blobs.erase(bottom_blob);
						mapStrXLayer::iterator cur_it = it;
						it++;
						xgraph_opt->layers.erase(cur_it);

					}else{
						it++;
						continue;
					}
				}else{

					if(it_top_b->second->consumers.size()==2){

						int cnt_type =0;
						int conv_indx=-1;
						int relu_indx=-1;
						vector<string> name_layer_vect;
						string tmp_conv_name ;
						string tmp_relu_name ;
						int l_conv_f=0;
						vector<map < string, XLayer*>::iterator> vect_layer_it;

						for(int iter_c_t=0;iter_c_t<it_top_b->second->consumers.size();iter_c_t++){

							map < string, XLayer*>::iterator cuns_layer_t = xgraph_opt->layers.find(it_top_b->second->consumers[iter_c_t]);
							vect_layer_it.push_back(cuns_layer_t);
							if((cuns_layer_t->second->type.compare(*conv_layer_type)==0) &&(l_conv_f==0)){
								tmp_conv_name = cuns_layer_t->second->name;
								conv_indx=iter_c_t;
								l_conv_f=1;
								cnt_type++;

							}else{
								if(cuns_layer_t->second->type.compare(*relu_layer_type)==0){
									tmp_relu_name = cuns_layer_t->second->name;
									relu_indx=iter_c_t;
									cnt_type++;
								}
							}
						}
						if(cnt_type==2){

							vect_layer_it[conv_indx]->second->conv_params->M_3d = it->second->pool_params->N;
							vect_layer_it[conv_indx]->second->conv_params->N_3d = it->second->pool_params->N;
							vect_layer_it[conv_indx]->second->conv_params->filter_h_3d = it->second->pool_params->kernel_h;
							vect_layer_it[conv_indx]->second->conv_params->filter_w_3d = it->second->pool_params->kernel_w;

							vect_layer_it[conv_indx]->second->conv_params->stride_h_3d = it->second->pool_params->stride_h;
							vect_layer_it[conv_indx]->second->conv_params->stride_w_3d = it->second->pool_params->stride_w;
							vect_layer_it[conv_indx]->second->conv_params->pad_h_3d = it->second->pool_params->pad_h;
							vect_layer_it[conv_indx]->second->conv_params->pad_w_3d = it->second->pool_params->pad_w;
							vect_layer_it[conv_indx]->second->conv_params->reluflag_3d=1;
							vect_layer_it[conv_indx]->second->conv_params->ip_bw_3d = it->second->ip_bw;
							vect_layer_it[conv_indx]->second->conv_params->ip_fl_3d = it->second->ip_fl;
							vect_layer_it[conv_indx]->second->conv_params->op_bw_3d = it->second->op_bw;
							vect_layer_it[conv_indx]->second->conv_params->op_fl_3d = it->second->op_fl;
							vect_layer_it[conv_indx]->second->opcode=OPCODE_POOL_CONV2CONV;

							vect_layer_it[conv_indx]->second->conv_params->th_layer_in_3d = it->second->th_layer_in;
							vect_layer_it[conv_indx]->second->conv_params->th_layer_out_3d = it->second->th_layer_out;
							vect_layer_it[conv_indx]->second->conv_params->th_params_3d = it->second->th_params;

							for(int iter_bs=0;iter_bs<it->second->bottomShape.size();iter_bs++){
								vect_layer_it[conv_indx]->second->bottomShape[iter_bs]=it->second->bottomShape[iter_bs];
							}
							xgrap_layer_delete(xgraph_opt,vect_layer_it[relu_indx]->second->name);

							map < string, XBlob*>::iterator bottom_blob = xgraph_opt->blobs.find(vect_layer_it[conv_indx]->second->bottom[0].blob->name);

							std::vector<string>::iterator it_str=std::find(it->second->bottom[0].blob->consumers.begin(),it->second->bottom[0].blob->consumers.end(),it->second->name);

							if(it_str!=it->second->bottom[0].blob->consumers.end())
							{
								it->second->bottom[0].blob->consumers.erase(it_str);
							}

							vect_layer_it[conv_indx]->second->bottom[0].blob=it->second->bottom[0].blob;
							vect_layer_it[conv_indx]->second->bottom[0].id = it->second->bottom[0].id;

							vect_layer_it[conv_indx]->second->intermediateShape.push_back(it->second->topShape[0]);
							it->second->bottom[0].blob->consumers.push_back(vect_layer_it[conv_indx]->second->name);

							xgraph_opt->blobs.erase(bottom_blob);
							mapStrXLayer::iterator cur_it = it;
							it++;
							xgraph_opt->layers.erase(cur_it);

						}else{
							it++;
							continue;
						}
					}else{
						it++;
					}
				}
			}else
			{
				cerr << "[BEO001] Current version of the chAIDNN won't support more than one top" << endl;
				exit(-1);
			}
		}else{
			it++;
		}
	}
}

void XGraphOpt::opt_eltwise(XGraph *xgraph_opt){

	string *eltwise_layer_type = new string("Eltwise");

	int eltwise_cnt=0;

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();it++){

		if (it->second->type.compare(*eltwise_layer_type)==0){


			string layer_name = "conv_eltwise";
			std::stringstream ss1;

			ss1<<layer_name<<eltwise_cnt;
			string eltwise_layer_name = ss1.str();//"conv_lrn1";

			XLayer* dst0 = new XLayer(eltwise_layer_name, "Convolution");
			xgraph_opt->layers[eltwise_layer_name] = dst0;

			map < string, XBlob*>::iterator top_blob_it;
			map < string, XBlob*>::iterator bottom_blob_it;

			for(int iter_top=0;iter_top<it->second->top.size();iter_top++){

				dst0->top.push_back(it->second->top[iter_top]);
				dst0->topShape.push_back(it->second->topShape[iter_top]);
				string top_blob_name = it->second->top[iter_top].blob->name;

				top_blob_it = xgraph_opt->blobs.find(top_blob_name);

				for(int iter_t_p=0;iter_t_p<top_blob_it->second->producers.size();iter_t_p++){

					if(top_blob_it->second->producers[iter_t_p].compare(it->second->name)==0){
						top_blob_it->second->producers[iter_t_p] = eltwise_layer_name;
					}
				}
			}

			for(int iter_bot=0;iter_bot<it->second->bottom.size();iter_bot++){

				dst0->bottom.push_back(it->second->bottom[iter_bot]);
				dst0->bottomShape.push_back(it->second->bottomShape[iter_bot]);

				string bottom_blob_name = it->second->bottom[iter_bot].blob->name;

				bottom_blob_it = xgraph_opt->blobs.find(bottom_blob_name);

				for(int iter_b_c=0;iter_b_c<bottom_blob_it->second->consumers.size();iter_b_c++){

					if(bottom_blob_it->second->consumers[iter_b_c].compare(it->second->name)==0){
						bottom_blob_it->second->consumers[iter_b_c] = eltwise_layer_name;
					}
				}

			}
			dst0->output_file=it->second->output_file;
			dst0->conv_params->N = bottom_blob_it->second->shape.at(1);
			dst0->conv_params->M = top_blob_it->second->shape.at(1);
			dst0->conv_params->filter_h = 0;
			dst0->conv_params->filter_w = 0;
			dst0->conv_params->pad_h=0;
			dst0->conv_params->pad_w=0;
			dst0->conv_params->dilation=1;
			dst0->conv_params->group =1;
			dst0->conv_params->has_bias = 0;
			dst0->conv_params->reluflag = false;

			dst0->th_layer_in = it->second->th_layer_in;
			dst0->th_layer_out = it->second->th_layer_out;
			dst0->th_params = it->second->th_params;
			dst0->quantization_scheme = it->second->quantization_scheme;
			dst0->ip_bw = it->second->ip_bw;
			dst0->op_bw = it->second->op_bw;
			dst0->ip_fl = it->second->ip_fl;
			dst0->op_fl = it->second->op_fl;
			dst0->wt_bw = it->second->wt_bw;
			dst0->wt_fl = it->second->wt_fl;

			dst0->opcode = OPCODE_ELTWISE;
			xgraph_opt->layers.erase(it);
			eltwise_cnt++;
		}
	}
	delete eltwise_layer_type;
}


void XGraphOpt::opt_relu(XGraph *xgraph_opt){

	string *relu_layer_type = new string("ReLU");
	string *fc_layer_type = new string("InnerProduct");
	string *conv_layer_type = new string("Convolution");
	string *custom_layer_type =   new string("XCustom");

#if O_DEBUG
	cout<< "[O_DEBUG] opt_relu: blobs loop start" << endl;
#endif
	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();){



		int relu_layer_flage=0;

		if (it->second->type.compare(*relu_layer_type)==0){
			string output_file = it->second->output_file;
			map < string, XBlob*>::iterator top_it = xgraph_opt->blobs.find(it->second->top[0].blob->name);
			relu_layer_flage=1;
			if(it->second->relu_params->inPlace){

				mapStrXLayer::iterator cur_it = it;

				it++;
				xgrap_layer_delete(xgraph_opt,cur_it->second->name);

				for(int producers_it =0;producers_it<top_it->second->producers.size(); producers_it++){

#if O_DEBUG
					cout<< "[O_DEBUG] opt_relu: find the blob consumer is relu and enabling the relu flag it producer layer " << endl;
#endif
					string tmp_producer = top_it->second->producers[producers_it];

					map < string, XLayer* > ::iterator producer_layer = xgraph_opt->layers.find(tmp_producer);

					if(producer_layer->second->type.compare(*conv_layer_type)==0)
						producer_layer->second->conv_params->reluflag =1;
					else if (producer_layer->second->type.compare(*fc_layer_type)==0)
						producer_layer->second->fc_params->reluflag=1;
					else if (producer_layer->second->type.compare(*custom_layer_type)==0)
						producer_layer->second->xcustom_params->reluflag=1;
					producer_layer->second->output_file=output_file;
				}
			}else{
				mapStrXLayer::iterator cur_it = it;

				it++;
				xgrap_layer_delete(xgraph_opt,cur_it->second->name);

				for(int producers_it =0;producers_it<top_it->second->producers.size(); producers_it++){

#if O_DEBUG
					cout<< "[O_DEBUG] opt_relu: find the blob consumer is relu and enabling the relu flag it producer layer " << endl;
#endif
					string tmp_producer = top_it->second->producers[producers_it];

					map < string, XLayer* > ::iterator producer_layer = xgraph_opt->layers.find(tmp_producer);

					if(producer_layer->second->type.compare(*conv_layer_type)==0)
						producer_layer->second->conv_params->reluflag =1;
					else if (producer_layer->second->type.compare(*fc_layer_type)==0)
						producer_layer->second->fc_params->reluflag=1;
					else if (producer_layer->second->type.compare(*custom_layer_type)==0)
						producer_layer->second->xcustom_params->reluflag=1;
					producer_layer->second->output_file=output_file;
				}
			}

		}
		if(relu_layer_flage==0)
			it++;
		else{

			relu_layer_flage=0;
		}

	}
	delete relu_layer_type;
	delete fc_layer_type;
	delete conv_layer_type;
}


void XGraphOpt::opt_dropout(XGraph *xgraph_opt){

	string *dropout_layer_type = new string("Dropout");
	int drop_out_flag=0;

	//map < string, XLayer*>::iterator cur_it;

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();){

		if (it->second->type.compare(*dropout_layer_type)==0){
			drop_out_flag=1;
			map < string, XBlob*>::iterator top_it = xgraph_opt->blobs.find(it->second->top[0].blob->name);

			if(it->second->dropout_params->inPlace){

				mapStrXLayer::iterator cur_it = it;

				it++;
				xgrap_layer_delete(xgraph_opt,cur_it->second->name);


			}else{

				mapStrXLayer::iterator cur_it = it;
				it++;
				xgrap_layer_delete(xgraph_opt,cur_it->second->name);
				it=cur_it;

			}

		}
		if(drop_out_flag==0)
			it++;
		else{

			drop_out_flag=0;
		}
	}
	delete dropout_layer_type;
}


void XGraphOpt::opt_concat(XGraph *xgraph_opt){

	string *concat_layer_type = new string("Concat");

	string *concat_layer_name = new string();
	int producers_id =0;
	int concat_flag=0;
#if O_DEBUG
	cout<< "[O_DEBUG] opt_concat: layers loop start" << endl;
#endif
	for(map < string, XLayer* >::iterator it = xgraph_opt->layers.begin();it != xgraph_opt->layers.end();)
	{
		if(it->second->type.compare(*concat_layer_type)==0){

			mapStrXLayer::iterator cur_it = it;
			it++;
			xgrap_layer_delete(xgraph_opt,cur_it->second->name);
			concat_flag=1;
		}
		if(concat_flag==0)
		{
			it++;
		}else{
			concat_flag=0;
		}

	}
	delete concat_layer_type;
	delete concat_layer_name;
}

void XGraphOpt::opt_deconv(XGraph *xgraph_opt){

	string *deconv_layer_type = new string("Deconvolution");
	string *softmax_layer_type = new string("Softmax");
	string *argmax_layer_type = new string("Argmax");

	string *crop_layer_type = new string("Crop");


	int deconv_opt_flag = 0;

	for(map < string, XLayer* > ::iterator it = xgraph_opt->layers.begin();it != xgraph_opt->layers.end(); it++)
	{

		if(it->second->type.compare(*deconv_layer_type)==0){


#if O_DEBUG
			cout<< "[O_DEBUG] opt_deconv: checking deconv_layer_type " << endl;
#endif
			if(it->second->top.size()==1)
			{
				string dev_top_blob_name = it->second->top[0].blob->name;

				XBlob* dev_top_blob = it->second->top[0].blob;

				if(dev_top_blob->consumers.size()==1){

					string consumer_name =  dev_top_blob->consumers[0];

					map < string, XLayer* >::iterator dev_top_consumer_layer = xgraph_opt->layers.find(consumer_name);

					if(dev_top_consumer_layer->second->type.compare(*crop_layer_type)==0){

						if(!dev_top_consumer_layer->second->top.empty())
						{
							if(dev_top_consumer_layer->second->top.size()==1)
							{
								XBlob* crop_layer_top_blob = dev_top_consumer_layer->second->top[0].blob;

								if(!crop_layer_top_blob->consumers.empty()){

									string crop_layer_consumer_name = crop_layer_top_blob->consumers[0];

									map < string, XLayer* >::iterator crop_top_consumer_layer = xgraph_opt->layers.find(crop_layer_consumer_name);

									if(crop_top_consumer_layer->second->type.compare(*argmax_layer_type)==0){
										// cout<<"Deconv,crop and argmax"<<endl;

										if(crop_top_consumer_layer->second->top.size()==1){

											map < string, XBlob*>::iterator argmax_top_blob = xgraph_opt->blobs.find(crop_top_consumer_layer->second->top[0].blob->name);

											map < string, XBlob*>::iterator crop_top_blob_iter = xgraph_opt->blobs.find(crop_top_consumer_layer->second->bottom[0].blob->name);

											crop_top_blob_iter->second->consumers.erase(crop_top_blob_iter->second->consumers.begin() + 0); // Argmax connection from it bottom blob

											if(dev_top_consumer_layer->second->bottom[0].blob->shape.size()==3)
												dev_top_consumer_layer->second->bottom[0].blob->shape[0]=1;
											else{
												if(dev_top_consumer_layer->second->bottom[0].blob->shape.size()==4)
													dev_top_consumer_layer->second->bottom[0].blob->shape[1]=1;
											}
											crop_top_blob_iter->second->shape=crop_top_consumer_layer->second->top[0].blob->shape;
											crop_top_consumer_layer->second->bottomShape[0] = dev_top_consumer_layer->second->bottom[0].blob->shape;
											it->second->topShape[0] = dev_top_consumer_layer->second->bottom[0].blob->shape;

											xgraph_opt->blobs.erase(argmax_top_blob);
											xgraph_opt->layers.erase(crop_top_consumer_layer);
										}

									}
									else
									{
										cerr<<" FCN don't support the " << crop_top_consumer_layer->second->type<<" layer"<< endl;
										exit(-1);
									}
								}
							}
						}
					}
					else{

						if(dev_top_consumer_layer->second->type.compare(*argmax_layer_type)==0){

							if(!dev_top_consumer_layer->second->top.empty())
							{
								XBlob* argmax_layer_top_blob = dev_top_consumer_layer->second->top[0].blob;

								if(!argmax_layer_top_blob->consumers.empty()){

									string argmax_layer_consumer_name = argmax_layer_top_blob->consumers[0];

									map < string, XLayer* >::iterator argmax_top_consumer_layer = xgraph_opt->layers.find(argmax_layer_consumer_name);
									if(argmax_top_consumer_layer->second->type.compare(*crop_layer_type)==0){

										// cout<<"Deconv,argmax and crop"<<endl;
										XBlob* crop_b_blob = argmax_top_consumer_layer->second->bottom[0].blob;

										map < string, XBlob*>::iterator dev_top_blob_iter = xgraph_opt->blobs.find(it->second->top[0].blob->name);
										xgraph_opt->blobs.erase(dev_top_blob_iter);
										xgraph_opt->layers.erase(dev_top_consumer_layer);

										crop_b_blob->producers.erase(crop_b_blob->producers.begin()+0);
										crop_b_blob->producers.push_back(it->second->name);
										it->second->top[0]=dev_top_consumer_layer->second->top[0];
										it->second->topShape[0] = crop_b_blob->shape;
										it->second->output_file= dev_top_consumer_layer->second->output_file;
									}
								}
								else{
									// cout<<"Deconv and argmax"<<endl;
									map < string, XBlob*>::iterator arg_top_blob_iter = xgraph_opt->blobs.find(dev_top_consumer_layer->second->top[0].blob->name);
									map < string, XBlob*>::iterator dev_top_blob_iter = xgraph_opt->blobs.find(it->second->top[0].blob->name);

									arg_top_blob_iter->second->producers.erase(arg_top_blob_iter->second->producers.begin()+0);
									it->second->topShape[0] = arg_top_blob_iter->second->shape;
									arg_top_blob_iter->second->producers.push_back(it->second->name);
									it->second->top[0] = dev_top_consumer_layer->second->top[0];
									xgraph_opt->blobs.erase(dev_top_blob_iter);
									xgraph_opt->layers.erase(dev_top_consumer_layer);
								}
							}
						}
					}
				}
			}
		}
	}

	delete deconv_layer_type;
	delete softmax_layer_type;
	delete argmax_layer_type;
	delete crop_layer_type;
}
