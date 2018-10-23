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

#ifdef __SDSOC
#include <cblas.h>
#else
#include <include/cblas.h>
#endif


#ifdef __SDSOC
#include "sds_lib.h"
#endif

//# Performance check
#ifdef __SDSOC
long long int clock_start1, clock_end1, frequency1;
#define TIME_STAMP_INIT  clock_start1 = sds_clock_counter();
#define TIME_STAMP  { \
		clock_end1 = sds_clock_counter(); \
		frequency1 = sds_clock_frequency(); \
}
#endif

using namespace std;

#define ASSERT(condition, code, message)   \
		if(!(condition))                     \
		{                                   \
			cerr << "[" << #code << "] ";    \
			cerr << message << endl;        \
			exit(-1);                       \
		}

vector<float> readtxt1(const string& filename);
void savetxt1(const vector<float>& data, const string& filepath);
vector<float> getArray(int R, int C, int divisor);

// Matrix multiplication
// X - input matrix shape : m x n
// Y - input matrix shape : n x o  (Basically weights)
// B - input matrix shape : 1 x n  (Basically Bias values)
// Z - Output matrix shape : m x o
void matrix_multiply(float* X, float* Y, float* Z, int M, int N, int O);
void matrix_multiply2(float* X, float* Y, float* Z, int M, int N, int O);
void matrix_multiplyT(float* X, float* Y, float* B, float* Z, int M, int N, int O);

void matrix_multiply(float* X, float* Y, float* Z, int M, int N, int O)
{
	for(int m=0; m < M; ++m) {
		for(int o=0; o < O; ++o) {
			Z[m*O + o] = 0.0f;
			for(int n=0; n<N; ++n) {
				Z[m*O + o] += X[m*N + n] * Y[n*O + o];
			}
		}
	}
}

void matrix_multiply2(float* X, float* Y, float* B, float* Z, int M, int N, int O)
{
	for(int i=0; i<M; ++i)
		memcpy(Z+i, B, O*sizeof(float));

	for(int m=0; m < M; ++m) {
		for(int n=0; n<N; ++n) {
			float xval = X[m*N + n];
			for(int o=0; o < O; ++o) {
				Z[m*O + o] += xval * Y[n*O + o];
			}
		}
	}
}

void reluWrapper(float* in, float* out, int *scalar_fc_args)//M, int N, int O)
{

	int out_planes = scalar_fc_args[2]*scalar_fc_args[3];

	for(int out_id=0; out_id < out_planes; ++out_id)
	{
		float in_val = in[out_id];

		if(in_val < 0)   //check if negative
		{
			in_val = 0;
		}

		out[out_id] = in_val;

	}


}

void computeFC(float *weight, float *bias, float *data, 
		int M, int N, int K, float *output)
{
	int lda = K;
	int ldb = K;
	if(M<4) {
		for(int mi=0; mi<M; ++mi) {
			cblas_sgemv(CblasRowMajor, CblasNoTrans,
					N, K, 1.0,
					weight, lda,        /*weights*/
					data + mi*K, /*incx*/ 1, 0.0,   /*input*/
					output + mi*N, /*incy*/ 1);       /*output*/
		}
	}
	else {
		cblas_sgemm(CblasRowMajor,
				(CBLAS_TRANSPOSE)CblasNoTrans, (CBLAS_TRANSPOSE)CblasTrans,
				M, N, K, 1.,
				data, lda, weight, ldb, 0., output, N);
	}

	// bias is not transposed
	lda = 1;
	ldb = N;
	std::vector<float> bias_multiplier(M, 1.);
	cblas_sgemm(CblasRowMajor, 
			(CBLAS_TRANSPOSE)CblasNoTrans, (CBLAS_TRANSPOSE)CblasNoTrans,
			M, N, 1, 1.,
			&(bias_multiplier[0]), lda, bias, ldb, 1., output, N);
}


// M - Batch Size (Eg : typically 1)
// N - Input feature size (Eg : 1024 for googlenet FC)
// O - Output feature size (Eg : 1000 for googlenet FC)

// X - input matrix, shape : M X N
// Y - input matrix, shape : N X O  (Basically weights)
// B - input matrix, shape : 1 X O  (Basically Bias)
// Z - Output matrix, shape : M X O

