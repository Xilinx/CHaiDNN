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

#ifndef __XI_CHECKERS_HPP__
#define __XI_CHECKERS_HPP__

#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>

#include "../include/xchange_structs.hpp"

#define EN_HISTOGRAM	1
#define EN_FILE_WRITE   1

#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)
#define ConvertToFP(fVal, iPart, fbits)	((int)((iPart<<fbits) + ((fVal-(float)iPart))*(1<<fbits)))

//# Checks funtionality
int errorCheck(
		xChangeLayer inLayer
);

//# Checks funtionality
int errorCheck1(
		xChangeLayer inLayer
);

//# Enable Output file write
int outFileWrite(
		xChangeLayer inLayer,
		const char *img_path
);

//# Checks Convolution/Pool funtionality
int cpCheck(
		xChangeLayer inLayer,
		FILE *&csv_fp,
		FILE *&error_fp
);

//# Checks Convolution/Pool funtionality
int cpCheck_packed(
		xChangeLayer inLayer
);

//# Checks FC funtionality
int fcCheck(
		xChangeLayer inLayer,
		FILE *&csv_fp,
		FILE *&error_fp
);

int swfcCheck(
		xChangeLayer inLayer,
		FILE *&csv_fp,
		FILE *&error_fp
);

//# Checks softmax funtionality
int softmaxCheck(
		xChangeLayer inLayer
);

//# Checks NMS functionality
int nmsCheck(
		xChangeLayer inLayer,
		FILE *&csv_fp,
		FILE *&error_fp
);


//# Checks normalization functionality
int normCheck(
		xChangeLayer inLayer,
		FILE *&csv_fp,
		FILE *&error_fp
);

//# Checks Permute functionality
int permuteCheck(
		xChangeLayer inLayer,
		FILE *&csv_fp,
		FILE *&error_fp
);

int swSoftmaxCheck(
		xChangeLayer inLayer,
		FILE *&csv_fp,
		FILE *&error_fp
);

int deconvCheck(
		xChangeLayer inLayer,
		FILE *&error_fp
);

void softmax_outfloatdatawrite1(float *input, int depth, int height, int width, const char *path);

void softmax_outfloatdatawrite2(float *input, int depth, int height, int width, const char *path, int batch_size);

void softmax_outfloatdatawrite3(IO_DATA_TYPE *input, int depth, int height, int width, const char *path, int batch_size, int fp_bits);

void conv_loadinput_split(xChangeLayer inLayer, char *path);

void LoadInputData(xChangeLayer inLayer, char *lay_path);

void LoadInputData_flmode(xChangeLayer inLayer, char *lay_path);

void LoadInputData_singleio(xChangeLayer inLayer, char *lay_path);

void LoadInputData_singleio_flmode(xChangeLayer inLayer, char *lay_path);

void conv_loadinput_pack(xChangeLayer inLayer, char *path);

int swSoftmaxWrite(
		xChangeLayer inLayer,
		const char *output_file_path
);

//# Checks crop layer output
int cropWrite(
		xChangeLayer inLayer,
		const char *output_file_path
);

//# Writes nms output data to file
int nmsWrite(
		xChangeLayer inLayer,
		const char *output_file_path
);

const char* getFileNameFromPath(char* path);
void LoadInputData(xChangeLayer inLayer, char *lay_path);
void LoadInputData_singleio(xChangeLayer inLayer, char *lay_path);
void LoadInputfcData(xChangeLayer inLayer, char *lay_path);
void LoadInputNormData(xChangeLayer inLayer, char *lay_path);


#endif      // __XI_CHECKERS_HPP__
