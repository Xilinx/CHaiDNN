<table style="width:100%">
<tr>
<th width="100%" colspan="6"><img src="https://www.xilinx.com/content/dam/xilinx/imgs/press/media-kits/corporate/xilinx-logo.png" width="30%"/><h1>CHaiDNN-v2</h2>
</th>
</tr>
  <tr>
    <th rowspan="6" width="17%">Analysis and Eval</th>
   </tr>
<tr>
	<td align="center" colspan="2"><a href="./docs/SUPPORTED_LAYERS.md">Supported Layers</a></td>
	<td align="center" colspan="2"><a href="./docs/PERFORMANCE_SNAPSHOT.md">Performance/Resource Utilization</a></td>
</tr>
  <tr></tr>
<tr>
	<td align="center" colspan="4"><a href="./docs/PERFORMANCE_EVAL.md">Performance Eval</a></td>	
</tr>
<tr></tr>
    <tr></tr>
  <tr><th colspan="6"></th></tr>

  <tr></tr>
  <tr>
     <th rowspan="7" width="17%">Design and Development</th>
   </tr>

<tr>
	<td  align="center"><a href="./docs/API.md">API Reference</a></td>
	<td  align="center"><a href="./docs/QUANTIZATION.md">Quantization User Guide for CHaiDNN</a></td>
	<td  align="center"><a href="./docs/MODELZOO.md">Model Zoo</a></td>
	<td  align="center"><a href="./docs/RUN_NEW_NETWORK.md">Running Inference on new Network</a></td>
</tr>
  <tr></tr>
<tr>
	<td  align="center"><a href="./docs/BUILD_USING_SDX_GUI.md">Creating SDx GUI Project</a></td>
	<td  align="center"><a href="./docs/CONFIGURABLE_PARAMS.md">Configurable Parameters</a></td>
	<td  align="center"><a href="./docs/CUSTOM_PLATFORM_GEN.md">Custom Platform Generation</a></td>
	<td  align="center"><a href="./docs/SOFTWARE_LAYER_PLUGIN.md">Software Layer Plugin</a></td>
</tr>
  <tr></tr>
<tr>
	<td  align="center" colspan="2"><a href="https://www.xilinx.com/support/documentation/sw_manuals/xilinx2017_4/ug1027-sdsoc-user-guide.pdf">SDSoC Environment User Guide</a></td>	
	<td align="center" colspan="2"><a href="./docs/HW_SW_PARTITIONING.md">Hardware-Software Partitioning for Performance</a></td>

</tr>  
</table>

## Introduction

CHaiDNN is a Xilinx Deep Neural Network library for acceleration of deep neural networks on Xilinx UltraScale MPSoCs. It is designed for maximum compute efficiency at 6-bit integer data type. It also supports 8-bit integer data type.

The design goal of CHaiDNN is to achieve best accuracy with maximum performance. The inference on CHaiDNN works in fixed point domain for better performance. All the feature maps and trained parameters are converted from single precision to fixed point based on the precision parameters specified by the user. The precision parameters can vary a lot depending upon the network, datasets, or even across layers in the same network. Accuracy of a network depends on the precision parameters used to represent the feature maps and trained parameters. Well-crafted precision parameters are expected to give accuracy similar to accuracy obtained from a single precision model.

## What's new in CHaiDNN-v2


- 4x GOPS compared to CHaiDNN-v1 (2017.4) [(Performance numbers)](./docs/PERFORMANCE_SNAPSHOT.md)

- 2x MAC on DSPs at int6

- Double-Pumped DSPs allowing the DSPs to be clocked at twice the core clock (Some configs can go upto 350/700Mhz)

- **Introducing DietChai** - A miniature version of CHai for smaller MPSoC/ Zynq devices

