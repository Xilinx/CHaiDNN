.PHONY: all

ARM_CXX = aarch64-linux-gnu-g++

IDIRS = -I$(SDx_BUILD_PATH)/target/aarch64-linux/include

OPENCV_LIBS = -lopencv_core -llzma -ltiff -lpng16 -lz -ljpeg -lopencv_imgproc -lopencv_imgcodecs -ldl -lrt -lwebp

LDIRS = -L../../SD_Card/lib
LLIBS = -lprotobuf -lpthread -lxstack -lxlnxdnn -lparser_arm
CFLAGS = -std=c++0x -Wno-Wwrite-strings

CFLAGS_ARM = -std=c++11 -D__SDSOC=1 -Wno-write-strings 

all: googlenet.elf alexnet.elf alexnetfcn.elf ssd.elf vgg.elf adv_resnet50.elf

googlenet.elf : ../example/googlenet_ex.cpp
	@echo -e "\n## Compiling GoogleNet \n"
	$(ARM_CXX) $(CFLAGS_ARM) -L$(PB_ARM_DIR)/lib -I$(PB_ARM_DIR)/include -L$(OPENCV_DIR)/lib -I$(OPENCV_DIR)/include $(IDIRS) $(LDIRS) $(LLIBS) $(OPENCV_LIBS) $^ -o $@
	@echo

alexnetfcn.elf : ../example/alexnetfcn_ex.cpp
	@echo -e "\n## Compiling AlexNet-FCN\n"
	$(ARM_CXX) $(CFLAGS_ARM) -L$(PB_ARM_DIR)/lib -I$(PB_ARM_DIR)/include -L$(OPENCV_DIR)/lib -I$(OPENCV_DIR)/include $(IDIRS) $(LDIRS) $(LLIBS) $(OPENCV_LIBS) $^ -o $@
	@echo

alexnet.elf : ../example/alexnet_ex.cpp
	@echo -e "\n## Compiling AlexNet\n"
	$(ARM_CXX) $(CFLAGS_ARM) -L$(PB_ARM_DIR)/lib -I$(PB_ARM_DIR)/include -L$(OPENCV_DIR)/lib -I$(OPENCV_DIR)/include $(IDIRS) $(LDIRS) $(LLIBS) $(OPENCV_LIBS) $^ -o $@
	@echo

ssd.elf : ../example/ssd_ex.cpp
	@echo -e "\n## Compiling SSD\n"
	$(ARM_CXX) $(CFLAGS_ARM) -L$(PB_ARM_DIR)/lib -I$(PB_ARM_DIR)/include -L$(OPENCV_DIR)/lib -I$(OPENCV_DIR)/include $(IDIRS) $(LDIRS) $(LLIBS) $(OPENCV_LIBS) $^ -o $@
	@echo

vgg.elf : ../example/vgg_ex.cpp
	@echo -e "\n## Compiling VGG16\n"
	$(ARM_CXX) $(CFLAGS_ARM) -L$(PB_ARM_DIR)/lib -I$(PB_ARM_DIR)/include -L$(OPENCV_DIR)/lib -I$(OPENCV_DIR)/include $(IDIRS) $(LDIRS) $(LLIBS) $(OPENCV_LIBS) $^ -o $@
	@echo

adv_resnet50.elf : ../example/advanced_resnet50_ex.cpp
	@echo -e "\n## Compiling Advanced ResNet50\n"
	$(ARM_CXX) $(CFLAGS_ARM) -L$(PB_ARM_DIR)/lib -I$(PB_ARM_DIR)/include -L$(OPENCV_DIR)/lib -I$(OPENCV_DIR)/include $(IDIRS) $(LDIRS) $(LLIBS) $(OPENCV_LIBS) $^ -o $@
	@echo

copy :
	cp *.elf ../../SD_Card/

clean :
	rm -rf *.elf
