.PHONY: all

# Provide Correct SDx Path
SDx_BUILD_PATH := /proj/xbuilds/2017.4_released/installs/lin64/SDx/2017.4

OPENCV_DIR := ../../SD_Card/opencv_arm64
PB_ARM_DIR := ../../SD_Card/protobuf_arm64

export

# Compile complete CHaiDNN software using single makefile
all:
	@echo -e "\n## Compiling Front & Back End \n"
	make -f xtract.mk 
	make -f xtract.mk copy
	@echo -e "\n## Compiling Software stack \n"
	make -f xstack.mk
	make -f xstack.mk copy
	@echo -e "\n## Compiling Examples \n"
	make -f example.mk
	make -f example.mk copy 
	@echo
	

# Compile only xtract (libparser_arm.so)
xtract:
	@echo -e "\n## Compiling Front & Back End \n"
	make -f xtract.mk
	@echo
# Compile only xstack (libxstack.so)
xstack:
	@echo -e "\n## Compiling Software stack \n"
	make -f xstack.mk
	@echo
# Compile examples (.elf)
example:
	@echo -e "\n## Compiling Examples \n"
	make -f example.mk
	@echo

# Compile GoogleNet example
googlenet:
	make -f example.mk googlenet.elf
# Compile AlexNet example
alexnet:
	make -f example.mk alexnet.elf
# Compile AlexNet-FCN example
alexnetfcn:
	make -f example.mk alexnetfcn.elf
# Compile SSD example
ssd:
	make -f example.mk ssd.elf
# Compile VGG16 example
vgg16:
	make -f example.mk vgg.elf
# Compile Advanced-ResNet50 example
adv_resnet50:
	make -f example.mk adv_resnet50.elf
# Clean object and elf files
clean :
	rm -rf *.o *.elf
# Clean all binaries
ultraclean :
	rm -rf *.o *.so *.elf
