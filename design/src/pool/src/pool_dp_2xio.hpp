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

#ifndef _AU_POOLING_LAYER_HPP_
#define _AU_POOLING_LAYER_HPP_

#define PORT_WIDTH		128//64
#define BRAM_SETS		8//4
#define MEM_BYTES		16//8
#define I_BRAM_DEPTH	(1024*2)
#define O_BRAM_DEPTH	(1024*1)
#define F_BITS			4

#define MIN(x, y)	((x)<(y) ? (x) : (y))

//# Registering function
template<typename T>
T readRegisters(T &in)
{
#pragma HLS interface register port=return
	T x = in;
	return x;
}

template<int SETS, int DEPTH>
void readData(GMEM_MAXPOOLTYPE *in1, GMEM_MAXPOOLTYPE *in2, short iBuff1[SETS][DEPTH], short iBuff2[SETS][DEPTH], short iDim)
{
#pragma HLS inline off
	ap_uint<PORT_WIDTH> iReg1[1],iReg2[1];
#if FILE_OUTPUT
	fprintf(stderr, "** readData: iDim = %d\n", iDim);
#endif
	IN_RD:
	for(short i = 0; i < iDim; i++)
	{
#pragma HLS loop_tripcount min=8800 max=15800
#pragma HLS pipeline II=1
#pragma HLS loop_flatten off

		memcpy(iReg1, in1+i, MEM_BYTES);
		memcpy(iReg2, in2+i, MEM_BYTES);

		UNPACK1:
		for(int k = 0, bit = 0; k < BRAM_SETS; k++, bit+=16)
		{
#pragma HLS unroll
			iBuff1[k][i] = iReg1[0].range(bit+15, bit);
			iBuff2[k][i] = iReg2[0].range(bit+15, bit);
		}
	}//# IN_RD
#if FILE_OUTPUT
	fprintf(stderr, "** readData: End\n");
#endif
}
//#readData

template<int SETS, int DEPTH>
void writeData(GMEM_MAXPOOLTYPE *out1, GMEM_MAXPOOLTYPE *out2, short oBuff1[SETS][DEPTH], short oBuff2[SETS][DEPTH], short oDim, bool wflag)
{
#pragma HLS inline off
	ap_uint<PORT_WIDTH> oReg1[1], oReg2[1];
	if(wflag)
	{
#if FILE_OUTPUT
		fprintf(stderr, "** writeData: oDim = %d\n", oDim);
#endif
		OUT_WR:
		for(short i = 0; i < oDim; i++)
		{
#pragma HLS loop_tripcount min=2200 max=3900
#pragma HLS pipeline II=1
#pragma HLS loop_flatten off
			PACK1:
			for(int k = 0, bit = 0; k < BRAM_SETS; k++, bit+=16)
			{
#pragma HLS unroll
				oReg1[0].range(bit+15, bit) = oBuff1[k][i+1];
				oReg2[0].range(bit+15, bit) = oBuff2[k][i+1];
			}
			memcpy(out1+i, oReg1, MEM_BYTES);
			memcpy(out2+i, oReg2, MEM_BYTES);
		}//# OUT_WR
#if FILE_OUTPUT
		fprintf(stderr, "** writeData: End\n");
#endif
	}
}
//# writeData

