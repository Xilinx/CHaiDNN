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

//#include "xi_deconv_config.h"

// Function to compute the log2 of a number which is power of 2
int computeLog21(int number)
{
#pragma HLS INLINE off
	assert(number <= KMAX);
	int res;
	switch(number)
	{
	case 2:
		res = 1; break;
	case 4:
		res = 2; break;
	case 8:
		res = 3; break;
	case 16:
		res = 4; break;
	case 32:
		res = 5; break;
	}

	return res;
}


// Load the complete input data to BRAMs.
// size - Number of elements in the input data.
// template args : [TC_N, TC_Ri, TC_Ci] = [# planes, input rows, input cols]
template<int TC_N, int TC_Ri, int TC_Ci>
void loadInput(INTYPE_FLOAT* in, INBRAMTYPE buffer[DECONV_Tn][2][(NMAX/DECONV_Tn) * ((WIMAX * WIMAX) / 2)], int N, int Ri, int Ci, int size)
{
	INTYPE_FLOAT tmp;					// local variable to read from DDR, then route into	BRAM
	int ddrIndex, bramIndex, bramID1, bramN, dim2;


	LI0:for(int n=0; n<N; n++)
	{
#pragma HLS LOOP_TRIPCOUNT min=TC_N max=TC_N
		LI1:for(int r=0; r<Ri; r++)
		{
#pragma HLS LOOP_TRIPCOUNT min=TC_Ri max=TC_Ri
			LI2:for(int c=0; c<Ci; c++)
			{
#pragma HLS LOOP_TRIPCOUNT min=TC_Ci max=TC_Ci
#pragma HLS LOOP_FLATTEN
#pragma HLS PIPELINE
				ddrIndex = n * Ri * Ci + r * Ci + c;
				bramID1 = n % DECONV_Tn;
				bramN = n/DECONV_Tn;
				dim2 = c%2;			// dim2 is either 0 or 1
				bramIndex = bramN*WIMAX*WIMAX/2 + r * WIMAX/2 + c/2;
				memcpy(&tmp, in+ddrIndex, SIZEOF(INTYPE_FLOAT));
				//tmp = 256;
				buffer[bramID1][dim2][bramIndex] = (INBRAMTYPE)tmp;
			}
		}
	}
}

// Load the complete filters to BRAMs
// size - total number of elements in the filters (N*K*K)
// template args : [TC_N, TC_K] = [# planes, Filter Width or Height]
template<int TC_N, int TC_K>
void loadWeights(WTYPE* in, WBRAMTYPE buffer[4][KMAX/2 * KMAX/2], int N, int K, int DS, int logDS)
{
	WTYPE tmp;					// local variable to read from DDR, then route into BRAM
	int n = 0, r=0, c=0, ind = 0;
	int ddrIndex, bramIndex, bramID1;
	int dim2i, dim2j, dim2, dim3i, dim3j, dim3;

//	LW0:for(int n=0; n<N; n++)
//	{
//#pragma HLS LOOP_TRIPCOUNT min=TC_N max=TC_N
		L1W:for(int r=0; r<K; r++)
		{
#pragma HLS LOOP_TRIPCOUNT min=TC_K max=TC_K
			L2W:for(int c=0; c<K; c++)
			{
#pragma HLS LOOP_TRIPCOUNT min=TC_K max=TC_K
#pragma HLS LOOP_FLATTEN
#pragma HLS PIPELINE
//				ddrIndex = n * K * K + r * K + c;
				ddrIndex = r * K + c;
				//bramID1 = n % DECONV_Tn;
				bramIndex = r * KMAX + c;
				dim2i = r >> logDS; dim2j = c >> logDS; dim2 = dim2i*2 + dim2j;
				dim3i = r & (DS-1); dim3j = c & (DS-1);
//				dim3 = (n/DECONV_Tn)*(KMAX/2) * (KMAX/2) + dim3i*DS + dim3j;
				dim3 = dim3i*DS + dim3j;
				memcpy(&tmp, in+ddrIndex, SIZEOF(WTYPE));
				buffer[dim2][dim3] = tmp;
			}
		}
	}
