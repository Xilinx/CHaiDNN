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

#define EN_HISTOGRAM	0

#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)
#define ConvertToFP(fVal, iPart, fbits)	((int)((iPart<<fbits) + ((fVal-(float)iPart))*(1<<fbits)))

//# Checks funtionality
int errorCheck(
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
		const char *ref_path,
		const char *output_file_path,
		FILE *&csv_fp
);

//# Checks Convolution/Pool funtionality
int cpCheck_packed(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path
);

//# Checks FC funtionality
int fcCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path,
		FILE *&csv_fp
);

//# Checks softmax funtionality
int softmaxCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path
);

//# Checks NMS functionality
int nmsCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path,
		FILE *&csv_fp
);


//# Checks normalization functionality
int normCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path,
		FILE *&csv_fp
);

//# Checks Permute functionality
int permuteCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path,
		FILE *&csv_fp
);

int swSoftmaxCheck(
		xChangeLayer inLayer,
		const char *ref_path,
		const char *output_file_path,
		FILE *&csv_fp
);

void softmax_outfloatdatawrite1(float *input, int depth, int height, int width, const char *path);

void conv_loadinput_split(xChangeLayer inLayer, char *path);

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


#endif      // __XI_CHECKERS_HPP__
