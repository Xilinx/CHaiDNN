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


#include "xgraph_opt.hpp"
#include "xi_funcs.hpp"
#include <math.h>


void xi_mem_type_print(map<string, string> &mem_type_table)
{
	for(map<string, string>::iterator it = mem_type_table.begin(); it != mem_type_table.end(); it++)
	{
		cout <<"blob name: " << it->first<< "blob mem type"<< it->second <<endl;
		// cout << stringVectorToString(it->second->producers) << " >>> " << it->second->name << " >>> " << stringVectorToString(it->second->consumers) << endl;
	}
}
void xlayer_print(vector<XlayerData> & xlayer_seq){

	int seq_len = xlayer_seq.size();
	//	int it1=0;
	cout<<"xlayer_print: xlayer_seq[iter_c].resize_height"<<xlayer_seq[0].resize_height<<endl;
	cout<<"xlayer_print: xlayer_seq[iter_c].resize_width"<<xlayer_seq[0].resize_width<<endl;
	for(int iter_c=0;iter_c<seq_len;iter_c++){

		cout<<"xlayer_print layer id : "<< xlayer_seq[iter_c].cur_seq_id<<endl;

		cout<<"xlayer_print layer opcode : "<< xlayer_seq[iter_c].opcode<<endl;

		cout<<"xlayer_print layer type : "<< xlayer_seq[iter_c].layer_type<<endl;
		cout<<"xlayer_print layer names : "<< xlayer_seq[iter_c].hw_ops->name<<endl;

		cout<<" xlayer_seq[iter_c] current layer ip_bw & ip_fl: "<< xlayer_seq[iter_c].hw_ops->ip_bw<<xlayer_seq[iter_c].hw_ops->ip_fl<<endl;
		cout<<" xlayer_seq[iter_c].prev_layers op_bw & op_fl: "<< xlayer_seq[iter_c].hw_ops->op_bw<<xlayer_seq[iter_c].hw_ops->op_fl<<endl;

		//cout<< xlayer_seq[iter_c].hw_ops->str(true) <<endl;
#if 1
		if(!xlayer_seq[iter_c].prev_layers_ID.empty()){

			int p_size = xlayer_seq[iter_c].prev_layers_ID.size();
			for(int it=0;it<p_size;it++){

				cout<<" xlayer_seq[iter_c].prev_layers_ID: "<< xlayer_seq[iter_c].prev_layers_ID[it].layerindex<<endl;
				cout<<" xlayer_seq[iter_c].prev_layers pos: "<< xlayer_seq[iter_c].prev_layers_ID[it].pos<<endl;
			}
		}
		if(!xlayer_seq[iter_c].next_layer_ID.empty()){

			int p_size = xlayer_seq[iter_c].next_layer_ID.size();

			for(int it=0;it<p_size;it++){

				cout<<" xlayer_seq[iter_c].next_layer_ID: "<< xlayer_seq[iter_c].next_layer_ID[it].layerindex<<endl;
				cout<<" xlayer_seq[iter_c].next_layer pos: "<< xlayer_seq[iter_c].next_layer_ID[it].pos<<endl;

			}
		}
		if(!xlayer_seq[iter_c].ip_peers_shapes.empty()){

			for(int it0=0;it0 < xlayer_seq[iter_c].ip_peers_shapes.size(); it0++){

				for(int it1=0;it1 < xlayer_seq[iter_c].ip_peers_shapes[it0].shape.size(); it1++){

					cout<<" xlayer_seq[iter_c].peer.pos: "<< xlayer_seq[iter_c].ip_peers_shapes[it0].pos<<"xlayer_seq[iter_c].ip_peers_shapes :"<<xlayer_seq[iter_c].ip_peers_shapes[it0].shape[it1]<<endl;
				}
			}
		}
		if(!xlayer_seq[iter_c].op_peers_shapes.empty()){

			for(int it0=0;it0 < xlayer_seq[iter_c].op_peers_shapes.size(); it0++){


				for(int it1=0; it1 < xlayer_seq[iter_c].op_peers_shapes[it0].shape.size(); it1++){

					cout<<" xlayer_seq[iter_c].op_peers_pos: "<< xlayer_seq[iter_c].op_peers_shapes[it0].pos<<"<<xlayer_seq[iter_c].op_peers_shapes[it0].shape[it1]"<<xlayer_seq[iter_c].op_peers_shapes[it0].shape[it1]<<endl;

				}
			}
		}
#endif
		if(!xlayer_seq[iter_c].ip_blob.empty()){

			for(int it0=0;it0 < xlayer_seq[iter_c].ip_blob.size(); it0++){

				//cout<<" xlayer_seq[iter_c].ip_blob[it0]: "<< it0 <<endl;

				cout<<" xlayer_seq[iter_c].ip_blob[it0].handle name: "<< xlayer_seq[iter_c].ip_blob[it0].handle<<endl;
				cout<<" xlayer_seq[iter_c].ip_blob[it0].pos name: "<< xlayer_seq[iter_c].ip_blob[it0].pos<<endl;
				cout<<" xlayer_seq[iter_c].ip_blob[it0].mem_type name: "<< xlayer_seq[iter_c].ip_blob[it0].mem_type<<endl;
				if(!xlayer_seq[iter_c].ip_blob[it0].handle_shape.empty()){



					cout<<" xlayer_seq[iter_c].ip_blob[it0].handle_shape : "<< xlayer_seq[iter_c].ip_blob[it0].handle_shape[0]<<endl;
					cout<<" xlayer_seq[iter_c].ip_blob[it0].handle_shape : "<< xlayer_seq[iter_c].ip_blob[it0].handle_shape[1]<<endl;
					cout<<" xlayer_seq[iter_c].ip_blob[it0].handle_shape : "<< xlayer_seq[iter_c].ip_blob[it0].handle_shape[2]<<endl;
					cout<<" xlayer_seq[iter_c].ip_blob[it0].handle_shape : "<< xlayer_seq[iter_c].ip_blob[it0].handle_shape[3]<<endl;
				}
			}
		}
		if(!xlayer_seq[iter_c].op_blob.empty()){

			for(int it0=0;it0 < xlayer_seq[iter_c].op_blob.size(); it0++){

				//cout<<" xlayer_seq[iter_c].op_blob[it0]: "<< it0 <<endl;

				cout<<" xlayer_seq[iter_c].op_blob[it0].handle name: "<< xlayer_seq[iter_c].op_blob[it0].handle<<endl;
				cout<<" xlayer_seq[iter_c].op_blob[it0].pos name: "<< xlayer_seq[iter_c].op_blob[it0].pos<<endl;
				cout<<" xlayer_seq[iter_c].op_blob[it0].mem_type name: "<< xlayer_seq[iter_c].op_blob[it0].mem_type<<endl;

				if(!xlayer_seq[iter_c].ip_blob[it0].handle_shape.empty()){


					//	for(auto i=0;  i<xlayer_seq[iter_c].op_blob[it0].handle_shape.size(); i++);{

					cout<<" xlayer_seq[iter_c].op_blob[it0].handle_shape : "<< xlayer_seq[iter_c].op_blob[it0].handle_shape[0]<<endl;
					cout<<" xlayer_seq[iter_c].op_blob[it0].handle_shape : "<< xlayer_seq[iter_c].op_blob[it0].handle_shape[1]<<endl;
					cout<<" xlayer_seq[iter_c].op_blob[it0].handle_shape : "<< xlayer_seq[iter_c].op_blob[it0].handle_shape[2]<<endl;
					cout<<" xlayer_seq[iter_c].op_blob[it0].handle_shape : "<< xlayer_seq[iter_c].op_blob[it0].handle_shape[3]<<endl;
					//	}
				}
			}
		}
		if(!xlayer_seq[iter_c].meanShape.empty()){

			for(int it0=0;it0 < xlayer_seq[iter_c].meanShape.size(); it0++){
				cout<<" xlayer_seq[iter_c].meanShape[it0] : "<< xlayer_seq[iter_c].meanShape[it0]<<endl;

			}
			cout<<" xlayer_seq[iter_c].meanFile : "<< xlayer_seq[iter_c].meanFile<<endl;
		}

	}
}

void XGraphOpt::xi_opt_find_max_q_factor(XGraph* xgraph_opt){

 	for(map < string, XBlob*>::iterator it = xgraph_opt->blobs.begin();it !=xgraph_opt->blobs.end();it++){

		int Qfactor_consumer =  0;
		int Ffactor_consumer =  0;
		int Qfactor_producer =  0;
		int Ffactor_producer =  0;
		int Qfactor_max =  0;
		int Ffactor_max =  0;

		if(it->second->consumers.size()!=0){

			for(int iter_con_c=0;iter_con_c<it->second->consumers.size();iter_con_c++){

				string consumer_name = it->second->consumers[iter_con_c];
				map < string, XLayer*>::iterator layer_it = xgraph_opt->layers.find(consumer_name);
				int Qfactor_tmp =0;
				Qfactor_tmp = layer_it->second->ip_bw;

				if(Qfactor_tmp>Qfactor_consumer){

					Ffactor_consumer = layer_it->second->ip_fl;
					Qfactor_consumer=(Qfactor_tmp-layer_it->second->ip_fl);

				}
			}
			for(int iter_pro_c=0;iter_pro_c<it->second->producers.size();iter_pro_c++){

				string producer_name = it->second->producers[iter_pro_c];

				map < string, XLayer*>::iterator layer_it = xgraph_opt->layers.find(producer_name);
				int Qfactor_tmp =0;
				Qfactor_tmp = layer_it->second->op_bw;

				if(Qfactor_tmp>Qfactor_producer){

					Ffactor_producer = layer_it->second->op_fl;
					Qfactor_producer=(Qfactor_tmp-layer_it->second->op_fl);

				}

			}
			if(Qfactor_producer<Qfactor_consumer){
				Qfactor_max=Qfactor_consumer;
				Ffactor_max=Ffactor_consumer;
			}else{
				Qfactor_max=Qfactor_producer;
				Ffactor_max=Ffactor_producer;

			}

			for(int iter_con_c=0;iter_con_c<it->second->consumers.size();iter_con_c++){

				string consumer_name = it->second->consumers[iter_con_c];
				map < string, XLayer*>::iterator layer_it = xgraph_opt->layers.find(consumer_name);

				layer_it->second->ip_bw=Qfactor_max+Ffactor_max;
				layer_it->second->ip_fl=Ffactor_max;


			}
			for(int iter_pro_c=0;iter_pro_c<it->second->producers.size();iter_pro_c++){

				string producer_name = it->second->producers[iter_pro_c];
				map < string, XLayer*>::iterator layer_it = xgraph_opt->layers.find(producer_name);
				layer_it->second->op_bw=Qfactor_max+Ffactor_max;
				layer_it->second->op_fl=Ffactor_max;
			}
		}
	}

}

