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

#include "xi_input_image.hpp"

 
void loadMeanTXT(float *ptrRetArray, const char* path, int nElems)
{
	FILE* fp = fopen(path, "r");

	if(fp == NULL)
	{
		printf("\n** Cannot open mean file (or) No mean file : %s **\n", path);
		//return NULL;
	}

	for(int i = 0; i < nElems; i++)
	{
		fscanf(fp, "%f ", ptrRetArray+i);
	}

	fclose(fp);

}


//int loadAndPrepareImage(cv::Mat &frame, short *inptr, float* mean_data, int img_w, int img_h, int img_channel, int resize_h, int resize_w, const char *mean_path, int mean_status)
int loadAndPrepareImage(cv::Mat &frame, xChangeLayer *xChangeHostInpLayer)
{

	short  *in_ptr  = (short *)xChangeHostInpLayer->in_ptrs[2];
	short *mean_ptr = (short *)xChangeHostInpLayer->mean;
	int *params_ptr = (int *)xChangeHostInpLayer->params;

	int height      = params_ptr[0];
	int width       = params_ptr[1];
	int img_channel = params_ptr[5];
	int resize_h    = xChangeHostInpLayer->resize_h;//params_ptr[0];    //??  how to handle
	int resize_w    = xChangeHostInpLayer->resize_w;//params_ptr[1];    //??

	int mean_sub_flag = params_ptr[26];

	int scale = 1;
	int h_off = 0;
	int w_off = 0;
	int mean_idx = 0;

	int fbits_input = xChangeHostInpLayer->qf_format.ip_fbits;//7;//convLayer_precision[0].in_fbits;

	cv::Mat cv_img[1], cv_cropped_img[1];

	if((resize_h != frame.rows) || (resize_w != frame.cols))
	{
		cv::resize(frame, cv_img[0], cv::Size(resize_h, resize_w));

		if(resize_h > height)
		{
			h_off = (resize_h - height) / 2;
			w_off = (resize_w - width) / 2;
			cv::Rect roi(w_off, h_off, height, width);
			cv_cropped_img[0] = cv_img[0](roi);
		}
		else
			cv_cropped_img[0] = cv_img[0];
	}
	else
	{
		frame.copyTo(cv_cropped_img[0]);
	}

#if FILE_WRITE
	FILE *fp = fopen("input.txt", "w");
#endif

	//float pixel;
	uchar pixel;
	int input_index=0;
	float float_val;
	short fxval;

	for (int h = 0; h < height; ++h)
	{
		const uchar* ptr = cv_cropped_img[0].ptr<uchar>(h);//data;
		int img_index = 0;
		for (int w = 0; w < width; ++w)
		{
			//TODO : Generic for 4
			for (int ch = 0; ch < 4; ++ch)
			{
				if(ch < img_channel)
				{
					//pixel = static_cast<float>(ptr[img_index++]);
					pixel = ptr[img_index++];

					short pixel1 = (short)pixel << fbits_input;

					if(mean_sub_flag == 1)
						mean_idx = ch;
					else
						mean_idx = (ch * resize_h + h + h_off) * resize_w + w + w_off;

					//float_val =  (pixel - mean_ptr[mean_idx]) * scale;

					//short ival = (short)float_val;
					//fxval = ConvertToFP(float_val, ival, fbits_input);
					fxval = pixel1 - mean_ptr[mean_idx];
				}
				else
				{
					float_val = 0;
					fxval = 0;
					in_ptr[input_index] = 0;
				}

				in_ptr[input_index] = fxval;

#if FILE_WRITE
				float_val = ((float)fxval)/(1 << fbits_input);
				fprintf(fp,"%f ", float_val);
#endif
				input_index++;
			}// Channels
		}// Image width
#if FILE_WRITE
		fprintf(fp,"\n");
#endif
	}// Image Height
#if FILE_WRITE
	fclose(fp);
#endif
}
// loadAndPrepareImage

