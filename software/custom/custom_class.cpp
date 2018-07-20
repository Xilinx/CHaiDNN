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

#include "custom_class.hpp"
#include <stdio.h>
#include <string.h>

void LRN_AcrossChannel(  const float* src, float *dst, const vector<int> dim, const int local_size,
		const float alpha, const float beta)
{
	int batch = 1;//dim[0];
	int channels = dim[1];
	int height = dim[2];
	int width = dim[3];
	int dst_cnt = 0;
	int N = batch*channels*height*width;

	float a_by_n = alpha / (static_cast<float>(local_size));
	int half_size = local_size / 2;
	int channel_size = height * width;
	int batch_size = channels * channel_size;

	//vector<float> dst; dst.reserve(N);

	// Square the input
	vector<float> sq; sq.reserve(N);
	for(int i=0; i<N; ++i)
		sq.push_back(src[i]*src[i]);

	// Do sliding window (1 + a/n * x.sum()) ^ b
	for(int b=0; b<batch; ++b) {
		for(int c=0; c<channels; ++c) {
			for(int h=0; h<height; ++h) {
				for(int w=0; w<width; ++w) {
					// For each output pixel, iterate over the sliding window
					int start_c = max(0, c-half_size);
					int end_c = min(channels-1, c+half_size);
					float sum = 0.0f;
					for(int kc=start_c; kc<=end_c; ++kc) {
						int ind = b*batch_size + kc*channel_size + h*width + w;
						float val= sq[ind];
						sum += val;
					}
					float tmp = 1.0f + a_by_n * sum;
					float tmp2= powf(tmp, beta);
					float res = src[b*batch_size + c*channel_size + h*width + w] / tmp2;
					//dst.push_back(res);
					dst[dst_cnt++] = res;
				}
			}
		}
	}

	//return dst;
}


void Frcnn_LRN(const float *src, float *dst,const vector<int> dim, const int local_size=3,
		const float alpha=0.001, const float beta=0.75)//, const string& norm_region="WITHIN_CHANNEL")
{

	const string& norm_region="WITHIN_CHANNEL";
	int batch = 1;//dim[0];
	int channels = dim[1];
	int height = dim[2];
	int width = dim[3];

	int N = batch*channels*height*width;

	float a_by_n = alpha / (static_cast<float>(local_size)*static_cast<float>(local_size));
	int half_size = local_size / 2;
	int channel_size = height * width;
	int batch_size = channels * channel_size;
	int dst_cnt=0;

	// Square the input
	vector<float> sq; sq.reserve(N);
	for(int i=0; i<N; ++i)
		sq.push_back(src[i]*src[i]);

	// Do sliding window (1 + a/n * x.sum()) ^ b
	for(int b=0; b<batch; ++b) {
		for(int c=0; c<channels; ++c) {
			for(int h=0; h<height; ++h) {
				for(int w=0; w<width; ++w) {
					// For each output pixel, iterate over the sliding window
					int start_h = max(0, h-half_size);
					int start_w = max(0, w-half_size);
					int end_h = min(height-1, h+half_size);
					int end_w = min(width-1, w+half_size);
					float sum = 0.0f;
					float window_size = (end_h-start_h+1) * (end_w-start_w+1);
					for(int kh=start_h; kh<=end_h; ++kh) {
						for(int kw=start_w; kw<=end_w; ++kw) {
							int ind = b*batch_size + c*channel_size + kh*width + kw;
							float val= sq[ind];
							sum += val;
						}
					}
					float tmp = 1.0f + a_by_n * sum;
					float tmp2= powf(tmp, beta);
					float res = src[b*batch_size + c*channel_size + h*width + w] / tmp2;
					//dst.push_back(res);
					dst[dst_cnt++]=res;
				}
			}
		}
	}
}