void XGraphOpt::xi_opt_lrn(XGraph* xgraph_opt){

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

			string uniqname = xgraph_opt->getUniqueBlobName();
			string username = lrn1_name;
			XBlob* tmp_blob = new XBlob(username, uniqname);
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

void XGraphOpt::xi_opt_reshape(XGraph* xgraph_opt){

	string *reshape_layer_type = new string("Reshape");

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();){

		int cont=0;
		if (it->second->type.compare(*reshape_layer_type)==0){

#if O_DEBUG
			cout<<"find the Reshape layer: "<<it->second->name<<"Reshape layer bottom name "<<it->second->bottom[0].blob->name<<"Reshape layer top name "<<it->second->top[0].blob->name<<endl;
#endif

			string layer_bottom_name =  it->second->bottom[0].blob->name;

			mapStrXBlob::iterator bottom_it  =  xgraph_opt->blobs.find(layer_bottom_name);

			XBlob* bottom_blob = bottom_it->second;
			XBlob* top_blob = it->second->top[0].blob;

			top_blob->producers.erase(top_blob->producers.begin()+0);

			for(int iter_b_p=0;iter_b_p<bottom_blob->producers.size();iter_b_p++)
			{
				mapStrXLayer::iterator p_layer = xgraph_opt->layers.find(bottom_blob->producers[iter_b_p]);
				p_layer->second->top[0] = it->second->top[0];
				top_blob->producers.push_back(bottom_blob->producers[iter_b_p]);
			}


			if(bottom_it!=xgraph_opt->blobs.end()){
				xgraph_opt->blobs.erase(bottom_it);
			}
			mapStrXLayer::iterator cur_it = it;

			if(cur_it!=xgraph_opt->layers.end()){
				cont=1;
				it++;
				xgraph_opt->layers.erase(cur_it);
			}
		}
		if(cont==0)
			it++;
	}
	delete reshape_layer_type;
}



void XGraphOpt::xi_opt_flatten(XGraph* xgraph_opt){

	string *flatten_layer_type = new string("Flatten");

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();){

		int cont=0;
		if (it->second->type.compare(*flatten_layer_type)==0){

#if O_DEBUG
			cout<<"find the Reshape layer: "<<it->second->name<<"flatten layer bottom name "<<it->second->bottom[0].blob->name<<"Reshape layer top name "<<it->second->top[0].blob->name<<endl;
#endif

			string layer_bottom_name =  it->second->bottom[0].blob->name;

			mapStrXBlob::iterator bottom_it  =  xgraph_opt->blobs.find(layer_bottom_name);

			XBlob* bottom_blob = bottom_it->second;
			XBlob* top_blob = it->second->top[0].blob;

			top_blob->producers.erase(top_blob->producers.begin()+0);

			for(int iter_b_p=0;iter_b_p<bottom_blob->producers.size();iter_b_p++)
			{
				mapStrXLayer::iterator p_layer = xgraph_opt->layers.find(bottom_blob->producers[iter_b_p]);
				p_layer->second->top[0] = it->second->top[0];
				top_blob->producers.push_back(bottom_blob->producers[iter_b_p]);
			}


			if(bottom_it!=xgraph_opt->blobs.end()){
				xgraph_opt->blobs.erase(bottom_it);
			}
			mapStrXLayer::iterator cur_it = it;

			if(cur_it!=xgraph_opt->layers.end()){
				cont=1;
				it++;
				xgraph_opt->layers.erase(cur_it);
			}
		}
		if(cont==0)
			it++;
	}
	delete flatten_layer_type;
}

int xi_vectDim (vector<int>& src)
{
	int finalDim = 1;

	for(int i=0; i<src.size(); i++)
	{
		finalDim *= src.at(i);
	}

	return finalDim;
}

void xi_maxsize_blob(XGraph *xgraph_opt,vector <int> &max_shape){

	int Dim0,Dim1;

	max_shape.push_back(1);
	max_shape.push_back(1);
	max_shape.push_back(1);
	max_shape.push_back(1);
	Dim0 = xi_vectDim(max_shape);

	for(map < string, XBlob*>::iterator it=xgraph_opt->blobs.begin();it!=xgraph_opt->blobs.end();it++){

		Dim1 = xi_vectDim(it->second->shape);

		if(Dim1>Dim0){

			max_shape=it->second->shape;
			Dim0 = Dim1;

		}

	}
}

void xi_find_blobs_degree(XGraph *xgraph,vector<BDegree> &Bdepth){


	BDegree tmp_bdegree1;// = new BDegree();

	int degree_flag=0;

#if O_DEBUG
	cout<< "[O_DEBUG] xi_find_blobs_degree: xi_find_blobs_degree start" << endl;
#endif

	map < string, XBlob*>::iterator d_blob = xgraph->blobs.find("data");

	if(d_blob==xgraph->blobs.end()){

		cerr << "[Error:] data blob is not exists, this kind of prototxt format won't support" << endl;
		exit(-1);
	}

	tmp_bdegree1.blobs_degree = degree_flag;
	tmp_bdegree1.blobs_name.push_back(d_blob->second->name);
	Bdepth.push_back(tmp_bdegree1);
	int top_blob_exist =0;
	XBlob *tmp_blob = new XBlob();

	int blob_str_flag =0;

#if O_DEBUG
	cout<<"Bdepth.size() :"<<Bdepth.size()<<endl;
#endif

	for(int iter_d=0; iter_d < Bdepth.size();iter_d++)
	{

#if O_DEBUG
		cout<< "[O_DEBUG] xi_find_blobs_degree: Bdepth.size() loop" << endl;
#endif
		BDegree tmp_bdegree;// = new BDegree();

		for(int iter_db=0;iter_db<Bdepth[iter_d].blobs_name.size();iter_db++)
		{

#if O_DEBUG
			cout<< "[O_DEBUG] xi_find_blobs_degree: Bdepth[iter_d].blobs_name.size() loop" << endl;
#endif
			map < string, XBlob*>::iterator blob_ptr = xgraph->blobs.find(Bdepth[iter_d].blobs_name[iter_db]);

			if(blob_ptr!=xgraph->blobs.end())
				tmp_blob = blob_ptr->second;
			else
				return;

			for(int iter_c=0;iter_c<tmp_blob->consumers.size();iter_c++)
			{

#if O_DEBUG
				cout<< "[O_DEBUG] xi_find_blobs_degree: tmp_blob->consumers.size() loop" << endl;
#endif
				map < string, XLayer* >::iterator tmp_c_layer = xgraph->layers.find(tmp_blob->consumers[iter_c]);

				//string tmp_top_name = tmp_c_layer->second->top[0].blob->name;
				//map < string, XBlob*>::iterator tmp_top_it = xgraph->blobs.find(tmp_top_name);

				if(tmp_c_layer->second->top.size()==0) // consumer does not have top blobs then continue for next consumer
					continue;

				XBlob * tmp_top = tmp_c_layer->second->top[0].blob; // we are assuming only one top blob is exist for each layer.

				for(int iter_p=0;iter_p<tmp_top->producers.size();iter_p++)
				{

#if O_DEBUG
					cout<< "[O_DEBUG] xi_find_blobs_degree: tmp_top->producers.size() loop" << endl;
#endif
					string name_producer = tmp_top->producers[iter_p];

					if(name_producer.compare(tmp_blob->consumers[iter_c])==0) // checking the self producer or multiple producers form this tmp_blob
					{

						if(tmp_c_layer->second->bottom.size()==1)
						{
							blob_str_flag=0;
							continue;
						}
						else
						{

							for(int iter_b_b_c=0;iter_b_b_c<tmp_c_layer->second->bottom.size();iter_b_b_c++)
							{

								string tmp_bottom = tmp_c_layer->second->bottom[iter_b_b_c].blob->name;

								for(int iter_bs=0;iter_bs<Bdepth.size();iter_bs++)
								{
#if O_DEBUG
									cout<< "[O_DEBUG] xi_find_blobs_degree: Bdepth.size() element find loop" << endl;
#endif
									std::vector<string>::iterator it=std::find(Bdepth[iter_bs].blobs_name.begin(),Bdepth[iter_bs].blobs_name.end(),tmp_bottom);

									if(it==Bdepth[iter_bs].blobs_name.end())
									{
#if O_DEBUG
										cout<< "[O_DEBUG] xi_find_blobs_degree: std::find element not find in bdepth queue" << endl;
#endif
										blob_str_flag=1;
									}
									else
									{

										blob_str_flag=0;
										break;
									}


								}
								if(blob_str_flag==1)
									break;
							}
							if(blob_str_flag==0)
								continue;
						}
					}
					else
					{

						map < string, XLayer* >::iterator tmp_p_layer = xgraph->layers.find(name_producer);
						string tmp_bottom = tmp_p_layer->second->bottom[0].blob->name;

						for(int iter_bs=0;iter_bs<Bdepth.size();iter_bs++)
						{
#if O_DEBUG
							cout<< "[O_DEBUG] xi_find_blobs_degree: Bdepth.size() element find loop" << endl;
#endif
							std::vector<string>::iterator it=std::find(Bdepth[iter_bs].blobs_name.begin(),Bdepth[iter_bs].blobs_name.end(),tmp_bottom);
							if(it==Bdepth[iter_bs].blobs_name.end())
							{
#if O_DEBUG
								cout<< "[O_DEBUG] xi_find_blobs_degree: std::find element not find in bdepth queue" << endl;
#endif
								blob_str_flag=1;
							}
							else
							{

								blob_str_flag=0;
								break;
							}
						}
					}
				}
				if(blob_str_flag==0)
				{

					std::vector<string>::iterator it = std::find(tmp_bdegree.blobs_name.begin(),tmp_bdegree.blobs_name.end(),tmp_top->name);

					if(it==tmp_bdegree.blobs_name.end())
					{

						tmp_bdegree.blobs_name.push_back(tmp_top->name);
						tmp_bdegree.blobs_degree=iter_d+1;
						blob_str_flag=1;
						top_blob_exist=1;
					}
				}
			}
		}

		if(top_blob_exist)
		{
			Bdepth.push_back(tmp_bdegree);
			top_blob_exist=0;
		}

	}

}

