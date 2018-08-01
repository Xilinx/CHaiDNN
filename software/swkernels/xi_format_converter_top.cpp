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
#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <numeric>
#include <cfloat>
#include <cmath>
#include <ctime>
#include <cstring>
#include "xi_format_converter.hpp"

#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)
#define ConvertToFP(fVal, iPart, fbits)	((int)((iPart<<fbits) + ((fVal-(float)iPart))*(1<<fbits)))
#define ASSERT(condition, code, message)   \
		if(!(condition))                     \
		{                                   \
			cerr << "[" << #code << "] ";    \
			cerr << message << endl;        \
			exit(-1);                       \
		}
//#ifdef __SDSOC
//#include <sds_lib.h>
//#define PERFORMANCE_CHECK2			0
//long long int clock_start_norm, clock_end_norm;
//#endif

template<class T>
void unpackDataToSoftmax(T* conv_out,int width,int indepth,int fbits,float &sf,float &th,string &quant,float *software_output){

	cout<<"enter unpackDataToSoftmax" <<endl;
#if FILE_WRITE
	FILE *fout_pack=fopen("softmax_unpack_output.txt", "w");
	FILE *fin_pack=fopen("softmax_unpack_in.txt", "w");
	FILE *ffin_pack=fopen("fc_out.txt", "w");
	if(fin_pack == NULL)
	{
		std :: cout << "File not found - " << "pack_output.txt" << std :: endl;
		return;
	}
#endif


	int hw_buf=0;
	if(quant.compare("Xilinx")==0){
		for(int i=0;i<(indepth);i++)
		{
			for(int it_btch=0;it_btch<XBATCH_SIZE;it_btch++)
			{

				int iter_soft_buf = it_btch*indepth+i;
				float temp_val;
				T fxval2=conv_out[hw_buf++];

				software_output[iter_soft_buf]=((float)fxval2)*sf;
#if FILE_WRITE
				fprintf(fout_pack, "%d\n ", fxval2);
				//fprintf(fin_pack, "%f\n ", f_val);
				//fprintf(ffin_pack, "%f\n ", fc_val);
#endif

			}
		}
	}
	else if(quant.compare("DynamicFixed")==0){
		for(int i=0;i<(indepth);i++)
		{
			for(int it_btch=0;it_btch<XBATCH_SIZE;it_btch++)
			{

				int iter_soft_buf = it_btch*indepth+i;
				T fxval2=conv_out[hw_buf++];
				float temp_val= ((float)fxval2)/(1 << fbits);
				software_output[iter_soft_buf]=temp_val;


#if FILE_WRITE
				fprintf(fout_pack, "%d\n ", fxval2);
				//fprintf(fin_pack, "%f\n ", f_val);
				//fprintf(ffin_pack, "%f\n ", fc_val);
#endif

			}
		}
	}
#if FILE_WRITE
	fclose(fin_pack);
	fclose(fout_pack);
	fclose(ffin_pack);
#endif
}

template<class T>
void unPackPermutetocustom(T* conv_out,int height,int width,int indepth,int fbits,float &sf,float &th,string &quant,float *software_output){

	T hls_val;
	int cnt1=0;

	if(quant.compare("Xilinx")==0){

		for(int iter_depth=0;iter_depth<indepth;iter_depth++){

			for(int i=0;i<height;i++){

				for(int j=0;j<width;j++){

					for(int planes=0;planes<PACK_ELEMS;planes++){

						for(int it_btch=0;it_btch<XBATCH_SIZE;it_btch++){

							hls_val = conv_out[cnt1];
							cnt1++;
							int iter_soft_buf = iter_depth*width*height + i*width +j + it_btch*width*height*indepth;

							float temp_val= (float)hls_val* sf;
							software_output[iter_soft_buf] = temp_val;

						}
					}
				}
			}
		}
	}
	else if(quant.compare("DynamicFixed")==0){
		for(int iter_depth=0;iter_depth<indepth;iter_depth++){

			for(int i=0;i<height;i++){

				for(int j=0;j<width;j++){

					for(int planes=0;planes<PACK_ELEMS;planes++){

						for(int it_btch=0;it_btch<XBATCH_SIZE;it_btch++){

							hls_val = conv_out[cnt1];
							cnt1++;
							int iter_soft_buf = iter_depth*width*height + i*width +j + it_btch*width*height*indepth;

							float temp_val= ((float)hls_val)/(1 << fbits);

							software_output[iter_soft_buf] = temp_val;

						}
					}
				}
			}
		}
	}
}