- 128, 256, 512, 1024 DSP design configs verified for [ZU9](https://www.xilinx.com/products/boards-and-kits/zcu102.html)

- Support for URAM

- 128, 256, 512 DSP configs verified for [ZU7](https://www.xilinx.com/products/boards-and-kits/zcu104.html)

- [ModelZoo](./docs/MODELZOO.md) of 6 networks at int8 and int6 precision

- Support for two quantization modes - [Dynamic fixed point and Xilinx Quantizer](./docs/QUANTIZATION.md)

- Enhanced API to enable better [hardware- software partitioning](./docs/HW_SW_PARTITIONING.md) for users

- Support for [software custom layer](./docs/SOFTWARE_LAYER_PLUGIN.md) plug-ins

- Fully Connected layers on CPU
- More documentation

## Performance Benchmarks(fps)

<table>
  <tr>
    <th>Network</th>
    <th>Xilinx CHai w/ 1024DSP @ 250/500MHz (Measured on <a href="https://www.xilinx.com/products/boards-and-kits/zcu104.html">ZU9</a>)</th>
    <th>Nvidia Jetson TX2 @ 1.3GHz*</th>
  </tr>
  <tr>
    <td>GoogleNet-6bit w/o FC</td>
    <td width="40%" align="center">220</td>
    <td rowspan="4" align="center">Googlenet-16FP: 201</td>
  </tr>
  <tr>
    <td>GoogleNet-6bit w/ FC</td>
    <td width="40%" align="center">207</td>

  </tr>
  <tr>
    <td>GoogleNet-8bit w/o FC </td>
    <td width="40%" align="center">151</td>

  </tr>
  <tr>
    <td>GoogleNet-8bit w/ FC</td>
    <td width="40%" align="center">145</td>

  </tr>


  <tr>
    <td>Alexnet-6bit w/o FC</td>
    <td width="40%" align="center">606</td>
    <td rowspan="4" align="center">Alexnet-16FP: 250</td>
  </tr>
  <tr>
    <td>Alexnet-6bit w/ FC</td>
    <td width="40%" align="center">10</td>

  </tr>
  <tr>
    <td>Alexnet-8bit w/o FC</td>
    <td width="40%" align="center">390</td>

  </tr>
  <tr>
    <td>Alexnet-8bit w/ FC</td>
    <td width="40%" align="center">10</td>

  </tr>
</table>

<sup>* Source: https://devblogs.nvidia.com/jetson-tx2-delivers-twice-intelligence-edge/</sup>

## Hardware and Software Requirements

The CHaiDNN library is designed to work with Zynq UltraScale+ MPSoCs. The library has been verified on [zcu102](https://www.xilinx.com/products/boards-and-kits/zcu102.html) and [zcu104](https://www.xilinx.com/products/boards-and-kits/zcu104.html) boards. Xilinx [SDSoC 2018.2 Development Environment](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/sdx-development-environments.html) is required to work with the library.


## How to Download the Repository

To get a local copy of the CHaiDNN repository, configure [git-lfs](https://github.com/git-lfs/git-lfs/wiki/Installation) and then, clone this repository to the local system with the following command:
``` sh
git clone https://github.com/Xilinx/CHaiDNN.git CHaiDNN
```
Where `CHaiDNN` is the name of the directory where the repository will be stored on the local system. This command needs to be executed only once to retrieve the latest version of the CHaiDNN library.



<details>
<summary><big><strong>GitHub Repository Structure</strong></big></summary>

```sh
CHaiDNN/
|
|-- CONTRIBUTING.md
|-- LICENSE
|-- README.md
|-- SD_Card
|   |-- lib
|   |-- cblas
|   |-- images
|   |-- opencv
|   |-- protobuf
|   |-- zcu102
|   `-- zcu104
|-- design
|   |-- build
|   |-- conv
|   |-- deconv
|   |-- pool
|   `-- wrapper
|-- docs
|   |-- API.md
|   |-- BUILD_USING_SDX_GUI.md
|   |-- CONFIGURABLE_PARAMS.md
|   |-- CUSTOM_PLATFORM_GEN.md
|   |-- HW_SW_PARTITIONING.md
|   |-- MODELZOO.md
|   |-- PERFORMANCE_SNAPSHOT.md
|   |-- QUANTIZATION.md
|   |-- RUN_NEW_NETWORK.md
|   |-- SOFTWARE_LAYER_PLUGIN.md
|   |-- SUPPORTED_LAYERS.md
|   `-- images
|-- software
|   |-- bufmgmt
|   |-- checkers
|   |-- common
|   |-- custom
|   |-- example
|   |-- imageread
|   |-- include
|   |-- init
|   |-- interface
|   |-- scheduler
|   |-- scripts
|   |-- swkernels
|   `-- xtract
`-- tools
    |-- SETUP_TOOLS.md
    `-- tools.zip

```
</details>

## Run Inference

<details>
<summary><strong>Using Pre-built binaries</strong></summary>
<a name="Pre-built"></a>

To run inference on example networks, follow these steps:
1. Download the example network [6-bit GoogleNet with Xilinx Quantization scheme](https://www.xilinx.com/member/forms/download/xef.html?filename=GoogleNetWithoutLRN_XQ.zip). More networks are available as part of the [ModelZoo](./docs/MODELZOO.md).
1. Place the downloaded and unzipped contents at "`SD_Card/models`" directory. Create `SD_Card/models` directory if not present already.

1. Copy the required contents of "`SD_Card`" folder into a SD-Card.
	- opencv
	- protobuf
	- cblas
	- images
	- bit-stream, boot loader, lib & executables (either from `SD_Card/zcu102` or `SD_Card/zcu104`)
1. Insert the SD-Card and power ON the board.

	>**:pushpin: NOTE:**  A serial port emulator (Teraterm/Minicom) is required to interface the user commands to the board

1. Attach a USB-UART cable from the board to the host PC. Set the UART serial port to
	```
	Baud rate: 115200
	Data: 8 bit
	Parity: none
	Stop: 1 bit
	Flow control: none
	```
1. After boot sequence, set LD_LIBRARY_PATH env variable.
    ``` sh
    export OPENBLAS_NUM_THREADS=2
    export LD_LIBRARY_PATH=lib/:opencv/arm64/lib/:protobuf/arm64/lib:cblas/arm64/lib
    ```
1. Create a folder "`out`" inside the network directory to save the outputs
    	``` sh
       cd /mnt
       mkdir models/<network>/out
       ```
1. Execute "`*.elf`" file to run inference

    - The format for running these example networks is described below:
      ```sh
      ./<example network>.elf <quantization scheme> <bit width> <img1_path> <img2_path>
      ```
    - For GoogleNet 6-bit inference with Xilinx quantization scheme execute the following
      ```sh
      ./googlenet.elf Xilinx 6 images/camel.jpg images/goldfish.JPEG
      ```	  
1. Sync after execution		
	``` sh
	cd /
	sync
	umount /mnt
	```
1. Output will be written into text file inside respective output folders.
	```
	Ex : models/<network>/out
	```
>**:pushpin: NOTE:**  Failing to run `sync` might corrupt the file system and cause crash on subsequent runs.

>**:pushpin: NOTE:**  For running inference on a new network, please follow the instructions in [Run new Network using CHaiDNN](./RUN_NEW_NETWORK.md).

</details>

<details>
<summary><strong>Build from Source</strong></summary>

CHaiDNN can be built using Makefiles OR using SDx IDE. The below steps describe how to build CHaiDNN using Makefiles. For steps to build using SDx IDE, see the instructions in [Build using SDx IDE](./docs/BUILD_USING_SDX_GUI.md).

<details>
<summary><strong>Build CHaiDNN Hardware</strong></summary>

Please follow the steps to build the design for zcu102 (ZU9 device based board)

1. Please generate a custom platform with 1x and 2x clocks using the steps described [here](./docs/CUSTOM_PLATFORM_GEN.md). With Chai-v2, we now have the DSPs operating at twice the frequency of the rest of the core.


1. Go to `CHaiDNN/design/build` folder.


1. Set SDx tool environment
   - For BASH:
	 ```sh
	 source <SDx Installation Dir>/installs/lin64/SDx/2018.2/settings64.sh
	 ```
   - For CSH
	 ```sh
	 source <SDx Installation Dir>/installs/lin64/SDx/2018.2/settings64.csh
	 ```
1. To build the design, run Makefile. (By default this will build 1024 DSP design @ 200/400 MHz)
	 ```sh
	 make ultraclean
	 make
	 ```

   >**:pushpin: NOTE:**  
   > - To build `DietChai`, run `make DIET_CHAI_Z=1`. This builds a design with 128 compute DSPs and 64-bit AXI interface. Run `make DIET_CHAI_ZUPLUS=1` to build a design with 128 compute DSPs and 128-bit AXI interface.
   > - To exclude deconv Kernel, set `DECONV_ENABLE=0` in Makefile. Default is `DECONV_ENABLE=1`.
   > - To exclude Pool Kernel, set `POOL_ENABLE=0` in Makefile. With this setting, Pooling functionality embedded in Convolution accelerator is used. Default is `POOL_ENABLE=1`.
   > - When building `DietChai`, do not change `POOL_ENABLE`, `DECONV_ENABLE` values in Makefile.

1. After the build is completed, copy the `libxlnxdnn.so` file and other build files (`BOOT.BIN`, `image.ub` and `_sds` directory) inside `build/sd_card` to `SD_Card` directory.
	 ```sh
	 make copy
	 ```
1. The hardware setup is now ready.


  >**:pushpin: NOTE:**  
  > - The 1024 DSP config was timing closed at 250/500Mhz with an iterative synthesis and P&R strategy. In the first iteration, the design was taken through the SDx flow (all the way till the bitstream Gen) at 200/400Mhz. In the second iteration the post-routed design from the first iteration was re-routed at 250/500Mhz. We believe that this is a general strategy that can be applied for other configs also. We would definitely like to hear from you on this if you are able to crank the frequency further up on other configs with this strategy.
  > - Please note that when you try building some of the configs that are mentioned in the [performance table](./docs/PERFORMANCE_SNAPSHOT.md), you might see some negative slack reported by the tools but we encourage you to try the bitstreams generated on hardware for functionality. These timing closure issues can be cleaned up with some special synthesis and P&R strategies. (You are welcome to try the timing-closure strategies that have worked for you in the past on other designs.)
 

</details>

<details>
<summary><strong>Build CHaiDNN Software</strong></summary>

Follow the steps to compile the software stack.

1. Copy `libxlnxdnn.so` to `SD_Card/lib` directory. The `libxlnxdnn.so` file can be found in the `design/build/sd_card` directory once the HW build is finished. You can skip this step if have already copied the `libxlnxdnn.so` file to the suggested directory.

1. Set the SDx tool environment.
   - CSH
	 ```sh
	 source <SDx Installation Dir>/installs/lin64/SDx/2018.2/settings64.csh
	 ```     
   - BASH
	 ```sh
	 source <SDx Installation Dir>/installs/lin64/SDx/2018.2/settings64.sh
	 ```

1. Go to the `software` directory. This contains all the files to generate software libraries (.so).    
	 ```sh
	 cd <path to CHaiDNN>/software
	 ```

1. Go to `scripts` directory, open `Makefile` and update the `SDx_BUILD_PATH` variable. See example below.
	 ```
	 SDx_BUILD_PATH = <SDx Installation Dir>/installs/lin64/SDx/2018.2
	 ```

1. Now run the following commands.

	 ```sh
	 make ultraclean
	 make
	 ```

   >**:pushpin: NOTE:**  
   > - To build `DietChai`, run `make DIET_CHAI_Z=1`. This builds a design with 128 compute DSPs and 64-bit AXI interface. Run `make DIET_CHAI_ZUPLUS=1` to build a design with 128 compute DSPs and 128-bit AXI interface.
   > - To exclude deconv Kernel, set `DECONV_ENABLE=0` in Makefile. Default is `DECONV_ENABLE=1`.
   > - To exclude Pool Kernel, set `POOL_ENABLE=0` in Makefile. With this setting, Pooling functionality embedded in Convolution accelerator is used. Default is `POOL_ENABLE=1`.
   > - When building `DietChai`, do not change `POOL_ENABLE`, `DECONV_ENABLE` values in Makefile.

   >**:pushpin: NOTE:**  Ensure that the software and the hardware are build with the same settings.

1. Make will copy all executables to `SD_Card` directory and all `.so` files to `SD_Card/lib` directory.

1. Now, we are set to run inference. Follow the steps mentioned in "[run inference using pre-build binaries](./README.md#Pre-built)"
</details>
</details>


## Additional Resources and Support

For questions and to get help on this project or your own projects, visit the [CHaiDNN Github Issues](https://github.com/Xilinx/CHaiDNN/issues).

<details>
<summary><strong>License and Contributing to the Repository</strong></summary>
	
The source for this project is licensed under the [Apache License 2.0](./LICENSE)

To contribute to this project, follow the guidelines in the [Repository Contribution README](./CONTRIBUTING.md)
</details>
<details>
<summary><strong>Acknowledgements</strong></summary>

- [Xilinx Inc.](http://www.xilinx.com)
- [Caffe | Berkeley Vision and Learning Center](http://caffe.berkeleyvision.org/)
- [Protocol Buffers | Google](https://developers.google.com/protocol-buffers/)
- [CBLAS](http://www.openblas.net/)
- [OpenCV](https://opencv.org/)

</details>
<details>
<summary><strong>Revision History</strong></summary>

Date      | Readme Version | Release Notes			| Tool Version
----------|----------------|------------------------|-------------
Feb, 2018 | 1.0            | Initial Xilinx release | SDx-2017.4
June, 2018 | 2.0            | CHaiDNN-v2 | SDx-2018.2
</details>
<details>
<summary><strong>Deprecated Features</strong></summary>

- 16-bit activations
</details>

<hr/>
<p align="center"><sup>Copyright&copy; 2018 Xilinx</sup></p>
