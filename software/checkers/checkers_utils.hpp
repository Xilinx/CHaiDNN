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

#include "ap_int.h"
#include "../common/kernelinfo_class.h"

//# Checks Convolution/Pool funtionality
int cpCheck(
		xChangeLayer inLayer,
		FILE *&csv_fp,
		FILE *&error_fp
)
{
#if LAYERWISE_OUTPUT_WRITE

	//std :: cout << std :: endl;
	//std :: cout << "[CHECKS] start : cpCheck()" << std :: endl;
	
	const char *output_file_path = inLayer.out_path;
	FILE *fout = fopen(output_file_path, "w");
	if(fout == NULL)
	{
		std :: cout << "can't create file - " << fout << std :: endl;
		return -1;
	}
	
	int *params = (int*)inLayer.params;

	int height, width, indepth;
	bool relu;
	int en_batch_size_one;
	int offline_quant_mode;

	if(inLayer.kernType == CONV)
	{
		height = params[2];
		width  = params[3];

		if(params[30])  //for group case
			indepth = params[4] << 1;
		else
			indepth = params[4];

		relu   = params[10];
		en_batch_size_one = params[126];//58];
	}
	else if(inLayer.kernType == NORM)
	{
		height = params[1];
		width  = params[2];
		indepth = params[0];
		en_batch_size_one = params[8];
		offline_quant_mode = params[9];
	}
	else //POOL
	{
		height = params[2];
		width  = params[3];
		indepth = params[4];// << 1;  TODO:ANITHA
		relu   = 1;
		en_batch_size_one = params[20];
	}
	
	int out_fbits;

	//# Offline quant mode out_fbits are zero
	string quant; float th = 0.0, sf = 0.0;
	if((params[103] == 1) || (offline_quant_mode == 1) )
	{
		out_fbits = 0;
		quant = "Xilinx";
		th = inLayer.float_params[1]; sf = inLayer.float_params[3];
	}
	else
	{
		out_fbits = inLayer.qf_format.op_fbits;
		quant = "DynamicFixed";
	}

	std :: cout << "[CHECKS] (h, w, d, out_fbits, quant mode, th, sf) : " << height << " " << width << " " << indepth << " " << out_fbits << " " << quant << " " << th << " " << sf << std :: endl;
	
	IO_DATA_TYPE* hls_out1, *hls_out2;

	hls_out1 = (IO_DATA_TYPE*)inLayer.out_ptrs[0];

#if SINGLE_IO_PORT==0
	hls_out2 = (IO_DATA_TYPE*)inLayer.out_ptrs[1];
#endif

	int single_buf=0;
#if SINGLE_IO_PORT==0
	single_buf=0;
#else
	single_buf=1;
#endif
	
	float *kernel_output;
	kernel_output = (float*)malloc(XBATCH_SIZE*height*width*indepth*16*sizeof(float));
	if(kernel_output == NULL)
	{
		fprintf(stderr, "[ERROR] Failed to create memory\n");
		return -1;
	}
	
	//fprintf(stderr, "Before call :\n");
	SoftwareUnpack<IO_DATA_TYPE>(hls_out1, hls_out2, height, width, indepth, out_fbits, sf, th, quant,
	 kernel_output, single_buf);
	//fprintf(stderr, "After call\n");

	int idx = 0;
	
	for(int i = 0; i < XBATCH_SIZE; i++)
	{
		for(int j = 0; j < indepth; j++)
		{
			for(int k = 0; k < height*width; k++)
			{
				fprintf(fout, "%f ", kernel_output[idx++]);
			}
			fprintf(fout, "\n");
		}
		fprintf(fout, "\n\n");
	}
	fclose(fout);
		
	free(kernel_output);
	
	return 0;
#else
	
	////////////// ERROR CHECKS ///////////////
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : cpCheck()" << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] start : cpCheck()\n");
#endif

	const char *ref_path = inLayer.ref_path;
	const char *output_file_path = inLayer.out_path;

	/* for layer name */
	char *path = inLayer.out_path;

	char *ssc;
	int l = 0;
	ssc = strstr(path, "/");
	do{
		l = strlen(ssc) + 1;
		path = &path[strlen(path)-l+2];
		ssc = strstr(path, "/");
	}while(ssc);
	fprintf(stderr, "%s\n", path);

#if EN_HISTOGRAM
	fprintf(csv_fp, "%s,", path);
#endif

	int *params = (int*)inLayer.params;

	int height, width, indepth;
	bool relu;
	int en_batch_size_one;
	int offline_quant_mode;

	if(inLayer.kernType == CONV)
	{
		height = params[2];
		width  = params[3];

		if(params[30])  //for group case
			indepth = params[4] << 1;
		else
			indepth = params[4];

		relu   = params[10];
		en_batch_size_one = params[126];//58];

		int pool_split_cnt = params[110];
		if(pool_split_cnt>1)
		{
			indepth = params[109];
		}
	}
	else if(inLayer.kernType == NORM)
	{
		height = params[1];
		width  = params[2];
		indepth = params[0];
		en_batch_size_one = params[8];
		offline_quant_mode = params[9];
	}
	else if(inLayer.kernType == ELTWISEADD)
	{
		height = params[4];
		width  = params[5];
		indepth = params[3];
		en_batch_size_one = 0;//params[8];
		offline_quant_mode = 1;//params[9];
	}
	else //POOL
	{
		height = params[2];
		width  = params[3];
		indepth = params[4];// << 1;  TODO:ANITHA
		relu   = 1;
		en_batch_size_one = params[20];
	}

	fprintf(stderr, "\n\n");
	for (int i = 0; i < MAX_PARAM_SIZE; i++)
	{
		fprintf(stderr, "%d ", (int) params[i]);
	}

	int depth = AlignSize(indepth, KER_PROC);
	//TODO::Remove hardcoded
	//int out_fbits = 2;//inLayer.qf_format.op_fbits;
	int out_fbits;

	//# Offline quant mode out_fbits are zero
	if((params[103] == 1) || (offline_quant_mode == 1) )
	{
		out_fbits = 0;
	}
	else
		out_fbits = inLayer.qf_format.op_fbits;

	std :: cout << "\n[CHECKS] (h, w, d, out_fbits) : " << height << " " << width << " " << indepth << " " << out_fbits << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp, "\n[CHECKS] (h, w, d, out_fbits) : %d	 %d		%d		%d\n", height, width, indepth,out_fbits);
#endif
	int size = height*width*depth*sizeof(float);

	//# Buffers used to hold intermediate data
	float *ref_output		= (float*)malloc(size);

	float **kernel_output;
	kernel_output = (float**)malloc(XBATCH_SIZE*sizeof(float*));
	for(int i= 0; i < XBATCH_SIZE; i++)
	{
		kernel_output[i] = (float*)malloc(size);
	}

	int *index_op			= (int*)malloc(size);

#if FILE_WRITE
	FILE *fout = fopen(output_file_path,"w");
	if(fout == NULL)
	{
		std :: cout << "can't create file - " << fout << std :: endl;
		return -1;
	}
#endif

	IO_DATA_TYPE* hls_out1  = (IO_DATA_TYPE*)inLayer.out_ptrs[0];

#if SINGLE_IO_PORT==0
	IO_DATA_TYPE* hls_out2 =  (IO_DATA_TYPE*)inLayer.out_ptrs[1];
#endif


#if 0
	IO_DATA_TYPE* hls_out1  = (IO_DATA_TYPE*)inLayer.out_ptrs[0];
	IO_DATA_TYPE* hls_out2 =  (IO_DATA_TYPE*)inLayer.out_ptrs[1];


	FILE *fout = fopen(output_file_path,"w");
	if(fout == NULL)
	{
		std :: cout << "can't create file - " << fout << std :: endl;
		return -1;
	}

	int idx = 0;
	for(int i = 0; i < (height*width*depth/2); i++)
	{
		for(int j = 0; j < XBATCH_SIZE; j++)
		{
			fprintf(fout, "%d ", hls_out1[idx++]);
		}
	}
	for(int i = 0; i < (height*width*depth/2); i++)
	{
		for(int j = 0; j < XBATCH_SIZE; j++)
		{
			fprintf(fout, "%d ", hls_out2[idx++]);
		}
	}
	fclose(fout);
#endif

#if 0
	FILE *fp_pool1 = fopen("pool1_in","w");
	FILE *fp_pool2 = fopen("pool2_in","w");
	for(int i=0; i<2*112*112*32;i++){
		fprintf(fp_pool1,"%d ",hls_out1[i]);
		fprintf(fp_pool2,"%d ",hls_out2[i]);
	}
	fclose(fp_pool1);
	fclose(fp_pool2);
#endif
	IO_DATA_TYPE hls_val;

	int cnt2 = 0, cnt1 = 0;

	for(int i = 0; i < (height*width*depth); i++)
	{
		int plane = i/(height*width);
		//# 2x-IO ports
		int plane16 = plane%(2*PLANE_PACK);//16;

#if FILE_WRITE
		fprintf(fout,"(");
#endif

		for(int j = 0; j < XBATCH_SIZE; j++)
		{
#if IO_TYPE==16
			IO_DATA_TYPE hls_val6;
#else
			if (params[39] == 1)
			{
				ap_int<6> hls_val6;
			}
			else
			{
				IO_DATA_TYPE hls_val6;
			}
			IO_DATA_TYPE hls_val6;
#endif

			if(plane16 < PLANE_PACK)//8)
			{
				hls_val6 = hls_out1[cnt1];
				cnt1++;
			}
			else
			{
#if SINGLE_IO_PORT==0
				hls_val6 = hls_out2[cnt2];
				cnt2++;
#else
				hls_val6 = hls_out1[cnt1];
				cnt1++;
#endif
			}

			float temp_val = ((float)hls_val6)/(1 << out_fbits);

			//	if((i < 32) && (j == 0))
			//		fprintf(stderr, "plane16 = %d i = %d HLS out = %f\n", plane16, i, temp_val);

			kernel_output[j][i] = temp_val;

#if FILE_WRITE
			fprintf(fout,"%f, ",temp_val);
#endif
		}
#if FILE_WRITE
		fprintf(fout,")");
		if(i%(height*width*8)==0)
		{
			fprintf(fout,"\n");
		}

		if(i%(height*8)==0)
		{
			fprintf(fout,"\n");
		}
#endif
	}
#if FILE_WRITE
	fclose(fout);
