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

#ifndef _XCHANGE_CONV_UTILS_HPP_
#define _XCHANGE_CONV_UTILS_HPP_

void straddleFactorCount( int *scalar_conv_args)
{

	int inDepth       = scalar_conv_args[5];
	int filter_size   = scalar_conv_args[7];
	int group_flag    = scalar_conv_args[11];

	int	numInpPlanes  = AlignSize(inDepth, 4);

	int fsz2 = filter_size*filter_size;

	int split;
	if(inDepth < 4)
	{
		split = 1;
	}else
	{
		split = group_flag + 1;
	}

	int inp_planes;
	if((group_flag) && (inDepth > 4))
		inp_planes = numInpPlanes/2;
	else
		inp_planes = numInpPlanes;

	bool exp1;
	bool exp2;
	int strad_fact=1;

	do{
		int comp_planes = inp_planes/ strad_fact;
		//expression based on inputbuff depth
		//## input_buff depth is fixed to 1024
		exp1 =  (((comp_planes/4)*fsz2) <=  (1024/2));
		//exp2 =  ((comp_planes*fsz2) <= KBUFF_DEPTH);
		strad_fact= strad_fact <<1;

	}while(!(exp1));

	int straddle_factor = strad_fact>>1;
	//printf("***straddle factor is %d\n", strad_fact>>1);

	int compute_planes =  numInpPlanes / (straddle_factor*split);

	scalar_conv_args[16] = compute_planes;
	scalar_conv_args[17] = straddle_factor;

}

void stgRowCount(int *scalar_conv_args)
{

	int ih         = scalar_conv_args[0];
	int iw         = scalar_conv_args[1];
	int oh         = scalar_conv_args[2];
	int ow         = scalar_conv_args[3];
	int outDepth   = scalar_conv_args[4];
	int inDepth    = scalar_conv_args[5];
	int fst        = scalar_conv_args[6];
	int fsz        = scalar_conv_args[7];
	int group_flag = scalar_conv_args[11];
	int dilation   = scalar_conv_args[29];
	int layerID    = scalar_conv_args[12];


	int outputplanes = AlignSize(outDepth, 8);
	int	inputplanes  = AlignSize(inDepth, 4);

	int iStgCnt, oStgCnt;

	int fsz_dilated = 1 + (fsz-1)*dilation;

	if(layerID==4)
		int stop=0;

	int ip,op;
	if(group_flag){
		ip = inputplanes/2;
		op = outputplanes/2;
	}else{
		ip = inputplanes;
		op = outputplanes;
	}

	int stgcount;

	int op_eff;
	if(op%32==0)
		op_eff = op/32;
	else
		op_eff = (op/32) + 1;

	int ip_eff;
	if(ip%32==0)
		ip_eff = ip/32;
	else
		ip_eff = (ip/32) + 1;

	int possible_output_rows = (OSTG_BUFF_DEPTH)/((op_eff)*ow);
	if(possible_output_rows == 0)
	{
		fprintf(stderr, "\n***** ostageBuf bram depth is not sufficient");
	}

	int possible_input_rows = ((possible_output_rows-1)*fst) + fsz_dilated;;

#if ROW2==1
	int inputRows_inBRAM = 2;
#elif ROW4==1
	int inputRows_inBRAM = 4;
#elif ROW8==1
	int inputRows_inBRAM = 8;
#endif

	if(layerID==0){
		if(possible_input_rows < inputRows_inBRAM*8)
			stgcount = possible_input_rows;
		else
			stgcount = inputRows_inBRAM*8;
	}else{
		/*
                              int expression = possible_input_rows*(ip_eff)*iw;
                              if(expression<ISTG_BUFF_DEPTH)
                                             stgcount = possible_input_rows;
                              else
                                             stgcount = (ISTG_BUFF_DEPTH)/((ip_eff)*iw);
		 */
		int expression; int temp;
		temp = possible_input_rows;
		do
		{
			if(possible_input_rows <= 0)
			{
				fprintf(stderr, "\n************unable to set istage_row count");
			}

			expression = temp*(ip_eff)*iw;
			temp = temp -fst;
		}while(expression>ISTG_BUFF_DEPTH);
		stgcount = temp + fst;
		//possible_input_rows possible_input_rows+fst;
	}

	if(stgcount > ih)
		stgcount = ih;

	//*stgRowCount = stgcount;
	scalar_conv_args[14] = stgcount;

	//            *ostgRowCount = (stgcount - fsz)/fst + 1 ;

	int ostgcount;

	if(stgcount == ih && possible_output_rows >= oh)
		ostgcount = oh;
	else
		ostgcount = (stgcount - fsz_dilated)/fst + 1 ;

	//*ostgRowCount = ostgcount;
	scalar_conv_args[15] = ostgcount;

	//fprintf(stderr,"\n+++++++++++++++***** ostageBuf possible rows = %d\n", ostgcount);

}

