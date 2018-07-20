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

#include "xtract_utility.hpp"


int compare_pack_info(vector<packinfo> &info_1,vector<packinfo> &info_2){

	if(info_1.size()==info_2.size()){

		for(int iter_in=0;iter_in<info_1.size();iter_in++){

			if(info_1[iter_in].dim==info_2[iter_in].dim){
				if(info_1[iter_in].mem_contiguous==info_2[iter_in].mem_contiguous){
					if(info_1[iter_in].mod_div_factor==info_2[iter_in].mod_div_factor){
						if(info_1[iter_in].sep==info_2[iter_in].sep){
							return 1;
						}else{
							return 0;
						}
					}else{
						return 0;
					}
				}else{
					return 0;
				}
			}else{
				return 0;
			}
		}
	}else{
		cerr<<"[INFOx] Packinfo fields are not matching"<<endl;
	}
}

int pack_info_type(Port_info_s &current_in_port,Port_info_s &prev_out_port,kernelInfo& opt_descriptor,XGraph *xgraph,int &pack_fl,string &t_prods_name,string &t_cons_name,string &quantization_scheme){


	string in_layer_name,in_layer_type;
	string output_layer_name,out_layer_type;
	int input_port_flag=0,output_port_flag=0;
	vector<string> current_graph_layer;

	for(map < string, XLayer*>::iterator it=xgraph->layers.begin();it!=xgraph->layers.end();it++){
		current_graph_layer.push_back(it->second->type);
	}
	map < string, XLayer*>::iterator it_cons=xgraph->layers.find(t_cons_name);
	map < string, XLayer*>::iterator it_prods=xgraph->layers.find(t_prods_name);

	for(map<opcode_num_e, Opcode_info> ::iterator it_opt=opt_descriptor.opcode_info_map.begin();it_opt!=opt_descriptor.opcode_info_map.end();it_opt++){

		io_type_e tmp_type;
		Port_info prot_defualt;
		input_port_flag=0;
		if(it_cons->second->type.compare(it_opt->second.mega_module.module_name)!=0)
			continue;
		std::vector<string>::iterator it_str=std::find(current_graph_layer.begin(),current_graph_layer.end(),it_opt->second.mega_module.module_name);

		if(it_str==current_graph_layer.end())
		{
			continue;
		}

		for(int iter_pv=0;iter_pv < it_opt->second.port_vec.size();iter_pv++){

			if(it_opt->second.port_vec[iter_pv].port_enum==XI_INPUT){
				input_port_flag=0;
				for(int it_info_indx=0;it_info_indx<it_opt->second.port_vec[iter_pv].pack_info_vec.size();it_info_indx++){

					if(it_opt->second.port_vec[iter_pv].pack_info_vec[it_info_indx].dim==current_in_port.pack_info_vec[it_info_indx].dim){
						if(it_opt->second.port_vec[iter_pv].pack_info_vec[it_info_indx].mem_contiguous==current_in_port.pack_info_vec[it_info_indx].mem_contiguous){
							if(it_opt->second.port_vec[iter_pv].pack_info_vec[it_info_indx].mod_div_factor==current_in_port.pack_info_vec[it_info_indx].mod_div_factor){
								if(it_opt->second.port_vec[iter_pv].pack_info_vec[it_info_indx].sep==current_in_port.pack_info_vec[it_info_indx].sep){
									input_port_flag++;
									if(input_port_flag==it_opt->second.port_vec[iter_pv].pack_info_vec.size()){
										in_layer_name=it_opt->second.mega_module.module_name;
									}
								}
							}
						}
					}
				}
			}
		}

		for(map<opcode_num_e, Opcode_info> ::iterator it_opt=opt_descriptor.opcode_info_map.begin();it_opt!=opt_descriptor.opcode_info_map.end();it_opt++){

			io_type_e tmp_type;
			Port_info prot_defualt;
			if(it_prods->second->type.compare(it_opt->second.mega_module.module_name)!=0)
				continue;
			std::vector<string>::iterator it_str=std::find(current_graph_layer.begin(),current_graph_layer.end(),it_opt->second.mega_module.module_name);

			if(it_str==current_graph_layer.end())
			{
				continue;
			}

			for(int iter_pv=0;iter_pv < it_opt->second.port_vec.size();iter_pv++){

				if(it_opt->second.port_vec[iter_pv].port_enum==XI_OUTPUT){
					output_port_flag=0;
					for(int it_info_indx=0;it_info_indx<it_opt->second.port_vec[iter_pv].pack_info_vec.size();it_info_indx++){

						if(it_opt->second.port_vec[iter_pv].pack_info_vec[it_info_indx].dim==prev_out_port.pack_info_vec[it_info_indx].dim){
							if(it_opt->second.port_vec[iter_pv].pack_info_vec[it_info_indx].mem_contiguous==prev_out_port.pack_info_vec[it_info_indx].mem_contiguous){
								if(it_opt->second.port_vec[iter_pv].pack_info_vec[it_info_indx].mod_div_factor==prev_out_port.pack_info_vec[it_info_indx].mod_div_factor){
									if(it_opt->second.port_vec[iter_pv].pack_info_vec[it_info_indx].sep==prev_out_port.pack_info_vec[it_info_indx].sep){
										output_port_flag++;
										if(output_port_flag==it_opt->second.port_vec[iter_pv].pack_info_vec.size())
											output_layer_name=it_opt->second.mega_module.module_name;
									}
								}
							}
						}
					}
				}
			}
		}

		int tmp_op_fl=0,tmp_in_fl=0;
		if(in_layer_name.compare("XCustom")==0){

			if(output_layer_name.compare("Convolution")==0){

				if((it_prods->second->opcode!=OPCODE_AVRPOOL2CONV)&&(it_prods->second->opcode!=OPCODE_POOL2CONV)){
					pack_fl=it_prods->second->op_fl;
					quantization_scheme= it_prods->second->quantization_scheme;
				}
				else{
					if(it_prods->second->opcode==OPCODE_AVRPOOL2CONV){
						pack_fl=it_prods->second->conv_params->op_fl_3d;
						quantization_scheme= it_prods->second->quantization_scheme;
					}
					else
					{
						map < string, XLayer*>::iterator tmp_prods = xgraph->layers.find(it_prods->second->bottom[0].blob->producers[0]);//TODO Assumed its convolution
						pack_fl=tmp_prods->second->op_fl;
						quantization_scheme = tmp_prods->second->quantization_scheme;
					}
				}
#if SINGLE_IO_PORT==0
				return 0;
#else
				return 5;
#endif
			}else{
				if(output_layer_name.compare("Pooling")==0){

					map < string, XLayer*>::iterator tmp_prods = xgraph->layers.find(it_prods->second->bottom[0].blob->producers[0]);//TODO Assumed its convolution
					pack_fl=tmp_prods->second->op_fl;
					quantization_scheme = tmp_prods->second->quantization_scheme;
					return 0;
				}
			}

		}else if(output_layer_name.compare("XCustom")==0){

			if(in_layer_name.compare("Convolution")==0){

				if((it_cons->second->opcode!=OPCODE_AVRPOOL2CONV)&&(it_cons->second->opcode!=OPCODE_POOL2CONV)){
					pack_fl=it_cons->second->ip_fl;
					quantization_scheme = it_cons->second->quantization_scheme;
				}
				else{
					if(it_cons->second->opcode==OPCODE_AVRPOOL2CONV){
						pack_fl=it_cons->second->conv_params->ip_fl_3d;
						quantization_scheme = it_cons->second->quantization_scheme;
					}
					else{
						if(it_cons->second->top[0].blob->consumers.size()){
							map < string, XLayer*>::iterator tmp_cons_top = xgraph->layers.find(it_cons->second->top[0].blob->consumers[0]);//TODO Assumed its convolution
							pack_fl=tmp_cons_top->second->ip_fl;
							quantization_scheme = tmp_cons_top->second->quantization_scheme;
						}else{
							map < string, XLayer*>::iterator tmp_cons_b_p = xgraph->layers.find(it_cons->second->bottom[0].blob->producers[0]);//TODO Assumed its convolution
							map < string, XLayer*>::iterator tmp_cons_b_p_b_p = xgraph->layers.find(tmp_cons_b_p->second->bottom[0].blob->producers[0]);//TODO Assumed its convolution

							pack_fl=tmp_cons_b_p_b_p->second->ip_fl;
							quantization_scheme = tmp_cons_b_p_b_p->second->quantization_scheme;
#if SINGLE_IO_PORT==0
							return 1;
#else
							return 4;

#endif
						}
					}
				}
#if SINGLE_IO_PORT==0
				return 1;
#else
				return 4;

#endif


			}else if(in_layer_name.compare("Pooling")==0){

				map < string, XLayer*>::iterator tmp_cons_top = xgraph->layers.find(it_cons->second->top[0].blob->consumers[0]);//TODO Assumed its convolution

				pack_fl=tmp_cons_top->second->ip_fl;
				quantization_scheme = tmp_cons_top->second->quantization_scheme;
				return 1;
			}



		}else if((output_layer_name.compare("XCustom")==0)&&(in_layer_name.compare("Softmax")==0)){

			return 2;

		}else if((output_layer_name.compare("Permute")==0)&&(in_layer_name.compare("XCustom")==0)){

			map < string, XLayer*>::iterator tmp_prod_bot = xgraph->layers.find(it_prods->second->bottom[0].blob->producers[0]);//TODO Assumed its convolution

			pack_fl=tmp_prod_bot->second->op_fl;
			quantization_scheme = tmp_prod_bot->second->quantization_scheme;
			return 3;

		}else{
			if((output_layer_name.compare("XCustom")==0)&&(in_layer_name.compare("Permute")==0)){

				map < string, XLayer*>::iterator tmp_cons_top = xgraph->layers.find(it_cons->second->top[0].blob->consumers[0]);//TODO Assumed its convolution

				pack_fl=tmp_cons_top->second->ip_fl;
				quantization_scheme = tmp_cons_top->second->quantization_scheme;
				return 0;
			}
		}if(in_layer_name.compare("InnerProduct")==0){

			if(output_layer_name.compare("Convolution")==0){

				if((it_prods->second->opcode!=OPCODE_AVRPOOL2CONV)&&(it_prods->second->opcode!=OPCODE_POOL2CONV)){
					pack_fl=it_prods->second->op_fl;
					quantization_scheme = it_prods->second->quantization_scheme;
				}
				else{
					if(it_prods->second->opcode==OPCODE_AVRPOOL2CONV){
						pack_fl=it_prods->second->conv_params->op_fl_3d;
						quantization_scheme = it_prods->second->quantization_scheme;
					}else{
						map < string, XLayer*>::iterator tmp_prods = xgraph->layers.find(it_prods->second->bottom[0].blob->producers[0]);//TODO Assumed its convolution
						pack_fl=tmp_prods->second->op_fl;
						quantization_scheme = tmp_prods->second->quantization_scheme;
					}

				}
#if SINGLE_IO_PORT==0
				return 0;
#else
				return 5;

#endif
			}else if(output_layer_name.compare("Pooling")==0){

				map < string, XLayer*>::iterator tmp_prods = xgraph->layers.find(it_prods->second->bottom[0].blob->producers[0]);//TODO Assumed its convolution

				pack_fl=tmp_prods->second->op_fl;
				quantization_scheme = tmp_prods->second->quantization_scheme;
				return 0;
			}

		}


	}
	return -1;
}