#endif

	if(0)//params[12] == 4)
	{

		char path[500];
		sprintf(path, "%s_buf1.txt", output_file_path);
		FILE *fp_out1 = fopen(path, "w");
		if(fp_out1 == NULL)
		{
			fprintf(stderr, "failed to create file\n");
		}

		sprintf(path, "%s_buf2.txt", output_file_path);
		FILE *fp_out2 = fopen(path, "w");
		if(fp_out2 == NULL)
		{
			fprintf(stderr, "failed to create file\n");
		}

		for(int i = 0; i < (height*width*depth); i++)
		{
			if((i != 0) && (i%(2*height*width) == 0))
			{
				fprintf(fp_out1, "\n");
				fprintf(fp_out2, "\n");
			}

			fprintf(fp_out1, "%d ", hls_out1[i]);
#if SINGLE_IO_PORT==0
			fprintf(fp_out2, "%d ", hls_out2[i]);
#endif
		}
		fclose(fp_out1);
		fclose(fp_out2);
	}

	FILE *frefout=fopen(ref_path, "r");
	if(frefout == NULL)
	{
		std :: cout << "File not found - " << ref_path << std :: endl;
		for(int i= 0; i < XBATCH_SIZE; i++)
		{
			//free(kernel_output[i]);// = (float*)malloc(size);
		}
		free(kernel_output);
		free(index_op);
		free(ref_output);
#if EN_HISTOGRAM
		for(int i=0;i<4;i++)
		{
			fprintf(csv_fp, ",");
		}
		fprintf(csv_fp, "\n");
#endif
		return 0;//-1;
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
	for(int ker=0; ker<depth;ker+=PLANE_PACK)
	{
		for(int op_size=0; op_size<width*height;op_size++)
		{
			int inx_ref_file = (ker*width*height) + op_size;

			for(int iii=0;iii<PLANE_PACK;iii++)
			{
				index_op[count+iii]   = inx_ref_file + (iii*width*height);
			}
			count+=PLANE_PACK;
		}
	}


	int batch_loop_cnt = 0;
	if(en_batch_size_one)
		batch_loop_cnt = 1;
	else
		batch_loop_cnt = XBATCH_SIZE;

	float max_diff = 0;

	float ref_out = 0;
	float tmp_f = 0;
	float ref_last = 0;
	float ker_last = 0;
	int error_cnt = 0;
	int error_cnt2 = 0;
	int max_err_cnt=0;

	int loop_cnt = width*height*indepth;
	int cnt_90_100 = 0, cnt_10_90 = 0, cnt_0_10 = 0, cnt_4_100 = 0, cnt_1_4 = 0, cnt_0_1 = 0;

	float norm_0_10_batch[XBATCH_SIZE];

	int cnt_10_20 = 0, cnt_20_30 = 0, cnt_30_40 = 0, cnt_40_50 = 0;
	int cnt_50_60 = 0, cnt_60_70 = 0, cnt_70_80 = 0, cnt_80_90 = 0;

	for(int j = 0; j < batch_loop_cnt; j++)
	{
		max_diff = 0; max_err_cnt = 0;
		cnt_90_100 = 0; cnt_10_90 = 0; cnt_0_10 = 0; cnt_4_100 = 0; cnt_1_4 = 0; cnt_0_1 = 0;
		cnt_10_20 = 0, cnt_20_30 = 0, cnt_30_40 = 0, cnt_40_50 = 0;
		cnt_50_60 = 0, cnt_60_70 = 0, cnt_70_80 = 0, cnt_80_90 = 0;

		error_cnt=0;
		error_cnt2 = 0;
		fprintf(stderr, "[CHECKS] Batch : %d\n", j);

		for(int i = 0; i < loop_cnt; i++)
		{
			tmp_f = kernel_output[j][i];

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
			{
				cnt_90_100++;

				/*if(cnt_90_100<11)
				{
					fprintf(stderr,"\n<%d>error_90_100 = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#if EN_FILE_WRITE
					fprintf(error_fp,"\n<%d>error_90_100 = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#endif
				}*/
			}
			else if(error_per>80)
			{
				cnt_80_90++;
			}
			else if(error_per>70)
			{
				cnt_70_80++;
			}
			else if(error_per>60)
			{
				cnt_60_70++;
			}
			else if(error_per>50)
			{
				cnt_50_60++;
			}
			else if(error_per>40)
			{
				cnt_40_50++;
			}
			else if(error_per>30)
			{
				cnt_30_40++;
			}
			else if(error_per>20)
			{
				cnt_20_30++;
			}
			else if(error_per>10)
			{
				cnt_10_20++;
				cnt_10_90++;
				/*if(cnt_10_90<11)
				{
					fprintf(stderr,"\n<%d>error_10_90 = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#if EN_FILE_WRITE
					fprintf(error_fp,"\n<%d>error_10_90 = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#endif
				}*/
			}
			else
				cnt_0_10++;


			if(error_per>4)
				cnt_4_100++;
			else if(error_per>1)
				cnt_1_4++;
			else
				cnt_0_1++;

#if 0
			if((i<128*2))
			{
				fprintf(stderr,"\n[ERROR] <%d>error = %f  ref=%f   ker=%f", i, diff, ref_out, tmp_f);
				fprintf(error_fp, "\n[ERROR] <%d>error = %f  ref=%f   ker=%f", i, diff, ref_out, tmp_f);
				//error_cnt++;
			}
#endif

#if 1
			if((error_cnt2<32) && (diff > 0))
			{
				fprintf(stderr,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#if EN_FILE_WRITE
				fprintf(error_fp,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#endif
				error_cnt2++;
			}
#endif
			if((error_cnt<8) && (diff > 5))// && (tmp_f == 0.0f))
				//if((error_cnt<50))
			{
				fprintf(stderr,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#if EN_FILE_WRITE
				fprintf(error_fp,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#endif
				error_cnt++;
			}

			/*	if(max_diff > 120)
			{
				fprintf(stderr,"\n<%d>error = %f  ref=%f   ker=%f*************", i,diff, ref_out, tmp_f);
			}
			 */
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

		//# To compare both batches
		norm_0_10_batch[j] = norm_0_10;

		float norm_10_90 = ((float)cnt_10_90*100)/((float)loop_cnt);
		float norm_90_100 = ((float)cnt_90_100*100)/((float)loop_cnt);

		float norm_80_90 = ((float)cnt_80_90*100)/((float)loop_cnt);
		float norm_70_80 = ((float)cnt_70_80*100)/((float)loop_cnt);
		float norm_60_70 = ((float)cnt_60_70*100)/((float)loop_cnt);
		float norm_50_60 = ((float)cnt_50_60*100)/((float)loop_cnt);
		float norm_40_50 = ((float)cnt_40_50*100)/((float)loop_cnt);
		float norm_30_40 = ((float)cnt_30_40*100)/((float)loop_cnt);
		float norm_20_30 = ((float)cnt_20_30*100)/((float)loop_cnt);
		float norm_10_20 = ((float)cnt_10_20*100)/((float)loop_cnt);


		std :: cout << std :: endl;
		std :: cout << std :: endl;
		std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;

#if EN_HISTOGRAM
		if(j==0)
		{
			fprintf(csv_fp, "%d, %d, %f, %f, %f, ", params[17], params[19], norm_0_10, norm_10_90, norm_90_100);
		}
		else
		{
			if(norm_0_10_batch[0] == norm_0_10_batch[1])
				fprintf(csv_fp, "Pass\n");
			else
				fprintf(csv_fp, "Fail\n");
		}


		/*
		fprintf(csv_fp, "%d, %d, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n", params[17], params[19], norm_0_10, norm_10_20, norm_20_30,
				                                                                                    norm_30_40, norm_40_50, norm_50_60,
																									norm_60_70, norm_70_80, norm_80_90,
				                                                                                    norm_90_100);
		 */

#endif

		fprintf(stderr, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_err_cnt, max_diff, ref_last, ker_last);
#if EN_FILE_WRITE
		fprintf(error_fp, "\n[ERROR]  (0-10%) : %f (10-90%) : %f (90-100%) : %f\n", norm_0_10, norm_10_90, norm_90_100);
		fprintf(error_fp, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_err_cnt, max_diff, ref_last, ker_last);
#endif
	}

	//std :: cout << "Free memory!" << std :: endl;
	free(kernel_output);
	free(index_op);
	free(ref_output);
	//std :: cout << "Free memory done!" << std :: endl;

	if(max_diff > 0.1f)
		return -1;
	else
		return 0;
#endif  //if LAYERWISE_OUTPUT_WRITE

}
// cpCheck

//# Checks Convolution/Pool funtionality
int cpCheck_packed(
		xChangeLayer inLayer
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : cpCheck_packed()" << std :: endl;

	const char *ref_path = inLayer.ref_path;
	const char *output_file_path = inLayer.out_path;

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
	float **kernel_output;
	kernel_output = (float**)malloc(XBATCH_SIZE*sizeof(float*));
	for(int i= 0; i < XBATCH_SIZE; i++)
	{
		kernel_output[i] = (float*)malloc(size);
	}

	int *index_op			= (int*)malloc(size);

	IO_DATA_TYPE* hls_out1  = (IO_DATA_TYPE*)inLayer.out_ptrs[0];
#if SINGLE_IO_PORT==0
	IO_DATA_TYPE* hls_out2 =  (IO_DATA_TYPE*)inLayer.out_ptrs[1];
#endif

	IO_DATA_TYPE hls_val;

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

		for(int j = 0; j < XBATCH_SIZE;  j++)
		{
			if(plane16<8)
			{
				hls_val = hls_out1[cnt1];
				cnt1++;
			}
			else
			{
#if SINGLE_IO_PORT==0
				hls_val = hls_out2[cnt2];
				cnt2++;
#else
				hls_val = hls_out1[cnt1];
				cnt1++;
#endif
			}

			float temp_val = ((float)hls_val)/(1 << out_fbits);
			kernel_output[j][i] = temp_val;

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

	for(int j = 0; j < XBATCH_SIZE; j++)
	{
		max_diff = 0; max_err_cnt = 0;

		for(int i=0;i<width*height*indepth;i++)
		{
			tmp_f = kernel_output[j][i];
			ref_out = ref_output[i];

			float diff = (ref_out-tmp_f);
			if(diff<0)
			{
				diff = -diff;
			}

			if(i < 32)
			{
				fprintf(stderr,"\n[ERROR] pos = %d, error = %f, ref=%f, ker=%f", i, diff, ref_out, tmp_f);
			}

			if( (diff>1) && (cnt<10) )
			{
				cnt++;
				fprintf(stderr,"\n[ERROR] pos = %d, error = %f  ref=%f   ker=%f*******", i,diff, ref_out, tmp_f);
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
	}

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
		FILE *&csv_fp,
		FILE *&error_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : fcCheck()" << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] start : fcCheck()\n");
#endif

	const char *ref_path = inLayer.ref_path;
	const char *output_file_path = inLayer.out_path;

	/* for layer name */
	char *path = inLayer.out_path;

	char *ssc;
	int l = 0;
	ssc = strstr(path, "/");
	do{
		l = strlen(ssc) + 1;
		path = &path[strlen(path)-l+2];
		ssc = strstr(path, "/");
	}while(ssc);
	fprintf(stderr, "%s\n", path);

#if EN_HISTOGRAM
	fprintf(csv_fp, "%s,", path);
	fprintf(csv_fp, ",");
	fprintf(csv_fp, ",");
#endif

	int *params = (int*)inLayer.params;

	//# Output vector length
	//int size  = params[16]; // Alignment care added
	//bool relu = params[2];
	int size  = params[2]; // out_depth //Alignment care added
	bool relu = params[10];  //?? check
	int out_fbits = inLayer.qf_format.op_fbits;

	std :: cout << "[CHECKS] (m, relu, fbits) : " << size << " " << relu << " " << out_fbits << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp, "[CHECKS] (m, relu, fbits) : %d	%d	%d\n", size, relu, out_fbits);
#endif

	//float *kernel_output = (float*)malloc(size*sizeof(float*));
	float **kernel_output;
	kernel_output = (float**)malloc(XBATCH_SIZE*sizeof(float*));
	for(int i= 0; i < XBATCH_SIZE; i++)
	{
		kernel_output[i] = (float*)malloc(size*sizeof(float));
	}

	float *ref_output = (float*)malloc(size*sizeof(float*));

#if FILE_WRITE
	FILE *foutput = fopen(output_file_path,"w");
#endif

	//HFC_OUT_TYPE *hls_out = (HFC_OUT_TYPE*)inLayer.out_ptrs[0];
	IO_DATA_TYPE *hls_out = (IO_DATA_TYPE*)inLayer.out_ptrs[0];

	int idx = 0;
	for(int i = 0; i < size; i++)
	{
		for(int j = 0; j < XBATCH_SIZE; j++)
		{
			float temp_val = ((float)hls_out[idx++])/(1 << out_fbits);
			kernel_output[j][i] = temp_val;
			temp_val = ((float)hls_out[idx++])/(1 << out_fbits);

#if FILE_WRITE
			if(i%(size)==0)
			{
				fprintf(foutput,"\n");
			}

			fprintf(foutput,"%f ",temp_val);
#endif
		}
		idx+=12;
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

	for(int j = 0; j < XBATCH_SIZE; j++)
	{
		max_diff = 0; max_err_cnt = 0;
		cnt_90_100 = 0; cnt_10_90 = 0; cnt_0_10 = 0; cnt_4_100 = 0; cnt_1_4 = 0; cnt_0_1 = 0;

		for(int i = 0; i < size; i++)
		{

			tmp_f = kernel_output[j][i];

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
#if EN_FILE_WRITE
				fprintf(error_fp,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#endif
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

		std :: cout << std :: endl;
		std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;

#if EN_HISTOGRAM
		if(j==0)
			fprintf(csv_fp, "%f, %f, %f\n", norm_0_10, norm_10_90, norm_90_100);
#endif

		fprintf(stderr, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_err_cnt, max_diff, ref_last, ker_last);
#if EN_FILE_WRITE
		fprintf(error_fp, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_err_cnt, max_diff, ref_last, ker_last);
#endif
	}
	//fprintf(stderr, "\n$$$$$$$$$<%d> max error = %f, ref_value = %f,  kernal_op = %f\n\n", max_err_cnt, max_diff, ref_last, ker_last);

	//std :: cout << "Free memory" << std :: endl;
	free(kernel_output);
	//std :: cout << "Done" << std :: endl;
	if(max_diff > 0.1f)
		return -1;
	else
		return 0;
}

int swfcCheck(
		xChangeLayer inLayer,
		FILE *&csv_fp,
		FILE *&error_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : swFcCheck()" << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] start : swFcCheck()\n");
#endif

	const char *ref_path = inLayer.ref_path;
	const char *output_file_path = inLayer.out_path;

	/* for layer name */
	char *path = inLayer.out_path;

	char *ssc;
	int l = 0;
	ssc = strstr(path, "/");
	do{
		l = strlen(ssc) + 1;
		path = &path[strlen(path)-l+2];
		ssc = strstr(path, "/");
	}while(ssc);
	fprintf(stderr, "%s\n", path);

#if EN_HISTOGRAM
	fprintf(csv_fp, "%s,", path);
	fprintf(csv_fp, ",");
	fprintf(csv_fp, ",");
#endif

	float* hls_out  = (float*)inLayer.out_ptrs[0];

	int *params = (int*)inLayer.params;

	int height		= 1;//params[2];
	int width		= 1;//params[1];
	int indepth		= params[2];
	bool relu   	= 0;//params[10];

	int depth = indepth;

	std :: cout << "[CHECKS] (h, w, d) : " << height << " " << width << " " << depth << " " << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] (h, w, d) : %d	%d	%d\n",height,width,depth);
#endif

	int size = height*width*depth*sizeof(float);

	//# Buffers used to hold intermediate data
	float *ref_output		= (float*)malloc(size);
	//	float *kernel_output	= (float*)malloc(size);
	float **kernel_output;
	kernel_output = (float**)malloc(XBATCH_SIZE*sizeof(float*));
	for(int j = 0; j < XBATCH_SIZE; j++)
	{
		kernel_output[j] = (float*)malloc(size);
	}

#if FILE_WRITE
	FILE *foutput = fopen(output_file_path,"w");
#endif

	int idx = 0;
	int idx1 = 0;
	float *hls_out1 = &hls_out[indepth];
	for(int i = 0; i < (height*width*depth); i++)
	{
		for(int j = 0; j < XBATCH_SIZE; j++)
		{
			if(j==0)
			{
				kernel_output[j][i] = hls_out[idx++];
			}
			else
			{
				kernel_output[j][i] = hls_out1[idx1++];
			}

#if FILE_WRITE
			fprintf(foutput,"%f", kernel_output[j][i]);

			if(i%(width)==0)
			{
				fprintf(foutput,"\n");
			}
#endif
		}
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

	int en_batch_size_one = params[5];
	int batch_loop_cnt = 0;
	if(en_batch_size_one)
		batch_loop_cnt = 1;
	else
		batch_loop_cnt = XBATCH_SIZE;

	float max_diff = 0;

	float ref_out = 0;
	float tmp_f = 0;
	float ref_last = 0;
	float ker_last = 0;
	int max_diff_cnt = 0;

	int loop_cnt = width*height*depth;
	int cnt_90_100 = 0, cnt_10_90 = 0, cnt_0_10 = 0, cnt_4_100 = 0, cnt_1_4 = 0, cnt_0_1 = 0;

	float norm_0_10_batch[XBATCH_SIZE];

	for(int j = 0; j < 2; j++)
	{
		max_diff = 0; max_diff_cnt = 0;
		cnt_90_100 = 0; cnt_10_90 = 0; cnt_0_10 = 0; cnt_4_100 = 0; cnt_1_4 = 0; cnt_0_1 = 0;

		fprintf(stderr, "Batch : %d\n", j);

		for(int i = 0; i < loop_cnt; i++)
		{

			tmp_f = kernel_output[j][i];

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

			if(i<30)
			{
				fprintf(stderr,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#if EN_FILE_WRITE
				fprintf(error_fp,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#endif
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

		norm_0_10_batch[j] = norm_0_10;

		float norm_10_90 = ((float)cnt_10_90*100)/((float)loop_cnt);
		float norm_90_100 = ((float)cnt_90_100*100)/((float)loop_cnt);
		std :: cout << std :: endl;
		std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;

#if EN_HISTOGRAM
		if(j==0)
			fprintf(csv_fp, "%f, %f, %f, ", norm_0_10, norm_10_90, norm_90_100);
		else
		{
			if(norm_0_10_batch[0] == norm_0_10_batch[1])
				fprintf(csv_fp, "Pass\n");
			else
				fprintf(csv_fp, "Fail\n");
		}
#endif

		fprintf(stderr,"\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_diff_cnt, max_diff, ref_last, ker_last);
#if EN_FILE_WRITE
		fprintf(error_fp,"\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_diff_cnt, max_diff, ref_last, ker_last);
#endif
	}
	//std :: cout << "Free memory!" << std :: endl;
	free(kernel_output);
	free(ref_output);
	//std :: cout << "Free memory done!" << std :: endl;

	if(max_diff > 0.1f)
		return -1;
	else
		return 0;

}
// swFcCheck


//# Checks softmax funtionality
int softmaxCheck(
		xChangeLayer inLayer
)
{
	int *hls_max_idx   = (int *)inLayer.out_ptrs[1];
	float *hls_max_val = (float *)inLayer.out_ptrs[0];

	const char *ref_path = inLayer.ref_path;
	const char *output_file_path = inLayer.out_path;

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
		FILE *&error_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : cropCheck()" << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] start : cropCheck()\n");
#endif

	const char *ref_path = inLayer.ref_path;
	const char *output_file_path = inLayer.out_path;

	int *hls_out = (int*)inLayer.out_ptrs[0];
	int *params = (int*)inLayer.params;

	int depth	= params[3];
	int height	= params[4];
	int width	= params[5];

	std :: cout << "[CHECKS] (d, h, w) : " << depth << " " << height << " " << width << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] (d, h, w) : %d	%d	%d\n",depth,height,width);
#endif

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

	float f_val; int k = 0;
	int *ref_buf = (int*)malloc(height*width*sizeof(int));
	for(int i = 0; i < (height); i++)
	{
		for(int j = 0; j < width; j++)
		{
			//# Caffe ref output is in floating point
			fscanf(frefout, "%f ", &f_val);
			ref_buf[k++] = (int)f_val;
		}
	}
	fclose(frefout);


	int ref_id = 0; int hls_val = 0;
	int max_diff = 0;

	for(int batch_id = 0; batch_id < XBATCH_SIZE; batch_id++)
	{
		fprintf(stderr, "[CHECKS] Batch : %d\n", batch_id);
		k = 0; max_diff = 0;

		int idx = batch_id*(height*width*1);
		for(int i = 0; i < (height*width*1); i++)
		{
			ref_id = ref_buf[k++];
			hls_val = hls_out[idx+i];

			if(ref_id != hls_val)
			{
				max_diff++;
			}
		}

		fprintf(stderr,"\n[ERROR] max error count = %d\n", max_diff);
#if EN_FILE_WRITE
		fprintf(error_fp,"\n[ERROR] max error count = %d\n", max_diff);
#endif
	}

	free(ref_buf);

	return 0;
}
// cropCheck

//# Checks crop layer output
int deconvCheck(
		xChangeLayer inLayer,
		FILE *&error_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : deconvCheck()" << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] start : deconvCheck()\n");
#endif

	const char *ref_path = inLayer.ref_path;
	const char *output_file_path = inLayer.out_path;

	int *hls_out = (int*)inLayer.out_ptrs[0];
	int *params = (int*)inLayer.params;

	int depth	= 1;//params[0];
	int height	= params[4];
	int width	= params[3];
	int n_width = AlignSize(width, 2);

	std :: cout << "[CHECKS] (d, h, w) : " << depth << " " << height << " " << width << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] (d, h, w) : %d	%d	%d\n",depth,height,width);
#endif

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
			fprintf(foutput,"%d ", hls_out[i*n_width+j]);
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

	float f_val; int k = 0;
	int *ref_buf = (int*)malloc(height*width*sizeof(int));
	for(int i = 0; i < (height); i++)
	{
		for(int j = 0; j < width; j++)
		{
			//# Caffe ref output is in floating point
			fscanf(frefout, "%f ", &f_val);
			ref_buf[k++] = (int)f_val;
		}
	}
	fclose(frefout);

	int ref_id = 0; int hls_val = 0;
	int max_diff = 0;


	for(int batch_id = 0; batch_id < XBATCH_SIZE; batch_id++)
	{
		fprintf(stderr, "[CHECKS] Batch : %d\n", batch_id);
		k = 0; max_diff = 0;
		int idx = batch_id*(height*n_width*1);
		for(int i = 0; i < (height); i++)
		{
			for(int j = 0; j < width; j++)
			{
				ref_id = ref_buf[k++];
				hls_val = hls_out[idx+(i*n_width+j)];

				if(ref_id != hls_val)
				{
					max_diff++;
				}
			}
		}

		fprintf(stderr,"\n[ERROR] max error count = %d\n", max_diff);
#if EN_FILE_WRITE
		fprintf(error_fp,"\n[ERROR] max error count = %d\n", max_diff);
#endif
	}

	free(ref_buf);

	return 0;
}
// cropCheck


//# Checks NMS functionality
int nmsCheck(
		xChangeLayer inLayer,
		FILE *&csv_fp,
		FILE *&error_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : nmsCheck()" << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp, "[CHECKS] start : nmsCheck()\n");
#endif

	const char *ref_path = inLayer.ref_path;
	const char *output_file_path = inLayer.out_path;

	/* for layer name */
	char *path = inLayer.out_path;

	char *ssc;
	int l = 0;
	ssc = strstr(path, "/");
	do{
		l = strlen(ssc) + 1;
		path = &path[strlen(path)-l+2];
		ssc = strstr(path, "/");
	}while(ssc);
	fprintf(stderr, "%s\n", path);

#if EN_HISTOGRAM
	fprintf(csv_fp, "%s,", path);
	fprintf(csv_fp, ",");
	fprintf(csv_fp, ",");
#endif

	//TODO : Enable Anusha
	//int *params		= (int*)inLayer.params;
	float *hls_out	= (float*)inLayer.out_ptrs[0];
	//float *hls_out	= (float*)inLayer.xtra_ptrs[2];

	//int *hls_int_out = (int *)inLayer.out_ptrs[1];
	int nms_finalboxcount = hls_out[0];

	std :: cout << "[CHECKS] nms_finalboxcount : " << nms_finalboxcount << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp, "[CHECKS] nms_finalboxcount : %d\n", nms_finalboxcount);
#endif

	//Box_id Checking
	std :: cout << "[CHECKS] Box_id Checking ..." << std :: endl;
	//TODO : chack params
	//int depth	= nms_finalboxcount*7;//params[2];//49*7

	//TODO : chack params
	int depth	= nms_finalboxcount*7;//49*7;//params[2];//49*7
	int height	= 1;//params[0];  //nboxes
	int width	= 1;//params[1];  //nclasses
	bool relu	= 0;//params[4];

	std :: cout << "[CHECKS] (h, w, d) : " << height << " " << width << " " << depth << " " << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp, "[CHECKS] (h, w, d) : %d	%d	%d\n", height, width, depth);
#endif

	//# Allocation with max box count size
	int size = 200*7*sizeof(float);


	//# Buffers used to hold intermediate data
	float *ref_output		= (float*)malloc(size);
	//float *kernel_output	= (float*)malloc(size);
	float **kernel_output;
	kernel_output = (float**)malloc(XBATCH_SIZE*sizeof(float*));
	for(int i= 0; i < XBATCH_SIZE; i++)
	{
		kernel_output[i] = (float*)malloc(size);
	}

	int *params = (int*)inLayer.params;
	int en_batch_size_one = params[8];
	int batch_loop_cnt = 0;
	if(en_batch_size_one)
		batch_loop_cnt = 1;
	else
		batch_loop_cnt = XBATCH_SIZE;


#if FILE_WRITE
	FILE *foutput = fopen(output_file_path, "w");
#endif

	int idx = 0;

	for(int j = 0; j < batch_loop_cnt; j++)
	{
		hls_out += 1;
		for(int i = 0; i < (height*width*depth); i++)
		{
			//for(int j = 0; j < XBATCH_SIZE; j++)
			//{
			kernel_output[j][i] = hls_out[idx++];

#if FILE_WRITE
			if(i%(width)==0)
			{
				fprintf(foutput,"\n");
			}

			fprintf(foutput,"%f ", hls_out[i]);
#endif
		}
	}
#if FILE_WRITE
	fclose(foutput);
#endif

	FILE *frefout = fopen(ref_path,"r");
	if(frefout == NULL)
	{
		std :: cout << "can't read file : " << frefout << std :: endl;
		return -1;
	}
	//FILE *frefout=fopen("/proj/sdxapps/refdes/anonymoX/optimization/SSDNet/caffe_ref/nms_score_cord.txt","r");

	float val; int count = 0;
	while(fscanf(frefout, "%f", &ref_output[count]) == 1)
		count++;

	//fprintf(stderr, "NMS ref file size = %d\n", count);
	fclose(frefout);


	float max_diff = 0;

	float ref_out = 0;
	float tmp_f = 0;
	float ref_last = 0;
	float ker_last = 0;
	int max_diff_cnt = 0;

	int loop_cnt = count;
	int cnt_90_100 = 0, cnt_10_90 = 0, cnt_0_10 = 0, cnt_4_100 = 0, cnt_1_4 = 0, cnt_0_1 = 0;

	//for(int j = 0; j < XBATCH_SIZE; j++)
	for(int j = 0; j < batch_loop_cnt; j++)
	{
		max_diff = 0; max_diff_cnt = 0;
		cnt_90_100 = 0; cnt_10_90 = 0; cnt_0_10 = 0; cnt_4_100 = 0; cnt_1_4 = 0; cnt_0_1 = 0;

		for(int i = 0; i < loop_cnt; i++)
		{
			tmp_f = kernel_output[j][i];
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

			if(i<7*5)
			{
				fprintf(stderr,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#if EN_FILE_WRITE
				fprintf(error_fp,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#endif
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

		std :: cout << std :: endl;
		std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;

#if EN_HISTOGRAM
		if(j==0)
			fprintf(csv_fp, "%f, %f, %f\n", norm_0_10, norm_10_90, norm_90_100);
#endif

		fprintf(stderr, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_diff_cnt, max_diff, ref_last, ker_last);
#if EN_FILE_WRITE
		fprintf(error_fp, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_diff_cnt, max_diff, ref_last, ker_last);
#endif
	}

	//std :: cout << "Free memory" << std :: endl;
	free(kernel_output);
	free(ref_output);
	//std :: cout << "Done" << std :: endl;
	if(max_diff > 0.1f)
		return -1;
	else
		return 0;

} // nmsCheck
int xcustCheck(
		xChangeLayer inLayer,
		FILE *&error_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : xcustCheck()" << std :: endl;
	//fprintf(error_fp,"[CHECKS] start : xcustCheck()\n");

	const char *ref_path = inLayer.ref_path;
	const char *output_file_path = inLayer.out_path;

	float *hls_out = (float*)inLayer.out_ptrs[0];
	//int *params = (int*)inLayer.params;

	std::vector<int> dim=inLayer.output_dims[0];
	// TODO @raju these dimentions should be calculated with vector size

	int depth	= dim[1];
	int height	= dim[2];
	int width	= dim[3];
	int size=1;
	for(int iter_s=0;iter_s<dim.size();iter_s++){

		size=size*dim[iter_s];
		std :: cout << "[CHECKS] (d, h, w) : " << dim[iter_s] << std :: endl;
	}


	//fprintf(error_fp,"[CHECKS] (d, h, w) : %d	%d	%d\n",depth,height,width);

#if FILE_WRITE

	FILE *foutput = fopen(output_file_path,"w");
	if(foutput == NULL)
	{
		std :: cout << "can't create file : " << foutput << std :: endl;
		return -1;
	}
	for(int k = 0; k < depth; k++){
		for(int i = 0; i < height; i++)
		{
			for(int j = 0; j < width; j++)
			{
				fprintf(foutput,"%d ", hls_out[k*height*width + i*width+j]);
			}
			fprintf(foutput,"\n");
		}
	}
	fclose(foutput);
#endif

	FILE *frefout=fopen(ref_path, "r");
	if(frefout == NULL)
	{
		std :: cout << "can't read file : " << frefout << std :: endl;
		return -1;
	}
	int count=0;

	float *ref_output	= (float*)malloc(size*sizeof(float));
	float tmp=0.0;
	while(fscanf(frefout, "%f", &tmp)==1){
		ref_output[count++]=tmp;
	}
	fclose(frefout);

	float hls_val = 0, ref_val=0, max_hls=0;
	float max_diff = 0, diff = 0;int pos;
	int cnt_90_100 = 0, cnt_10_90 = 0, cnt_0_10 = 0, cnt_4_100 = 0, cnt_1_4 = 0, cnt_0_1 = 0;
	float max_err_per=0;

	float f_val;

	//int size = height*width*depth;
	for(int j=0;j<XBATCH_SIZE;j++){

		max_diff = 0;
		cnt_90_100 = 0; cnt_10_90 = 0; cnt_0_10 = 0; cnt_4_100 = 0; cnt_1_4 = 0; cnt_0_1 = 0;
		diff=0,max_err_per=0,ref_val=0,max_hls=0;
		for(int i = 0; i < size; i++)
		{
			//# Caffe ref output is in floating point
			//fscanf(frefout, "%f ", &f_val);
			f_val=ref_output[i];
			hls_val = hls_out[i+j*size];

			diff = f_val - hls_val;
			if(diff < 0) {
				diff = -diff;
			}

			float error_per;// = (diff/f_val)*100;
			if( ( ((f_val==0) && (hls_val!=0)) || ((f_val!=0) && (hls_val==0)) ) && (diff>0.05f) )
				error_per= 100;
			else if(f_val == hls_val)
				error_per = 0;
			else
				error_per = (diff/f_val)*100;

			if(error_per>90)
				cnt_90_100++;
			else if(error_per>10)
				cnt_10_90++;
			else
				cnt_0_10++;

			if(i <  20 && diff>0)
			{
				fprintf(stderr, "[INFOx] pos : %d err_per : %f diff = %f ref = %f hls = %f\n", i, error_per, diff, f_val, hls_val);
			}

			if(max_diff < diff)
			{
				pos = i;
				max_diff = diff;
				ref_val = f_val;
				max_hls = hls_val;
				max_err_per = error_per;
			}
		}

		float norm_0_10 = ((float)cnt_0_10*100)/((float)size);
		float norm_10_90 = ((float)cnt_10_90*100)/((float)size);
		float norm_90_100 = ((float)cnt_90_100*100)/((float)size);

		std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;

		fprintf(stderr, "[CHECKS] pos : %d max error : %f err_per : %f ref value : %f hls out : %f\n", pos, max_diff, max_err_per, ref_val, max_hls);
	}


	return 0;
}


//# Checks normalization functionality
int normCheck(
		xChangeLayer inLayer,
		FILE *&csv_fp,
		FILE *&error_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : normCheck()" << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] start : normCheck()\n");
#endif

	const char *ref_path = inLayer.ref_path;
	const char *output_file_path = inLayer.out_path;

	IO_DATA_TYPE* hls_out1  = (IO_DATA_TYPE*)inLayer.out_ptrs[0];
	IO_DATA_TYPE* hls_out2 =  (IO_DATA_TYPE*)inLayer.out_ptrs[1];

	int *params = (int*)inLayer.params;

	int indepth		= params[0];
	int height		= params[1];
	int width		= params[2];

	bool relu   	= 0;//params[10];

	int out_fbits = inLayer.qf_format.op_fbits;

#if (IO_TYPE==8)
	out_fbits = 3;
#endif

	std :: cout << "[CHECKS] (h, w, d, relu, out_fbits) : " << height << " " << width << " " << indepth << " " << relu << " " << out_fbits << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] (h, w, d, relu, out_fbits) : %d	%d	%d	%d	%d\n",height,width,indepth,relu,out_fbits);
#endif

	int depth = AlignSize(indepth, 4);

	int size = height*width*depth*sizeof(float);

	//# Buffers used to hold intermediate data
	float *ref_output		= (float*)malloc(size);
	//float *kernel_output	= (float*)malloc(size);
	float **kernel_output;
	kernel_output = (float**)malloc(XBATCH_SIZE*sizeof(float*));
	for(int i= 0; i < XBATCH_SIZE; i++)
	{
		kernel_output[i] = (float*)malloc(size);
	}

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
	IO_DATA_TYPE hls_val;
	int cnt2 = 0;
	//int out_index = 0;
	int cnt1 = 0;
	//	short hls_val_ref;

	for(int i=0;i<(height*width*depth);i++)
	{

		int plane = i/(height*width);
		int plane16 = plane%16;

		for(int j = 0; j < XBATCH_SIZE; j++)
		{
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

			kernel_output[j][i] = temp_val;

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
		//	out_index++;

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

	int en_batch_size_one = params[8];
	int batch_loop_cnt = 0;
	if(en_batch_size_one)
		batch_loop_cnt = 1;
	else
		batch_loop_cnt = XBATCH_SIZE;

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

	for(int j = 0; j < batch_loop_cnt; j++)
	{
		max_diff = 0; max_err_cnt = 0;
		cnt_90_100 = 0; cnt_10_90 = 0; cnt_0_10 = 0; cnt_4_100 = 0; cnt_1_4 = 0; cnt_0_1 = 0;

		fprintf(stderr, "Batch : %d\n", j);

		for(int i = 0; i < loop_cnt; i++)
		{
			tmp_f = kernel_output[j][i];

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
#if EN_FILE_WRITE
				fprintf(error_fp,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#endif
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

		std :: cout << std :: endl;
		std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;

#if EN_HISTOGRAM
		if(j==0)
			fprintf(csv_fp, "%f, %f, %f\n", norm_0_10, norm_10_90, norm_90_100);
#endif

		fprintf(stderr, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_err_cnt, max_diff, ref_last, ker_last);
#if EN_FILE_WRITE
		fprintf(error_fp, "\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_err_cnt, max_diff, ref_last, ker_last);
#endif
	}
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
		FILE *&csv_fp,
		FILE *&error_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : permuteCheck()" << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] start : permuteCheck()\n");
#endif

	const char *ref_path = inLayer.ref_path;
	const char *output_file_path = inLayer.out_path;

	/* for layer name */
	char *path = inLayer.out_path;

	char *ssc;
	int l = 0;
	ssc = strstr(path, "/");
	do{
		l = strlen(ssc) + 1;
		path = &path[strlen(path)-l+2];
		ssc = strstr(path, "/");
	}while(ssc);
	fprintf(stderr, "%s\n", path);

#if EN_HISTOGRAM
	fprintf(csv_fp, "%s,", path);
	fprintf(csv_fp, ",");
	fprintf(csv_fp, ",");
#endif

	int *params		= (int*)inLayer.params;
	IO_DATA_TYPE *hls_out	= (IO_DATA_TYPE *)inLayer.out_ptrs[0];

	//TODO : chack params
	int depth	= params[0];
	int height	= params[1];
	int width	= params[2];
	int inp_fbits = params[3];
	int relu    = 0;

	int offline_quant_mode = params[6];

	//# Offline quant mode out_fbits are zero
	if(offline_quant_mode == 1)
	{
		inp_fbits = 0;
	}


	std :: cout << "[CHECKS] (h, w, d) : " << height << " " << width << " " << depth << " " << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] (h, w, d) : %d	%d	%d\n", height,width,depth);
#endif

	int size = height*width*depth*sizeof(float);

	//# Buffers used to hold intermediate data
	float *ref_output		= (float*)malloc(size);
	//float *kernel_output	= (float*)malloc(size);
	float **kernel_output;
	kernel_output = (float**)malloc(XBATCH_SIZE*sizeof(float*));
	for(int j = 0; j < XBATCH_SIZE; j++)
	{
		kernel_output[j] = (float*)malloc(size);
	}


#if FILE_WRITE
	FILE *foutput = fopen(output_file_path, "w");
#endif

	int idx=0;
	for(int i = 0; i < (height*width*depth); i++)
	{
		for(int j = 0; j < XBATCH_SIZE; j++)
		{
			//kernel_output[i] = hls_out[i];

			IO_DATA_TYPE hls_val = hls_out[idx++];
			float float_val = ((float)hls_val)/(1 << inp_fbits);
			kernel_output[j][i] = float_val;

#if FILE_WRITE
			if(i%(width)==0)
			{
				fprintf(foutput,"\n");
			}

			fprintf(foutput,"%f ", hls_out[i]);
#endif
		}  //for(int j = 0; j < XBATCH_SIZE; j++)
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
		fscanf(frefout, "%f ", &ref_output[i]);

		if(relu==1 && (ref_output[i]<0))
			ref_output[i]= 0;

	}
	fclose(frefout);

	int en_batch_size_one = params[5];
	int batch_loop_cnt = 0;
	if(en_batch_size_one)
		batch_loop_cnt = 1;
	else
		batch_loop_cnt = XBATCH_SIZE;

	float max_diff = 0;

	float ref_out = 0;
	float tmp_f = 0;
	float ref_last = 0;
	float ker_last = 0;
	int max_diff_cnt = 0;

	int loop_cnt = width*height*depth;
	int cnt_90_100 = 0, cnt_10_90 = 0, cnt_0_10 = 0, cnt_4_100 = 0, cnt_1_4 = 0, cnt_0_1 = 0;

	for(int j = 0; j < batch_loop_cnt; j++)
	{
		max_diff = 0; max_diff_cnt = 0;
		cnt_90_100 = 0; cnt_10_90 = 0; cnt_0_10 = 0; cnt_4_100 = 0; cnt_1_4 = 0; cnt_0_1 = 0;

		fprintf(stderr, "Batch : %d\n", j);
		for(int i = 0; i < loop_cnt; i++)
		{

			tmp_f = kernel_output[j][i];

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

			if((i<10) && (diff > 0))
			{
				fprintf(stderr,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#if EN_FILE_WRITE
				fprintf(error_fp,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#endif
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

		std :: cout << std :: endl;
		std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;

#if EN_HISTOGRAM
		if(j==0)
			fprintf(csv_fp, "%f, %f, %f\n", norm_0_10, norm_10_90, norm_90_100);
#endif

		fprintf(stderr,"\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_diff_cnt, max_diff, ref_last, ker_last);
#if EN_FILE_WRITE
		fprintf(error_fp,"\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_diff_cnt, max_diff, ref_last, ker_last);
#endif
	}  //for(int j = 0; j < XBATCH_SIZE; j++)

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
		FILE *&csv_fp,
		FILE *&error_fp
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] start : swSoftmaxCheck()" << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] start : swSoftmaxCheck()\n");
#endif

	const char *ref_path = inLayer.ref_path;
	const char *output_file_path = inLayer.out_path;

	/* for layer name */
	char *path = inLayer.out_path;

	char *ssc;
	int l = 0;
	ssc = strstr(path, "/");
	do{
		l = strlen(ssc) + 1;
		path = &path[strlen(path)-l+2];
		ssc = strstr(path, "/");
	}while(ssc);
	fprintf(stderr, "%s\n", path);

#if EN_HISTOGRAM
	fprintf(csv_fp, "%s,", path);
	fprintf(csv_fp, ",");
	fprintf(csv_fp, ",");
#endif

	float* hls_out  = (float*)inLayer.out_ptrs[0];

	int *params = (int*)inLayer.params;

	int height		= params[0];
	int width		= params[1];
	int indepth		= 1;//params[4];
	bool relu   	= 0;//params[10];

	int depth = indepth;

	std :: cout << "[CHECKS] (h, w, d) : " << height << " " << width << " " << depth << " " << std :: endl;
#if EN_FILE_WRITE
	fprintf(error_fp,"[CHECKS] (h, w, d) : %d	%d	%d\n",height,width,depth);
#endif

	int size = height*width*depth*sizeof(float);

	//# Buffers used to hold intermediate data
	float *ref_output		= (float*)malloc(size);
	//	float *kernel_output	= (float*)malloc(size);
	float **kernel_output;
	kernel_output = (float**)malloc(XBATCH_SIZE*sizeof(float*));
	for(int j = 0; j < XBATCH_SIZE; j++)
	{
		kernel_output[j] = (float*)malloc(size);
	}

#if FILE_WRITE
	FILE *foutput = fopen(output_file_path,"w");
#endif

	int idx = 0;
	for(int i = 0; i < (height*width*depth); i++)
	{
		for(int j = 0; j < XBATCH_SIZE; j++)
		{
			kernel_output[j][i] = hls_out[idx++];

#if FILE_WRITE
			fprintf(foutput,"%f", kernel_output[j][i]);

			if(i%(width)==0)
			{
				fprintf(foutput,"\n");
			}
#endif
		}
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

	int en_batch_size_one = params[5];
	int batch_loop_cnt = 0;
	if(en_batch_size_one)
		batch_loop_cnt = 1;
	else
		batch_loop_cnt = XBATCH_SIZE;

	float max_diff = 0;

	float ref_out = 0;
	float tmp_f = 0;
	float ref_last = 0;
	float ker_last = 0;
	int max_diff_cnt = 0;

	int loop_cnt = width*height*depth;
	int cnt_90_100 = 0, cnt_10_90 = 0, cnt_0_10 = 0, cnt_4_100 = 0, cnt_1_4 = 0, cnt_0_1 = 0;

	for(int j = 0; j < batch_loop_cnt; j++)
	{
		max_diff = 0; max_diff_cnt = 0;
		cnt_90_100 = 0; cnt_10_90 = 0; cnt_0_10 = 0; cnt_4_100 = 0; cnt_1_4 = 0; cnt_0_1 = 0;

		fprintf(stderr, "Batch : %d\n", j);

		for(int i = 0; i < loop_cnt; i++)
		{

			tmp_f = kernel_output[j][i];

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
#if EN_FILE_WRITE
				fprintf(error_fp,"\n<%d>error = %f  ref=%f   ker=%f", i,diff, ref_out, tmp_f);
#endif
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
		std :: cout << std :: endl;
		std::cout << "[ERROR]  (0-10%) : " << norm_0_10 <<  "  (10-90%) : "  << norm_10_90 << "  (90-100%) : " << norm_90_100 << std :: endl;

#if EN_HISTOGRAM
		if(j==0)
			fprintf(csv_fp, "%f, %f, %f\n", norm_0_10, norm_10_90, norm_90_100);
#endif

		fprintf(stderr,"\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_diff_cnt, max_diff, ref_last, ker_last);
#if EN_FILE_WRITE
		fprintf(error_fp,"\n[ERROR] pos = %d, max error = %f, ref_value = %f,  kernal_op = %f\n", max_diff_cnt, max_diff, ref_last, ker_last);
#endif
	}
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

void softmax_outfloatdatawrite2(float *input, int depth, int height, int width, const char *path, int batch_size)
{
	FILE *fp = fopen(path,"r");
	if(fp == NULL)
	{
		std :: cout << "File not found - " << path << std :: endl;
		//return -1;
	}

#if 0
#ifdef __SDSOC
	FILE *fp1 = fopen("int8_models/SSD_300_300_8Bit/out/mbox_conf_flatten_out_out.txt","w");
#else
	FILE *fp1 = fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_models/SSD_300_300_8Bit/out/mbox_conf_flatten_out_out.txt","w");
#endif
	if(fp1 == NULL)
	{
		std :: cout << "File not found - " << path << std :: endl;
		//return -1;
	}

#ifdef __SDSOC
	FILE *fp2 = fopen("int8_models/SSD_300_300_8Bit/out/mbox_conf_flatten_out_diff.txt","w");
#else
	FILE *fp2 = fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_models/SSD_300_300_8Bit/out/mbox_conf_flatten_out_diff.txt","w");
#endif
	if(fp2 == NULL)
	{
		std :: cout << "File not found - " << path << std :: endl;
		//return -1;
	}
#endif

	int inputsize=height*width*depth;//*batch_size;

	float float_val;

	for(int i = 0; i < inputsize; i++)
	{
		fscanf(fp, "%f ", &float_val);

#if 0
		fprintf(fp2, "ref=%f   inp=%f   diff=%f\n", float_val, input[2*i], (float_val-input[2*i]));
		fprintf(fp1, "%f\n", input[2*i]);
#endif

		//input[i] = float_val;
		for(int j=0;j<batch_size; j++)
		{
			input[2*i+j] = float_val;
		}
	}
	fclose(fp);

#if 0
	fclose(fp1);
	fclose(fp2);
#endif
}
// softmax_outfloatdatawrite1

//#define IO_DATA_TYPE char//char
//softmax_outfloatdatawrite3((IO_DATA_TYPE*)hwQueue[0][whichSoftmax].in_ptrs[0], nms_nboxes,nms_nclasses,1,output_ref_path1);//,0, conv0_output, kernel_output, output_file_path, 0);
void softmax_outfloatdatawrite3(IO_DATA_TYPE *input, int depth, int height, int width, const char *path, int batch_size, int inp_fbits)
{
	FILE *fp = fopen(path,"r");
	if(fp == NULL)
	{
		std :: cout << "File not found - " << path << std :: endl;
		//return -1;
	}

#if 0

#ifdef __SDSOC
	FILE *fp1 = fopen("int8_models/SSD_300_300_8Bit/out/mbox_conf_out_out.txt","w");
#else
	FILE *fp1 = fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_models/SSD_300_300_8Bit/out/mbox_conf_out_out.txt","w");
#endif
	if(fp1 == NULL)
	{
		std :: cout << "File not found - " << path << std :: endl;
		//return -1;
	}

#ifdef __SDSOC
	FILE *fp2 = fopen("int8_models/SSD_300_300_8Bit/out/mbox_conf_out_diff.txt","w");
#else
	FILE *fp2 = fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_models/SSD_300_300_8Bit/out/mbox_conf_out_diff.txt","w");
#endif
	if(fp2 == NULL)
	{
		std :: cout << "File not found - " << path << std :: endl;
		//return -1;
	}
#endif

	int inputsize=height*width*depth*batch_size;
	float float_val;
	float float_val1;

	for(int i = 0; i < inputsize; i+=batch_size)
	{
		//int j=0;
		//for(int j=0;j<batch_size; j++)
		{
			fscanf(fp, "%f ", &float_val);

#if 0
			IO_DATA_TYPE i_val1=input[i];
			float_val1 = (float)i_val1;
			float_val1 = float_val1/(1<<inp_fbits);

			fprintf(fp2, "ref=%f   inp=%f   diff=%f\n", float_val, float_val1, (float_val-float_val1));
			fprintf(fp1, "%f\n", float_val1);
#endif

			IO_DATA_TYPE i_val=(IO_DATA_TYPE)float_val;
			IO_DATA_TYPE fxval=ConvertToFP(float_val,i_val,inp_fbits);

			//for offline mode
			//fxval = (int8_t)(float_val);

			input[i] = fxval;
			input[i+1] = fxval;

			//input[i] = float_val;
			//input[i+1] = float_val;
		}
	}
	fclose(fp);

#if 0
	fclose(fp1);
	fclose(fp2);
#endif

}
// softmax_outfloatdatawrite1

#if 0
void conv_loadinput_split(xChangeLayer inLayer, char *inp_path)
{
	char path[500];
	char *base_path = inLayer.base_path;
	int *params		= (int*)inLayer.params;

	//sprintf(path, "%s/outputs/group0_block0_eltwise_out.txt", base_path);
	//sprintf(path, "%s/caffe_ref/pool1_3x3_s2_out.txt", base_path);
	//sprintf(path, "%s/outputs/group1_block0_preact_bn_relu_out.txt", base_path);
	//sprintf(path, "%s/outputs/pool0_out.txt", base_path);

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
#endif

void LoadInputData(xChangeLayer inLayer, char *lay_path)
{
	int *params = (int*)inLayer.params;
	//planes : 192 h = 28 w = 28 lay_id : 5 batchsize : 2 in_fbits : 4 scale : 14.311368

#if 0
	int planes	= 192;//params[];
	int height	= 28;//params[];
	int width	= 28;//params[];
	int layerid	= 5;//params[12];
	int batchsize	=	XBATCH_SIZE;
	int in_fbits	= 4;
	ap_uint<128> *ptr_1 = (ap_uint<128> *)inLayer.in_ptrs[0];
	ap_uint<128> *ptr_2	= (ap_uint<128> *)inLayer.in_ptrs[1];
	ap_uint<64> *ptr_l1	= (ap_uint<64> *)inLayer.in_ptrs[2];

	fprintf(stderr, "input file path : %s\n", lay_path);
	FILE *fp_in = fopen("/proj/sdxapps/users/maheshm/verif/20180425_offQ_gnet/data_incp3a1x1/pool2_3x3_s2_out.txt", "r");
	if(fp_in == NULL)
	{
		fprintf(stderr, "failed to open ref file\n");
		return;
	}
	float scale_input = 14.311368;
#endif

	//1024 h = 7 w = 7 lay_id : 61 batchsize : 2 in_fbits :0

	float scale_input = 0;

	//int planes	=  params[50];//1024;//params[];
	//int height	= params[43];//7;//params[];
	//int width	=  params[44];//7;//params[];

	int planes;
	int height;
	int width;

	if(inLayer.kernType == CONV)
	{
		planes	= params[5];
		height	= params[0];
		width	= params[1];
		int pool_split_cnt = params[110];
		if(pool_split_cnt>1)
		{
			planes = params[109];
		}
	}

	if(inLayer.kernType == POOL)
	{
		planes	= params[5];
		height	= params[0];
		width	= params[1];
	}

	if(inLayer.kernType == FC_LAYER)
	{
		planes	= params[1];
		height	= 1;//params[2];
		width	= 1;//params[3];
	}

	if(inLayer.kernType == NORM)
	{
		planes	= params[0];
		height	= params[1];
		width	= params[2];
	}

	if(inLayer.kernType == PERMUTE)
	{
		planes	= params[0];
		height	= params[1];
		width	= params[2];
	}

	int fbits_input_other = inLayer.qf_format.ip_fbits;//inLayer->pa;//convLayer_args[layer_id].in_fbits;  //TODO
	fprintf(stderr, "fbits_input_other:%d\n", fbits_input_other);

	std :: cout << "[CHECKS] (h, w, d, in_fbits) : " << height << " " << width << " " << planes << " " << fbits_input_other << std :: endl;


	int layerid	= 61;//params[12];
	int batchsize	=	XBATCH_SIZE;
	int in_fbits	= 0;
	ap_uint<128> *ptr_1 = (ap_uint<128> *)inLayer.in_ptrs[0];
	ap_uint<128> *ptr_2	= (ap_uint<128> *)inLayer.in_ptrs[1];
	ap_uint<64> *ptr_l1	= (ap_uint<64> *)inLayer.in_ptrs[2];

	fprintf(stderr, "input file path : %s\n", lay_path);
	//FILE *fp_in = fopen("/proj/sdxapps/users/nkpavan/VIVADO_HLS_WORKSPACE_1/20180410_pool_standalone/data/quant/inception_5b_output_out.txt", "r");
	FILE *fp_in = fopen(lay_path, "r");
	if(fp_in == NULL)
	{
		fprintf(stderr, "failed to open ref file\n");
		return;
	}

	fprintf(stderr, "\n\ninput : planes : %d h = %d w = %d lay_id : %d batchsize : %d in_fbits :%d \n", planes, height, width, layerid, batchsize, in_fbits );

	//Read Caffe Input file and load floating point data

	int pln_align;
	if(layerid!=0)
	{
		if(batchsize == 1)
		{
			pln_align = AlignSize(planes, 32);

		}
		else
		{
			pln_align = AlignSize(planes, 16);

		}
	}
	else
		pln_align = 4;

	float a=0;
	float *in_fl       = (float *)malloc(2*pln_align*height*width*sizeof(float));
	for(int p=0; p < pln_align; p++){
		for(int h=0; h<height; h++){
			for(int w=0; w<width; w++){

				fscanf(fp_in,"%f",&a);
				if(layerid==0 && p > 2)
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
				}
				else
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
				}
			}
		}
	}
	fclose(fp_in); // Floating point data loaded

	//DRAM data Arrangement
	int plane_loop_bound;

	if(batchsize == 1)
		plane_loop_bound = ((pln_align/32) * height * width);
	else
		plane_loop_bound = ((pln_align/16) * height * width);


	if(layerid != 0) // For Other layers
	{
		for(int i=0;  i < plane_loop_bound; i++)
		{

			ap_uint<128> a,b;

			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = (j)+32*(i/(height * width));
				else
					e2 = (j/2)+16*(i/(height * width));
				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;

				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];
				float in_by_sc = round(in1);///scale_input);//+0.5;

				a.range(bit+7,bit) = (ap_int<8>)((float)(in_by_sc));//in1 *(1<<in_fbits)));
			}
			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = 16+(j)+32*(i/(height * width));
				else
					e2 = 8+(j/2)+16*(i/(height * width));

				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;
				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];
				float in_by_sc = round(in1);///scale_input);//+0.5;

				b.range(bit+7,bit) = (ap_int<8>)((float)(in_by_sc));//in1 *(1<<in_fbits)));

			}
			ptr_1[i] = a;
			ptr_2[i] = b;

		}
	}

	else // For layer 1
	{
		for(int h=0; h <height; h++){
			for(int w=0; w <width; w++){
				ap_uint<64> a;
				for(int k=0, bit=0; bit<64; k++,bit+=8){
					float b = in_fl[((k/2)*height*width)+(h*width)+w];
					//float bs = b*(1<<in_fbits);
					float in_by_sc = round(b/scale_input);//+0.5;
					int fxd = in_by_sc;
					a.range(bit+7,bit) = fxd;
				}
				ptr_l1[(h*width) + w]=a;
			}
		}
	}
}

void LoadInputData_flmode(xChangeLayer inLayer, char *lay_path)
{
	int *params = (int*)inLayer.params;
	//planes : 192 h = 28 w = 28 lay_id : 5 batchsize : 2 in_fbits : 4 scale : 14.311368

	//1024 h = 7 w = 7 lay_id : 61 batchsize : 2 in_fbits :0

	float scale_input = 0;

	//int planes	=  params[50];//1024;//params[];
	//int height	= params[43];//7;//params[];
	//int width	=  params[44];//7;//params[];

	int planes;
	int height;
	int width;

	if(inLayer.kernType == CONV)
	{
		planes	= params[5];
		height	= params[0];
		width	= params[1];
	}

	if(inLayer.kernType == POOL)
	{
		planes	= params[5];
		height	= params[0];
		width	= params[1];
	}

	if(inLayer.kernType == FC_LAYER)
	{
		planes	= params[1];
		height	= 1;//params[2];
		width	= 1;//params[3];
	}

	if(inLayer.kernType == NORM)
	{
		planes	= params[0];
		height	= params[1];
		width	= params[2];
	}

	if(inLayer.kernType == PERMUTE)
	{
		planes	= params[0];
		height	= params[1];
		width	= params[2];
	}

	int fbits_input_other = inLayer.qf_format.ip_fbits;//inLayer->pa;//convLayer_args[layer_id].in_fbits;  //TODO
	fprintf(stderr, "fbits_input_other:%d\n", fbits_input_other);

	std :: cout << "[CHECKS] (h, w, d, in_fbits) : " << height << " " << width << " " << planes << " " << fbits_input_other << std :: endl;


	int layerid	= 61;//params[12];
	int batchsize	=	XBATCH_SIZE;
	int in_fbits	= 0;
	ap_uint<128> *ptr_1 = (ap_uint<128> *)inLayer.in_ptrs[0];
	ap_uint<128> *ptr_2	= (ap_uint<128> *)inLayer.in_ptrs[1];
	ap_uint<64> *ptr_l1	= (ap_uint<64> *)inLayer.in_ptrs[2];

	fprintf(stderr, "input file path : %s\n", lay_path);
	//FILE *fp_in = fopen("/proj/sdxapps/users/nkpavan/VIVADO_HLS_WORKSPACE_1/20180410_pool_standalone/data/quant/inception_5b_output_out.txt", "r");
	FILE *fp_in = fopen(lay_path, "r");
	if(fp_in == NULL)
	{
		fprintf(stderr, "failed to open ref file\n");
		return;
	}

	fprintf(stderr, "\n\ninput : planes : %d h = %d w = %d lay_id : %d batchsize : %d in_fbits :%d \n", planes, height, width, layerid, batchsize, in_fbits );

	//Read Caffe Input file and load floating point data

	int pln_align;
	if(layerid!=0)
	{
		if(batchsize == 1)
		{
			pln_align = AlignSize(planes, 32);

		}
		else
		{
			pln_align = AlignSize(planes, 16);

		}
	}
	else
		pln_align = 4;

	float a=0;
	float *in_fl       = (float *)malloc(2*pln_align*height*width*sizeof(float));
	for(int p=0; p < pln_align; p++){
		for(int h=0; h<height; h++){
			for(int w=0; w<width; w++){

				fscanf(fp_in,"%f",&a);
				if(layerid==0 && p > 2)
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
				}
				else
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
				}
			}
		}
	}
	fclose(fp_in); // Floating point data loaded

	//DRAM data Arrangement
	int plane_loop_bound;

	if(batchsize == 1)
		plane_loop_bound = ((pln_align/32) * height * width);
	else
		plane_loop_bound = ((pln_align/16) * height * width);


	if(layerid != 0) // For Other layers
	{
		for(int i=0;  i < plane_loop_bound; i++)
		{

			ap_uint<128> a,b;

			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = (j)+32*(i/(height * width));
				else
					e2 = (j/2)+16*(i/(height * width));
				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;

				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];
				float in_by_sc = round(in1);///scale_input);//+0.5;

				a.range(bit+7,bit) = (ap_int<8>)((float)(in_by_sc));//in1 *(1<<in_fbits)));
			}
			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = 16+(j)+32*(i/(height * width));
				else
					e2 = 8+(j/2)+16*(i/(height * width));

				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;
				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];
				float in_by_sc = round(in1);///scale_input);//+0.5;

				//b.range(bit+7,bit) = (ap_int<8>)((float)(in_by_sc));//in1 *(1<<in_fbits)));

				IO_DATA_TYPE ival = (IO_DATA_TYPE)(in1);
				IO_DATA_TYPE fxval = ConvertToFP( in1, ival, fbits_input_other);

				//a.range(bit+7,bit) = (ap_int<8>)((float)(in1 *(1<<in_fbits)));
				a.range(bit+7,bit) = (ap_int<8>)(fxval);//(in1);


			}
			ptr_1[i] = a;
			ptr_2[i] = b;

		}
	}

	else // For layer 1
	{
		for(int h=0; h <height; h++){
			for(int w=0; w <width; w++){
				ap_uint<64> a;
				for(int k=0, bit=0; bit<64; k++,bit+=8){
					float b = in_fl[((k/2)*height*width)+(h*width)+w];
					//float bs = b*(1<<in_fbits);
					//float in_by_sc = round(b/scale_input);//+0.5;
					//int fxd = in_by_sc;

					IO_DATA_TYPE ival = (IO_DATA_TYPE)(b);
					IO_DATA_TYPE fxd = ConvertToFP( b, ival, fbits_input_other);
					a.range(bit+7,bit) = fxd;
				}
				ptr_l1[(h*width) + w]=a;
			}
		}
	}
}


void LoadInputData_singleio(xChangeLayer inLayer, char *lay_path)
{


	int *params = (int*)inLayer.params;
	//planes : 192 h = 28 w = 28 lay_id : 5 batchsize : 2 in_fbits : 4 scale : 14.311368

#if 0
	int planes	= 192;//params[];
	int height	= 28;//params[];
	int width	= 28;//params[];
	int layerid	= 5;//params[12];
	int batchsize	=	XBATCH_SIZE;
	int in_fbits	= 4;
	ap_uint<128> *ptr_1 = (ap_uint<128> *)inLayer.in_ptrs[0];
	ap_uint<128> *ptr_2	= (ap_uint<128> *)inLayer.in_ptrs[1];
	ap_uint<64> *ptr_l1	= (ap_uint<64> *)inLayer.in_ptrs[2];

	fprintf(stderr, "input file path : %s\n", lay_path);
	FILE *fp_in = fopen("/proj/sdxapps/users/maheshm/verif/20180425_offQ_gnet/data_incp3a1x1/pool2_3x3_s2_out.txt", "r");
	if(fp_in == NULL)
	{
		fprintf(stderr, "failed to open ref file\n");
		return;
	}
	float scale_input = 14.311368;
#endif

	//1024 h = 7 w = 7 lay_id : 61 batchsize : 2 in_fbits :0

	float scale_input = 0;

	//int planes	=  params[109];//params[50]*params[110];//1024;//params[];
	//int planes	=  params[50];
	//int height	= params[43];//7;//params[];
	//int width	=  params[44];//7;//params[];

	int planes;
	int height;
	int width;

	if(inLayer.kernType == CONV)
	{
		planes	= params[5];
		height	= params[0];
		width	= params[1];
	}

	if(inLayer.kernType == POOL)
	{
		planes	= params[5];
		height	= params[0];
		width	= params[1];
	}

	if(inLayer.kernType == FC_LAYER)
	{
		planes	= params[1];
		height	= 1;//params[2];
		width	= 1;//params[3];
	}

	if(inLayer.kernType == NORM)
	{
		planes	= params[0];
		height	= params[1];
		width	= params[2];
	}

	if(inLayer.kernType == PERMUTE)
	{
		planes	= params[0];
		height	= params[1];
		width	= params[2];
	}

	int fbits_input_other = inLayer.qf_format.ip_fbits;//inLayer->pa;//convLayer_args[layer_id].in_fbits;  //TODO
	fprintf(stderr, "fbits_input_other:%d\n", fbits_input_other);

	std :: cout << "[CHECKS] (h, w, d, in_fbits) : " << height << " " << width << " " << planes << " " << fbits_input_other << std :: endl;

	int layerid	= 61;//params[12];
	int batchsize	=	XBATCH_SIZE;
	int in_fbits	= 0;
	ap_uint<128> *ptr_1 = (ap_uint<128> *)inLayer.in_ptrs[0];
	ap_uint<128> *ptr_2	= (ap_uint<128> *)inLayer.in_ptrs[1];
	ap_uint<64> *ptr_l1	= (ap_uint<64> *)inLayer.in_ptrs[2];

	fprintf(stderr, "input file path : %s\n", lay_path);
	//FILE *fp_in = fopen("/proj/sdxapps/users/nkpavan/VIVADO_HLS_WORKSPACE_1/20180410_pool_standalone/data/quant/inception_5b_output_out.txt", "r");
	FILE *fp_in = fopen(lay_path, "r");
	if(fp_in == NULL)
	{
		fprintf(stderr, "failed to open ref file\n");
		return;
	}

	fprintf(stderr, "\n\ninput : planes : %d h = %d w = %d lay_id : %d batchsize : %d in_fbits :%d \n", planes, height, width, layerid, batchsize, in_fbits );

	//Read Caffe Input file and load floating point data

	int pln_align;
	if(layerid!=0)
	{
		if(batchsize == 1)
		{
			pln_align = AlignSize(planes, 32);

		}
		else
		{
			pln_align = AlignSize(planes, 16);

		}
	}
	else
		pln_align = 4;


	float a=0;
	float *in_fl       = (float *)malloc(2*pln_align*height*width*sizeof(float));
	for(int p=0; p < pln_align; p++){
		for(int h=0; h<height; h++){
			for(int w=0; w<width; w++){

				fscanf(fp_in,"%f",&a);
				if(layerid==0 && p > 2)
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
				}
				else
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
				}
			}
		}
	}
	fclose(fp_in); // Floating point data loaded

	//DRAM data Arrangement
	int plane_loop_bound;

	if(batchsize == 1)
		plane_loop_bound = ((pln_align/16) * height * width);
	else
		plane_loop_bound = ((pln_align/8) * height * width);


	if(layerid != 0) // For Other layers
	{
		for(int i=0;  i < plane_loop_bound; i++)
		{

			//gmem_inputtype_layerx a,b;
			ap_uint<128> a, b;

			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = (j)+16*(i/(height * width));
				else
					e2 = (j/2)+8*(i/(height * width));
				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;

				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];


				//a.range(bit+7,bit) = (ap_int<8>)((float)(in1 *(1<<in_fbits)));
				a.range(bit+7,bit) = (ap_int<8>)(in1);

			}
			ptr_1[i] = a;

		}
	}

	else // For layer 1
	{
		for(int h=0; h <height; h++){
			for(int w=0; w <width; w++){
				ap_uint<64> a;
				for(int k=0, bit=0; bit<64; k++,bit+=8){
					float b = in_fl[((k/2)*height*width)+(h*width)+w];
					float bs = b*(1<<in_fbits);
					int fxd = bs;
					a.range(bit+7,bit) = fxd;
				}
				ptr_l1[(h*width) + w]=a;
			}
		}
	}

#if 0
	//Read Caffe Input file and load floating point data

	int pln_align;
	if(layerid!=0)
	{
		if(batchsize == 1)
		{
			pln_align = align(planes, 32);

		}
		else
		{
			pln_align = align(planes, 16);

		}
	}
	else
		pln_align = 4;

	float a=0;
	float *in_fl       = (float *)malloc(2*pln_align*height*width*sizeof(float));
	for(int p=0; p < pln_align; p++){
		for(int h=0; h<height; h++){
			for(int w=0; w<width; w++){

				fscanf(fp_in,"%f",&a);
				if(layerid==0 && p > 2)
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
				}
				else
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
				}
			}
		}
	}
	fclose(fp_in); // Floating point data loaded

	//DRAM data Arrangement
	int plane_loop_bound;

	if(batchsize == 1)
		plane_loop_bound = ((pln_align/32) * height * width);
	else
		plane_loop_bound = ((pln_align/16) * height * width);


	if(layerid != 0) // For Other layers
	{
		for(int i=0;  i < plane_loop_bound; i++)
		{

			gmem_inputtype_layerx a,b;

			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = (j)+32*(i/(height * width));
				else
					e2 = (j/2)+16*(i/(height * width));
				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;

				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];


				//a.range(bit+7,bit) = (ap_int<8>)((float)(in1 *(1<<in_fbits)));
				a.range(bit+7,bit) = (ap_int<8>)(in1);

			}
			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = 16+(j)+32*(i/(height * width));
				else
					e2 = 8+(j/2)+16*(i/(height * width));

				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;
				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];
				//b.range(bit+7,bit) = (ap_int<8>)((float)(in1 *(1<<in_fbits)));
				b.range(bit+7,bit) = (ap_int<8>)(in1);

			}
			ptr_1[i] = a;
			ptr_2[i] = b;

		}
	}

	else // For layer 1
	{
		for(int h=0; h <height; h++){
			for(int w=0; w <width; w++){
				ap_uint<64> a;
				for(int k=0, bit=0; bit<64; k++,bit+=8){
					float b = in_fl[((k/2)*height*width)+(h*width)+w];
					float bs = b*(1<<in_fbits);
					int fxd = bs;
					a.range(bit+7,bit) = fxd;
				}
				ptr_l1[(h*width) + w]=a;
			}
		}
	}
#endif
}