void Custom::custom_norm(const xChangeLayer *xchange){



	float *In_ptr;

	float *out_ptr;

	vector<int> out_dim;
	vector<int> in_dim;
	int batch ;
	int channels;
	int height;
	int width;

	int iN=0,oN=0;
	int off = 0;

	int local_size = (int)xchange->float_args[off+0];
	float alpha = xchange->float_args[off+1];
	float beta =xchange->float_args[off+2];
	for(int it_num_in=0;it_num_in<xchange->input_dims.size();it_num_in++){

		in_dim=xchange->input_dims[it_num_in];

		channels = in_dim[1];
		height = in_dim[2];
		width = in_dim[3];
		iN = channels*height*width;

	}

	for(int it_num_out=0;it_num_out<xchange->output_dims.size();it_num_out++){

		out_dim=xchange->output_dims[it_num_out];
		channels = out_dim[1];
		height = out_dim[2];
		width = out_dim[3];
		oN = channels*height*width;

	}
	batch = in_dim[0]; // default batch size = 2
	for(int batch_num=0;batch_num<batch;batch_num++){
		In_ptr = (float*)xchange->in_ptrs[0]+batch_num*iN;
		out_ptr = (float*)xchange->out_ptrs[0]+batch_num*oN;
		LRN_AcrossChannel(In_ptr, out_ptr, out_dim, local_size, alpha, beta);
	}
}

void func_memcopy(const float *In_ptr,float *out_ptr,int in_height,int in_width,int out_height,int out_width,int depth){


	memcpy((float*)out_ptr,(const float*)In_ptr,in_height*in_width*depth*sizeof(float));

}

Custom::Custom()
{
	//cout<<"obj create"<<endl;

	custom_func_keys["LRN"]= &Custom::custom_norm;
	custom_func_keys["gemm"]= &Custom::custom_gemm;
	custom_func_keys["identity"]= &Custom::custom_function;
	custom_func_keys["Python"]=&Custom::custom_reorg;
	custom_func_keys["Python_passthrough"]= &Custom::custom_passthrough;
}

Custom::~Custom()
{

}


void Custom::custom(const string before_processing,const xChangeLayer *xchange)
{


	map<string, functionptr>::iterator result = custom_func_keys.find(before_processing);
	if(result != custom_func_keys.end()){

			(this->*(result->second))(xchange);
	}
}

void Frcnn_gemm(const float *src, const vector<float> &weightsT, const vector<float>& bias, float *dst, int M, int N, int O){


	memcpy(dst, &bias[0], O*sizeof(float));

	for(int m=0; m < M; ++m) {
		for(int n=0; n<N; ++n) {
			float xval = src[m*N + n];
			for(int o=0; o < O; ++o) {
				dst[m*O + o] += xval * weightsT[n*O + o];
			}
		}
	}

}
void Custom::custom_gemm(const xChangeLayer *xchange ){

	float *In_ptr;

	float *out_ptr;

	int num_outputs = (int)xchange->float_args[0];

	vector<int> out_dim=xchange->output_dims[0];
	vector<int> weights_dim = xchange->params_dims[0];
	vector<int> in_dim = xchange->input_dims[0];
	int in_size=1;
	int out_size=1;

	for(int it_num_in=1;it_num_in<in_dim.size();it_num_in++){
		in_size=in_size*in_dim[it_num_in];
	}
	for(int it_num_out=1;it_num_out<out_dim.size();it_num_out++){

		out_size =  out_size*out_dim[it_num_out];

	}

	int M = in_dim.at(0);
	int N = in_dim.at(1);
	int batch = M;
	
	int O = num_outputs;
	
	vector<float> weights = xchange->custom_float_params[0]; // OxN
	vector<float> bias = xchange->custom_float_params[1];

	
	
	vector<float> weights_transpose; weights_transpose.resize(weights.size()); // NxO
	for(int i=0; i<O; ++i) {
		for(int j=0; j<N; ++j) {
			weights_transpose[j*O + i] = weights[i*N + j];
		}
	}
	
	for(int batch_num=0;batch_num<batch;batch_num++){

		In_ptr = (float*)xchange->in_ptrs[0]+batch_num*in_size;

		out_ptr = (float*)xchange->out_ptrs[0]+batch_num*out_size;
		
		Frcnn_gemm(In_ptr, weights_transpose, bias, out_ptr, 1, N, O);
	}

}

