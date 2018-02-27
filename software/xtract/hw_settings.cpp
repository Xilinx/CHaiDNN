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


#include "../include/hw_settings.h"
using namespace std;
void xi_get_hw_and_sw_layer(vector <struct xtract_layer_type> &xtract_layer){

	xtract_layer_type tmp;

	string *_conv_type = new string("Convolution");
	string *_pooling_type = new string("Pooling");
	string *_softmax_type = new string("Softmax");
	string *_argmax_type = new string("Argmax");
	string *_deconv_type = new string("Deconvolution");
	string *_innerproduct_type = new string("InnerProduct");
	string *_nms_type = new string("NMS");
	string *_permute_type = new  string("Permute");
	string *_crop_type = new  string("Crop");
	string *_l2norm_type =  new string("L2Normalize");
	string *_eltwise_type =  new string("Eltwise");

	tmp.layer_name=*_eltwise_type;
	tmp.layer_type = EXE_ON_SW;
	xtract_layer.push_back(tmp);
	
	tmp.layer_name=*_conv_type;
	tmp.layer_type = EXE_ON_HW;
	xtract_layer.push_back(tmp);

	tmp.layer_name=*_pooling_type;
	tmp.layer_type = EXE_ON_HW;
	xtract_layer.push_back(tmp);

	tmp.layer_name=*_softmax_type;
	tmp.layer_type = EXE_ON_SW;
	xtract_layer.push_back(tmp);

	tmp.layer_name=*_deconv_type;
	tmp.layer_type = EXE_ON_HW;
	xtract_layer.push_back(tmp);
	tmp.layer_name=*_innerproduct_type;
	tmp.layer_type = EXE_ON_HW;
	xtract_layer.push_back(tmp);

	tmp.layer_name=*_nms_type;
	tmp.layer_type = EXE_ON_SW;
	xtract_layer.push_back(tmp);

	tmp.layer_name=*_permute_type;
	tmp.layer_type = EXE_ON_SW;
	xtract_layer.push_back(tmp);

	tmp.layer_name=*_crop_type;
	tmp.layer_type = EXE_ON_SW;
	xtract_layer.push_back(tmp);

	tmp.layer_name=*_argmax_type;
	tmp.layer_type = EXE_ON_SW;
	xtract_layer.push_back(tmp);

	tmp.layer_name=*_l2norm_type;
	tmp.layer_type = EXE_ON_SW;
	xtract_layer.push_back(tmp);

	delete _conv_type;
	delete _pooling_type;
	delete _softmax_type;
	delete _argmax_type;
	delete _deconv_type;
	delete _innerproduct_type;
	delete _nms_type;
	delete _permute_type;
	delete _crop_type;
	delete _l2norm_type;


}
void xi_get_opcode_xlayer(vector < struct xtract_opcode_num> &xlayers_opcode){

	string *_conv_type = new string("Convolution");
	string *_pooling_type = new string("Pooling");
	string *_softmax_type = new string("Softmax");
	string *_argmax_type = new string("Argmax");
	string *_deconv_type = new string("Deconvolution");
	string *_innerproduct_type = new string("InnerProduct");
	string *_nms_type = new string("NMS");
	string *_permute_type = new  string("Permute");
	string *_crop_type = new  string("Crop");
	string *_lrn_type = new string("LRN");
	string *_l2norm_type =  new string("L2Normalize");
	string *_tmp_relu_type =  new string("ReLU");
	string *_tmp_scale_type =  new string("Scale");
	string *_tmp_eltwise_type =  new string("Eltwise");
	string *_tmp_concat_type =  new string("Concat");
	string *_tmp_batchnorm_type =  new string("BatchNorm");
	string *_tmp_power_type =  new string("Power");


	xtract_opcode_num tmp;


	tmp.layer_name=*_tmp_power_type;
	tmp.opcode = OPCODE_TMP_POWER;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_tmp_batchnorm_type;
	tmp.opcode = OPCODE_TMP_BN;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_tmp_concat_type;
	tmp.opcode = OPCODE_Concat;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_tmp_relu_type;
	tmp.opcode = OPCODE_ReLU;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_tmp_scale_type;
	tmp.opcode = OPCODE_BN;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_tmp_eltwise_type;
	tmp.opcode = OPCODE_ELTWISE;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_conv_type;
	tmp.opcode = OPCODE_CONV;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_lrn_type;
	tmp.opcode = OPCODE_TMP_LRN;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_pooling_type;
	tmp.opcode = OPCODE_POOL;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_softmax_type;
	tmp.opcode = OPCODE_SOFTMAX;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_deconv_type;
	tmp.opcode = OPCODE_DECONV;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_innerproduct_type;
	tmp.opcode = OPCODE_FC;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_nms_type;
	tmp.opcode = OPCODE_NMS;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_permute_type;
	tmp.opcode = OPCODE_PERM;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_crop_type;
	tmp.opcode = OPCODE_CROP;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_argmax_type;
	tmp.opcode = OPCODE_ARGMAX;
	xlayers_opcode.push_back(tmp);

	tmp.layer_name=*_l2norm_type;
	tmp.opcode = OPCODE_L2NORM;
	xlayers_opcode.push_back(tmp);

	delete _conv_type;
	delete _pooling_type;
	delete _softmax_type;
	delete _argmax_type;
	delete _deconv_type;
	delete _innerproduct_type;
	delete _nms_type;
	delete _permute_type;
	delete _crop_type;
	delete _lrn_type;
	delete _l2norm_type;

}