void xi_print_blobs_degre(vector < BDegree > &Bdepth){

	for(int iter_bd=0;iter_bd<Bdepth.size();iter_bd++){

		for(int iter_b=0;iter_b<Bdepth[iter_bd].blobs_name.size();iter_b++){

			cout<<" Bdepth[iter_bd].blobs_degree:" <<Bdepth[iter_bd].blobs_degree<<"Bdepth[iter_bd].blobs_name: "<<Bdepth[iter_bd].blobs_name[iter_b]<<endl;
		}
	}
}
void xi_create_seq_num_table(vector < XlayerData > &xlayer_seq,map<string, int> &layer_seq_table,map<string, string> &layer_type_table,vector < BDegree >&Bdepth,XGraph *xgraph){

	int cur_seq_id=0;

	for(int iter_d=0;iter_d<Bdepth.size();iter_d++){

		for(int iter_b=0;iter_b<Bdepth[iter_d].blobs_name.size();iter_b++){

			string bdepth_current_blob = Bdepth[iter_d].blobs_name[iter_b];
			map < string, XBlob*>::iterator blob_ptr = xgraph->blobs.find(bdepth_current_blob);
			XBlob *tmp_blob = blob_ptr->second;

			for(int iter_c=0;iter_c< tmp_blob->consumers.size();iter_c++){

				XlayerData tmp_xlayerdata;

				map < string, XLayer* >::iterator tmp_c_layer = xgraph->layers.find(tmp_blob->consumers[iter_c]);

				if(tmp_c_layer->second->bottom.size()<2){

					tmp_xlayerdata.hw_ops=tmp_c_layer->second;
					tmp_xlayerdata.cur_seq_id = cur_seq_id++;//iter_d*Bdepth[iter_d].blobs_name.size()+iter_b;
					//int_ptr->layer_num=tmp_xlayerdata.cur_seq_id;
					layer_seq_table[tmp_blob->consumers[iter_c]]=tmp_xlayerdata.cur_seq_id;
					layer_type_table[tmp_c_layer->second->name] = tmp_c_layer->second->type;

					xlayer_seq.push_back(tmp_xlayerdata);

				}else{

					for(int iter_b_c=0;iter_b_c<tmp_c_layer->second->bottom.size();iter_b_c++){

						string tmp_bottom = tmp_c_layer->second->bottom[iter_b_c].blob->name;
						int tmp_bottom_inx=0;

						for(int iter_bs=0;iter_bs<Bdepth.size();iter_bs++){
#if O_DEBUG
							cout<< "[O_DEBUG] xi_create_seq_num_table: Bdepth.size() element find loop" << endl;
#endif
							std::vector<string>::iterator it=std::find(Bdepth[iter_bs].blobs_name.begin(),Bdepth[iter_bs].blobs_name.end(),tmp_bottom);

							if(it==Bdepth[iter_bs].blobs_name.end()){
#if O_DEBUG
								cout<< "[O_DEBUG] xi_create_seq_num_table: std::find element not find in bdepth queue" << endl;
#endif
								tmp_bottom_inx=-1;

							}else{

								tmp_bottom_inx=iter_bs;
								break;
							}
						}
						if(Bdepth[iter_d].blobs_degree>tmp_bottom_inx){

							string curr_layer_name = tmp_c_layer->second->name;

							map<string, int>::iterator layer_id_it= layer_seq_table.find(curr_layer_name);
							if(layer_id_it!=layer_seq_table.end())
							{
								break;

							}else{
								tmp_xlayerdata.hw_ops=tmp_c_layer->second;
								tmp_xlayerdata.cur_seq_id = cur_seq_id++;//iter_d*Bdepth[iter_d].blobs_name.size()+iter_b;
								layer_seq_table[tmp_blob->consumers[iter_c]]=tmp_xlayerdata.cur_seq_id;
								layer_type_table[tmp_c_layer->second->name] = tmp_c_layer->second->type;
								xlayer_seq.push_back(tmp_xlayerdata);
							}
						}else if((Bdepth[iter_d].blobs_degree==tmp_bottom_inx) &&(tmp_bottom.compare(bdepth_current_blob)!=0)){
							string curr_layer_name = tmp_c_layer->second->name;

							map<string, int>::iterator layer_id_it= layer_seq_table.find(curr_layer_name);
							if(layer_id_it!=layer_seq_table.end())
							{
								break;

							}else{
								tmp_xlayerdata.hw_ops=tmp_c_layer->second;
								tmp_xlayerdata.cur_seq_id = cur_seq_id++;//iter_d*Bdepth[iter_d].blobs_name.size()+iter_b;
								layer_seq_table[tmp_blob->consumers[iter_c]]=tmp_xlayerdata.cur_seq_id;
								layer_type_table[tmp_c_layer->second->name] = tmp_c_layer->second->type;
								xlayer_seq.push_back(tmp_xlayerdata);
							}
						}
					}
				}

			}
		}
	}
}
void xi_update_prev_next_layer_info(vector < XlayerData > &xlayer_seq, map<string, int> &layer_seq_table,XGraph *xgraph){


	io_buffer_pos tmp_pt;

	for(map < string, XLayer* >::iterator it = xgraph->layers.begin();it !=xgraph->layers.end();it++){


		vector < vector<int> > ip_peer;
		vector < vector<int> > out_peer;

		XLayer* curr_layer = it->second;
		string curr_layer_name = curr_layer->name;

		map<string, int>::iterator layer_id_it= layer_seq_table.find(curr_layer_name);

		for(int iter_l_b=0;iter_l_b<curr_layer->bottom.size();iter_l_b++){

			for(int iter_pre_c=0;iter_pre_c<curr_layer->bottom[iter_l_b].blob->producers.size();iter_pre_c++){

				string name_prevlayer =curr_layer->bottom[iter_l_b].blob->producers[iter_pre_c];
				map < string, XLayer* >::iterator prev_it = xgraph->layers.find(name_prevlayer);
				XLayer* prev_layer = prev_it->second;
				map<string, int>::iterator it_table = layer_seq_table.find(name_prevlayer);
				tmp_pt.layerindex = it_table->second;
				tmp_pt.pos =curr_layer->bottom[iter_l_b].id; // I need to check with Abid like no layer writing to multiple top blobs
				xlayer_seq[layer_id_it->second].prev_layers_ID.push_back(tmp_pt);

			}

		}
		for(int iter_l_t=0;iter_l_t<curr_layer->top.size();iter_l_t++){

			for(int iter_next_c=0;iter_next_c<curr_layer->top[iter_l_t].blob->consumers.size();iter_next_c++){

				string name_nextlayer =curr_layer->top[iter_l_t].blob->consumers[iter_next_c];
				map < string, XLayer* >::iterator next_it = xgraph->layers.find(name_nextlayer);
				XLayer* next_layer = next_it->second;
				map<string, int>::iterator it_table = layer_seq_table.find(name_nextlayer);
				tmp_pt.layerindex = it_table->second;
				tmp_pt.pos = curr_layer->top[iter_l_t].id;
				xlayer_seq[layer_id_it->second].next_layer_ID.push_back(tmp_pt);


			}
		}
		// filling input peer shapes and its partitions

		for(int iter_b_s=0;iter_b_s<curr_layer->bottom.size();iter_b_s++){

			for(int iter_b_c=0;iter_b_c<curr_layer->bottom[iter_b_s].id;iter_b_c++){

				string name_bottom_c =curr_layer->bottom[iter_b_s].blob->consumers[iter_b_c];

				if(name_bottom_c.compare(curr_layer_name)==0)
					continue;

				map < string, XLayer* >::iterator bottom_c_it = xgraph->layers.find(name_bottom_c);
				io_peers  in_peer_ojb;// = new io_peers();

				XLayer* bottom_c_layer = bottom_c_it->second;
				in_peer_ojb.pos = bottom_c_layer->bottom[iter_b_s].id;
				//in_peer_ojb->shape = bottom_c_layer->bottom[0].blob->shape;

				//for(int s_it=0;s_it<bottom_c_layer->bottom[0].blob->shape.size();s_it++){
				for(int s_it=0;s_it<bottom_c_layer->bottomShape[iter_b_s].size();s_it++){

					in_peer_ojb.shape.push_back(bottom_c_layer->bottomShape[iter_b_s][s_it]);
#if O_DEBUG
					cout<<"in_peer_ojb.pos:"<<in_peer_ojb.pos<<" in_peer_ojb.shape[s_it]: " << in_peer_ojb.shape[s_it]<<endl;
					cout<<"bottom_c_layer->bottom[0].id:"<<bottom_c_layer->bottom[iter_b_s].id<<" bottom_c_layer->bottom[0].blob->shape[s_it]:" << bottom_c_layer->bottom[iter_b_s].blob->shape[s_it]<<endl;
#endif
				}

				xlayer_seq[layer_id_it->second].ip_peers_shapes.push_back(in_peer_ojb);

			}
		}

		// filling output peer shapes and its partitions
		for(int iter_t_s=0;iter_t_s<curr_layer->top.size();iter_t_s++){

			for(int iter_b_c=0;iter_b_c<curr_layer->top[iter_t_s].id;iter_b_c++){

				string name_top_p =curr_layer->top[iter_t_s].blob->producers[iter_b_c];

				if(name_top_p.compare(curr_layer_name)==0)
					continue;

				map < string, XLayer* >::iterator top_c_it = xgraph->layers.find(name_top_p);
				io_peers  on_peer_ojb;// = new io_peers();

				XLayer* top_p_layer = top_c_it->second;
				on_peer_ojb.pos = top_p_layer->top[iter_t_s].id;
				//on_peer_ojb->shape = top_p_layer->top[0].blob->shape;

				//for(int s_it=0;s_it<top_p_layer->top[0].blob->shape.size();s_it++){
				for(int s_it=0;s_it<top_p_layer->topShape[iter_t_s].size();s_it++){

					on_peer_ojb.shape.push_back(top_p_layer->topShape[iter_t_s][s_it]);
#if O_DEBUG
					cout<<"on_peer_ojb.pos:"<<on_peer_ojb.pos<<" on_peer_ojb.shape[s_it]: " << on_peer_ojb.shape[s_it]<<endl;
					cout<<"top_p_layer->top[0].id:"<<top_p_layer->top[0].id<<" top_p_layer->top[0].blob->shape[s_it]: " << top_p_layer->top[0].blob->shape[s_it]<<endl;
#endif
				}

				xlayer_seq[layer_id_it->second].op_peers_shapes.push_back(on_peer_ojb);

			}
		}
	}
}

void xi_find_hw_and_sw_layer_type(vector <struct xtract_layer_type> &xtract_layer_type_e,string &layer_type,int &hs_ops_type){

	int flag_layer_type_find=0;
	int iter_lt=0;
	for(iter_lt=0;iter_lt< xtract_layer_type_e.size();iter_lt++){

		if(xtract_layer_type_e[iter_lt].layer_name.compare(layer_type)==0){
			hs_ops_type=xtract_layer_type_e[iter_lt].layer_type;
			flag_layer_type_find=1;
			break;
		}else{
			flag_layer_type_find=0;
		}
	}
	if(flag_layer_type_find==1)
	{
#if O_DEBUG
		cout<<"xi_find_hw_and_sw_layer_type:Layer is find "<<"layer type:"<<layer_type<<endl;
#endif
	}
	else{
		iter_lt=iter_lt-1;
		ELOG(!(xtract_layer_type_e[iter_lt].layer_name.compare(layer_type)==0),"EO005", " layer type: Layer is not support : "<< layer_type);
	}
}

