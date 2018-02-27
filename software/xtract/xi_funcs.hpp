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


#ifndef __XIFUNCS_HPP__
#define __XIFUNCS_HPP__

#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <iterator>
#include <cmath>
#include "../include/hw_settings.h"

using namespace std;

#if DATA_IN_BINARY
#define SAVEDATA savebin
#define READDATA readbin
#else
#define SAVEDATA savetxt
#define READDATA readtxt
#endif

// Handy LOG() function, useful for throwing INFO, DEBUG, WARNING
// condition : If this condition passes, message is thrown
// message code : Unique code for easy finding
// message : The original message to be printed
#define LOG(condition, code, message)   \
    if((condition))                     \
    {                                   \
        cerr << "[" << #code << "] ";    \
        cerr << message << endl;        \
    }

// Error LOG() function. useful for throwing ERROR
// If condition is met, throws error msg and exit application
#define ELOG(condition, code, message)   \
    if((condition))                     \
    {                                   \
        cerr << "[" << #code << "] ";    \
        cerr << message << endl;        \
        exit(-1);                       \
    }

// ASSERT function. useful for throwing ERROR
// If condition is NOT  met, throws error msg and exit application
#define ASSERT(condition, code, message)   \
    if(!(condition))                     \
    {                                   \
        cerr << "[" << #code << "] ";    \
        cerr << message << endl;        \
        exit(-1);                       \
    }

enum _rounding
{
    ROUND_NEAREST=0,
    ROUND_STOCHASTIC=1
};

enum _filemode
{
    FILE_TXT=0,
    FILE_BIN=1
};

typedef enum _rounding RoundingMethod;
typedef enum _filemode FileMode;

// Pre-C++11 equivalent implementation of std::to_string()
template<typename T>
string to_string(T number);

//template string to_string<int>(int number);
//template string to_string<float>(float number);
//template string to_string<double>(double number);

// Dummy function
void getDummyOutputDims(vector<vector<int> >& srcDims, vector<vector<int> >& dstDims); 

// // This function simply checks the number of tops and bottoms of a particular layer
// // A negative number denotes it can have any number of tops/bottoms
// // Zero means, it doesn't have top/bottom. 
// // TODO : @ARK: How to enable one-or-more blobs? (Eg: Concat layer). Negative number can have 0 blobs also
// template<typename bottomType, typename topType>
// void checkNumberOfTopAndBottom(string layerType, const bottomType& bottom, const topType& top, int nB=-1, int nT=-1);
// template<>
// void checkNumberOfTopAndBottom(string layerType, const vector<nameIndex>& bottom, const vector<nameIndex>& top, int nB, int nT)

// Dimension <A, B, C, D>  -->  <1, 1, 1, A*B*C*D>
vector<int> flattenTensorDim (vector<int>& src);

// Get total number of elems in a blob
int getSize(vector<int>& src);

// Get total number of in a blob excluding a particular axis
int getSizeExcludingAxis(vector<int>& src, int axis=-1);

// Represent a vector<int> dimension as string
// Dimension <A, B, C, D>  -->  String AxBxCxD
template<typename T>
string TensorDimToString (const vector<T>& src, const string delimiter="x");

// A function to join/concatenate several strings to a single string
string stringVectorToString (const vector<string>& src, const string delimiter=", ");
    
// A function to join multiple blob shapes to a single string
string dimVectorToString (const vector< vector<int> >& src, const string delimiter=", ");

// A function return Byte size as human readable size.
string humanReadableSize(const int size, const string& unit = "", const int precision = 3);

// A function to dump the vector<float> to a txt file   
void savetxt(const vector<float>& data, const string& filepath);

// A function to dump the vector<float> to a bin file
void savebin(vector<float> data, string filepath);

// Overloaded function for savetxt with custom range
void savetxt(vector<float>::const_iterator start, vector<float>::const_iterator end, string filepath);

// A function to read a txt file to a vector<float>
vector<float> readtxt(const string& filename);

// A function to read a bin file to a vector<float>
vector<float> readbin(const string& filename);

#endif      // __XIFUNCS_HPP__