void LoadInputData_singleio_flmode(xChangeLayer inLayer, char *lay_path)
{
	int *params = (int*)inLayer.params;
	//planes : 192 h = 28 w = 28 lay_id : 5 batchsize : 2 in_fbits : 4 scale : 14.311368
	//1024 h = 7 w = 7 lay_id : 61 batchsize : 2 in_fbits :0

	float scale_input = 0;

	int planes;
	int height;
	int width;

	if(inLayer.kernType == CONV)
	{
		planes	= params[5];
		height	= params[0];
		width	= params[1];
	}

	if(inLayer.kernType == POOL)
	{
		planes	= params[5];
		height	= params[0];
		width	= params[1];
	}

	if(inLayer.kernType == FC_LAYER)
	{
		planes	= params[1];
		height	= 1;//params[2];
		width	= 1;//params[3];
	}

	if(inLayer.kernType == NORM)
	{
		planes	= params[0];
		height	= params[1];
		width	= params[2];
	}

	if(inLayer.kernType == PERMUTE)
	{
		planes	= params[0];
		height	= params[1];
		width	= params[2];
	}

	int fbits_input_other = inLayer.qf_format.ip_fbits;//inLayer->pa;//convLayer_args[layer_id].in_fbits;  //TODO
	fprintf(stderr, "fbits_input_other:%d\n", fbits_input_other);

	std :: cout << "[CHECKS] (h, w, d, in_fbits) : " << height << " " << width << " " << planes << " " << fbits_input_other << std :: endl;


	//int planes	=  params[109];//params[50]*params[110];//1024;//params[];
	//int planes	=  params[50];
	//int height	= params[43];//7;//params[];
	//int width	=  params[44];//7;//params[];

	int layerid	= 61;//params[12];
	int batchsize	=	XBATCH_SIZE;
	int in_fbits	= 0;
	ap_uint<128> *ptr_1 = (ap_uint<128> *)inLayer.in_ptrs[0];
	ap_uint<128> *ptr_2	= (ap_uint<128> *)inLayer.in_ptrs[1];
	ap_uint<64> *ptr_l1	= (ap_uint<64> *)inLayer.in_ptrs[2];

	fprintf(stderr, "input file path : %s\n", lay_path);
	//FILE *fp_in = fopen("/proj/sdxapps/users/nkpavan/VIVADO_HLS_WORKSPACE_1/20180410_pool_standalone/data/quant/inception_5b_output_out.txt", "r");
	FILE *fp_in = fopen(lay_path, "r");
	if(fp_in == NULL)
	{
		fprintf(stderr, "failed to open ref file\n");
		return;
	}

	fprintf(stderr, "\n\ninput : planes : %d h = %d w = %d lay_id : %d batchsize : %d in_fbits :%d \n", planes, height, width, layerid, batchsize, in_fbits );

	//Read Caffe Input file and load floating point data

	int pln_align;
	if(layerid!=0)
	{
		if(batchsize == 1)
		{
			pln_align = AlignSize(planes, 32);

		}
		else
		{
			pln_align = AlignSize(planes, 16);

		}
	}
	else
		pln_align = 4;


	float a=0;
	float *in_fl       = (float *)malloc(2*pln_align*height*width*sizeof(float));
	for(int p=0; p < pln_align; p++){
		for(int h=0; h<height; h++){
			for(int w=0; w<width; w++){

				fscanf(fp_in,"%f",&a);
				if(layerid==0 && p > 2)
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
				}
				else
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
				}
			}
		}
	}
	fclose(fp_in); // Floating point data loaded

	//DRAM data Arrangement
	int plane_loop_bound;

	if(batchsize == 1)
		plane_loop_bound = ((pln_align/16) * height * width);
	else
		plane_loop_bound = ((pln_align/8) * height * width);


	if(layerid != 0) // For Other layers
	{
		for(int i=0;  i < plane_loop_bound; i++)
		{

			//gmem_inputtype_layerx a,b;
			ap_uint<128> a, b;

			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = (j)+16*(i/(height * width));
				else
					e2 = (j/2)+8*(i/(height * width));
				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;

				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];

				IO_DATA_TYPE ival = (IO_DATA_TYPE)(in1);
				IO_DATA_TYPE fxval = ConvertToFP( in1, ival, fbits_input_other);

				//a.range(bit+7,bit) = (ap_int<8>)((float)(in1 *(1<<in_fbits)));
				a.range(bit+7,bit) = (ap_int<8>)(fxval);//(in1);

			}
			ptr_1[i] = a;

		}
	}

	else // For layer 1
	{
		for(int h=0; h <height; h++){
			for(int w=0; w <width; w++){
				ap_uint<64> a;
				for(int k=0, bit=0; bit<64; k++,bit+=8){
					float b = in_fl[((k/2)*height*width)+(h*width)+w];
					//float bs = b*(1<<in_fbits);
					//int fxd = bs;
					IO_DATA_TYPE ival = (IO_DATA_TYPE)(b);
					IO_DATA_TYPE fxd = ConvertToFP( b, ival, fbits_input_other);
					a.range(bit+7,bit) = fxd;
				}
				ptr_l1[(h*width) + w]=a;
			}
		}
	}