void xi_find_mem_type_io_buffers(XGraph *xgraph,vector < BDegree > &Bdepth,map<string, string> &layer_type_table,map<string, string> &mem_type_table){

	vector <struct xtract_layer_type> xtract_layer_type_e;
	xi_get_hw_and_sw_layer(xtract_layer_type_e);

	mem_type_table[Bdepth[0].blobs_name[0]]="non_cacheable";

	for(int iter_d=1;iter_d<Bdepth.size();iter_d++){

		int hs_ops_type=-1;

		int p_hw_type=0;
		int p_sw_type=0;
		int c_hw_type=0;
		int c_sw_type=0;

		for(int iter_db=0;iter_db<Bdepth[iter_d].blobs_name.size();iter_db++){ // data is input buffer,so we are starting from 1 index

			map < string, XBlob* >::iterator blob_it = xgraph->blobs.find(Bdepth[iter_d].blobs_name[iter_db]);

			if(blob_it==xgraph->blobs.end()){
				cerr<<"invalid blob name"<<endl;
				exit(-1);
			}

			p_hw_type=0;
			p_sw_type=0;

			c_hw_type=0;
			c_sw_type=0;

			for(int iter_b_p_c=0;iter_b_p_c<blob_it->second->producers.size();iter_b_p_c++){

				string cur_blob_p_name = blob_it->second->producers[iter_b_p_c];
				map<string, string>::iterator p_it = layer_type_table.find(cur_blob_p_name);
				string curr_blob_p_type = p_it->second;
				xi_find_hw_and_sw_layer_type(xtract_layer_type_e,curr_blob_p_type,hs_ops_type);
				if(hs_ops_type==EXE_ON_HW)// ops layer type is hardware
					p_hw_type=1;
				else
					p_sw_type=1;
			}

			for(int iter_b_c_c=0;iter_b_c_c<blob_it->second->consumers.size();iter_b_c_c++){

				string cur_blob_c_name = blob_it->second->consumers[iter_b_c_c];
				map<string, string>::iterator c_it = layer_type_table.find(cur_blob_c_name);
				string curr_blob_c_type = c_it->second;
				xi_find_hw_and_sw_layer_type(xtract_layer_type_e,curr_blob_c_type,hs_ops_type);

				if(hs_ops_type==EXE_ON_HW)// ops layer type is hardware
					c_hw_type=1;
				else
					c_sw_type=1;
			}

			if(p_hw_type==1 && p_sw_type==1){

				if(c_hw_type==1 && c_sw_type==1){

					mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="non_cacheable";

				}
				else if(c_hw_type==0 && c_sw_type==1)
				{
					mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="cacheable";
				}else
				{
					if(c_hw_type==1 && c_sw_type==0)
						mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="non_cacheable";
				}
			}
			else if(p_hw_type==0 && p_sw_type==1)
			{
				if(c_hw_type==1 && c_sw_type==1)
				{
					mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="non_cacheable";
				}
				else if(c_hw_type==0 && c_sw_type==1){
					mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="malloc";
				}
				else
				{
					if(c_hw_type==1 && c_sw_type==0)
						mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="non_cacheable";

					else{
						if(blob_it->second->consumers.size()==0)
						{
							mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="malloc";

						}else{

							cerr<<" xi_find_mem_type_io_buffers invalid consumers type"<<endl;
							return;

						}
					}
				}

			}
			else
			{
				if(p_hw_type==1 && p_sw_type==0){

					if(c_hw_type==1 && c_sw_type==1)
					{
						mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="non_cacheable";
					}
					else if(c_hw_type==0 && c_sw_type==1){

						mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="cacheable";
					}
					else
					{
						mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="non_cacheable";
					}
				}
				else{
					cerr<<"xi_find_mem_type_io_buffers invalid producers type p_sw_type = p_hw_type= "<<p_sw_type<<p_hw_type<<endl;
					return;
				}

			}
		}
	}
}

void xi_update_io_mem_type(vector < XlayerData > & xlayer_seq,XGraph *xgraph,vector < BDegree > &Bdepth,map<string, string> &layer_type_table,map<string, string> &mem_type_table){


	vector <int> max_shape;

	xi_maxsize_blob(xgraph,max_shape);

	vector <struct xtract_layer_type> xtract_layer_type_e;
	xi_get_hw_and_sw_layer(xtract_layer_type_e);

	xi_find_mem_type_io_buffers(xgraph,Bdepth,layer_type_table,mem_type_table);

	for(int iter_l=0;iter_l<xlayer_seq.size();iter_l++){

		if(iter_l==0){

			if(!xgraph->meanShape.empty()){
				xlayer_seq[iter_l].meanShape = xgraph->meanShape;
				xlayer_seq[iter_l].meanFile = xgraph->meanFile;
			}
			xlayer_seq[iter_l].resize_height = xgraph->transform_params.resize_height;
			xlayer_seq[iter_l].resize_width = xgraph->transform_params.resize_width;

		}

		string curr_layer_type =xlayer_seq[iter_l].hw_ops->type;
		int hs_ops_type=-1;

		xi_find_hw_and_sw_layer_type(xtract_layer_type_e,curr_layer_type,hs_ops_type);

		if(hs_ops_type==EXE_ON_HW)// ops layer type is hardware
			xlayer_seq[iter_l].layer_type="hardware";
		else
			xlayer_seq[iter_l].layer_type="software";

		xlayer_seq[iter_l].opcode = xlayer_seq[iter_l].hw_ops->opcode;
		for(int iter_l_b_c=0;iter_l_b_c<xlayer_seq[iter_l].hw_ops->bottom.size();iter_l_b_c++){

			HBlob tmp_hblob_ip;
			map<string, string>::iterator mem_it = mem_type_table.find(xlayer_seq[iter_l].hw_ops->bottom[iter_l_b_c].blob->name);
			tmp_hblob_ip.handle = mem_it->first;
			tmp_hblob_ip.pos = xlayer_seq[iter_l].hw_ops->bottom[iter_l_b_c].id;
			tmp_hblob_ip.handle_shape=max_shape;
			tmp_hblob_ip.mem_type = mem_it->second;
			xlayer_seq[iter_l].ip_blob.push_back(tmp_hblob_ip);
		}
		for(int iter_l_t_c=0;iter_l_t_c<xlayer_seq[iter_l].hw_ops->top.size();iter_l_t_c++){

			HBlob tmp_hblob_op;
			map<string, string>::iterator mem_it = mem_type_table.find(xlayer_seq[iter_l].hw_ops->top[iter_l_t_c].blob->name);
			tmp_hblob_op.handle = mem_it->first;
			tmp_hblob_op.pos = xlayer_seq[iter_l].hw_ops->top[iter_l_t_c].id;
			tmp_hblob_op.handle_shape=max_shape;
			tmp_hblob_op.mem_type = mem_it->second;
			xlayer_seq[iter_l].op_blob.push_back(tmp_hblob_op);
		}
	}
}

void xi_cmp_freehandle_layers_indx_and_curr_blob_layers_indx(vector < BDegree > &Bdepth,vector < XlayerData > & xlayer_seq,XGraph *xgraph,map<string, int> &layer_seq_table,int free_handle_degree_num,int free_handle_blob_num,int curr_degree_num,int curr_blob_num,int *res){


	map < string, XBlob*>::iterator free_handles_org_blob_it = xgraph->blobs.find(Bdepth[free_handle_degree_num].blobs_name[free_handle_blob_num]);
	map < string, XBlob*>::iterator curr_blob_it = xgraph->blobs.find(Bdepth[curr_degree_num].blobs_name[curr_blob_num]);

	int tmp_free_pro_indx=0;
	int tmp_free_cons_indx=0;
	int tmp_free_max_indx=0;

	int tmp_curr_pro_indx=0;
	int tmp_curr_cons_indx=0;
	int tmp_curr_max_indx=0;

	for(int iter_cur_b_p=0;iter_cur_b_p<free_handles_org_blob_it->second->producers.size();iter_cur_b_p++){

		map<string, int>::iterator table_it_l = layer_seq_table.find(free_handles_org_blob_it->second->producers[iter_cur_b_p]);

		int ind_l = table_it_l->second;

		if(ind_l>tmp_free_pro_indx)
			tmp_free_pro_indx=ind_l;
	}

	for(int iter_cur_b_c=0;iter_cur_b_c<free_handles_org_blob_it->second->consumers.size();iter_cur_b_c++){

		map<string, int>::iterator table_it_l = layer_seq_table.find(free_handles_org_blob_it->second->consumers[iter_cur_b_c]);

		int ind_l = table_it_l->second;

		if(ind_l>tmp_free_cons_indx)
			tmp_free_cons_indx=ind_l;


	}

	for(int iter_cur_b_p=0;iter_cur_b_p<curr_blob_it->second->producers.size();iter_cur_b_p++){

		map<string, int>::iterator table_it_l = layer_seq_table.find(curr_blob_it->second->producers[iter_cur_b_p]);

		int ind_l = table_it_l->second;

		if(ind_l>tmp_curr_pro_indx)
			tmp_curr_pro_indx=ind_l;


	}

	for(int iter_cur_b_c=0;iter_cur_b_c<curr_blob_it->second->consumers.size();iter_cur_b_c++){

		map<string, int>::iterator table_it_l = layer_seq_table.find(curr_blob_it->second->consumers[iter_cur_b_c]);

		int ind_l = table_it_l->second;

		if(ind_l>tmp_curr_cons_indx)
			tmp_curr_cons_indx=ind_l;
	}
	/// find max  xlayer indx from free handle consumers vs free handle produces
	if(tmp_free_pro_indx<tmp_free_cons_indx){

		tmp_free_max_indx=tmp_free_cons_indx;
	}
	else
	{
		tmp_free_max_indx=tmp_free_pro_indx;
	}

	/// find max  xlayer indx from curr handle consumers vs curr handle produces

	if(tmp_curr_pro_indx<tmp_curr_cons_indx){

		tmp_curr_max_indx=tmp_curr_cons_indx;
	}
	else
	{
		tmp_curr_max_indx=tmp_curr_pro_indx;
	}

	/// update res value based on the max of free handle cons & pros indx's vs  curr handle cons & pros indx's

	if(tmp_free_max_indx<tmp_curr_max_indx){

		*res=0;

	}else{

		*res=1;
	}

}

