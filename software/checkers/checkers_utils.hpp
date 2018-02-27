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

#ifndef __XI_CHECKERS_UTIS_HPP__
#define __XI_CHECKERS_UTIS_HPP__


//# Checks Convolution/Pool funtionality
int cpCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path,
		FILE *&csv_fp//,
		//FILE *&error_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : cpCheck()" << std :: endl;

	short* hls_out1  = (short*)inLayer.out_ptrs[0];
	short* hls_out2 =  (short*)inLayer.out_ptrs[1];

	int *params = (int*)inLayer.params;

	int height;
	int width; 
	int indepth;
	bool relu;

	if(inLayer.kernType == CONV)
	{
		height = params[2];
		width  = params[3];

		if(params[30])
			indepth = params[4] << 1;
		else
			indepth = params[4];

		relu   = params[10];
	}
	else
	{
		height = params[2];
		width  = params[3];
		indepth = params[4] << 1;
		relu   = 1;
	}

	int depth = AlignSize(indepth, KER_PROC);
	int out_fbits = inLayer.qf_format.op_fbits;

	std :: cout << "[CHECKS] (h, w, d, out_fbits) : " << height << " " << width << " " << indepth << " " << out_fbits << std :: endl;
	//fprintf(error_fp, "[CHECKS] (h, w, d, out_fbits) : %d %d %d %d", height, width, indepth,out_fbits);

	int size = height*width*depth*sizeof(float);

	//# Buffers used to hold intermediate data
	float *ref_output		= (float*)malloc(size);
	float *kernel_output	= (float*)malloc(size);
	int *index_op			= (int*)malloc(size);

#if FILE_WRITE
	FILE *fout = fopen(output_file_path,"w");
	if(fout == NULL)
	{
		std :: cout << "can't create file - " << fout << std :: endl;
		return -1;
	}
#endif

	short hls_val;
	int cnt2 = 0;
	int out_index = 0;
	int cnt1 = 0;

	for(int i = 0; i < (height*width*depth); i++)
	{
		int plane = i/(height*width);
		int plane16 = plane%16;

		if(plane16<8)
		{
			hls_val = hls_out1[cnt1];
			cnt1++;
		}
		else
		{
			hls_val = hls_out2[cnt2];
			cnt2++;
		}

		float temp_val = ((float)hls_val)/(1 << out_fbits);
		kernel_output[out_index++] = temp_val;

#if FILE_WRITE
		if(i%(height*width*8)==0)
		{
			fprintf(fout,"\n");
		}

		if(i%(height*8)==0)
		{
			fprintf(fout,"\n");
		}
		fprintf(fout,"%f ",temp_val);
#endif

	}
#if FILE_WRITE
	fclose(fout);
#endif

	FILE *frefout=fopen(ref_path, "r");
	if(frefout == NULL)
	{
		std :: cout << "File not found - " << ref_path << std :: endl;
		return -1;
	}

	for(int i=0; i< height*width*depth;i++)
	{
		if(i < height*width*indepth)
			fscanf(frefout, "%f ", &ref_output[i]);
		else
			ref_output[i]=0;

		if(relu==1 && (ref_output[i]<0))
			ref_output[i]= 0;

	}
	fclose(frefout);

	int count=0;
	for(int ker=0; ker<depth;ker+=KER_PROC)
	{
		for(int op_size=0; op_size<width*height;op_size++)
		{
			int inx_ref_file = (ker*width*height) + op_size;

			for(int iii=0;iii<KER_PROC;iii++)
			{
				index_op[count+iii]   = inx_ref_file + (iii*width*height);
			}
			count+=KER_PROC;
		}
	}


	float max_diff = 0;

	float ref_out = 0;
	float tmp_f = 0;
	float ref_last = 0;
	float ker_last = 0;
	int error_cnt = 0;
	int max_err_cnt=0;

	int loop_cnt = width*height*indepth;
	int cnt_90_100 = 0, cnt_10_90 = 0, cnt_0_10 = 0, cnt_4_100 = 0, cnt_1_4 = 0, cnt_0_1 = 0;

	for(int i = 0; i < loop_cnt; i++)
	{
		tmp_f = kernel_output[i];

		ref_out = ref_output[index_op[i]];

		float diff = (ref_out-tmp_f);
		if(diff<0)
		{
			diff = -diff;
		}

		float error_per;

		if((( (ref_out==0) && (tmp_f!=0) ) || ( (ref_out!=0) && (tmp_f==0) )) && (diff>0.05) )
			error_per=100;
		else if(ref_out== tmp_f)
			error_per =0;
		else
			error_per = (diff/ref_out)*100;

		if(error_per>90)
			cnt_90_100++;
		else if(error_per>10)
			cnt_10_90++;
		else
			cnt_0_10++;


		if(error_per>4)
			cnt_4_100++;
		else if(error_per>1)
			cnt_1_4++;
		else
			cnt_0_1++;


		if((i<10))
		{
			fprintf(stderr,"\n[ERROR] <%d>error = %f  ref=%f   ker=%f", i, diff, ref_out, tmp_f);
			//fprintf(error_fp, "\n[ERROR] <%d>error = %f  ref=%f   ker=%f", i, diff, ref_out, tmp_f);
			//error_cnt++;
		}

		if((error_cnt<10) && (diff >5))
		{
			//fprintf(stderr,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
			error_cnt++;
		}

		//max_diff = MAX(max_diff,diff);
		if(diff > max_diff)
		{
			max_diff = diff;
		}


		if(max_diff == diff){
			ref_last = ref_out;
			ker_last = tmp_f;
			max_err_cnt = i;
		}
	}

	float norm_0_10 = ((float)cnt_0_10*100)/((float)loop_cnt);
	float norm_10_90 = ((float)cnt_10_90*100)/((float)loop_cnt);
	float norm_90_100 = ((float)cnt_90_100*100)/((float)loop_cnt);

