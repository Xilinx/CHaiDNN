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

#ifndef _XI_UTILS_HPP_
#define _XI_UTILS_HPP_

#define PACK_ELEMS				8	//# TODO : Should come from "hw_settings" file
#define XI_UTILS_CONSOLE_TEXT	0

//# Re-arranges data for HW FC
void fc_inputdatawrite(short *conv_out, short *conv_out1, int height, int width, int indepth, bool relu,
		short *kernel_output, char *output_file_path);

//# Re-arranges data for HW deconv
void DeconvInReArrange(short *input1, short *input2, short *output, int height, int width, int inp_planes);

#endif//_XI_UTILS_HPP_