template <int iDEPTH, int oDEPTH, int TM>
void PoolGeneral(
		short iBuff1[TM][iDEPTH], short oBuff1[TM][oDEPTH],
		short iBuff2[TM][iDEPTH], short oBuff2[TM][oDEPTH],
		int lBound, short inh, short inw, short outh, short outw,
		char pw_h, char pw_w, char ps_h, char ps_w,
		bool flag, bool avg_pool,
		unsigned char one_by_diviser, unsigned char pool,
		short row_init, short row_end, short half_rows,
		unsigned char pad
)
{
#pragma HLS inline

	short max1[TM];
#pragma HLS ARRAY_PARTITION variable=max1 complete dim=1
	short max2[TM];
#pragma HLS ARRAY_PARTITION variable=max2 complete dim=1
	short arr1[TM];
#pragma HLS ARRAY_PARTITION variable=arr1 complete dim=1
	short arr2[TM];
#pragma HLS ARRAY_PARTITION variable=arr2 complete dim=1
	ap_int<24> sum1[TM];
#pragma HLS ARRAY_PARTITION variable=sum1 complete dim=1
	ap_int<24> sum2[TM];
#pragma HLS ARRAY_PARTITION variable=sum2 complete dim=1

	short max3[TM];
#pragma HLS ARRAY_PARTITION variable=max3 complete dim=1
	short max4[TM];
#pragma HLS ARRAY_PARTITION variable=max4 complete dim=1
	short arr3[TM];
#pragma HLS ARRAY_PARTITION variable=arr3 complete dim=1
	short arr4[TM];
#pragma HLS ARRAY_PARTITION variable=arr4 complete dim=1
	ap_int<24> sum3[TM];
#pragma HLS ARRAY_PARTITION variable=sum3 complete dim=1
	ap_int<24> sum4[TM];
#pragma HLS ARRAY_PARTITION variable=sum4 complete dim=1

	INIT1:
	for(int i = 0; i < TM; i++)
	{
		if(pad){
			max1[i] = max2[i] = 0x0;
			max3[i] = max4[i] = 0x0;
		} else {
			max1[i] = max2[i] = 0x8000;
			max3[i] = max4[i] = 0x8000;
		}
		sum1[i] = sum2[i] = 0;
		sum3[i] = sum4[i] = 0;
	}

	char pool_h = pw_h;
	char pool_w = pw_w;

	//# Start Rows
	short row1 = -ps_h-pad;
	short row2 = readRegisters(row_init)-pad;

	int lB = readRegisters(lBound);
#if FILE_OUTPUT
	fprintf(stderr, "Row Init = %d ## %d \tBound = %d\n", row1+ps_h, row2+ps_h, lBound);
#endif

	//# Indices
	int k = 0, oidx1 = 0, oidx2 = half_rows;

	short idx1 = 0, idx2 = 0;

	short col1 = 0;
	short col2 = 0;

	char r1 = 0, c1 = 0;
	char r2 = 0, c2 = 0;

	int boundcounter = 0;

	char pool_cnt = 0;
	short out_cnt = row_end;
	POOLING1:
	for(k = 0; k < lB/2; k++)
	{
#pragma HLS loop_tripcount min=9900 max=17550
#pragma HLS pipeline
#pragma HLS dependence variable=oBuff1 intra false
#pragma HLS dependence variable=oBuff2 intra false
		if(c1 == pool_w)
		{
			r1++; c1 = 0;
			r2++; c2 = 0;
		}

		if(r1 == pool_h)
		{
			col1 += ps_w; r1 = 0;
			col2 += ps_w; r2 = 0;
		}

		if(out_cnt == row_end)
		{
			row1 += ps_h; col1 = 0;
			row2 += ps_h; col2 = 0;
			out_cnt = 0;
		}

		short colidx1 = c1+col1-pad;
		short rowidx1 = r1+row1;
		short colidx2 = c2+col2-pad;
		short rowidx2 = r2+row2;

		//		fprintf(stderr, "rowidx = %hd colidx = %hd\n", rowidx, colidx);

		idx1 = (rowidx1)*inw + colidx1;
		idx2 = (rowidx2)*inw + colidx2;

		if(pool_cnt == pool)
		{
			oidx1++; oidx2++;
			WR_2_BRAM1:
			for(int i = 0 ; i < TM; i++)
			{
				if(avg_pool)
				{
					oBuff1[i][oidx1] = (sum1[i] * one_by_diviser) >> 8;
					sum1[i] = iBuff1[i][idx1];
					oBuff1[i][oidx2] = (sum2[i] * one_by_diviser) >> 8;
					sum2[i] = iBuff1[i][idx2];

					oBuff2[i][oidx1] = (sum3[i] * one_by_diviser) >> 8;
					sum3[i] = iBuff2[i][idx1];
					oBuff2[i][oidx2] = (sum4[i] * one_by_diviser) >> 8;
					sum4[i] = iBuff2[i][idx2];
				}
				else
				{
					oBuff1[i][oidx1] = max1[i];
					oBuff1[i][oidx2] = max2[i];
					oBuff2[i][oidx1] = max3[i];
					oBuff2[i][oidx2] = max4[i];
//					if(i == 1)
//						fprintf(stderr, "%hd\tx\t%hd\t%hd\n", rowidx1, colidx1, max1[i]);

					if (colidx1 < 0 || rowidx1 < 0){
						max1[i] = max3[i] = 0;
					} else {
						max1[i] = iBuff1[i][idx1];
						max3[i] = iBuff2[i][idx1];
					}
					if (colidx2 < 0 || rowidx2 < 0){
						max2[i] = max4[i] = 0;
					} else {
						max2[i] = iBuff1[i][idx2];
						max4[i] = iBuff2[i][idx2];
					}
				}
			}
			pool_cnt = 0;
		}
		else
		{

			RD_4RM_BRAM1:
			for(int i = 0 ; i < TM; i++)
			{
				if (colidx1 < 0 || rowidx1 < 0){
					arr1[i] = arr3[i] = 0;
				} else {
					arr1[i] = iBuff1[i][idx1];
					arr3[i] = iBuff2[i][idx1];
				}
				if (colidx2 < 0 || rowidx2 < 0){
					arr2[i] = arr4[i] = 0;
				} else {
					arr2[i] = iBuff1[i][idx2];
					arr4[i] = iBuff2[i][idx2];
				}

//				if(i == 1)
//					fprintf(stderr, "%hd\tx\t%hd\t%hd\n", rowidx1, colidx1, arr1[i]);

			}

			if((colidx1 >= inw) || (rowidx1) >= inh)
			{
				for(int i = 0 ; i < TM; i++)
				{
					if(avg_pool){
						arr1[i] = arr3[i] = 0;
					} else {
						arr1[i] = 0x8000;
						arr3[i] = 0x8000;
					}
				}
			}
			//if((colidx2 >= inw) || (rowidx2-pad) >= inh)
			if((colidx2 >= inw) || (rowidx2) >= inh)
			{
				for(int i = 0 ; i < TM; i++)
				{
					if(avg_pool){
						arr2[i] = arr4[i] = 0;
					} else {
						arr2[i] = 0x8000;
						arr4[i] = 0x8000;
					}
				}
			}
			PROC1:
			for(int i = 0 ; i < TM; i++)
			{
				if(avg_pool) {
					sum1[i] += arr1[i];
					sum2[i] += arr2[i];
					sum3[i] += arr3[i];
					sum4[i] += arr4[i];
				} else {
					max1[i] = max1[i] > arr1[i] ? max1[i] : arr1[i];
					max2[i] = max2[i] > arr2[i] ? max2[i] : arr2[i];
					max3[i] = max3[i] > arr3[i] ? max3[i] : arr3[i];
					max4[i] = max4[i] > arr4[i] ? max4[i] : arr4[i];
				}
			}
		}
		c1++; c2++;
		pool_cnt++; out_cnt++;
	}
	oidx1++; oidx2++;
	WR_2_BRAM2:
	for(int i = 0 ; i < TM; i++)
	{
		if(avg_pool){
			oBuff1[i][oidx1] = (sum1[i] * one_by_diviser) >> 8;
			oBuff1[i][oidx2] = (sum2[i] * one_by_diviser) >> 8;
			oBuff2[i][oidx1] = (sum3[i] * one_by_diviser) >> 8;
			oBuff2[i][oidx2] = (sum4[i] * one_by_diviser) >> 8;
		} else {
			oBuff1[i][oidx1] = max1[i];
			oBuff1[i][oidx2] = max2[i];
			oBuff2[i][oidx1] = max3[i];
			oBuff2[i][oidx2] = max4[i];
		}
	}



	if(flag)
	{
#if FILE_OUTPUT
		fprintf(stderr, "** OutH x OutW = %hd x %hd\n", outh, outw);
#endif
		col2  = 0; c2 = 0; r2 = 0;

//		fprintf(stderr, "Boundary Counter = %d pad = %d\n", boundcounter, pad);

		out_cnt = row_end;
		pool_cnt = 0;
		INIT2:
		for(int i = 0; i < TM; i++)
		{
			if(pad){
				max2[i] = 0x0;
				max4[i] = 0x0;
			} else {
				max2[i] = 0x8000;
				max4[i] = 0x8000;
			}
			sum2[i] = sum4[i] = 0;
		}
		if(lBound == 0) oidx2 = half_rows;
		POOLING2:
		for(k = 0; k < row_end; k++)
		{
#pragma HLS pipeline
#pragma HLS loop_tripcount min=900 max=900

			if(c2 == pool_w)
			{
				r2++; c2 = 0;
			}

			if(r2 == pool_h)
			{
				col2 += ps_w; r2 = 0;
			}

			if(out_cnt == row_end)
			{
				row2 += ps_h; col2 = 0;
				out_cnt = 0;
			}
			short colidx2 = c2+col2-pad;
			short rowidx2 = r2+row2;

			idx2 = (rowidx2)*inw + (colidx2);

			if(pool_cnt == pool)
			{
				oidx2++;
				WR_2_BRAM3:
				for(int i = 0 ; i < TM; i++)
				{
					if(avg_pool)
					{
						oBuff1[i][oidx2] = (sum2[i] * one_by_diviser) >> 8;
						sum2[i] = iBuff1[i][idx2];

						oBuff2[i][oidx2] = (sum4[i] * one_by_diviser) >> 8;
						sum4[i] = iBuff2[i][idx2];

					}
					else
					{
						oBuff1[i][oidx2] = max2[i];
						oBuff2[i][oidx2] = max4[i];
						if (colidx2 < 0 || rowidx2 < 0){
							max2[i] = max4[i] = 0;
						} else {
							max2[i] = iBuff1[i][idx2];
							max4[i] = iBuff2[i][idx2];
						}
					}
				}
				pool_cnt = 0;
			}
			else
			{
				RD_4RM_BRAM2:
				for(int i = 0 ; i < TM; i++)
				{
					if (colidx2 < 0 || rowidx2 < 0){
						arr2[i] = arr4[i] = 0;
					} else {
						arr2[i] = iBuff1[i][idx2];
						arr4[i] = iBuff2[i][idx2];
					}
				}

				if((colidx2 >= inw) || (rowidx2) >= inh)
                                {
					for(int i = 0 ; i < TM; i++)
					{
						if(avg_pool){
							arr2[i] = arr4[i] = 0;
						} else {
							arr2[i] = 0x8000;
							arr4[i] = 0x8000;
						}
					}
				}
				PROC2:
				for(int i = 0 ; i < TM; i++)
				{
					if(avg_pool) {
						sum2[i] += arr2[i];
						sum4[i] += arr4[i];
					} else {
						max2[i] = max2[i] > arr2[i] ? max2[i] : arr2[i];
						max4[i] = max4[i] > arr4[i] ? max4[i] : arr4[i];
					}
				}
			}
			c2++;
			pool_cnt++; out_cnt++;
		}
		oidx2++;
		WR_2_BRAM4:
		for(int i = 0 ; i < TM; i++)
		{
			if(avg_pool){
				oBuff1[i][oidx2] = (sum2[i] * one_by_diviser) >> 8;
				oBuff2[i][oidx2] = (sum4[i] * one_by_diviser) >> 8;
			} else {
				oBuff1[i][oidx2] = max2[i];
				oBuff2[i][oidx2] = max4[i];
			}
		}
	}
#if DEBUG
	bool bramflag = true;
	fprintf(stderr, "** OutH x OutW = %hd x %hd \n", outh, outw);
	for(int x = 0, idx = 0; x < outh*outw; x++, idx++)
	{
		fprintf(pFp, "%hd\n", oBuff1[0][idx+1]);
	}
#endif
}
//#PoolGeneral