void prod_port_comp_out(Port_info_s &pack_port,vector<Port_info_s> &layer_port, vector<int> &io_match){

	vector<int> tmp_vect;

	if(layer_port.size()==1){

		if((layer_port[0].data_type!=pack_port.data_type)&&(layer_port[0].qant_scheme_type!=pack_port.qant_scheme_type)){
			tmp_vect.push_back(1);
		}else{
			tmp_vect.push_back(0);
		}
	}
	io_match=tmp_vect;
}
void prod_port_comp_input(Port_info_s &pack_port,vector<Port_info_s> &layer_port, vector<int> &io_match){
	vector<int> tmp_vect;

	for(int iter_in_vc=0;iter_in_vc<layer_port.size();iter_in_vc++){

		if((layer_port[iter_in_vc].data_type!=pack_port.data_type)&&(layer_port[iter_in_vc].qant_scheme_type!=pack_port.qant_scheme_type)){

			tmp_vect.push_back(iter_in_vc+1);

		}else{

			tmp_vect.push_back(0);
		}
	}

	io_match=tmp_vect;
}
void prod_port_comp(Port_info_s &pack_port,vector<Port_info_s> &layer_port, vector<int> &io_match){

	vector<Port_info_s> input_port_v;
	vector<Port_info_s> out_port_v;
	vector<int> tmp_vect;

	for(int iter_p_v=0;iter_p_v<layer_port.size();iter_p_v++){

		if(layer_port[iter_p_v].port_enum==XI_INPUT){

			input_port_v.push_back(layer_port[iter_p_v]);
		}
		else{

			if(layer_port[iter_p_v].port_enum==XI_OUTPUT){

				out_port_v.push_back(layer_port[iter_p_v]);

			}
		}
	}
	if(out_port_v.size()==1){

		if((out_port_v[0].data_type!=pack_port.data_type)&&(out_port_v[0].qant_scheme_type!=pack_port.qant_scheme_type)){
			tmp_vect.push_back(1);
		}else{
			tmp_vect.push_back(0);
		}
	}else{
		for(int iter_in_vc=0;iter_in_vc<input_port_v.size();iter_in_vc++){

			if((input_port_v[iter_in_vc].data_type!=pack_port.data_type)&&(input_port_v[iter_in_vc].qant_scheme_type!=pack_port.qant_scheme_type)){

				tmp_vect.push_back(iter_in_vc+1);

			}else{

				tmp_vect.push_back(0);
			}
		}
	}
	io_match=tmp_vect;
}