template<class T>
void packDataToSoftmax(float *software_output,int width,int indepth,int fbits,float &sf,float &th,string &quant,T* conv_out){

	cout<<"enter packDataToSoftmax" <<endl;
#if FILE_WRITE
	FILE *fout_pack=fopen("softmax_pack_output.txt", "w");
	FILE *fin_pack=fopen("softmax_pack_in.txt", "w");
	FILE *ffin_pack=fopen("fc_out.txt", "w");
	if(fin_pack == NULL)
	{
		std :: cout << "File not found - " << "pack_output.txt" << std :: endl;
		return;
	}
#endif

	int pack_indx=0;
	int fc_indx=0;
	if(quant.compare("Xilinx")==0){

		for(int i=0;i<(indepth);i++)
		{
			for(int it_btch=0;it_btch<XBATCH_SIZE;it_btch++){

				int iter_soft_buf = it_btch*indepth+i;


				float f_val = software_output[iter_soft_buf];
				float max_data = th;
				float min_data = -max_data;
				f_val = std::max(std::min(f_val, max_data), min_data);
				f_val =  (f_val)/sf;
				T fxval2 = (T)floorf(f_val + 0.5f);
				conv_out[pack_indx++] = fxval2;

#if FILE_WRITE
				fprintf(fout_pack, "%d\n ", fxval2);
				//fprintf(fin_pack, "%f\n ", f_val);
				//fprintf(ffin_pack, "%f\n ", fc_val);
#endif
				fc_indx++;
			}
		}
	}
	else if(quant.compare("DynamicFixed")==0){

		for(int i=0;i<(indepth);i++)
		{
			for(int it_btch=0;it_btch<XBATCH_SIZE;it_btch++){

				int iter_soft_buf = it_btch*indepth+i;


				float f_val = software_output[iter_soft_buf];

				T ival = (T)( f_val);
				int fxval = ConvertToFP(f_val, ival, fbits);
				int fxval2 = std::max(static_cast<int>(numeric_limits<T>::min()),
						std::min(fxval, static_cast<int>(numeric_limits<T>::max()) ) );
				conv_out[pack_indx++] = fxval2;

#if FILE_WRITE
				fprintf(fout_pack, "%d\n ", fxval2);
				//fprintf(fin_pack, "%f\n ", f_val);
				//fprintf(ffin_pack, "%f\n ", fc_val);
#endif
				fc_indx++;
			}
		}
	}
#if FILE_WRITE
	fclose(fin_pack);
	fclose(fout_pack);
	fclose(ffin_pack);
#endif
}

