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

#ifndef __CUSTOM_CLASS_H__
#define __CUSTOM_CLASS_H__

#include <iostream>
#include <string>
#include <map>
#include <cmath>
#include <vector>
#include <cfloat>
#include <stdlib.h>
#include <algorithm>
#include <iterator>
#include "../include/xchange_structs.hpp"
using namespace std;

class Custom;

typedef void (Custom::*functionptr)(const xChangeLayer *xchange);


class Custom
{

public:

	void custom_function(const xChangeLayer *xchange);
	void custom_function1(xChangeLayer *xchange);
	void custom_gemm(const xChangeLayer *xchange);
	void custom_norm(const xChangeLayer *xchange);
	void custom_reorg(const xChangeLayer *xchange);
	void custom_passthrough(const xChangeLayer *xchange);
	xChangeLayer * arg_xchange;

	void custom(const string layer_type,const xChangeLayer *xchange);
	
	map<string, functionptr>   custom_func_keys;
	string  after_processing;
	Custom();
	~Custom();

};
#endif