void find_input_port_type(XGraph *xgraph_opt,kernelInfo& opt_descriptor,map < string, XBlob*>::iterator &it_blob,Port_info_s &pack_port,vector<Port_info_s> &cons_in_pack,int &change_status){


	Port_info_s in_portv_prev;
	vector<Port_info_s> tmp_input_port;

	if(it_blob->second->consumers.size()!=0){

		for(int it_b_c_c=0;it_b_c_c<it_blob->second->consumers.size();it_b_c_c++){

			map < string, XLayer*>::iterator cons_layer_it = xgraph_opt->layers.find(it_blob->second->consumers[it_b_c_c]);

			opcode_num_e cons_opcode = (opcode_num_e)cons_layer_it->second->opcode;
			map<opcode_num_e, Opcode_info>::iterator it_map_opcode = opt_descriptor.opcode_info_map.find(cons_opcode);
			for(int iter_in_p_t=0;iter_in_p_t<it_map_opcode->second.port_vec.size();iter_in_p_t++){

				if(it_map_opcode->second.port_vec[iter_in_p_t].port_enum==XI_INPUT){
					tmp_input_port.push_back(it_map_opcode->second.port_vec[iter_in_p_t]);
				}
			}
			for(int iter_b_i=0;iter_b_i<cons_layer_it->second->bottom.size();iter_b_i++){

				if(cons_layer_it->second->bottom[iter_b_i].blob->name.compare(it_blob->second->name)==0)
				{
					in_portv_prev = tmp_input_port[iter_b_i];
				}
			}


		}
		cons_in_pack.push_back(in_portv_prev);
		for(int iter_bc=1;iter_bc<it_blob->second->consumers.size();iter_bc++){

			map < string, XLayer*>::iterator cons_layer_it = xgraph_opt->layers.find(it_blob->second->consumers[iter_bc]);

			opcode_num_e cons_opcode = (opcode_num_e)cons_layer_it->second->opcode;
			map<opcode_num_e, Opcode_info>::iterator it_map_opcode = opt_descriptor.opcode_info_map.find(cons_opcode);

			for(int iter_cv=0;iter_cv<it_map_opcode->second.port_vec.size();iter_cv++){

				if(it_map_opcode->second.port_vec[iter_cv].port_enum==XI_INPUT){

					if(!compare_pack_info(in_portv_prev.pack_info_vec,it_map_opcode->second.port_vec[iter_cv].pack_info_vec)){

						if(it_map_opcode->second.port_vec[iter_cv].data_type!=in_portv_prev.data_type){

							if(it_map_opcode->second.port_vec[iter_cv].size_datatype > in_portv_prev.size_datatype){
								in_portv_prev = it_map_opcode->second.port_vec[iter_cv];
								cons_in_pack.push_back(in_portv_prev);
								change_status=1;

							}else {
								if(it_map_opcode->second.port_vec[iter_cv].size_datatype==in_portv_prev.size_datatype){
									map<_datatype_size_e,_datainfo_s>::iterator it_data_info = opt_descriptor.datainfo_map.find(in_portv_prev.size_datatype);
									if(it_data_info->second.t_data==it_map_opcode->second.port_vec[iter_cv].data_type){

										in_portv_prev=it_map_opcode->second.port_vec[iter_cv];
										cons_in_pack.push_back(in_portv_prev);
										change_status=1;

									}
								}else{
									//in_portv_prev=it_map_opcode->second.port_vec[iter_cv];
									cons_in_pack.push_back(it_map_opcode->second.port_vec[iter_cv]);
									change_status=1;
								}
							}
						}else{
							in_portv_prev=it_map_opcode->second.port_vec[iter_cv];
							cons_in_pack.push_back(in_portv_prev);
							change_status=1;
						}
					}
				}
			}
		}
		pack_port = in_portv_prev;
		change_status=1;
	}
}
bool check_producers_port(vector<Port_info_s> &prod_port_format){

	if(prod_port_format.size()>1){

		return false;

	}else{

		return true;
	}
}
bool check_consumers_port(vector<Port_info_s> &cons_port_format){

	if(cons_port_format.size()>1){

		return false;

	}else{

		return true;
	}
}
bool check_cons_prod_port_same(Port_info_s &prod_port,Port_info_s &cons_port){

	if(compare_pack_info(prod_port.pack_info_vec,cons_port.pack_info_vec)){

		if(prod_port.data_type==cons_port.data_type ){
			return true;
		}else{
			return false;
		}
	}else{
		return false;
	}
}
void inser_format_conversion_layer_above_it(XGraph *xgraph,map < string, XBlob*>::iterator &it_blob,kernelInfo& opt_descriptor,Port_info_s &output_port,Port_info_s &input_port,int &custom_layer_cnt){


	string tmp_prod_layer_name;
	string tmp_cons_layer_name;
	string quantization_scheme;
	for(int iter_bp=0;iter_bp<it_blob->second->producers.size();iter_bp++){

		Port_info_s tmp_port;
		vector<Port_info> port_vec_in;
		vector<Port_info> port_vec_out;

		map < string, XLayer*>::iterator prod_layer_it = xgraph->layers.find(it_blob->second->producers[iter_bp]);
		tmp_prod_layer_name = prod_layer_it->second->name;
		opcode_num_e prods_opcode = (opcode_num_e)prod_layer_it->second->opcode;

		map<opcode_num_e, Opcode_info>::iterator it_map_opcode = opt_descriptor.opcode_info_map.find(prods_opcode);
		int top_b_indx=-1;
		for(int it_p_c=0;it_p_c<it_map_opcode->second.port_vec.size();it_p_c++){

			if(it_map_opcode->second.port_vec[it_p_c].port_enum==XI_OUTPUT)
				port_vec_out.push_back(it_map_opcode->second.port_vec[it_p_c]);
			if(it_map_opcode->second.port_vec[it_p_c].port_enum==XI_INPUT)
				port_vec_in.push_back(it_map_opcode->second.port_vec[it_p_c]);
		}
		for(int iter_cons_bb_c=0;iter_cons_bb_c<prod_layer_it->second->top.size();iter_cons_bb_c++){

			if(prod_layer_it->second->top[iter_cons_bb_c].blob->name.compare(it_blob->second->name)==0){

				top_b_indx=iter_cons_bb_c;

				if(iter_cons_bb_c<=it_map_opcode->second.port_vec.size()){

					if(port_vec_out[iter_cons_bb_c].port_enum==XI_OUTPUT)
						tmp_port = port_vec_out[iter_cons_bb_c];
				}

			}
		}
		//		if(tmp_port.data_type!=XI_INT8){

		if(!compare_pack_info(tmp_port.pack_info_vec,input_port.pack_info_vec)){

			map < string, XLayer*>::iterator cons_layer_it = xgraph->layers.find(it_blob->second->consumers[0]); // TODO current version assumed to be one consumer
			tmp_cons_layer_name=cons_layer_it->second->name;
			//string &t_prods_name,string &t_cons_name
			int pack_fl=0;
			int pack=-1;
			pack=pack_info_type(input_port,tmp_port,opt_descriptor,xgraph,pack_fl,tmp_prod_layer_name,tmp_cons_layer_name,quantization_scheme);
			if(pack==-1)
				return;


			string layer_name;
			layer_name= "format_converter";
			std::stringstream ss1;

			ss1<<layer_name<<custom_layer_cnt;
			string packed_layer_name = ss1.str();

			XLayer* dst0=new XLayer(packed_layer_name, "XPack");

			dst0->xpack_params->pack=pack;
			dst0->xpack_params->fbits = pack_fl;
			xgraph->layers[packed_layer_name] = dst0;
			dst0->opcode=OPCODE_XPACK;

			dst0->quantization_scheme = quantization_scheme; //TODO In feature we need to update this as layer based instead of xgraph

			string username = it_blob->second->producers[iter_bp];
			XBlob* tmp_blob = new XBlob(username);
			xgraph->blobs[username] = tmp_blob;
			nameIndex new_blob(tmp_blob, 0);

			tmp_blob->shape=prod_layer_it->second->topShape[0];

			dst0->top.push_back(it_blob->second); //it means layer
			dst0->top[0].id = iter_bp;

			dst0->bottomShape.push_back(tmp_blob->shape);
			dst0->topShape.push_back(tmp_blob->shape);

			//******** pack info **********/
			dst0->xpack_params->in_port_vec.push_back(output_port);
			dst0->xpack_params->out_port_vec.push_back(tmp_port);

			tmp_blob->consumers.push_back(dst0->name);
			tmp_blob->producers.push_back(it_blob->second->producers[iter_bp]);

			vector<string>::iterator it_str = find(prod_layer_it->second->top[0].blob->producers.begin(),prod_layer_it->second->top[0].blob->producers.end(),it_blob->second->producers[iter_bp]);
			if(it_str!=prod_layer_it->second->top[0].blob->producers.end())
			{
				prod_layer_it->second->top[0].blob->producers.erase(it_str);
			}
			prod_layer_it->second->top[0].blob->producers.insert(it_str,dst0->name);
			prod_layer_it->second->top[0]=tmp_blob;
			prod_layer_it->second->top[0].id=0;
			dst0->bottom.push_back(tmp_blob);
			dst0->op_fl = prod_layer_it->second->op_fl;
			dst0->ip_fl = prod_layer_it->second->ip_fl;

			if(it_blob->second->consumers.size()!=0){

				//map < string, XLayer*>::iterator cons_layer_it = xgraph->layers.find(it_blob->second->consumers[0]);

				if(cons_layer_it->second->opcode==OPCODE_AVRPOOL2CONV){

					dst0->th_layer_in = cons_layer_it->second->conv_params->th_layer_in_3d;
					dst0->ip_bw = cons_layer_it->second->conv_params->ip_bw_3d;
				}else if(cons_layer_it->second->opcode==OPCODE_POOL2CONV){

					dst0->th_layer_in = cons_layer_it->second->conv_params->th_layer_in_3d;
					dst0->ip_bw = cons_layer_it->second->conv_params->ip_bw_3d;
				}else{
					dst0->th_layer_in = cons_layer_it->second->th_layer_in;
					dst0->ip_bw = cons_layer_it->second->ip_bw;
				}

			}
			custom_layer_cnt++;

		}
	}

}
void inser_format_conversion_layer_below_it(XGraph *xgraph,map < string, XBlob*>::iterator &it_blob,kernelInfo& opt_descriptor,Port_info_s &output_port,Port_info_s &input_port,int &custom_layer_cnt){


	string tmp_prod_layer_name;
	string tmp_cons_layer_name;
	string quantization_scheme;
	for(int iter_bp=0;iter_bp<it_blob->second->producers.size();iter_bp++){


		map < string, XLayer*>::iterator prod_it = xgraph->layers.find(it_blob->second->producers[iter_bp]);
		tmp_prod_layer_name=prod_it->second->name; // need for packinfo

		opcode_num_e prod_opcode = (opcode_num_e)prod_it->second->opcode;
		map<opcode_num_e, Opcode_info>::iterator it_map_opcode_prod = opt_descriptor.opcode_info_map.find(prod_opcode);
		vector<Port_info_s> prod_port_vec_out,prod_port_vec_in;
		for(int it_p_c=0;it_p_c<it_map_opcode_prod->second.port_vec.size();it_p_c++){

			if(it_map_opcode_prod->second.port_vec[it_p_c].port_enum==XI_OUTPUT)
				prod_port_vec_out.push_back(it_map_opcode_prod->second.port_vec[it_p_c]);
			if(it_map_opcode_prod->second.port_vec[it_p_c].port_enum==XI_INPUT)
				prod_port_vec_in.push_back(it_map_opcode_prod->second.port_vec[it_p_c]);
		}
		if(prod_it->second->top[0].blob->consumers.size()>1){

			for(int iter_ptc=0;iter_ptc<prod_it->second->top[0].blob->consumers.size();iter_ptc++){

				Port_info_s tmp_port;
				vector<Port_info_s> port_vec_out,port_vec_in;

				map < string, XLayer*>::iterator cons_layer_it = xgraph->layers.find(prod_it->second->top[0].blob->consumers[iter_ptc]);
				if((cons_layer_it->second->type.compare("Softmax")==0) || (cons_layer_it->second->type.compare("Deconvolution")==0)){ //TODO currently as per the discussion with Arun/Anitha asked to not to insert any pack convertion.
					return;
				}
				opcode_num_e cons_opcode = (opcode_num_e)cons_layer_it->second->opcode;
				map<opcode_num_e, Opcode_info>::iterator it_map_opcode = opt_descriptor.opcode_info_map.find(cons_opcode);
				int bottom_b_indx=-1;

				for(int it_p_c=0;it_p_c<it_map_opcode->second.port_vec.size();it_p_c++){

					if(it_map_opcode->second.port_vec[it_p_c].port_enum==XI_OUTPUT)
						port_vec_out.push_back(it_map_opcode->second.port_vec[it_p_c]);
					if(it_map_opcode->second.port_vec[it_p_c].port_enum==XI_INPUT)
						port_vec_in.push_back(it_map_opcode->second.port_vec[it_p_c]);
				}

				for(int iter_cons_bb_c=0;iter_cons_bb_c<cons_layer_it->second->bottom.size();iter_cons_bb_c++){

					if(cons_layer_it->second->bottom[iter_cons_bb_c].blob->name.compare(it_blob->second->name)==0){
						bottom_b_indx=iter_cons_bb_c;
						if(iter_cons_bb_c<=it_map_opcode->second.port_vec.size()){

							if(port_vec_in[iter_cons_bb_c].port_enum==XI_INPUT)
								tmp_port = port_vec_out[iter_cons_bb_c];
						}
					}
				}

				//if(tmp_port.data_type!=prod_port_vec_out[0].data_type){


				if(!compare_pack_info(tmp_port.pack_info_vec,prod_port_vec_out[0].pack_info_vec)){

					tmp_cons_layer_name = cons_layer_it->second->name; // need for packinfo

					int pack_fl=0;
					int pack=-1;
					pack=pack_info_type(tmp_port,prod_port_vec_out[0],opt_descriptor,xgraph,pack_fl,tmp_prod_layer_name,tmp_cons_layer_name,quantization_scheme);
					if(pack==-1)
						return;
					string layer_name;

					layer_name= "format_converter";
					std::stringstream ss1;

					ss1<<layer_name<<custom_layer_cnt;
					string packed_layer_name = ss1.str();

					XLayer* dst0=new XLayer(packed_layer_name, "XPack");



					dst0->xpack_params->pack=pack;

					dst0->xpack_params->fbits = pack_fl;


					if(prod_it->second->opcode==OPCODE_AVRPOOL2CONV){

						dst0->th_layer_in = prod_it->second->conv_params->th_layer_out_3d;
						dst0->ip_bw = prod_it->second->conv_params->op_bw_3d;

					}else if(prod_it->second->opcode==OPCODE_POOL2CONV){

						dst0->th_layer_in = prod_it->second->conv_params->th_layer_out_3d;
						dst0->ip_bw = prod_it->second->conv_params->op_bw_3d;

					}else{
						dst0->th_layer_in = prod_it->second->th_layer_out;
						dst0->ip_bw = prod_it->second->op_bw;
					}


					xgraph->layers[packed_layer_name] = dst0;
					dst0->opcode=OPCODE_XPACK;

					dst0->quantization_scheme = quantization_scheme; //TODO In feature we need to update this as layer based instead of xgraph


					string username = packed_layer_name;
					XBlob* tmp_blob = new XBlob(username);
					xgraph->blobs[username] = tmp_blob;
					nameIndex new_blob(tmp_blob, 0);

					tmp_blob->shape=it_blob->second->shape;

					dst0->top.push_back(tmp_blob); //it means layer


					dst0->bottomShape.push_back(tmp_blob->shape);
					dst0->topShape.push_back(tmp_blob->shape);

					//******** pack info **********/
					dst0->xpack_params->in_port_vec.push_back(output_port);
					dst0->xpack_params->out_port_vec.push_back(tmp_port);

					tmp_blob->consumers.push_back(prod_it->second->top[0].blob->consumers[iter_ptc]);

					vector<string>::iterator it_str = find(prod_it->second->top[0].blob->consumers.begin(),prod_it->second->top[0].blob->consumers.end(),prod_it->second->top[0].blob->consumers[iter_ptc]);
					if(it_str!=prod_it->second->top[0].blob->consumers.end())
					{
						prod_it->second->top[0].blob->consumers.erase(it_str);
					}
					prod_it->second->top[0].blob->consumers.insert(it_str,dst0->name);
					tmp_blob->producers.push_back(dst0->name);

					dst0->bottom.push_back(prod_it->second->top[0]);

					cons_layer_it->second->bottom[bottom_b_indx]=tmp_blob;
					custom_layer_cnt++;
					dst0->ip_fl = prod_it->second->ip_fl;

				}
			}
		}else{

			Port_info_s tmp_port;
			vector<Port_info_s> port_vec_out,port_vec_in;
			map < string, XLayer*>::iterator cons_layer_it = xgraph->layers.find(prod_it->second->top[0].blob->consumers[0]);
			tmp_cons_layer_name = cons_layer_it->second->name; // need for packinfo
			if((cons_layer_it->second->type.compare("Softmax")==0)|| (cons_layer_it->second->type.compare("Deconvolution")==0)){ //TODO currently as per the discussion with Arun/Anitha asked to not to insert any pack convertion.
				return;
			}
			opcode_num_e cons_opcode = (opcode_num_e)cons_layer_it->second->opcode;
			map<opcode_num_e, Opcode_info>::iterator it_map_opcode = opt_descriptor.opcode_info_map.find(cons_opcode);
			int bottom_b_indx=-1;

			for(int it_p_c=0;it_p_c<it_map_opcode->second.port_vec.size();it_p_c++){

				if(it_map_opcode->second.port_vec[it_p_c].port_enum==XI_OUTPUT)
					port_vec_out.push_back(it_map_opcode->second.port_vec[it_p_c]);
				if(it_map_opcode->second.port_vec[it_p_c].port_enum==XI_INPUT)
					port_vec_in.push_back(it_map_opcode->second.port_vec[it_p_c]);
			}
			for(int iter_cons_bb_c=0;iter_cons_bb_c<cons_layer_it->second->bottom.size();iter_cons_bb_c++){

				if(cons_layer_it->second->bottom[iter_cons_bb_c].blob->name.compare(it_blob->second->name)==0){
					bottom_b_indx=iter_cons_bb_c;
					if(iter_cons_bb_c<=it_map_opcode->second.port_vec.size()){

						if(port_vec_in[iter_cons_bb_c].port_enum==XI_INPUT)
							tmp_port = port_vec_in[iter_cons_bb_c];
					}
				}
			}
			if(!compare_pack_info(tmp_port.pack_info_vec,prod_port_vec_out[0].pack_info_vec)){
				//			if(tmp_port.data_type!=prod_port_vec_out[0].data_type){



				int pack_fl=0;
				int pack=-1;
				pack=pack_info_type(tmp_port,prod_port_vec_out[0],opt_descriptor,xgraph,pack_fl,tmp_prod_layer_name,tmp_cons_layer_name,quantization_scheme);
				if (pack==-1)
					return ;

				string layer_name;
				layer_name= "format_converter";

				std::stringstream ss1;

				ss1<<layer_name<<custom_layer_cnt;
				string packed_layer_name = ss1.str();

				XLayer* dst0= new XLayer(packed_layer_name, "XPack");;
				dst0->xpack_params->pack=pack;
				dst0->xpack_params->fbits = pack_fl;
				if(prod_it->second->opcode==OPCODE_AVRPOOL2CONV){

					dst0->th_layer_in = prod_it->second->conv_params->th_layer_out_3d;
					dst0->ip_bw = prod_it->second->conv_params->op_bw_3d;
				}else if(prod_it->second->opcode==OPCODE_POOL2CONV){

					dst0->th_layer_in = prod_it->second->conv_params->th_layer_out_3d;
					dst0->ip_bw = prod_it->second->conv_params->op_bw_3d;
				}else{
					dst0->th_layer_in = prod_it->second->th_layer_out;
					dst0->ip_bw = prod_it->second->op_bw;
				}

				xgraph->layers[packed_layer_name] = dst0;
				dst0->opcode=OPCODE_XPACK;
				dst0->quantization_scheme = quantization_scheme; //TODO In feature we need to update this as layer based instead of xgraph

				string username = packed_layer_name;
				XBlob* tmp_blob = new XBlob(username);
				xgraph->blobs[username] = tmp_blob;
				nameIndex new_blob(tmp_blob, 0);

				tmp_blob->shape=it_blob->second->shape;

				dst0->top.push_back(tmp_blob); //it means layer


				dst0->bottomShape.push_back(tmp_blob->shape);
				dst0->topShape.push_back(tmp_blob->shape);

				dst0->xpack_params->in_port_vec.push_back(output_port);
				dst0->xpack_params->out_port_vec.push_back(tmp_port);

				for(int iter_ptc=0;iter_ptc<prod_it->second->top[0].blob->consumers.size();iter_ptc++){

					tmp_blob->consumers.push_back(prod_it->second->top[0].blob->consumers[iter_ptc]);

					vector<string>::iterator it_str = find(prod_it->second->top[0].blob->consumers.begin(),prod_it->second->top[0].blob->consumers.end(),prod_it->second->top[0].blob->consumers[iter_ptc]);
					if(it_str!=prod_it->second->top[0].blob->consumers.end())
					{
						prod_it->second->top[0].blob->consumers.erase(it_str);
					}
				}

				prod_it->second->top[0].blob->consumers.push_back(dst0->name);
				int in_fl=0,out_fl=0;
				dst0->bottom.push_back(prod_it->second->top[0]);
				tmp_blob->producers.push_back(dst0->name);

				for(int iter_cbb=0;iter_cbb<tmp_blob->consumers.size();iter_cbb++){

					map < string, XLayer*>::iterator cons_layer_it = xgraph->layers.find(tmp_blob->consumers[iter_cbb]);

					for(int iter_cbc=0;iter_cbc<cons_layer_it->second->bottom.size();iter_cbc++){

						if(cons_layer_it->second->bottom[iter_cbc].blob->name.compare(it_blob->second->name)==0){
							cons_layer_it->second->bottom[iter_cbc]=tmp_blob;
							in_fl = cons_layer_it->second->ip_fl;
							out_fl = cons_layer_it->second->op_fl;
						}
					}
				}
				custom_layer_cnt++;
				dst0->ip_fl = in_fl;
				dst0->op_fl = out_fl;

				break;
			}
		}
	}
}

