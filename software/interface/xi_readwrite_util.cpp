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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if 1//def __SDSCC__
#undef __ARM_NEON__
#undef __ARM_NEON
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#define __ARM_NEON__
#define __ARM_NEON
#else
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#endif

using namespace std;
using namespace cv;

#include <iostream>

#include "../interface/xi_interface.hpp"
#include "xi_readwrite_util.hpp"


#define ConvertToFP(fVal, iPart, fbits)	((int)((iPart<<fbits) + ((fVal-(float)iPart))*(1<<fbits)))

template<typename T>
class descendingCmp
{
public:
	const T* x;
	int offset;
	descendingCmp(const T* d, int off): x(d), offset(off) {}
	bool operator() (int a, int b) { return x[a+offset] > x[b+offset]; }
};

void Trim2FP_Offline(float *data, const int bw, float threshold)
{
	//# Compute scale factor
	float scaling_factor = threshold / (pow(2, bw-1) - 1);

	//# Clip the data b/w (-threshold, threshold)
	data[0] = std::max(std::min(data[0], threshold), -threshold);

	if(threshold != 0)
	{
		data[0] /= scaling_factor;
		data[0] = round(data[0]);
	}
}

void Trim2FixedPoint(float &data, const int bw, const int fl, int round_floor_flag)
{
	float pow_bw_minus_1 = pow(2, bw-1);
	float pow_minus_fl = pow(2, -fl);
	float pow_plus_fl = pow(2, fl);
	float max_data = (pow_bw_minus_1 - 1) * pow_minus_fl;
	float min_data = -pow_bw_minus_1 * pow_minus_fl;

	// Saturate data
	data = std::max(std::min(data, max_data), min_data);
	if(round_floor_flag == 1)    //if floor
	{
		//Floor data
		data = (floor(data*pow_plus_fl))/pow_plus_fl;
	}
	else
	{
		// Round data
		data /= pow_minus_fl;
		data = round(data);

		data *= pow_minus_fl;

	}

}

int loadPixelMeanSubtractedDatatoBuffptr(const char *img_path, IO_DATA_TYPE *buff_ptr, int height, int width, int img_channel, int resize_h, int resize_w, int fbits_input, char *mean_path)
{

	cv::Mat frame;

	frame = cv::imread(img_path, 1);
	if(!frame.data)
	{
		std :: cout << "[ERROR] Image read failed - " << img_path << std :: endl;
		return -1;
	}
	else
	{
		std :: cout << "[IMRDx] Image read : " << img_path << std :: endl;
	}

	int h_off = 0;
	int w_off = 0;

	cv::Mat cv_img[1], cv_cropped_img[1];

	if((resize_h != frame.rows) || (resize_w != frame.cols))
	{
		cv::resize(frame, cv_img[0], cv::Size(resize_w, resize_h));

		if(resize_h > height)
		{
			h_off = (resize_h - height) / 2;
			w_off = (resize_w - width) / 2;
			cv::Rect roi(w_off, h_off, width, height);
			cv_cropped_img[0] = cv_img[0](roi);
		}
		else
			cv_cropped_img[0] = cv_img[0];
	}
	else
	{
		frame.copyTo(cv_cropped_img[0]);
	}

	float *mean_ptr = (float *)malloc(256*256*3*sizeof(float));
	FILE *fmean=fopen(mean_path, "r");
	if(fmean == NULL)
	{
		std :: cout << "File not found - " << mean_path << std :: endl;
		return -1;
	}

	float f_val;
	for(int i=0; i< 256*256*3;i++)
	{
		fscanf(fmean, "%f ", &f_val);
		mean_ptr[i] = f_val;
	}
	fclose(fmean);

	//float mean_ptr[3] = {104.0, 117.0, 123.0};

	//float pixel;
	uchar pixel;
	int input_index=0;
	float float_val;
	short fxval;
	int mean_idx = 0;

	for (int h = 0; h < height; ++h)
	{
		const uchar* ptr = cv_cropped_img[0].ptr<uchar>(h);//data;
		int img_index = 0;
		for (int w = 0; w < width; ++w)
		{
			for (int ch = 0; ch < img_channel; ++ch)
			{
				//pixel = static_cast<float>(ptr[img_index++]);
				pixel = ptr[img_index++];

				short pixel1 = (short)pixel << fbits_input;

				mean_idx = (ch * resize_h + h + h_off) * resize_w + w + w_off;
				float float_mean = mean_ptr[mean_idx];
				//float float_mean = mean_ptr[ch];

				IO_DATA_TYPE ival = (IO_DATA_TYPE)float_mean;
				IO_DATA_TYPE mean_fxval = ConvertToFP(float_mean, ival, fbits_input);
				fxval = pixel1 - mean_fxval;

				buff_ptr[input_index] = fxval;

				input_index++;
			}// Channels
		}// Image width

	}// Image Height

	free(mean_ptr);
	return 0;
}


