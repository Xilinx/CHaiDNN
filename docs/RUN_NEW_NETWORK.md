<table style="width:100%">
<tr>
<th width="100%" colspan="6"><img src="https://www.xilinx.com/content/dam/xilinx/imgs/press/media-kits/corporate/xilinx-logo.png" width="30%"/><h1>CHaiDNN-v2</h2>
</th>
</tr>
  <tr>
    <th rowspan="6" width="17%">Analysis and Eval</th>
   </tr>
<tr>
	<td align="center" colspan="2"><a href="../docs/SUPPORTED_LAYERS.md">Supported Layers</a></td>
	<td align="center" colspan="2"><a href="../docs/PERFORMANCE_SNAPSHOT.md">Performance/Resource Utilization</a></td>
</tr>
  <tr></tr>
<tr>
	<td align="center" colspan="4"><a href="../docs/PERFORMANCE_EVAL.md">Performance Eval</a></td>	
</tr>
<tr></tr>
    <tr></tr>
  <tr><th colspan="6"></th></tr>

  <tr></tr>
  <tr>
     <th rowspan="7" width="17%">Design and Development</th>
   </tr>

<tr>
	<td  align="center"><a href="../docs/API.md">API Reference</a></td>
	<td  align="center"><a href="../docs/QUANTIZATION.md">Quantization User Guide for CHaiDNN</a></td>
	<td  align="center"><a href="../docs/MODELZOO.md">Model Zoo</a></td>
	<td  align="center">Running Inference on new Network</td>
</tr>
  <tr></tr>
<tr>
	<td  align="center"><a href="../docs/BUILD_USING_SDX_GUI.md">Creating SDx GUI Project</a></td>
	<td  align="center"><a href="../docs/CONFIGURABLE_PARAMS.md">Configurable Parameters</a></td>
	<td  align="center"><a href="../docs/CUSTOM_PLATFORM_GEN.md">Custom Platform Generation</a></td>
	<td  align="center"><a href="../docs/SOFTWARE_LAYER_PLUGIN.md">Software Layer Plugin</a></td>
</tr>
  <tr></tr>
<tr>
	<td  align="center" colspan="2"><a href="https://www.xilinx.com/support/documentation/sw_manuals/xilinx2017_4/ug1027-sdsoc-user-guide.pdf">SDSoC Environment User Guide</a></td>	
	<td align="center" colspan="2"><a href="../docs/HW_SW_PARTITIONING.md">Hardware-Software Partitioning for Performance</a></td>

</tr>  
</table>

## Running a Network using CHaiDNN

### Prerequisites
- Network files (*.prototxt*, *.caffemodel* & *mean file*)
- All the layers of the network must be supported by CHaiDNN
- Quantization parameters for all layers
- CHaiDNN repository & pre-built binaries / SD-Card image

### Combination of Supported bit-widths
 Weights/Bias  |  Activations
---------------|--------------
   8-Bit | 8-bit
   6-Bit | 6-Bit

>**:pushpin: NOTE:**  For all the layer of a given network the bit-widths should be same for activations. Same is the case with bit-widths for weights and biases

## **Layer Support**
Supported Layers||||
:--------------:|:---------------:|:---------------:|:---------------:|
Convolution     | BatchNorm       | Power           | Scale           |
Deconvolution   | ReLU            | Pooling(Max, Avg)         | InnerProduct    |
Dropout         | Softmax         | Crop            | Concat          |
Permute         | Normalize(L2 Norm)       | Argmax          | Flatten         |
PriorBox        | Reshape         | NMS             | Eltwise         |
CReLU<sup>*</sup> | Depthwise Separable Convolution | Software Layer Plugin<sup>**</sup>|Input/ Data|
Dilated Convolution||||         |

>**:pushpin: NOTE:**  The above mentioned layers are verified for the layer configurations/params defined in the provided 6 example networks only.

## **Steps to Run a network using CHaiDNN**

<details>
<summary><strong><big>Step 1: Update prototxt with precision Parameters</big> </strong></summary>

