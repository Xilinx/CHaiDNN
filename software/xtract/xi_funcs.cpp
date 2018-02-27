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


#include "xi_funcs.hpp"
#include <iomanip>

using namespace std;

// Pre-C++11 equivalent implementation of std::to_string()
template<typename T>
string to_string(T number)
{
    ostringstream ostr; //output string stream
    ostr << number;     
    string tmp = ostr.str();
    return tmp;
}
template<>
string to_string(bool number)
{
    string tmp = number ? "true" : "false" ;
    return tmp;
}
template
string to_string<unsigned long>(unsigned long number);
template
string to_string<unsigned int>(unsigned int number);

//template string to_string<int>(int number);
//template string to_string<float>(float number);
//template string to_string<double>(double number);

// Dummy function
void getDummyOutputDims(vector<vector<int> >& srcDims, vector<vector<int> >& dstDims) {} 

// Dimension <A, B, C, D>  -->  <1, 1, 1, A*B*C*D>
vector<int> flattenTensorDim (vector<int>& src)
{
    int finalDim = 1;
    for(int i=0; i<src.size(); i++)
    {
        finalDim *= src.at(i);
    }
    
    vector<int> dst(4, 1);          // 1x1x1x1
    dst.at(3) = finalDim;           // 1x1x1xfinalDim
    return dst;
}

// Get total number of elems in a blob
int getSize(vector<int>& src)
{
    int finalDim = 1;
    for(int i=0; i<src.size(); i++)
    {
        finalDim *= src.at(i);
    }
    
    return finalDim;
}

// Get total number of in a blob excluding a particular axis
int getSizeExcludingAxis(vector<int>& src, int axis)
{
    if(axis >= src.size())
    {
        cerr << "[EG001] axis should be less that array dimension" << endl;
        exit(-1);
    }
    
    if(axis == -1)
        return getSize(src);

    int finalDim = 1;
    for(int i=0; i<src.size(); i++)
    {
        if(i != axis)
            finalDim *= src.at(i);
    }
    
    return finalDim;
}


// Represent a vector<int> dimension as string
// Dimension <A, B, C, D>  -->  String AxBxCxD
template<typename T>
string TensorDimToString (const vector<T>& src, const string delimiter)
{
    string tmp = "";
    string delim = ""; 
    for(int i=0; i<src.size(); i++)
    {
        tmp += to_string(src.at(i));
        delim = (i==src.size()-1) ?  "" : delimiter;
        tmp += delim;
    }
    return tmp;
}
template
string TensorDimToString<int>(const vector<int>& src, const string delimiter);
template
string TensorDimToString<float>(const vector<float>& src, const string delimiter);

// A function to join/concatenate several strings to a single string
string stringVectorToString (const vector<string>& src, const string delimiter)
{
    string tmp = "";
    string delim = "";
    for(int i=0; i<src.size(); i++)
    {
        tmp += src.at(i);
        delim = (i==src.size()-1) ?  "" : delimiter;
        tmp += delim;
    }
    return tmp;
}
    
// A function to join multiple blob shapes to a single string
string dimVectorToString (const vector< vector<int> >& src, const string delimiter)
{
    string tmp = "";
    string delim = "";
    for(int i=0; i<src.size(); i++)
    {
        tmp += TensorDimToString(src.at(i));
        delim = (i==src.size()-1) ?  "" : delimiter;
        tmp += delim;
    }
    return tmp;
}