int loadNormalizedInputDatatoBuffptr(const char *img_path, IO_DATA_TYPE *buff_ptr, float* mean_ptr, float* var_ptr, int height, int width, int img_channel,
		int resize_h, int resize_w, int inp_bw, int fbits_input)//, int normalize_enable)
{

	cv::Mat frame;

	frame = imread(img_path, 1);
	//frame = imread("/proj/sdxapps/refdes/ML_SoftwareTeam/Ristretto/ResNet-50-Tf_Q_5_11_test/input/camel.jpg", 1);
	if(!frame.data)
	{
		std :: cout << "[ERROR] Image read failed - " << img_path << std :: endl;
		return -1;
	}
	else
	{
		std :: cout << "[IMRDx] Image read : " << img_path << std :: endl;
	}

	int h_off = 0;
	int w_off = 0;

	cv::Mat cv_img[1], cv_cropped_img[1];

	if((resize_h != frame.rows) || (resize_w != frame.cols))
	{
		cv::resize(frame, cv_img[0], cv::Size(resize_w, resize_h));

		if(resize_h > height)
		{
			h_off = (resize_h - height) / 2;
			w_off = (resize_w - width) / 2;
			cv::Rect roi(w_off, h_off, width, height);
			cv_cropped_img[0] = cv_img[0](roi);
		}
		else
			cv_cropped_img[0] = cv_img[0];
	}
	else
	{
		frame.copyTo(cv_cropped_img[0]);
	}
	
#if DEBUG
	char path1[100];
	sprintf(path1, "data_out.txt");

	FILE* fp_img = fopen(path1, "r");

	if(fp_img == NULL)
	{
		fprintf(stderr, "\n** Cannot open mean file (or) No mean file : %s **\n", path1);
		//return NULL;
	}

	int size_of_input = height*width*img_channel;
	float *in_buf = (float *)malloc(size_of_input*sizeof(float));

	float float_val1;
	short fxval1;
	for(int i = 0; i < height*width*img_channel; i++)
	{
		fscanf(fp_img, "%f ", &float_val1);
		in_buf[i] = float_val1;
	}
	fclose(fp_img);
#endif

	uchar pixel;
	int input_index=0;
	float float_val;
	IO_DATA_TYPE fxval;
	int mean_idx = 0;

#if FILE_WRITE
	FILE* fp_inp = fopen("norminpdata.txt", "w");
#endif
	//if(normalize_enable)
	{
		for (int h = 0; h < height; ++h)
		{
			const uchar* ptr = cv_cropped_img[0].ptr<uchar>(h);//data;
			int img_index = 0;
			for (int w = 0; w < width; ++w)
			{
				for (int ch = 0; ch < img_channel; ++ch)
				{
#if DEBUG
					int img_index1 = h*width + w;
					int img_index2 = img_index1 + ch*(height*width);
					float pixel2 = in_buf[img_index2];
#endif
					//pixel = static_cast<float>(ptr[img_index++]);
					pixel = ptr[img_index++];
					float float_mean = mean_ptr[ch];

					float float_var  = var_ptr[ch];

					float_val = ( ((float)pixel/255.0) - float_mean)/float_var;

#if (IO_TYPE==8)
					Trim2FixedPoint(float_val, inp_bw, fbits_input, 0);    //Floor=1, Round=0 option //TODO:Resnet-50Adv rounding verification
#endif
#if DEBUG
					if(pixel2 != float_val)
						fprintf(stderr, "pixel2= %f  float_val = %f\n", pixel2, float_val);
#endif

#if FILE_WRITE
					fprintf(fp_inp,"%f ", float_val);
#endif

					IO_DATA_TYPE ival = (IO_DATA_TYPE)float_val;
					fxval = ConvertToFP(float_val, ival, fbits_input);

					buff_ptr[input_index] = fxval;

					//fprintf(fp_inp,"%d ", fxval);
					input_index++;
				}// Channels

			}// Image width
			//fprintf(fp_inp,"\n");
		}// Image Height
	}

#if DEBUG
	free(in_buf);
#endif
#if FILE_WRITE
	fclose(fp_inp);
#endif
}

