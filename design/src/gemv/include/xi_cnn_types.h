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

#ifndef _XI_CNN_TYPES_H_
#define _XI_CNN_TYPES_H_

#include "ap_int.h"
#include "xi_cnn_params.h"

typedef ap_uint<1024> 	ap_uint1024_t;
typedef ap_uint<512> 	ap_uint512_t;
typedef ap_uint<256>	ap_uint256_t;
typedef ap_uint<128> 	ap_uint128_t;
typedef ap_uint<64> 	ap_uint64_t;
typedef ap_uint<32> 	ap_uint32_t;
typedef ap_uint<16> 	ap_uint16_t;
typedef ap_uint<4> 		ap_uint4_t;
typedef ap_uint<1> 		ap_uint1_t;

template<int T> struct InType  { };
template<> struct InType<XI_IN_WIDTH_32>    { typedef ap_uint<32>   name; static const int elements = 1;   };
template<> struct InType<XI_IN_WIDTH_64>    { typedef ap_uint<64>   name; static const int elements = 2;   };
template<> struct InType<XI_IN_WIDTH_128>   { typedef ap_uint<128>  name; static const int elements = 4;   };
template<> struct InType<XI_IN_WIDTH_256>   { typedef ap_uint<256>  name; static const int elements = 8;   };
template<> struct InType<XI_IN_WIDTH_512>   { typedef ap_uint<512>  name; static const int elements = 16;  };
template<> struct InType<XI_IN_WIDTH_1024>  { typedef ap_uint<1024> name; static const int elements = 32;  };

template<int T> struct CplxType  { };
template<> struct CplxType<XI_CPLX_IN_WIDTH_64>    { typedef ap_uint<64>   name; static const int elements = 1;   };
template<> struct CplxType<XI_CPLX_IN_WIDTH_128>   { typedef ap_uint<128>  name; static const int elements = 2;   };
template<> struct CplxType<XI_CPLX_IN_WIDTH_256>   { typedef ap_uint<256>  name; static const int elements = 4;   };
template<> struct CplxType<XI_CPLX_IN_WIDTH_512>   { typedef ap_uint<512>  name; static const int elements = 8;   };
template<> struct CplxType<XI_CPLX_IN_WIDTH_1024>  { typedef ap_uint<1024> name; static const int elements = 16;  };

#define XI_IN_TYPE(flag) \
	typename InType<flag>::name

#define XI_CPLX_TYPE(flag) \
	typename CplxType<flag>::name

#define XI_NUM_ELEMS(flag) \
	InType<flag>::elements

#define XI_CPLX_NUM_ELEMS(flag) \
		CplxType<flag>::elements

#define __MOST_NEG_VAL 4286578687

//# Find Min/Max
#define MIN(x,y)  ((x)>(y) ? (y) : (x))
#define MAX(x,y)  ((x)>(y) ? (x) : (y))
#define ABS(x)    (((x) > 0) ? (x) : (-(x)))

#endif//_XI_CNN_TYPES_H_