#if 0 /// working for one pro & cons
void inser_format_conversion_layer_below_it(XGraph *xgraph,map < string, XBlob*>::iterator &it_blob,kernelInfo& opt_descriptor,Port_info_s &output_port,Port_info_s &input_port,int &custom_layer_cnt){


	for(int iter_bp=0;iter_bp<it_blob->second->producers.size();iter_bp++){

		map < string, XLayer*>::iterator prod_it = xgraph->layers.find(it_blob->second->producers[iter_bp]);



		string layer_name = "xpack";
		std::stringstream ss1;

		ss1<<layer_name<<custom_layer_cnt;
		string packed_layer_name = ss1.str();

		XLayer* dst0 = new XLayer(packed_layer_name, "XPack");
		xgraph->layers[packed_layer_name] = dst0;

		dst0->xpack_params->pack=false;
		dst0->opcode=OPCODE_XPACK;
		string username = packed_layer_name;
		XBlob* tmp_blob = new XBlob(username);
		xgraph->blobs[username] = tmp_blob;
		nameIndex new_blob(tmp_blob, 0);

		tmp_blob->shape=it_blob->second->shape;

		dst0->top.push_back(tmp_blob); //it means layer


		dst0->bottomShape.push_back(tmp_blob->shape);
		dst0->topShape.push_back(tmp_blob->shape);

		for(int iter_ptc=0;iter_ptc<prod_it->second->top[0].blob->consumers.size();iter_ptc++){

			tmp_blob->consumers.push_back(prod_it->second->top[0].blob->consumers[iter_ptc]);
			//map < string, XLayer*>::iterator prod_it = xgraph->layers.find(prod_it->second->top[0].blob->consumers[iter_ptc]);
			//opcode_num_e cons_opcode = (opcode_num_e)cons_layer_it->second->opcode;
			//map<opcode_num_e, Opcode_info>::iterator it_map_opcode = opt_descriptor.opcode_info_map.find(cons_opcode);

			vector<string>::iterator it_str = find(prod_it->second->top[0].blob->consumers.begin(),prod_it->second->top[0].blob->consumers.end(),prod_it->second->top[0].blob->consumers[iter_ptc]);
			if(it_str!=prod_it->second->top[0].blob->consumers.end())
			{
				prod_it->second->top[0].blob->consumers.erase(it_str);
			}
		}

		prod_it->second->top[0].blob->consumers.push_back(dst0->name);

		dst0->bottom.push_back(prod_it->second->top[0]);

		tmp_blob->producers.push_back(dst0->name);

		for(int iter_cbb=0;iter_cbb<tmp_blob->consumers.size();iter_cbb++){

			map < string, XLayer*>::iterator cons_layer_it = xgraph->layers.find(tmp_blob->consumers[iter_cbb]);

			for(int iter_cbc=0;iter_cbc<cons_layer_it->second->bottom.size();iter_cbc++){

				if(cons_layer_it->second->bottom[iter_cbc].blob->name.compare(it_blob->second->name)==0){
					cons_layer_it->second->bottom[iter_cbc]=tmp_blob;
				}
			}
		}
		custom_layer_cnt++;
	}
}
#endif
void find_output_port_type(XGraph *xgraph_opt,kernelInfo& opt_descriptor,map < string, XBlob*>::iterator &it_blob,Port_info_s &pack_port,vector<Port_info_s> &prod_in_pack,int &change_status){


	Port_info_s out_portv_prev;
	vector<Port_info_s> tmp_out_port;
	if(it_blob->second->producers.size()!=0){

		for(int it_b_p_c=0;it_b_p_c<it_blob->second->producers.size();it_b_p_c++){

			map < string, XLayer*>::iterator prod_layer_it = xgraph_opt->layers.find(it_blob->second->producers[it_b_p_c]);

			opcode_num_e prod_opcode = (opcode_num_e)prod_layer_it->second->opcode;

			map<opcode_num_e, Opcode_info>::iterator it_map_opcode = opt_descriptor.opcode_info_map.find(prod_opcode);

			for(int iter_out_p_t=0;iter_out_p_t<it_map_opcode->second.port_vec.size();iter_out_p_t++){

				if(it_map_opcode->second.port_vec[iter_out_p_t].port_enum==XI_OUTPUT){

					tmp_out_port.push_back(it_map_opcode->second.port_vec[iter_out_p_t]);
				}
			}
			for(int iter_b_i=0;iter_b_i<prod_layer_it->second->top.size();iter_b_i++){

				if(prod_layer_it->second->top[iter_b_i].blob->name.compare(it_blob->second->name)==0)
				{
					out_portv_prev=tmp_out_port[iter_b_i];
				}
			}
		}

		prod_in_pack.push_back(out_portv_prev);

		for(int iter_bc=1;iter_bc<it_blob->second->producers.size();iter_bc++){

			map < string, XLayer*>::iterator  prod_layer_it = xgraph_opt->layers.find(it_blob->second->producers[iter_bc]);

			opcode_num_e  prod_opcode = (opcode_num_e)prod_layer_it->second->opcode;
			map<opcode_num_e, Opcode_info>::iterator it_map_opcode = opt_descriptor.opcode_info_map.find(prod_opcode);

			for(int iter_cv=0;iter_cv<it_map_opcode->second.port_vec.size();iter_cv++){

				if(it_map_opcode->second.port_vec[iter_cv].port_enum==XI_OUTPUT){

					if(!compare_pack_info(out_portv_prev.pack_info_vec,it_map_opcode->second.port_vec[iter_cv].pack_info_vec)){

						if(it_map_opcode->second.port_vec[iter_cv].data_type!=out_portv_prev.data_type){

							if(it_map_opcode->second.port_vec[iter_cv].size_datatype > out_portv_prev.size_datatype){
								out_portv_prev = it_map_opcode->second.port_vec[iter_cv];
								prod_in_pack.push_back(out_portv_prev);
								change_status=1;

							}else {
								if(it_map_opcode->second.port_vec[iter_cv].size_datatype==out_portv_prev.size_datatype){
									map<_datatype_size_e,_datainfo_s>::iterator it_data_info = opt_descriptor.datainfo_map.find(out_portv_prev.size_datatype);
									if(it_data_info->second.t_data==it_map_opcode->second.port_vec[iter_cv].data_type){

										out_portv_prev=it_map_opcode->second.port_vec[iter_cv];
										prod_in_pack.push_back(out_portv_prev);
										change_status=1;

									}
								}else{
									//out_portv_prev=it_map_opcode->second.port_vec[iter_cv];
									prod_in_pack.push_back(it_map_opcode->second.port_vec[iter_cv]);
									change_status=1;
								}
							}
						}else{
							out_portv_prev=it_map_opcode->second.port_vec[iter_cv];
							prod_in_pack.push_back(out_portv_prev);
							change_status=1;
						}
					}
				}
			}
		}
		pack_port = out_portv_prev;
		change_status=1;
	}
}
void get_hw_and_sw_layer(vector <struct xtract_layer_type> &xtract_layer,kernelInfo& opt_descriptor){


	vector<string> tmp_string;

	for(map<opcode_num_e, Opcode_info> ::iterator it_opt=opt_descriptor.opcode_info_map.begin();it_opt!=opt_descriptor.opcode_info_map.end();it_opt++){

		xtract_layer_type tmp;

		tmp.layer_name = it_opt->second.mega_module.module_name;
		tmp.layer_type = it_opt->second.mega_module.module_target_info;
		vector<string>::iterator it_str = find(tmp_string.begin(),tmp_string.end(),tmp.layer_name);

		if(it_str==tmp_string.end()){

			tmp_string.push_back(tmp.layer_name);
			xtract_layer.push_back(tmp);
		}
	}
}
void get_opcode_xlayer(vector < struct xtract_opcode_num> &xlayers_opcode,kernelInfo& opt_descriptor){



	for(map<opcode_num_e, Opcode_info> ::iterator it_opt=opt_descriptor.opcode_info_map.begin();it_opt!=opt_descriptor.opcode_info_map.end();it_opt++){

		xtract_opcode_num tmp;
		tmp.layer_name = it_opt->second.mega_module.module_name;

		tmp.opcode = it_opt->second.op_code;
		xlayers_opcode.push_back(tmp);
	}

}
void update_layer_io_type(map<string,io_type_e> &LayerIOType,kernelInfo& opt_descriptor){




	for(map<opcode_num_e, Opcode_info> ::iterator it_opt=opt_descriptor.opcode_info_map.begin();it_opt!=opt_descriptor.opcode_info_map.end();it_opt++){

		io_type_e tmp_type;
		Port_info prot_defualt;

		map<string,io_type_e>::iterator it_LayerIOType = LayerIOType.find(it_opt->second.mega_module.module_name);

		if(it_LayerIOType==LayerIOType.end()){

			for(int iter_pv=0;iter_pv < it_opt->second.port_vec.size();iter_pv++){

				if(it_opt->second.port_vec[iter_pv].port_enum==XI_INPUT){

					tmp_type.input_type.push_back(it_opt->second.port_vec[iter_pv].data_type);

				}
				if(it_opt->second.port_vec[iter_pv].port_enum==XI_OUTPUT){
					tmp_type.output_type.push_back(it_opt->second.port_vec[iter_pv].data_type);
				}
			}
			LayerIOType[it_opt->second.mega_module.module_name] = tmp_type;
		}
	}
#if 0
	for(map<string,io_type_e>::iterator it= LayerIOType.begin();it!= LayerIOType.end();it++){
		cout<<"module name"<<it->first<<endl;
		for(int i=0;i<it->second.input_type.size();i++){
			cout<<"module input data type "<<it->second.input_type[i]<<endl;
		}
		for(int i=0;i<it->second.output_type.size();i++){
			cout<<"module output data type "<<it->second.output_type[i]<<endl;
		}
	}
#endif
}