//}

// Load the bias values to biasBuffer
template<int TC_N>
void loadBias(WTYPE* gmemBias, WBRAMTYPE buffer[NMAX], int N)
{
	WTYPE tmp;
	LB0:for(int i=0; i<N; i++)
	{
#pragma HLS LOOP_TRIPCOUNT min=TC_N max=TC_N
#pragma HLS PIPELINE
		memcpy(&tmp, gmemBias+i, SIZEOF(WTYPE));
		buffer[i] = tmp;
	}
}

// Initialize the output buffers to zero.
// template args : [TC_RiMAX] = [WOMAX]
template<int TC_RiMAX>
void bufferInitialize(OUTBRAMTYPE buffer[WOMAX])
{
	ZI0:for(int i=0; i<WOMAX; i+=2)
	{
#pragma HLS LOOP_TRIPCOUNT min=TC_RiMAX/2 max=TC_RiMAX/2
#pragma HLS PIPELINE
		buffer[i] = -10000.0f;
		buffer[i+1] = -10000.0f;
	}
}

// Function to write the output to DDR
// size - how many elements to write out
template<int TC_Co>
void writeOut(OUTIDTYPE* deconvIDout, int outIDBuffer[WOMAX], int size)//, int offset)
{
	int tmp1, tmp2;
	OUTI_AP_DTYPE tmp;
	//OUTIDTYPE* deconvIDoutptr = deconvIDout + offset;

	W0: for(int i=0; i<size; i++)
	{
#pragma HLS LOOP_TRIPCOUNT min=TC_Co max=TC_Co
#pragma HLS PIPELINE
		for(int j=0; j<OUTIDPACKS; j++)
		{
			tmp1 = outIDBuffer[OUTIDPACKS*i + j];
			tmp.range(((j+1)*32)-1, j*32) = tmp1;		// First value at LSB. Last value at MSB
		}
		memcpy(deconvIDout + i, &tmp, 8);//SIZEOF(OUTIDTYPE));
		//deconvIDout[i] = tmp;
	}
}


