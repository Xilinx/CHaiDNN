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

#include "xi_sgemv_config.h"
#include "../src/xi_sgemv.hpp"

void xiSgemvTop(
		GMEM_TYPE * weights1,
		GMEM_TYPE * weights2,
		GMEM_TYPE * input,
		GMEM_TYPE * bias,
		GMEM_TYPE * output,
		int *scalar_gemv_args)
{

	xiSgemv<GMEM_SHIFT_CNT, GMEM_SHIFT_CNT, MORDER_TRIP, NORDER_TRIP, TOT_TRIP>(weights1, weights2, input, bias, output,scalar_gemv_args);

}