CHaiDNN works in fixed point domain for better performance. All the feature maps and trained parameters are converted from single precision to fixed point before the computation starts. The steps to obtain the updated deploy.prototxt with precision parameters are shared in the [Quantization user guide](./doc/QUANTIZATION.md)
</details>

<details>
<summary><strong><big>Step 2: Build application with new network</big></strong></summary>

To implement a network inference using CHaiDNN APIs, follow these steps.

##### 1. Create an Application with new Network
1. Create a cpp file. Say, `MyNet_ex.cpp`.
2. Include standard headers and opencv header files.

    ```c++
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>

    #undef __ARM_NEON__
    #undef __ARM_NEON
    #include <opencv2/core/core.hpp>
    #include <opencv2/imgproc/imgproc.hpp>
    #include <opencv2/highgui/highgui.hpp>
    #define __ARM_NEON__
    #define __ARM_NEON

    #include <iostream>
    using namespace std;
    using namespace cv;
    ```

3. Include sds include.
    ```c++
    #include "sds_lib.h"
    ```
4. Include interface header which has CHaiDNN API prototypes
   ```c++
    #include "../api/xi.hpp"
    #include "../api/xi_readwrite_util.hpp"
    #include "../checkers/checkers.hpp"
   ```
5. Define start/end time Macros for performance measurement
    ```c++
    //# Performance check
    long long int clock_start, clock_end, frequency;
    #define TIME_STAMP_INIT  clock_start = sds_clock_counter();
    #define TIME_STAMP  { \
            clock_end = sds_clock_counter(); \
            frequency = sds_clock_frequency(); \
    }
    ```

6. Write `main()`.

   1. Create a structure to hold info about input/output layers
      ```c++
        io_layer_info io_layer_info_ptr;
      ```
   2. Define variables to hold network directory/file paths.
      ```c++
      char *dirpath    = "/mnt/models/MyNet"; /* Path to the network model directory */
	    char *prototxt   = "deploy.prototxt";       /* Prototxt file name residing in network model directory */
	    char *caffemodel = "MyNet.caffemodel";  /* caffemodel file name residing in network model directory */
      ```

   3.  Define variables to hold input image path.
       ```c++
       char *img_path  = "/mnt/models/MyNet/input/camel.jpg";
       ```

   4. The data initialization can now be done using the `xiInit()` API. This API parses the network and initializes the Job-queue with memory and store network params in buffers.
      ```c++
	    void *chai_handle = xiInit(dirpath, prototxt, caffemodel, &io_layer_info_ptr, numImg_to_process, is_first_layer, start_layer, end_layer);
      ```
   5. Read and pre-process input image. This includes resizing the input image and subtract the mean if the mean is pixel wise. Two example utility functions are provided to make preprocessing easier.
      ```c++
	    int status = inputNormalization(normalizeInput, resize_h, resize_w, img_path1, img_path2,
			inp_mode, mean_path, numImg_to_process, io_layer_info_ptr);
      ```
  6. Create input buffer
     ```c++
     int in_size = io_layer_info_ptr.inlayer_sizebytes;
     //# Create input/output Buffers
     vector<void *> input;
     void *ptr;
     for(int i = 0; i < io_layer_info_ptr.num_in_bufs; i++)
     {
         if(io_layer_info_ptr.inlayer_exectype.compare("hardware") == 0)
             ptr = sds_alloc_non_cacheable(in_size);
         else
             ptr = malloc(in_size);
         input.push_back(ptr);
     }
     ```
  7. Create output buffer
     ```c++
     int out_size = io_layer_info_ptr.outlayer_sizebytes;
     vector<void *> output;

     for(int i = 0; i < io_layer_info_ptr.num_out_bufs; i++)
     {
        if(io_layer_info_ptr.outlayer_exectype.compare("hardware") == 0)
            ptr = sds_alloc_non_cacheable(out_size);
        else
            ptr = malloc(out_size);
        output.push_back(ptr);
     }
     ```
 8. Pack the mean-subtracted input to input buffer
    ```c++
    xiInputRead(normalizeInput, input, numImg_to_process, io_layer_info_ptr);
    ```

 9. Call `xiExec` to run inference
    ```c++
        TIME_STAMP_INIT
        xiExec(chai_handle, input, output);
        TIME_STAMP
    ```
    >**:pushpin: NOTE:**  `TIME_STAMP_INIT` and `TIME_STAMP` stores the start and end cycles which can be used to check the performance of the network.

 10. Check the latency
     ```c++
     //# Total time for the API in Images/Second
     double tot_time = (((double)(clock_end-clock_start)/(double)frequency)*1000)*(double)XBATCH_SIZE;
     fprintf(stderr, "\n[PERFM] Performance : %lf Images/second\n", (double)(1000)/tot_time);
     fprintf(stderr, "\n\n");
     ```
 11. Unpack the output and write to output file (optional)
     ```c++
     int unpack_out_size = io_layer_info_ptr.outlayer_sizebytes;

     //# Create memory for unpack output data
     vector<void *> unpack_output;
     for(int batch_id = 0; batch_id < numImg_to_process; batch_id++)
     {
        void *ptr = malloc(unpack_out_size);
        unpack_output.push_back(ptr);
     }
     //# Loading required params for unpack function
     kernel_type_e out_kerType = io_layer_info_ptr.out_kerType;
     int out_layer_size = io_layer_info_ptr.out_size;
     //# unpacks the output data
     xiUnpackOutput(output, unpack_output, out_kerType, out_layer_size, numImg_to_process);
     //# Write the output data to txt file
     outputWrite(dirpath, img_path1, unpack_output, numImg_to_process, io_layer_info_ptr, 0);
     ```
 12. Release Memory
     ```c++
     xiRelease(chai_handle); //# Release before exiting application
     ```

