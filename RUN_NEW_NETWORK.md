# **Running a Network using CHaiDNN**

## **Prerequisites**
* Network files (*.prototxt*, *.caffemodel* & *mean file*)
* All the layers of the network must be supported by CHaiDNN
* Quantization parameters for all layers
* CHaiDNN repository & pre-built binaries / SD-Card image

## **Supported Data types**
Data            | Supported Data width
----------------|---------------------
Weights/Bias    |   8-Bits
Activations     |   16-Bits

## **Layer Support**
Supported Layers||||
:--------------:|:---------------:|:---------------:|:---------------:|
Convolution     | BatchNorm       | Power           | Scale           |
Deconvolution   | ReLU            | Pooling         | InnerProduct    |
Dropout         | Softmax         | Crop            | Concat          |
Permute         | Normalize       | Argmax          | Flatten         |
PriorBox        | Reshape         | NMS             | Eltwise         |

**Note:** The above mentioned layers are varified for the layer configurations/params defined in the provided 6 example networks only.

## **Steps to Run a network using CHaiDNN**

1. **Get Precision Parameters**

User has to arrive at the precision parameters for the network parameters and activations. User can come up with his/her own techniques for determining the precision parameters or use readily available tools for arriving at the precision parameters. [Ristretto](http://lepsucd.com/?page_id=621) is one such tool. For more details refer to [Ristretto Quantization User Guide](http://lepsucd.com/?page_id=630).

CHaiDNN expects `Q.F` format for the precision, Where `Q` represent the bits for the integer part and `F` for the fractional.

2. **Update *deploy.prototxt* with Precision Parameters**

The Precision Parameters are communicated to CHaiDNN via *deploy.prototxt*. Set the `quantization_param{…}` section appropriately, based on the precision formats deduced, in deploy prototxt. 

Please refer to the [Quantization Parameter User Guide](./Quantization_Parameters_UG.md) for the syntax and detailed requirements for specifying precision parameters.

**Note:** `caffemodel` require no changes as CHaiDNN takes care of the format conversion from floating point representation to fixed point based on the precision formats specified in `prototxt`.

3. **Build an Application with a new Network**

Please refer to the below section on how to implement a network inference using CHaiDNN APIs.

## **Create & Compile Application with new Network**
## Create new inference application

1. Create a cpp file. Say, `MyNet_ex.cpp`.
2. Inlcude standard headers and opencv header files.

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
    #include "../interface/xi_interface.hpp"
    ```
5. Define start/end time MACROs for performance measurement
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

* Create vector for buffer pointers. This structure will be used to hold the addresses of memory created by `xiInit` API.
    ```c++
    bufPtrs ptrsList;
    ```	

* Create Job-Queue. This will hold the network layers' data for processing by the accelerator.
    ```cpp
    std::vector<xChangeLayer> jobQueue[1];
    ```
* Create Input and Output Buffers.
    ```c++
    //# Using GoogleNet IO size as example
    //# Input dimentions are mentioned in prototxt file
    void *input  = malloc (224 * 224 * 3 * sizeof(char)); 
    //# Output dimention based on the last layer of the network
    void *output = malloc (1024 * sizeof(float)); 
    ```
* Define variables to hold network directory/file paths.
    ```c++
    char *dirpath    = "/mnt/models/MyNet"; /* Path to the network model directory */
	char *prototxt   = "deploy.prototxt";       /* Prototxt file name residing in network model directory */
	char *caffemodel = "MyNet.caffemodel";  /* caffemodel file name residing in network model directory */
	char *mean_file	 = "mean_file.txt";         /* mean file name residing in network model directory */
	int  mean_type	 = 0                        /* 0 for .txt, 1 for .binaryproto */
    ```
* Mean data can be of two types.

    Mean type   | File type     | Size                          | mean_type
    ------------|---------------|-------------------------------|-----------
    Pixel wise  | .binaryproto  | Same as input dimensions      | 1
    Plane wise  | .txt          | 3 Values for 3 input planes   | 0

*  Define variables to hold input image path.
    ```c++
    char *img_path  = "/mnt/models/MyNet/input/camel.jpg";
    ```

* Now the data initialization can be done using the `xiInit` API. This API parses the network and initializes the Job-queue with memory and store network params in buffers.
    ```c++
    //# Call Initialization: One time initalization of network params
    xiInit (dirpath, prototxt, caffemodel, mean_file, mean_type, jobQueue, ptrsList, 224, 224);
    ```
* Read and preprocess input image. This includes resizing the input image and subtract the mean if the mean is pixel wise. We have provided two example utility functions to make preprocessing easier. 
    ```c++
    //# This can be used when mean_type is 0. In this case, mean will be subtracted from input by the accelerator
    int loadImagetoBuffptr(const char *img_path, unsigned char *buff_ptr, int height, int width, int img_channel, int resize_h, int resize_w);
    ```
    and
    ```c++
    //# This can be used when mean_type is 1. In this case, mean needs to be subtracted on CPU before sending it for processing.
    int loadMeanSubtractedDatatoBuffptr(const char *img_path, short *buff_ptr, int height, int width, int img_channel, int resize_h, int resize_w, short *mean_ptr, int fbits_input);
    ```
    For detailed usage of above functions, please go through the [example](./software/example) files.

* Call `xiExec` to run inference
    ```c++
    int inBytes = 1;
    TIME_STAMP_INIT
	xiExec(jobQueue, input, output, inBytes);	/* Call this in a loop for running multiple times with new input/output buffers */
	TIME_STAMP
    ```
    > `TIME_STAMP_INIT` and `TIME_STAMP` stores the start and end cycles which can be used to check the performance of the network.
    
    > `inBytes` represents the input data width. Only 1 and 2 are the valid values for it.

    inBytes | Data width
    --------|-----------
    1       | 8-Bit
    2       | 16-Bits

* Check the latency
    ```c++
    //# Total time for the API in Images/Second
    double tot_time = (((double)(clock_end-clock_start)/(double)frequency)*1000)/(double)NUM_IMG; //# NUM_IMG = 1
    fprintf(stderr, "\n[PERFM] Performance : %lf Images/second\n", (double)(1000)/tot_time);
    fprintf(stderr, "\n\n");
    ```
* Write output file (optional)
    ```c++
    //# Open file for writing
	FILE *out = fopen("/mnt/models/MyNet/out/out.txt", "w");
	//# Write output to a file
	for(int i = 0; i < 1000; ++i)
	{
		fprintf(out, "%f\n", ((float*)output)[i]);
	}
	fclose(out);
    ```

* Release Memory
    ```c++
	xiRelease(ptrsList); //# Release before exiting application
    ```
## Compile application

Compile using Makefile.

> **Note:** Paths provided for libs/includes in below Makefile example might change based on where the Makefile is located. Use Relative/Absolute paths to libs/includes based on the directory structure. These instructions assumes that all the  libraries are already built and kept in `SD_Card` directory.  

* Set ARM compiler & SDx install path
    ```makefile
    ARM_CXX = aarch64-linux-gnu-g++

    # Provide Correct SDx Path
    SDx_BUILD_PATH = /proj/xbuilds/2017.4_released/installs/lin64/SDx/2017.4
    ```
* Set include path
    ```makefile
    IDIRS = -I$(SDx_BUILD_PATH)/target/aarch64-linux/include
    ```
* Set OpenCV and Protobuf paths
    ```makefile
    PB_ARM_DIR = ../../SD_Card/protobuf_arm64
    OPENCV_DIR = ../../SD_Card/opencv_arm64
    ```
* Set required libraries
    ```makefile
    OPENCV_LIBS = -lopencv_core -llzma -ltiff -lpng16 -lz -ljpeg -lopencv_imgproc -lopencv_imgcodecs -ldl -lrt -lwebp
    LDIRS = -L../../SD_Card/lib
    LLIBS = -lprotobuf -lpthread -lxstack -lxlnxdnn -lparser_arm
    ```
* Set compilation flags
    ```makefile
    CFLAGS_ARM = -std=c++11 -D__SDSOC=1 -Wno-write-strings
    .PHONY: all
    ```
* Set compilation commands using above variables
    ```makefile 
    MyNet.elf : ./MyNet_ex.cpp
    	$(ARM_CXX) $(CFLAGS_ARM) -L$(PB_ARM_DIR)/lib -I$(PB_ARM_DIR)/include -L$(OPENCV_DIR)/lib -I$(OPENCV_DIR)/include $(IDIRS) $(LDIRS) $(LLIBS) $(OPENCV_LIBS) $^ -o $@
    ```

* Save Makefile and run make
    ```sh
    make MyNet.elf
    ```
    > This will generate an executable `MyNet.elf` to run the network inference. 