int loadPlaneMeanSubtractedDatatoBuffptr(const char *img_path, IO_DATA_TYPE *buff_ptr, float *mean_ptr, int height, int width, int img_channel,
		int resize_h, int resize_w, int fbits_input, int ip_bw, float th_in, int quant_scheme_flag)
{

	cv::Mat frame;

	frame = imread(img_path, 1);
	if(!frame.data)
	{
		std :: cout << "[ERROR] Image read failed - " << img_path << std :: endl;
		return -1;
	}
	else
	{
		//std :: cout << "[IMRDx] Image read : " << img_path << std :: endl;
	}

	int h_off = 0;
	int w_off = 0;

	cv::Mat cv_img[1], cv_cropped_img[1];

	if((resize_h != frame.rows) || (resize_w != frame.cols))
	{
		cv::resize(frame, cv_img[0], cv::Size(resize_w, resize_h));

		if(resize_h > height)
		{
			h_off = (resize_h - height) / 2;
			w_off = (resize_w - width) / 2;
			cv::Rect roi(w_off, h_off, width, height);
			cv_cropped_img[0] = cv_img[0](roi);
		}
		else
			cv_cropped_img[0] = cv_img[0];
	}
	else
	{
		frame.copyTo(cv_cropped_img[0]);
	}

	uchar pixel;
	int input_index=0;
	float float_val;
	IO_DATA_TYPE fxval;
	int mean_idx = 0;
	int input_mean_sub_val=0;

#if FILE_WRITE
	FILE* fp_inp = fopen("norminpdata.txt", "w");
#endif

	if(quant_scheme_flag == 1)
	{
		for (int h = 0; h < height; ++h)
		{
			const uchar* ptr = cv_cropped_img[0].ptr<uchar>(h);//data;
			int img_index = 0;
			for (int w = 0; w < width; ++w)
			{
				for (int ch = 0; ch < img_channel; ++ch)
				{
#if DEBUG
					int img_index1 = h*width + w;
					int img_index2 = img_index1 + ch*(height*width);
					float pixel2 = in_buf[img_index2];
#endif

					pixel = ptr[img_index++];
					float float_mean = mean_ptr[ch];

					float pixel1 = (float)pixel - float_mean;
					Trim2FP_Offline(&pixel1, ip_bw, th_in);

					fxval = (IO_DATA_TYPE)pixel1;
					buff_ptr[input_index] = fxval;

					//fprintf(fp_inp,"%d ", fxval);
					input_index++;
				}// Channels
			}// Image width
			//fprintf(fp_inp,"\n");
		}// Image Height
	}
	else  //dynamic fixed
	{

		int max_positive = ((1<<(ip_bw-1))-1);
		int max_negative = -(1<<(ip_bw-1));


		//int cnt1 = 0;
		for (int h = 0; h < height; ++h)
		{
			const uchar* ptr = cv_cropped_img[0].ptr<uchar>(h);//data;
			int img_index = 0;

			for (int w = 0; w < width; ++w)
			{

				for (int ch = 0; ch < img_channel; ++ch)
				{
					//pixel = static_cast<float>(ptr[img_index++]);
					pixel = ptr[img_index++];

					float float_mean = mean_ptr[ch];

					short pixel1 = (short)pixel << fbits_input;

					IO_DATA_TYPE ival = (IO_DATA_TYPE)float_mean;
					IO_DATA_TYPE mean_fxval = ConvertToFP(float_mean, ival, fbits_input);
					input_mean_sub_val = pixel1 - mean_fxval;

					if(input_mean_sub_val > max_positive)
					{
						fxval = max_positive;
					}
					else if(input_mean_sub_val < max_negative)
					{
						fxval = max_negative;
					}
					else
					{
						fxval = input_mean_sub_val;
					}


					/*
				if(cnt1 > 865)
				{
					fprintf(stderr, "cnt1 = %d, pixel1 = %d, mean = %d, fxval = %d\n", cnt1, pixel1, mean_fxval, fxval);
				}
					 */

					buff_ptr[input_index] = fxval;
#if FILE_WRITE
					fprintf(fp_inp,"%d ", fxval);
#endif
					input_index++;
				}// Channels
				//cnt1++;
			}// Image width
			//fprintf(fp_inp,"\n");
		}// Image Height
	}  //dynamic fixed

#if FILE_WRITE
	fclose(fp_inp);
#endif

	return 0;
}