template <int TM, int iDEPTH, int oDEPTH>
void poolingLayer(
		short iBuff1[TM][iDEPTH], short oBuff1[TM][oDEPTH],
		short iBuff2[TM][iDEPTH], short oBuff2[TM][oDEPTH],
		short inh, short inw, short outh, short width,
		char pw_h, char pw_w, char ps_h, char ps_w,
		bool avg_pool, unsigned char one_by_diviser,
		unsigned char pool, short half_rows,
		unsigned char pad
)
{
#pragma HLS inline off

	short row_end = readRegisters(width);
	short out_h   = readRegisters(outh);

	int lBound = 0;
	bool flag;
	short temp;

	if(outh & 0x1) {
		temp = (out_h-1);  flag = true;
	} else {
		temp = out_h;  	   flag = false;
	}

	lBound = temp * row_end;
	short row_init = (out_h / 2) * ps_h - ps_h;

	unsigned char psh[TM], psw[TM], poolsz[TM];
	for(ap_uint<4> i = 0; i < TM; i++)
	{
#pragma HLS unroll
		psh[i]    = readRegisters(ps_h);
		psw[i]    = readRegisters(ps_w);
		poolsz[i] = readRegisters(pool);
	}

	PoolGeneral<iDEPTH, oDEPTH, TM>
	(iBuff1, oBuff1, iBuff2, oBuff2, lBound, inh, inw, outh, 27, pw_h, pw_w, ps_h, ps_w, flag, avg_pool, one_by_diviser, pool, row_init, row_end, half_rows, pad);

}
//#poolingLayer