#if 0
int loadMeanSubtractedImage(cv::Mat &frame, xChangeLayer *xChangeHostInpLayer)
{

	short  *in_ptr  = (short *)xChangeHostInpLayer->in_ptrs[2];
	float *mean_ptr = (float *)xChangeHostInpLayer->mean;
	float *variance_ptr = (float *)xChangeHostInpLayer->variance;
	int *params_ptr = (int *)xChangeHostInpLayer->params;

	int height      = params_ptr[0];
	int width       = params_ptr[1];
	int img_channel = params_ptr[5];
	int resize_h    = xChangeHostInpLayer->resize_h;//params_ptr[0];    //??  how to handle
	int resize_w    = xChangeHostInpLayer->resize_w;//params_ptr[1];    //??

	int mean_sub_flag = params_ptr[26];

	int scale = 1;
	int h_off = 0;
	int w_off = 0;
	int mean_idx = 0;

	int fbits_input = xChangeHostInpLayer->qf_format.ip_fbits;//7;//convLayer_precision[0].in_fbits;

	cv::Mat cv_img[1], cv_cropped_img[1];

	if((resize_h != frame.rows) || (resize_w != frame.cols))
	{
		cv::resize(frame, cv_img[0], cv::Size(resize_h, resize_w));

		if(resize_h > height)
		{
			h_off = (resize_h - height) / 2;
			w_off = (resize_w - width) / 2;
			cv::Rect roi(w_off, h_off, height, width);
			cv_cropped_img[0] = cv_img[0](roi);
		}
		else
			cv_cropped_img[0] = cv_img[0];
	}
	else
	{
		frame.copyTo(cv_cropped_img[0]);
	}

#if FILE_WRITE
	FILE *fp = fopen("input.txt", "w");
#endif

	//float pixel;
	uchar pixel;
	int input_index=0;
	float float_val;
	short fxval;

	for (int h = 0; h < height; ++h)
	{
		const uchar* ptr = cv_cropped_img[0].ptr<uchar>(h);//data;
		int img_index = 0;
		for (int w = 0; w < width; ++w)
		{
			//TODO : Generic for 4
			for (int ch = 0; ch < 4; ++ch)
			{
				if(ch < img_channel)
				{
					//pixel = static_cast<float>(ptr[img_index++]);
					pixel = ptr[img_index++];

					float in_val = pixel/255.0;
					float mean_val = mean_ptr[mean_idx];
					int var_idx = (ch * resize_h + h + h_off) * resize_w + w + w_off;
					float var_val = variance_ptr[var_idx];

					in_val = (in_val - mean_val)/var_val;
					short ival = (short)float_val;
					fxval = ConvertToFP(float_val, ival, fbits_input);
				}
				else
				{
					float_val = 0;
					fxval = 0;
					in_ptr[input_index] = 0;
				}

				in_ptr[input_index] = fxval;

#if FILE_WRITE
				float_val = ((float)fxval)/(1 << fbits_input);
				fprintf(fp,"%f ", float_val);
#endif
				input_index++;
			}// Channels
		}// Image width
#if FILE_WRITE
		fprintf(fp,"\n");
#endif
	}// Image Height
#if FILE_WRITE
	fclose(fp);
#endif
}
// loadAndPrepareImage
#endif