void xi_update_buffer_reuse_xlayer_seq(vector < XlayerData > & xlayer_seq,vector < BDegree > &Bdepth,map<string, int> &layer_seq_table,map<string, string> &mem_type_table,XGraph *xgraph,vector < BDegree > &reUseBdepth){


	vector< Handle_depth_info > free_handles;
	BDegree tmp_Bdepth;
	int sw=1;
	map<string,int> reuse_buff_table;
	Handle_depth_info *tmp_handle_info =   new Handle_depth_info();

	// copy the Bdepth structure into another tmp structure(reUseBdepth) and also find the layer type whether sw/hw and create reuse_buff_table map structure.
	for(int iter_bg=0;iter_bg<Bdepth.size();iter_bg++){

		int layer_sw=0;

		for(int iter_bb_c=0;iter_bb_c<Bdepth[iter_bg].blobs_name.size();iter_bb_c++){

			tmp_Bdepth.blobs_name.push_back(Bdepth[iter_bg].blobs_name[iter_bb_c]);
			tmp_Bdepth.blobs_degree = iter_bg;
			map < string, XBlob*>::iterator blob_it = xgraph->blobs.find(Bdepth[iter_bg].blobs_name[iter_bb_c]);
			XBlob* cur_blob = blob_it->second;

			for(int iter_cur_b_p=0;iter_cur_b_p<cur_blob->producers.size();iter_cur_b_p++){

				map<string, int>::iterator table_it_l = layer_seq_table.find(cur_blob->producers[iter_cur_b_p]);
				int ind_l = table_it_l->second;

				if(xlayer_seq[ind_l].layer_type.compare("software")==0)
				{
					layer_sw=1;
				}
			}

			for(int iter_cur_b_c=0;iter_cur_b_c<cur_blob->consumers.size();iter_cur_b_c++){

				map<string, int>::iterator table_it_l = layer_seq_table.find(cur_blob->consumers[iter_cur_b_c]);

				int ind_l = table_it_l->second;

				if(xlayer_seq[ind_l].layer_type.compare("software")==0){

					layer_sw=1;
				}
			}

			reuse_buff_table[Bdepth[iter_bg].blobs_name[iter_bb_c]]=layer_sw;
		}
		reUseBdepth.push_back(Bdepth[iter_bg]);

	}
	// interate over blobs depth

	//
	for(int iter_bg=0;iter_bg<Bdepth.size();iter_bg++){

		if(iter_bg>1){

			// iterate over blobs in each depth and fill blobs name to temp queue

			for(int iter_b=0;iter_b<reUseBdepth[iter_bg-2].blobs_name.size();iter_b++){

				//Handle_depth_info *tmp_handle_info =   new Handle_depth_info();

				if(reUseBdepth[iter_bg-2].blobs_name[iter_b].compare("data")!=0){

					int reuse_buff_type=0;
					map<string, int>::iterator it_reuse_buff = reuse_buff_table.find(reUseBdepth[iter_bg-2].blobs_name[iter_b]);

					if(it_reuse_buff==reuse_buff_table.end()){

						tmp_handle_info->free_handle = reUseBdepth[iter_bg-2].blobs_name[iter_b];
						tmp_handle_info->degree_num=iter_bg-2;
						tmp_handle_info->degree_blob_num=iter_b;
						free_handles.push_back(*tmp_handle_info);

					}
					else{
						reuse_buff_type = it_reuse_buff->second;

						if(reuse_buff_type!=sw){

							tmp_handle_info->free_handle = reUseBdepth[iter_bg-2].blobs_name[iter_b];
							tmp_handle_info->degree_num=iter_bg-2;
							tmp_handle_info->degree_blob_num=iter_b;
							free_handles.push_back(*tmp_handle_info);

						}
					}

				}

			}
#if O_DEBUG
			cout << " filled the blobs name to temp vector done" << endl;
#endif
		}
		// iterate over blobs in each depth
		for(int iter_b=0;iter_b<Bdepth[iter_bg].blobs_name.size();iter_b++){

			int io_blob_flag=0;// ip_op_blobs are same

			string blob_name = Bdepth[iter_bg].blobs_name[iter_b];

			map < string, XBlob*>::iterator blob_it = xgraph->blobs.find(blob_name);
			XBlob* cur_blob = blob_it->second;

			// check the free handles to reuse
			if(!free_handles.empty()){

#if O_DEBUG
				cout << " enter the free_handles loop " << endl;
#endif
				for(int iter_handle=0;iter_handle<free_handles.size();iter_handle++){

					string handle_name  = free_handles[iter_handle].free_handle;

					int handle_degree_num = free_handles[iter_handle].degree_num;
					int handle_blob_num = free_handles[iter_handle].degree_blob_num;
					int res=0;

					xi_cmp_freehandle_layers_indx_and_curr_blob_layers_indx(Bdepth,xlayer_seq,xgraph,layer_seq_table,handle_degree_num,handle_blob_num,iter_bg,iter_b,&res);

					if(res==1)
						continue;

					int flag_handle_not_use=0;
					vector<int> curr_con_layer_ind;
					vector <int> consumers_handle_list;
					map<string, string>::iterator mem_type_it = mem_type_table.find(handle_name);

					string free_handle_mem_type_name = mem_type_it->second;

					map<string, string>::iterator tmp_mem_type_it = mem_type_table.find(blob_name);

					if(free_handle_mem_type_name.compare(tmp_mem_type_it->second)==0){

#if O_DEBUG
						cout << " free_handle and current handle are same mem type " << endl;
#endif
						for(int iter_l_p=0;iter_l_p<cur_blob->producers.size();iter_l_p++){

							map<string, int>::iterator table_it = layer_seq_table.find(cur_blob->producers[iter_l_p]);

							int ind_l = table_it->second;

							for(int iter_ip_b_c=0;iter_ip_b_c<xlayer_seq[ind_l].ip_blob.size();iter_ip_b_c++){

								if(xlayer_seq[ind_l].ip_blob[iter_ip_b_c].handle.compare(handle_name)==0){

#if O_DEBUG
									cout<<"free_handle is same as it current input handle, don't use this handle and look for next handle if handle vector has"<<endl;
#endif
									flag_handle_not_use=1;
									break;
								}

							}

							if(flag_handle_not_use){
#if O_DEBUG
								cout<<"free_handle is same as it current input handle, don't use this handle and break for loop and look for  next handle if handle vector has"<<endl;
#endif

								break;
							}
						}
						if(flag_handle_not_use){
#if O_DEBUG
							cout<<"free_handle is same as it current input handle, don't use this handle and continue for loop and look for next handle if handle vector has"<<endl;
#endif

							continue;

						}else{
#if O_DEBUG
							cout<<"free_handle and it current input handles are not same, so we can use remaining conditions met"<<endl;
#endif
							map < string, XBlob*>::iterator free_handles_org_blob_it = xgraph->blobs.find(Bdepth[handle_degree_num].blobs_name[handle_blob_num]);

							if(free_handles_org_blob_it->second->consumers.size()>0){

#if O_DEBUG
								cout<<"free_handle and it current input handles are not same,enter checking the consumers types condition"<<endl;
#endif

								map<string, int>::iterator table_it = layer_seq_table.find(free_handles_org_blob_it->second->consumers[0]);
								int ind0 = table_it->second;

								for(int iter_l_c=1;iter_l_c<free_handles_org_blob_it->second->consumers.size();iter_l_c++){

									table_it = layer_seq_table.find(free_handles_org_blob_it->second->consumers[iter_l_c]);
									int ind_l = table_it->second;

									//consumers_handle_list.push_back(ind_l);
									if(xlayer_seq[ind0].hw_ops->type.compare(xlayer_seq[ind_l].hw_ops->type)==0)
									{
										curr_con_layer_ind.push_back(ind_l);
									}

								}
								if((curr_con_layer_ind.size()+1)==free_handles_org_blob_it->second->consumers.size())
								{

#if O_DEBUG
									cout<<"all the layers of the free handle consumers types are same"<<endl;
#endif
									for(int iter_cur_b_p=0;iter_cur_b_p<cur_blob->producers.size();iter_cur_b_p++){
										map<string, int>::iterator table_it_l = layer_seq_table.find(cur_blob->producers[iter_cur_b_p]);
										int ind_l = table_it_l->second;

										for(int iter_cur_l_o_c=0;iter_cur_l_o_c<xlayer_seq[ind_l].op_blob.size();iter_cur_l_o_c++){
											if(xlayer_seq[ind_l].op_blob[iter_cur_l_o_c].handle.compare(blob_name)==0)
												xlayer_seq[ind_l].op_blob[iter_cur_l_o_c].handle=handle_name;
										}

									}

									for(int iter_cur_b_c=0;iter_cur_b_c<cur_blob->consumers.size();iter_cur_b_c++){

										map<string, int>::iterator table_it_l = layer_seq_table.find(cur_blob->consumers[iter_cur_b_c]);
										int ind_l = table_it_l->second;


										for(int iter_cur_l_i_c=0;iter_cur_l_i_c<xlayer_seq[ind_l].ip_blob.size();iter_cur_l_i_c++){
											if(xlayer_seq[ind_l].ip_blob[iter_cur_l_i_c].handle.compare(blob_name)==0)
												xlayer_seq[ind_l].ip_blob[iter_cur_l_i_c].handle=handle_name;
										}

									}
#if O_DEBUG
									cout<<" update the free handle in the reUseBdepth vector"<<endl;
#endif

									reUseBdepth[iter_bg].blobs_name[iter_b]=handle_name;
#if O_DEBUG
									cout<<" delete free handles from free_handles vector"<<endl;
#endif
									free_handles.erase(free_handles.begin()+iter_handle);
								}
							}
						}
					}
				}
			}
		}
	}
	delete tmp_handle_info;
}

void xi_find_xlayer_opcode(vector < struct xtract_opcode_num> xlayers_opcode_e,string &layer_type,int &hs_ops_type){

	int flag_layer_type_find=0;
	int iter_lt=0;

	for(iter_lt=0;iter_lt< xlayers_opcode_e.size();iter_lt++){

		if(xlayers_opcode_e[iter_lt].layer_name.compare(layer_type)==0){
			hs_ops_type=xlayers_opcode_e[iter_lt].opcode;
			flag_layer_type_find=1;
			break;
		}else{

			flag_layer_type_find=0;
		}
	}
	if(flag_layer_type_find==1)
	{
#if O_DEBUG
		cout<<"xi_find_xlayer_opcode:Layer is find "<<"layer type:"<<layer_type<<endl;
#endif
	}
	else{
		iter_lt=iter_lt-1;
		ELOG(!(xlayers_opcode_e[iter_lt].layer_name.compare(layer_type)==0),"EO005", "Layer opcode: Layer is not support : "<< layer_type);
	}
}

void xi_update_xlayer_opcode(XGraph *xgraph)
{
	vector < struct xtract_opcode_num> xlayers_opcode;
	xi_get_opcode_xlayer(xlayers_opcode);
	int hs_ops_type=0;
	for(map < string, XLayer*>::iterator it = xgraph->layers.begin();it !=xgraph->layers.end();it++){

		xi_find_xlayer_opcode(xlayers_opcode,it->second->type,hs_ops_type);
		it->second->opcode = hs_ops_type;
	}

}

