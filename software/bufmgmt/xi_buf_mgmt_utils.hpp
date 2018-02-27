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

#ifndef __XI_BUF_MGMT_UTILS_HPP__
#define __XI_BUF_MGMT_UTILS_HPP__

#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)

///////////////	Convolution Utils  //////////////////////
int convInputSize(int in_depth, int input_h, int input_w)
{
	int inDepth = AlignSize(in_depth, KER_PROC);
	int size = inDepth * input_h * input_w * sizeof(short);

	return(size);
}

int convBiasSize(int in_depth)
{
	int inDepth = AlignSize(in_depth, KER_PROC);
	int size = inDepth * sizeof(HCONV_BIAS_TYPE);

	return(size);
}

int deconvWgtSize(int input_n)
{
	int inputSize = AlignSize(input_n, 8);

	int tensor_size = inputSize * sizeof(short);

	return(tensor_size);
}


template<typename T, int align_bytes>
int shapeToSize( vector<vector<int> > shape)
{
#if EN_SHAPE2SIZE_PRINT
	cout << endl;
	cout << "shapeToSize : (dim1 size) - " << shape.size() << endl;
#endif

	int size = sizeof(T);

	for(unsigned int i = 0; i < shape.size(); i++)
	{
#if EN_SHAPE2SIZE_PRINT
		cout << "(1, d, h, w) : ";
#endif
		for(unsigned int j = 0; j < shape[i].size(); j++)
		{
			if(j == 1)
				size *= AlignSize(shape[i][j], align_bytes);
			else
				size *= shape[i][j];
#if EN_SHAPE2SIZE_PRINT
			cout << shape[i][j] << " ";
#endif
		}
#if EN_SHAPE2SIZE_PRINT
		cout << endl;
#endif
	}
#if EN_SHAPE2SIZE_PRINT
		cout << "size : " << size << endl;
#endif
	return size;
}

//# Computes the offset required for concatenated buffers
template<typename T, int align_bytes>
int peersToShape(vector < io_peers > shape)
{
#if EN_PEER2SIZE_PRINT
	cout << endl;
	cout << "peersToShape : (dim1 size) - " << shape.size() << endl;
#endif

	io_peers inPair;

	int f_size = 0;

	for(unsigned int i = 0; i < shape.size(); i++)
	{
		inPair = shape[i];

		vector <int > off_shape = inPair.shape;
#if EN_PEER2SIZE_PRINT
		cout << "off_shape (size, pos) : " << off_shape.size() << " " << inPair.pos << endl;
#endif
		int size = sizeof(T);

#if EN_PEER2SIZE_PRINT
		cout << "(1, d, h, w) : ";
#endif
		for(unsigned int j = 0; j < off_shape.size(); j++)
		{
			//# Always planes are in pos-1
			if(j == 1)
			{
				//# Planes are divided by 2 because of 2x-IO ports
				int x = AlignSize(off_shape[j], align_bytes);
				x = x/2;
				size *= x;
			}
			else
			{
				size *= off_shape[j];
			}
#if EN_PEER2SIZE_PRINT
			cout << off_shape[j] << " ";
#endif
		}
#if EN_PEER2SIZE_PRINT
		cout << endl;
#endif
		f_size = f_size + size;
	}
#if EN_PEER2SIZE_PRINT
	cout << "final offset : " << f_size << endl;
#endif

	return f_size;


}