//int loadImage(cv::Mat &frame, char *inptr, float* mean_data, int img_w, int img_h, int img_channel, int resize_h, int resize_w, const char *mean_path, int mean_status)
int loadImage(cv::Mat &frame, xChangeLayer *xChangeHostInpLayer)
{

	char  *in_ptr = (char *)xChangeHostInpLayer->in_ptrs[2];
	float *mean_ptr = (float *)xChangeHostInpLayer->mean;
	int *params_ptr = (int *)xChangeHostInpLayer->params;

	int height      = params_ptr[0];
	int width       = params_ptr[1];
	int img_channel = params_ptr[5];
	int resize_h    = xChangeHostInpLayer->resize_h;//params_ptr[0];    //??  how to handle
	int resize_w    = xChangeHostInpLayer->resize_w;//params_ptr[1];    //??

	int fbits_input = xChangeHostInpLayer->qf_format.ip_fbits;//7;//convLayer_precision[0].in_fbits;
	int h_off = 0;
	int w_off = 0;

	cv::Mat cv_img[1], cv_cropped_img[1];

	if((resize_h != frame.rows) || (resize_w != frame.cols))
	{
		cv::resize(frame, cv_img[0], cv::Size(resize_h, resize_w));

		if(resize_h > height)
		{
			h_off = (resize_h - height) / 2;
			w_off = (resize_w - width) / 2;
			cv::Rect roi(w_off, h_off, height, width);
			cv_cropped_img[0] = cv_img[0](roi);
		}
		else
			cv_cropped_img[0] = cv_img[0];
	}
	else
	{
		frame.copyTo(cv_cropped_img[0]);
	}

#if FILE_WRITE
	FILE *fp = fopen("input.txt", "w");
#endif

	char pixel;
	int input_index=0;

	for (int h = 0; h < height; ++h)
	{
		const uchar* ptr = cv_cropped_img[0].ptr<uchar>(h);
		int img_index = 0;
		for (int w = 0; w < width; ++w)
		{
			//TODO : Generic for 4
			for (int c = 0; c < 4; ++c)
			{
				if(c < img_channel)
				{
					pixel = static_cast<char>(ptr[img_index++]);
				}
				else
				{
					pixel = 0;
				}
				in_ptr[input_index] =  pixel;

#if FILE_WRITE
				fprintf(fp,"%d ", pixel);
#endif
				input_index++;
			}// Channels
		}// Image width
#if FILE_WRITE
		fprintf(fp,"\n");
#endif
	}// Image Height
#if FILE_WRITE
	fclose(fp);
#endif
}
//loadImage

int loadImagetoBuffptr1(const char *img_path, unsigned char *buff_ptr, int height, int width, int img_channel, int resize_h, int resize_w)
{

	cv::Mat frame;

	frame = imread(img_path, 1);
	if(!frame.data)
	{
		std :: cout << "[ERROR] Image read failed - " << img_path << std :: endl;
		//fprintf(stderr,"image read failed\n");
		//return -1;
	}

	int h_off = 0;
	int w_off = 0;

	cv::Mat cv_img[1], cv_cropped_img[1];

	if((resize_h != frame.rows) || (resize_w != frame.cols))
	{
		cv::resize(frame, cv_img[0], cv::Size(resize_h, resize_w));

		if(resize_h > height)
		{
			h_off = (resize_h - height) / 2;
			w_off = (resize_w - width) / 2;
			cv::Rect roi(w_off, h_off, height, width);
			cv_cropped_img[0] = cv_img[0](roi);
		}
		else
			cv_cropped_img[0] = cv_img[0];
	}
	else
	{
		frame.copyTo(cv_cropped_img[0]);
	}

	const uchar* ptr = cv_cropped_img[0].ptr<uchar>(0);
	for (int ind = 0; ind < height*width*img_channel; ind++)
	{
		buff_ptr[ind] = ptr[ind];
	}
}
//loadImage