#if EN_HISTOGRAM
	std :: cout << std :: endl;
	std :: cout << std :: endl;
	fprintf(csv_fp, "%f, %f, %f\n", norm_0_10, norm_10_90, norm_90_100);
	std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;
#endif

	fprintf(stderr, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_err_cnt, max_diff, ref_last, ker_last);
	//fprintf(error_fp, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_err_cnt, max_diff, ref_last, ker_last);

	//std :: cout << "Free memory!" << std :: endl;
	free(kernel_output);
	free(index_op);
	free(ref_output);
	//std :: cout << "Free memory done!" << std :: endl;

	if(max_diff > 0.1f)
		return -1;
	else
		return 0;

}
// cpCheck

//# Checks Convolution/Pool funtionality
int cpCheck_packed(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : cpCheck_packed()" << std :: endl;

	short* hls_out1  = (short*)inLayer.out_ptrs[0];
	short* hls_out2 =  (short*)inLayer.out_ptrs[1];

	int *params = (int*)inLayer.params;

	int height;
	int width;
	int indepth;
	bool relu;

	if(inLayer.kernType == CONV)
	{
		height = params[2];
		width  = params[3];

		if(params[30])
			indepth = params[4] << 1;
		else
			indepth = params[4];

		relu   = params[10];
	}
	else if(inLayer.kernType == NORM)
	{

		indepth		= params[0];
		height		= params[1];
		width		= params[2];

		relu   	= 0;//params[10];
	}
	else
	{
		height = params[2];
		width  = params[3];
		indepth = params[4] << 1;
		relu   = 1;
	}

	int depth = AlignSize(indepth, KER_PROC);
	int out_fbits = inLayer.qf_format.op_fbits;

	std :: cout << "[CHECKS] (h, w, d, out_fbits) : " << height << " " << width << " " << indepth << " " << out_fbits << std :: endl;

	int size = height*width*depth*sizeof(float);

	//# Buffers used to hold intermediate data
	float *ref_output		= (float*)malloc(size);
	float *kernel_output	= (float*)malloc(size);
	int *index_op			= (int*)malloc(size);

	short hls_val;
	int cnt2 = 0;
	int cnt1 = 0;

#if FILE_WRITE
	FILE *fout = fopen(output_file_path,"w");
	if(fout == NULL)
	{
		std :: cout << "can't create file - " << fout << std :: endl;
		return -1;
	}
#endif

	for(int i=0;i<(height*width*depth);i++)
	{
		int plane = i/(height*width);
		int plane16 = plane%16;
		if(plane16<8)
		{
			hls_val = hls_out1[cnt1];
			cnt1++;
		}
		else
		{
			hls_val = hls_out2[cnt2];
			cnt2++;
		}

		float temp_val = ((float)hls_val)/(1 << out_fbits);
		kernel_output[i] = temp_val;

#if FILE_WRITE
		if(i%(height*width*8)==0)
		{
			fprintf(fout,"\n");
		}

		if(i%(height*8)==0)
		{
			fprintf(fout,"\n");
		}
		fprintf(fout,"%f ",temp_val);
#endif

	}

#if FILE_WRITE
	fclose(fout);
#endif


	FILE *frefout=fopen(ref_path,"r");
	if(frefout == NULL)
	{
		std :: cout << "File not found - " << ref_path << std :: endl;
		return -1;
	}

	for(int i=0; i< height*width*depth;i++)
	{
		if(i < height*width*indepth)
			fscanf(frefout, "%f ", &ref_output[i]);
		else
			ref_output[i]=0;

		if(relu==1 && (ref_output[i]<0))
			ref_output[i]= 0;
	}
	fclose(frefout);

	int count=0;
	for(int ker=0; ker<depth;ker+=KER_PROC)
	{
		for(int op_size=0; op_size<width*height;op_size++)
		{
			int inx_ref_file = (ker*width*height) + op_size;

			for(int iii=0;iii<KER_PROC;iii++)
				index_op[count+iii]   = inx_ref_file + (iii*width*height);

			count+=KER_PROC;
		}
	}

	float max_diff = 0;

	float ref_out = 0;
	float tmp_f = 0;
	float ref_last = 0;
	float ker_last = 0;
	int cnt=0;
	int max_err_cnt=0;

	for(int i=0;i<width*height*indepth;i++)
	{

		tmp_f = kernel_output[i];

		ref_out = ref_output[i];

		float diff = (ref_out-tmp_f);
		if(diff<0)
		{
			diff = -diff;
		}

		if(i < 20)
		{
			fprintf(stderr,"\n[ERROR] pos = %d, error = %f, ref=%f, ker=%f", i, diff, ref_out, tmp_f);
		}

		if( (diff>1) && (cnt<10) )
		{
			cnt++;
			fprintf(stderr,"\n[ERROR] <%d>error = %f  ref=%f   ker=%f___________________________________***", i,diff, ref_out, tmp_f);
		}

		//max_diff = MAX(max_diff,diff);
		if(diff>max_diff)
		{
			max_diff=diff;
		}

		if(max_diff == diff){
			ref_last = ref_out;
			ker_last = tmp_f;
			max_err_cnt = i;
		}

	}

	fprintf(stderr, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n\n", max_err_cnt, max_diff, ref_last, ker_last);

	//std :: cout << "Free memory!" << std :: endl;
	free(kernel_output);
	free(index_op);
	free(ref_output);
	//std :: cout << "Free memory done!" << std :: endl;

	if(max_diff > 0.1f)
		return -1;
	else
		return 0;
}

//# Checks FC funtionality
int fcCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path,
		FILE *&csv_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : fcCheck()" << std :: endl;

	short *hls_out = (short*)inLayer.out_ptrs[0];
	int *params = (int*)inLayer.params;

	//# Output vector length
	int size  = params[0];
	bool relu = params[2];
	int out_fbits = inLayer.qf_format.op_fbits;

	std :: cout << "[CHECKS] (m, relu, fbits) : " << size << " " << relu << " " << out_fbits << std :: endl;

	float *kernel_output = (float*)malloc(size*sizeof(float*));
	float *ref_output = (float*)malloc(size*sizeof(float*));

#if FILE_WRITE
	FILE *foutput = fopen(output_file_path,"w");
#endif

	for(int i = 0; i < size; i++)
	{
		float temp_val = ((float)hls_out[i])/(1 << out_fbits);
		kernel_output[i] = temp_val;

#if FILE_WRITE
		if(i%(size)==0)
		{
			fprintf(foutput,"\n");
		}

		fprintf(foutput,"%f ",temp_val);
#endif
	}
#if FILE_WRITE
	fclose(foutput);
#endif

	FILE *frefout=fopen(ref_path,"r");
	if(frefout == NULL)
	{
		std :: cout << "File not found - " << ref_path << std :: endl;
		return -1;
	}

	for(int i=0; i < size; i++)
	{
		if(i < size)
			fscanf(frefout, "%f ", &ref_output[i]);
		else
			ref_output[i]=0;

		if(relu==1 && (ref_output[i]<0))
			ref_output[i]= 0;

	}
	fclose(frefout);

	float max_diff = 0;

	float ref_out = 0;
	float tmp_f = 0;
	float ref_last = 0;
	float ker_last = 0;
	int max_err_cnt=0;

	int cnt_90_100 = 0, cnt_10_90 = 0, cnt_0_10 = 0, cnt_4_100 = 0, cnt_1_4 = 0, cnt_0_1 = 0;

	for(int i = 0; i < size; i++)
	{

		tmp_f = kernel_output[i];

		ref_out = ref_output[i];

		float diff = (ref_out-tmp_f);
		if(diff<0)
		{
			diff = -diff;
		}

		float error_per;

		if((( (ref_out==0) && (tmp_f!=0) ) || ( (ref_out!=0) && (tmp_f==0) )) && (diff>0.05) )
			error_per=100;
		else if(ref_out== tmp_f)
			error_per =0;
		else
			error_per = (diff/ref_out)*100;

		if(error_per>90)
			cnt_90_100++;
		else if(error_per>10)
			cnt_10_90++;
		else
			cnt_0_10++;


		if(error_per>4)
			cnt_4_100++;
		else if(error_per>1)
			cnt_1_4++;
		else
			cnt_0_1++;

		if(i<10)
		{
			fprintf(stderr,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
		}

		if(diff > max_diff)
		{
			max_diff = diff;
		}


		if(max_diff == diff){
			ref_last = ref_out;
			ker_last = tmp_f;
			max_err_cnt = i;
		}
	}

	float norm_0_10 = ((float)cnt_0_10*100)/((float)size);
	float norm_10_90 = ((float)cnt_10_90*100)/((float)size);
	float norm_90_100 = ((float)cnt_90_100*100)/((float)size);

#if EN_HISTOGRAM
	std :: cout << std :: endl;
	fprintf(csv_fp, "%f, %f, %f\n", norm_0_10, norm_10_90, norm_90_100);
	std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;
#endif

	fprintf(stderr, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_err_cnt, max_diff, ref_last, ker_last);

	//fprintf(stderr, "\n$$$$$$$$$<%d> max error = %f, ref_value = %f,  kernal_op = %f\n\n", max_err_cnt, max_diff, ref_last, ker_last);

	//std :: cout << "Free memory" << std :: endl;
	free(kernel_output);
	//std :: cout << "Done" << std :: endl;
	if(max_diff > 0.1f)
		return -1;
	else
		return 0;
}

//# Checks softmax funtionality
int softmaxCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path
)
{
	int *hls_max_idx   = (int *)inLayer.out_ptrs[1];
	float *hls_max_val = (float *)inLayer.out_ptrs[0];

	// Print the softmax result
	fprintf(stderr, "\n\n*** Top 5 class-id : \n");
	for(int i = 0; i < 5; i++)
	{
		fprintf(stderr,"Id = %d\tProbability = %f \n", hls_max_idx[i], hls_max_val[i]);
	}
	return 0;
}