template<class T>
void PackDataToHardware(float *software_output,int height,int width,int indepth,int fbits,float &sf,float &th,string &quant,T* conv_out, T *conv_out1,int split){

	//fbits = 5;
	fprintf(stderr, "Pack Layer : (h, w, d, fbits ) : %d, %d, %d, %d\n", height, width, indepth, fbits);

	cout<<"enter PackDataToHardware" <<endl;
#if FILE_WRITE
	FILE *fin_pack=fopen("pack_output.txt", "w");
	if(fin_pack == NULL)
	{
		std :: cout << "File not found - " << "pack_output.txt" << std :: endl;
		return;
	}
#endif
	int cnt2 = 0;
	int cnt1 = 0;
	int cnt=0;
	int flag_check=0;
	T* tmp_ptr;
	int depth=AlignSize(indepth, PACK_ELEMS);


	if(quant.compare("DynamicFixed")==0){

		for(int iter_indepth = 0; iter_indepth < depth; iter_indepth += PACK_ELEMS)
		{
			if(flag_check==0){
				cnt2=cnt;
				flag_check=1;
				tmp_ptr=conv_out;
				cnt=cnt1;
			}
			else{
				cnt1=cnt;
				flag_check=0;
				tmp_ptr=conv_out1;
				cnt=cnt2;
			}

			for(int i=0;i<(height);i++)
			{
				for(int j=0;j<width;j++)
				{

					for(int planes = 0; planes < PACK_ELEMS; planes++)
					{

						int tmp_planes= planes;

						for(int it_btch=0;it_btch<XBATCH_SIZE;it_btch++){

							int iter_soft_buf = (tmp_planes+iter_indepth)*(width*height) + i*width + j + (it_btch*width*height*depth);

							float f_val = software_output[iter_soft_buf];

							T fxval;

							T ival = (T)( f_val);
							fxval = ConvertToFP(f_val, ival, fbits);

							tmp_ptr[cnt++] = fxval;

						}
					}
				}
			}
		}
	}
	else if(quant.compare("Xilinx")==0){

		for(int iter_indepth = 0; iter_indepth < depth; iter_indepth += PACK_ELEMS)
		{
			if(flag_check==0){
				cnt2=cnt;
				flag_check=1;
				tmp_ptr=conv_out;
				cnt=cnt1;
			}
			else{
				cnt1=cnt;
				flag_check=0;
				tmp_ptr=conv_out1;
				cnt=cnt2;
			}
			for(int i=0;i<(height);i++)
			{
				for(int j=0;j<width;j++)
				{

					for(int planes = 0; planes < PACK_ELEMS; planes++)
					{

						int tmp_planes= planes;

						for(int it_btch=0;it_btch<XBATCH_SIZE;it_btch++){

							int iter_soft_buf = (tmp_planes+iter_indepth)*(width*height) + i*width + j + (it_btch*width*height*depth);

							float f_val = software_output[iter_soft_buf];

							T fxval;

							float max_data = th;
							float min_data = -max_data;
							f_val = std::max(std::min(f_val, max_data), min_data);
							f_val = f_val/sf;
							fxval = (T)floorf(f_val + 0.5f);

							tmp_ptr[cnt++] = fxval;
						}
					}
				}
			}
		}
	}

	cout<<"done PackDataToHardware" <<endl;
#if FILE_WRITE
	fclose(fin_pack);
#endif
}