// Internal function that computes the TmxDECONV_Tn pixels of output and store it in temporary buffer
void processTmxDECONV_Tn(INBRAMTYPE inputArray[DECONV_Tn][2][(NMAX/DECONV_Tn) * ((WIMAX * WIMAX) / 2)], WBRAMTYPE weightsArray[4][KMAX/2 * KMAX/2], \
		WBRAMTYPE biasArray[NMAX], OUTBRAMTYPE pxF[Tp][DECONV_Tn], int N, int Ri, int Ci, int K, int DS, int logDS, int r, int c, int n)
{
#pragma HLS INLINE

	WBRAMTYPE wtA, wtB, wtC, wtD;
	OUTBRAMTYPE partProdA, partProdB, partProdC, partProdD, partSumAB, partSumCD;
	OUTBRAMTYPE res, finalRes;
	int xi, xj, yi, yj;
	int pxInd;
	ap_uint<1> dim2_AandC, dim2_BandD;
	int inIndexA, inIndexB, inIndexC, inIndexD;
	int addr0, addr1, addr2, addr3;
	INBRAMTYPE inA[DECONV_Tn], inB[DECONV_Tn], inC[DECONV_Tn], inD[DECONV_Tn];

	int inN = n/DECONV_Tn;			// n-index in input BRAM
	int inNoffset = inN * WIMAX * WIMAX/2;		// Remember, it stores only half of a column
	//int wDECONV_Tnoffset = inN * KMAX/2 * KMAX/2;

	xi = r >> logDS; xj = c >> logDS; 				// For input, actually xj=(c+cc)/DS, but here since minimum 2x, it is ok.
	dim2_AandC = (xj-1)&0x1;				// Obtained it from inIndexA=inNoffset + (xi-1)*WIMAX + xj-1;
	dim2_BandD = !dim2_AandC;					// dim2_BandD = !dim2_AandC

	inIndexA = inNoffset + (xi-1)*WIMAX/2 + (xj-1)/2;
	inIndexB = inNoffset + (xi-1)*WIMAX/2 + (xj)/2;
	inIndexC = inNoffset + (xi)*WIMAX/2 + (xj-1)/2;
	inIndexD = inNoffset + (xi)*WIMAX/2 + xj/2;

	if (dim2_BandD) {
		addr0 = inIndexA;
		addr1 = inIndexC;
		addr2 = inIndexB;
		addr3 = inIndexD;
	}
	else {
		addr0 = inIndexB;
		addr1 = inIndexD;
		addr2 = inIndexA;
		addr3 = inIndexC;
	}

	PIN: for(int nn=0; nn<DECONV_Tn; nn++)				// Loop over Tp output pixels in a plane
	{
#pragma HLS UNROLL

		INBRAMTYPE in0 = inputArray[nn][0][addr0];
		INBRAMTYPE in1 = inputArray[nn][0][addr1];
		INBRAMTYPE in2 = inputArray[nn][1][addr2];
		INBRAMTYPE in3 = inputArray[nn][1][addr3];
		if (dim2_BandD)
		{
			inA[nn] = ((xi==0) || (xj==0)) ? (INBRAMTYPE)0.0 : in0;
			inB[nn] = ((xi==0) || (xj==Ci)) ? (INBRAMTYPE)0.0 : in2;
			inC[nn] = ((xi==Ri) || (xj==0)) ? (INBRAMTYPE)0.0 : in1;
			inD[nn] = ((xi==Ri) || (xj==Ci)) ? (INBRAMTYPE)0.0 : in3;
		}
		else
		{
			inA[nn] = ((xi==0) || (xj==0)) ? (INBRAMTYPE)0.0 : in2;
			inB[nn] = ((xi==0) || (xj==Ci)) ? (INBRAMTYPE)0.0 : in0;
			inC[nn] = ((xi==Ri) || (xj==0)) ? (INBRAMTYPE)0.0 : in3;
			inD[nn] = ((xi==Ri) || (xj==Ci)) ? (INBRAMTYPE)0.0 : in1;
		}
	}

	P1: for(int cc=0; cc<Tp; cc++)					// Loop over Tp output pixels in a plane
	{
#pragma HLS UNROLL

		P2: for(int nn=0; nn<DECONV_Tn; nn++)		// Loop over DECONV_Tn planes
		{
#pragma HLS UNROLL

			//	int wtIndex = wDECONV_Tnoffset + (yi<<logDS) + yj;
				yi = r & (DS-1); yj = (c+cc) & (DS-1);			// For weights %DS operation

				// Load weights for processing
				int dsplusyi = DS + yi;
				int dsplusyj = DS + yj;
				bool DSYI = (dsplusyi == K);
				bool DSYJ = (dsplusyj == K);
				bool DSY = (DSYI || DSYJ);
				int wtIndex = (yi<<logDS) + yj;
				wtA = DSY ? (WBRAMTYPE)0 : weightsArray[3][wtIndex];
				wtB = DSYI ? (WBRAMTYPE)0 : weightsArray[2][wtIndex];
				wtC = DSYJ ? (WBRAMTYPE)0 : weightsArray[1][wtIndex];
				wtD = 0 ? (WBRAMTYPE)0 : weightsArray[0][wtIndex];



			partProdA = inA[nn] * wtA; partProdB = inB[nn] * wtB;
			partProdC = inC[nn] * wtC; partProdD = inD[nn] * wtD;

			partSumAB = partProdA + partProdB;
			partSumCD = partProdC + partProdD;
			res = partSumAB + partSumCD;		// Final result

			// Boundary check for number of planes
			WBRAMTYPE biasValue = biasArray[n+nn];
			finalRes = (n+nn < N) ? res + biasValue: (OUTBRAMTYPE)MINUS_INFINITY ;

			pxF[cc][nn] = finalRes;
		}	// P2
	}	// P1
}