//# Checks crop layer output
int cropCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : cropCheck()" << std :: endl;

	int *hls_out = (int*)inLayer.out_ptrs[0];
	int *params = (int*)inLayer.params;

	int depth	= params[3];
	int height	= params[4];
	int width	= params[5];

	std :: cout << "[CHECKS] (d, h, w) : " << depth << " " << height << " " << width << std :: endl;

#if FILE_WRITE

	FILE *foutput = fopen(output_file_path,"w");
	if(foutput == NULL)
	{
		std :: cout << "can't create file : " << foutput << std :: endl;
		return -1;
	}

	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			fprintf(foutput,"%d ", hls_out[i*width+j]);
		}
		fprintf(foutput,"\n");
	}
	fclose(foutput);
#endif

	FILE *frefout=fopen(ref_path, "r");
	if(frefout == NULL)
	{
		std :: cout << "can't read file : " << frefout << std :: endl;
		return -1;
	}

	int ref_id = 0; int hls_val = 0;
	int max_diff = 0;

	float f_val;
	for(int i = 0; i < (height*width*1); i++)
	{
		//# Caffe ref output is in floating point
		fscanf(frefout, "%f ", &f_val);
		ref_id = (int)f_val;
		hls_val = hls_out[i];

		if(ref_id != hls_val)
		{
			max_diff++;
		}
	}
	fclose(frefout);

	fprintf(stderr,"\n[ERROR] max error count = %d\n", max_diff);
	return 0;
}
// cropCheck