template<class T>
void UnpackDataToSoftware(T* conv_out, T *conv_out1, int height,int width,int indepth,int fbits,float &sf,float &th,string &quant,float *kernel_output){

#if FILE_WRITE
	fprintf(stderr, "Unpack Layer : (h, w, d, fbits ) : %d, %d, %d, %d\n", height, width, indepth, fbits);

	FILE *fin_pack=fopen("unpack_input.txt", "w");
	if(fin_pack == NULL)
	{
		std :: cout << "File not found - " << "unpack_input.txt" << std :: endl;
		return;
	}
#endif
	int depth;

	depth=AlignSize(indepth, PACK_ELEMS);

	T hls_val;
	int cnt2 = 0;
	int cnt1 = 0;
	int cnt=0;
	int flag_check=0;
	T *tmp_ptr;

	if(quant.compare("Xilinx")==0){

		for(int iter_depth=0;iter_depth<depth;iter_depth+=PACK_ELEMS){

			if(flag_check==0){
				cnt2=cnt;
				flag_check=1;
				tmp_ptr=conv_out;
				cnt=cnt1;
			}
			else{
				cnt1=cnt;
				flag_check=0;
				tmp_ptr=conv_out1;
				cnt=cnt2;
			}

			for(int i=0;i<height;i++){

				for(int j=0;j<width;j++){

					for(int planes=0;planes<PACK_ELEMS;planes++){

						for(int it_btch=0;it_btch<XBATCH_SIZE;it_btch++){

							//int plane = (iter_depth+i+j+planes)/(height*width);
							int tmp_planes = planes;
							int plane16 = planes%16; //

							hls_val=tmp_ptr[cnt++];

							int iter_soft_buf = (tmp_planes+iter_depth)*(width*height) + i*width +j + (it_btch*width*height*depth);

							float temp_val= (float)hls_val* sf;
							kernel_output[iter_soft_buf] = temp_val;

						}
					}
				}
			}
		}
	}
	else if(quant.compare("DynamicFixed")==0)
	{

		for(int iter_depth=0;iter_depth<depth;iter_depth+=PACK_ELEMS){

			if(flag_check==0){
				cnt2=cnt;
				flag_check=1;
				tmp_ptr=conv_out;
				cnt=cnt1;
			}
			else{
				cnt1=cnt;
				flag_check=0;
				tmp_ptr=conv_out1;
				cnt=cnt2;
			}

			for(int i=0;i<height;i++){

				for(int j=0;j<width;j++){

					for(int planes=0;planes<PACK_ELEMS;planes++){

						for(int it_btch=0;it_btch<XBATCH_SIZE;it_btch++){

							//int plane = (iter_depth+i+j+planes)/(height*width);
							int tmp_planes = planes;
							int plane16 = planes%16; //

							hls_val=tmp_ptr[cnt++];

							//if(plane16<PACK_ELEMS)
							//							if(flag_check)
							//							{
							//								hls_val = conv_out[cnt1];
							//#if FILE_WRITE
							//								fprintf(fin_pack, "%d\n", hls_val);
							//#endif
							//								cnt1++;
							//							}
							//							else
							//							{
							//								hls_val = conv_out1[cnt2];
							//#if FILE_WRITE
							//								fprintf(fin_pack, "%d\n", hls_val);
							//#endif
							//								cnt2++;
							//							}

							//int iter_soft_buf = (planes*(width*height)+j + i*width + ((width*height)*iter_depth));

							int iter_soft_buf = (tmp_planes+iter_depth)*(width*height) + i*width +j + (it_btch*width*height*depth);

							float temp_val= ((float)hls_val)/(1 << fbits);

							kernel_output[iter_soft_buf] = temp_val;

						}
					}
				}
			}
		}
	}
#if FILE_WRITE
	fclose(fin_pack);

	fin_pack=fopen("unpack_input_fc.txt", "w");
	if(fin_pack == NULL)
	{
		std :: cout << "File not found - " << "unpack_input_fc.txt" << std :: endl;
		return;
	}
	for(int iter_o=0;iter_o<depth*width*height;iter_o++){

		fprintf(fin_pack, "%f\n", kernel_output[iter_o]);
	}
	fclose(fin_pack);
#endif
}
template<class T>
void unpack_processed_ch(float *caffe_buff,T *conv_out,int ch_act,int min_ch,int in_hight,int in_width,int ch_total,float sft_fbits,float sf,float th,string quant_q,int chw_offset,int hw_offset){
	int cnt=0;
	if(quant_q.compare("DynamicFixed")==0){

		for(int i=0;i<in_hight;i++){
			int i_base=i*in_width;
			for(int j=0;j<in_width;j++){

				int tmp_ch_act=ch_act;

				for(int mc=0;mc<min_ch;mc++,tmp_ch_act++){

					T val0 = conv_out[cnt++];
					T val1 = conv_out[cnt++];
					float temp_val= ((float)val0)*sft_fbits;///(1 << fbits);
					//					int off = 0*ch_total*in_hight*in_width+tmp_ch_act*in_hight*in_width+i*in_width+j;
					int off = tmp_ch_act*hw_offset+i_base+j;
					caffe_buff[off] = temp_val;

					temp_val= ((float)val1)*sft_fbits;///(1 << fbits);
					//					off = 1*ch_total*in_hight*in_width+tmp_ch_act*in_hight*in_width+i*in_width+j;
					off = chw_offset+tmp_ch_act*hw_offset+i_base+j;

					caffe_buff[off] = temp_val;
				}
			}
		}
	}else{
		if(quant_q.compare("Xilinx")==0){

			for(int i=0;i<in_hight;i++){
				int i_base=i*in_width;
				for(int j=0;j<in_width;j++){
					int tmp_ch_act=ch_act;
					for(int mc=0;mc<min_ch;mc++,tmp_ch_act++){

						T val0 = conv_out[cnt++];
						T val1 = conv_out[cnt++];

						float temp_val= (float)val0* sf;
						//						int off = 0*ch_total*in_hight*in_width+tmp_ch_act*in_hight*in_width+i*in_width+j;
						int off = tmp_ch_act*hw_offset+i_base+j;
						caffe_buff[off] = temp_val;

						temp_val= (float)val1* sf;
						//						off=1*ch_total*in_hight*in_width+tmp_ch_act*in_hight*in_width+i*in_width+j;
						off=chw_offset+tmp_ch_act*hw_offset+i_base+j;

						caffe_buff[off] = temp_val;
					}
				}
			}
		}

	}
}

