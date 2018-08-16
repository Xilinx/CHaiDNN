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

void mem_offset_group(int in_h, int  in_w, int in_p,
		int out_h, int  out_w, int out_p,
		int filter_h, int  filter_w,
		bool group_en,
		int *input_group_offset_1st_fa0,
		int *input_group_offset_other_fa0,
		int *weights_group_offset_fa0,
		int *output_group_offset_fa0,
		int *bias_group_offset_fa0)
{

	int I_hwp_ffb0 = in_h * in_w * in_p;//input_desc.planes;

	int K_nkp_ffb0 = out_p*in_p;

	int K_nkpfsz2_ffb0 = K_nkp_ffb0*(filter_h *  filter_w);

	int O_hwp_ffb0 = out_h * out_w * out_p;

	if(group_en == 0)
	{
		*input_group_offset_1st_fa0 = 0;
		*input_group_offset_other_fa0 = 0;
		*weights_group_offset_fa0 = 0;
		*output_group_offset_fa0 = 0;
		*bias_group_offset_fa0 = 0;
	}
	else
	{
		*input_group_offset_1st_fa0 = (I_hwp_ffb0)>>XI_INPUTPACKCOUNT_LOG2;
		*input_group_offset_other_fa0 = (I_hwp_ffb0)>>(XI_INPUTPACKCOUNT2_LOG2+1);
		*output_group_offset_fa0 = (O_hwp_ffb0)>>(XI_OUTPUTPACKCOUNT_LOG2+1);
		*weights_group_offset_fa0 = (K_nkpfsz2_ffb0)>>(XI_WEIGHTPACKCOUNT_LOG2);
		*bias_group_offset_fa0 = (out_p)>>XI_BIASPACKCOUNT_LOG2;
	}

}

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

	int n_inbuff_depth = FEEDING_BUFF_DEPTH;
	
#if	DP_ENABLE
	n_inbuff_depth = FEEDING_BUFF_DEPTH - 1;
#else
	n_inbuff_depth = FEEDING_BUFF_DEPTH;
#endif

	do{
		int comp_planes = inp_planes/ strad_fact;
		//expression based on inputbuff depth
		//## input_buff depth is fixed to 1024
		exp1 =  (((comp_planes/4)*fsz2) <=  (n_inbuff_depth/2));
		//exp2 =  ((comp_planes*fsz2) <= XI_WEIGHTBUFF_DEPTH);
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
#if 0
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

	int possible_output_rows = (XI_OSTAGEBUFF_DEPTH)/((op_eff)*ow);
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
                              if(expression<XI_ISTAGEBUFF_DEPTH)
                                             stgcount = possible_input_rows;
                              else
                                             stgcount = (XI_ISTAGEBUFF_DEPTH)/((ip_eff)*iw);
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
		}while(expression>XI_ISTAGEBUFF_DEPTH);
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
#endif

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
	int conv3d_ih    	= scalar_conv_args[43];
	int conv3d_iw    	= scalar_conv_args[44];
	int conv3d_inDepth  = scalar_conv_args[50];
	int conv3d_fst    	= scalar_conv_args[53];
	int conv3d_fsz    	= scalar_conv_args[47];
	int conv3d_input_offset = scalar_conv_args[55];


	int outputplanes = AlignSize(outDepth, 8);
	int	inputplanes  = AlignSize(inDepth, 4);
	int	conv3d_inputplanes  = AlignSize(conv3d_inDepth, 4);
	int	conv3d_inputplanes_align32  = AlignSize(conv3d_inDepth, 32);

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
	if(op%(4*XI_OSTG_BUFFER_SET)==0)
		op_eff = op/(4*XI_OSTG_BUFFER_SET);
	else
		op_eff = (op/(4*XI_OSTG_BUFFER_SET)) + 1;

	int ip_eff;
	if(ip%32==0)
		ip_eff = ip/32;
	else
		ip_eff = (ip/32) + 1;

	int possible_output_rows = (XI_OSTAGEBUFF_DEPTH)/((op_eff)*ow);

	if( (scalar_conv_args[34] != OPCODE_POOL2CONV) && (scalar_conv_args[34] != OPCODE_AVRPOOL2CONV) )  //otherthan pool split
	{
		if(possible_output_rows == 0)
		{
			//fprintf(stderr, "\n***** ostageBuf bram depth is not sufficient");

			fprintf(stderr, "\n[INFOx] Design will not fit for output dimension %dx%dx%d\n", outputplanes, oh, ow);

			exit(-1);
		}
	}

	int possible_input_rows = ((possible_output_rows-1)*fst) + fsz_dilated;

	int conv3d_possible_input_rows = ((possible_input_rows-1)*conv3d_fst) + conv3d_fsz;
	if(conv3d_possible_input_rows > conv3d_ih)
		conv3d_possible_input_rows = conv3d_ih;

#if XI_ROW2==1
	int inputRows_inBRAM = 2;
#elif XI_ROW4==1
	int inputRows_inBRAM = 4;
#elif XI_ROW8==1
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
                              if(expression<XI_ISTAGEBUFF_DEPTH)
                                             stgcount = possible_input_rows;
                              else
                                             stgcount = (XI_ISTAGEBUFF_DEPTH)/((ip_eff)*iw);
		 */
		int expression; int temp;
		bool conv3d_ok;
		temp = possible_input_rows;
		do{
			do
			{
				if(possible_input_rows <= 0)
				{
					//fprintf(stderr, "\n************unable to set istage_row count");
				}

				expression = temp*(ip_eff)*iw;
				temp = temp -fst;
			}while(expression>(XI_ISTAGEBUFF_DEPTH - conv3d_input_offset));
			stgcount = temp + fst;

			int conv3d_stg_row_cnt_temp = ((stgcount-1)*conv3d_fst) + conv3d_fsz;
			int conv3d_bram_depth = conv3d_stg_row_cnt_temp * conv3d_iw *(conv3d_inputplanes_align32/32);

			if(scalar_conv_args[34] == OPCODE_3D_CONV)
			{
				conv3d_ok = conv3d_bram_depth<(XI_ISTAGEBUFF_DEPTH-conv3d_input_offset);
			}
			else
			{
				conv3d_ok = 1;
			}

			if(conv3d_ok == 0)
				temp = temp -fst;

		}while(conv3d_ok == 0);

		//possible_input_rows possible_input_rows+fst;
	}

	int conv3d_stg_row_cnt = ((stgcount-1)*conv3d_fst) + conv3d_fsz;
	if(conv3d_stg_row_cnt > conv3d_ih)
		conv3d_stg_row_cnt = conv3d_ih;
	scalar_conv_args[56] = conv3d_stg_row_cnt;

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

	if( (scalar_conv_args[34] != OPCODE_POOL2CONV) && (scalar_conv_args[34] != OPCODE_AVRPOOL2CONV) )  //otherthan pool split
	{
	if(stgcount < fsz)
	{
			fprintf(stderr, "\n[INFOx] Design will not fit for input dimension  %dx%dx%d with filter dimension %dx%d\n", inputplanes, ih, iw, fsz, fsz);
		if(ostgcount < 1)
		{
				fprintf(stderr, "\n[INFOx] Design will not fit for output dimension %dx%dx%d\n", outputplanes, oh, ow);
		}
			exit(-1);
		}
	}

	if((scalar_conv_args[34] == OPCODE_3D_CONV) || (scalar_conv_args[34] == OPCODE_POOL_CONV2CONV))//if(opcode == 23 || opcode == 19)
		scalar_conv_args[58] =  stgcount;
	else
		scalar_conv_args[58] =  ostgcount;

	//fprintf(stderr,"\n+++++++++++++++***** ostageBuf possible rows = %d\n", ostgcount);
}