int loadMeanSubtractedDatafromBuffptr(std::vector<void *> normalizeInput, IO_DATA_TYPE *in_ptr, int height, int width, int img_channel, int en_batch_size_one)
{

#if FILE_WRITE
	FILE *fp = fopen("input.txt", "w");
#endif

#if 0
	FILE *finput=fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_models/SSD_8Bit_dummy/input/data_out.txt", "r");
	if(finput == NULL)
	{
		std :: cout << "File not found - " << finput << std :: endl;
		return -1;
	}

	float f_val;
	int fbits_input = 4;
	IO_DATA_TYPE *buff_ptr = (IO_DATA_TYPE *)normalizeInput[0];
	for(int i=0; i< 300*300*3;i++)
	{
		fscanf(finput, "%f ", &f_val);
		IO_DATA_TYPE ival = (IO_DATA_TYPE)f_val;
		IO_DATA_TYPE fxval = ConvertToFP(f_val, ival, fbits_input);
		ival = buff_ptr[i];
		buff_ptr[i] = fxval;
		if(ival!=fxval)
			fprintf(stderr, "i:%d   diff:%d   ival:%d   fxval:%d\n", i, abs(ival-fxval), ival, fxval);
	}
	fclose(finput);
#endif

	int batch_cnt1;
	int batch_cnt2;

	if(XBATCH_SIZE < BATCH_PACK_SIZE)
	{
		batch_cnt2 = 1;
		batch_cnt1 = XBATCH_SIZE;
	}
	else
	{
		batch_cnt2 = XBATCH_SIZE/BATCH_PACK_SIZE;
		if((batch_cnt2 * BATCH_PACK_SIZE) < XBATCH_SIZE)
			batch_cnt2 += 1;
		batch_cnt1 = BATCH_PACK_SIZE;
	}

	int batch_loop_cnt;
	if(en_batch_size_one)
		batch_loop_cnt = 1;
	else
		batch_loop_cnt = XBATCH_SIZE;

	int align_width;

	if(XBATCH_SIZE == 1)
	{
		align_width = (width%2 == 0) ? width : ((width/2 + 1)*2);
	}
	else
	{
		align_width = width;
	}

	IO_DATA_TYPE pixel;
	int input_index=0;

	int img_index = 0;
	for(int batch_id2 = 0; batch_id2 < batch_cnt2; batch_id2++)
	{
		for (int h = 0; h < height; ++h)
		{
			//for (int w = 0; w < width; ++w)
			for (int w = 0; w < align_width; ++w)
			{
				//TODO : Generic for 4
				for (int c = 0; c < 4; ++c)
				{

					for(int batch_id1 = 0; batch_id1 < batch_cnt1; batch_id1++)
					{
						int batch_id = (batch_id2 * batch_cnt1) +  batch_id1;
						IO_DATA_TYPE *buff_ptr = (IO_DATA_TYPE *)normalizeInput[batch_id];
						if( (c < img_channel) && (batch_id < batch_loop_cnt) && (w < width) )
						{
							pixel = buff_ptr[img_index];
						}
						else
						{
							pixel = 0;
						}
						in_ptr[input_index] =  pixel;
						input_index++;
#if FILE_WRITE
						fprintf(fp,"%d ", pixel);
#endif
					} // for(int batch_id1 = 0; batch_id1 < batch_cnt1; batch_id1++)

					if(c < img_channel)
						img_index++;

				}// Channels
			}// Image width
#if FILE_WRITE
			fprintf(fp,"\n");
#endif
		}// Image Height
	} //for(int batch_id2 = 0; batch_id2 < batch_cnt1; batch_id2++)
#if FILE_WRITE
	fclose(fp);
#endif

	return 0;
}
//loadDatafromBuffptr

