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

#ifndef _XI_SCHEDULER_HPP_
#define _XI_SCHEDULER_HPP_

//#include <cstdint>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <string.h>
#include <stdio.h>

using namespace std;

//# Buffer Management Module
#include "../include/xchange_structs.hpp"

//# Declarations of all kernel functions
#include "../common/xi_kernels.h"

//# Thread Sub-routines
#include "xi_thread_routines.hpp"

//# Error Check
#include "../checkers/checkers.hpp"

//# Layers required
#define NEEDED_CONV		1
#define NEEDED_POOL		1
#define NEEDED_FC		1
#define NEEDED_DECONV	1
#define NEEDED_SOFTMAX	1
#define NEEDED_NMS		1
#define NEEDED_NORM		1
#define NEEDED_PERMUTE	1
#define NEEDED_CROP		1
#define NEEDED_XCUSTOM	1
#define NEEDED_XPACK	1

//# Code Folding 
#define ENABLE_SCHEDULER			1
#define ENABLE_IMAGE_READ_THREAD	0
#define RESET_DONE_FLAGS			1
#define ENABLE_CONSOLE_TEXT			0
#define ENABLE_ERROR_CHECKS			0
#define MANUAL_LAYER_ENABLE			0

#ifdef __SDSOC
#include <sds_lib.h>
#define PERFORMANCE_CHECK			1
#else
#define PERFORMANCE_CHECK			0
#endif

#define LAYERWISE_PERFORMANCE 1

#endif//_XI_SCHEDULER_HPP_