##### 2. Compile Application using Makefile.

>**:pushpin: NOTE:**  Paths provided for libs/includes in below Makefile example might change based on where the Makefile is located. Use Relative/Absolute paths to libs/includes based on the directory structure. These instructions assumes that all the  libraries are already built and kept in `SD_Card` directory.  

 1. Set ARM compiler & SDx install path
    ```makefile
    ARM_CXX = aarch64-linux-gnu-g++

    # Provide Correct SDx Path
    SDx_BUILD_PATH = /proj/xbuilds/2017.4_released/installs/lin64/SDx/2017.4
    ```
2. Set include path
    ```makefile
    IDIRS = -I$(SDx_BUILD_PATH)/target/aarch64-linux/include
    ```
3. Set OpenCV and Protobuf paths
    ```makefile
    PB_ARM_DIR = ../../SD_Card/protobuf_arm64
    OPENCV_DIR = ../../SD_Card/opencv_arm64
    CBLAS_ARM_DIR = ../../SD_Card/cblas_arm64
    ```
4. Set required libraries
    ```makefile
    OPENCV_LIBS = -lopencv_core -llzma -ltiff -lpng16 -lz -ljpeg -lopencv_imgproc -lopencv_imgcodecs -ldl -lrt -lwebp
    LDIRS = -L../../SD_Card/lib
    LLIBS = -lprotobuf -lpthread -lxstack -lxlnxdnn -lparser_arm
    ```
5. Set compilation flags
    ```makefile
    CFLAGS_ARM = -std=c++11 -D__SDSOC=1 -Wno-write-strings
    .PHONY: all
    ```
6. Set compilation commands using above variables
    ```makefile
    MyNet.elf : ./MyNet_ex.cpp
    	$(ARM_CXX) $(CFLAGS_ARM) -L$(PB_ARM_DIR)/lib -I$(PB_ARM_DIR)/include -L$(OPENCV_DIR)/lib -I$(OPENCV_DIR)/include -L$(CBLAS_ARM_DIR)/lib -I(CBLAS_ARM_DIR)/include $(IDIRS) $(LDIRS) $(LLIBS) $(OPENCV_LIBS) $^ -o $@
    ```

7. Save Makefile and run make
    ```sh
    make MyNet.elf
    ```

This will generate an executable `MyNet.elf` to run the network inference.
</details>


<hr/>
<p align="center"><sup>Copyright&copy; 2018 Xilinx</sup></p>