//# Input Read
int inputNormalization(std::vector<void *>input, int resize_h, int resize_w, 
					char *img_path1, char *img_path2, bool inp_mode, 
					float *mean_ptr, float *var_ptr,
					int numImg_to_process, io_layer_info io_layer_info_ptr)
{

	//# use for Pixel wise mean subtraction
	//# Currently support is disabled
	char *mean_path;
	
	//# Load input layer params
	int inp_height  = io_layer_info_ptr.in_height;
	int inp_width   = io_layer_info_ptr.in_width;
	int inp_bw      = io_layer_info_ptr.in_bw;
	int inp_fbits   = io_layer_info_ptr.in_fbits;
	int inp_channel = io_layer_info_ptr.in_channel;
	int inp_depth   = io_layer_info_ptr.in_depth;
	float th_in		= io_layer_info_ptr.th_in;
	int quant_scheme_flag = io_layer_info_ptr.quant_scheme_flag;

	int en_batch_size_one;

	if(numImg_to_process == 2)
		en_batch_size_one = 0;
	else
		en_batch_size_one = 1;

	int normalize_enable=inp_mode;
	int status;

	if((resize_h < inp_height) || (resize_w < inp_width))
	{
		fprintf(stderr,"[ERROR] invalid resize params\n");
		return -1;
	}

	int batch_loop_cnt;
	if(en_batch_size_one)
		batch_loop_cnt = 1;
	else
		batch_loop_cnt = XBATCH_SIZE;
	
		if(inp_mode==0)
		{
			for(int batch_id = 0; batch_id < batch_loop_cnt; batch_id++)
			{
				if (batch_id == 0)
				{
					status = loadPlaneMeanSubtractedDatatoBuffptr(img_path1, (IO_DATA_TYPE *)input[batch_id], 
					mean_ptr, inp_height, inp_width, inp_channel, resize_h, resize_w, inp_fbits, inp_bw, th_in, quant_scheme_flag);
				}
				else
				{
					status = loadPlaneMeanSubtractedDatatoBuffptr(img_path2, (IO_DATA_TYPE *)input[batch_id], mean_ptr, inp_height, inp_width, inp_channel, resize_h, resize_w, inp_fbits, inp_bw, th_in, quant_scheme_flag);
				}
			}
		}

		if(inp_mode==1)
		{
			//normalize_enable = 1;
			for(int batch_id = 0; batch_id < batch_loop_cnt; batch_id++)
			{
				if (batch_id == 0)
				{
					status = loadNormalizedInputDatatoBuffptr(img_path1, (IO_DATA_TYPE *)input[batch_id], mean_ptr, var_ptr, inp_height, inp_width, inp_channel, resize_h, resize_w, inp_bw, inp_fbits);//, normalize_enable);
				}
				else
				{
					status = loadNormalizedInputDatatoBuffptr(img_path2, (IO_DATA_TYPE *)input[batch_id], mean_ptr, var_ptr,
					inp_height, inp_width, inp_channel, resize_h, resize_w, inp_bw, inp_fbits);//, normalize_enable);
				}
			}
		}

		if(inp_mode==2)
		{
			for(int batch_id = 0; batch_id < batch_loop_cnt; batch_id++)
			{
				if (batch_id == 0)
				{
					status = loadPixelMeanSubtractedDatatoBuffptr(img_path1, (IO_DATA_TYPE *)input[batch_id], inp_height, inp_width, inp_channel, resize_h, resize_w, inp_fbits, mean_path);
				}
				else
				{
					status = loadPixelMeanSubtractedDatatoBuffptr(img_path2, (IO_DATA_TYPE *)input[batch_id], inp_height, inp_width, inp_channel, resize_h, resize_w, inp_fbits, mean_path);
				}
			}
		}
	

	return status;

}

