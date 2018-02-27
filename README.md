# **CHaiDNN**

The CHaiDNN is Xilinx Deep Neural Network library for acceleration of deep neural networks on Xilinx Ultrascale MPSoCs. It is designed for maximum compute efficiency at 16-bit integer data type.

The design goal of CHaiDNN is to achieve best accuracy with maximum performance. The inference on CHaiDNN works in fixed point domain for better performance. All the feature maps and trained parameters are converted from single precision to fixed point based on the precison parameters specified by the user. The precision parameters can vary a lot depending upon the network, datasets, or even across layers in the same network. Accuracy of a network depends on the precision parameters used to represent the feature maps and trained parameters. Well-crafted precision parameters are expected to give accuracy similar to accuracy obtained
from a single precision model. 

## **How to Download the Repository**

To get a local copy of the CHaiDNN repository, clone this repository to the local system with the following command:
``` sh
git clone https://github.com/Xilinx/CHaiDNN.git CHaiDNN
```
Where `CHaiDNN` is the name of the directory where the repository will be stored on the local system. This command needs to be executed only once to retrieve the latest version of the CHaiDNN library.

## **Hardware and Software Requirements**

The CHaiDNN library is designed to work with Zynq Ultrascale+ MPSoCs. The library has been verified on zcu102 board. Xilinx [SDSoC 2017.4 Development Environment](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/sdx-development-environments.html) is required to work with the library.

## **Run Inference Using Pre-built binaries**

Follow the below steps to run inference on example networks.