//# Computes the offset required for concatenated buffers
template<typename T, int align_bytes>
int peersToShapeNoalign(vector < io_peers > shape)
{
#if EN_PEER2SIZE_PRINT
	cout << endl;
	cout << "peersToShape : (dim1 size) - " << shape.size() << endl;
#endif

	io_peers inPair;

	int f_size = 0;

	for(unsigned int i = 0; i < shape.size(); i++)
	{
		inPair = shape[i];

		vector <int > off_shape = inPair.shape;
#if EN_PEER2SIZE_PRINT
		cout << "off_shape (size, pos) : " << off_shape.size() << " " << inPair.pos << endl;
#endif
		int size = sizeof(T);

#if EN_PEER2SIZE_PRINT
		cout << "(1, d, h, w) : ";
#endif
		for(unsigned int j = 0; j < off_shape.size(); j++)
		{

			size *= off_shape[j];

#if EN_PEER2SIZE_PRINT
			cout << off_shape[j] << " ";
#endif
		}
#if EN_PEER2SIZE_PRINT
		cout << endl;
#endif
		f_size = f_size + size;
	}
#if EN_PEER2SIZE_PRINT
	cout << "final offset : " << f_size << endl;
#endif

	return f_size;


}

template<typename T>
int maxshapeToSize( vector<int> shape)
{
#if EN_SHAPE2SIZE_PRINT
	cout << endl;
	cout << "maxshapeToSize : size() - " << shape.size() << endl;
#endif

	int size = sizeof(T);

#if EN_SHAPE2SIZE_PRINT
	cout << "(1, d, h, w) : ";
#endif
	for(unsigned int j = 0; j < shape.size(); j++)
	{
		if(j == 1)
			size *= AlignSize(shape[j], KER_PROC);
		else
			size *= shape[j];

#if EN_SHAPE2SIZE_PRINT
		cout << shape[j] << " ";
#endif
	}

#if EN_SHAPE2SIZE_PRINT
	cout << "size : " << size << endl;
#endif

	return size;
}

template<typename T>
int halfshapeToSize( vector<int> shape)
{
#if EN_SHAPE2SIZE_PRINT
	cout << endl;
	cout << "maxshapeToSize : size() - " << shape.size() << endl;
#endif

	int size = sizeof(T);

#if EN_SHAPE2SIZE_PRINT
	cout << "(1, d, h, w) : ";
#endif
	for(unsigned int j = 0; j < shape.size(); j++)
	{
		if(j == 1)
			size *= AlignSize(shape[j]/2, KER_PROC);
		else
			size *= shape[j];

#if EN_SHAPE2SIZE_PRINT
		cout << shape[j] << " ";
#endif
	}

#if EN_SHAPE2SIZE_PRINT
	cout << "size : " << size << endl;
#endif

	return size;
}



int convOutputSize(int out_depth, int output_h, int output_w)
{
	int con_outDepth, half_depth;
	int buffer_split_flag = 1;

	con_outDepth = AlignSize(out_depth, KER_PROC);

	if(buffer_split_flag)
	{
		half_depth = con_outDepth/2;
	}
	else
	{
		half_depth = con_outDepth;
	}

	if(half_depth < KER_PROC)
	{
		half_depth = KER_PROC;
	}

	int size = half_depth * output_h * output_w * sizeof(HCONV_OUT_TYPE);
	return(size);
}

//# Convolution weights size compute
int convWeightsSize(int out_depth, int in_depth, int kernel_h, int kernel_w, int group)
{
	/*cout << "convWeightsSize (o_d, i_d, k_h, k_w, g) :" << out_depth << " " << in_depth
			<< " " << kernel_h << " " << kernel_w << " " << group << endl;
*/

	int no_kernals_1 = out_depth;
	int no_planes_in = AlignSize(in_depth, 4);
	int filter_size = kernel_h;

	//group = group + 1;   //default num groups is 1

	int no_planes_1 = no_planes_in/group;
	int remainder;
	int size_of_weights;
	int no_kernals,no_planes;

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


	int no_ele=filter_size*filter_size;

	size_of_weights=((no_kernals)*(no_planes)*(no_ele));

	int size = size_of_weights * sizeof(char);
	return(size);
}

///////////////	FC Utils  //////////////////////
int fcWgtsSize(int output_h, int output_w)
{
	int fc_outh = AlignSize(output_h, 32);
	int fc_outw = AlignSize(output_w, 32);

	int size = fc_outh * fc_outw * sizeof(HFC_WGT_TYPE);

	return(size);
}

#endif   // __XI_BUF_MGMT_UTILS_HPP__