int outputUnpack(void* output, std::vector<void *> output_unpack, kernel_type_e kernType, int outputSize, int en_batch_size_one)
{

	int batch_loop_cnt;
	if(en_batch_size_one)
		batch_loop_cnt = 1;
	else
		batch_loop_cnt = XBATCH_SIZE;

	//# For Classification Networks
	if(kernType == SOFTMAX)
	{

		for(int batch_id = 0; batch_id < batch_loop_cnt; batch_id++)
		{
			float *output_unpack_ptr = (float*)output_unpack[batch_id];
			float *output_ptr	= (float*)output;

			output_ptr += batch_id;
			for(int i=0;i<outputSize;i++)
			{
				output_unpack_ptr[i] = output_ptr[i*XBATCH_SIZE];
			}
		}

	}

	//# For Detection Networks
	else if (kernType == NMS)
	{
		float *output_ptr	= (float*)output;
		for(int batch_id = 0; batch_id < batch_loop_cnt; batch_id++)
		{
			float *output_unpack_ptr = (float*)output_unpack[batch_id];
			int BoxCount = output_ptr[0];
			//memcpy((int*)output_unpack_ptr, &BoxCount, sizeof(int));
			memcpy((float*)(output_unpack_ptr), (float*)(output_ptr), (BoxCount*7+1)*sizeof(float));
			output_ptr += BoxCount*7+1;
		}
	}

	//# For Segmentation Networks
	else if (kernType == CROP)
	{
		int *output_ptr	= (int*)output;
		for(int batch_id = 0; batch_id < batch_loop_cnt; batch_id++)
		{
			int *output_unpack_ptr = (int*)output_unpack[batch_id];
			memcpy((int*)output_unpack_ptr, output_ptr, outputSize*sizeof(int));
			output_ptr += (outputSize);
		}
	}
}


//# Input Read
void xiInputRead(std::vector<void *> normalizeInput, vector<void *> input, int numImg_to_process, io_layer_info io_layer_info_ptr)
{

	int en_batch_size_one;

	if(numImg_to_process == 2)
		en_batch_size_one = 0;
	else
		en_batch_size_one = 1;

	//# Load input layer params
	int inp_height  = io_layer_info_ptr.in_height;
	int inp_width   = io_layer_info_ptr.in_width;
	int inp_channel = io_layer_info_ptr.in_channel;

	loadMeanSubtractedDatafromBuffptr(normalizeInput, (IO_DATA_TYPE *)input[0], inp_height, inp_width, inp_channel, en_batch_size_one);
}

//# Output Write
void xiUnpackOutput(std::vector<void *> output, std::vector<void *> output_unpack, kernel_type_e kernelType, int outputSize, int numImg_to_process)
{
	int en_batch_size_one;

	if(numImg_to_process == 2)
		en_batch_size_one = 0;
	else
		en_batch_size_one = 1;

	outputUnpack(output[0], output_unpack, kernelType, outputSize, en_batch_size_one);
}


// inarray - Input array (shape : nboxes x nclasses)
// outarray - output array. These are indices of the sorted array (shape : nboxes x top_k)
// nobjs- number of objects (For classification networks, it is mostly 1 because only one image. For FCN, it can be HxW)
// nclasses - number of classes. (For ImageNet, it is 1000. For PASCAL-VOC, it is 21)
// top_k    - Return top-k IDs. (User must take care of output array dimension)
template<typename T>
void argmaxLayer(T* inarray, int* outarray, int nobjs, int nclasses, int top_k)
{
	int nelems = nclasses * nobjs;
	switch(top_k)
	{
	case 0:             // Unsupported
		cerr << "top_k = 0 for argmax layer is not supported" << endl;
		exit(-1);

	case 1:             // if top_k = 1, all ID creation, copy, sort etc is overkill. Simply find the location of max element
		// // STL implementation. (This looks "slightly" slower than the vanilla implementation, need to verify on ARM for exact picture)
		// for(int i=0, j=0; i<nobjs; i++, j+=nclasses)
		// {
		//     int maxLoc = std::distance(inarray+j, max_element(inarray+j, inarray+j+nclasses));
		//     outarray[i] = maxLoc;
		// }

		// Vanilla Implementation
		for(int i=0; i<nobjs; i++)
		{
			int startoffset = i*nclasses;
			T maxVal = inarray[startoffset];
			int maxLoc = 0;
			for(int j=1; j<nclasses; j++)
			{
				T val = inarray[startoffset + j];
				if(val > maxVal)
				{
					maxVal = val;
					maxLoc = j;
				}
			}
			outarray[i] = maxLoc;
		}
		break;

	default:            // if top_k > 1, use partial_sort
		int* ID = new int [nelems];
		// Create permanent IDs
		for(int i=0; i<nelems; i++)
			ID[i] = i;

		// setup temporary ids.
		int* tmpID = new int [nelems];

		// Take each set of values and do argsort
		for(int i=0, j=0, k=0; i<nobjs; i++, j+=nclasses, k+=top_k)
		{
			// Reset the tmpID
			copy(ID, ID+nclasses, tmpID);
			// Do the partial sort
			partial_sort(tmpID, tmpID+top_k, tmpID+nclasses, descendingCmp<T>(inarray, j));
			// Copy the result to output array
			copy(tmpID, tmpID+top_k, outarray+k);
		}
		break;
	}
}