void XGraphOpt::xi_opt_fuse_bn_conv(XGraph* xgraph_opt,vector <string> &bn_opt_table){

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
					it_layer->second->conv_params->biasPath = it_conv_layer->second->conv_params->biasPath;
					it_layer->second->conv_params->biasDim = it_conv_layer->second->conv_params->biasDim;
					it_layer->second->conv_params->weightsDim = it_conv_layer->second->conv_params->weightsDim;
					it_layer->second->conv_params->weightsPath = it_conv_layer->second->conv_params->weightsPath;
					it_layer->second->wt_bw = it_conv_layer->second->wt_bw;
					it_layer->second->wt_fl = it_conv_layer->second->wt_fl;

					it_layer->second->output_file = it_conv_layer->second->output_file;

					it_layer->second->conv_params->M = it_conv_layer->second->top[conv_top_indx].blob->shape.at(1);

					if(it_conv_layer->second->conv_params->reluflag)
						it_layer->second->conv_params->reluflag = true;

					it_layer->second->op_bw = it_conv_layer->second->op_bw;
					it_layer->second->op_fl = it_conv_layer->second->op_fl;

					it_layer->second->top[0]=it_conv_layer->second->top[conv_top_indx]; // TODO assuming only one top blob for each layer
					it_layer->second->top[0].blob->producers[conv_top_prod_indx]=it_layer->second->name; // TODO assuming only one top blob for each layer
					it_layer->second->opcode = OPCODE_FUSE_BN_CONV;
					xgraph_opt->layers.erase(it_conv_layer);
					xgraph_opt->blobs.erase(it_blob);

				}
			}

		}else{
			if(it_layer->second->top.size()>0){

				ELOG((it_layer->second->top.size()==1),"xi_opt_fuse_bn_conv", "bn Layer has more than one top blobs,its not support right now : "<< it_layer->second->top.size());
			}
		}
	}
}
/*void Abid_software_fuse(XLayer* bn_layer,XLayer* scale_layer,XLayer* Conv_layer){

	cout<<"Abid_software_fuse for conv + bn+scale+relu"<<endl;

}*/

// Function to compare absolute values. 
// Used to find absolute max_value in an array using std::max_element
// TODO : It is a generic function. It should go to some other file, probably xi_funcs.cpp
inline bool xi_compareAbsValues(float a, float b)
{
	return (std::abs(a) < std::abs(b));
}

// This function is same as Trim2FixedPoint() in caffeNetworkParser.cpp
// TODO : It is a generic function. It should go to some other file, probably xi_funcs.cpp
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

void xi_fuseBNScaleConvParams(XLayer* convLayer, XLayer* bnLayer, XLayer* scaleLayer, int mode)
{
    // Load Convolution Data
    std::string conv_weights_path = convLayer->conv_params->weightsPath.at(0);
    std::string conv_bias_path = convLayer->conv_params->biasPath.at(0);
    std::vector<float> conv_weights = READDATA(conv_weights_path);
    std::vector<float> conv_bias = READDATA(conv_bias_path);
    std::vector<int> conv_weights_dim = convLayer->conv_params->weightsDim.at(0);
    std::vector<int> conv_bias_dim = convLayer->conv_params->biasDim.at(0);

    int final_op_bw = convLayer->op_bw;
    int final_op_fl = convLayer->op_fl;
    
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
                    std::string mean_path = bnLayer->batchnorm_params->meanPath.at(0);
                    std::string variance_path = bnLayer->batchnorm_params->variancePath.at(0);
                    mean = READDATA(mean_path);
                    variance = READDATA(variance_path);
                    eps = bnLayer->batchnorm_params->eps;
                    final_op_bw = bnLayer->op_bw;
                    final_op_fl = bnLayer->op_fl;
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
                    std::string gamma_path = scaleLayer->scale_params->gammaPath.at(0);
                    std::string beta_path = scaleLayer->scale_params->betaPath.at(0);
                    gamma = READDATA(gamma_path);
                    beta = READDATA(beta_path);
                    final_op_bw = scaleLayer->op_bw;
                    final_op_fl = scaleLayer->op_fl;
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

                const int CONV_BW = convLayer->wt_bw;
                int MAX_VALUE = std::pow(2, CONV_BW-1);
                int int_bits, fl_bits;
                float log2_val=0;
                vector<float>::iterator abs_max_iter = std::max_element(conv_weights.begin(), conv_weights.end(), xi_compareAbsValues);
                float abs_max_val = std::abs(*abs_max_iter);
            //    ASSERT( abs_max_val <= MAX_VALUE, EO99, 
            //            "Insufficient bit-precision for weights in " << convLayer->name << ": " << abs_max_val << " v/s " << MAX_VALUE)

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
                //Trim2FixedPoint2(conv_weights, CONV_BW, fl_bits, ROUND_NEAREST);
                //Trim2FixedPoint2(conv_bias, final_op_bw, final_op_fl, ROUND_NEAREST);
                convLayer->op_bw = final_op_bw;
                convLayer->op_fl = final_op_fl;
                convLayer->wt_fl = fl_bits;

                // cerr << "DEBUG : " << convLayer->name << " : " << abs_max_val << " , " 
                //     << log2_val << " " << std::abs(log2_val) << " " << std::abs(*abs_max_iter) << " " << fl_bits << endl; 

                break;
            }
        default:
            {
                ELOG(true, EX022, "Unrecognized mode in Fusion")
            }
    }

    // Dump the new weights and Bias to same files
    SAVEDATA(conv_weights, conv_weights_path);
    SAVEDATA(conv_bias, conv_bias_path);

}