// NB: Caffe stores the weights in OxN format. So internally we transpose it to NxO format.
// NB: This could be done offline one-time for performance gain.

//void matrix_multiplyT(float* X, float* Y, float* B, float* Z, int M, int N, int O)
//void SwFcWrapper(float* X, float* Y, float* B, float* Z, int *scalar_fc_args)//M, int N, int O)
void SwFcWrapper(float* X, float* WT, float* B, float* Z, int *scalar_fc_args)//M, int N, int O)
{

	int batch = scalar_fc_args[0]*scalar_fc_args[3];  //BATCH_SIZE - scalar_fc_args[3]
	int input_units = scalar_fc_args[1];
	int output_units = scalar_fc_args[2];

	int M = batch;
	int N = output_units;
	int K = input_units;

#if 0
	if(scalar_fc_args[5]==11)
	{
		FILE *frefout=fopen("fc6_input.txt","w");
		if(frefout == NULL)
		{
			//std :: cout << "File not found - " << ref_path << std :: endl;
			//return -1;
			printf("file open error");
		}

		for(int i=0; i< M*K;i++)
		{
			fprintf(frefout, "%f\n", X[i]);
		}
		fclose(frefout);
	}
#endif

#if 0
	//if(scalar_fc_args[5]==1)
	{
		//const string xfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/GoogleNetWithoutLRN_New/caffe_ref/pool5_7x7_s1_sp.txt";
		//const string xfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/AlexNet-8bit/caffe_ref/fc6_out.txt";//pool5_sp.txt";
		const string xfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/dynamicfixed_models/AlexNet_8Bit_withRounding/caffe_ref/pool5_out.txt";//pool5_sp.txt";
		vector<float> vX = readtxt1(xfile);
		//X = &(vX[0]);
#if 1
		for(int i=0; i<scalar_fc_args[1]; i++)
		{
			X[i] = vX[i];
			//if(X[i] != vX[i])
			//fprintf(stderr, "i=%d vX[i]=%f X[i]=%f \n", i, vX[i], X[i]);
		}

		//Batch 2 input
		for(int i=scalar_fc_args[1]; i<scalar_fc_args[1]*2; i++)
		{
			X[i] = vX[i-scalar_fc_args[1]];
			//if(X[i] != vX[i-scalar_fc_args[1]])
			//fprintf(stderr, "i=%d vX[i]=%f X[i]=%f \n", i, vX[i-scalar_fc_args[1]], X[i]);
		}
#endif

#if 0
		//const string yfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/GoogleNetWithoutLRN_New/weightsTrimmed/loss3_classifier_wts.txt";
		const string yfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/AlexNet-8bit/weightsTrimmed/fc7_wts.txt";
		vector<float> vY = readtxt1(yfile);
		//WT = &(vY[0]);

		//const string bfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/GoogleNetWithoutLRN_New/weightsTrimmed/loss3_classifier_bias.txt";
		const string bfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/AlexNet-8bit/weightsTrimmed/fc7_bias.txt";
		vector<float> vB = readtxt1(bfile);
		//B = &(vB[0]);

		vector<float> WT1; WT1.resize(N*O);
		for(int i=0; i<O; ++i) {
			for(int j=0; j<N; ++j) {
				WT1[j*O + i] = vY[i*N + j];
			}
		}
		//WT = &(WT1[0]);

		for(int i=0; i<scalar_fc_args[1] * scalar_fc_args[2]; i++)
			//for(int i=0; i<20; i++)
		{
			//X[i] = vX[i];
			if(WT[i] != WT1[i])
				fprintf(stderr, "i=%d WT[i]=%f WT1[i]=%f \n", i, WT[i], WT1[i]);
		}

		for(int i=0; i<scalar_fc_args[2]; i++)
			//for(int i=0; i<20; i++)
		{
			//X[i] = vX[i];
			if(B[i] != vB[i])
				fprintf(stderr, "i=%d B[i]=%f vB[i]=%f \n", i, B[i], vB[i]);
		}
#endif
		//WT = &(WT1[0]);
	}
#endif


	//fprintf(stderr, "FC execution start\n");

	//cerr << "CBLAS Start :" << openblas_get_num_threads() << endl; // initially should be 4
	/*openblas_set_num_threads(1);
	cerr << "CBLAS running " << openblas_get_num_threads() << endl; // should be 1*/

	//openblas_set_num_threads(3);

	//vector<double> time;

	//for(int i = 0; i < 100; i++)
	{

#if 0// def __SDSOC
		TIME_STAMP_INIT
#endif
		computeFC(WT, B, X, M, N, K, Z);
#if 0// def __SDSOC
		TIME_STAMP
#endif

#if 0//def __SDSOC
		//# Total time for the API in Images/Second
		double tot_time = (((double)(clock_end1-clock_start1)/(double)frequency1*1000));
		cerr << "FC TIME : " << tot_time << endl;
		// time.push_back(tot_time);
#endif
	}

	/*cerr << "FIRST FIVE RUNS : ";
	std::copy(time.begin(), time.begin()+5, ostream_iterator<double>(cerr, " "));
	vector<double>::iterator abs_max_iter = std::max_element(time.begin(), time.end());
	cerr << " FC MAX TIME : " << std::abs(*abs_max_iter) << endl;
	vector<double>::iterator abs_min_iter = std::min_element(time.begin(), time.end());
	cerr << " FC MIN TIME : " << std::abs(*abs_min_iter) << endl;
	double sum_of_elems = std::accumulate(time.begin(), time.end(), 0);
	cerr << " FC AVG TIME : " << sum_of_elems/100.0 << endl;

	 */



	//fprintf(stderr, "FC execution ends\n");

#if 0
	if(scalar_fc_args[5]==1)
	{
		//const string yfile1 = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/GoogleNetWithoutLRN_New/caffe_ref/loss3_classifier_out.txt";
		const string yfile1 = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/AlexNet-8bit/caffe_ref/fc7_out.txt";
		vector<float> vY1 = readtxt1(yfile1);

		for(int i=0;i<O;i++)
		{
			if(vY1[i] != Z[i])
				fprintf(stderr, "i=%d vY[i]=%f Z[i]=%f \n", i, vY1[i], Z[i]);
		}

		//Batch 2
		for(int i=O;i<2*O;i++)
		{
			if(vY1[i-O] != Z[i])
				fprintf(stderr, "i=%d vY[i]=%f Z[i]=%f \n", i, vY1[i-O], Z[i]);
		}
	}
#endif
}