//# Checks NMS functionality
int nmsCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path,
		FILE *&csv_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : nmsCheck()" << std :: endl;

	//TODO : Enable Anusha
	//int *params		= (int*)inLayer.params;
	//float *hls_out	= (float*)inLayer.out_ptrs[0];
	float *hls_out	= (float*)inLayer.xtra_ptrs[2];

	int *hls_int_out = (int *)inLayer.out_ptrs[1];
	int nms_finalboxcount = hls_int_out[0];

	std :: cout << "[ERROR] nms_finalboxcount : " << nms_finalboxcount << std :: endl;

	//Box_id Checking
	std :: cout << "[CHECKS] Box_id Checking ..." << std :: endl;
	//TODO : chack params
	//int depth	= nms_finalboxcount*7;//params[2];//49*7

	//TODO : chack params
	int depth	= 49*7;//params[2];//49*7
	int height	= 1;//params[0];  //nboxes
	int width	= 1;//params[1];  //nclasses
	bool relu	= 0;//params[4];

	std :: cout << "[CHECKS] (h, w, d) : " << height << " " << width << " " << depth << " " << std :: endl;

	int size = height*width*depth*sizeof(float);


	//# Buffers used to hold intermediate data
	float *ref_output		= (float*)malloc(size);
	float *kernel_output	= (float*)malloc(size);


#if FILE_WRITE
	FILE *foutput = fopen(output_file_path, "w");
#endif

	for(int i = 0; i < (height*width*depth); i++)
	{
		kernel_output[i] = hls_out[i];

#if FILE_WRITE
		if(i%(width)==0)
		{
			fprintf(foutput,"\n");
		}

		fprintf(foutput,"%f ", hls_out[i]);
#endif
	}
#if FILE_WRITE
	fclose(foutput);
#endif

	FILE *frefout=fopen(ref_path,"r");
	//FILE *frefout=fopen("/proj/sdxapps/refdes/anonymoX/optimization/SSDNet/caffe_ref/nms_score_cord.txt","r");

	for(int i=0; i< height*width*depth;i++)
	{
		if(i < height*width*depth)
			fscanf(frefout, "%f ", &ref_output[i]);
		else
			ref_output[i]=0;

		if(relu==1 && (ref_output[i]<0))
			ref_output[i]= 0;

	}
	fclose(frefout);

	float max_diff = 0;

	float ref_out = 0;
	float tmp_f = 0;
	float ref_last = 0;
	float ker_last = 0;
	int max_diff_cnt = 0;

	int loop_cnt = width*height*depth;
	int cnt_90_100 = 0, cnt_10_90 = 0, cnt_0_10 = 0, cnt_4_100 = 0, cnt_1_4 = 0, cnt_0_1 = 0;

	for(int i = 0; i < loop_cnt; i++)
	{

		tmp_f = kernel_output[i];

		ref_out = ref_output[i];

		float diff = (ref_out-tmp_f);
		if(diff<0)
		{
			diff = -diff;
		}

		float error_per;

		if((( (ref_out==0) && (tmp_f!=0) ) || ( (ref_out!=0) && (tmp_f==0) )) && (diff>0.05) )
			error_per=100;
		else if(ref_out== tmp_f)
			error_per =0;
		else
			error_per = (diff/ref_out)*100;

		if(error_per>90)
			cnt_90_100++;
		else if(error_per>10)
			cnt_10_90++;
		else
			cnt_0_10++;


		if(error_per>4)
			cnt_4_100++;
		else if(error_per>1)
			cnt_1_4++;
		else
			cnt_0_1++;

		if(i<10)
		{
			fprintf(stderr,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
		}

		if(diff > max_diff)
		{
			max_diff = diff;
		}

		if(max_diff == diff)
		{
			ref_last = ref_out;
			ker_last = tmp_f;
			max_diff_cnt = i;
		}
	}
	float norm_0_10 = ((float)cnt_0_10*100)/((float)loop_cnt);
	float norm_10_90 = ((float)cnt_10_90*100)/((float)loop_cnt);
	float norm_90_100 = ((float)cnt_90_100*100)/((float)loop_cnt);

#if EN_HISTOGRAM
	std :: cout << std :: endl;
	fprintf(csv_fp, "%f, %f, %f\n", norm_0_10, norm_10_90, norm_90_100);
	std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;
#endif

	fprintf(stderr, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_diff_cnt, max_diff, ref_last, ker_last);

	//std :: cout << "Free memory" << std :: endl;
	free(kernel_output);
	free(ref_output);
	//std :: cout << "Done" << std :: endl;
	if(max_diff > 0.1f)
		return -1;
	else
		return 0;

} // nmsCheck


//# Checks normalization functionality
int normCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path,
		FILE *&csv_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : normCheck()" << std :: endl;

	short* hls_out1  = (short*)inLayer.out_ptrs[0];
	short* hls_out2 =  (short*)inLayer.out_ptrs[1];

	int *params = (int*)inLayer.params;

	int indepth		= params[0];
	int height		= params[1];
	int width		= params[2];

	bool relu   	= 0;//params[10];

	int out_fbits = inLayer.qf_format.op_fbits;

	std :: cout << "[CHECKS] (h, w, d, relu, out_fbits) : " << height << " " << width << " " << indepth << " " << relu << " " << out_fbits << std :: endl;

	int depth = AlignSize(indepth, 4);

	int size = height*width*depth*sizeof(float);

	//# Buffers used to hold intermediate data
	float *ref_output		= (float*)malloc(size);
	float *kernel_output	= (float*)malloc(size);
	int *index_op			= (int*)malloc(size);

	//TODO : check this
	//	int buffer_split_flag = 0;

#if FILE_WRITE
	FILE *fout = fopen(output_file_path,"w");
	if(fout == NULL)
	{
		fprintf(stderr, "can't create file : %s\n", fout);
		//return -1;
	}
