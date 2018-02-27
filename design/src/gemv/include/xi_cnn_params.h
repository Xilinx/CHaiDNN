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

#ifndef _XI_CNN_PARAMS_H_
#define _XI_CNN_PARAMS_H_

enum _BUS_WIDTH
{
	XI_IN_WIDTH_32   = 0,
	XI_IN_WIDTH_64   = 1,
	XI_IN_WIDTH_128  = 2,
	XI_IN_WIDTH_256  = 3,
	XI_IN_WIDTH_512  = 4,
	XI_IN_WIDTH_1024 = 5
};
typedef _BUS_WIDTH _bus_width;

enum _CPLX_BUS_WIDTH
{
	XI_CPLX_IN_WIDTH_64   = 0,
	XI_CPLX_IN_WIDTH_128  = 1,
	XI_CPLX_IN_WIDTH_256  = 2,
	XI_CPLX_IN_WIDTH_512  = 3,
	XI_CPLX_IN_WIDTH_1024 = 4
};
typedef _CPLX_BUS_WIDTH _cplx_bus_width;

enum _triangle_t
{
	UPPER_TRIANG, LOWER_TRIANG
};
typedef _triangle_t triangle_t;

#endif//_XI_CNN_PARAMS_H_