void SwFcWrapper2(float* X, float* WT, float* B, float* Z, int *scalar_fc_args)//M, int N, int O)
{

	int M = scalar_fc_args[0]*scalar_fc_args[3];  //BATCH_SIZE - scalar_fc_args[3]
	int N = scalar_fc_args[1];
	int O = scalar_fc_args[2];

#if 0
	if(scalar_fc_args[5]==1)
	{
		//const string xfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/GoogleNetWithoutLRN_New/caffe_ref/pool5_7x7_s1_sp.txt";
		const string xfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/AlexNet-8bit/caffe_ref/fc6_out.txt";//pool5_sp.txt";
		vector<float> vX = readtxt1(xfile);
		//X = &(vX[0]);
#if 1
		//const string yfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/GoogleNetWithoutLRN_New/weightsTrimmed/loss3_classifier_wts.txt";
		const string yfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/AlexNet-8bit/weightsTrimmed/fc7_wts.txt";
		vector<float> vY = readtxt1(yfile);
		//WT = &(vY[0]);

		//const string bfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/GoogleNetWithoutLRN_New/weightsTrimmed/loss3_classifier_bias.txt";
		const string bfile = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/AlexNet-8bit/weightsTrimmed/fc7_bias.txt";
		vector<float> vB = readtxt1(bfile);
		//B = &(vB[0]);
#endif
#if 0
		for(int i=0; i<scalar_fc_args[1]; i++)
		{
			X[i] = vX[i];
			if(X[i] != vX[i])
				fprintf(stderr, "i=%d vX[i]=%f X[i]=%f \n", i, vX[i], X[i]);
		}

		//Batch 2 input
		for(int i=scalar_fc_args[1]; i<scalar_fc_args[1]*2; i++)
		{
			X[i] = vX[i-scalar_fc_args[1]];
			if(X[i] != vX[i-scalar_fc_args[1]])
				fprintf(stderr, "i=%d vX[i]=%f X[i]=%f \n", i, vX[i-scalar_fc_args[1]], X[i]);
		}
#endif
		vector<float> WT1; WT1.resize(N*O);
		for(int i=0; i<O; ++i) {
			for(int j=0; j<N; ++j) {
				WT1[j*O + i] = vY[i*N + j];
			}
		}
		//WT = &(WT1[0]);

		for(int i=0; i<scalar_fc_args[1] * scalar_fc_args[2]; i++)
			//for(int i=0; i<20; i++)
		{
			//X[i] = vX[i];
			if(WT[i] != WT1[i])
				fprintf(stderr, "i=%d WT[i]=%f WT1[i]=%f \n", i, WT[i], WT1[i]);
		}

		for(int i=0; i<scalar_fc_args[2]; i++)
			//for(int i=0; i<20; i++)
		{
			//X[i] = vX[i];
			if(B[i] != vB[i])
				fprintf(stderr, "i=%d B[i]=%f vB[i]=%f \n", i, B[i], vB[i]);
		}

		//WT = &(WT1[0]);
	}
#endif


	//fprintf(stderr, "FC execution start\n");




	// Bias Addition - basically initialize the output with bias values
	for(int i=0; i<M*O; i+=O)
		memcpy(Z+i, B, O*sizeof(float));

#if 0

	// Transpose the weights.
	// This can be done offline in init side for performance improvement
	vector<float> WT; WT.resize(N*O);
	for(int i=0; i<O; ++i) {
		for(int j=0; j<N; ++j) {
			WT[j*O + i] = Y[i*N + j];
		}
	}

#endif
// Actual matrix multiplication
	for(int m=0; m < M; ++m) {
		for(int n=0; n<N; ++n) {
			float xval = X[m*N + n];
			for(int o=0; o < O; ++o) {
				Z[m*O + o] += xval * WT[n*O + o];
			}
		}
	}

	//fprintf(stderr, "FC execution ends\n");

#if 0
	if(scalar_fc_args[5]==1)
	{
		//const string yfile1 = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/GoogleNetWithoutLRN_New/caffe_ref/loss3_classifier_out.txt";
		const string yfile1 = "/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/AlexNet-8bit/caffe_ref/fc7_out.txt";
		vector<float> vY1 = readtxt1(yfile1);

		for(int i=0;i<O;i++)
		{
			if(vY1[i] != Z[i])
				fprintf(stderr, "i=%d vY[i]=%f Z[i]=%f \n", i, vY1[i], Z[i]);
		}

		//Batch 2
		for(int i=O;i<2*O;i++)
		{
			if(vY1[i-O] != Z[i])
				fprintf(stderr, "i=%d vY[i]=%f Z[i]=%f \n", i, vY1[i-O], Z[i]);
		}
	}
#endif
}

