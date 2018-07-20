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

#ifndef __XBLOB_HPP__
#define __XBLOB_HPP__

#include <iostream>
#include <string>
#include <vector>

#include "xi_funcs.hpp"

using namespace std;

class XBlob
{
private:
    static int UID;
public:
    // fields
    string name;                                // User provided name in network file
    string reuse_name;                          // this name is used only for graph_draw
    string uname;                               // Unique name assigned by the program
    vector< int > shape;                        // Overall Dim of the tensor
    int size;                                   // Total size of this tensor (size = # elems)
    vector< vector<int> > producerDim;          // Dim of data written by producer to this array
    vector< vector<int> > consumerDim;          // Dim of data read by consumer from this array
    vector< string > producers;                 // List of layers writing to this tensor
    vector< string > consumers;                 // List of layers reading from this tensor

    // methods

    // Constructor 1
    XBlob()
    :uname("B"+to_string(UID++))
    {}

    // Constructor 2
    XBlob(string _name)
    :name(_name), uname("B"+to_string(UID++)),reuse_name(_name)
    {}
    
    // Destructor 2
    ~XBlob() { }
    
    // Helper function to return a string representation of XBlob
    string str(bool use_user_names=true) 
    { 
        string printName = use_user_names ? name : uname; 

        string tmp = ""; 
        tmp     +=  printName + " : " + TensorDimToString(shape);
        return tmp;
    }

};

class nameIndex
{
public:
    XBlob* blob;
    int id;

    nameIndex()
    : blob(NULL), id(0)
    {}

    nameIndex(XBlob* _blob)
    : blob(_blob), id(0)
    {}

    nameIndex(XBlob* _blob, int _i)
    : blob(_blob), id(_i)
    {}
    
    ~nameIndex() {}; 

    // Return a nameIndex as a string --> "blob:id"
    string str(bool use_user_names = true)
    {
        string printName = use_user_names ? blob->name : blob->uname; 
        string tmp = ""; 
        tmp     +=  printName + ":" + to_string(id);
        return tmp;
    }
};

#endif  // __XBLOB_HPP__