// Tree Comparator
void treeComparator1(OUTBRAMTYPE px8[DECONV_Tn], OUTBRAMTYPE *ret_out, int *ret_outloc)
{
#pragma HLS INLINE

	OUTBRAMTYPE px4[DECONV_Tn/2];			// 4
	int loc4[DECONV_Tn/2];

	OUTBRAMTYPE px2[DECONV_Tn/4];			// 2
	int loc2[DECONV_Tn/4];

	OUTBRAMTYPE px1[DECONV_Tn/8];			// 1
	int loc1[DECONV_Tn/8];

	// Start Processing

	PX4:for(int k=0; k<4; k++)
	{
#pragma HLS UNROLL
		px4[k] = MAX(px8[2*k], px8[2*k+1]);
		loc4[k] = MAXLOC(px8[2*k], px8[2*k+1], 2*k, 2*k+1);
	}

	PX2:for(int l=0; l<2; l++)
	{
#pragma HLS UNROLL
		px2[l] = MAX(px4[2*l], px4[2*l+1]);
		loc2[l] = MAXLOC(px4[2*l], px4[2*l+1], loc4[2*l], loc4[2*l+1]);
	}

	px1[0] = MAX(px2[0], px2[1]);
	loc1[0] = MAXLOC(px2[0], px2[1], loc2[0], loc2[1]);

	*ret_out = px1[0];
	*ret_outloc = loc1[0];
}

// Function to compute Tm x DECONV_Tn pixels of output and store it in temporary buffer.
template<int TC_Tp, int TC_DECONV_Tn, int TC_N, int TC_Ro, int TC_Co, int TC_NbyDECONV_Tn, int TC_CobyTp>
void processOneRow(INBRAMTYPE inputArray[DECONV_Tn][2][(NMAX/DECONV_Tn) * ((WIMAX * WIMAX) / 2)], WBRAMTYPE weightsArray[4][KMAX/2 * KMAX/2], \
		WBRAMTYPE biasArray[NMAX], OUTBRAMTYPE scoreBuffer[WOMAX], int idBuffer[WOMAX], int N, int Ri, int Ci, int Ro, int Co, int K, int DS, int logDS, int r)
{
	OUTBRAMTYPE pxF[Tp][DECONV_Tn];

	OUTBRAMTYPE tmpOutA, tmpOutB, newOutA, newOutB;
	int tmpOutLocA, tmpOutLocB, newOutLocA, newOutLocB;

	OUTBRAMTYPE refOutA, refOutB;
	int refLocA, refLocB;
	bool checkA, checkB;

	ML1: for(int n=0; n<N; n+=DECONV_Tn)
	{
#pragma HLS LOOP_TRIPCOUNT min=TC_NbyDECONV_Tn max=TC_NbyDECONV_Tn
		P0: for(int c=0; c<Co; c+=Tp)			// Loop over Co/Tp columns of output
		{
#pragma HLS LOOP_TRIPCOUNT min=TC_CobyTp max=TC_CobyTp
#pragma HLS PIPELINE II=2

			refOutA = scoreBuffer[c];
			refLocA = idBuffer[c];
			refOutB = scoreBuffer[c+1];
			refLocB = idBuffer[c+1];

			processTmxDECONV_Tn(inputArray, weightsArray, biasArray, pxF, N, Ri, Ci, K, DS, logDS, r, c, n);

			treeComparator1(pxF[0], &tmpOutA, &tmpOutLocA);			// We get old Max value from partial output for 8 planes at location c
			treeComparator1(pxF[1], &tmpOutB, &tmpOutLocB);			// We get old Max value from partial output for 8 planes at location c+1

			checkA = refOutA >= tmpOutA;
			newOutA = checkA ? refOutA:tmpOutA;					// Compare it with the refOut
			newOutLocA = checkA ? refLocA:tmpOutLocA+n;

			checkB = refOutB >= tmpOutB;
			newOutB = checkB ? refOutB:tmpOutB;					// Compare it with the refOut
			newOutLocB = checkB ? refLocB:tmpOutLocB+n;

			scoreBuffer[c] = newOutA;									// Keep the new Max value in partial output buffer
			scoreBuffer[c+1] = newOutB;
			idBuffer[c] = newOutLocA;
			idBuffer[c+1] = newOutLocB;

		}	//P0, 8 planes for one row finished, partial output
	} // ML1, All Planes for a row are processed

}