void nkpfCount(int *scalar_conv_args)
{

	int group_flag   = scalar_conv_args[11];
	int inputplanes  = scalar_conv_args[16];   //compute_planes
	int fsz          = scalar_conv_args[7];

	int outDepth     = scalar_conv_args[4];
	int outputplanes = AlignSize(outDepth, 8);
	int inDepth      = scalar_conv_args[5];
	//int inputplanes  = AlignSize(inDepth, 4);
	int con_outDepth = AlignSize(outDepth, 8);
	int con_inDepth = AlignSize(inDepth, 4);

	int ip = inputplanes;
	int op;
	if((group_flag) &&(outDepth > 8))
	{
		op = outputplanes/2;
	}else{
		op = outputplanes;
	}

	if(ip%4!=0)
		fprintf(stderr, "Number of input plane should be multiple of 4");
#if KER_PROC==1
	int max_nkpf = KBUFF_DEPTH/(((fsz*fsz))*(ip/4)*4);
#elif KER_PROC==2
	int max_nkpf = KBUFF_DEPTH/(((fsz*fsz))*(ip/4)*2);
#else
	int max_nkpf = KBUFF_DEPTH/(((fsz*fsz))*(ip/4));
#endif

	if(max_nkpf>15)
		max_nkpf=15;

	int rem;

#if KER_PROC==8
	rem = op%(max_nkpf*8);
#else
	rem = op%(max_nkpf*4);
#endif

	while(rem!=0){
		max_nkpf--;

#if KER_PROC==8
		rem = op%(max_nkpf*8);
#else
		rem = op%(max_nkpf*4);
#endif

		if(max_nkpf==0){
			fprintf(stderr, "Not able to set nkpf parameter.Check layer configuration");
			break;
		}
	}

	//*nkpf = max_nkpf;
	scalar_conv_args[13] = max_nkpf;

	int compute_planes =  scalar_conv_args[16];

	bool slk;
	if(((con_outDepth/(2*KER_PROC))*fsz*fsz*(con_inDepth/4) < KBUFF_DEPTH) && (con_inDepth == compute_planes) && (2*max_nkpf*KER_PROC >= con_outDepth))
		slk = 1;
	else
		slk = 0;

#ifdef __SDSOC
	scalar_conv_args[19] = slk;//0;//TODO:: slk  //HLS failing with slk
#else
	scalar_conv_args[19] = 0;//TODO:: slk  //HLS failing with slk
#endif

}