int loadImagefromBuffptr(unsigned char *buff_ptr, xChangeLayer *xChangeHostInpLayer)
{

	char  *in_ptr = (char *)xChangeHostInpLayer->in_ptrs[2];
	int *params_ptr = (int *)xChangeHostInpLayer->params;

	int height      = params_ptr[0];
	int width       = params_ptr[1];
	int img_channel = params_ptr[5];

#if FILE_WRITE
	FILE *fp = fopen("input.txt", "w");
#endif

	char pixel;
	int input_index=0;

	int img_index = 0;
	for (int h = 0; h < height; ++h)
	{
		for (int w = 0; w < width; ++w)
		{
			//TODO : Generic for 4
			for (int c = 0; c < 4; ++c)
			{
				if(c < img_channel)
				{
					pixel = buff_ptr[img_index++];
				}
				else
				{
					pixel = 0;
				}
				in_ptr[input_index] =  pixel;

#if FILE_WRITE
				fprintf(fp,"%d ", pixel);
#endif
				input_index++;
			}// Channels
		}// Image width
#if FILE_WRITE
		fprintf(fp,"\n");
#endif
	}// Image Height
#if FILE_WRITE
	fclose(fp);
#endif
}
//loadImagefromBuffptr

int loadDatafromBuffptr(short *buff_ptr, xChangeLayer *xChangeHostInpLayer)
{

	short *in_ptr = (short *)xChangeHostInpLayer->in_ptrs[2];
	int *params_ptr = (int *)xChangeHostInpLayer->params;

	int height      = params_ptr[0];
	int width       = params_ptr[1];
	int img_channel = params_ptr[5];
	params_ptr[26]  = 0;   //mean_sub_flag


#if FILE_WRITE
	FILE *fp = fopen("input.txt", "w");
#endif

	short pixel;
	int input_index=0;

	int img_index = 0;
	for (int h = 0; h < height; ++h)
	{
		for (int w = 0; w < width; ++w)
		{
			//TODO : Generic for 4
			for (int c = 0; c < 4; ++c)
			{
				if(c < img_channel)
				{
					pixel = buff_ptr[img_index++];
				}
				else
				{
					pixel = 0;
				}
				in_ptr[input_index] =  pixel;

#if FILE_WRITE
				fprintf(fp,"%d ", pixel);
#endif
				input_index++;
			}// Channels
		}// Image width
#if FILE_WRITE
		fprintf(fp,"\n");
#endif
	}// Image Height
#if FILE_WRITE
	fclose(fp);
#endif
}
//loadDatafromBuffptr

int loadData(const char *img_data_path, xChangeLayer *xChangeHostInpLayer)
{

	short  *in_ptr = (short *)xChangeHostInpLayer->in_ptrs[2];
	int *params_ptr = (int *)xChangeHostInpLayer->params;

	int height      = params_ptr[0];
	int width       = params_ptr[1];
	int img_channel = params_ptr[5];
	params_ptr[26] = 0;   //making mean_sub_flag = 0;

	FILE* fp_img = fopen(img_data_path, "r");

	if(fp_img == NULL)
	{
		fprintf(stderr, "\n** Cannot open mean file (or) No mean file : %s **\n", img_data_path);
		//return NULL;
	}

	int size_of_input = height*width*img_channel;

	short *in_buf = (short *)malloc(size_of_input*sizeof(short));
	int fbits_input = xChangeHostInpLayer->qf_format.ip_fbits;

    float float_val;
    short fxval;
	for(int i = 0; i < height*width*img_channel; i++)
	{
		fscanf(fp_img, "%f ", &float_val);

		short ival = (short)float_val;
		fxval = ConvertToFP(float_val, ival, fbits_input);
		in_buf[i] = fxval;
	}
	fclose(fp_img);

#if FILE_WRITE
	FILE *fp = fopen("input.txt", "w");
#endif

	short pixel;
	int input_index=0;

	int img_index1 = 0;
	int img_index = 0;
	for (int h = 0; h < height; ++h)
	{
		for (int w = 0; w < width; ++w)
		{
			img_index1 = h*width + w;

			//TODO : Generic for 4
			for (int c = 0; c < 4; ++c)
			{
				img_index = img_index1 + c*(height*width);
				if(c < img_channel)
				{
					pixel = in_buf[img_index];
				}
				else
				{
					pixel = 0;
				}
				in_ptr[input_index] =  pixel;

#if FILE_WRITE
				fprintf(fp,"%d ", pixel);
#endif
				input_index++;
			}// Channels
		}// Image width
#if FILE_WRITE
		fprintf(fp,"\n");
#endif
	}// Image Height

#if FILE_WRITE
	fclose(fp);
#endif
}
//loadData

