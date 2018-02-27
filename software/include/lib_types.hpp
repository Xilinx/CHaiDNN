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

#ifndef __LIB_TYPES_HPP__
#define __LIB_TYPES_HPP__

#include <map>
#include <vector>
#include "xgraph.hpp"

// common datatypes
typedef map<string, XLayer*> mapStrXLayer;
typedef map<string, XBlob*> mapStrXBlob;

// common iterators
typedef map<string, XLayer*>::iterator mapStrXLayer_it;
typedef map<string, XLayer*>::const_iterator mapStrXLayer_cit;
typedef map<string, XBlob*>::iterator mapStrXBlob_it;
typedef map<string, XBlob*>::const_iterator mapStrXBlob_cit;



#endif      // __LIB_TYPES_HPP__