1. Download the example networks that need to to be benchmarked and place the unzipped contents at `SD_Card/models` directory.
	* [AlexNet](https://www.xilinx.com/content/dam/xilinx/open-source/AlexNet.zip)
	* [GoogleNet](https://www.xilinx.com/content/dam/xilinx/open-source/GoogleNet.zip)
	* [AlexNetFCN](https://www.xilinx.com/content/dam/xilinx/open-source/AlexNetFCN.zip)
	* [SSD](https://www.xilinx.com/content/dam/xilinx/open-source/SSD.zip)
	* [VGG-16](https://www.xilinx.com/content/dam/xilinx/open-source/vgg16.zip)
	* [Advanced ResNet50](https://www.xilinx.com/content/dam/xilinx/open-source/AdvResnet50.zip)
2. Verify the contents of `SD_Card` with the below checklist.
	* `_sds` folder
	* `models` folder
	* `lib` folder
	* `opencv_arm64` folder
	* `protobuf_arm64` folder
	* `BOOT.BIN` file
	* `image.ub` file
	* `.elf` file(s)

3. Copy the contents of `SD_Card` folder into a SD-Card.
4. Insert the SD-Card and power on the board.

> **Note:** A serial port emulator (Teraterm/Minicom) is required to interface the user commands to the board

5. Attach a USB-UART cable from the ZCU102 board to the host PC. Set the UART serial port to
	```
	Baud rate: 115200
	Data: 8 bit
	Parity: none
	Stop: 1 bit
	Flow control: none
	```

6. After boot, go to `/mnt` directory.

	``` sh
	cd /mnt
	```

7. Export library paths.
	```	sh
	export LD_LIBRARY_PATH=lib/:opencv_arm64/lib/:protobuf_arm64/lib
	```

8. Execute the commands to run inference for the networks 

-  AlexNet	
	```
	./alexnet.elf
	```
- GoogleNet
	```
	./googlenet.elf
	```

- VGG16
	```   
	./vgg.elf
	```
- AlexNetFCN
	```
	./alexnetfcn.elf
	```
- SSD
	```
	./ssd.elf
	```	
- Advanced ResNet50
	```
	./adv_resnet50.elf
	```	
9. Sync after execution
	``` sh
	cd /
	sync
	umount /mnt
	```	
10.	Output will be written into text file inside respective output folders.
	```	
	Ex : models/<network>/out
	```

**Note:** Failing to run `sync` might corrupt the file system and cause crash on subsequent runs.

**Note:** For running inference on a new network, please follow the instructions in [Run new Network using CHaiDNN](./RUN_NEW_NETWORK.md).

## **Build from Source**

CHaiDNN can be built using Makefiles OR using SDx IDE. The below steps describe how to build CHaiDNN using Makefiles. For steps to build using SDx IDE, check the instructions in [Build using SDx IDE](./BUILD_USING_SDX_GUI.md). 

## 1. Build CHaiDNN Hardware from source
Please follow the steps to build the design for zcu102

1. 	Go to the local `CHaiDNN` folder.

2. 	Go to `design` directory. This contains all the design files required to build the design.

3. 	Go to `scripts/kernel` directory. This contains the Makefile to compile the design.

4. 	Source the SDx-2017.4 path. Please check the example command below.

- For BASH:
	```sh
	source <SDx Installation Dir>/installs/lin64/SDx/2017.4/settings64.sh
	```
- For CSH
	```sh
	source <SDx Installation Dir>/installs/lin64/SDx/2017.4/settings64.csh
	```
	
5.	Run Makefile. This will build the design.
	```sh
	make ultraclean
	make
	```	
6.	After the build is completed, copy the `libxlnxdnn.so` file and other build files (`BOOT.BIN`, `image.ub` and `_sds` directory) inside `scripts/kernel/sd_card` to `SD_Card` directory.
	```sh
	make copy
	```
8. The hardware setup is now ready. 

## 2. Build CHaiDNN Software from source

Follow the steps to compile the software stack.

1. Copy `libxlnxdnn.so` to `SD_Card/lib` directory. This will be present inside `design/scripts/kernel/sd_card` directory once the HW build is finished.

	> Skip this step if already copied.

2. Source SDx path.   
* CSH
	```sh
	source <SDx Installation Dir>/installs/lin64/SDx/2017.4/settings64.csh
	```     
* BASH
	```sh
	source <SDx Installation Dir>/installs/lin64/SDx/2017.4/settings64.sh
	```

3. Go to the `software` directory. This contains all the files to generate software libraries (.so).    
	```sh
	cd <path to CHaiDNN>/software
	```

4. Go to `scripts` directory, open `Makefile` and update the `SDx_BUILD_PATH` variable. Check example below.
	```
	SDx_BUILD_PATH = <SDx Installation Dir>/installs/lin64/SDx/2017.4
	``` 

5. Now run the following commands.

	```sh
	cd scripts
	make ultraclean
	make
	```

6. Make will copy all executables to `SD_Card` directory and all `.so` files to `SD_Card/lib` directory.

7. Now all the files are available to run inference on zcu102. 

- Download the example models and copy the unzipped contents to `SD_Card/models` folder.
- Copy the contents of `SD_Card` directory to SD-Card

8. Insert the SD-Card in the board card slot and switch it ON.

9. Upon successful boot, execute the following commands.    
	```sh
	cd /mnt
	export LD_LIBRARY_PATH=opencv_arm64/lib/:lib/:protobuf_arm64/lib/ 
	./<name>.elf 
	```

10. Make sure to sync before removing SD-Card from the board OR power off the board. 
	```sh
	cd /
	sync
	umount /mnt
	```
	
## **CHaiDNN-2017.4 Performance**

Network | Images/ Second @ 200 MHz (Batch Size =1 on zcu102)
--------|-----------------------------------------
GoogleNet| 72.97
AlexNet(with FC)| 55.11
AlexNet(without FC)| 160.03
VGG-16| 9.68
AlexNet-FCN| 7.59
SSD | 4.64
[AdvResNet](https://arxiv.org/abs/1603.05027)| 21.18
[ResNet50](https://arxiv.org/abs/1512.03385)| 24.59
[ResNet34](https://arxiv.org/abs/1512.03385)| 37.68


**IMP : Only Performance Projections for Legacy *ResNet34* and *ResNet50*.** 

## **CHaiDNN-2017.4 Resource Utilization**

Parameter | Available on zcu102 | Used | % Utilization
----------|---------------------|------|--------------
DSP | 2520 | 1939 | 76.94
BRAM | 912 | 828.5 | 90.84
LUT | 274080 | 135033 | 49.27
FF | 548160 | 176146 | 32.13

## **Other Information**

For CHaiDNN API Document click [CHaiDNN API document](./API_DOCUMENT.md)

For Running Inference on other Networks [Running Inference for new networks](./RUN_NEW_NETWORK.md).

For information on CHaiDNN quantization parameters check [Quantization Parameters User Guide](./QUANTIZATION_PARAMETERS_UG.md)

For insturctions on creating the SDx GUI Project for CHaiDNN click [Build using SDx IDE](./BUILD_USING_SDX_GUI.md)

For SDSoC User Guide click [SDSoC User Guides](https://www.xilinx.com/support/documentation/sw_manuals/xilinx2017_4/ug1027-sdsoc-user-guide.pdf)

## **Important Notes**
* none

## **Support**

For questions and to get help on this project or your own projects, visit the [SDSoC Forums](https://forums.xilinx.com/t5/SDSoC-Development-Environment/bd-p/sdsoc).

## **Licence and Contributing to the Repository**
The source for this project is licensed under the [Apache License 2.0](./LICENSE)

To contribute to this project, follow the guidelines in the [Repository Contribution README](./CONTRIBUTING.md)

## **Acknowledgements**

- [Xilinx Inc.](http://www.xilinx.com)
- [Caffe | Berkeley Vision and Learning Center](http://caffe.berkeleyvision.org/)
- [Protocol Buffers | Google](https://developers.google.com/protocol-buffers/)

## **Revision History**

Date      | Readme Version | Release Notes			| Tool Version
----------|----------------|------------------------|-------------
Feb, 2018 | 1.0            | Initial Xilinx release | SDx-2017.4

## **Change Log**

1. First Release

## **Known Issues**
* none