#if 0
	//Read Caffe Input file and load floating point data

	int pln_align;
	if(layerid!=0)
	{
		if(batchsize == 1)
		{
			pln_align = align(planes, 32);

		}
		else
		{
			pln_align = align(planes, 16);

		}
	}
	else
		pln_align = 4;

	float a=0;
	float *in_fl       = (float *)malloc(2*pln_align*height*width*sizeof(float));
	for(int p=0; p < pln_align; p++){
		for(int h=0; h<height; h++){
			for(int w=0; w<width; w++){

				fscanf(fp_in,"%f",&a);
				if(layerid==0 && p > 2)
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
				}
				else
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
				}
			}
		}
	}
	fclose(fp_in); // Floating point data loaded

	//DRAM data Arrangement
	int plane_loop_bound;

	if(batchsize == 1)
		plane_loop_bound = ((pln_align/32) * height * width);
	else
		plane_loop_bound = ((pln_align/16) * height * width);


	if(layerid != 0) // For Other layers
	{
		for(int i=0;  i < plane_loop_bound; i++)
		{

			gmem_inputtype_layerx a,b;

			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = (j)+32*(i/(height * width));
				else
					e2 = (j/2)+16*(i/(height * width));
				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;

				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];


				//a.range(bit+7,bit) = (ap_int<8>)((float)(in1 *(1<<in_fbits)));
				a.range(bit+7,bit) = (ap_int<8>)(in1);

			}
			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = 16+(j)+32*(i/(height * width));
				else
					e2 = 8+(j/2)+16*(i/(height * width));

				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;
				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];
				//b.range(bit+7,bit) = (ap_int<8>)((float)(in1 *(1<<in_fbits)));
				b.range(bit+7,bit) = (ap_int<8>)(in1);

			}
			ptr_1[i] = a;
			ptr_2[i] = b;

		}
	}

	else // For layer 1
	{
		for(int h=0; h <height; h++){
			for(int w=0; w <width; w++){
				ap_uint<64> a;
				for(int k=0, bit=0; bit<64; k++,bit+=8){
					float b = in_fl[((k/2)*height*width)+(h*width)+w];
					float bs = b*(1<<in_fbits);
					int fxd = bs;
					a.range(bit+7,bit) = fxd;
				}
				ptr_l1[(h*width) + w]=a;
			}
		}
	}