void Custom::custom_function(const xChangeLayer *xchange)
{

	cout << "enters custom_function"<<endl;

	vector<int> out_dim=xchange->output_dims[0];
	float *In_ptr = (float*)xchange->in_ptrs[0];
	float *out_ptr = (float*)xchange->out_ptrs[0];

	int out_width = out_dim.at(3);
	int out_height = out_dim.at(2);
	int depth =  out_dim.at(1);

	FILE *fin_cust=fopen("Cust_input_pre.txt", "w");

	if(fin_cust == NULL)
	{
		std :: cout << "File not found - " << "Cust_input.txt" << std :: endl;
		return;
	}
	for(int iter_p=0;iter_p<depth;iter_p++){

		for(int iter_h=0;iter_h<out_height;iter_h++){

			for(int iter_w=0;iter_w<out_width;iter_w++){

				fprintf(fin_cust, "%f\n", In_ptr[iter_p*out_height*out_width+iter_h*out_width+iter_w]);
			}
		}
	}

	fclose(fin_cust);
	vector<int> in_dim;


	for(int iter_i=0;iter_i<xchange->input_dims.size();iter_i++){

		for(int iter_ii=0;iter_ii<xchange->input_dims[iter_i].size();iter_ii++){

			in_dim.push_back(xchange->input_dims[iter_i][iter_ii]);
		}
	}



	for(int iter_i=0;iter_i<xchange->output_dims.size();iter_i++){
		for(int iter_ii=0;iter_ii<xchange->output_dims[iter_i].size();iter_ii++){
			in_dim.push_back(xchange->output_dims[iter_i][iter_ii]);
		}
	}
	int in_width = in_dim.at(3);
	int in_height = in_dim.at(2);

	func_memcopy(In_ptr,out_ptr,in_height,in_width,out_height,out_width,depth);
}

void reorg_cpu(float *x, int w, int h, int c, int batch, int stride, int forward, float *out)
{
	int b,i,j,k;
	int out_c = c/(stride*stride);

	for(b = 0; b < batch; ++b){
		for(k = 0; k < c; ++k){
			for(j = 0; j < h; ++j){
				for(i = 0; i < w; ++i){
					int in_index  = i + w*(j + h*(k + c*b));
					int c2 = k % out_c;
					int offset = k / out_c;
					int w2 = i*stride + offset % stride;
					int h2 = j*stride + offset / stride;
					int out_index = w2 + w*stride*(h2 + h*stride*(c2 + out_c*b));
					//		    printf("%d %d\n",out_index,in_index);
					if(forward)
						out[out_index] = x[in_index];
					else out[in_index] = x[out_index];
				}
			}
		}
	}
}

void Custom::custom_reorg(const xChangeLayer *xchange){

	float *In_ptr;

	float *out_ptr;

	vector<int> out_dim=xchange->output_dims[0];
	vector<int> in_dim = xchange->input_dims[0];

	int in_size=1;
	int out_size=1;
	int batch = in_dim[0];
	for(int it_num_in=1;it_num_in<in_dim.size();it_num_in++){
		in_size=in_size*in_dim[it_num_in];
	}


	for(int it_num_out=1;it_num_out<out_dim.size();it_num_out++){

		out_size =  out_size*out_dim[it_num_out];

	}

	int c = in_dim.at(1);
	int w = in_dim.at(2);
	int h = in_dim.at(3);
	int stride = (int)xchange->float_args[0];
	int forward=0;
	for(int batch_num=0;batch_num<batch;batch_num++){
		In_ptr = (float*)xchange->in_ptrs[0]+batch_num*in_size;
		out_ptr = (float*)xchange->out_ptrs[0]+batch_num*out_size;
		reorg_cpu(In_ptr, w, h, c, 1, stride, forward, out_ptr);
	}
}
void Custom::custom_passthrough(const xChangeLayer *xchange){

	float *In_ptr;//[INPUT_PORTS];

	float *out_ptr;//[OUTPUT_PORTS];

	vector<int> out_dim=xchange->output_dims[0];
	vector<int> in_dim = xchange->input_dims[0];

	int in_size=1;
	int out_size=1;
	int batch=in_dim[0];
	for(int it_num_in=1;it_num_in<in_dim.size();it_num_in++){
		in_size=in_size*in_dim[it_num_in];
	}


	for(int it_num_out=1;it_num_out<out_dim.size();it_num_out++){

		out_size =  out_size*out_dim[it_num_out];

	}

	int c = in_dim.at(1);
	int w = in_dim.at(2);
	int h = in_dim.at(3);
	for(int batch_num=0;batch_num<batch;batch_num++){
		In_ptr = (float*)xchange->in_ptrs[0]+batch_num*in_size;
		out_ptr = (float*)xchange->out_ptrs[0]+batch_num*out_size;
		memcpy((float*)out_ptr,(const float*)In_ptr,h*w*c*sizeof(float));
	}
}