void find_total_output_plane(XGraph *xgraph){


	for(map < string, XBlob*>::iterator it = xgraph->blobs.begin();it !=xgraph->blobs.end();it++){

		int output_plane_sum=0;

		for(int iter_pro_c=0;iter_pro_c<it->second->producers.size();iter_pro_c++){

			string producer_name = it->second->producers[iter_pro_c];
			map < string, XLayer*>::iterator layer_it = xgraph->layers.find(producer_name);
			output_plane_sum = output_plane_sum + layer_it->second->topShape[0].at(1);
		}
		for(int iter_pro_c=0;iter_pro_c<it->second->producers.size();iter_pro_c++){

			string producer_name = it->second->producers[iter_pro_c];

			map < string, XLayer*>::iterator layer_it = xgraph->layers.find(producer_name);

			layer_it->second->total_output_plane=output_plane_sum;
		}
	}
}

void xgrap_layer_delete(XGraph *xgraph, string &delate_layer_name){

	string layer_name;//= layer_it->name;

	map < string, XLayer*>::iterator it_layer = xgraph->layers.find(delate_layer_name);

	if(it_layer==xgraph->layers.end())
	{

		cerr<<" layers doesn't exists name = " << delate_layer_name << endl;
		exit(-1);

	}

	if(it_layer->second->inPlace){

		map < string, XBlob* >::iterator top_it =xgraph->blobs.find(it_layer->second->top[0].blob->name);

		std::vector<string>::iterator it_str=std::find(top_it->second->producers.begin(),top_it->second->producers.end(),delate_layer_name);

		if(it_str!=top_it->second->producers.end())
		{
			top_it->second->producers.erase(it_str);
		}
		it_str=std::find(top_it->second->consumers.begin(),top_it->second->consumers.end(),delate_layer_name);

		if(it_str!=top_it->second->consumers.end())
		{
			top_it->second->consumers.erase(it_str);
		}

		//map < string, XLayer* >::iterator layer_it = xgraph->layers.find(layer_name);
		delete it_layer->second;
		xgraph->layers.erase(it_layer);

	}else{

		string top_name = it_layer->second->top[0].blob->name; // TODO we need think and add for multiple top scenarios

		map < string, XBlob*>::iterator top_blob = xgraph->blobs.find(top_name);

		top_blob->second->producers.erase(top_blob->second->producers.begin() + 0); // concat layer is produces for top blob and clearing it first

		for(int l_bottoms=0;l_bottoms<it_layer->second->bottom.size();l_bottoms++){

			string bottom_name = it_layer->second->bottom[l_bottoms].blob->name;

			map < string, XBlob*>::iterator bottom_blob = xgraph->blobs.find(bottom_name);

			if(bottom_blob->second->producers.size()== 1){ // TODO we need think and add for multiple producers scenarios

				string tmp_producer = bottom_blob->second->producers[0];
				map < string, XLayer* >::iterator producer_layer = xgraph->layers.find(tmp_producer);
				producer_layer->second->top[0].blob = it_layer->second->top[0].blob;
				producer_layer->second->top[0].id = l_bottoms;
				it_layer->second->top[0].blob->producers.push_back(tmp_producer);

			}
			delete bottom_blob->second;
			xgraph->blobs.erase(bottom_blob);
		}
		//map < string, XLayer* >::iterator layer_it = xgraph->layers.find(layer_name);
		//if(layer_it!=xgraph->layers.end())
		delete it_layer->second;
		xgraph->layers.erase(it_layer);
	}
}

void mem_type_print(map<string, string> &mem_type_table)
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
		cout<<" xlayer_seq[iter_c].current layer op_bw & op_fl: "<< xlayer_seq[iter_c].hw_ops->op_bw<<xlayer_seq[iter_c].hw_ops->op_fl<<endl;

		cout<<" layer_it->second->total_output_plane: "<< xlayer_seq[iter_c].hw_ops->total_output_plane<<endl;

		if(xlayer_seq[iter_c].hw_ops->type=="Convolution" || xlayer_seq[iter_c].hw_ops->opcode==OPCODE_SEPARABLE_CONV)
			cout<<" xlayer_seq[iter_c].hw_ops wt_bw & wt_fl "<< xlayer_seq[iter_c].hw_ops->wt_bw<<xlayer_seq[iter_c].hw_ops->wt_fl<<endl;


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

					cout<<" xlayer_seq[iter_c].ip_peers_shapes[it0].pos: "<< xlayer_seq[iter_c].ip_peers_shapes[it0].pos<<"xlayer_seq[iter_c].ip_peers_shapes :"<<xlayer_seq[iter_c].ip_peers_shapes[it0].shape[it1]<<endl;
				}
			}
		}
		if(!xlayer_seq[iter_c].ip_peers_shapes_total.empty()){

			for(int it0=0;it0 < xlayer_seq[iter_c].ip_peers_shapes_total.size(); it0++){

				for(int it1=0;it1 < xlayer_seq[iter_c].ip_peers_shapes_total[it0].shape.size(); it1++){

					cout<<" xlayer_seq[iter_c].ip_peers_shapes_total[it0].pos: "<< xlayer_seq[iter_c].ip_peers_shapes_total[it0].pos<<"xlayer_seq[iter_c].ip_peers_shapes_total :"<<xlayer_seq[iter_c].ip_peers_shapes_total[it0].shape[it1]<<endl;
				}
			}
		}
		if(!xlayer_seq[iter_c].op_peers_shapes.empty()){

			for(int it0=0;it0 < xlayer_seq[iter_c].op_peers_shapes.size(); it0++){


				for(int it1=0; it1 < xlayer_seq[iter_c].op_peers_shapes[it0].shape.size(); it1++){

					cout<<" xlayer_seq[iter_c].op_peers_shapes[it0].pos: "<< xlayer_seq[iter_c].op_peers_shapes[it0].pos<<"<<xlayer_seq[iter_c].op_peers_shapes[it0].shape[it1]"<<xlayer_seq[iter_c].op_peers_shapes[it0].shape[it1]<<endl;

				}
			}
		}
		if(!xlayer_seq[iter_c].op_peers_shapes_total.empty()){

			for(int it0=0;it0 < xlayer_seq[iter_c].op_peers_shapes_total.size(); it0++){


				for(int it1=0; it1 < xlayer_seq[iter_c].op_peers_shapes_total[it0].shape.size(); it1++){

					cout<<" xlayer_seq[iter_c].op_peers_shapes_total[it0].pos: "<< xlayer_seq[iter_c].op_peers_shapes_total[it0].pos<<"<<xlayer_seq[iter_c].op_peers_shapes_total[it0].shape[it1]"<<xlayer_seq[iter_c].op_peers_shapes_total[it0].shape[it1]<<endl;

				}
			}
		}