template<class T>
void pack_processed_ch(float *caffe_buff,T *conv_out,int &ch_act,int min_ch,int in_hight,int in_width,int ch_total,int fbits,float sf,float th,string quant_q,int chw_offset,int hw_offset){
	int cnt=0;
	int tmp_ch_act=ch_act;
	if(quant_q.compare("DynamicFixed")==0){

		for(int i=0;i<in_hight;i++){
			int i_base = i*in_width;
			for(int j=0;j<in_width;j++){
				tmp_ch_act=ch_act;
				for(int mc=0;mc<min_ch;mc++,tmp_ch_act++){

					//					int off=0*ch_total*in_hight*in_width+tmp_ch_act*in_hight*in_width+i*in_width+j;
					int off=tmp_ch_act*hw_offset+i_base+j;

					float f_val =caffe_buff[off];

					T ival = (T)( f_val);
					T fxval = ConvertToFP(f_val, ival, fbits);
					conv_out[cnt++] = fxval;

					//					off=1*ch_total*in_hight*in_width+tmp_ch_act*in_hight*in_width+i*in_width+j;
					off=chw_offset+tmp_ch_act*hw_offset+i_base+j;

					f_val =caffe_buff[off];
					ival = (T)( f_val);
					fxval = ConvertToFP(f_val, ival, fbits);
					conv_out[cnt++] = fxval;
				}
				for(int mc=min_ch;mc<PACK_ELEMS;mc++){
					conv_out[cnt++] = 0;
					conv_out[cnt++] = 0;
				}
			}
		}
	}else{
		if(quant_q.compare("Xilinx")==0){
			tmp_ch_act=ch_act;
			for(int i=0;i<in_hight;i++){
				int i_base = i*in_width;
				for(int j=0;j<in_width;j++){

					for(int mc=0;mc<min_ch;mc++,tmp_ch_act++){

						//						int off=0*ch_total*in_hight*in_width+tmp_ch_act*in_hight*in_width+i*in_width+j;
						int off=tmp_ch_act*hw_offset+i_base+j;

						float f_val = caffe_buff[off];

						T fxval;

						float max_data = th;
						float min_data = -max_data;
						f_val = std::max(std::min(f_val, max_data), min_data);
						f_val = f_val*sf;
						fxval = (T)floorf(f_val + 0.5f);

						conv_out[cnt++] = fxval;
						//						off=1*ch_total*in_hight*in_width+tmp_ch_act*in_hight*in_width+i*in_width+j;
						off=chw_offset+tmp_ch_act*hw_offset+i_base+j;

						f_val = caffe_buff[off];


						max_data = th;
						min_data = -max_data;
						f_val = std::max(std::min(f_val, max_data), min_data);
						f_val = f_val*sf;
						fxval = (T)floorf(f_val + 0.5f);

						conv_out[cnt++] = fxval;
					}
					for(int mc=min_ch;mc<PACK_ELEMS;mc++){
						conv_out[cnt++] = 0;
						conv_out[cnt++] = 0;
					}
				}
			}
		}

	}
}
template<class T>
void HardwarePack(float *software_output,int height,int width,int indepth,int fbits,float &sf,float &th,string &quant,T* conv_out, T *conv_out1,int split,int single_buf){



	int ch_act=0;
	int ch_total=indepth;
	int ch_actual=indepth;
	T *tmp_conv,*tmp_conv1;
	tmp_conv=conv_out;
	tmp_conv1 = conv_out1;
	int total_size=0;
	int depth=AlignSize(indepth, PACK_ELEMS);
	int ch_to_proc=depth;
	int buffer_offset = height*width*PACK_ELEMS*XBATCH_SIZE;
	int hw_offset=height*width;
	int chw_offset=ch_total*height*width;
	//#if PERFORMANCE_CHECK2
	//		clock_start_norm = sds_clock_counter();
	//#endif
	float inv_sf = 1.0f/sf;
	while(ch_to_proc>0){

		int min_ch=std::min(PACK_ELEMS,ch_total);

		pack_processed_ch(software_output,tmp_conv,ch_act,min_ch,height,width,indepth,fbits,inv_sf,th,quant,chw_offset,hw_offset);
		ch_act=ch_act+min_ch;
		ch_total=ch_total-min_ch;
		ch_to_proc=ch_to_proc-PACK_ELEMS;
		tmp_conv=tmp_conv+buffer_offset;
		if(single_buf==0){
			pack_processed_ch(software_output,tmp_conv1,ch_act,min_ch,height,width,indepth,fbits,inv_sf,th,quant,chw_offset,hw_offset);
			tmp_conv1=tmp_conv1+buffer_offset;
			ch_act=ch_act+min_ch;
			ch_total=ch_total-min_ch;
			ch_to_proc=ch_to_proc-PACK_ELEMS;
		}
	}
	//#if PERFORMANCE_CHECK2
	//			clock_end_norm_inter = sds_clock_counter();
	//			Kerneltime1 = ((double)(clock_end_norm_inter-clock_start_norm_inter))/arm_clock_norm;
	//			fprintf(stderr,"\nAverage time for Norm function 1 in ms: %f ms\n", Kerneltime1);
	//#endif
}
template<class T>
void SoftwareUnpack(T* conv_out, T *conv_out1, int height,int width,int indepth,int fbits,float &sf,float &th,string &quant,float *kernel_output,int single_buf){


	T *tmp_conv=conv_out;
	T *tmp_conv1 =conv_out1;
	int depth=AlignSize(indepth, PACK_ELEMS);
	int ch_to_proc=depth;
	int ch_act=0;
	int ch_actual=indepth;
	int ch_total=indepth;
	int hw_offset=height*width;
	int chw_offset=ch_total*height*width;
	int buffer_offset = height*width*PACK_ELEMS*XBATCH_SIZE;
	float sft_fbits = 1.0f/(1<<fbits);

	while(ch_to_proc>0){

		int min_ch = std::min(PACK_ELEMS,ch_total);

		unpack_processed_ch(kernel_output,tmp_conv,ch_act,min_ch,height,width,ch_actual,sft_fbits,sf,th,quant,chw_offset,hw_offset);

		ch_act=ch_act+min_ch;
		ch_total=ch_total-min_ch;
		ch_to_proc=ch_to_proc-PACK_ELEMS;
		tmp_conv=tmp_conv+buffer_offset;
		if(single_buf==0){
			unpack_processed_ch(kernel_output,tmp_conv1,ch_act,min_ch,height,width,ch_actual,sft_fbits,sf,th,quant,chw_offset,hw_offset);
			tmp_conv1=tmp_conv1+buffer_offset;
			ch_act=ch_act+min_ch;
			ch_total=ch_total-min_ch;
			ch_to_proc=ch_to_proc-PACK_ELEMS;
		}
	}
}
void XpackWrapper(void *input1, void *input2, void *output1, void *output2, string quant, int *params, float *float_params)
{

	//int pack_flag = (int)xpackArgs->Layer->pack;
	void *input, *output;
	float *software_output;
	IO_DATA_TYPE * conv_out;
	IO_DATA_TYPE * conv_out1;
	float *kernel_output;

	float th_layer_in = float_params[0];
	float sf_in  = float_params[1];

	int pack_flag = params[8];
	int fbits = params[9];

	//string quant = xpackArgs->Layer->string_args[0];
	int single_buf=0;
#if SINGLE_IO_PORT==0
	single_buf=0;
#else
	single_buf=1;
#endif
	if(pack_flag==1){
		software_output = (float*)input1;//xpackArgs->Layer->in_ptrs[0];
		conv_out  		= (IO_DATA_TYPE*)output1;//xpackArgs->Layer->out_ptrs[0];
		conv_out1 		= (IO_DATA_TYPE*)output2;//xpackArgs->Layer->out_ptrs[1];
	}else if (pack_flag==0){

		conv_out  		= (IO_DATA_TYPE*)input1;//xpackArgs->Layer->in_ptrs[0];
		conv_out1 		= (IO_DATA_TYPE*)input2;//xpackArgs->Layer->in_ptrs[1];
		software_output 	= (float*)output1;//xpackArgs->Layer->out_ptrs[0];

	}else if (pack_flag==2){
		software_output = (float*)input1;//xpackArgs->Layer->in_ptrs[0];
		conv_out  		= (IO_DATA_TYPE*)output1;//xpackArgs->Layer->out_ptrs[0];
	}
	else if(pack_flag==3){
		conv_out  		= (IO_DATA_TYPE*)input1;//xpackArgs->Layer->in_ptrs[0];
		software_output = (float*)output1;//xpackArgs->Layer->out_ptrs[0];

	}else if(pack_flag==5){
		conv_out= (IO_DATA_TYPE*)input1;//xpackArgs->Layer->in_ptrs[0];
		software_output	= (float*)output1;//xpackArgs->Layer->out_ptrs[0];
	}else{
		if(pack_flag==4){
			software_output	= (float*)input1;//xpackArgs->Layer->in_ptrs[1];
			conv_out	= (IO_DATA_TYPE*)output1;//xpackArgs->Layer->out_ptrs[0];
		}
	}
	//int fbits = (int)xpackArgs->Layer->fbits;//qf_format.op_fbits;//fbits;


	int in_height;
	int in_width;
	int in_indepth;
	int out_height;
	int out_width;
	int out_indepth;
	if(pack_flag!=2){
		out_indepth= params[3];
		out_width= params[4];
		out_height=params[5];
		in_height=params[2];
		in_width= params[1];
		in_indepth= params[0];
	}else{
		out_indepth= params[1];
		in_indepth= params[0];
		in_width=params[2];
		out_width= params[3];
	}

	int split =1;
	//# Crop sw-kernel
	if(pack_flag==1){

		//PackDataToHardware<IO_DATA_TYPE>(software_output,out_height, out_width, out_indepth,fbits,conv_out, conv_out1,split);
		//		PackDataToHardware<IO_DATA_TYPE>(software_output,out_height,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,conv_out,conv_out1,split);
		HardwarePack<IO_DATA_TYPE>(software_output,out_height,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,conv_out,conv_out1,split,single_buf);


	}else if(pack_flag==0){
		//UnpackDataToSoftware<IO_DATA_TYPE>(conv_out,conv_out1,out_height, out_width, out_indepth,fbits,kernel_output);
		//		UnpackDataToSoftware<IO_DATA_TYPE>(conv_out, conv_out1,  out_height,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,kernel_output);
		SoftwareUnpack<IO_DATA_TYPE>(conv_out,conv_out1,out_height,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,software_output,single_buf);

	}else if(pack_flag==2){
		packDataToSoftmax<IO_DATA_TYPE>(software_output,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,conv_out);
		//packDataToSoftmax<IO_DATA_TYPE>(software_output,out_width,out_indepth,fbits,conv_out);
	}
	else if(pack_flag==3){
		unPackPermutetocustom<IO_DATA_TYPE>(conv_out,out_height,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,software_output);
	}else if(pack_flag==4){
		single_buf=1;
		HardwarePack<IO_DATA_TYPE>(software_output,out_height,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,conv_out,conv_out1,split,single_buf);

		//		PackDataToHardware_one_input<IO_DATA_TYPE>(software_output,out_height,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,conv_out,split);

	}else{
		if(pack_flag==5){
			single_buf=1;
			SoftwareUnpack<IO_DATA_TYPE>(conv_out,conv_out1,out_height,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,software_output,single_buf);
			//			UnpackDataToSoftware_one_output<IO_DATA_TYPE>(conv_out, out_height,out_width,out_indepth,fbits,sf_in,th_layer_in,quant,kernel_output);

		}
	}

}
