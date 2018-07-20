.PHONY: all

# Provide Correct SDx Path
SDx_BUILD_PATH = /proj/xbuilds/2018.2_daily_latest/installs/lin64/SDx/2018.2

ARM_CXX = aarch64-linux-gnu-g++
OPENCV_DIR := ../../SD_Card/opencv/arm64
PB_ARM_DIR := ../../SD_Card/protobuf/arm64
BLAS_ARM_DIR := ../../SD_Card/cblas/arm64

# ARM_CXX = arm-linux-gnueabihf-g++
# OPENCV_DIR := ../../SD_Card/opencv/arm32
# PB_ARM_DIR := ../../SD_Card/protobuf/arm32
# BLAS_ARM_DIR := ../../SD_Card/cblas/arm32

OPENCV_INC := $(OPENCV_DIR)/include
ARM_INC := $(SDx_BUILD_PATH)/target/aarch64-linux/include
SDX_INC := $(SDx_BUILD_PATH)/../../Vivado/2018.2/include

LIB_DIR := ../../SD_Card/lib
ELF_DIR := ../../SD_Card

# H/W Kernel enables
DIET_CHAI_Z	     ?= 0
DIET_CHAI_ZUPLUS ?= 0

POOL_ENABLE   ?= 1
DECONV_ENABLE ?= 1

ifeq ($(DIET_CHAI_Z), 1)
	POOL_ENABLE   := 0
	DECONV_ENABLE := 0
endif	

ifeq ($(DIET_CHAI_ZUPLUS), 1)
	POOL_ENABLE   := 0
	DECONV_ENABLE := 0
endif

WRAPPER_FLAGS := -DXI_DIET_CHAI_Z=$(DIET_CHAI_Z) -DXI_DIET_CHAI_ZUPLUS=$(DIET_CHAI_ZUPLUS) -D__POOL_ENABLE__=$(POOL_ENABLE) -D__DECONV_ENABLE__=$(DECONV_ENABLE)

export

# Compile complete CHaiDNN software using single makefile
all:
	@echo -e "\n## Compiling Front & Back End \n"
	make -j4 -f xtract.mk 
	make -f xtract.mk copy
	@echo -e "\n## Compiling Software stack \n"
	make -j4 -f xstack.mk
	make -f xstack.mk copy
	@echo -e "\n## Compiling Examples \n"
	make -j4 -f example.mk
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
resnet50:
	make -f example.mk resnet50.elf
	
# Clean object and elf files
clean :
	rm -rf *.o *.elf /*.so $(LIB_DIR)/libparser_arm.so $(LIB_DIR)/libxstack.so $(ELF_DIR)/*.elf
	
# Clean all binaries
ultraclean :
	rm -rf *.o *.so *.elf $(LIB_DIR)/*.so $(ELF_DIR)/*.elf