void loadConvWgtsTXT(CONV_WT_TYPE *weights_modified1, CONV_WT_TYPE *weights_modified2,
		char *path, int *scalar_conv_args, int fbits, int wt_bw, int wt_fl)
{

	int no_kernals_1   = scalar_conv_args[4];
	int inDepth        = scalar_conv_args[5];
	int filter_size    = scalar_conv_args[7];
	int group          = scalar_conv_args[11];
	int ratio_factor   = scalar_conv_args[17];

	int no_planes_in  = AlignSize(inDepth, 4);

	int split;
	if(inDepth < 4)
	{
		split = 1;
	}else
	{
		split = group + 1;
	}

	int no_planes_1 = inDepth/split;
	int remainder;
	int size_of_weights,no_of_filters;
	int no_kernals,no_planes;
	int padding_no=0;
	remainder=no_kernals_1%KER_PROC;

	if(remainder==0)
		no_kernals=no_kernals_1;
	else
		no_kernals=no_kernals_1+KER_PROC-remainder;

	if(no_kernals<4)
		no_kernals=4; //TODO: make it as a argument minimum number of kernels are processing


	remainder=no_planes_1%4;

	if(remainder==0)
		no_planes=no_planes_1;
	else
		no_planes=no_planes_1+4-remainder;
	/*
#else
	int remainder;
	int size_of_weights,no_of_filters;
	int no_kernals,no_planes;
	int padding_no=0;
	remainder=no_kernals_1%KER_PROC;

	if(remainder==0)
		no_kernals=no_kernals_1;
	else
		no_kernals=no_kernals_1+KER_PROC-remainder;
	if(no_kernals<4)
		no_kernals =4;

	remainder=no_planes_in%4;

	if(remainder==0)
		no_planes=no_planes_in;
	else
		no_planes=no_planes_in+4-remainder;
#endif
	 */

	int no_ele=filter_size*filter_size;
	int flag=0;

	/*
#if KER_PROC==4
	if((no_ele%2)!=0)
	{
		no_ele+=1;
		flag=1;
	}
#endif
	 */

	size_of_weights=((no_kernals)*(no_planes)*(no_ele)*split);

#if 1
	float *Weights=(float*)malloc(size_of_weights*sizeof(float));
	if(Weights == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for weights!\n");
	}

#if DATA_IN_BINARY
	FILE *fp_weights = fopen(path, "rb");
#else
	FILE *fp_weights = fopen(path, "r");
#endif

	if(fp_weights==NULL)
	{
		fprintf(stderr, "%s : File not found ******\n", path);
	}

	int size_of_weights1 = filter_size* filter_size * no_planes_1 * no_kernals_1;

#if DATA_IN_BINARY
	float *wts_buf = (float *)malloc(size_of_weights1*sizeof(float));
	fread(wts_buf, sizeof(float)*size_of_weights1, 1, fp_weights);
#endif

	int wts_idx = 0;

	float val;
	for(int i=0;i < no_kernals;i++)
	{
		for(int j=0;j < no_planes;j++)
		{

			for(int k=0; k < no_ele;k++)
			{

				if((k<( filter_size* filter_size)) && (j < no_planes_1) && (i < no_kernals_1)) //only for first layer
				{

#if DATA_IN_BINARY
					val = wts_buf[wts_idx++];
#else
					fscanf(fp_weights, "%f ", &val);
#endif
					Trim2FixedPoint(&val, wt_bw, wt_fl, ROUND_NEAREST, 1);

					Weights[(i*no_planes*no_ele)+j*no_ele+k]=val;
				}
				else
				{
					Weights[(i*no_planes*no_ele)+j*no_ele+k]=0.0f;

				}


			}
		}
	}

#if DATA_IN_BINARY
	free(wts_buf);
#endif

	fclose(fp_weights);

	float *weights_modified_float = (float*)malloc(size_of_weights*sizeof(float));

	if(weights_modified_float == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for tensor!\n");
	}

	int required_planes=no_planes/ratio_factor;
	int counter=0;
	int counter1=0;

	int idx_cnt = 0;
	int wt_counter=0;

	char kernal[200];
#if FILE_WRITE
	FILE *fp_kernal=fopen("kernal.txt","w");
	FILE *index1=fopen("Index.txt","w");
#endif
	int total=no_kernals/KER_PROC;
	int half=total/split;
	for(int groups=0;groups<split;groups++)
	{
		for(int l=0;l<ratio_factor;l++)
		{
			for(int kernal_count=half*groups;kernal_count<(half+half*groups);kernal_count++)
			{
				for(int h=0;h< required_planes/4;h++)
				{

					for(int i=0;i< no_ele;i++)// No of elements loop
					{
						for(int j=0;j < KER_PROC;j++)// 4 Kernals
						{

							for(int k=0;k < 4;k++) //4 planes
							{
								int temp=((kernal_count*KER_PROC+j)*no_ele*no_planes);
								//int temp1=(l*required_planes+k)*(no_ele);
								int temp1=(l*required_planes+h*4+k)*(no_ele);

								//int index=((kernal_count*KER_PROC+j)*no_ele*no_planes)+(h*no_ele*4)+(k*no_ele)+i+l*4*no_ele;
								int index=temp+temp1+i;

								float float_val = Weights[index];
								short ival = (short)(float_val);
								CONV_WT_TYPE fxval = ConvertToFP(float_val, ival, fbits);
								if(j<4)
								{
									weights_modified1[counter] = fxval;
									counter++;
								}
								else{
									weights_modified2[idx_cnt] = fxval;
									idx_cnt++;
								}
								weights_modified_float[wt_counter++] = float_val;


								//sprintf(kernal,"K%dP%dE%d",(kernal_count*KER_PROC+j),(l*required_planes+k),i);
#if FILE_WRITE
								sprintf(kernal,"K%dP%dE%d",kernal_count*KER_PROC+j,(l*required_planes+h*4+k),i);
								fprintf(fp_kernal,"%s ",kernal);
								fprintf(index1,"%d ",index);
#endif
								//counter++;
								counter1++;

								if(counter1==32)
								{
#if FILE_WRITE
									fprintf(fp_kernal,"\n");
									fprintf(index1,"\n");
#endif
									counter1=0;
								}


							}

						}

					}

				}

			}

		}

	}

#if FILE_WRITE
	int ele=no_planes*no_kernals*no_ele;
	counter1=0;
	FILE *fp = fopen("weights_modified_file.txt","w");
	for(int i=0;i<ele;i++)
	{

		fprintf(fp,"%f ",weights_modified_float[i]);
		counter1++;


		if(counter1==32)
			//if(counter1==no_planes*no_ele)
		{
			fprintf(fp,"\n");
			counter1=0;
		}
	}
	fclose(fp);
#endif

	/*fprintf(stderr,"\n----inside utils");
				for(int i =0;i<16;i++)
					fprintf(stderr,"weights[%d] = %d\t",i,weights_modified1[i]);
				for(int i =0;i<16;i++)
					fprintf(stderr,"weights[%d] = %d\t",i,weights_modified2[i]);

	 */
	free(Weights);
	free(weights_modified_float);
#endif

}

