// Convolution and Pool Init standalone functions
#include "ap_int.h"
#include "../common/kernelinfo_class.h"
#include "../init/xi_dnn_conv_utils.h"

#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)
#define ConvertToFP(fVal, iPart, fbits)	((int)((iPart<<fbits) + ((fVal-(float)iPart))*(1<<fbits)))

struct Parameter_struct
{
	// fields
	int N;                            // Input Feature Maps
	int M;                            // Output Feature Maps
	int filter_h, filter_w;           // Filter size - filter_h x filter_w
	int stride_h, stride_w;           // Stride
	int pad_h, pad_w;                 // Padding
	int dilation;                     // dilation factor
	int reluflag;                     // ReLU enabled/disabled
	int group;                        //
	int opcode;
	int inp_height;
	int inp_width;
	int out_height;
	int out_width;
};

void mem_offset_group_sa(int in_h, int  in_w, int in_p,
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

void straddleFactorCount_sa( int *scalar_conv_args)
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

void stgRowCount_sa(int *scalar_conv_args)
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

	//*stgRowCount_sa = stgcount;
	scalar_conv_args[14] = stgcount;

	//            *ostgRowCount_sa = (stgcount - fsz)/fst + 1 ;

	int ostgcount;

	if(stgcount == ih && possible_output_rows >= oh)
		ostgcount = oh;
	else
		ostgcount = (stgcount - fsz_dilated)/fst + 1 ;

	//*ostgRowCount_sa = ostgcount;
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
	if(possible_output_rows == 0)
	{
		fprintf(stderr, "\n***** ostageBuf bram depth is not sufficient");
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
	scalar_conv_args[56] = conv3d_stg_row_cnt;

	if(stgcount > ih)
		stgcount = ih;

	//*stgRowCount_sa = stgcount;
	scalar_conv_args[14] = stgcount;

	//            *ostgRowCount_sa = (stgcount - fsz)/fst + 1 ;

	int ostgcount;

	if(stgcount == ih && possible_output_rows >= oh)
		ostgcount = oh;
	else
		ostgcount = (stgcount - fsz_dilated)/fst + 1 ;

	//*ostgRowCount_sa = ostgcount;
	scalar_conv_args[15] = ostgcount;

	if((scalar_conv_args[34] == OPCODE_3D_CONV) || (scalar_conv_args[34] == OPCODE_POOL_CONV2CONV))//if(opcode == 23 || opcode == 19)
		scalar_conv_args[58] =  stgcount;
	else
		scalar_conv_args[58] =  ostgcount;

	//fprintf(stderr,"\n+++++++++++++++***** ostageBuf possible rows = %d\n", ostgcount);
}

void stgRowCount_sa_poolsplit(int *scalar_conv_args)
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
	if(possible_output_rows == 0)
	{
		fprintf(stderr, "\n***** ostageBuf bram depth is not sufficient");
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

	//*stgRowCount_sa = stgcount;
	scalar_conv_args[117] = stgcount;

	//            *ostgRowCount_sa = (stgcount - fsz)/fst + 1 ;

	int ostgcount;

	if(stgcount == ih && possible_output_rows >= oh)
		ostgcount = oh;
	else
		ostgcount = (stgcount - fsz_dilated)/fst + 1 ;

	//*ostgRowCount_sa = ostgcount;
	scalar_conv_args[118] = ostgcount;

	if((scalar_conv_args[34] == OPCODE_3D_CONV) || (scalar_conv_args[34] == OPCODE_POOL_CONV2CONV))//if(opcode == 23 || opcode == 19)
		scalar_conv_args[116] =  stgcount;
	else
		scalar_conv_args[116] =  ostgcount;

	//fprintf(stderr,"\n+++++++++++++++***** ostageBuf possible rows = %d\n", ostgcount);
}

void nkpfCount_sa(int *scalar_conv_args)
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

void overflowPattern_sa(int l_inout_precision_ffa0, int l_int6_en_out, ap_uint<24> &overflow_pattern)
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


void hardwareParamInit(Parameter_struct *conv_params, int *scalar_conv_args)
{

#if	EN_DEBUG_INIT_PRINT
	cout<<"convInit Start: "<<endl;
#endif

	//ConvolutionParameter *conv_params = xlayer_seq->hw_ops->conv_params;
	//int *scalar_conv_args = (int *)currentLayer->params;

	//vector<int> bottomShape =  xlayer_seq->hw_ops->bottomShape.at(0);//.blob->shape;
	//vector<int> topShape    =  xlayer_seq->hw_ops->topShape.at(0);//.blob->shape;

	int conv_inp_height, conv_inp_width;
	int conv_out_height, conv_out_width;
	int conv_inp_planes, conv_out_planes;
	int conv_filter_height, conv_filter_width;
	int conv_stride, conv_dilation, conv_pad, conv_group;
	int th_in, th_out;

	scalar_conv_args[34] = conv_params->opcode;        //Opcode

	//string quant_scheme = xlayer_seq->hw_ops->quantization_scheme;

	//cout << "Quant scheme : " << xlayer_seq->hw_ops->quantization_scheme << endl;

	//# Loading first layer threshold param
	//# This is used for in offline quant mode for input quantization
	float th_layer_in = 0;
	float th_layer_out = 0;
	float sf_in = 0;
	float sf_out = 0;

	scalar_conv_args[127] = 0;

#if 0
	//if((layer_id == 0) && (quant_scheme.compare("Xilinx") == 0))
	if(quant_scheme.compare("Xilinx") == 0)
	{
		th_layer_in  = xlayer_seq->hw_ops->th_layer_in;
		th_layer_out = xlayer_seq->hw_ops->th_layer_out;
		scalar_conv_args[127] = 1;  //offline mode

		if((scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV) || (scalar_conv_args[34] == OPCODE_FC2CONV))//Checking for MAX_POOL or AVG_POOL
		{
			th_layer_in  = conv_params->th_layer_in_3d;//39.5895589117;
			th_layer_out = conv_params->th_layer_out_3d;//39.5895589117;
		}

		int ip_bw =  currentLayer->qf_format.ip_bw;
		sf_in = th_layer_in / (pow(2, ip_bw - 1) - 1);

		int op_bw =  currentLayer->qf_format.op_bw;
		sf_out = th_layer_out / (pow(2, op_bw - 1) - 1);

	}

	currentLayer->float_params.push_back(th_layer_in);
	currentLayer->float_params.push_back(th_layer_out);
	currentLayer->float_params.push_back(sf_in);
	currentLayer->float_params.push_back(sf_out);
#endif

	{
		conv_inp_height  = conv_params->inp_height;  //input_height
		conv_inp_width   = conv_params->inp_width;   //input_width
		conv_out_height  = conv_params->out_height;  //output_height
		conv_out_width   = conv_params->out_width;   //output_width
		conv_out_planes  = conv_params->M;     //output planes
		conv_inp_planes  = conv_params->N;     //input planes
		conv_stride      = conv_params->stride_h;  //stride_h
		conv_filter_height = conv_params->filter_h;   //filter_h
		conv_filter_width  = conv_params->filter_w;  //filter_w
		conv_pad         = conv_params->pad_h;       //pad_h
		conv_group       = conv_params->group-1;     //default group is 1
		conv_dilation    = conv_params->dilation;    //dilation
	}

	scalar_conv_args[0]  = conv_inp_height;
	scalar_conv_args[1]  = conv_inp_width;
	scalar_conv_args[2]  = conv_out_height;
	scalar_conv_args[3]  = conv_out_width;
	scalar_conv_args[4]  = conv_out_planes;
	scalar_conv_args[5]  = conv_inp_planes;
	scalar_conv_args[6]  = conv_stride;
	scalar_conv_args[7]  = conv_filter_height;
	scalar_conv_args[8]  = conv_filter_width;
	scalar_conv_args[9]  = conv_pad;         //pad_h
	scalar_conv_args[11] = conv_group;
	scalar_conv_args[29] = conv_dilation;

	scalar_conv_args[10] = conv_params->reluflag;     //relu
	if (conv_inp_planes < 4)
		scalar_conv_args[12] = 0;//conv_params->layer_id;     //layer_id   //for layer 0
	else
		scalar_conv_args[12] = 1;
	//cout << "relu start : " << scalar_conv_args[10] << endl;


	if( (scalar_conv_args[34] == OPCODE_BN) || (scalar_conv_args[34] == OPCODE_ELTWISE) )
	{
		//scalar_conv_args[10] = conv_params->extra_reluflag;
		scalar_conv_args[6] = 1;
		scalar_conv_args[7] = 1;
		scalar_conv_args[8] = 1;
	}

	if((scalar_conv_args[34] == OPCODE_3D_CONV) || (scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV))
	{
		scalar_conv_args[43] = scalar_conv_args[0];  //input height
		scalar_conv_args[44] = scalar_conv_args[1];  //input width
	}

	if((scalar_conv_args[34] == OPCODE_3D_CONV))
	{
		scalar_conv_args[45] = scalar_conv_args[0];  //output height
		scalar_conv_args[46] = scalar_conv_args[1];  //output width
	}

	if((scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV))
	{
		scalar_conv_args[45] = scalar_conv_args[2];  //output height
		scalar_conv_args[46] = scalar_conv_args[3];  //output width
	}

	if((scalar_conv_args[34] == OPCODE_3D_CONV) || (scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV) || (scalar_conv_args[34] == OPCODE_POOL_CONV2CONV))
	{
		scalar_conv_args[47] = conv_params->filter_h;
		scalar_conv_args[48] = conv_params->filter_w;
		scalar_conv_args[49] = conv_params->filter_h*conv_params->filter_w;//-fsz2;   //???
		scalar_conv_args[50] = conv_params->N;    //input plane
		scalar_conv_args[51] = conv_params->M;    //output plane
		scalar_conv_args[52] = conv_params->pad_h;
		scalar_conv_args[53] = conv_params->stride_h;
		scalar_conv_args[54] = conv_params->reluflag;
		/*
		scalar_conv_args[55] = 0;                    //input offset
		//scalar_conv_args[55] = ((conv_params->pad_h_3d/conv_params->stride_h_3d)*scalar_conv_args[44]) + 1;  //((3d_pad/3d_stride)*3d_ip_w) + 1
		int conv_3d_req_rows = (conv_params->filter_h_3d-conv_params->stride_h_3d-conv_params->pad_h_3d);
		if(conv_3d_req_rows < 0)
		{
			conv_3d_req_rows = 0;
		}
		scalar_conv_args[55] = (conv_3d_req_rows*scalar_conv_args[44]) + conv_params->pad_h_3d;  //((3d_pad/3d_stride)*3d_ip_w) + 1
		*/

		scalar_conv_args[55] = ((scalar_conv_args[52] + scalar_conv_args[53] -1)/scalar_conv_args[53])*(scalar_conv_args[44] + 1);

		//scalar_conv_args[56] = 0;                    //istg_row_cnt

		//int io_precision = currentLayer->qf_format.ip_fbits+currentLayer->qf_format.wt_fbits-currentLayer->qf_format.op_fbits;

		int io_precision = 0;//conv_params->ip_fl_3d+multiplier_fl-conv_params->op_fl_3d;

		//fprintf(stderr, "io_precision : %d multiplier_fl : %d\n", io_precision, multiplier_fl);

		// TODO: Hardcoded for googlenet, has to be modified
		scalar_conv_args[57] = 16;//15;//io_precision;

		scalar_conv_args[42] = 1<< (io_precision - 1);   //rounding

		scalar_conv_args[59] = scalar_conv_args[43]*scalar_conv_args[44];                    //inp_h*inp_w
	}
	else
	{
		//Default 3D conv params
		scalar_conv_args[43] = scalar_conv_args[0];  	//input height
		scalar_conv_args[44] = scalar_conv_args[1];  	//input width
		scalar_conv_args[45] = scalar_conv_args[2];  	//output height
		scalar_conv_args[46] = scalar_conv_args[3];  	//output width
		//scalar_conv_args[47] = 1;  						//filter height
		scalar_conv_args[47] = 1+conv_pad;//conv_params->pad_h;
		scalar_conv_args[48] = 1;					    //filter width
		scalar_conv_args[49] = 1;                       //fsz2
		scalar_conv_args[50] = conv_inp_planes;//conv_params->N;    		//input plane
		scalar_conv_args[51] = conv_out_planes;//conv_params->M;    		//output plane

		scalar_conv_args[52] = conv_pad;//conv_params->pad_h;
		scalar_conv_args[53] = 1;                       //stride_h_3d

		//# For normal Conv - Loading the extra out ptrs with dummy values
		//# This is to avoid HW exception
		//currentLayer->out_ptrs[2] = currentLayer->out_ptrs[0];
		//currentLayer->out_ptrs[3] = currentLayer->out_ptrs[1];


		//scalar_conv_args[56] = scalar_conv_args[14];    //conv3d_stg_row_cnt
		scalar_conv_args[59] = scalar_conv_args[0]*scalar_conv_args[1];                    //inp_h*inp_w
	}

	if((scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV))
	{
		scalar_conv_args[5]  = scalar_conv_args[50];
		scalar_conv_args[6]  = scalar_conv_args[53];
	}


	if(scalar_conv_args[34] == OPCODE_AVRPOOL2CONV)
	{
			//printf("stop");
	}

	if( (scalar_conv_args[34] == OPCODE_POOL2CONV) || (scalar_conv_args[34] == OPCODE_AVRPOOL2CONV) )
	{
		int inp_planes = conv_inp_planes;
		//if(inp_planes > 400)
			//printf("stop");

		int out_planes = conv_out_planes;

		int op_planes_split_cnt = 1;
		int output_planes_split = 1;

		do{
			stgRowCount_sa(scalar_conv_args);  //update 56-conv3d_stg_row_cnt, 58-conv3d_ostg_row_cnt, 14-istg_row_cnt, 15-ostgRowCount_sa params

			if(scalar_conv_args[15] <= 0)  //ostgrowcnt
			{
				inp_planes = inp_planes/2;
				out_planes = out_planes/2;
				inp_planes = AlignSize(inp_planes, 32);
				out_planes = AlignSize(out_planes, 32);
				scalar_conv_args[4] = inp_planes;
				scalar_conv_args[5] = out_planes;
				scalar_conv_args[50] = inp_planes;    //input plane
				scalar_conv_args[51] = out_planes;    //output plane
				op_planes_split_cnt++;
			}
			else
				output_planes_split = 0;
		}while(output_planes_split==1);

		if(op_planes_split_cnt>0)
		{
			scalar_conv_args[113] = AlignSize((conv_inp_planes-(inp_planes*(op_planes_split_cnt-1))),32);
			scalar_conv_args[114] = AlignSize((conv_out_planes-(out_planes*(op_planes_split_cnt-1))),32);
#if !SINGLE_IO_PORT
			inp_planes = inp_planes/2;
			out_planes = out_planes/2;
#endif
			scalar_conv_args[110] = op_planes_split_cnt;
			scalar_conv_args[111] = inp_planes * conv_inp_width * conv_inp_height * XBATCH_SIZE;
			scalar_conv_args[112] = out_planes * conv_out_width * conv_out_height * XBATCH_SIZE;

			stgRowCount_sa_poolsplit(scalar_conv_args);  //update 115-conv3d_stg_row_cnt, 116-conv3d_ostg_row_cnt, 117-istg_row_cnt, 118-ostgRowCount_sa params
			scalar_conv_args[109] = conv_out_planes;
#if 0
	for(int i=110;i<119;i++)
		fprintf(stderr,"\t%d",(int)scalar_conv_args[i]);
#endif
		}
	}
	else
	{
		stgRowCount_sa(scalar_conv_args);
		scalar_conv_args[110] = 1;
	}

	if(scalar_conv_args[34] != OPCODE_3D_CONV)
	{
		scalar_conv_args[56] = scalar_conv_args[14];    //conv3d_stg_row_cnt
	}

	straddleFactorCount_sa(scalar_conv_args);  //update 16-compute_planes, 17-straddle_factor

	nkpfCount_sa(scalar_conv_args);  //19-slk


	int group_flag = scalar_conv_args[11];

	int io_precision = 0;

	if(scalar_conv_args[34] == OPCODE_FC2CONV)
	{
		io_precision = 0;
	}

	io_precision = QUANT_PREC_SHIFT;
	scalar_conv_args[20] = QUANT_PREC_SHIFT;


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
	float alpha   = 1;//xlayer_seq->hw_ops->conv_params->lrn_alpha;
	int localSize = 1;//xlayer_seq->hw_ops->conv_params->lrn_lsize;
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

	scalar_conv_args[36] = 2;//currentLayer->qf_format.ip_fbits;//6;//2;  //Norm_prec is assume to be 2
	//Norm_prec2 = 14 + Fbits_in + Fbits_out
	scalar_conv_args[37] = 14+2+2;//currentLayer->qf_format.ip_fbits+currentLayer->qf_format.op_fbits;

	//# int6 input flag
	//# TODO: Fix this
	//if((conv_params->ip_bw == 6))
		scalar_conv_args[39] = 1;
	//else
		//scalar_conv_args[39] = 0;

	//# int6 output flag
	//# TODO: Fix this
	//if((conv_params->op_bw == 6))
		scalar_conv_args[40] = 1;
	//else
		//scalar_conv_args[40] = 0;


	ap_uint<24> overflow_pattern;
	//# overflowPattern_sa
	//overflowPattern_sa(io_precision, scalar_conv_args[40], overflow_pattern);
	//scalar_conv_args[41] = overflow_pattern;

	scalar_conv_args[41] = 1 << (io_precision - 1);   // conv rounding //TODO CHECKING Functionality with ROUND: ANITHA
	//scalar_conv_args[43] = scalar_conv_args[41] - 1;
	//scalar_conv_args[44] = l_rounding_bn - 1;


	if( (scalar_conv_args[34] == OPCODE_BN) || (scalar_conv_args[34] == OPCODE_FUSE_BN_CONV) )
	{
#if 0
		if(scalar_conv_args[34] == OPCODE_FUSE_BN_CONV)
		{
			scalar_conv_args[34] = OPCODE_BN;
			scalar_conv_args[10] = 0;
		}
#endif

		scalar_conv_args[35] = 1<<2;//currentLayer->qf_format.ip_fbits;

		float epsilon =  1;//xlayer_seq->hw_ops->conv_params->batchnorm_eps;
		int nElems  = scalar_conv_args[5];   //inp planes
		int ip_fbits = 2;//currentLayer->qf_format.ip_fbits;
		int op_fbits = 2;//currentLayer->qf_format.op_fbits;

		int mean_fbits = 2;//xlayer_seq->hw_ops->bn_mean_fl;
		int variance_fbits = 2;//xlayer_seq->hw_ops->bn_variance_fl;

		int gamma_fbits = 2;//xlayer_seq->hw_ops->scale_gamma_fl;
		int beta_fbits = 2;//xlayer_seq->hw_ops->scale_beta_fl;
		int gamma_by_std_fbits = 2;//xlayer_seq->hw_ops->scale_gamma_by_std_fl;

		int norm_prec = ip_fbits+gamma_by_std_fbits-op_fbits;
		int norm_prec_3 = ip_fbits+gamma_by_std_fbits-beta_fbits;
		scalar_conv_args[36] = norm_prec;
		scalar_conv_args[37] = 0;
		scalar_conv_args[38] = norm_prec_3;

		//# overflowPattern_sa for batch norm
		overflowPattern_sa(norm_prec, scalar_conv_args[40], overflow_pattern);
		scalar_conv_args[42] = overflow_pattern;    //TODO:FOR LRN we need to update



		mean_fbits = ip_fbits;


		gamma_fbits = 2+8;//xlayer_seq->hw_ops->scale_gamma_fl + 8;
		beta_fbits = 2+8;//xlayer_seq->hw_ops->scale_beta_fl+8;
		gamma_by_std_fbits = 2+8;//xlayer_seq->hw_ops->scale_gamma_by_std_fl+8;

		norm_prec = ip_fbits+8+gamma_by_std_fbits-op_fbits;
		norm_prec_3 = ip_fbits+8+gamma_by_std_fbits-beta_fbits;
		scalar_conv_args[36] = norm_prec;
		scalar_conv_args[37] = 0;
		scalar_conv_args[38] = norm_prec_3;

		//# overflowPattern_sa for batch norm
		//overflowPattern_sa(norm_prec, scalar_conv_args[40], overflow_pattern);
		//scalar_conv_args[42] = overflow_pattern;    //TODO:FOR LRN we need to update

		mean_fbits = ip_fbits + 8;
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

	//scalar_conv_args[11] = 0;              //making group as zero for the first group


	//scalar_conv_args[58] = currentLayer->en_batch_size_one; //BATCH_SIZE_ONE_ENABLE


	//TODO:Extra Relu flag

#if 1
	///*************************************Scalar arguments from 60 to 94 for DSP optimization *************************************

	int pixProc;
	int input_height = scalar_conv_args[0];  	//input height
	int input_width  = scalar_conv_args[1];  	//input width
	int output_height = scalar_conv_args[2];  	//output height
	int output_width  = scalar_conv_args[3];  	//output width
	int filter_height = scalar_conv_args[7];//conv_params->filter_h;  //filter_h
	int filter_width  = scalar_conv_args[8];//conv_params->filter_w;  //filter_w
	int dilation_factor = scalar_conv_args[29]; //dilation
	int stride        = scalar_conv_args[6];    //stride_h

	if((scalar_conv_args[39] == 1))// || (scalar_conv_args[58]==1))
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

	mem_offset_group_sa(input_height, input_width, scalar_conv_args[63], output_height, output_width, scalar_conv_args[62],
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

	//scalar_conv_args[86] = (XI_ROW8*8 + XI_ROW4*4 + XI_ROW2*2)*input_width;               //Layer1: input_rows_per_bram * input_width
	//scalar_conv_args[86] = (2)*input_width;               //Layer1: input_rows_per_bram * input_width

#if XI_ROW2==1
	int inputRows_inBRAM = 2;
#elif XI_ROW4==1
	int inputRows_inBRAM = 4;
#elif XI_ROW8==1
	int inputRows_inBRAM = 8;
#endif
	scalar_conv_args[86] = inputRows_inBRAM*input_width;

	//scalar_conv_args[87] = output_height * output_width * (scalar_conv_args[62]/16);  //crelu_out_offset
	scalar_conv_args[87]  = scalar_conv_args[45] * scalar_conv_args[46];//3d_conv_output_height * 3d_conv_output_width   //output size


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

	if((scalar_conv_args[15] * output_width) <= pix_per_kp){
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

	scalar_conv_args[11] = 0;              //making group as zero for the first group

	/* when group=2 */
	scalar_conv_args[78] = 0;
	scalar_conv_args[79] = 0;
	scalar_conv_args[80] = 0;
	scalar_conv_args[81] = 0;
	scalar_conv_args[82] = 0;

	/* args for dsp-opt code changes */
	int lrn_inter_sos_enable;
	if (scalar_conv_args[34] == 7)  //OPCODE_LRN_INTER_SOS = 7
		lrn_inter_sos_enable = 1;
	else
		lrn_inter_sos_enable = 0;

	int filter_size_dilated;
	int filter_stride;
	int pad_num;
	if (lrn_inter_sos_enable == 1)
	{
		filter_size_dilated = 1;
		filter_stride = 1;
		pad_num = 0;
	}
	else
	{
		filter_size_dilated = scalar_conv_args[67];//l_filter_height_dil_ffa0;
		filter_stride = scalar_conv_args[6];//(ap_uint<4> ) (l_conv_stride_ffa0);
		pad_num = scalar_conv_args[9];//(ap_uint<8> ) l_pad_ffa0;
	}

	scalar_conv_args[98] = -scalar_conv_args[52];  //-conv_params->pad_h_3d;
	//int endrow_fb0 = startrow_inc_fb0 + (weight_desc.filter_size_dilated - weight_desc.filter_stride - conv_desc.pad_num -1)*(conv_desc.conv3d_stride)+(conv_desc.conv3d_ft_h - conv_desc.conv3d_pad - 1);
	//int endrow_fb0 = scalar_conv_args[83] + (filter_size_dilated - filter_stride - pad_num -1) * (scalar_conv_args[53])+(scalar_conv_args[47] - scalar_conv_args[52] - 1);
        int endrow_fb0;
	if(scalar_conv_args[34] != 20)
	endrow_fb0 = (scalar_conv_args[15]*stride + filter_size_dilated - filter_stride - pad_num -1) * (scalar_conv_args[53])+(scalar_conv_args[47] - scalar_conv_args[52] - 1);
	else
	endrow_fb0 = ((scalar_conv_args[58] -1)*scalar_conv_args[53]) + scalar_conv_args[47] - scalar_conv_args[52] -1 ;
	//endrow = (pool_ostg_row_cnt -1) * (pool_stride) + pool_ft_h - pool_pad - 1


	scalar_conv_args[99] = endrow_fb0;

	//# Added for avg pool
	//# Pool Pad : scalar_conv_args[53]

	input_width = scalar_conv_args[44];
	output_width = scalar_conv_args[46];

	short eff_w;
	short op_width = ((input_width +2*conv_pad- filter_width)/scalar_conv_args[53])+1;

	if(op_width < output_width)
		eff_w = input_width + (output_width - op_width);
	else
		eff_w = input_width;

	scalar_conv_args[101] = eff_w;


	/* Average pool param */
	float f_val = (float)1/(conv_filter_height * conv_filter_width);
	ival = (char)(f_val);
	fxval = ConvertToFP( f_val, ival, 8);

	scalar_conv_args[100] = 1;


	int offset_size;
	int align_factor = ALIGN_FACTOR(HCONV_BIAS_PORT_WIDTH, HCONV_BIAS_DATA_WIDTH);
	int t_group_size, sf_grp_size, sf_align_size;


		scalar_conv_args[103] = 0;
		scalar_conv_args[102] = 0;


	//scalar_conv_args[102] = AlignSize(scalar_conv_args[4], ALIGN_FACTOR(HCONV_BIAS_PORT_WIDTH, HCONV_BIAS_DATA_WIDTH));


	scalar_conv_args[120] = input_group_offset_1st;
	scalar_conv_args[121] = input_group_offset_other;
	scalar_conv_args[122] = weights_group_offset;
	scalar_conv_args[123] = output_group_offset;
	scalar_conv_args[124] = bias_group_offset;
	//# Scale factor offset for group
	int t_size = AlignSize(offset_size, align_factor);
	scalar_conv_args[125] = (t_size/align_factor) + (t_size/2);

	scalar_conv_args[126] = 0;//currentLayer->en_batch_size_one;

	//# Used for reset in grouping case
	scalar_conv_args[119] = scalar_conv_args[102];


	//	cout << "relu end : " << scalar_conv_args[10] << endl;
	if(scalar_conv_args[34] == OPCODE_FC2CONV)
	{
		scalar_conv_args[125] = 0;
	}



#endif

#if	EN_DEBUG_INIT_PRINT
	cout<<"convInit End: "<<endl;
#endif
}