void stgRowCount_poolsplit(int *scalar_conv_args)
{

	int ih         = scalar_conv_args[0];
	int iw         = scalar_conv_args[1];
	int oh         = scalar_conv_args[2];
	int ow         = scalar_conv_args[3];

	int fst        = scalar_conv_args[6];
	int fsz        = scalar_conv_args[7];
	int group_flag = scalar_conv_args[11];
	int dilation   = scalar_conv_args[29];
	int layerID    = scalar_conv_args[12];
	int conv3d_ih    	= scalar_conv_args[43];
	int conv3d_iw    	= scalar_conv_args[44];

	int conv3d_fst    	= scalar_conv_args[53];
	int conv3d_fsz    	= scalar_conv_args[47];
	int conv3d_input_offset = scalar_conv_args[55];

	int outDepth   = scalar_conv_args[114];
	int inDepth    = scalar_conv_args[113];
	int conv3d_inDepth  = scalar_conv_args[113];

	int outputplanes = AlignSize(outDepth, 8);
	int	inputplanes  = AlignSize(inDepth, 4);
	int	conv3d_inputplanes  = AlignSize(conv3d_inDepth, 4);
	int	conv3d_inputplanes_align32  = AlignSize(conv3d_inDepth, 32);

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
	if(op%(4*XI_OSTG_BUFFER_SET)==0)
		op_eff = op/(4*XI_OSTG_BUFFER_SET);
	else
		op_eff = (op/(4*XI_OSTG_BUFFER_SET)) + 1;

	int ip_eff;
	if(ip%32==0)
		ip_eff = ip/32;
	else
		ip_eff = (ip/32) + 1;

	int possible_output_rows = (XI_OSTAGEBUFF_DEPTH)/((op_eff)*ow);
	int possible_input_rows = ((possible_output_rows-1)*fst) + fsz_dilated;

	int conv3d_possible_input_rows = ((possible_input_rows-1)*conv3d_fst) + conv3d_fsz;
	if(conv3d_possible_input_rows > conv3d_ih)
		conv3d_possible_input_rows = conv3d_ih;

#if XI_ROW2==1
	int inputRows_inBRAM = 2;
#elif XI_ROW4==1
	int inputRows_inBRAM = 4;
#elif XI_ROW8==1
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
                              if(expression<XI_ISTAGEBUFF_DEPTH)
                                             stgcount = possible_input_rows;
                              else
                                             stgcount = (XI_ISTAGEBUFF_DEPTH)/((ip_eff)*iw);
		 */
		int expression; int temp;
		bool conv3d_ok;
		temp = possible_input_rows;
		do{
			do
			{
				if(possible_input_rows <= 0)
				{
					fprintf(stderr, "\n************unable to set istage_row count");
				}

				expression = temp*(ip_eff)*iw;
				temp = temp -fst;
			}while(expression>(XI_ISTAGEBUFF_DEPTH - conv3d_input_offset));
			stgcount = temp + fst;

			int conv3d_stg_row_cnt_temp = ((stgcount-1)*conv3d_fst) + conv3d_fsz;
			int conv3d_bram_depth = conv3d_stg_row_cnt_temp * conv3d_iw *(conv3d_inputplanes_align32/32);

			if(scalar_conv_args[34] == OPCODE_3D_CONV)
			{
				conv3d_ok = conv3d_bram_depth<(XI_ISTAGEBUFF_DEPTH-conv3d_input_offset);
			}
			else
			{
				conv3d_ok = 1;
			}

			if(conv3d_ok == 0)
				temp = temp -fst;

		}while(conv3d_ok == 0);

		//possible_input_rows possible_input_rows+fst;
	}

	int conv3d_stg_row_cnt = ((stgcount-1)*conv3d_fst) + conv3d_fsz;
	if(conv3d_stg_row_cnt > conv3d_ih)
		conv3d_stg_row_cnt = conv3d_ih;
	scalar_conv_args[115] = conv3d_stg_row_cnt;

	if(stgcount > ih)
		stgcount = ih;

	//*stgRowCount = stgcount;
	scalar_conv_args[117] = stgcount;

	//            *ostgRowCount = (stgcount - fsz)/fst + 1 ;

	int ostgcount;

	if(stgcount == ih && possible_output_rows >= oh)
		ostgcount = oh;
	else
		ostgcount = (stgcount - fsz_dilated)/fst + 1 ;

	//*ostgRowCount = ostgcount;
	scalar_conv_args[118] = ostgcount;

	if((scalar_conv_args[34] == OPCODE_3D_CONV) || (scalar_conv_args[34] == OPCODE_POOL_CONV2CONV))//if(opcode == 23 || opcode == 19)
		scalar_conv_args[116] =  stgcount;
	else
		scalar_conv_args[116] =  ostgcount;

	//fprintf(stderr,"\n+++++++++++++++***** ostageBuf possible rows = %d\n", ostgcount);
}