void loadConvBiasTXT(short *ptrRetArray, const char* path, int nElems, int layer_id, int fbits, int bs_bw, int bs_fl)
{
#if FILE_WRITE
	FILE *fpW = fopen("bias.txt", "w");
#endif

#if DATA_IN_BINARY
	FILE* fp = fopen(path, "rb");
#else
	FILE* fp = fopen(path, "r");
#endif

	if(fp == NULL)
	{
		fprintf(stderr, "\n** Error : Cannot open the file : %s **\n", path);
		exit(-1);
	}

	int n_size;

	//# Check for multiples of 8
	n_size = AlignSize(nElems, 8);

#if DATA_IN_BINARY
	float *bias_buf = (float *)malloc(n_size*sizeof(float));
	fread(bias_buf, sizeof(float)*nElems, 1, fp);
#endif

	float f_val;
	float f_val1;
	float max_err;
	for(int i = 0; i < n_size; i++)
	{
		if(i < nElems)
		{
#if DATA_IN_BINARY
			f_val = bias_buf[i];
#else
			fscanf(fp, "%f ", &f_val);
#endif
		}
		else
			f_val = 0;

		Trim2FixedPoint(&f_val, bs_bw, bs_fl, ROUND_NEAREST, 1);

		short ival = (short)f_val;
		short fxval = ConvertToFP(f_val, ival, fbits);
		ptrRetArray[i] = fxval;

#if FILE_WRITE
		fprintf(fpW, "%f\n", f_val);
#endif
	}
#if FILE_WRITE
	fclose(fpW);
#endif

#if DATA_IN_BINARY
	free(bias_buf);
#endif

	fclose(fp);
}
// loadBiasTXT