#endif

	//int t_size = height*width*depth;
	//	int half_size = t_size/2;
	short hls_val;
	int cnt2 = 0;
	int out_index = 0;
	int cnt1 = 0;
	//	short hls_val_ref;

	for(int i=0;i<(height*width*depth);i++)
	{

		int plane = i/(height*width);
		int plane16 = plane%16;

		if(plane16<8)
		{
			hls_val = hls_out1[cnt1];
			cnt1++;
		}
		else
		{
			hls_val = hls_out2[cnt2];
			cnt2++;
		}

		float temp_val = ((float)hls_val)/(1 << out_fbits);

		kernel_output[out_index++] = temp_val;

#if FILE_WRITE
		if(i%(height*width*4)==0)
		{
			fprintf(fout,"\n");
		}

		if(i%(height*4)==0)
		{
			fprintf(fout,"\n");
		}
		fprintf(fout,"%f ",temp_val);
#endif

	}
#if FILE_WRITE
	fclose(fout);
#endif

	FILE *frefout=fopen(ref_path,"r");
	if(frefout == NULL)
	{
		std :: cout << "File not found - " << ref_path << std :: endl;
		return -1;
	}

	for(int i=0; i< height*width*depth;i++)
	{
		if(i < height*width*indepth)
			fscanf(frefout, "%f ", &ref_output[i]);
		else
			ref_output[i]=0;

		//int relu =1;
		if(relu==1 && (ref_output[i]<0))
			ref_output[i]= 0;

	}
	fclose(frefout);

	int count=0;
	for(int ker=0; ker < depth; ker += KER_PROC)
	{
		for(int op_size=0; op_size<width*height;op_size++)
		{
			int inx_ref_file = (ker*width*height) + op_size;

			for(int iii=0;iii<KER_PROC;iii++)
				index_op[count+iii]   = inx_ref_file + (iii*width*height);
			count+=KER_PROC;
		}
	}

#if FILE_WRITE
	FILE *ferr = fopen("err.txt", "w");