// A function to read a txt file to a vector<float>
vector<float> readtxt1(const string& filename)
				{
	std::ifstream input_file(filename.c_str());
	if(!input_file)
	{
		cerr << "[ERROR] Couldn't open" << filename << endl;
		exit(-1);
	}

	std::vector<float> v;
	std::copy(istream_iterator<float>(input_file), istream_iterator<float>(), back_inserter(v));
	input_file.close();
	return v;
				}

// A function to dump the vector<float> to a txt file   
void savetxt1(const vector<float>& data, const string& filepath)
{
	std::ofstream output_file(filepath.c_str());
	if(!output_file)
	{
		cerr << "[ERROR] Couldn't create " << filepath << endl;
		exit(-1);
	}

	std::ios::fmtflags oldFlags = output_file.flags();
	output_file.precision(10);
	std::ostream_iterator<float> output_iterator(output_file, "\n");
	std::copy(data.begin(), data.end(), output_iterator);
	output_file.flags(oldFlags);
	output_file.close();
}

vector<float> getArray(int R, int C, int divisor)
				{
	vector<float> tmp;
	tmp.reserve(R*C);
	int cnt = 0;
	for(int i=0; i<R; ++i) {
		for(int j=0; j<C; ++j) {
			tmp.push_back(cnt % divisor);
			cnt++;
		}
	}

	return tmp;
				}