// A function return Byte size as human readable size
// If no unit provided, the function will decide proper unit.
string humanReadableSize(const int sizeinBytes, const string& unit, const int precision)
{
    ostringstream ostr; //output string stream
    std::ios::fmtflags oldFlags = ostr.flags();
    ostr.precision(precision);
    // ostr.setf(std::ios::fixed, std::ios::floatfield);

    // Deducing proper unit
    if(unit.empty())
    {
        if(sizeinBytes >= (1<<29))       // More than 512 MB, print it in terms of GB
        {
            float sizeInGB = (float)sizeinBytes / (1<<30);
            ostr << sizeInGB << " GB";
        }
        else if((sizeinBytes >= (1<<19)) && (sizeinBytes < (1<<29))) // 512 KB < size < 512 MB, print it in terms of MB
        {
            float sizeInMB = (float)sizeinBytes / (1<<20);
            ostr << sizeInMB << " MB";
        }
        else if((sizeinBytes >= (1<<9)) && (sizeinBytes < (1<<19))) // 512 B < size < 512 KB, print it in terms of KB
        {
            float sizeInKB = (float)sizeinBytes / (1<<10);
            ostr << sizeInKB << " KB";
        }
        else
        {
            ostr << sizeinBytes << " B";
        }
    }
    else
    {
        // Return user defined format
        if(unit == "B")
        {
            ostr << sizeinBytes << " B";
        }
        else if(unit == "KB")
        {
            float sizeInKB = (float)sizeinBytes / (1<<10);
            ostr << sizeInKB << " KB";
        }
        else if(unit == "MB")
        {
            float sizeInMB = (float)sizeinBytes / (1<<20);
            ostr << sizeInMB << " MB";
        }
        else if(unit == "GB")
        {
            float sizeInGB = (float)sizeinBytes / (1<<30);
            ostr << sizeInGB << " GB";
        }
        else
        {
            cerr << "[EG015] Unit should be one of B/KB/MB/GB" << endl;
        }
    }
    ostr.flags(oldFlags);

    return ostr.str();
}

// A function to dump the vector<float> to a txt file   
void savetxt(const vector<float>& data, const string& filepath)
{
    std::ofstream output_file(filepath.c_str());
    if(!output_file)
    {
        cerr << "[ERROR] Couldn't create " << filepath << endl;
        exit(-1);
    }

    std::ios::fmtflags oldFlags = output_file.flags();
    output_file.precision(10);
    std::ostream_iterator<float> output_iterator(output_file, "\n");
    std::copy(data.begin(), data.end(), output_iterator);
    output_file.flags(oldFlags);
    output_file.close();
}

// A function to dump the vector<float> to a bin file
void savebin(vector<float> data, string filepath)
{
    std::ofstream output_file(filepath.c_str(), ios::out | ios_base::binary);
    if(!output_file)
    {   
        cerr << "[ERROR] Couldn't create " << filepath << endl;
        exit(-1);
    }   

    output_file.write(reinterpret_cast<char*>(&data[0]), data.size() * sizeof(float));
    output_file.close();
}

// Overloaded function for savetxt with custom range
void savetxt(vector<float>::const_iterator start, vector<float>::const_iterator end, string filepath)
{
    cerr << "[IG002] Saving " << filepath << endl;
    std::ofstream output_file(filepath.c_str());
    if(!output_file)
    {
        cerr << "[ERROR] Couldn't create " << filepath << endl;
        exit(-1);
    }

    std::ostream_iterator<float> output_iterator(output_file, "\n");
    std::copy(start, end, output_iterator);
    output_file.close();
}

// A function to read a txt file to a vector<float>
vector<float> readtxt(const string& filename)
{
    std::ifstream input_file(filename.c_str());
    if(!input_file)
    {   
        cerr << "[ERROR] Couldn't open" << filename << endl;
        exit(-1);
    }   

    std::vector<float> v;
    std::copy(istream_iterator<float>(input_file), istream_iterator<float>(), back_inserter(v));
    input_file.close();
    return v;
}
// A function to read float binary file to a vector<float>
vector<float> readbin(const string& filename)
{
    std::ifstream input_file(filename.c_str(), ios::in | ios_base::binary);
    if(!input_file)
    {
        cerr << "[ERROR] Couldn't open" << filename << endl;
        exit(-1);
    }

    std::streampos posA = input_file.tellg();
    input_file.seekg(0, std::ios::end);
    std::streampos posB = input_file.tellg();
    input_file.seekg(posA);
    std::streamsize size = posB - posA;

    std::vector<float> v(size/sizeof(float));
    input_file.read(reinterpret_cast<char*>(&v[0]), size);
    input_file.close();
    return v;
}