void XGraphOpt::xi_opt_conv_bn_scale(XGraph* xgraph_opt){

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

							xi_fuseBNScaleConvParams(it->second,bn_layer,scale_layer,0);
							it->second->conv_params->reluflag=1;
							it->second->conv_params->has_software_fuse = 1;
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

								xi_fuseBNScaleConvParams(it->second,bn_layer,scale_layer,0);
								xtract_fuse_flag=1;
								it->second->conv_params->has_software_fuse = 1;
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

										if(it_t_s_blob->second->consumers.size()==1){

											map < string, XLayer*>::iterator it_sc_c_layer = xgraph_opt->layers.find(it_t_s_blob->second->consumers[0]);

											if(it_sc_c_layer->second->type.compare(*relu_layer_type)==0){
												relu_layer=it_sc_c_layer->second;
												layer_in_place.push_back(it_sc_c_layer->second->name);

												//Abid_software_fuse(bn_layer,scale_layer,it->second);
												xi_fuseBNScaleConvParams(it->second,bn_layer,scale_layer,0);
												it->second->top[0]=it_sc_c_layer->second->top[0];
												it->second->top[0].blob->producers[0]=it->second->name;
												it->second->conv_params->reluflag=1;
												it->second->conv_params->has_software_fuse = 1;
												it->second->output_file = it_sc_c_layer->second->output_file;

												for(int iter_inc=0;iter_inc<layer_in_place.size();iter_inc++){

													map < string, XLayer*>::iterator it_d_layer = xgraph_opt->layers.find(layer_in_place[iter_inc]);
													xgraph_opt->layers.erase(it_d_layer);
												}
												
												xgraph_opt->blobs.erase(it_t_blob);
												
												xgraph_opt->blobs.erase(it_t_bn_blob);
												xgraph_opt->blobs.erase(it_t_s_blob);
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
}

void XGraphOpt::xi_opt_crelu(XGraph *xgraph_opt){

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
}

void xlayer_sequence_generator(vector < XlayerData > & xlayer_seq,XGraph *xgraph){

	XGraphOpt xgraphopt;
	map<string, int> layer_seq_table;
	map<string, string> layer_type_table;

	map<string, string> mem_type_table;
	vector < BDegree > Bdepth,reUseBdepth;
	vector<string> bn_opt_table;
	checkGraphInsanity(xgraph);

	xgraphopt.xi_opt_priorbox(xgraph);
#if O_DEBUG
	cout<<"xgraphopt.xi_opt_priorbox(&graph) done" << endl;
#endif

#if O_DEBUG
	cout<<"xgraphopt.xi_opt_relu(&graph) done" << endl;
#endif
	xgraphopt.xi_opt_dropout(xgraph);
#if O_DEBUG
	cout<<"xgraphopt.xi_opt_dropout(&graph) done" << endl;
#endif



	xgraphopt.xi_opt_reshape(xgraph);
	xgraphopt.xi_opt_flatten(xgraph);



#if O_DEBUG
	cout << " xi_opt_concat optimization done" << endl;
#endif



	xgraphopt.xi_opt_deconv(xgraph);
#if O_DEBUG
	cout << " xi_opt_deconv optimization done" << endl;
#endif


	xi_update_xlayer_opcode(xgraph);
	xgraphopt.xi_opt_crelu(xgraph);
	xgraphopt.xi_opt_conv_bn_scale(xgraph);

	xgraphopt.xi_opt_batchNorm(xgraph,bn_opt_table);
	xgraphopt.xi_opt_eltwise(xgraph);

	xgraphopt.xi_opt_fuse_bn_conv(xgraph,bn_opt_table);



	//xgraphopt.xi_opt_eltwise(xgraph);

	xgraphopt.xi_opt_relu(xgraph);
	xgraphopt.xi_opt_lrn(xgraph);
	xgraphopt.xi_opt_concat(xgraph);

	xgraphopt.xi_opt_find_max_q_factor(xgraph);
	xi_find_blobs_degree(xgraph,Bdepth);

#if O_DEBUG
	xi_print_blobs_degre(Bdepth);

#endif

	xi_create_seq_num_table(xlayer_seq,layer_seq_table,layer_type_table,Bdepth,xgraph);
	xi_update_prev_next_layer_info(xlayer_seq,layer_seq_table,xgraph);
	xi_update_io_mem_type(xlayer_seq,xgraph,Bdepth,layer_type_table,mem_type_table);
	xi_update_buffer_reuse_xlayer_seq(xlayer_seq,Bdepth,layer_seq_table,mem_type_table,xgraph,reUseBdepth);
	
}


void checkGraphInsanity(XGraph* graph)
{
	//  #. Check if input shape, resize shape & mean shape are OK.
	int nchannels = graph->input_blob.blob->shape.at(1);
	int input_height = graph->input_blob.blob->shape.at(2);
	int input_width = graph->input_blob.blob->shape.at(3);

	int resize_height = graph->transform_params.resize_height;
	int resize_width = graph->transform_params.resize_width;

	// If mean data has only one value or no value, then there is no issue at all.
	if(graph->meanData.size() > 1)
	{
		// If mean has same number of elements as number of channels, then also no issue
		if(graph->meanData.size() != nchannels)
		{
			// If resize shape is mentioned, number of elements in resized image and mean should be same
			// If not specified, number of elements in input image and mean should be same.
			if((resize_height > 0) && (resize_width > 0))
			{
				ELOG(   (nchannels * resize_height * resize_width != graph->meanData.size()),
						EO006, "Number of elements in the mean data doesn't match total pixels as per resize dimensions : "
						<< nchannels*resize_height*resize_width << " v/s " << graph->meanData.size() );
			}
			else
			{
				ELOG(   (nchannels * input_height * input_width != graph->meanData.size()),
						EO007, "Number of elements in the mean data doesn't match total pixels as per input dimensions : "
						<< nchannels*input_height*input_width << " v/s " << graph->meanData.size() );
			}
		}
	}
}


void XGraphOpt::xi_opt_priorbox(XGraph* graph)
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
		string pboxFileName = graph->saveDir + nms_layer->name + "_weights";
		string varFileName = graph->saveDir + nms_layer->name + "_bias";

#if GENERATE_CONSTDATA
		// Open files to dump the priorbox
		//string pboxFileName = nms_layer->uname + "_pbox.txt";

		std::ofstream pbox_output_file(pboxFileName.c_str());
		if(!pbox_output_file)
		{
			cerr << "[ERROR] Couldn't create " << pboxFileName << endl;
			exit(-1);
		}
		std::ostream_iterator<float> pbox_output_iterator(pbox_output_file, "\n");

		// Open file to dump variance
		//string varFileName = nms_layer->uname + "_var.txt";
		std::ofstream var_output_file(varFileName.c_str());
		if(!var_output_file)
		{
			cerr << "[ERROR] Couldn't create " << varFileName << endl;
			exit(-1);
		}
		std::ostream_iterator<float> var_output_iterator(var_output_file, "\n");
#endif

		// Get the parent layers of prior_box_concat layer.
		vector<string> pb_layers = graph->getParentLayers(nms_parent_layers.at(2));
		vector<int> shape(3, 1);
		shape[2] = 0;                   // Initialize last dim for accumulation

		vector<string> blobsToBeDeleted;

#if GENERATE_CONSTDATA
		// #. Dump its data to corresponding txt files
		cerr << "[IG003] Saving " << pboxFileName << endl;
		cerr << "[IG004] Saving " << varFileName << endl;
#endif
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

#if GENERATE_CONSTDATA
			// #. Dump its data to corresponding txt files
			std::copy(pb_layer->priorbox_params->pbox.begin(), pb_layer->priorbox_params->pbox.begin() + dim, pbox_output_iterator);
			std::copy(pb_layer->priorbox_params->pbox.begin()+dim, pb_layer->priorbox_params->pbox.end(), var_output_iterator);
#endif

			// #. delete layer from the graph
			vector<string> blobsToDelete = graph->deleteLayerFromGraph(pb_layer->name);
			std::copy(blobsToDelete.begin(), blobsToDelete.end(), std::back_inserter(blobsToBeDeleted));
		}

		nms_layer->nms_params->pboxFile = pboxFileName;
		nms_layer->nms_params->varFile = varFileName;
		nms_layer->nms_params->pboxShape = shape;
		nms_layer->nms_params->varShape = shape;

#if GENERATE_CONSTDATA
		pbox_output_file.close();
		var_output_file.close();
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
void XGraphOpt::xi_opt_batchNorm(XGraph *xgraph_opt,vector<string> &bn_opt_table){

	string *batchNorm_layer_type = new string("BatchNorm");
	string *scale_layer_type = new string("Scale");
	string *relu_layer_type = new string("ReLU");
	vector<string> names_bt_norm;
	int batchnorm_cnt=0;
	vector<string> names_layers_del;

	for(map < string, XLayer*>::iterator it = xgraph_opt->layers.begin();it !=xgraph_opt->layers.end();it++){

		if(it->second->type.compare(*batchNorm_layer_type)==0){
			string relu_output_file,scale_output_file;
			if(it->second->batchnorm_params->global_stats==true){ //TODO currently resnet 50/34 we see global_stats always true... we may need to implement false case as well.

				if(it->second->batchnorm_params->inPlace){

					int relu_flag_enabled =0;
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

								string uniqname = xgraph_opt->getUniqueBlobName();
								string username = scale_layer_name;
								XBlob* tmp_blob = new XBlob(username, uniqname);
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
								dst0->conv_params->extra_reluflag = false;
								dst0->conv_params->scale_gammaDim = layer_it->second->scale_params->gammaDim;
								dst0->conv_params->scale_gammaPath = layer_it->second->scale_params->gammaPath;
								dst0->conv_params->scale_betaDim = layer_it->second->scale_params->betaDim;
								dst0->conv_params->scale_betaPath = layer_it->second->scale_params->betaPath;
								//cout<<"Opcode 10"<< layer_it->second->scale_params->gammaPath[0]<<endl;
								//cout<<"Opcode 10"<< layer_it->second->scale_params->betaPath[0]<<endl;
								dst0->scale_beta_bw=layer_it->second->scale_beta_bw;
								dst0->scale_beta_fl=layer_it->second->scale_beta_fl;
								dst0->scale_gamma_bw= layer_it->second->scale_gamma_bw;
								dst0->scale_gamma_fl= layer_it->second->scale_gamma_fl;
								dst0->op_bw = layer_it->second->op_bw;
								dst0->op_fl = layer_it->second->op_fl;
								dst0->scale_gamma_by_std_bw = layer_it->second->scale_gamma_by_std_bw;
								dst0->scale_gamma_by_std_fl = layer_it->second->scale_gamma_by_std_fl;

								//map < string, XBlob*>::iterator blob_it = xgraph_opt->blobs.find(it->second->top[0].blob->name);
								map < string, XLayer*>::iterator relu_layer_it;
								map < string, XLayer*>::iterator scale_layer_it;

								for(int iter_prod=0;iter_prod<blob_it->second->producers.size();iter_prod++){

									map < string, XLayer*>::iterator prod_layer_it = xgraph_opt->layers.find(blob_it->second->producers[iter_prod]);

									if(prod_layer_it->second->type.compare(*relu_layer_type)==0){

										if(prod_layer_it->second->relu_params->inPlace==true){

											relu_layer_it = prod_layer_it;
											relu_flag_enabled=1;
											//names_layers_del.push_back(prod_layer_it->second->name);
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
											//names_layers_del.push_back(prod_layer_it->second->name);
											blob_it->second->producers.erase(blob_it->second->producers.begin() + iter_prod);
											iter_prod=iter_prod-2;
											if(iter_prod==-2)
												iter_prod=-1;
										}
									}
								}

								map < string, XLayer*>::iterator bn_layer_it=xgraph_opt->layers.find(tmp_batchnorm_name);

								dst0->conv_params->batchnorm_meanDim = bn_layer_it->second->batchnorm_params->meanDim;
								dst0->conv_params->batchnorm_meanPath = bn_layer_it->second->batchnorm_params->meanPath;
								dst0->conv_params->batchnorm_varianceDim = bn_layer_it->second->batchnorm_params->varianceDim;
								dst0->conv_params->batchnorm_variancePath = bn_layer_it->second->batchnorm_params->variancePath;
								dst0->conv_params->batchnorm_eps = bn_layer_it->second->batchnorm_params->eps;
								dst0->ip_bw = bn_layer_it->second->ip_bw;
								dst0->ip_fl = bn_layer_it->second->ip_fl;
								dst0->bn_mean_bw = bn_layer_it->second->bn_mean_bw;
								dst0->bn_mean_fl = bn_layer_it->second->bn_mean_fl;
								dst0->bn_variance_bw = bn_layer_it->second->bn_variance_bw;
								dst0->bn_variance_fl = bn_layer_it->second->bn_variance_fl;

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
										//	blob_it->second->consumers.erase(blob_it->second->consumers.begin()+ iter_con_c);
										blob_it->second->consumers[iter_con_c]=scale_layer_name;
										//iter_con_c--;
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
								tmp_blob->producers.push_back(dst0->name);

								if(relu_flag_enabled){

									dst0->output_file=relu_output_file;
									xgraph_opt->layers.erase(relu_layer_it);

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
													dst0->conv_params->scale_gammaPath = it_cons_layer->second->scale_params->gammaPath;
													dst0->conv_params->scale_betaDim = it_cons_layer->second->scale_params->betaDim;
													dst0->conv_params->scale_betaPath = it_cons_layer->second->scale_params->betaPath;

													dst0->scale_beta_bw=it_cons_layer->second->scale_beta_bw;
													dst0->scale_beta_fl=it_cons_layer->second->scale_beta_fl;
													dst0->scale_gamma_bw= it_cons_layer->second->scale_gamma_bw;
													dst0->scale_gamma_fl= it_cons_layer->second->scale_gamma_fl;
													dst0->scale_gamma_by_std_bw = it_cons_layer->second->scale_gamma_by_std_bw;
													dst0->scale_gamma_by_std_fl = it_cons_layer->second->scale_gamma_by_std_fl;

													dst0->op_bw = it_cons_layer->second->op_bw;
													dst0->op_fl = it_cons_layer->second->op_fl;

													dst0->conv_params->batchnorm_meanDim = it->second->batchnorm_params->meanDim;
													dst0->conv_params->batchnorm_meanPath = it->second->batchnorm_params->meanPath;
													dst0->conv_params->batchnorm_varianceDim = it->second->batchnorm_params->varianceDim;
													dst0->conv_params->batchnorm_variancePath = it->second->batchnorm_params->variancePath;
													dst0->conv_params->batchnorm_eps = it->second->batchnorm_params->eps;
													dst0->ip_bw = it->second->ip_bw;
													dst0->ip_fl = it->second->ip_fl;
													dst0->bn_mean_bw = it->second->bn_mean_bw;
													dst0->bn_mean_fl = it->second->bn_mean_fl;
													dst0->bn_variance_bw = it->second->bn_variance_bw;
													dst0->bn_variance_fl = it->second->bn_variance_fl;
													dst0->output_file = scale_top_cons_layer->second->output_file;
													dst0->opcode = OPCODE_BN;
													dst0->bottom.push_back(it->second->bottom[0]);
													it->second->bottom[0].blob->consumers[0] = dst0->name;

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
													dst0->conv_params->scale_gammaPath = it_cons_layer->second->scale_params->gammaPath;
													dst0->conv_params->scale_betaDim = it_cons_layer->second->scale_params->betaDim;
													dst0->conv_params->scale_betaPath = it_cons_layer->second->scale_params->betaPath;

													dst0->scale_beta_bw=it_cons_layer->second->scale_beta_bw;
													dst0->scale_beta_fl=it_cons_layer->second->scale_beta_fl;
													dst0->scale_gamma_bw= it_cons_layer->second->scale_gamma_bw;
													dst0->scale_gamma_fl= it_cons_layer->second->scale_gamma_fl;
													dst0->scale_gamma_by_std_bw = it_cons_layer->second->scale_gamma_by_std_bw;
													dst0->scale_gamma_by_std_fl = it_cons_layer->second->scale_gamma_by_std_fl;

													dst0->op_bw = it_cons_layer->second->op_bw;
													dst0->op_fl = it_cons_layer->second->op_fl;

													dst0->conv_params->batchnorm_meanDim = it->second->batchnorm_params->meanDim;
													dst0->conv_params->batchnorm_meanPath = it->second->batchnorm_params->meanPath;
													dst0->conv_params->batchnorm_varianceDim = it->second->batchnorm_params->varianceDim;
													dst0->conv_params->batchnorm_variancePath = it->second->batchnorm_params->variancePath;




													dst0->conv_params->batchnorm_eps = it->second->batchnorm_params->eps;
													dst0->ip_bw = it->second->ip_bw;
													dst0->ip_fl = it->second->ip_fl;
													dst0->bn_mean_bw = it->second->bn_mean_bw;
													dst0->bn_mean_fl = it->second->bn_mean_fl;
													dst0->bn_variance_bw = it->second->bn_variance_bw;
													dst0->bn_variance_fl = it->second->bn_variance_fl;
													dst0->output_file = it_cons_layer->second->output_file;
													dst0->opcode = OPCODE_BN;
													dst0->bottom.push_back(it->second->bottom[0]);
													it->second->bottom[0].blob->consumers[0] = dst0->name;

													map < string, XBlob*>::iterator bn_top_blob_it = xgraph_opt->blobs.find(it->second->top[0].blob->name);
													xgraph_opt->blobs.erase(bn_top_blob_it);
													xgraph_opt->layers.erase(it);

													map < string, XBlob*>::iterator scale_top_blob_it = xgraph_opt->blobs.find(it_cons_layer->second->top[0].blob->name);
													xgraph_opt->blobs.erase(scale_top_blob_it);
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
	}
}


void XGraphOpt::xi_opt_scale(XGraph *xgraph_opt){

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

				string uniqname = xgraph_opt->getUniqueBlobName();
				string username = scale_layer_name;
				XBlob* tmp_blob = new XBlob(username, uniqname);
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
				dst0->conv_params->scale_gammaPath = it->second->scale_params->gammaPath;
				dst0->conv_params->scale_betaDim = it->second->scale_params->betaDim;
				dst0->conv_params->scale_betaPath = it->second->scale_params->betaPath;


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
				dst0->conv_params->batchnorm_meanPath = bn_layer_it->second->batchnorm_params->meanPath;
				dst0->conv_params->batchnorm_varianceDim = bn_layer_it->second->batchnorm_params->varianceDim;
				dst0->conv_params->batchnorm_variancePath = bn_layer_it->second->batchnorm_params->variancePath;
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
}



void XGraphOpt::xi_opt_eltwise(XGraph *xgraph_opt){

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

			dst0->opcode = OPCODE_ELTWISE;
			xgraph_opt->layers.erase(it);
			eltwise_cnt++;
		}
	}
}

void XGraphOpt::xi_opt_relu(XGraph *xgraph_opt){

	string *relu_layer_type = new string("ReLU");
	string *fc_layer_type = new string("InnerProduct");
	string *conv_layer_type = new string("Convolution");
	//string *eltwise_layer_type = new string("Eltwise");
#if O_DEBUG
	cout<< "[O_DEBUG] xi_opt_relu: blobs loop start" << endl;
#endif
	for(map < string, XBlob*>::iterator it = xgraph_opt->blobs.begin();it !=xgraph_opt->blobs.end();){

		int no_inplace=0;
		for(int index1=0; index1 < it->second->consumers.size();index1++){

#if O_DEBUG
			cout<< "[O_DEBUG] xi_opt_relu: specific blob consumers loop start" << endl;
#endif
			string tmp_consumer = it->second->consumers[index1];
			vector< string > vec_consumer =  it->second->consumers;

			map < string, XLayer* > ::iterator consumer_layer = xgraph_opt->layers.find(tmp_consumer);

			if(consumer_layer != xgraph_opt->layers.end()){

				if (consumer_layer->second->type.compare(*relu_layer_type)==0){

					if(consumer_layer->second->relu_params->inPlace){

						for(int producers_it =0;producers_it<it->second->producers.size(); producers_it++){

#if O_DEBUG
							cout<< "[O_DEBUG] xi_opt_relu: find the blob consumer is relu and enabling the relu flag it producer layer " << endl;
#endif
							string tmp_producer = it->second->producers[producers_it];
							map < string, XLayer* > ::iterator producer_layer = xgraph_opt->layers.find(tmp_producer);
							string producer_type = producer_layer->second->type;
							if(producer_type.compare(*relu_layer_type)!=0){

								if(producer_layer->second->type.compare(*conv_layer_type)==0)
									producer_layer->second->conv_params->reluflag =1;
								else if (producer_layer->second->type.compare(*fc_layer_type)==0)
									producer_layer->second->fc_params->reluflag=1;
							}
							else {
								it->second->producers.erase(it->second->producers.begin() + producers_it);
							}
						}
						xgraph_opt->layers.erase(consumer_layer);
						it->second->consumers.erase(it->second->consumers.begin()+ index1);
					}
					else{

						for(int producers_it =0;producers_it<it->second->producers.size(); producers_it++){

#if O_DEBUG
							cout<< "[O_DEBUG] xi_opt_relu: find the blob consumer is relu and enabling the relu flag it producer layer " << endl;
#endif
							string tmp_producer = it->second->producers[producers_it];

							map < string, XLayer* > ::iterator producer_layer = xgraph_opt->layers.find(tmp_producer);

							string producer_type = producer_layer->second->type;
							if(producer_type.compare(*relu_layer_type)!=0){

								if(producer_layer->second->type.compare(*conv_layer_type)==0)
									producer_layer->second->conv_params->reluflag =1;
								else if (producer_layer->second->type.compare(*fc_layer_type)==0)
									producer_layer->second->fc_params->reluflag=1;

								XBlob* relu_top_blob = consumer_layer->second->top[0].blob;

								producer_layer->second->top[0] = consumer_layer->second->top[0];
								relu_top_blob->producers.erase(relu_top_blob->producers.begin() + 0);
								relu_top_blob->producers.push_back(tmp_producer);

							}
							else {
								it->second->producers.erase(it->second->producers.begin() + producers_it);
							}
						}
						map < string, XBlob*>::iterator relu_bottom = it;
						no_inplace=1;
						it=it++;
						xgraph_opt->layers.erase(consumer_layer);
						xgraph_opt->blobs.erase(relu_bottom);
					}
				}
			}

		}

		if(no_inplace!=1)
			it++;
	}
	delete relu_layer_type;
	delete fc_layer_type;
	delete conv_layer_type;
}

void XGraphOpt::xi_opt_dropout(XGraph *xgraph_opt){

	string *dropout_layer_type = new string("Dropout");

	for(map < string, XBlob*>::iterator it = xgraph_opt->blobs.begin();it !=xgraph_opt->blobs.end();it++){

		for(int index1=0;index1 < it->second->consumers.size();index1++){

			string tmp_consumer = it->second->consumers[index1];
			vector< string > vec_consumer =  it->second->consumers;

			map < string, XLayer* >::iterator consumer_layer = xgraph_opt->layers.find(tmp_consumer);

			if(consumer_layer != xgraph_opt->layers.end()){

				if (consumer_layer->second->type.compare(*dropout_layer_type)==0){
#if O_DEBUG
					cout << "find the Dropout layer" <<endl;
#endif
					for(int producers_it =0;producers_it<it->second->producers.size(); producers_it++){

						map < string, XLayer* >::iterator producer_layer = xgraph_opt->layers.find(it->second->producers[producers_it]);

						if (producer_layer->second->type.compare(*dropout_layer_type)==0)
						{
							it->second->producers.erase(it->second->producers.begin()+ producers_it);
						}
					}
					xgraph_opt->layers.erase(consumer_layer);
					it->second->consumers.erase(it->second->consumers.begin()+ index1);
				}
			}
		}
		for(int index1=0;index1 < it->second->producers.size();index1++){

			string tmp_producer = it->second->producers[index1];
			vector< string > vec_producer =  it->second->producers;

			map < string, XLayer* >::iterator producer_layer = xgraph_opt->layers.find(tmp_producer);

			if(producer_layer != xgraph_opt->layers.end()){

				if (producer_layer->second->type.compare(*dropout_layer_type)==0){
					//	cout << "find the Dropout layer" <<endl;
					xgraph_opt->layers.erase(producer_layer);
					it->second->producers.erase(it->second->producers.begin() + index1);
				}
			}
		}
	}

	delete dropout_layer_type;
}

void XGraphOpt::xi_opt_concat(XGraph *xgraph_opt){

	string *concat_layer_type = new string("Concat");

	string *concat_layer_name = new string();
	int producers_id =0;

#if O_DEBUG
	cout<< "[O_DEBUG] xi_opt_concat: layers loop start" << endl;
#endif
	for(map < string, XLayer* >::iterator it = xgraph_opt->layers.begin();it != xgraph_opt->layers.end(); it++)
	{

		//auto concat_layer = xgraph_opt->layers.find( it->second->name);

		if(it->second->type.compare(*concat_layer_type)==0){

#if O_DEBUG
			cout<< "[O_DEBUG] xi_opt_concat: find the layer type as concat " << endl;
#endif
			*concat_layer_name = it->second->name;
#if O_DEBUG
			cout << "Concat found" <<endl;
#endif
			string concat_top_name = it->second->top[0].blob->name; // TODO we need think and add for multiple top scenarios

#if O_DEBUG
			cout<< "[O_DEBUG] xi_opt_concat: concat layer is produces for top blob and clearing it first " << endl;
#endif
			map < string, XBlob*>::iterator top_blob = xgraph_opt->blobs.find(concat_top_name);

			top_blob->second->producers.erase(top_blob->second->producers.begin() + 0); // concat layer is produces for top blob and clearing it first

			for(int l_bottoms=0;l_bottoms<it->second->bottom.size();l_bottoms++){

				string bottom_name = it->second->bottom[l_bottoms].blob->name;

				map < string, XBlob*>::iterator bottom_blob = xgraph_opt->blobs.find(bottom_name);

				if(bottom_blob->second->producers.size()== 1){ // TODO we need think and add for multiple producers scenarios

					string tmp_producer = bottom_blob->second->producers[0];
					map < string, XLayer* >::iterator producer_layer = xgraph_opt->layers.find(tmp_producer);
					producer_layer->second->top[0].blob = it->second->top[0].blob;
					producer_layer->second->top[0].id = l_bottoms;
					it->second->top[0].blob->producers.push_back(tmp_producer);

				}
#if O_DEBUG
				cout<< "[O_DEBUG] xi_opt_concat: clearing concant layer bottom blobs one by one " << endl;
#endif
				xgraph_opt->blobs.erase(bottom_blob);
			}
		}
		map < string, XLayer* >::iterator concat_layer = xgraph_opt->layers.find( *concat_layer_name);
		if(concat_layer!=xgraph_opt->layers.end())
			xgraph_opt->layers.erase(concat_layer);
	}

	delete concat_layer_type;
	delete concat_layer_name;
}

void XGraphOpt::xi_opt_deconv(XGraph *xgraph_opt){

	string *deconv_layer_type = new string("Deconvolution");
	string *softmax_layer_type = new string("Softmax");
	string *argmax_layer_type = new string("Argmax");
	//string *argmax_layer_type = new string("Softmax");

	string *deconv_layer_name = new string();
	string *crop_layer_type = new string("Crop");
	string *softmax_name = new string();
	XLayer * deconv_layer_obj = new XLayer();

	int deconv_opt_flag = 0;

	for(map < string, XLayer* > ::iterator it = xgraph_opt->layers.begin();it != xgraph_opt->layers.end(); it++)
	{

		if(it->second->type.compare(*deconv_layer_type)==0){


#if O_DEBUG
			cout<< "[O_DEBUG] xi_opt_deconv: checking deconv_layer_type " << endl;
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
	delete deconv_layer_name;
	delete crop_layer_type;
	delete softmax_name;
	delete deconv_layer_obj;
}