#endif
		if(!xlayer_seq[iter_c].ip_blob.empty()){

			for(int it0=0;it0 < xlayer_seq[iter_c].ip_blob.size(); it0++){

				//cout<<" xlayer_seq[iter_c].ip_blob[it0]: "<< it0 <<endl;

				cout<<" xlayer_seq[iter_c].ip_blob[it0].handle name: "<< xlayer_seq[iter_c].ip_blob[it0].handle<<endl;
				cout<<" xlayer_seq[iter_c].ip_blob[it0].pos : "<< xlayer_seq[iter_c].ip_blob[it0].pos<<endl;
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
				cout<<" xlayer_seq[iter_c].op_blob[it0].pos : "<< xlayer_seq[iter_c].op_blob[it0].pos<<endl;
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

int vectDim (vector<int>& src)
{
	int finalDim = 1;

	for(int i=0; i<src.size(); i++)
	{
		finalDim *= src.at(i);
	}

	return finalDim;
}



void maxsize_blob(XGraph *xgraph_opt,vector <int> &max_shape){

	int Dim0,Dim1;

	max_shape.push_back(1);
	max_shape.push_back(1);
	max_shape.push_back(1);
	max_shape.push_back(1);
	Dim0 = vectDim(max_shape);

	for(map < string, XBlob*>::iterator it=xgraph_opt->blobs.begin();it!=xgraph_opt->blobs.end();it++){

		Dim1 = vectDim(it->second->shape);

		if(Dim1>Dim0){

			max_shape=it->second->shape;
			Dim0 = Dim1;

		}

	}
}
void find_blobs_degree(XGraph *xgraph,vector<BDegree> &Bdepth){


	BDegree tmp_bdegree1;// = new BDegree();

	int degree_flag=0;

#if O_DEBUG
	cout<< "[O_DEBUG] find_blobs_degree: find_blobs_degree start" << endl;
#endif

	string input_blob_name = xgraph->input_blob;

	map < string, XBlob*>::iterator d_blob = xgraph->blobs.find(input_blob_name);

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
		cout<< "[O_DEBUG] find_blobs_degree: Bdepth.size() loop" << endl;
#endif
		BDegree tmp_bdegree;// = new BDegree();
		vector<string> same_depth_incept;

		for(int iter_db=0;iter_db<Bdepth[iter_d].blobs_name.size();iter_db++)
		{

#if O_DEBUG
			cout<< "[O_DEBUG] find_blobs_degree: Bdepth[iter_d].blobs_name.size() loop" << endl;
#endif
			map < string, XBlob*>::iterator blob_ptr = xgraph->blobs.find(Bdepth[iter_d].blobs_name[iter_db]);

			if(blob_ptr!=xgraph->blobs.end())
				tmp_blob = blob_ptr->second;
			else
				return;

			for(int iter_c=0;iter_c<tmp_blob->consumers.size();iter_c++)
			{

#if O_DEBUG
				cout<< "[O_DEBUG] find_blobs_degree: tmp_blob->consumers.size() loop" << endl;
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
					cout<< "[O_DEBUG] find_blobs_degree: tmp_top->producers.size() loop" << endl;
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
									cout<< "[O_DEBUG] find_blobs_degree: Bdepth.size() element find loop" << endl;
#endif
									std::vector<string>::iterator it=std::find(Bdepth[iter_bs].blobs_name.begin(),Bdepth[iter_bs].blobs_name.end(),tmp_bottom);

									if(it==Bdepth[iter_bs].blobs_name.end())
									{
#if O_DEBUG
										cout<< "[O_DEBUG] find_blobs_degree: std::find element not find in bdepth queue" << endl;
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
						if(tmp_p_layer->second->bottom.size()){
							string tmp_bottom = tmp_p_layer->second->bottom[0].blob->name;

							for(int iter_bs=0;iter_bs<Bdepth.size();iter_bs++)
							{
#if O_DEBUG
								cout<< "[O_DEBUG] find_blobs_degree: Bdepth.size() element find loop" << endl;
#endif
								std::vector<string>::iterator it=std::find(Bdepth[iter_bs].blobs_name.begin(),Bdepth[iter_bs].blobs_name.end(),tmp_bottom);
								if(it==Bdepth[iter_bs].blobs_name.end())
								{
#if O_DEBUG
									cout<< "[O_DEBUG] find_blobs_degree: std::find element not find in bdepth queue" << endl;
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
				}
				if(blob_str_flag==0)
				{

					std::vector<string>::iterator it_str_same_in=std::find(same_depth_incept.begin(),same_depth_incept.end(),tmp_top->name);

					if(it_str_same_in==same_depth_incept.end()){

						std::vector<string>::iterator it = std::find(tmp_bdegree.blobs_name.begin(),tmp_bdegree.blobs_name.end(),tmp_top->name);

						if(it==tmp_bdegree.blobs_name.end())
						{

							vector<string> tmp_prods_name;
							map < string, XBlob*>::iterator l_blob_ptr = xgraph->blobs.find(tmp_top->name);

							for(int it_p_name=0;it_p_name<l_blob_ptr->second->consumers.size();it_p_name++){

								tmp_prods_name.push_back(l_blob_ptr->second->consumers[it_p_name]);

							}

							for(int it_tbd=0;it_tbd<tmp_bdegree.blobs_name.size();it_tbd++){

								map < string, XBlob*>::iterator tmp_blob_ptr = xgraph->blobs.find(tmp_bdegree.blobs_name[it_tbd]);

								for(int it_e_b_p=0;it_e_b_p<tmp_blob_ptr->second->producers.size();it_e_b_p++){

									std::vector<string>::iterator it_str_p = std::find(tmp_prods_name.begin(),tmp_prods_name.end(),tmp_blob_ptr->second->producers[it_e_b_p]);

									if(it_str_p!=tmp_prods_name.end()){
										same_depth_incept.push_back(tmp_bdegree.blobs_name[it_tbd]);
										tmp_bdegree.blobs_name.erase(tmp_bdegree.blobs_name.begin()+it_tbd);
									}
								}
							}

							tmp_bdegree.blobs_name.push_back(tmp_top->name);
							tmp_bdegree.blobs_degree=iter_d+1;
							blob_str_flag=1;
							top_blob_exist=1;

						}
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


void print_blobs_degre(vector < BDegree > &Bdepth){

	for(int iter_bd=0;iter_bd<Bdepth.size();iter_bd++){

		for(int iter_b=0;iter_b<Bdepth[iter_bd].blobs_name.size();iter_b++){

			cout<<" Bdepth[iter_bd].blobs_degree:" <<Bdepth[iter_bd].blobs_degree<<"Bdepth[iter_bd].blobs_name: "<<Bdepth[iter_bd].blobs_name[iter_b]<<endl;
		}
	}
}

void create_seq_num_table(vector < XlayerData > &xlayer_seq,map<string, int> &layer_seq_table,map<string, string> &layer_type_table,vector < BDegree >&Bdepth,XGraph *xgraph){

	int cur_seq_id=0;

	//vector < XlayerData > tmp_xlayer_seq;

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

					//tmp_xlayer_seq.push_back(tmp_xlayerdata);
					xlayer_seq.push_back(tmp_xlayerdata);

				}else{

					for(int iter_b_c=0;iter_b_c<tmp_c_layer->second->bottom.size();iter_b_c++){

						string tmp_bottom = tmp_c_layer->second->bottom[iter_b_c].blob->name;
						int tmp_bottom_inx=0;

						for(int iter_bs=0;iter_bs<Bdepth.size();iter_bs++){
#if O_DEBUG
							cout<< "[O_DEBUG] create_seq_num_table: Bdepth.size() element find loop" << endl;
#endif
							std::vector<string>::iterator it=std::find(Bdepth[iter_bs].blobs_name.begin(),Bdepth[iter_bs].blobs_name.end(),tmp_bottom);

							if(it==Bdepth[iter_bs].blobs_name.end()){
#if O_DEBUG
								cout<< "[O_DEBUG] create_seq_num_table: std::find element not find in bdepth queue" << endl;
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
								continue;

							}else{
								tmp_xlayerdata.hw_ops=tmp_c_layer->second;
								tmp_xlayerdata.cur_seq_id = cur_seq_id++;//iter_d*Bdepth[iter_d].blobs_name.size()+iter_b;
								layer_seq_table[tmp_blob->consumers[iter_c]]=tmp_xlayerdata.cur_seq_id;
								layer_type_table[tmp_c_layer->second->name] = tmp_c_layer->second->type;
								//tmp_xlayer_seq.push_back(tmp_xlayerdata);
								xlayer_seq.push_back(tmp_xlayerdata);
							}
						}
						else if((Bdepth[iter_d].blobs_degree==tmp_bottom_inx) &&(tmp_bottom.compare(bdepth_current_blob)!=0)){ // need to check this condition thoroughly
							string curr_layer_name = tmp_c_layer->second->name;

							map<string, int>::iterator layer_id_it= layer_seq_table.find(curr_layer_name);
							if(layer_id_it!=layer_seq_table.end())
							{
								continue;

							}else{
								tmp_xlayerdata.hw_ops=tmp_c_layer->second;
								tmp_xlayerdata.cur_seq_id = cur_seq_id++;//iter_d*Bdepth[iter_d].blobs_name.size()+iter_b;
								layer_seq_table[tmp_blob->consumers[iter_c]]=tmp_xlayerdata.cur_seq_id;
								layer_type_table[tmp_c_layer->second->name] = tmp_c_layer->second->type;
								//tmp_xlayer_seq.push_back(tmp_xlayerdata);
								xlayer_seq.push_back(tmp_xlayerdata);
							}
						}
					}
				}
			}
		}
	}
}
void update_prev_next_layer_info(vector < XlayerData > &xlayer_seq, map<string, int> &layer_seq_table,XGraph *xgraph){


	io_buffer_pos tmp_pt;

	for(map < string, XLayer* >::iterator it = xgraph->layers.begin();it !=xgraph->layers.end();it++){


		vector < vector<int> > ip_peer;
		vector < vector<int> > out_peer;

		XLayer* curr_layer = it->second;
		string curr_layer_name = curr_layer->name;

		map<string, int>::iterator layer_id_it = layer_seq_table.find(curr_layer_name);

		if(layer_id_it==layer_seq_table.end()){
			cerr<<"layer name is not present "<<curr_layer_name<<endl;
			exit(-1);

		}
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

			io_peers  in_peer_ojb_totoal;


			for(int iter_b_c=0;iter_b_c<curr_layer->bottom[iter_b_s].id;iter_b_c++){

				string name_bottom_c =curr_layer->bottom[iter_b_s].blob->consumers[iter_b_c];

				map < string, XLayer* >::iterator bottom_c_it = xgraph->layers.find(name_bottom_c);


				if(name_bottom_c.compare(curr_layer_name)==0)
					continue;


				io_peers  in_peer_ojb;// = new io_peers();

				XLayer* bottom_c_layer = bottom_c_it->second;
				in_peer_ojb.pos = bottom_c_layer->bottom[iter_b_s].id;

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

				map < string, XLayer* >::iterator top_c_it = xgraph->layers.find(name_top_p);

				if(name_top_p.compare(curr_layer_name)==0)
					continue;


				io_peers  out_peer_ojb;// = new io_peers();

				XLayer* top_p_layer = top_c_it->second;
				out_peer_ojb.pos = top_p_layer->top[iter_t_s].id;

				for(int s_it=0;s_it<top_p_layer->topShape[iter_t_s].size();s_it++){

					out_peer_ojb.shape.push_back(top_p_layer->topShape[iter_t_s][s_it]);
#if O_DEBUG
					cout<<"on_peer_ojb.pos:"<<out_peer_ojb.pos<<" out_peer_ojb.shape[s_it]: " << out_peer_ojb.shape[s_it]<<endl;
					cout<<"top_p_layer->top[0].id:"<<top_p_layer->top[0].id<<" top_p_layer->top[0].blob->shape[s_it]: " << top_p_layer->top[0].blob->shape[s_it]<<endl;
#endif
				}

				xlayer_seq[layer_id_it->second].op_peers_shapes.push_back(out_peer_ojb);
			}
		}
	}
}

void find_hw_and_sw_layer_type(vector <struct xtract_layer_type> &xtract_layer_type_e,string &layer_type,int &hs_ops_type){

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
		cout<<"find_hw_and_sw_layer_type:Layer is find "<<"layer type:"<<layer_type<<endl;
#endif
	}
	else{
		iter_lt=iter_lt-1;
		ELOG(!(xtract_layer_type_e[iter_lt].layer_name.compare(layer_type)==0),"EO005", " layer type: Layer is not support : "<< layer_type);
	}
}

void find_blob_data_type(vector< string > &vect_p_layer_type,vector< string > &vect_c_layer_type,map<string,struct io_type> &LayerIOType,_datatype_e  & ret_data_type){

	struct io_type tmp;

	ret_data_type = XI_INT8;

	io_type_e tmp_io_type;

	for(int iter_t=0;iter_t<vect_p_layer_type.size();iter_t++){

		map<string,struct io_type>::iterator it_io_type = LayerIOType.find(vect_p_layer_type[iter_t]);

		tmp.output_type = it_io_type->second.output_type;

		for(int i=0;i<tmp.output_type.size();i++){

			if(ret_data_type<tmp.output_type[i]){

				ret_data_type=tmp.output_type[i];
			}
		}
	}
	for(int iter_t=0;iter_t<vect_c_layer_type.size();iter_t++){

		map<string,struct io_type>::iterator it_io_type = LayerIOType.find(vect_c_layer_type[iter_t]);

		tmp.input_type = it_io_type->second.input_type;

		for(int i=0;i<tmp.input_type.size();i++){

			if(ret_data_type<tmp.input_type[i]){

				ret_data_type=tmp.input_type[i];

			}
		}
	}
}

void find_mem_type_io_buffers(XGraph *xgraph,vector < BDegree > &Bdepth,map<string, string> &layer_type_table,map<string, io_mem_data_type_e> &mem_type_table,map<string,struct io_type> &LayerIOType,kernelInfo& opt_descriptor){

	vector <struct xtract_layer_type> xtract_layer_type_e;
	get_hw_and_sw_layer(xtract_layer_type_e,opt_descriptor);
	io_mem_data_type_e tmp_m_d_type;
	_datatype_e tmp_data_type;
	tmp_m_d_type.io_mem_type ="non_cacheable";
	tmp_m_d_type.io_data_type = XI_INT8;

	mem_type_table[Bdepth[0].blobs_name[0]]=tmp_m_d_type;

	for(int iter_d=1;iter_d<Bdepth.size();iter_d++){

		int hs_ops_type=-1;

		int p_hw_type=0;
		int p_sw_type=0;
		int c_hw_type=0;
		int c_sw_type=0;

		for(int iter_db=0;iter_db<Bdepth[iter_d].blobs_name.size();iter_db++){ // data is input buffer,so we are starting from 1 index

			map < string, XBlob* >::iterator blob_it = xgraph->blobs.find(Bdepth[iter_d].blobs_name[iter_db]);
			vector< string > vect_consumer_layer_type;
			vector< string > vect_producer_layer_type;
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

				if(curr_blob_p_type.compare("XPack")==0){

					map < string, XLayer* >::iterator it_l_pack=xgraph->layers.find(cur_blob_p_name);

					if(it_l_pack->second->xpack_params->pack==0){
						vect_producer_layer_type.push_back("XUnPack_conv");
					}else if(it_l_pack->second->xpack_params->pack==1){
						vect_producer_layer_type.push_back("XPack");
					}else if(it_l_pack->second->xpack_params->pack==2){
						vect_producer_layer_type.push_back("XPack_sf");
					}else{
						if(it_l_pack->second->xpack_params->pack==3){
							vect_producer_layer_type.push_back("XUnPack_permute");
						}
					}
				}else{
					vect_producer_layer_type.push_back(curr_blob_p_type);
				}
				find_hw_and_sw_layer_type(xtract_layer_type_e,curr_blob_p_type,hs_ops_type);

				if(hs_ops_type==FPGA)// ops layer type is hardware
					p_hw_type=1;
				else
					p_sw_type=1;
			}

			for(int iter_b_c_c=0;iter_b_c_c<blob_it->second->consumers.size();iter_b_c_c++){

				string cur_blob_c_name = blob_it->second->consumers[iter_b_c_c];
				map<string, string>::iterator c_it = layer_type_table.find(cur_blob_c_name);
				string curr_blob_c_type = c_it->second;

				//vect_consumer_layer_type.push_back(curr_blob_c_type);

				if(curr_blob_c_type.compare("XPack")==0){

					map < string, XLayer* >::iterator it_l_pack1 = xgraph->layers.find(cur_blob_c_name);

					if(it_l_pack1->second->xpack_params->pack==0){
						vect_consumer_layer_type.push_back("XUnPack_conv");
					}else if(it_l_pack1->second->xpack_params->pack==1){
						vect_consumer_layer_type.push_back("XPack");
					}else if(it_l_pack1->second->xpack_params->pack==2){
						vect_consumer_layer_type.push_back("XPack_sf");
					}else{
						if(it_l_pack1->second->xpack_params->pack==3){
							vect_consumer_layer_type.push_back("XUnPack_permute");
						}
					}
				}else{
					vect_consumer_layer_type.push_back(curr_blob_c_type);
				}

				find_hw_and_sw_layer_type(xtract_layer_type_e,curr_blob_c_type,hs_ops_type);

				if(hs_ops_type==FPGA)// ops layer type is hardware
					c_hw_type=1;
				else
					c_sw_type=1;
			}

			find_blob_data_type(vect_producer_layer_type,vect_consumer_layer_type,LayerIOType,tmp_data_type);

			if(p_hw_type==1 && p_sw_type==1){

				if(c_hw_type==1 && c_sw_type==1){

					tmp_m_d_type.io_mem_type ="non_cacheable";
					tmp_m_d_type.io_data_type = tmp_data_type;
					mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]=tmp_m_d_type;

				}
				else if(c_hw_type==0 && c_sw_type==1)
				{
					//mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="cacheable";
					tmp_m_d_type.io_mem_type ="cacheable";
					tmp_m_d_type.io_data_type = tmp_data_type;
					mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]=tmp_m_d_type;
				}else
				{
					if(c_hw_type==1 && c_sw_type==0){
						tmp_m_d_type.io_mem_type ="non_cacheable";
						tmp_m_d_type.io_data_type = tmp_data_type;
						mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]=tmp_m_d_type;
						//mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="non_cacheable";
					}else{
						if(blob_it->second->consumers.size()==0)
						{
							tmp_m_d_type.io_mem_type ="cacheable";
							tmp_m_d_type.io_data_type = tmp_data_type;
							mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]=tmp_m_d_type;
						}
					}
				}
			}
			else if(p_hw_type==0 && p_sw_type==1)
			{
				if(c_hw_type==1 && c_sw_type==1)
				{
					//mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="non_cacheable";
					tmp_m_d_type.io_mem_type ="non_cacheable";
					tmp_m_d_type.io_data_type = tmp_data_type;
					mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]=tmp_m_d_type;
				}
				else if(c_hw_type==0 && c_sw_type==1){
					//mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="malloc";
					tmp_m_d_type.io_mem_type ="malloc";
					tmp_m_d_type.io_data_type = tmp_data_type;
					mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]=tmp_m_d_type;
				}
				else
				{
					if(c_hw_type==1 && c_sw_type==0){
						//mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="non_cacheable";
						tmp_m_d_type.io_mem_type ="non_cacheable";
						tmp_m_d_type.io_data_type = tmp_data_type;
						mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]=tmp_m_d_type;
					}

					else{
						if(blob_it->second->consumers.size()==0)
						{
							//mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="malloc";
							tmp_m_d_type.io_mem_type ="malloc";
							tmp_m_d_type.io_data_type = tmp_data_type;
							mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]=tmp_m_d_type;

						}else{

							cerr<<" find_mem_type_io_buffers invalid consumers type"<<endl;
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
						//mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="non_cacheable";
						tmp_m_d_type.io_mem_type ="non_cacheable";
						tmp_m_d_type.io_data_type = tmp_data_type;
						mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]=tmp_m_d_type;
					}
					else if(c_hw_type==0 && c_sw_type==1){

						//mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="cacheable";
						tmp_m_d_type.io_mem_type ="cacheable";
						tmp_m_d_type.io_data_type = tmp_data_type;
						mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]=tmp_m_d_type;
					}
					else
					{
						//mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]="non_cacheable";
						tmp_m_d_type.io_mem_type ="non_cacheable";
						tmp_m_d_type.io_data_type = tmp_data_type;
						mem_type_table[Bdepth[iter_d].blobs_name[iter_db]]=tmp_m_d_type;
					}
				}
				else{
					cerr<<"find_mem_type_io_buffers invalid producers type p_sw_type = p_hw_type= "<<p_sw_type<<p_hw_type<<endl;
					return;
				}

			}
		}
	}
}