void nkpfCount(int *scalar_conv_args)
{

	int group_flag   = scalar_conv_args[11];
	int inputplanes  = scalar_conv_args[16];   //compute_planes
	int fsz          = scalar_conv_args[7];

	int outDepth     = scalar_conv_args[4];

	int outputplanes = AlignSize(outDepth, KER_PROC);
	int inDepth      = scalar_conv_args[5];
	int con_outDepth = AlignSize(outDepth, KER_PROC);
	int con_inDepth = AlignSize(inDepth, 4);

	int ip = inputplanes;
	int op;
	//TODO : groupflag for AlexNet
	if((group_flag) &&(outDepth > KER_PROC))
	{
		op = outputplanes/2;
	}else{
		op = outputplanes;
	}

	int n_kbuff_depth = XI_WEIGHTBUFF_DEPTH;
#if	DP_ENABLE
	n_kbuff_depth = XI_WEIGHTBUFF_DEPTH - 1;
#else
	n_kbuff_depth = XI_WEIGHTBUFF_DEPTH;
#endif

	if(ip%4!=0)
		fprintf(stderr, "Number of input plane should be multiple of 4");
#if KER_PROC==1
	int max_nkpf = n_kbuff_depth/(((fsz*fsz))*(ip/4)*4);
#elif KER_PROC==2
	int max_nkpf = n_kbuff_depth/(((fsz*fsz))*(ip/4)*2);
#else
	int max_nkpf = n_kbuff_depth/(((fsz*fsz))*(ip/4));
#endif

	if(max_nkpf>15)
		max_nkpf=15;

	int rem;

#if KER_PROC==16
	rem = op%(max_nkpf*16);
#elif KER_PROC==8
	rem = op%(max_nkpf*8);
#else
	rem = op%(max_nkpf*4);
#endif

	while(rem!=0){
		max_nkpf--;

#if KER_PROC==16
		rem = op%(max_nkpf*16);
#elif KER_PROC==8
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
	if(((con_outDepth/(2*KER_PROC))*fsz*fsz*(con_inDepth/4) < n_kbuff_depth) && (con_inDepth == compute_planes) && (2*max_nkpf*KER_PROC >= con_outDepth))
		slk = 1;
	else
		slk = 0;

#ifdef __SDSOC
	scalar_conv_args[19] = slk;//0;//TODO:: slk  //HLS failing with slk
#else
	scalar_conv_args[19] = 0;//TODO:: slk  //HLS failing with slk
#endif

}

void overflowPattern(int l_inout_precision_ffa0, int l_int6_en_out, ap_uint<24> &overflow_pattern)
{
	if(l_int6_en_out==0)//8 bit
	{
		for(int i=0; i<24; i++)
		{
			if(i < (l_inout_precision_ffa0+7))
				overflow_pattern[i]=0;
			else
				overflow_pattern[i]=1;
		}
	}
	else// 6bit
	{
		for(int i=0; i<24; i++)
		{
			if(i < (l_inout_precision_ffa0+5))
				overflow_pattern[i]=0;
			else
				overflow_pattern[i]=1;
		}
	}
}


void loadConvWgtsTXT(xChangeLayer *currentLayer, const float *wts_arr, const float *wts_th_arr, int *scalar_conv_args, int wt_bw, int wt_fl, int layer_id, char *layer_path, string quant_scheme)
{

	char *weights_modified1 = (char *)currentLayer->wts_ptrs[0];
	char *weights_modified2 = (char *)currentLayer->wts_ptrs[1];
#if (KER_PROC==16 || (PORT_BITWIDTH_64BIT==1 && KER_PROC==8))
	char *weights_modified3 = (char *)currentLayer->wts_ptrs[2];
	char *weights_modified4 = (char *)currentLayer->wts_ptrs[3];
#endif

	int fbits = currentLayer->qf_format.wt_fbits;

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

//	remainder=no_kernals_1%KER_PROC;
	remainder=no_kernals_1%CONV_IO_PACK_ELEMS;

	if(remainder==0)
		no_kernals=no_kernals_1;
	else
//		no_kernals=no_kernals_1+KER_PROC-remainder;
		no_kernals=no_kernals_1+CONV_IO_PACK_ELEMS-remainder;

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


#if FILE_WRITE
	int ele=no_planes*no_kernals*no_ele;
	int counter2=0;
	char path[100];
	int wts_idx2 = 0;
	//sprintf(path, "%s/data/%s_wts.txt", layer_name);
	FILE *fp = fopen(layer_path,"w");
	for(int i=0;i<ele;i++)
	{

		float val = wts_arr[wts_idx2++];
		Trim2FixedPoint(&val, wt_bw, wt_fl, ROUND_NEAREST, 1);

		fprintf(fp,"%0.8f ",val);
		counter2++;


		//if(counter2=32)
		if(counter2==no_planes*no_ele)
		{
			fprintf(fp,"\n");
			counter2=0;
		}
	}
	fclose(fp);
#endif

	/*
#if KER_PROC==4
	if((no_ele%2)!=0)
	{
		no_ele+=1;
		flag=1;
	}
#endif
	 */

#if DEBUG
	char path1[500];
	int wts_idx3 = 0;

	char *path = layer_path;

	char *ssc;
	int l = 0;
	ssc = strstr(path, "/");
	do{
		l = strlen(ssc) + 1;
		path = &path[strlen(path)-l+2];
		ssc = strstr(path, "/");
	}while(ssc);
	fprintf(stderr, "%s\n", path);

	//sprintf(path1, "/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_models/ResNet-50-Adv_8Bit_Fused/weightsTrimmed/%s.txt", path);
	sprintf(path1, "/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_models/ResNet-50-Adv_8Bit_withRounding/weightsTrimmed/%s", path);//group0_block0_convshortcut/group0_block0_convshortcut_wts.txt", layer_id);
	FILE *fp = fopen(path1,"r");
#endif

	size_of_weights=((no_kernals)*(no_planes)*(no_ele)*split);

	float *Weights=(float*)malloc(size_of_weights*sizeof(float));
	if(Weights == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for weights!\n");
	}

	int wts_idx = 0;

	float val;
	float th_param;

	for(int i = 0; i < no_kernals; i++)
	{
		if(quant_scheme.compare("Xilinx") == 0)
		{
			//# Read Threshold param for every output channel
			th_param = wts_th_arr[i];
		}

		for(int j=0;j < no_planes;j++)
		{

			for(int k=0; k < no_ele;k++)
			{

				if((k<( filter_size* filter_size)) && (j < no_planes_1) && (i < no_kernals_1)) //only for first layer
				{
					val = wts_arr[wts_idx++];

					if(quant_scheme.compare("Xilinx") == 0)
					{
						Trim2FixedPoint_Offline(&val, wt_bw, th_param);
					}
					else
					{
						Trim2FixedPoint(&val, wt_bw, wt_fl, ROUND_NEAREST, 1);
					}

#if DEBUG
					float val1;
					fscanf(fp, "%f ", &val1);
					//fscanf(fp, "%f ", &val);
					if(val1 != val)
						fprintf(stderr, "val1= %f  val = %f\n", val1, val);
#endif
				}
				else
				{
					val=0.0f;
				}

				Weights[(i*no_planes*no_ele)+j*no_ele+k]=val;

			}
		}
	}

#if DEBUG
	fclose(fp);
#endif

	float *weights_modified_float = (float*)malloc(size_of_weights*sizeof(float));

	if(weights_modified_float == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for tensor!\n");
	}

	int required_planes=no_planes/ratio_factor;
	int counter=0;
	int counter1=0;

	int wt_cnt1 = 0;
	int wt_cnt2 = 0;
	int wt_cnt3 = 0;
	int wt_cnt4 = 0;

	int wt_counter = 0;

	char kernal[200];
#if DEBUG
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

								CONV_WT_TYPE fxval;

								if(quant_scheme.compare("Xilinx") == 0)
								{
									fxval = (CONV_WT_TYPE)float_val;
								}
								else
									fxval = ConvertToFP(float_val, ival, fbits);

//#if KER_PROC==8
#if KER_PROC==8 && PORT_BITWIDTH_64BIT==0
								/*
								if(j < MAC_PIX_PROC)
								{
									weights_modified1[wt_cnt1] = fxval;
									wt_cnt1++;
								}
								else{
									weights_modified2[wt_cnt2] = fxval;
									wt_cnt2++;
								}
								 */
								if(j < KER_PROC/2)
								{
									weights_modified1[wt_cnt1] = fxval;
									wt_cnt1++;
								}
								else{
									weights_modified2[wt_cnt2] = fxval;
									wt_cnt2++;
								}
#else
								if(j < KER_PROC/4)
								{
									weights_modified1[wt_cnt1] = fxval;
									wt_cnt1++;
								}
								else if(j < KER_PROC/2)
								{
									weights_modified2[wt_cnt2] = fxval;
									wt_cnt2++;
								}
								else if(j < (KER_PROC*3)/4)
								{
									weights_modified3[wt_cnt3] = fxval;
									wt_cnt3++;
								}
								else
								{
									weights_modified4[wt_cnt4] = fxval;
									wt_cnt4++;
								}

#endif
								weights_modified_float[wt_counter++] = float_val;


								//sprintf(kernal,"K%dP%dE%d",(kernal_count*KER_PROC+j),(l*required_planes+k),i);
#if DEBUG
								sprintf(kernal,"K%dP%dE%d",kernal_count*KER_PROC+j,(l*required_planes+h*4+k),i);
								fprintf(fp_kernal,"%s ",kernal);
								fprintf(index1,"%d ",index);
#endif
								//counter++;
								counter1++;

								if(counter1==32)
								{
#if DEBUG
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

#if FILE_WRITE1
	int ele=no_planes*no_kernals*no_ele;
	counter1=0;
	char path[100];
	//sprintf(path, "%s/data/%s_wts.txt", layer_name);
	FILE *fp = fopen(layer_path,"w");
	for(int i=0;i<ele;i++)
	{
		fprintf(fp,"%0.8f ",weights_modified_float[i]);
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

}

void loadConvBiasTXT(HCONV_BIAS_TYPE *ptrRetArray, const float *bias_arr, const float *wts_th_arr, float th_out, float th_in, int nElems, int layer_id, int fbits, int in_bw, int op_bw, int op_fl,
		int wt_bw, int bs_bw, int bs_fl, char *layer_path, string quant_scheme )
{

#if FILE_WRITE
	char path[100];
	//sprintf(path, "/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_models/AlexNetFCN-8Bit/data/conv%d_bias.txt", layer_id);
	FILE *fpW = fopen("in_bias.txt", "w");
#endif

#if DEBUG
	char path1[100];
	sprintf(path1, "/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_models/Legacy-ResNet-50-8Bit_fused/weightsTrimmed/conv%d_bias.txt", layer_id);
	FILE *fp = fopen(path1,"r");
#endif


	int n_size;
	//# Check for multiples of 8
	int align_factor = ALIGN_FACTOR(HCONV_BIAS_PORT_WIDTH, HCONV_BIAS_DATA_WIDTH);
	n_size = AlignSize(nElems,  align_factor);
	float f_val;
	HCONV_BIAS_TYPE fxval;

	for(int i = 0; i < n_size; i++)
	{
		if(i < nElems)
		{
			f_val = bias_arr[i];
			//fprintf(stderr,"%f\n",f_val);

			if(quant_scheme.compare("Xilinx") == 0)
			{
				Trim2FixedPointBias_Offline(&f_val, bs_bw, th_out);
				fxval = (HCONV_BIAS_TYPE)f_val;
			}
			else
			{
				//Trim2FixedPoint(&f_val, bs_bw, bs_fl, ROUND_NEAREST, 1);
				Trim2FixedPoint(&f_val, op_bw, op_fl, ROUND_NEAREST, 1);

				HCONV_BIAS_TYPE ival = (HCONV_BIAS_TYPE)f_val;
				//fxval = ConvertToFP(f_val, ival, fbits);
				fxval = ConvertToFP(f_val, ival, bs_fl);
			}
#if DEBUG
			float val1;
			fscanf(fp, "%f ", &val1);
			//fscanf(fp, "%f ", &f_val);
			if(val1 != f_val)
				fprintf(stderr, "val1= %f  f_val = %f\n", val1, f_val);
#endif
		}
		else
		{
			//TODO : Anusha : fxval should be zero
			//f_val = 0;
			fxval = 0;
		}

		ptrRetArray[i] = fxval;

		//fprintf(stderr,"f_val1 = %f\n",f_val);
		//fprintf(stderr,"f_val2 = %f\n",f_val);
		//f_val = ((float)fxval)/(1 << fbits);
		//fprintf(stderr,"f_val3 = %f\n",f_val);
		//ptrRetArray[i] = 0;    //TODO:Anitha SSD-8Bit Debug

#if FILE_WRITE
		fprintf(fpW, "%f\n", f_val);
#endif
	}

#if DEBUG
	fclose(fp);
#endif

#if FILE_WRITE
	fclose(fpW);
#endif

	//# Load scaling factors in case of offline quant mode
	if(quant_scheme.compare("Xilinx") == 0)
	{
		//# shift is used 10.22 format
		int shift_factor = 22;

		//# Compute scale factor
		float sf_in	 = th_in  / (pow(2, in_bw-1) - 1);
		float sf_out = th_out / (pow(2, op_bw-1) - 1);

		//# After bias, load scale factors
		HCONV_BIAS_TYPE *InPtr = ptrRetArray + n_size;
		uint32_t *dstPtr = (uint32_t*)InPtr;

		uint32_t fx_val;

		for(int i = 0; i < n_size; i++)
		{
			float sf_param = wts_th_arr[i] / (pow(2, wt_bw-1) - 1);

			if(i < nElems)
			{
				//# TODO :: Rounding check
				f_val = round((sf_param * sf_in / sf_out) * (1 << shift_factor));
				if(quant_scheme.compare("Xilinx") == 0)
				{
					fx_val = (uint32_t)f_val;
				}
				else
				{
					fx_val = 1;
				}
			}
			else
			{
				fx_val = 0;
			}


			dstPtr[i] = fx_val;
		}
	}
	else
	{
		//# After bias, load scale factors
		HCONV_BIAS_TYPE *InPtr = ptrRetArray + n_size;
		uint32_t *dstPtr = (uint32_t*)InPtr;

		uint32_t fx_val;

		for(int i = 0; i < n_size; i++)
		{
			if(i < nElems)
			{
				fx_val = 1;

			}
			else
			{
				fx_val = 0;
			}

			dstPtr[i] = fx_val;
		}
	}

}
// loadBiasTXT


void loadScaleFactor(IO_DATA_TYPE *ptrRetArray, const float *wts_th_arr, float th_out, float th_in, int nElems, int layer_id, int bs_bw, char *layer_path)
{
}
// loadBiasTXT

template<typename Ptr_T>
void loadsf_avg_pool(Ptr_T *out_ptr0, Ptr_T *out_ptr1, float th_out, float th_in, int in_bw, int op_bw, int op_planes, int filter_size, int offline_quant_mode)
{

	//scalar_conv_args[57] and bias_fbits are same
	int bias_fbits = 16;//15;
	float sq_fsize = filter_size*filter_size;
	//fprintf(stderr, "Pool in planes : %d f_size : %d \n", op_planes, filter_size);

	float mul_factor = 1/sq_fsize;
	float a = mul_factor * (1 << bias_fbits);

	if(offline_quant_mode == 0)   //for dynamic fixed
	{
		a = 1;
	}

	int idx1 = 0, idx2 = 0;

	Ptr_T *dst_ptr1 = out_ptr0 + (op_planes/2);
	Ptr_T *dst_ptr2 = out_ptr1 + (op_planes/2);

	for(int i = 0; i < op_planes; i++)
	{
		int iper4 = i%8;

		if(iper4 < 4)
			dst_ptr1[idx1++] = (Ptr_T)(a);
		else
			dst_ptr2[idx2++] = (Ptr_T)(a);
	}

#if 0
	FILE *fp_out1 = fopen("sf1.txt", "w");
	if(fp_out1 == NULL)
	{
		fprintf(stderr, "failed to create file\n");
	}

	FILE *fp_out2 = fopen("sf2.txt", "w");
	if(fp_out2 == NULL)
	{
		fprintf(stderr, "failed to create file\n");
	}

	for(int i = 0; i < (op_planes); i++)
	{
		fprintf(fp_out1, "%d\n", out_ptr0[i]);
		fprintf(fp_out2, "%d\n", out_ptr1[i]);
	}
	fclose(fp_out1);
	fclose(fp_out2);
#endif

}


void load_mean_gamma_beta(HMEAN_TYPE *ptrRetArray, const float *mean_arr, const float *variance_arr, const float *gamma_arr, const float *beta_arr,
		int nElems, float epsilon, int mean_fbits, int gamma_new_fbits, int beta_fbits, char *layer_path)
{
#if FILE_WRITE
	char path1[100];
	sprintf(path1, "%s_mn_gamma_beta", layer_path);
	FILE *fpW = fopen(path1, "w");
#endif

	float float_val;
	float *gamma_new_arr = (float *)malloc(nElems*sizeof(float));
	float gamma_new_minval = 0.0f;
	float gamma_new_maxval = 0.0f;

	int	numInpPlanes  = AlignSize(nElems, 8);

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

	//fprintf(stderr, "gamma_new_minval=%f\n", gamma_new_minval);
	//fprintf(stderr, "gamma_new_maxval=%f\n", gamma_new_maxval);

	float mean_minval = 0.0f;
	float mean_maxval = 0.0f;

	int idx=0;
	for(int j=0;j<nElems;j++)
	{
		float f_val = mean_arr[j];
		HMEAN_TYPE ival = (HMEAN_TYPE)f_val;
		HMEAN_TYPE fxval = ConvertToFP(f_val, ival, mean_fbits);
		ptrRetArray[idx] = (HMEAN_TYPE)fxval;
		idx++;
#if FILE_WRITE
		fprintf(fpW, "%f\n", f_val);
#endif

		if(f_val < mean_minval)
			mean_minval = f_val;

		if(f_val > mean_maxval)
			mean_maxval = f_val;
	}
	//fprintf(stderr, "mean_minval=%f\n", mean_minval);
	//fprintf(stderr, "mean_maxval=%f\n", mean_maxval);

	for(int j=nElems;j<numInpPlanes;j++)
	{
		ptrRetArray[idx] = 0;
		idx++;
	}

	for(int j=0;j<nElems;j++)
	{
		float f_val = gamma_new_arr[j];
		Trim2FixedPoint(&f_val, 16, gamma_new_fbits, ROUND_NEAREST, 1);
		HMEAN_TYPE ival = (HMEAN_TYPE)f_val;
		HMEAN_TYPE fxval = ConvertToFP(f_val, ival, gamma_new_fbits);
		ptrRetArray[idx] = (HMEAN_TYPE)fxval;
		idx++;
#if FILE_WRITE
		fprintf(fpW, "%f\n", f_val);
#endif
	}

	for(int j=nElems;j<numInpPlanes;j++)
	{
		ptrRetArray[idx] = 0;
		idx++;
	}

	float beta_minval = 0.0f;
	float beta_maxval = 0.0f;

	for(int j=0;j<nElems;j++)
	{
		float f_val = beta_arr[j];
		HMEAN_TYPE ival = (HMEAN_TYPE)f_val;
		HMEAN_TYPE fxval = ConvertToFP(f_val, ival, beta_fbits);
		ptrRetArray[idx] = (HMEAN_TYPE)fxval;
		idx++;
#if FILE_WRITE
		fprintf(fpW, "%f\n", f_val);
#endif
		if(f_val < beta_minval)
			beta_minval = f_val;

		if(f_val > beta_maxval)
			beta_maxval = f_val;
	}
	//fprintf(stderr, "beta_minval=%f\n", beta_minval);
	//fprintf(stderr, "beta_maxval=%f\n", beta_maxval);


	for(int j=nElems;j<numInpPlanes;j++)
	{
		ptrRetArray[idx] = 0;
		idx++;
	}

	free(gamma_new_arr);

#if FILE_WRITE
	fclose(fpW);
#endif

}

// loadBiasTXT*/
void load3dSeperableConvWgts(char *ptrRetArray1, char *ptrRetArray2, const float *wts_arr, int *scalar_conv_args, int wt_bw, int wt_fl, char *layer_path)
{
#if FILE_WRITE
	FILE *fpW = fopen("wgts.txt", "w");
#endif

	int no_kernals_1   = scalar_conv_args[51];//*2;
	int filter_size    = scalar_conv_args[48];

	int no_planes_in  = AlignSize(no_kernals_1, CONV3D_WEIGHT_PACK_ELEMS);

	int size_of_weights,no_of_filters;
	int no_kernals,no_planes;

	no_kernals=no_kernals_1;
	no_planes = 1;

	int no_ele=filter_size*filter_size;

	int size_of_weights1 = filter_size * filter_size * no_planes * no_kernals_1;

	float float_val;
	int wts_idx = 0;
	int ptr_idx1 = 0;
	int ptr_idx2 = 0;

	for(int i=0;i<no_planes_in/CONV3D_WEIGHT_PACK_ELEMS;i++)
	{

		for(int j=0;j<no_ele;j++)
		{
			for(int k=0;k<CONV3D_WEIGHT_PACK_ELEMS;k++)
			{
				wts_idx = (i*CONV3D_WEIGHT_PACK_ELEMS+k)*no_ele+j;

				if((i*CONV3D_WEIGHT_PACK_ELEMS+k) < no_kernals_1)
				{
					float_val = wts_arr[wts_idx];
				}
				else
				{
					float_val = 0;
				}

				Trim2FixedPoint(&float_val, wt_bw, wt_fl, ROUND_NEAREST, 1);

				short ival = (short)(float_val);
				CONV_WT_TYPE fxval = ConvertToFP(float_val, ival, wt_fl);

				if((i%2)==0)  //first 8 planes
				{
					ptrRetArray1[ptr_idx1++] = fxval;
				}
				else   //next 8 planes
				{
					ptrRetArray2[ptr_idx2++] = fxval;
				}
			}
		}
	}

}
// loadSeperableConvWgtsTXT


void load3dSeperableConvBias(IO_DATA_TYPE *ptrRetArray1, IO_DATA_TYPE *ptrRetArray2, const float *bias_arr, int nElems, int layer_id, int bs_bw, int bs_fl, char *layer_path)
{

#if FILE_WRITE
	char path[100];
	//sprintf(path, "/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_models/AlexNetFCN-8Bit/data/conv%d_bias.txt", layer_id);
	FILE *fpW = fopen(layer_path, "w");
#endif

#if DEBUG
	char path1[100];
	sprintf(path1, "/proj/sdxapps/refdes/API_MIGRATE/sd_card/int8_models/Legacy-ResNet-50-8Bit_fused/weightsTrimmed/conv%d_bias.txt", layer_id);
	FILE *fp = fopen(path1,"r");
#endif


	int n_size;
	//# Check for multiples of 8
	n_size = AlignSize(nElems,  CONV3D_WEIGHT_PACK_ELEMS);
	float f_val;
	int cnt1=0, cnt2=0;
	int cnt_16=0;
	for(int i = 0; i < n_size; i++)
	{
		if(i < nElems)
		{
			f_val = bias_arr[i];
			//fprintf(stderr,"%f\n",f_val);
			Trim2FixedPoint(&f_val, bs_bw, bs_fl, ROUND_NEAREST, 1);
#if DEBUG
			float val1;
			fscanf(fp, "%f ", &val1);
			//fscanf(fp, "%f ", &f_val);
			if(val1 != f_val)
				fprintf(stderr, "val1= %f  f_val = %f\n", val1, f_val);
#endif
		}
		else
			f_val = 0;

		Trim2FixedPoint(&f_val, bs_bw, bs_fl, ROUND_NEAREST, 1);

		//fprintf(stderr,"f_val1 = %f\n",f_val);

		//fprintf(stderr,"f_val2 = %f\n",f_val);
		IO_DATA_TYPE ival = (IO_DATA_TYPE)f_val;
		IO_DATA_TYPE fxval = ConvertToFP(f_val, ival, bs_fl);

		//f_val = ((float)fxval)/(1 << fbits);
		//fprintf(stderr,"f_val3 = %f\n",f_val);
		if(cnt_16 < CONV3D_WEIGHT_PACK_ELEMS)
		{
			ptrRetArray1[cnt1++] = fxval;
		}
		else
		{
			ptrRetArray2[cnt2++] = fxval;
		}

		cnt_16++;
		if(cnt_16 == (CONV3D_WEIGHT_PACK_ELEMS*2))
			cnt_16=0;

		//ptrRetArray[i] = 0;    //TODO:Anitha SSD-8Bit Debug

#if FILE_WRITE
		fprintf(fpW, "%f\n", f_val);
#endif
	}

#if DEBUG
	fclose(fp);
#endif

#if FILE_WRITE
	fclose(fpW);
#endif


}
// load3dseperableConvBiasTXT

#if 0
struct ConvolutionParam
{
	int input_height;
	int input_width;
	int output_height;
	int output_width;
	int opcode;
	int ip_bw;
	int ip_fbits;
	int op_bw;
	int op_fbits;
	int wt_fbits;

	int input_height_3d;
	int input_width_3d;

	// fields
	int N;                              // Input Feature Maps, it doesn't comes from user, we have to compute from input blob
	int M;                              // Output Feature Maps
	int filter_h, filter_w;           // Filter size - filter_h x filter_w
	int stride_h, stride_w;           // Stride
	int pad_h, pad_w;                 // Padding
	int dilation;                       // dilation factor
	int reluflag;                       // ReLU enabled/disabled
	int extra_reluflag;
	int group;                        //
	int has_bias;                     // If this convolution has bias_term or not
	vector<string> weightsPath;           // All weight file names are stored in this.
	vector<string> biasPath;              // All bias file paths are stored in this
	vector< vector<float> > weights;
	vector< vector<float> > bias;
	vector< vector<int> > weightsDim;
	vector< vector<int> > biasDim;
	int has_software_fuse;
	// lrn_params
	int lrn_lsize;
	float lrn_alpha;
	float lrn_beta;
	float lrn_k;
	LRNType lrn_type;

	// batchnorm params
	float batchnorm_eps;
	vector<string> batchnorm_meanPath;
	vector<string> batchnorm_variancePath;
	vector< vector<int> > batchnorm_meanDim;
	vector< vector<int> > batchnorm_varianceDim;
	vector<float> batchnorm_mean;
	vector<float> batchnorm_variance;

	// scale params
	vector<string> scale_gammaPath;
	vector<string> scale_betaPath;
	vector< vector<int> > scale_gammaDim;
	vector< vector<int> > scale_betaDim;
	vector<float> scale_gamma;
	vector<float> scale_beta;

	// 3d Conv params
	int N_3d;                              // Input Feature Maps, it doesn't comes from user, we have to compute from input blob
	int M_3d;                              // Output Feature Maps
	int filter_h_3d, filter_w_3d;           // Filter size - filter_h x filter_w
	int stride_h_3d, stride_w_3d;           // Stride
	int pad_h_3d, pad_w_3d;                 // Padding
	int dilation_3d;                       // dilation factor
	int reluflag_3d;                       // ReLU enabled/disabled
	int group_3d;                        //
	int has_bias_3d;                     // If this convolution has bias_term or not
	int ip_bw_3d;
	int ip_fl_3d;
	int op_bw_3d;
	int op_fl_3d;
	int wt_bw_3d;
	int wt_fl_3d;
	vector<string> weightsPath_3d;           // All weight file names are stored in this.
	vector<string> biasPath_3d;              // All bias file paths are stored in this
	vector< vector<float> > weights_3d;
	vector< vector<float> > bias_3d;
	vector< vector<int> > weightsDim_3d;
	vector< vector<int> > biasDim_3d;

	char *wt_ptr0;
	char *wt_ptr1;
	IO_DATA_TYPE *bs_ptr0;
	IO_DATA_TYPE *bs_ptr1;
};

void generic_convInit(ConvolutionParam *conv_params, int *scalar_conv_args, int layer_id, file_paths *const_data_path, int &io_8bit_flag)
{

#if	EN_DEBUG_INIT_PRINT
	cout<<"convInit Start: "<<endl;
#endif

	scalar_conv_args[0]  = conv_params->input_height;
	scalar_conv_args[1]  = conv_params->input_width;
	scalar_conv_args[2]  = conv_params->output_height;
	scalar_conv_args[3]  = conv_params->output_width;
	scalar_conv_args[4]  = conv_params->M;             //output planes
	scalar_conv_args[5]  = conv_params->N;             //input planes
	scalar_conv_args[6]  = conv_params->stride_h;      //stride_h
	scalar_conv_args[7]  = conv_params->filter_h;      //filter_h
	scalar_conv_args[8]  = conv_params->filter_w;      //filter_w
	scalar_conv_args[9]  = conv_params->pad_h;         //pad_h
	scalar_conv_args[10] = conv_params->reluflag;      //relu
	scalar_conv_args[11] = conv_params->group-1;       //default group is 1
	scalar_conv_args[12] = layer_id;                   //layer_id
	scalar_conv_args[29] = conv_params->dilation;      //dilation
	scalar_conv_args[34] = conv_params->opcode;         //Opcode

	if(scalar_conv_args[34] == OPCODE_BN)
	{
		scalar_conv_args[10] = conv_params->extra_reluflag;
	}

	if( (scalar_conv_args[34] == OPCODE_BN) || (scalar_conv_args[34] == OPCODE_ELTWISE) )
	{
		scalar_conv_args[6] = 1;
		scalar_conv_args[7] = 1;
		scalar_conv_args[8] = 1;
	}

	if(scalar_conv_args[34] == OPCODE_3D_CONV)
	{
		scalar_conv_args[43] = scalar_conv_args[0];  //input height
		scalar_conv_args[44] = scalar_conv_args[1];  //input width

		scalar_conv_args[0]  = conv_params->input_height_3d;
		scalar_conv_args[1]  = conv_params->input_width_3d;

		scalar_conv_args[45] = scalar_conv_args[0];  //output height
		scalar_conv_args[46] = scalar_conv_args[1];  //output width

		scalar_conv_args[47] = conv_params->filter_h_3d;
		scalar_conv_args[48] = conv_params->filter_w_3d;
		scalar_conv_args[49] = conv_params->filter_h_3d*conv_params->filter_w_3d;//-fsz2;   //???
		scalar_conv_args[50] = conv_params->N_3d;    //input plane
		scalar_conv_args[51] = conv_params->M_3d;    //output plane
		scalar_conv_args[52] = conv_params->pad_h_3d;
		scalar_conv_args[53] = conv_params->stride_h_3d;
		scalar_conv_args[54] = conv_params->reluflag_3d;
		scalar_conv_args[55] = 0;                    //input offset
		//scalar_conv_args[55] = ((conv_params->pad_h_3d/conv_params->stride_h_3d)*scalar_conv_args[44]) + 1;  //((3d_pad/3d_stride)*3d_ip_w) + 1
		int conv_3d_req_rows = (conv_params->filter_h_3d-conv_params->stride_h_3d-conv_params->pad_h_3d);
		if(conv_3d_req_rows < 0)
		{
			conv_3d_req_rows = 0;
		}
		scalar_conv_args[55] = (conv_3d_req_rows*scalar_conv_args[44]) + conv_params->pad_h_3d;  //((3d_pad/3d_stride)*3d_ip_w) + 1

		//scalar_conv_args[56] = 0;                    //istg_row_cnt

		//int io_precision = currentLayer->qf_format.ip_fbits+currentLayer->qf_format.wt_fbits-currentLayer->qf_format.op_fbits;
		int io_precision = conv_params->ip_fl_3d+conv_params->wt_fl_3d-conv_params->op_fl_3d;
		scalar_conv_args[57] = io_precision;

		scalar_conv_args[42] = 1<< (io_precision - 1);   //rounding

		scalar_conv_args[59] = scalar_conv_args[43]*scalar_conv_args[44];                    //inp_h*inp_w

		//Loading Weights
		char *wt_ptr0 = (char *)conv_params->wt_ptr0;//xtra_ptrs[0];
		char *wt_ptr1 = (char *)conv_params->wt_ptr1;//xtra_ptrs[1];
		//currentLayer->in_ptrs[3] = wt_ptr0;
		//currentLayer->in_ptrs[4] = wt_ptr1;

		const float *weight_vec     = (const float *)&(conv_params->weights_3d[0][0]);
		//loadSeperableConvWgtsTXT(wt_ptr0, weight_vec, scalar_conv_args, currentLayer->qf_format.wt_fbits);
		load3dSeperableConvWgts(wt_ptr0, wt_ptr1, weight_vec, scalar_conv_args, conv_params->wt_bw_3d, conv_params->wt_fl_3d, const_data_path->wgt_path);

		//Loading Bias
		//# 3D-sep Conv bias
		IO_DATA_TYPE *bs_ptr0 = (IO_DATA_TYPE *)conv_params->bs_ptr0;//currentLayer->xtra_ptrs[2];
		IO_DATA_TYPE *bs_ptr1 = (IO_DATA_TYPE *)conv_params->bs_ptr1;//currentLayer->xtra_ptrs[3];
		//currentLayer->out_ptrs[2] = bs_ptr0;
		//currentLayer->out_ptrs[3] = bs_ptr1;

		const float *bias_vec     = (const float *)&(conv_params->bias_3d[0][0]);
		//loadSeperableConvWgtsTXT(wt_ptr0, weight_vec, scalar_conv_args, currentLayer->qf_format.wt_fbits);
		load3dSeperableConvBias(bs_ptr0, bs_ptr1, bias_vec, scalar_conv_args[51], layer_id, conv_params->op_bw_3d, conv_params->op_fl_3d, const_data_path->bias_path);
	}
	else
	{
		//Default 3D conv params
		scalar_conv_args[43] = scalar_conv_args[0];  	//input height
		scalar_conv_args[44] = scalar_conv_args[1];  	//input width
		scalar_conv_args[45] = scalar_conv_args[2];  	//output height
		scalar_conv_args[46] = scalar_conv_args[3];  	//output width
		//scalar_conv_args[47] = 1;  						//filter height
		scalar_conv_args[47] = 1+conv_params->pad_h;
		scalar_conv_args[48] = 1;					    //filter width
		scalar_conv_args[49] = 1;                       //fsz2
		scalar_conv_args[50] = conv_params->N;    		//input plane
		scalar_conv_args[51] = conv_params->M;    		//output plane

		scalar_conv_args[52] = conv_params->pad_h;
		scalar_conv_args[53] = 1;                       //stride_h_3d

		//# For normal Conv - Loading the extra out ptrs with dummy values
		//# This is to avoid HW exception
		//currentLayer->out_ptrs[2] = currentLayer->out_ptrs[0];
		//currentLayer->out_ptrs[3] = currentLayer->out_ptrs[1];


		//scalar_conv_args[56] = scalar_conv_args[14];    //conv3d_stg_row_cnt
		scalar_conv_args[59] = scalar_conv_args[0]*scalar_conv_args[1];                    //inp_h*inp_w
	}


	stgRowCount(scalar_conv_args);  //update 56-conv3d_stg_row_cnt, 14-istg_row_cnt, 15-ostgRowCount params

	if(scalar_conv_args[34] != OPCODE_3D_CONV)
	{
		scalar_conv_args[56] = scalar_conv_args[14];    //conv3d_stg_row_cnt
	}

	straddleFactorCount(scalar_conv_args);  //update 16-compute_planes, 17-straddle_factor

	nkpfCount(scalar_conv_args);  //19-slk


	int group_flag = scalar_conv_args[11];

	int io_precision = conv_params->ip_fbits+conv_params->wt_fbits-conv_params->op_fbits;

	scalar_conv_args[20] = io_precision;
	scalar_conv_args[21] = 0;

	//# Mean Values
	scalar_conv_args[22] = 0;
	scalar_conv_args[23] = 0;
	scalar_conv_args[24] = 0;
	scalar_conv_args[25] = 0;


	scalar_conv_args[18] = 0;              //out_offset
	scalar_conv_args[26] = 0;              //mean_sub_flag
	scalar_conv_args[27] = 0;              //bias_offset
	scalar_conv_args[28] = 0;              //weight_offset
	scalar_conv_args[30] = group_flag;     //group_flag for the next group iteration
	//scalar_conv_args[31] = 0;            //0

	//LRN parameters
	scalar_conv_args[31] = 1;              //read_from_ddr_en
	scalar_conv_args[32] = 1;              //write_from_ddr_en


	float float_val = 1.0f;
	short ival = (short)(float_val);
	int fbits = 2;
	short fxval = ConvertToFP(float_val, ival, fbits);
	scalar_conv_args[33] = fxval;              //norm_k = 1.0f in Q14.2 format


	//In LRN_SoS Mul_in = alpha/localSize, In LRN_PnS Mul_in = 1.0f in Q14.2
	float alpha   = conv_params->lrn_alpha;
	int localSize = conv_params->lrn_lsize;
	int mul_in = 0;

	if(scalar_conv_args[34] == 6)  //Sos
	{
		mul_in = alpha/localSize;
	}
	if(scalar_conv_args[34] == 9)  //LRN_PnS
	{
		mul_in = fxval;            //1.0f in Q14.2 format
	}
	scalar_conv_args[35] = mul_in;

	scalar_conv_args[36] = conv_params->ip_fbits;//6;//2;  //Norm_prec is assume to be 2
	//Norm_prec2 = 14 + Fbits_in + Fbits_out
	scalar_conv_args[37] = 14+conv_params->ip_fbits+conv_params->op_fbits;

	scalar_conv_args[20] = io_precision;

	//# int6 input flag
	//# TODO: Fix this
	if(conv_params->ip_bw == 6)
		scalar_conv_args[39] = 1;
	else
		scalar_conv_args[39] = 0;

	//# int6 output flag
	//# TODO: Fix this
	if(conv_params->op_bw == 6)
		scalar_conv_args[40] = 1;
	else
		scalar_conv_args[40] = 0;


	ap_uint<24> overflow_pattern;
	//# overflowPattern
	overflowPattern(io_precision, scalar_conv_args[40], overflow_pattern);
	scalar_conv_args[41] = overflow_pattern;

	scalar_conv_args[41] = 1 << (io_precision - 1);   // conv rounding //TODO CHECKING Functionality with ROUND: ANITHA
	//scalar_conv_args[43] = scalar_conv_args[41] - 1;
	//scalar_conv_args[44] = l_rounding_bn - 1;


#if EN_DEBUG_INIT_PRINT
	for(int i=0;i<MAX_PARAM_SIZE;i++)
		fprintf(stderr,"\t%d",(int)scalar_conv_args[i]);
	//	fprintf(stderr,"\n--------------in_height:plane = %d:%d \tout_height:plane = %d:%d\t in_stgcnt = %d \tout_stgcnt = %d ",l_input_height_ffa0 ,l_input_planes_ffa0, l_output_height_ffa0, l_output_planes_ffa0,l_istg_row_cnt_ffa0, l_ostg_row_cnt_ffa0);
#endif

	currentLayer->kernType = CONV;

	char *base_path = currentLayer->base_path;

	if( (scalar_conv_args[34] == OPCODE_CONV) || (scalar_conv_args[34] == OPCODE_FUSE_BN_CONV) || (scalar_conv_args[34] == OPCODE_CRELU) || (scalar_conv_args[34] == OPCODE_3D_CONV))
	{
		//Loading Weights
		int wt_bw = xlayer_seq->hw_ops->wt_bw;
		int wt_fl = xlayer_seq->hw_ops->wt_fl;

		const float *weights_vec = (const float *)&(conv_params->weights[0][0]);
		loadConvWgtsTXT(currentLayer, weights_vec, scalar_conv_args, wt_bw, wt_fl, layer_id, const_data_path->wgt_path);

		//Loading Bias
		IO_DATA_TYPE *bias_ptr = (IO_DATA_TYPE *)currentLayer->bias_ptr;
		int bs_bw = xlayer_seq->hw_ops->op_bw;
		int bs_fl = xlayer_seq->hw_ops->op_fl;

		//int bs_bw = xlayer_seq->hw_ops->wt_bw;
		//int bs_fl = xlayer_seq->hw_ops->wt_fl;

		const float *bias_vec = (const float *)&(conv_params->bias[0][0]);
		loadConvBiasTXT(bias_ptr, bias_vec, conv_params->M, layer_id, currentLayer->qf_format.bs_fbits, bs_bw, bs_fl, const_data_path->bias_path);

	}

	if( (scalar_conv_args[34] == OPCODE_BN) || (scalar_conv_args[34] == OPCODE_FUSE_BN_CONV) )
	{
#if 0
		if(scalar_conv_args[34] == OPCODE_FUSE_BN_CONV)
		{
			scalar_conv_args[34] = OPCODE_BN;
			scalar_conv_args[10] = 0;
		}
#endif

		scalar_conv_args[35] = 1<<currentLayer->qf_format.ip_fbits;

		HMEAN_TYPE *bias_ptr = (HMEAN_TYPE *)currentLayer->xtra_ptrs[0];//in_ptrs[3];//bias_ptr;
		currentLayer->in_ptrs[4] = bias_ptr;
		float epsilon =  xlayer_seq->hw_ops->conv_params->batchnorm_eps;
		int nElems  = scalar_conv_args[5];   //inp planes
		int ip_fbits = currentLayer->qf_format.ip_fbits;
		int op_fbits = currentLayer->qf_format.op_fbits;

		int mean_fbits = xlayer_seq->hw_ops->bn_mean_fl;
		int variance_fbits = xlayer_seq->hw_ops->bn_variance_fl;

		int gamma_fbits = xlayer_seq->hw_ops->scale_gamma_fl;
		int beta_fbits = xlayer_seq->hw_ops->scale_beta_fl;
		int gamma_by_std_fbits = xlayer_seq->hw_ops->scale_gamma_by_std_fl;

		int norm_prec = ip_fbits+gamma_by_std_fbits-op_fbits;
		int norm_prec_3 = ip_fbits+gamma_by_std_fbits-beta_fbits;
		scalar_conv_args[36] = norm_prec;
		scalar_conv_args[37] = 0;
		scalar_conv_args[38] = norm_prec_3;

		//# overflowPattern for batch norm
		overflowPattern(norm_prec, scalar_conv_args[40], overflow_pattern);
		scalar_conv_args[42] = overflow_pattern;    //TODO:FOR LRN we need to update



		mean_fbits = ip_fbits;


		gamma_fbits = xlayer_seq->hw_ops->scale_gamma_fl + 8;
		beta_fbits = xlayer_seq->hw_ops->scale_beta_fl+8;
		gamma_by_std_fbits = xlayer_seq->hw_ops->scale_gamma_by_std_fl+8;

		norm_prec = ip_fbits+8+gamma_by_std_fbits-op_fbits;
		norm_prec_3 = ip_fbits+8+gamma_by_std_fbits-beta_fbits;
		scalar_conv_args[36] = norm_prec;
		scalar_conv_args[37] = 0;
		scalar_conv_args[38] = norm_prec_3;

		//# overflowPattern for batch norm
		//overflowPattern(norm_prec, scalar_conv_args[40], overflow_pattern);
		//scalar_conv_args[42] = overflow_pattern;    //TODO:FOR LRN we need to update

		mean_fbits = ip_fbits + 8;

		const float *mean_vec     = (const float *)&(conv_params->batchnorm_mean[0]);
		const float *variance_vec = (const float *)&(conv_params->batchnorm_variance[0]);
		const float *gamma_vec    = (const float *)&(conv_params->scale_gamma[0]);
		const float *beta_vec     = (const float *)&(conv_params->scale_beta[0]);

		load_mean_gamma_beta(bias_ptr, mean_vec, variance_vec, gamma_vec, beta_vec,
				nElems, epsilon, mean_fbits, gamma_by_std_fbits, beta_fbits, const_data_path->wgt_path);

	}

	if(scalar_conv_args[34] == OPCODE_ELTWISE)
	{
		scalar_conv_args[35] = 0; //mul_in;
		scalar_conv_args[36] = 8; //norm_prec
		scalar_conv_args[37] = 8; //norm_prec_2
		scalar_conv_args[38] = 0; //norm_prec_3
		scalar_conv_args[42] = 0;
	}

	int inDepth    = scalar_conv_args[5];
	int outDepth   = scalar_conv_args[4];
	if((group_flag) && (inDepth > 4))
	{
		scalar_conv_args[5] = scalar_conv_args[5]/2;
	}

	if((group_flag) && (outDepth > 8))
	{
		scalar_conv_args[4] = scalar_conv_args[4]/2;
	}

	scalar_conv_args[11] = 0;              //making group as zero for the first group


	scalar_conv_args[58] = currentLayer->en_batch_size_one; //BATCH_SIZE_ONE_ENABLE


	//TODO:Extra Relu flag

#if 1
	///*************************************Scalar arguments from 60 to 94 for DSP optimization *************************************

	int pixProc;
	int input_height = scalar_conv_args[0];  	//input height
	int input_width  = scalar_conv_args[1];  	//input width
	int output_height = scalar_conv_args[2];  	//output height
	int output_width  = scalar_conv_args[3];  	//output width
	int filter_height = conv_params->filter_h;  //filter_h
	int filter_width  = conv_params->filter_w;  //filter_w
	int dilation_factor = scalar_conv_args[29]; //dilation
	int stride        = scalar_conv_args[6];    //stride_h

	if((scalar_conv_args[39] == 1) || (scalar_conv_args[58]==1))
		pixProc = XI_PIX_PROC;
	else
		pixProc = XI_PIX_PROC/2;


	scalar_conv_args[60] = AlignSize(scalar_conv_args[5], 4);  // input_planes_align4
	scalar_conv_args[61] = AlignSize(scalar_conv_args[16], 4); // compute_planes_align4
	scalar_conv_args[62] = AlignSize(scalar_conv_args[4], 16); // output_planes_align16
	scalar_conv_args[63] = AlignSize(scalar_conv_args[5], 16); // input_planes_align16
	scalar_conv_args[64] = input_height * input_width;
	scalar_conv_args[65] = output_height * output_width;
	scalar_conv_args[66] = filter_height * filter_width;
	scalar_conv_args[67] = (dilation_factor * (filter_height - 1)) + 1;//filter height for dilated conv
	scalar_conv_args[68] = (dilation_factor * (filter_width - 1)) + 1; //filter width for dilated conv

	scalar_conv_args[70] = scalar_conv_args[15] * scalar_conv_args[3]; // ostg_row_count x output_width
	scalar_conv_args[71] = scalar_conv_args[68] / stride;              //dilated_filter_width / stride
	scalar_conv_args[72] = scalar_conv_args[68] % stride;              //dilated_filter_width % stride
	scalar_conv_args[73] = pixProc / output_width;
	scalar_conv_args[74] = pixProc % output_width;
	scalar_conv_args[75] = (pixProc/2) / output_width;
	scalar_conv_args[76] = (pixProc/2) % output_width;
	scalar_conv_args[77] = filter_height * filter_width * (scalar_conv_args[61]/4); //compute_loop_count



	int input_group_offset_1st, input_group_offset_other, weights_group_offset, output_group_offset, bias_group_offset;
	bool group_enable = scalar_conv_args[11];

	mem_offset_group(input_height, input_width, scalar_conv_args[63], output_height, output_width, scalar_conv_args[62],
			filter_height, filter_width, group_enable,
			&input_group_offset_1st, &input_group_offset_other, &weights_group_offset, &output_group_offset, &bias_group_offset);

	scalar_conv_args[78] = input_group_offset_1st;
	scalar_conv_args[79] = input_group_offset_other;
	scalar_conv_args[80] = weights_group_offset;
	scalar_conv_args[81] = output_group_offset;
	scalar_conv_args[82] = bias_group_offset;

	int alignInputPlane;
	if(XBATCH_SIZE==1)   //batch size 1
	{
		alignInputPlane  = AlignSize(scalar_conv_args[5], 32);
	}
	else
	{
		alignInputPlane  = AlignSize(scalar_conv_args[5], 16);
	}

	int alignOutputPlane;
	if(XBATCH_SIZE==1)   //batch size 1
	{
		alignOutputPlane  = AlignSize(scalar_conv_args[4], 32);
	}
	else
	{
		alignOutputPlane  = AlignSize(scalar_conv_args[4], 16);
	}

	if(scalar_conv_args[34] == OPCODE_3D_CONV)
	{
		scalar_conv_args[69] = scalar_conv_args[56] * scalar_conv_args[44]; // 3d_istg_row_count x input_width
		scalar_conv_args[83] = scalar_conv_args[14]*scalar_conv_args[53]; //3d_ostage_row_count * 3d_stride
		if(XBATCH_SIZE==1)   //batch size 1
		{
			alignInputPlane  = AlignSize(scalar_conv_args[50], 32);
		}
		else
		{
			alignInputPlane  = AlignSize(scalar_conv_args[50], 16);
		}
		scalar_conv_args[84] = scalar_conv_args[43]*scalar_conv_args[44]*(alignInputPlane/16); //3d_input_height * 3d_input_width * 3d_input_plane_align16

	}
	else
	{
		scalar_conv_args[69] = scalar_conv_args[14] * scalar_conv_args[1]; // istg_row_count x input_width
		scalar_conv_args[83] = scalar_conv_args[15]*stride; //ostage_row_count * stride
		scalar_conv_args[84] = scalar_conv_args[64]*(alignInputPlane/16); //input_height * input_width * input_plane_align16
	}

	scalar_conv_args[85] = scalar_conv_args[65]*(alignOutputPlane/16); //output_height * output_width * output_plane_align16
	scalar_conv_args[86] = (ROW8*8 + ROW4*4 + ROW2*2)*input_width;               //Layer1: input_rows_per_bram * input_width
	scalar_conv_args[87] = output_height * output_width * (scalar_conv_args[62]/16);  //crelu_out_offset


	if(output_height % scalar_conv_args[15] == 0)
		scalar_conv_args[88] = scalar_conv_args[15];//ostageRow_cnt last iteration
	else
		scalar_conv_args[88] = (output_height%scalar_conv_args[15]);//ostageRow_cnt last iteration
	scalar_conv_args[89] = scalar_conv_args[88]*scalar_conv_args[3]; //ostgRow_last_iteration x width

	int total_pixel_fc0 = scalar_conv_args[15]*output_width;
	int pix_per_kp = pixProc;
	int pcmf_off1_fc0 = (total_pixel_fc0/pix_per_kp) +1;
	int pcmf_m1=(scalar_conv_args[17]-1);
	int pcmf_off_fc0 = pcmf_off1_fc0*pcmf_m1;
	int pc_loop_mul_fc0=pix_per_kp * pcmf_off_fc0;
	int pc_loop_bound = total_pixel_fc0 + pc_loop_mul_fc0;
	int Ltotal_pixel_fc0 = scalar_conv_args[88]*output_width;
	int Lpix_per_kp;
	int Lpcmf_off1_fc0 = (Ltotal_pixel_fc0/pix_per_kp) +1;
	int Lpcmf_m1=(scalar_conv_args[17]-1);
	int Lpcmf_off_fc0 = Lpcmf_off1_fc0*Lpcmf_m1;
	int Lpc_loop_mul_fc0=pix_per_kp * Lpcmf_off_fc0;
	int Last_itr_pc_loop_bound = Ltotal_pixel_fc0 + Lpc_loop_mul_fc0;

	if((scalar_conv_args[15] * output_width) < pix_per_kp){
		pc_loop_bound = scalar_conv_args[17]*pix_per_kp;
		Last_itr_pc_loop_bound = scalar_conv_args[17]*pix_per_kp;
	}

	scalar_conv_args[90] = pc_loop_bound;
	scalar_conv_args[91] = Last_itr_pc_loop_bound;
	scalar_conv_args[92] =  scalar_conv_args[13] * filter_height * filter_width * (scalar_conv_args[61]/4);//wts_loop_count          = nkpf x f_h x f_w x compute plane / 4
	scalar_conv_args[93] =  filter_height * filter_width * (scalar_conv_args[62]/16);                   //wts_offset_scale          = fsz^2 x outplane / 16
	scalar_conv_args[94] =  scalar_conv_args[71] * stride;
	scalar_conv_args[95] =  scalar_conv_args[75] * output_width;;

	int outplanes_align16 = scalar_conv_args[62]/32;
	int rem_planes = scalar_conv_args[62] - outplanes_align16*32;
	int crelu_planes0 =  (scalar_conv_args[62]/32) + 1;
	int crelu_planes1 =  (scalar_conv_args[62]/32) + 1;
	if(rem_planes == 16)
		crelu_planes1 = crelu_planes1 - 1;

	if(rem_planes == 0)
	{
		crelu_planes0 = crelu_planes0 - 1;
		crelu_planes1 = crelu_planes1 - 1;
	}

	scalar_conv_args[96] = output_height * output_width * crelu_planes0;  //crelu_out_offset0
	scalar_conv_args[97] = output_height * output_width * crelu_planes1;  //crelu out_offset1

	if(XBATCH_SIZE==2)   //checking for multiples of 16 for batch 2
	{
		scalar_conv_args[96] = output_height * output_width * (scalar_conv_args[62]/16);  //crelu_out_offset0
		scalar_conv_args[97] = output_height * output_width * (scalar_conv_args[62]/16);  //crelu out_offset1
	}
#endif

#if	EN_DEBUG_INIT_PRINT
	cout<<"convInit End: "<<endl;
#endif
}
#endif

#endif //_XCHANGE_CONV_UTILS_HPP_