//# Ping Pong between memory access and processing
void PlanesPingPong(GMEM_MAXPOOLTYPE *poolin1, GMEM_MAXPOOLTYPE *poolin2,
					GMEM_MAXPOOLTYPE *poolout1, GMEM_MAXPOOLTYPE *poolout2,
					int offset_in, int offset_out,
					int tiDim, int toDim, int pp_loop_idx, short in_height, short in_width, short width,
					short mORows, unsigned char pool, short half_rows,
					short ps_h, short ps_w, short pw_h, short pw_w, unsigned char avg_pool,
					unsigned char pad, unsigned char one_by_diviser,
					short iDim, short oDim, short planes)
{
#pragma HLS inline off
#pragma HLS interface register port=tiDim
#pragma HLS interface register port=toDim
#pragma HLS interface register port=pp_loop_idx
#pragma HLS interface register port=in_height
#pragma HLS interface register port=in_width
#pragma HLS interface register port=width
#pragma HLS interface register port=planes
#pragma HLS interface register port=iDim
#pragma HLS interface register port=oDim
#pragma HLS interface register port=offset_in
#pragma HLS interface register port=offset_out

	//# Input BRAMs: Set-1
	short i_buff_1[BRAM_SETS][I_BRAM_DEPTH];
#pragma HLS array_partition variable=i_buff_1 dim=1
#pragma HLS resource variable=i_buff_1 core=RAM_T2P_BRAM

	short i_buff_2[BRAM_SETS][I_BRAM_DEPTH];
#pragma HLS array_partition variable=i_buff_2 dim=1
#pragma HLS resource variable=i_buff_2 core=RAM_T2P_BRAM

	//# Input BRAMs: Set-2
	short i_buff_3[BRAM_SETS][I_BRAM_DEPTH];
#pragma HLS array_partition variable=i_buff_3 dim=1
#pragma HLS resource variable=i_buff_3 core=RAM_T2P_BRAM

	short i_buff_4[BRAM_SETS][I_BRAM_DEPTH];
#pragma HLS array_partition variable=i_buff_4 dim=1
#pragma HLS resource variable=i_buff_4 core=RAM_T2P_BRAM

	//# Output BRAMs: Set-1
	short o_buff_1[BRAM_SETS][O_BRAM_DEPTH];
#pragma HLS array_partition variable=o_buff_1 dim=1
#pragma HLS resource variable=o_buff_1 core=RAM_T2P_BRAM

	short o_buff_2[BRAM_SETS][O_BRAM_DEPTH];
#pragma HLS array_partition variable=o_buff_2 dim=1
#pragma HLS resource variable=o_buff_2 core=RAM_T2P_BRAM

	//# Output BRAMs: Set-2
	short o_buff_3[BRAM_SETS][O_BRAM_DEPTH];
#pragma HLS array_partition variable=o_buff_3 dim=1
#pragma HLS resource variable=o_buff_3 core=RAM_T2P_BRAM

	short o_buff_4[BRAM_SETS][O_BRAM_DEPTH];
#pragma HLS array_partition variable=o_buff_4 dim=1
#pragma HLS resource variable=o_buff_4 core=RAM_T2P_BRAM

	//# i/p & o/p plane indices
	int i_plane_idx = 0, o_plane_idx = 0;

	bool flag  = true; 						// Ping-Pong flag
	bool wflag = false;						// Write o/p flag

	//# Read Input Data
	readData<BRAM_SETS, I_BRAM_DEPTH>(poolin1+i_plane_idx+offset_in, poolin2+i_plane_idx+offset_in, i_buff_1, i_buff_3, iDim);

	PLANES:
	for(int np = 0; np < planes; np+=BRAM_SETS)
	{
#pragma HLS loop_tripcount min=1 max=256
#pragma HLS loop_flatten off

		//# Next Plane Index
		i_plane_idx += tiDim;

#if FILE_OUTPUT
	fprintf(stderr, "\n** Plane Offset IN = %d\n", i_plane_idx);
#endif

		if(flag)
		{
			//# Pooling Layer
			poolingLayer<BRAM_SETS, I_BRAM_DEPTH, O_BRAM_DEPTH>
			(i_buff_1, o_buff_1, i_buff_3, o_buff_3, in_height-pp_loop_idx, in_width, mORows, width, pw_h, pw_w, ps_h, ps_w, avg_pool, one_by_diviser, pool, half_rows, pad);
			//# Write Output Data
			writeData<BRAM_SETS, O_BRAM_DEPTH>(poolout1+o_plane_idx+offset_out, poolout2+o_plane_idx+offset_out, o_buff_2, o_buff_4, oDim, wflag);
			//# Read Input Data
			readData<BRAM_SETS, I_BRAM_DEPTH>(poolin1+i_plane_idx+offset_in, poolin2+i_plane_idx+offset_in, i_buff_2, i_buff_4, iDim);
			flag = false;
		}
		else
		{
			//# Pooling Layer
			poolingLayer<BRAM_SETS, I_BRAM_DEPTH, O_BRAM_DEPTH>
			(i_buff_2, o_buff_2, i_buff_4, o_buff_4, in_height-pp_loop_idx, in_width, mORows, width, pw_h, pw_w, ps_h, ps_w, avg_pool, one_by_diviser, pool, half_rows, pad);
			//# Write Output Data
			writeData<BRAM_SETS, O_BRAM_DEPTH>(poolout1+o_plane_idx+offset_out, poolout2+o_plane_idx+offset_out, o_buff_1, o_buff_3, oDim, wflag);
			//# Read Input Data
			readData<BRAM_SETS, I_BRAM_DEPTH>(poolin1+i_plane_idx+offset_in, poolin2+i_plane_idx+offset_in, i_buff_1, i_buff_3, iDim);
			flag = true;
		}
		if(wflag){
			o_plane_idx += toDim;
		}
		wflag = true;
	}
	if(flag)
	{
		//#  Write Output Data
		writeData<BRAM_SETS, O_BRAM_DEPTH>(poolout1+o_plane_idx+offset_out, poolout2+o_plane_idx+offset_out, o_buff_2, o_buff_4, oDim, wflag);

		//# Pooling Layer
		poolingLayer<BRAM_SETS, I_BRAM_DEPTH, O_BRAM_DEPTH>
		(i_buff_1, o_buff_1, i_buff_3, o_buff_3, in_height-pp_loop_idx, in_width, mORows, width, pw_h, pw_w, ps_h, ps_w, avg_pool, one_by_diviser, pool, half_rows, pad);

		if(wflag){
			o_plane_idx += toDim;
		}
		//# Write Output Data
		writeData<BRAM_SETS, O_BRAM_DEPTH>(poolout1+o_plane_idx+offset_out, poolout2+o_plane_idx+offset_out, o_buff_1, o_buff_3, oDim, true);
	}
	else
	{
		//# Write Output Data
		writeData<BRAM_SETS, O_BRAM_DEPTH>(poolout1+o_plane_idx+offset_out, poolout2+o_plane_idx+offset_out, o_buff_1, o_buff_3, oDim, wflag);

		//# Pooling Layer
		poolingLayer<BRAM_SETS, I_BRAM_DEPTH, O_BRAM_DEPTH>
		(i_buff_2, o_buff_2, i_buff_4, o_buff_4, in_height-pp_loop_idx, in_width, mORows, width, pw_h, pw_w, ps_h, ps_w, avg_pool, one_by_diviser, pool, half_rows, pad);

		if(wflag){
			o_plane_idx += toDim;
		}
		//#  Write Output Data
		writeData<BRAM_SETS, O_BRAM_DEPTH>(poolout1+o_plane_idx+offset_out, poolout2+o_plane_idx+offset_out, o_buff_2, o_buff_4, oDim, true);
	}
}


