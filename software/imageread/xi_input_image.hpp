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

#ifndef _XCHANGE_INPUTIMAGE_HPP_
#define _XCHANGE_INPUTIMAGE_HPP_

#if 1//def __SDSCC__
#undef __ARM_NEON__
#undef __ARM_NEON
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#define __ARM_NEON__
#define __ARM_NEON
#else
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#endif

using namespace std;
using namespace cv;

#include <iostream>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include "../include/xchange_structs.hpp"

#define EN_IO_PRINT    0

#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)
#define ConvertToFP(fVal, iPart, fbits)	((int)((iPart<<fbits) + ((fVal-(float)iPart))*(1<<fbits)))

//void inputImageRead(FILE **file_list, xChangeLayer *xChangeHostInpLayer);
void inputImageRead(const char*, xChangeLayer *xChangeHostInpLayer);
int loadData(const char *img_data_path, xChangeLayer *xChangeHostInpLayer);
int loadImagefromBuffptr(unsigned char *buff_ptr, xChangeLayer *xChangeHostInpLayer);
int loadDatafromBuffptr(short *inp_buff, xChangeLayer *xChangeHostInpLayer);

#endif //_XCHANGE_INPUTIMAGE_HPP_