int outputWrite(char *dir_path, char *inp_file_name, std::vector<void *> unpack_output, int numImg_to_process, io_layer_info io_layer_info_ptr, int ping_pong)
{
	int en_batch_size_one;

	if(numImg_to_process == 2)
		en_batch_size_one = 0;
	else
		en_batch_size_one = 1;

#if HW_DBG
	//////////// Network name fetch
	char network_name[500];
	for(int i=0;i<100;i++)
	{
		network_name[i]=0;
	}
	{
		char *path = dir_path;
		char *ssc;
		ssc = strstr(path, "models");
		int l = strlen(ssc) + 1;
		path = &path[strlen(path)-l+8];
		int l1 = strlen(path)+1;
		for(int i=0;i<(l1);i++)
		{
			network_name[i]=path[i];
		}
		//fprintf(stderr,"%s\n",network_name);
	}


	char file_name2[500];
	sprintf(file_name2, "%s", inp_file_name);
	char *file_name = (char *)file_name2;
	char *ssc;
	int l = 0;
	ssc = strstr(file_name, "/");
	do{
		l = strlen(ssc) + 1;
		file_name = &file_name[strlen(file_name)-l+2];
		ssc = strstr(file_name, "/");
	}while(ssc);
	fprintf(stderr, "%s\n", file_name);

	char *file_name1 = strstr(file_name, ".");
	l = strlen(file_name);
	int l1 = strlen(file_name1);
	file_name[l-l1] = '\0';
	fprintf(stderr,"output file name:%s\n",file_name);

#endif

	/* for layer name */
	int kernelType = io_layer_info_ptr.out_kerType;
	int outSize = io_layer_info_ptr.out_size;

	//# Write file for checking error
	char out_path[500];

	int batch_loop_cnt;
	if(en_batch_size_one)
		batch_loop_cnt = 1;
	else
		batch_loop_cnt = XBATCH_SIZE;

	fprintf(stderr, "\n\n");
	//# Write output to a file
	for(int batch_id = 0; batch_id < batch_loop_cnt; batch_id++)
	{

#if HW_DBG
		sprintf(out_path,"logs/%s_%s_out_%d_%d.txt", network_name, file_name, batch_id, ping_pong);
#else
		sprintf(out_path,"%s/out/out_%d_%d.txt", dir_path, batch_id, ping_pong);
#endif

		fprintf(stderr,"[OUTWR] Write Path : %s\n\n", out_path);
		FILE *fp_out = fopen(out_path, "w");
		if(fp_out == NULL)
		{
			std :: cout << "[ERROR] can't create file : " << out_path << std :: endl;
			return -1;
		}
		float *out_ptr = (float*)unpack_output[batch_id];

		if(kernelType==NMS)
		{
			int out_count = out_ptr[0]*7;
			for(int i = 0; i < (out_ptr[0]*7)+1; ++i)
			{
				fprintf(fp_out, "%f\n", out_ptr[i]);
			}
		}
		else if(kernelType==SOFTMAX)
		{
			for(int i = 0; i < outSize; ++i)
			{
				fprintf(fp_out, "%f\n", out_ptr[i]);
			}
		}
		else if(kernelType == CROP)
		{
			int *hls_out = (int*)out_ptr;
			for(int i = 0; i < outSize; ++i)
			{
				fprintf(fp_out, "%d\n", hls_out[i]);
			}
		}
		
		fclose(fp_out);

		if(kernelType == SOFTMAX)
		{
			float *out_ptr1 = (float*)unpack_output[batch_id];

			// buffer for final ID
			int HW = outSize;
			int* classID = (int*)malloc(HW * sizeof(int));

			argmaxLayer(&(out_ptr1[0]), classID, 1, outSize, 5);  //top 5ids

			fprintf(stderr, "[INFOx] Classification Output for batch : %d\n", batch_id);
			for(int i = 0; i < 5; ++i)
			{
				fprintf(stderr, "[INFOx] classId : %d, score : %f\n", classID[i], out_ptr1[classID[i]]);
			}
			fprintf(stderr, "\n");
		}
	}
}