//# Pool Kernel Function
void poolKernel(
		GMEM_MAXPOOLTYPE *poolin1, GMEM_MAXPOOLTYPE *poolin2,
		GMEM_MAXPOOLTYPE *poolout1, GMEM_MAXPOOLTYPE *poolout2,
		short in_h, short in_w, short out_h,  short out_w,  short n_planes,
		short ps_h, short ps_w, short pwin_h, short pwin_w, unsigned char avg_pool,
		unsigned char pad, unsigned char one_by_diviser
)
{
#pragma HLS inline off

	//# Register Reads
	short in_height  = readRegisters(in_h);
	short in_width   = readRegisters(in_w);

	short out_height = readRegisters(out_h);
	short out_width  = readRegisters(out_w);

	short pw_h = readRegisters(pwin_h);
	short pw_w = readRegisters(pwin_w);

	short planes = readRegisters(n_planes);

#if FILE_OUTPUT
	iFp = fopen("poolInput.txt", "w");
	oFp = fopen("poolOutput.txt", "w");
#endif

	int tiDim = in_height * in_width;
	int toDim = out_height * out_width;


#if DEBUG
	pFp = fopen("ProcessedOut.txt", "w");
	wFp = fopen("WrittenOut.txt", "w");
	apFp = fopen("AfterProcess.txt", "w");
#endif

	unsigned char pool =  pw_h * pw_w;


	short n_irows = I_BRAM_DEPTH / in_width;
	short n_orows = (n_irows / ps_h) - 1;

	short temp = (n_orows * ps_h) + pwin_h-ps_h;

	//# Maxmum i/p & o/p rows that can fit into BRAMs
	short mIRows = (MIN(temp, in_height));
	short mORows = (MIN(n_orows, out_height));

	//# Initial size for data read & write
	short iDim = mIRows * in_width;
	short oDim = mORows * out_width;

	//# Initial Row Offsets
	int irow_offset = 0;
	int orow_offset = 0;

	int offset_in = 0, offset_out = 0;		// Offset register

	//# Step for next fetch
	short step;
	if(mORows == out_height)
		step = in_height;
	else
		step = mORows * ps_h;

	short half_rows;

	half_rows = ((mORows & 0xFFFE) * out_width) >> 1;

	short pp_loop_bound = in_height;
	int pp_loop_idx = 0;
	short width = pool * out_width;

#if FILE_OUTPUT
		fprintf(stderr, "\n** Ping Pong Bound = %d step = %d \n", pp_loop_bound/step, step);
#endif

	planes = planes - BRAM_SETS;
	RESOLUTION_LOOP:
	for(pp_loop_idx = 0; pp_loop_idx < pp_loop_bound; pp_loop_idx+=step)
	{
#pragma HLS loop_tripcount min=1 max=256
//#pragma HLS loop_flatten off

#if FILE_OUTPUT
		fprintf(stderr, "\n************************* Resolution Offset IN  = %d Rows to be read = %d\n", offset_in, mIRows);
		fprintf(stderr, "************************* Resolution Offset OUT = %d Rows to be written = %d\n", offset_out, mORows);
		fprintf(stderr, "************************* Half rows = %d\n", half_rows);
#endif
#if FILE_OUTPUT
		fprintf(stderr, "\n** Plane Offset IN  = %d\n", i_plane_idx);
		fprintf(stderr, "** Plane Offset OUT = %d\n", o_plane_idx);
#endif

		PlanesPingPong(
				poolin1, poolin2, poolout1, poolout2, offset_in, offset_out,
				tiDim, toDim, pp_loop_idx, in_height, in_width, width,
				mORows, pool, half_rows,
				ps_h, ps_w, pw_h, pw_w, avg_pool,
				pad, one_by_diviser,
				iDim, oDim, planes);

		//# Next i/p offset
		irow_offset += step;
		offset_in = irow_offset * in_width;
		mIRows = MIN(mIRows, in_height-irow_offset);
		iDim   = mIRows * in_width;

		//# Rows completed (Next Row Offset)
		orow_offset += mORows;
		//# Minimum of remaining rows & number of o/p rows to compute per iteration
		mORows = (MIN(mORows, out_height-orow_offset));
		//# Total elements to be written in next iteration
		oDim   = mORows * out_width;
		//# Next o/p offset
		offset_out = orow_offset*out_width;
#if FILE_OUTPUT
		fprintf(stderr, "\n** Output Offset = %d mORows = %d out rows offset = %d\n", offset_out, mORows, orow_offset);
#endif
		half_rows = ((mORows & 0xFFFE) * out_width) >> 1;
	}

#if DEBUG
	fclose(pFp);
	fclose(wFp);
	fclose(apFp);
#endif
#if FILE_OUTPUT
	fclose(iFp); iFp = NULL;
	fclose(oFp); oFp = NULL;
#endif
	return;
}



#endif //_AU_POOLING_LAYER_HPP_