void update_io_mem_type(vector < XlayerData > & xlayer_seq,XGraph *xgraph,vector < BDegree > &Bdepth,map<string, string> &layer_type_table,map<string, io_mem_data_type_e> &mem_type_table,map<string,io_type_e> &LayerIOType,kernelInfo& opt_descriptor){


	vector <int> max_shape;

	maxsize_blob(xgraph,max_shape);

	vector <struct xtract_layer_type> xtract_layer_type_e;
	get_hw_and_sw_layer(xtract_layer_type_e,opt_descriptor);

	find_mem_type_io_buffers(xgraph,Bdepth,layer_type_table,mem_type_table,LayerIOType,opt_descriptor);

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

		find_hw_and_sw_layer_type(xtract_layer_type_e,curr_layer_type,hs_ops_type);

		if(hs_ops_type==FPGA)// ops layer type is hardware
			xlayer_seq[iter_l].layer_type="hardware";
		else
			xlayer_seq[iter_l].layer_type="software";

		xlayer_seq[iter_l].opcode = xlayer_seq[iter_l].hw_ops->opcode;
		for(int iter_l_b_c=0;iter_l_b_c<xlayer_seq[iter_l].hw_ops->bottom.size();iter_l_b_c++){

			HBlob tmp_hblob_ip;
			map<string, io_mem_data_type_e>::iterator mem_it = mem_type_table.find(xlayer_seq[iter_l].hw_ops->bottom[iter_l_b_c].blob->name);
			tmp_hblob_ip.handle = mem_it->first;
			tmp_hblob_ip.pos = xlayer_seq[iter_l].hw_ops->bottom[iter_l_b_c].id;
			tmp_hblob_ip.handle_shape=max_shape;
			tmp_hblob_ip.mem_type = mem_it->second.io_mem_type;
			tmp_hblob_ip.io_data_type = mem_it->second.io_data_type;
			xlayer_seq[iter_l].ip_blob.push_back(tmp_hblob_ip);
		}
		for(int iter_l_t_c=0;iter_l_t_c<xlayer_seq[iter_l].hw_ops->top.size();iter_l_t_c++){

			HBlob tmp_hblob_op;
			map<string, io_mem_data_type_e>::iterator mem_it = mem_type_table.find(xlayer_seq[iter_l].hw_ops->top[iter_l_t_c].blob->name);
			tmp_hblob_op.handle = mem_it->first;
			tmp_hblob_op.pos = xlayer_seq[iter_l].hw_ops->top[iter_l_t_c].id;
			tmp_hblob_op.handle_shape=max_shape;
			tmp_hblob_op.mem_type = mem_it->second.io_mem_type;
			tmp_hblob_op.io_data_type = mem_it->second.io_data_type;
			xlayer_seq[iter_l].op_blob.push_back(tmp_hblob_op);
		}
	}
}
void update_buffer_reuse_xlayer_seq(vector < XlayerData > & xlayer_seq,vector < BDegree > &Bdepth,map<string, int> &layer_seq_table,map<string, io_mem_data_type_e> &mem_type_table,XGraph *xgraph,vector < BDegree > &reUseBdepth){


	vector< Handle_depth_info > free_handles;
	BDegree tmp_Bdepth;
	int sw=1;
	map<string,int> reuse_buff_table;
	Handle_depth_info *tmp_handle_info =   new Handle_depth_info();
	string input_blob_name = xgraph->input_blob;
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
			layer_sw=0;
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

				//				if(reUseBdepth[iter_bg-2].blobs_name[iter_b].compare("data")!=0){
				if(reUseBdepth[iter_bg-2].blobs_name[iter_b].compare(input_blob_name)!=0){

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

					check_free_handle_layers_indx_with_curr_blob_layers_indx(Bdepth,xlayer_seq,xgraph,layer_seq_table,handle_degree_num,handle_blob_num,iter_bg,iter_b,&res);

					if(res==1)
						continue;

					int flag_handle_not_use=0;
					vector<int> curr_con_layer_ind;
					vector <int> consumers_handle_list;
					//map<string, string>::iterator mem_type_it = mem_type_table.find(handle_name);

					map<string, io_mem_data_type_e>::iterator mem_type_it = mem_type_table.find(handle_name);

					string free_handle_mem_type_name = mem_type_it->second.io_mem_type;
					_datatype_e free_handle_data_type_name = mem_type_it->second.io_data_type;

					map<string, io_mem_data_type_e>::iterator tmp_mem_type_it = mem_type_table.find(blob_name);

					if((free_handle_mem_type_name.compare(tmp_mem_type_it->second.io_mem_type)==0 )&& (free_handle_data_type_name==tmp_mem_type_it->second.io_data_type)){

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

void update_xlayer_opcode(XGraph *xgraph,kernelInfo& opt_descriptor)
{
	vector < struct xtract_opcode_num> xlayers_opcode;

	get_opcode_xlayer(xlayers_opcode,opt_descriptor);

	int hs_ops_type=0;
	for(map < string, XLayer*>::iterator it = xgraph->layers.begin();it !=xgraph->layers.end();it++){

		find_xlayer_opcode(xlayers_opcode,it->second->type,hs_ops_type);
		if(it->second->opcode<0){
			it->second->opcode = hs_ops_type;
		}
	}

}

void check_free_handle_layers_indx_with_curr_blob_layers_indx(vector < BDegree > &Bdepth,vector < XlayerData > & xlayer_seq,XGraph *xgraph,map<string, int> &layer_seq_table,int free_handle_degree_num,int free_handle_blob_num,int curr_degree_num,int curr_blob_num,int *res){


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
		if(tmp_curr_pro_indx==0)
			tmp_curr_pro_indx=ind_l;
		if(ind_l<tmp_curr_pro_indx)
			tmp_curr_pro_indx=ind_l;


	}

	for(int iter_cur_b_c=0;iter_cur_b_c<curr_blob_it->second->consumers.size();iter_cur_b_c++){

		map<string, int>::iterator table_it_l = layer_seq_table.find(curr_blob_it->second->consumers[iter_cur_b_c]);

		int ind_l = table_it_l->second;

		if(ind_l>tmp_curr_cons_indx)
			tmp_curr_cons_indx=ind_l;
	}
	/*
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
	}*/
	if(tmp_free_cons_indx<tmp_curr_pro_indx){

		*res=0;

	}else{
		*res=1;
	}
}

void find_xlayer_opcode(vector < struct xtract_opcode_num> xlayers_opcode_e,string &layer_type,int &hs_ops_type){

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
		cout<<"find_xlayer_opcode:Layer is find "<<"layer type:"<<layer_type<<endl;
#endif
	}
	else{
		iter_lt=iter_lt-1;
		ELOG(!(xlayers_opcode_e[iter_lt].layer_name.compare(layer_type)==0),"EO005", "Layer opcode: Layer is not support : "<< layer_type);
	}
}


// void checkGraphInsanity(XGraph* graph)
// {
// 	//  #. Check if input shape, resize shape & mean shape are OK.
// 	int nchannels = graph->input_blob.blob->shape.at(1);
// 	int input_height = graph->input_blob.blob->shape.at(2);
// 	int input_width = graph->input_blob.blob->shape.at(3);
// 
// 	int resize_height = graph->transform_params.resize_height;
// 	int resize_width = graph->transform_params.resize_width;
// 
// 	// If mean data has only one value or no value, then there is no issue at all.
// 	if(graph->meanData.empty())
// 		return;
// 
// 	if(graph->meanData.size() > 1)
// 	{
// 		// If mean has same number of elements as number of channels, then also no issue
// 		if(graph->meanData.size() != nchannels)
// 		{
// 			// If resize shape is mentioned, number of elements in resized image and mean should be same
// 			// If not specified, number of elements in input image and mean should be same.
// 			if((resize_height > 0) && (resize_width > 0))
// 			{
// 				ELOG(   (nchannels * resize_height * resize_width != graph->meanData.size()),
// 						EO006, "Number of elements in the mean data doesn't match total pixels as per resize dimensions : "
// 						<< nchannels*resize_height*resize_width << " v/s " << graph->meanData.size() );
// 			}
// 			else
// 			{
// 				ELOG(   (nchannels * input_height * input_width != graph->meanData.size()),
// 						EO007, "Number of elements in the mean data doesn't match total pixels as per input dimensions : "
// 						<< nchannels*input_height*input_width << " v/s " << graph->meanData.size() );
// 			}
// 		}
// 	}
// }

void find_max_q_factor(XGraph* xgraph_opt){



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
				cout<<"layer name "<<consumer_name<<" layer_it->second->ip_bw "<<layer_it->second->ip_bw<<" layer_it->second->ip_fl "<<layer_it->second->ip_fl<<endl;

			}
			for(int iter_pro_c=0;iter_pro_c<it->second->producers.size();iter_pro_c++){

				string producer_name = it->second->producers[iter_pro_c];
				map < string, XLayer*>::iterator layer_it = xgraph_opt->layers.find(producer_name);
				layer_it->second->op_bw=Qfactor_max+Ffactor_max;
				layer_it->second->op_fl=Ffactor_max;
				cout<<"layer name "<<producer_name<<" layer_it->second->op_bw "<<layer_it->second->op_bw<<" layer_it->second->op_fl "<<layer_it->second->op_fl<<endl;
			}
		}
	}
}