#endif
}


void conv_loadinput_split(xChangeLayer inLayer, char *inp_path)
{
	char path[500];
	char *base_path = inLayer.base_path;
	int *params		= (int*)inLayer.params;

	FILE *fp = fopen(inp_path,"r");
	fprintf(stderr,"input file = %s\n", inp_path);

	if(fp == NULL)
	{
		std :: cout << "[ERROR] can't open file - " << inp_path << std :: endl;
		//return -1;
	}


	int fbits_input_other = inLayer.qf_format.ip_fbits;//inLayer->pa;//convLayer_args[layer_id].in_fbits;  //TODO
	fprintf(stderr, "fbits_input_other:%d\n", fbits_input_other);

	//int buffer_split_flag = 1;  //TODO : Added values

	int in_depth;
	int in_height;
	int in_width;

	if(inLayer.kernType == CONV)
	{
		in_depth	= params[5];
		in_height	= params[0];
		in_width	= params[1];
	}

	if(inLayer.kernType == POOL)
	{
		in_depth	= params[5];
		in_height	= params[0];
		in_width	= params[1];
	}

	if(inLayer.kernType == FC_LAYER)
	{
		in_depth	= params[1];
		in_height	= 1;//params[2];
		in_width	= 1;//params[3];
	}

	if(inLayer.kernType == NORM)
	{
		in_depth	= params[0];
		in_height	= params[1];
		in_width	= params[2];
	}

	if(inLayer.kernType == PERMUTE)
	{
		in_depth	= params[0];
		in_height	= params[1];
		in_width	= params[2];
	}

	std :: cout << "[CHECKS] (h, w, d, in_fbits) : " << in_height << " " << in_width << " " << in_depth << " " << fbits_input_other << std :: endl;

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

	IO_DATA_TYPE* input_tb;
	IO_DATA_TYPE* input_tb1;

	if( (params[34] == OPCODE_BN) || (params[34] == OPCODE_ELTWISE) )
	{
		input_tb  = (IO_DATA_TYPE*)inLayer.in_ptrs[2];
		input_tb1 =  (IO_DATA_TYPE*)inLayer.in_ptrs[3];
	}
	else
	{
		input_tb  = (IO_DATA_TYPE*)inLayer.in_ptrs[0];
		input_tb1 =  (IO_DATA_TYPE*)inLayer.in_ptrs[1];
	}

	//input_tb  = (IO_DATA_TYPE*)inLayer.wts_ptrs[0];
	//input_tb1 =  (IO_DATA_TYPE*)inLayer.wts_ptrs[1];

	int idx1 = 0; int idx2 = 0;
	int plane16 = 0;
	for(int planes4 = 0; planes4 < in_depth; planes4 += (KER_PROC/XBATCH_SIZE), plane16++)
	{

		for(int i=0;i<(in_height);i++)
		{
			for(int j=0;j<in_width;j++)
			{
				for(int planes = 0; planes < (KER_PROC/XBATCH_SIZE); planes++)
				{
					//input_tb[j*4+i*IN_W+planes] = refinput[planes*(IN_W*IN_H)+j];
					//					int index_ref = (planes*(in_width*in_height)+j + i*in_width + ((in_width*in_height)*planes4));
					float float_val = refinput[planes*(in_width*in_height)+j + i*in_width + ((in_width*in_height)*planes4)];
					IO_DATA_TYPE ival = (IO_DATA_TYPE)( refinput[planes*(in_width*in_height)+j + i*in_width + ((in_width*in_height)*planes4)]);
					IO_DATA_TYPE fxval = ConvertToFP( refinput[planes*(in_width*in_height)+j + i*in_width + ((in_width*in_height)*planes4)], ival, fbits_input_other);
					//input_tb[j*4+i*IN_W+planes] = fxval;

					if(((plane16%2)==0) || (in_depth <= KER_PROC/XBATCH_SIZE))// || (buffer_split_flag==0))
					{
						input_tb[idx1++] = fxval;
						if(XBATCH_SIZE==2)
						{
							input_tb[idx1++] = fxval;
						}
						//fprintf(fpin, "%d ", input_tb[index]);
					}
					else
					{
						input_tb1[idx2++] = fxval;
						if(XBATCH_SIZE==2)
						{
							input_tb1[idx2++] = fxval;
						}
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

void LoadInputfcData(xChangeLayer inLayer, char *lay_path)
{
	int *params = (int*)inLayer.params;
	//planes : 192 h = 28 w = 28 lay_id : 5 batchsize : 2 in_fbits : 4 scale : 14.311368

	int planes	= 256;//1024;//params[4];//192;//params[];
	int height	= 6;//1;//params[3];//28;//params[];
	int width	= 6;//1;//params[2];//28;//params[];
	int layerid	= 5;//params[12];
	int batchsize	=	XBATCH_SIZE;
	int in_fbits	= 4;
	ap_uint<128> *ptr_1 = (ap_uint<128> *)inLayer.in_ptrs[0];
	ap_uint<128> *ptr_2	= (ap_uint<128> *)inLayer.in_ptrs[1];
	ap_uint<64> *ptr_l1	= (ap_uint<64> *)inLayer.in_ptrs[2];

	fprintf(stderr, "input file path : %s\n", lay_path);
	//FILE *fp_in = fopen("/proj/sdxapps/users/maheshm/verif/20180425_offQ_gnet/data_incp3a1x1/pool2_3x3_s2_out.txt", "r");
	FILE *fp_in = fopen(lay_path, "r");
	if(fp_in == NULL)
	{
		fprintf(stderr, "failed to open ref file\n");
		return;
	}
	float scale_input = 14.311368;

	//Read Caffe Input file and load floating point data

	int pln_align;
	if(layerid!=0)
	{
		if(batchsize == 1)
		{
			pln_align = AlignSize(planes, 32);

		}
		else
		{
			pln_align = AlignSize(planes, 16);

		}
	}
	else
		pln_align = 4;

	float a=0;
	float *in_fl       = (float *)malloc(2*pln_align*height*width*sizeof(float));
	for(int p=0; p < pln_align; p++){
		for(int h=0; h<height; h++){
			for(int w=0; w<width; w++){

				fscanf(fp_in,"%f",&a);
				if(layerid==0 && p > 2)
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
				}
				else
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
				}
			}
		}
	}
	fclose(fp_in); // Floating point data loaded

	//DRAM data Arrangement
	int plane_loop_bound;

	if(batchsize == 1)
		plane_loop_bound = ((pln_align/32) * height * width);
	else
		plane_loop_bound = ((pln_align/16) * height * width);


	if(layerid != 0) // For Other layers
	{
		for(int i=0;  i < plane_loop_bound; i++)
		{

			ap_uint<128> a,b;

			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = (j)+32*(i/(height * width));
				else
					e2 = (j/2)+16*(i/(height * width));
				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;

				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];
				float in_by_sc = round(in1);///scale_input);//+0.5;

				a.range(bit+7,bit) = (ap_int<8>)((float)(in_by_sc));//in1 *(1<<in_fbits)));
			}
			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = 16+(j)+32*(i/(height * width));
				else
					e2 = 8+(j/2)+16*(i/(height * width));

				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;
				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];
				float in_by_sc = round(in1);///scale_input);//+0.5;

				b.range(bit+7,bit) = (ap_int<8>)((float)(in_by_sc));//in1 *(1<<in_fbits)));

			}
			ptr_1[i] = a;
			ptr_2[i] = b;

		}
	}

	else // For layer 1
	{
		for(int h=0; h <height; h++){
			for(int w=0; w <width; w++){
				ap_uint<64> a;
				for(int k=0, bit=0; bit<64; k++,bit+=8){
					float b = in_fl[((k/2)*height*width)+(h*width)+w];
					//float bs = b*(1<<in_fbits);
					float in_by_sc = round(b/scale_input);//+0.5;
					int fxd = in_by_sc;
					a.range(bit+7,bit) = fxd;
				}
				ptr_l1[(h*width) + w]=a;
			}
		}
	}
}