#if 0
#pragma HLS interface m_axi port=_gmemIN offset=slave bundle=gmem0 depth=1792 latency=100 num_write_outstanding=32 num_read_outstanding=32 max_write_burst_length=32 max_read_burst_length=32
#pragma HLS interface m_axi port=_gmemWT offset=slave bundle=gmem1 depth=28672 latency=100 num_write_outstanding=32 num_read_outstanding=32 max_write_burst_length=32 max_read_burst_length=32
#pragma HLS interface m_axi port=_gmemBias offset=slave bundle=gmem2 depth=28672 latency=100 num_write_outstanding=32 num_read_outstanding=32 max_write_burst_length=32 max_read_burst_length=32
#pragma HLS interface m_axi port=_gmemIDout offset=slave bundle=gmem3 depth=131072 latency=100 num_write_outstanding=32 num_read_outstanding=32 max_write_burst_length=32 max_read_burst_length=32

#pragma HLS interface s_axilite port=_gmemIN     		bundle=control
#pragma HLS interface s_axilite port=_gmemWT     		bundle=control
#pragma HLS interface s_axilite port=_gmemBias     		bundle=control
#pragma HLS interface s_axilite port=_gmemIDout    		bundle=control
#pragma HLS interface s_axilite port=_N    			bundle=control
#pragma HLS interface s_axilite port=_Wi    		bundle=control
#pragma HLS interface s_axilite port=_Hi    		bundle=control
#pragma HLS interface s_axilite port=_Wo    		bundle=control
#pragma HLS interface s_axilite port=_Ho    		bundle=control
#pragma HLS interface s_axilite port=_K    			bundle=control
#pragma HLS interface s_axilite port=_DS    		bundle=control

#pragma HLS interface s_axilite port=return 		bundle=control
#endif

