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

#include <string.h>

using namespace std;

//template<typename T>
void cropLayer(int* inarray, int* outarray, int in_height, int in_width, int out_height, int out_width, int offset, int channels)
{
    // deconv output width round-to-next-nearest-even
    in_width = ((in_width + 1)/2)*2;

    int in_nelems = in_height * in_width;
    int out_nelems = out_height * out_width;

    for(int c=0; c<channels; c++)
    {
        for(int rin=offset, rout=0; rout < out_height; rin++, rout++)
        {
            int src_off = c * in_nelems + rin * in_width + offset;
            int dst_off = c * out_nelems + rout * out_width;
		//	std::cout << "src off : " << src_off << " : dst_off : " << dst_off << std::endl; 
        //    copy(inarray + src_off, inarray + src_off + out_width, outarray + dst_off);
			memcpy(outarray + dst_off, inarray + src_off, out_width*sizeof(int));
        }
    }
}
#if 0
// inarray - Input array (shape : nboxes x nclasses)
// outarray - output array. These are indices of the sorted array (shape : nboxes x top_k)
// nobjs- number of objects (For classification networks, it is mostly 1 because only one image. For FCN, it can be HxW)
// nclasses - number of classes. (For ImageNet, it is 1000. For PASCAL-VOC, it is 21)
// top_k    - Return top-k IDs. (User must take care of output array dimension)
template<typename T>
void argmaxLayer(T* inarray, int* outarray, int nobjs, int nclasses, int top_k)
{
    int nelems = nclasses * nobjs;
    switch(top_k)
    {
        case 0:             // Unsupported
            cerr << "top_k = 0 for argmax layer is not supported" << endl;
            exit(-1);
    
        case 1:             // if top_k = 1, all ID creation, copy, sort etc is overkill. Simply find the location of max element
            // // STL implementation. (This looks "slightly" slower than the vanilla implementation, need to verify on ARM for exact picture)
            // for(int i=0, j=0; i<nobjs; i++, j+=nclasses)
            // {
            //     int maxLoc = std::distance(inarray+j, max_element(inarray+j, inarray+j+nclasses));
            //     outarray[i] = maxLoc;
            // }
            
            // Vanilla Implementation
            for(int i=0; i<nobjs; i++)
            {
                int startoffset = i*nclasses;
                T maxVal = inarray[startoffset];
                int maxLoc = 0;
                for(int j=1; j<nclasses; j++)
                {
                    T val = inarray[startoffset + j];
                    if(val > maxVal)
                    {
                        maxVal = val;
                        maxLoc = j;
                    }
                }
                outarray[i] = maxLoc;
            }
            break;

        default:            // if top_k > 1, use partial_sort
            int* ID = new int [nelems];
            // Create permanent IDs
            for(int i=0; i<nelems; i++)
                ID[i] = i;

            // setup temporary ids.
            int* tmpID = new int [nelems];
            
            // Take each set of values and do argsort
            for(int i=0, j=0, k=0; i<nobjs; i++, j+=nclasses, k+=top_k)
            {
                // Reset the tmpID
                copy(ID, ID+nclasses, tmpID);
                // Do the partial sort
                partial_sort(tmpID, tmpID+top_k, tmpID+nclasses, descendingCmp<T>(inarray, j));
                // Copy the result to output array
                copy(tmpID, tmpID+top_k, outarray+k);
            }
            break;
    }
}
#endif