int loadDatatoBuffptr1(const char *img_data_path, float *inp_buff, int height, int width, int img_channel)
{
	FILE* fp_img = fopen(img_data_path, "r");

	if(fp_img == NULL)
	{
		fprintf(stderr, "\n** Cannot open mean file (or) No mean file : %s **\n", img_data_path);
		//return NULL;
	}

    float float_val;
 	for(int i = 0; i < height*width*img_channel; i++)
	{
		fscanf(fp_img, "%f ", &float_val);

		inp_buff[i] = float_val;
	}
	fclose(fp_img);

}
//loadDatatoBuffptr

int loadDatafromBuffptr1(short *inp_buff, xChangeLayer *xChangeHostInpLayer)
{

	short  *in_ptr = (short *)xChangeHostInpLayer->in_ptrs[2];
	int *params_ptr = (int *)xChangeHostInpLayer->params;

	int height      = params_ptr[0];
	int width       = params_ptr[1];
	int img_channel = params_ptr[5];
	params_ptr[26] = 0;   //making mean_sub_flag = 0;

	int size_of_input = height*width*img_channel;

 #if FILE_WRITE
	FILE *fp = fopen("input.txt", "w");
#endif

	short pixel;
	int input_index=0;

	int img_index1 = 0;
	int img_index = 0;
	for (int h = 0; h < height; ++h)
	{
		for (int w = 0; w < width; ++w)
		{
			img_index1 = h*width + w;

			//TODO : Generic for 4
			for (int c = 0; c < 4; ++c)
			{
				img_index = img_index1 + c*(height*width);
				if(c < img_channel)
				{
					pixel = inp_buff[img_index];
				}
				else
				{
					pixel = 0;
				}
				in_ptr[input_index] =  pixel;

#if FILE_WRITE
				fprintf(fp,"%d ", pixel);
#endif
				input_index++;
			}// Channels
		}// Image width
#if FILE_WRITE
		fprintf(fp,"\n");
#endif
	}// Image Height

#if FILE_WRITE
	fclose(fp);
#endif
}
//loadDatafromBuffptr

void inputImageRead(const char *img_path, xChangeLayer *xChangeHostInpLayer)
//void inputImageRead(FILE **file_list, xChangeLayer *xChangeHostInpLayer)
{

#if EN_IO_PRINT
	cout<< "inputImageRead : " <<endl;
#endif

	//char img_path[500];
	//fscanf(*file_list, "%s", img_path);

	int num_mean_values;  //read from params
	int status;

	//xChangeLayer *xChangeHostInpLayer = &lay_obj[imgId][layerId];// = &xChangeHostLayers[0];

	//xChangeHostInpLayer.in_ptrs[0] = (char*)create_sdsalloc_mem(mem_size);
	//xChangeHostInpLayer.params     = (char*)create_sdsalloc_mem(params_mem_size);

	int *scalar_conv_args = (int *)xChangeHostInpLayer->params;

	int mean_sub_flag = scalar_conv_args[26];

	cv::Mat in_frame, out_frame;

	out_frame = imread(img_path, 1);
	if(!out_frame.data)
	{
		std :: cout << "[ERROR] Image read failed - " << img_path << std :: endl;
		//fprintf(stderr,"image read failed\n");
		//return -1;
	}

	if(mean_sub_flag)
	{
		status = loadImage(out_frame, xChangeHostInpLayer);
	}
	else
	{
		status = loadAndPrepareImage(out_frame, xChangeHostInpLayer);
	}

#if EN_IO_PRINT
	cout<< "inputImageRead done: " <<endl;
#endif
}
