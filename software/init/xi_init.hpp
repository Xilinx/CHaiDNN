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

#ifndef _XCHANGE_INIT_HPP_
#define _XCHANGE_INIT_HPP_

#define EN_DEBUG_INIT_PRINT	 0
#define EN_CONSOLE_TXT_INIT 0
#define EN_CONSOLE_TXT_INIT_DBG 1

#include "../xtract/xgraph_opt.hpp"
#include "../include/xchange_structs.hpp"
#include "../include/hw_settings.h"

#define AlignSize(x, y) (x%y == 0) ? x : ((x/y + 1)*y)
#define ConvertToFP(fVal, iPart, fbits)	((int)((iPart<<fbits) + ((fVal-(float)iPart))*(1<<fbits)))

void initPrint(std::vector<XlayerData> xlayer_seq, std::vector<xChangeLayer> lay_obj[NUM_IMG]);

void initXChangeHost(char *file_path, std::vector < XlayerData > &xlayer_seq, std::vector<xChangeLayer> (&lay_obj)[NUM_IMG]);

#endif //_XCHANGE_INIT_HPP_