#endif
	float max_diff = 0;

	float ref_out = 0;
	float tmp_f = 0;
	float ref_last = 0;
	float ker_last = 0;
	int error_cnt = 0;
	int max_err_cnt=0;

	int loop_cnt = width*height*depth;
	int cnt_90_100 = 0, cnt_10_90 = 0, cnt_0_10 = 0, cnt_4_100 = 0, cnt_1_4 = 0, cnt_0_1 = 0;

	for(int i = 0; i < loop_cnt; i++)
	{
		tmp_f = kernel_output[i];

		ref_out = ref_output[index_op[i]];

		float diff = (ref_out-tmp_f);
		if(diff<0)
		{
			diff = -diff;
		}

		float error_per;

		if((( (ref_out==0) && (tmp_f!=0) ) || ( (ref_out!=0) && (tmp_f==0) )) && (diff>0.05) )
			error_per=100;
		else if(ref_out== tmp_f)
			error_per =0;
		else
			error_per = (diff/ref_out)*100;

		if(error_per>90)
			cnt_90_100++;
		else if(error_per>10)
			cnt_10_90++;
		else
			cnt_0_10++;


		if(error_per>4)
			cnt_4_100++;
		else if(error_per>1)
			cnt_1_4++;
		else
			cnt_0_1++;

		/*	if((error_cnt<10) && (ref_out != 0.0))
		{
			fprintf(stderr,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
			error_cnt++;
		}*/

		if((i<10))
		{
			fprintf(stderr,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
			error_cnt++;
		}

#if FILE_WRITE
		if(diff > 4)
			fprintf(ferr,"\n<%d>error = %f  ref=%f   ker=%f**************", i,diff, ref_out, tmp_f);
		else
			fprintf(ferr,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#endif

		//max_diff = MAX(max_diff,diff);
		if(diff > max_diff)
		{
			max_diff = diff;
		}

		if(max_diff == diff){
			ref_last = ref_out;
			ker_last = tmp_f;
			max_err_cnt = i;
		}
	}

#if FILE_WRITE
	fclose(ferr);
#endif

	//TODO : Why to store
	//ref_output[0] = ref_last;
	//ref_output[1] = ker_last;

	float norm_0_10 = ((float)cnt_0_10*100)/((float)loop_cnt);
	float norm_10_90 = ((float)cnt_10_90*100)/((float)loop_cnt);
	float norm_90_100 = ((float)cnt_90_100*100)/((float)loop_cnt);

#if EN_HISTOGRAM
	std :: cout << std :: endl;
	fprintf(csv_fp, "%f, %f, %f\n", norm_0_10, norm_10_90, norm_90_100);
	std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;
#endif

	fprintf(stderr, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_err_cnt, max_diff, ref_last, ker_last);

	//std :: cout << "Free memory!" << std :: endl;
	free(kernel_output);
	free(index_op);
	free(ref_output);
	//std :: cout << "Free memory done!" << std :: endl;

	if(max_diff > 0.1f)
		return -1;
	else
		return 0;

}
// normCheck

//# Checks Permute functionality
int permuteCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path,
		FILE *&csv_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : permuteCheck()" << std :: endl;

	int *params		= (int*)inLayer.params;
	float *hls_out	= (float*)inLayer.out_ptrs[0];

	//TODO : chack params
	int depth	= params[0];
	int height	= params[1];
	int width	= params[2];
	int relu    = 0;

	std :: cout << "[CHECKS] (h, w, d) : " << height << " " << width << " " << depth << " " << std :: endl;

	int size = height*width*depth*sizeof(float);

	//# Buffers used to hold intermediate data
	float *ref_output		= (float*)malloc(size);
	float *kernel_output	= (float*)malloc(size);


#if FILE_WRITE
	FILE *foutput = fopen(output_file_path, "w");
#endif

	for(int i = 0; i < (height*width*depth); i++)
	{
		kernel_output[i] = hls_out[i];

#if FILE_WRITE
		if(i%(width)==0)
		{
			fprintf(foutput,"\n");
		}

		fprintf(foutput,"%f ", hls_out[i]);
#endif
	}
#if FILE_WRITE
	fclose(foutput);
#endif

	FILE *frefout=fopen(ref_path,"r");
	if(frefout == NULL)
	{
		std :: cout << "File not found - " << ref_path << std :: endl;
		return -1;
	}

	for(int i=0; i< height*width*depth;i++)
	{
		if(i < height*width*depth)
			fscanf(frefout, "%f ", &ref_output[i]);
		else
			ref_output[i]=0;

		if(relu==1 && (ref_output[i]<0))
			ref_output[i]= 0;

	}
	fclose(frefout);

	float max_diff = 0;

	float ref_out = 0;
	float tmp_f = 0;
	float ref_last = 0;
	float ker_last = 0;
	int max_diff_cnt = 0;

	int loop_cnt = width*height*depth;
	int cnt_90_100 = 0, cnt_10_90 = 0, cnt_0_10 = 0, cnt_4_100 = 0, cnt_1_4 = 0, cnt_0_1 = 0;


	for(int i = 0; i < loop_cnt; i++)
	{

		tmp_f = kernel_output[i];

		ref_out = ref_output[i];

		float diff = (ref_out-tmp_f);
		if(diff<0)
		{
			diff = -diff;
		}

		float error_per;

		if((( (ref_out==0) && (tmp_f!=0) ) || ( (ref_out!=0) && (tmp_f==0) )) && (diff>0.05) )
			error_per=100;
		else if(ref_out== tmp_f)
			error_per =0;
		else
			error_per = (diff/ref_out)*100;

		if(error_per>90)
			cnt_90_100++;
		else if(error_per>10)
			cnt_10_90++;
		else
			cnt_0_10++;


		if(error_per>4)
			cnt_4_100++;
		else if(error_per>1)
			cnt_1_4++;
		else
			cnt_0_1++;

		if(i<10)
		{
			fprintf(stderr,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
		}

		//max_diff = MAX(max_diff,diff);
		if(diff > max_diff)
		{
			max_diff = diff;
		}


		if(max_diff == diff){
			ref_last = ref_out;
			ker_last = tmp_f;
			max_diff_cnt = i;
		}
	}

	float norm_0_10 = ((float)cnt_0_10*100)/((float)loop_cnt);
	float norm_10_90 = ((float)cnt_10_90*100)/((float)loop_cnt);
	float norm_90_100 = ((float)cnt_90_100*100)/((float)loop_cnt);

#if EN_HISTOGRAM
	std :: cout << std :: endl;
	fprintf(csv_fp, "%f, %f, %f\n", norm_0_10, norm_10_90, norm_90_100);
	std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;
#endif

	fprintf(stderr,"\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_diff_cnt, max_diff, ref_last, ker_last);

	//std :: cout << "Free memory" << std :: endl;
	free(kernel_output);
	free(ref_output);
	//std :: cout << "Permute ErrorCheck Done" << std :: endl;

	if(max_diff > 0.1f)
		return -1;
	else
		return 0;

} // permuteCheck

int swSoftmaxCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path,
		FILE *&csv_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : swSoftmaxCheck()" << std :: endl;

	float* hls_out  = (float*)inLayer.out_ptrs[0];

	int *params = (int*)inLayer.params;

	int height		= params[0];
	int width		= params[1];
	int indepth		= 1;//params[4];
	bool relu   	= 0;//params[10];

	int depth = indepth;

	std :: cout << "[CHECKS] (h, w, d) : " << height << " " << width << " " << depth << " " << std :: endl;


	int size = height*width*depth*sizeof(float);

	//# Buffers used to hold intermediate data
	float *ref_output		= (float*)malloc(size);
	float *kernel_output	= (float*)malloc(size);

#if FILE_WRITE
	FILE *foutput = fopen(output_file_path,"w");
#endif

	for(int i=0;i<(height*width*depth);i++)
	{
		kernel_output[i] = hls_out[i];

#if FILE_WRITE
		fprintf(foutput,"%f", hls_out[i]);

		if(i%(width)==0)
		{
			fprintf(foutput,"\n");
		}
#endif
	}
#if FILE_WRITE
	fclose(foutput);
#endif

	FILE *frefout=fopen(ref_path,"r");
	if(frefout == NULL)
	{
		std :: cout << "File not found - " << ref_path << std :: endl;
		return -1;
	}

	for(int i=0; i< height*width*depth;i++)
	{
		if(i < height*width*indepth)
			fscanf(frefout, "%f ", &ref_output[i]);
		else
			ref_output[i]=0;

		if(relu==1 && (ref_output[i]<0))
			ref_output[i]= 0;

	}
	fclose(frefout);

	float max_diff = 0;

	float ref_out = 0;
	float tmp_f = 0;
	float ref_last = 0;
	float ker_last = 0;
	int max_diff_cnt = 0;

	int loop_cnt = width*height*depth;
	int cnt_90_100 = 0, cnt_10_90 = 0, cnt_0_10 = 0, cnt_4_100 = 0, cnt_1_4 = 0, cnt_0_1 = 0;

	for(int i = 0; i < loop_cnt; i++)
	{

		tmp_f = kernel_output[i];

		ref_out = ref_output[i];

		float diff = (ref_out-tmp_f);
		if(diff<0)
		{
			diff = -diff;
		}

		float error_per;

		if((( (ref_out==0) && (tmp_f!=0) ) || ( (ref_out!=0) && (tmp_f==0) )) && (diff>0.05) )
			error_per=100;
		else if(ref_out== tmp_f)
			error_per =0;
		else
			error_per = (diff/ref_out)*100;

		if(error_per>90)
			cnt_90_100++;
		else if(error_per>10)
			cnt_10_90++;
		else
			cnt_0_10++;


		if(error_per>4)
			cnt_4_100++;
		else if(error_per>1)
			cnt_1_4++;
		else
			cnt_0_1++;

		if(i<10)
		{
			fprintf(stderr,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
		}

		//max_diff = MAX(max_diff,diff);
		if(diff > max_diff)
		{
			max_diff = diff;
		}


		if(max_diff == diff){
			ref_last = ref_out;
			ker_last = tmp_f;
			max_diff_cnt = i;
		}
	}

	float norm_0_10 = ((float)cnt_0_10*100)/((float)loop_cnt);
	float norm_10_90 = ((float)cnt_10_90*100)/((float)loop_cnt);
	float norm_90_100 = ((float)cnt_90_100*100)/((float)loop_cnt);

#if EN_HISTOGRAM
	std :: cout << std :: endl;
	fprintf(csv_fp, "%f, %f, %f\n", norm_0_10, norm_10_90, norm_90_100);
	std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;
#endif

	fprintf(stderr,"\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_diff_cnt, max_diff, ref_last, ker_last);

	//std :: cout << "Free memory!" << std :: endl;
	free(kernel_output);
	free(ref_output);
	//std :: cout << "Free memory done!" << std :: endl;

	if(max_diff > 0.1f)
		return -1;
	else
		return 0;

}
// swSoftmaxCheck


void softmax_outfloatdatawrite1(float *input, int depth, int height, int width, const char *path)
{
	FILE *fp = fopen(path,"r");
	if(fp == NULL)
	{
		std :: cout << "File not found - " << path << std :: endl;
		//return -1;
	}


	int inputsize=height*width*depth;

	for(int i = 0; i < inputsize; i++)
	{
		fscanf(fp, "%f ", &input[i]);
	}
	fclose(fp);
}
// softmax_outfloatdatawrite1

void conv_loadinput_split(xChangeLayer inLayer, char *inp_path)
{
	char path[500];
	char *base_path = inLayer.base_path;
	int *params		= (int*)inLayer.params;

	//sprintf(path, "%s/outputs/group0_block0_eltwise_out.txt", base_path);
	//sprintf(path, "%s/caffe_ref/pool1_3x3_s2_out.txt", base_path);
	//sprintf(path, "%s/outputs/group1_block0_preact_bn_relu_out.txt", base_path);
	sprintf(path, "%s/outputs/pool0_out.txt", base_path);

	FILE *fp = fopen(inp_path,"r");
	fprintf(stderr,"input file = %s\n", inp_path);

	int fbits_input_other = inLayer.qf_format.ip_fbits;//inLayer->pa;//convLayer_args[layer_id].in_fbits;  //TODO
	fprintf(stderr, "fbits_input_other:%d\n", fbits_input_other);

	int buffer_split_flag = 1;  //TODO : Added values

	int in_depth	= params[4];
	int in_height	= params[0];
	int in_width	= params[1];


	int inputsize=in_height*in_width*in_depth;
	float *refinput;//[inputsize];
	refinput = (float*)malloc(inputsize*sizeof(float));
	if(refinput == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for tensor!\n");
	}


	for(int i = 0; i < inputsize; i++)
	{
		fscanf(fp, "%f ", &refinput[i]);
	}
	fclose(fp);

#if FILE_WRITE
	FILE *fpin = fopen("input.txt", "w");
#endif

	short* input_tb;
	short* input_tb1;

	if( (params[34] == OPCODE_CONV) )
	{
		input_tb  = (short*)inLayer.in_ptrs[0];
		input_tb1 =  (short*)inLayer.in_ptrs[1];
	}

	if( (params[34] == OPCODE_BN) || (params[34] == OPCODE_ELTWISE) )
	{
		input_tb  = (short*)inLayer.in_ptrs[2];
		input_tb1 =  (short*)inLayer.in_ptrs[3];
	}

	int idx1 = 0; int idx2 = 0;
	int plane16 = 0;
	for(int planes4 = 0; planes4 < in_depth; planes4 += KER_PROC, plane16++)
	{

		for(int i=0;i<(in_height);i++)
		{
			for(int j=0;j<in_width;j++)
			{
				for(int planes = 0; planes < KER_PROC; planes++)
				{
					//input_tb[j*4+i*IN_W+planes] = refinput[planes*(IN_W*IN_H)+j];
					//					int index_ref = (planes*(in_width*in_height)+j + i*in_width + ((in_width*in_height)*planes4));
					short ival = (short)( refinput[planes*(in_width*in_height)+j + i*in_width + ((in_width*in_height)*planes4)]);
					short fxval = ConvertToFP( refinput[planes*(in_width*in_height)+j + i*in_width + ((in_width*in_height)*planes4)], ival, fbits_input_other);
					//input_tb[j*4+i*IN_W+planes] = fxval;

					if(((plane16%2)==0) || (in_depth <=KER_PROC) || (buffer_split_flag==0))
					{
						input_tb[idx1++] = fxval;
						//fprintf(fpin, "%d ", input_tb[index]);
					}
					else
					{
						input_tb1[idx2++] = fxval;
						//fprintf(fpin, "%d ", input_tb1[idx2]);
					}
#if FILE_WRITE
					fprintf(fpin, "%d ", fxval);
#endif
					//printf("inputindex:%d\trefindex:%d\n",(int)(index),(int)(planes*(IN_W*IN_H)+j + i*IN_W + ((IN_W*IN_H)*planes4)));

				}
#if FILE_WRITE
				fprintf(fpin, "\n");
#endif
			}
		}
	}
#if FILE_WRITE
	fclose(fpin);
#endif


	free(refinput);
}
// conv_loadinput_split

void conv_loadinput_pack(xChangeLayer inLayer, char *path)
{

	//char path[500];
	char *base_path = inLayer.base_path;
	int *params		= (int*)inLayer.params;

	//sprintf(path, "%s/outputs/group0_block0_eltwise_out.txt", base_path);
	//sprintf(path, "%s/caffe_ref/pool1_3x3_s2_out.txt", base_path);
	//sprintf(path, "%s/out/group1_block0_preact_bn_relu_out.txt", base_path);
	//sprintf(path, "%s/out/pool0_out.txt", base_path);
	FILE *fp = fopen(path,"r");
	fprintf(stderr,"input file = %s\n", path);

	int fbits_input_other = inLayer.qf_format.ip_fbits;//inLayer->pa;//convLayer_args[layer_id].in_fbits;  //TODO
	fprintf(stderr, "fbits_input_other:%d\n", fbits_input_other);

	int buffer_split_flag = 1;  //TODO : Added values

	int in_depth	= params[4];
	int height	= params[0];
	int width	= params[1];

	//FILE *fp = fopen(path,"r");
	//fprintf(stderr,"input file = %s\n", path);
	int inputsize=height*width*in_depth;

	float *refinput;//[inputsize];
	refinput = (float*)malloc(inputsize*sizeof(float));
	if(refinput == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for tensor!\n");
	}

	for(int i = 0; i < inputsize; i++)
	{
		fscanf(fp, "%f ", &refinput[i]);
	}
	fclose(fp);

#if FILE_WRITE
	FILE *fpin = fopen("input.txt", "w");
#endif

	short* input_tb  = (short*)inLayer.in_ptrs[0];
	short* input_tb1 =  (short*)inLayer.in_ptrs[1];

	int cnt1=0;
	int cnt2=0;

	for(int i = 0; i < inputsize; i++)
	{
		//fscanf(fp, "%f ", &refinput[i]);

		float float_val =  refinput[i];
		short ival = (short)float_val;
		short fxval = ConvertToFP(float_val, ival, fbits_input_other);

		int plane = i/(height*width);
		int plane16 = plane%16;

		if(plane16<8)
		{
			input_tb[cnt1] = fxval;
			cnt1++;
		}
		else
		{
			input_tb1[cnt2] = fxval;
			cnt2++;
		}


		//input_tb[i] = fxval;
	}

	free(refinput);
}
// conv_loadinput_pack

////////////////////////////////////////	File Writes		//////////////////////////////////////

//# Writes softmax data to a file
int swSoftmaxWrite(
		xChangeLayer inLayer,
		const char *output_file_path
)
{
	float* hls_out  = (float*)inLayer.out_ptrs[0];
	int *params = (int*)inLayer.params;

	int height		= params[0];
	int width		= params[1];

	int depth = 1;//indepth;

	int size = height*width*depth*sizeof(float);

	FILE *foutput = fopen(output_file_path, "w");
	if(foutput == NULL)
	{
		std :: cout << "[ERROR] can't create file - " << output_file_path << std :: endl;
		return -1;
	}

	for(int i=0;i<(height*width*depth);i++)
	{

		fprintf(foutput,"%f", hls_out[i]);

		if(i%(width)==0)
		{
			fprintf(foutput,"\n");
		}
	}
	fclose(foutput);

	return 0;
}
// swSxWrite


//# Checks crop layer output
int cropWrite(
		xChangeLayer inLayer,
		const char *output_file_path
)
{
	int *hls_out = (int*)inLayer.out_ptrs[0];
	int *params = (int*)inLayer.params;

	int depth	= params[3];
	int height	= params[4];
	int width	= params[5];

	FILE *foutput = fopen(output_file_path, "w");
	if(foutput == NULL)
	{
		std :: cout << "[ERROR] can't create file : " << output_file_path << std :: endl;
		return -1;
	}

	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			fprintf(foutput,"%d ", hls_out[i*width+j]);
		}
		fprintf(foutput,"\n");
	}
	fclose(foutput);

	return 0;
}
// cropWrite

//# Writes nms output data to file
int nmsWrite(
		xChangeLayer inLayer,
		const char *output_file_path
)
{
	float *hls_out	= (float*)inLayer.xtra_ptrs[2];
	int *hls_int_out = (int *)inLayer.out_ptrs[1];
	int nms_finalboxcount = hls_int_out[0];

	//std :: cout << "[ERROR] nms_finalboxcount : " << nms_finalboxcount << std :: endl;

	//TODO : chack params
	int depth	= 49*7;//params[2];//49*7
	int height	= 1;//params[0];  //nboxes
	int width	= 1;//params[1];  //nclasses
	bool relu	= 0;//params[4];

	//std :: cout << "[CHECKS] (h, w, d) : " << height << " " << width << " " << depth << " " << std :: endl;

	int size = height*width*depth*sizeof(float);

	FILE *foutput = fopen(output_file_path, "w");
	if(foutput == NULL)
	{
		std :: cout << "[ERROR] can't create file : " << output_file_path << std :: endl;
		return -1;
	}

	fprintf(foutput,"nms_finalboxcount = %d\n", nms_finalboxcount);

	for(int i = 0; i < (height*width*depth); i++)
	{
		fprintf(foutput,"%f\n", hls_out[i]);
	}
	fclose(foutput);

	return 0;
}
// nmsWrite

const char* getFileNameFromPath(const char* path)
{
	const char *n_path;
	for(short i = strlen(path) - 1; i; i--)
	{
		if (path[i] == '/')
		{
			n_path = path+i+1;//&path[i+1];
			break;
		}
	}

	return n_path;
}

#endif      // __XI_CHECKERS_UTIS_HPP__