void LoadInputNormData(xChangeLayer inLayer, char *lay_path)
{
	int *params = (int*)inLayer.params;
	//planes : 192 h = 28 w = 28 lay_id : 5 batchsize : 2 in_fbits : 4 scale : 14.311368

	int planes	= 512;//1024;//params[4];//192;//params[];
	int height	= 38;//1;//params[3];//28;//params[];
	int width	= 38;//1;//params[2];//28;//params[];
	int layerid	= 5;//params[12];
	int batchsize	=	XBATCH_SIZE;
	int in_fbits	= 4;
	ap_uint<128> *ptr_1 = (ap_uint<128> *)inLayer.in_ptrs[0];
	ap_uint<128> *ptr_2	= (ap_uint<128> *)inLayer.in_ptrs[1];
	ap_uint<64> *ptr_l1	= (ap_uint<64> *)inLayer.in_ptrs[2];

	fprintf(stderr, "input file path : %s\n", lay_path);
	//FILE *fp_in = fopen("/proj/sdxapps/users/maheshm/verif/20180425_offQ_gnet/data_incp3a1x1/pool2_3x3_s2_out.txt", "r");
	FILE *fp_in = fopen(lay_path, "r");
	if(fp_in == NULL)
	{
		fprintf(stderr, "failed to open ref file\n");
		return;
	}
	float scale_input = 14.311368;

	//Read Caffe Input file and load floating point data

	int pln_align;
	if(layerid!=0)
	{
		if(batchsize == 1)
		{
			pln_align = AlignSize(planes, 32);

		}
		else
		{
			pln_align = AlignSize(planes, 16);

		}
	}
	else
		pln_align = 4;

	float a=0;
	float *in_fl       = (float *)malloc(2*pln_align*height*width*sizeof(float));
	for(int p=0; p < pln_align; p++){
		for(int h=0; h<height; h++){
			for(int w=0; w<width; w++){

				fscanf(fp_in,"%f",&a);
				if(layerid==0 && p > 2)
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = 0;
				}
				else
				{
					in_fl[(0*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
					in_fl[(1*pln_align*height*width)+(p*height*width)+(h*width)+w] = a;
				}
			}
		}
	}
	fclose(fp_in); // Floating point data loaded

	//DRAM data Arrangement
	int plane_loop_bound;

	if(batchsize == 1)
		plane_loop_bound = ((pln_align/32) * height * width);
	else
		plane_loop_bound = ((pln_align/16) * height * width);


	if(layerid != 0) // For Other layers
	{
		for(int i=0;  i < plane_loop_bound; i++)
		{

			ap_uint<128> a,b;

			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = (j)+32*(i/(height * width));
				else
					e2 = (j/2)+16*(i/(height * width));
				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;

				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];
				float in_by_sc = round(in1);///scale_input);//+0.5;

				a.range(bit+7,bit) = (ap_int<8>)((float)(in_by_sc));//in1 *(1<<in_fbits)));
			}
			for(int j=0, bit = 0; j < (128/8); j++, bit+=8)
			{

				int e1,e2,e3,e4;
				e1 = 0;
				if(batchsize == 1)
					e2 = 16+(j)+32*(i/(height * width));
				else
					e2 = 8+(j/2)+16*(i/(height * width));

				e3 = (i%(height * width))/width;
				e4 = (i%(height * width))%width;
				float in1 = in_fl[(e1*planes*height*width)+(e2*height*width)+(e3*width)+e4];
				float in_by_sc = round(in1);///scale_input);//+0.5;

				b.range(bit+7,bit) = (ap_int<8>)((float)(in_by_sc));//in1 *(1<<in_fbits)));

			}
			ptr_1[i] = a;
			ptr_2[i] = b;

		}
	}

	else // For layer 1
	{
		for(int h=0; h <height; h++){
			for(int w=0; w <width; w++){
				ap_uint<64> a;
				for(int k=0, bit=0; bit<64; k++,bit+=8){
					float b = in_fl[((k/2)*height*width)+(h*width)+w];
					//float bs = b*(1<<in_fbits);
					float in_by_sc = round(b/scale_input);//+0.5;
					int fxd = in_by_sc;
					a.range(bit+7,bit) = fxd;
				}
				ptr_l1[(h*width) + w]=a;
			}
		}
	}
}


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
	float *hls_out	= (float*)inLayer.out_ptrs[0];
	//int *hls_int_out = (int *)inLayer.out_ptrs[1];
	int nms_finalboxcount = hls_out[0];

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

	hls_out+=1;
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