void load_mean_gamma_beta(short *ptrRetArray, const char *mean_path, const char *variance_path, const char *gamma_path, const char *beta_path, int nElems, float epsilon,
		int mean_fbits, int gamma_new_fbits, int beta_fbits)
{
#if FILE_WRITE
	FILE *fpW = fopen("mn_gamma_beta.txt", "w");
#endif

	float float_val;

#if DATA_IN_BINARY
	FILE* fp_mean = fopen(mean_path, "rb");
#else
	FILE* fp_mean = fopen(mean_path, "r");
#endif

	if(fp_mean == NULL)
	{
		fprintf(stderr, "\n** Error : Cannot open the file : %s **\n", mean_path);
		exit(-1);
	}

	float *mean_arr = (float *)malloc(nElems*sizeof(float));
#if DATA_IN_BINARY
	fread(mean_arr, sizeof(float)*nElems, 1, fp_mean);
#else
	for(int j=0;j<nElems;j++)
	{
		fscanf(fp_mean,"%f",&float_val);
		mean_arr[j] = float_val;
	}
#endif

#if DATA_IN_BINARY
	FILE* fp_variance = fopen(variance_path, "rb");
#else
	FILE* fp_variance = fopen(variance_path, "r");
#endif

	if(fp_variance == NULL)
	{
		fprintf(stderr, "\n** Error : Cannot open the file : %s **\n", variance_path);
		exit(-1);
	}

	float *variance_arr = (float *)malloc(nElems*sizeof(float));
#if DATA_IN_BINARY
	fread(variance_arr, sizeof(float)*nElems, 1, fp_variance);
#else
	for(int j=0;j<nElems;j++)
	{
		fscanf(fp_variance,"%f",&float_val);
		variance_arr[j] = float_val;
	}
#endif

#if DATA_IN_BINARY
	FILE* fp_gamma = fopen(gamma_path, "rb");
#else
	FILE* fp_gamma = fopen(gamma_path, "r");
#endif

	if(fp_gamma == NULL)
	{
		fprintf(stderr, "\n** Error : Cannot open the file : %s **\n", gamma_path);
		exit(-1);
	}

	float *gamma_arr = (float *)malloc(nElems*sizeof(float));
#if DATA_IN_BINARY
	fread(gamma_arr, sizeof(float)*nElems, 1, fp_gamma);
#else
	for(int j=0;j<nElems;j++)
	{
		fscanf(fp_gamma,"%f",&float_val);
		gamma_arr[j] = float_val;
	}
#endif

#if DATA_IN_BINARY
	FILE* fp_beta = fopen(beta_path, "rb");
#else
	FILE* fp_beta = fopen(beta_path, "r");
#endif

	if(fp_beta == NULL)
	{
		fprintf(stderr, "\n** Error : Cannot open the file : %s **\n", beta_path);
		exit(-1);
	}

	float *beta_arr = (float *)malloc(nElems*sizeof(float));
#if DATA_IN_BINARY
	fread(beta_arr, sizeof(float)*nElems, 1, fp_beta);
#else
	for(int j=0;j<nElems;j++)
	{
		fscanf(fp_beta,"%f",&float_val);
		beta_arr[j] = float_val;
	}
#endif

	float *gamma_new_arr = (float *)malloc(nElems*sizeof(float));
	float gamma_new_minval = 0.0f;
	float gamma_new_maxval = 0.0f;


	for(int j=0;j<nElems;j++)
	{
		float gamma_val = gamma_arr[j];
		float variance_val = variance_arr[j];

		variance_val = variance_val + epsilon;
		variance_val = sqrtf(variance_val);
		float f_val = gamma_val/variance_val;
		gamma_new_arr[j] = f_val;

		if(f_val < gamma_new_minval)
			gamma_new_minval = f_val;

		if(f_val > gamma_new_maxval)
			gamma_new_maxval = f_val;
	}


	int idx=0;
	for(int j=0;j<nElems;j++)
	{
		float f_val = mean_arr[j];
		short ival = (short)f_val;
		short fxval = ConvertToFP(f_val, ival, mean_fbits);
		ptrRetArray[idx] = fxval;
		idx++;
#if FILE_WRITE
		fprintf(fpW, "%f\n", f_val);
#endif
	}

	for(int j=0;j<nElems;j++)
	{
		float f_val = gamma_new_arr[j];
		short ival = (short)f_val;
		short fxval = ConvertToFP(f_val, ival, gamma_new_fbits);
		ptrRetArray[idx] = fxval;
		idx++;
#if FILE_WRITE
		fprintf(fpW, "%f\n", f_val);
#endif
	}

	for(int j=0;j<nElems;j++)
	{
		float f_val = beta_arr[j];
		short ival = (short)f_val;
		short fxval = ConvertToFP(f_val, ival, beta_fbits);
		ptrRetArray[idx] = fxval;
		idx++;
#if FILE_WRITE
		fprintf(fpW, "%f\n", f_val);
#endif
	}

	free(mean_arr);
	free(gamma_arr);
	free(variance_arr);
	free(gamma_new_arr);
	free(beta_arr);

#if FILE_WRITE
	fclose(fpW);
#endif

}

#endif //_XCHANGE_CONV_UTILS_HPP_
