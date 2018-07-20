
/******************************************************************************
 *
 * (c) Copyright 2012-2016 Xilinx, Inc. All rights reserved.
 *
 * This file contains confidential and proprietary information of Xilinx, Inc.
 * and is protected under U.S. and international copyright and other
 * intellectual property laws.
 *
 * DISCLAIMER
 * This disclaimer is not a license and does not grant any rights to the
 * materials distributed herewith. Except as otherwise provided in a valid
 * license issued to you by Xilinx, and to the maximum extent permitted by
 * applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
 * FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
 * MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
 * and (2) Xilinx shall not be liable (whether in contract or tort, including
 * negligence, or under any other theory of liability) for any loss or damage
 * of any kind or nature related to, arising under or in connection with these
 * materials, including for any direct, or any indirect, special, incidental,
 * or consequential loss or damage (including loss of data, profits, goodwill,
 * or any type of loss or damage suffered as a result of any action brought by
 * a third party) even if such damage or loss was reasonably foreseeable or
 * Xilinx had been advised of the possibility of the same.
 *
 * CRITICAL APPLICATIONS
 * Xilinx products are not designed or intended to be fail-safe, or for use in
 * any application requiring fail-safe performance, such as life-support or
 * safety devices or systems, Class III medical devices, nuclear facilities,
 * applications related to the deployment of airbags, or any other applications
 * that could lead to death, personal injury, or severe property or
 * environmental damage (individually and collectively, "Critical
 * Applications"). Customer assumes the sole risk and liability of any use of
 * Xilinx products in Critical Applications, subject only to applicable laws
 * and regulations governing limitations on product liability.
 *
 * THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
 * AT ALL TIMES.
 *
 *******************************************************************************/

#ifndef _XI_READWRITE_UTIL_HPP_
#define _XI_READWRITE_UTIL_HPP_

int inputNormalization(std::vector<void *>input, int resize_h, int resize_w, char *img_path1, char *img_path2, bool inp_mode, 
		float *mean_ptr, float *var_ptr,
		int numImg_to_process, io_layer_info io_layer_info_ptr);

int outputUnpack(void *output, void *output_unpack[XBATCH_SIZE], int kernType, int outputSize, int en_batch_size_one);

void xiInputRead(std::vector<void *> normalizeInput, std::vector<void *> input, int numImg_to_process, io_layer_info io_layer_info_ptr);

void xiUnpackOutput(std::vector<void *> output, std::vector<void *> output_unpack, kernel_type_e kernelType, int outputSize, int numImg_to_process);

template<typename T>
void argmaxLayer(T* inarray, int* outarray, int nobjs, int nclasses, int top_k);

int outputWrite(char *dir_path, char *inp_file_name, std::vector<void *> unpack_output, int numImg_to_process, io_layer_info io_layer_info_ptr, int ping_pong);

#endif  //_XI_READWRITE_UTIL_HPP_