//extern "C" {
//int deconv(INTYPE_FLOAT* _gmemIN, WTYPE* _gmemWT, WTYPE* _gmemBias, OUTIDTYPE* _gmemIDout, int _N, int _Wi, int _Hi, int _Wo, int _Ho, int _K, int _DS)
//, int _N, int _Wi, int _Hi, int _Wo, int _Ho, int _K, int _DS)
int xideconv(INTYPE_FLOAT* deconvIN, WTYPE* deconvWT, WTYPE* deconvBias, OUTIDTYPE* deconvIDout, int *scalar_deconv_args)
{
		int N  = scalar_deconv_args[0];//_N;		// Number of planes
		int Ci = scalar_deconv_args[1];//_Wi;		// Input Width or number of Columns
		int Ri = scalar_deconv_args[2];//_Hi;		// Input Height or number of Rows
		int Co = scalar_deconv_args[3];//_Wo;		// Output Width or number of Columns
		int Ro = scalar_deconv_args[4];//_Ho;		// Output Height or number of Rows
		int K  = scalar_deconv_args[5];//_K;		// Filter Width/Height
		int DS = scalar_deconv_args[6];//_DS;		// Deconvolution Stride

		//fprintf(stderr,"planes:%d inpwidth:%d inpheight:%d opwidth%d opheight:%d filterwidth:%d stride:%d",N,Ci,Ri,Co,Ro,K,DS);

		//FILE *fp = fopen("deconv_wgts.txt", "w");

/*
	N = 2;//_N;			// Number of planes
	Ci = 30;//_Wi;		// Input Width or number of Columns
	Ri = 30;//_Hi;		// Input Height or number of Rows
	Co = 496;//_Wo;		// Output Width or number of Columns
	Ro = 496;//_Ho;		// Output Height or number of Rows
	K = 32;//_K;			// Filter Width/Height
	DS = 16;//_DS;		// Deconvolution Stride
*/
	int inSize = N * Ri * Ci;		// Total number of elems in the input		// TODO : At the end, check if it can be computed inside loadInput(). Same for wtSize
	int wtSize = N * K * K;			// Total number of elems in the filter
	
	#if 0//!__SYNTHESIS__
	
	fprintf(stderr,"planes:%d inpwidth:%d inpheight:%d opwidth%d opheight:%d filterwidth:%d stride:%d",N,Ci,Ri,Co,Ro,K,DS);
	
	
	FILE *fp_input = fopen("inp_deconv.txt","w");
	
	for(int i=0; i<2; i++)
        {
            for(int j=0; j<30; j++)
            {
                for(int k=0; k<30; k++)
                {
                    int idx = i*30*30 + j*30 + k;
                    
                    //fprintf(fp_input, "%d ", deconvIN[idx]);
                    fprintf(fp_input, "%f ", deconvIN[idx]);
                  
                }
            }
            fprintf(fp_input, "\n");
        }
	
	
	fclose(fp_input);
	
	
	FILE *fp_weights = fopen("weights_deconv.txt","w");
	
	for(int i=0; i<2; i++)
        {
            for(int j=0; j<32; j++)
            {
                for(int k=0; k<32; k++)
                {
                    int idx = i*32*32 + j*32 + k;
                    //fprintf(fp_weights, "%hu ", ((short)deconvWT[idx]));
                    fprintf(fp_weights, "%f ", ((float)deconvWT[idx]));
                }
            }
            fprintf(fp_weights, "\n");
        }
    fclose(fp_weights);
    
    FILE *fp_bias = fopen("bias_deconv.txt","w");
    
    //fprintf(fp_bias,"%d %d\n",((short)deconvBias[0]),((short)deconvBias[1]));
    fprintf(fp_bias,"%f %f\n",((float)deconvBias[0]),((float)deconvBias[1]));
    
    fclose(fp_bias);
    
	
	#endif
	

	int pingpong, offset;

	assert((N>0) && (N <= NMAX));
	assert((Ri>0) && (Ri <= WIMAX));
	assert((Ci>0) && (Ci <= WIMAX));
	assert((Ro>0) && (Ro <= WOMAX));
	assert((Co>0) && (Co <= WOMAX));
	assert((K > 0) && (K <= KMAX));
	assert((DS>0) && (DS <= KMAX/2));

	// Local Buffers for input and weights
	INBRAMTYPE inputBuffer[DECONV_Tn][2][(NMAX/DECONV_Tn) * ((WIMAX * WIMAX) / 2)];
#pragma HLS RESOURCE variable=inputBuffer core=RAM_T2P_BRAM
#pragma HLS ARRAY_PARTITION variable=inputBuffer complete dim=1
#pragma HLS ARRAY_PARTITION variable=inputBuffer complete dim=2

	WBRAMTYPE weightsBuffer[4][KMAX/2 * KMAX/2];
#pragma HLS RESOURCE variable=weightsBuffer core=RAM_T2P_BRAM
#pragma HLS ARRAY_PARTITION variable=weightsBuffer complete dim=1
//#pragma HLS ARRAY_PARTITION variable=weightsBuffer complete dim=2

	WBRAMTYPE biasBuffer[NMAX];
#pragma HLS ARRAY_PARTITION variable=biasBuffer complete dim=1

	// Local buffers for output buffer, stores 1 row at a time
	OUTBRAMTYPE outScoreBuffer0[WOMAX], outScoreBuffer1[WOMAX];
	IDTYPE outIDBuffer0[WOMAX], outIDBuffer1[WOMAX];
#pragma HLS ARRAY_PARTITION variable=outScoreBuffer0 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=outScoreBuffer1 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=outIDBuffer0 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=outIDBuffer1 cyclic factor=2 dim=1


	//OUTBRAMTYPE dbgArray[543][543];

	// initialize the log2 LUT
	int log2DS = computeLog21(DS);

	// Zero initialize the buffers
	bufferInitialize<WOMAX>(outScoreBuffer0);
	bufferInitialize<WOMAX>(outScoreBuffer1);

	// load the inputs and filters
	loadInput<7, 15, 15>(deconvIN, inputBuffer, N, Ri, Ci, inSize);
	loadWeights<7, 64>(deconvWT, weightsBuffer, N, K, DS, log2DS);
	loadBias<7>(deconvBias, biasBuffer, N);

	// Initialize PING processing
	pingpong = 0;
	processOneRow<Tp, DECONV_Tn, 7, 512, 512, (7+DECONV_Tn-1)/DECONV_Tn, (512+Tp-1)/Tp>(inputBuffer, weightsBuffer, biasBuffer, outScoreBuffer0, outIDBuffer0, N, Ri, Ci, Ro, Co, K, DS, log2DS, 0);		// Process first row - PING
	pingpong = 1;

#if DBGARRAY
			for(int i=0 ; i<Co; i++)
			{
				dbgArray[0][i] = outScoreBuffer0[i];
			}
#endif

	offset = 0;			// Offset for writing to DDR

	int Roby2 = (Ro + (OUTIDPACKS-1) ) / OUTIDPACKS;		 // = (Ro+1) / 2
	int Coby2 = (Co + (OUTIDPACKS-1) ) / OUTIDPACKS;		 // = (Ro+1) / 2

	ML0: for(int r=1; r<Ro; r++)//, offset+=Coby2)
	{
#pragma HLS LOOP_TRIPCOUNT min=512 max=512
		if(pingpong == 0)
		{
			processOneRow<Tp, DECONV_Tn, 7, 512, 512, (7+DECONV_Tn-1)/DECONV_Tn, (512+Tp-1)/Tp>(inputBuffer, weightsBuffer, biasBuffer, outScoreBuffer0, outIDBuffer0, N, Ri, Ci, Ro, Co, K, DS, log2DS, r);
			writeOut<256>(deconvIDout, outIDBuffer1, Coby2);//, offset);
			bufferInitialize<WOMAX>(outScoreBuffer1);
			pingpong = 1;

#if DBGARRAY
			for(int i=0 ; i<Co; i++)
			{
				dbgArray[r][i] = outScoreBuffer0[i];
			}
#endif
		}
		else
		{
			processOneRow<Tp, DECONV_Tn, 7, 512, 512, (7+DECONV_Tn-1)/DECONV_Tn, (512+Tp-1)/Tp>(inputBuffer, weightsBuffer, biasBuffer, outScoreBuffer1, outIDBuffer1, N, Ri, Ci, Ro, Co, K, DS, log2DS, r);
			writeOut<256>(deconvIDout, outIDBuffer0, Coby2);//, offset);
			bufferInitialize<WOMAX>(outScoreBuffer0);
			pingpong = 0;
#if DBGARRAY
			for(int i=0 ; i<Co; i++)
			{
				dbgArray[r][i] = outScoreBuffer1[i];
			}
#endif
		}

		deconvIDout = deconvIDout + Coby2;
	} // ML0, All rows are processed

	// Final PONG write
	if(pingpong == 0)
		writeOut<256>(deconvIDout, outIDBuffer1, Coby2);//, offset);
	else
		writeOut<256>(deconvIDout, outIDBuffer0, Coby2);//, offset);
		
		
		#if 0//!__SYNTHESIS__
		
		
				FILE *foutput = fopen("output_kernel.txt","w");

				for(int i=0;i<496;i++)
				{
				for(int j=0;j<248;j++)
				{
					//fprintf(foutput,"%d %d ",((int)deconvIDout[i*248+j].range(31,0)),((int)deconvIDout[i*248+j].range(63,32)));
					}
					fprintf(foutput,"\n");
				}

				fclose(foutput);
		
		#endif

#if DBGARRAY
	FILE* fp = fopen("outScore.txt", "w");
	for(int r=0 ; r<Ro; r++)
	{
		for(int c=0; c<Co; c++)
		{
			fprintf(fp, "%f\t", (float)dbgArray[r][c]);
		}
		fprintf(fp, "\n");
	}
#endif
	
	return 0;

}
//}
