.PHONY: all

IDIRS = -I$(OPENCV_INC) -I$(ARM_INC) -I$(SDX_INC)

OPENCV_LIBS = -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -ldl -lrt -llzma -ltiff -lpng16 -lz -ljpeg -lwebp
#enable following for 32-bit libs
#OPENCV_LIBS = -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -ldl -lrt

LDIRS = -L$(LIB_DIR) -L$(BLAS_ARM_DIR)/lib
LLIBS = -lprotobuf -lpthread -lxstack -lxlnxdnn -lparser_arm -lopenblas

CFLAGS = -std=c++0x -Wno-Wwrite-strings
CFLAGS_ARM = -std=c++11 -D__SDSOC=1 -Wno-write-strings -D__APIMODE=1
CFLAGS_ARM += $(WRAPPER_FLAGS)


all: resnet50.elf googlenet.elf alexnetfcn.elf alexnet.elf ssd.elf vgg.elf sw_layer_plugin.elf eval.elf


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

resnet50.elf : ../example/resnet50_ex.cpp
	@echo -e "\n## Compiling ResNet50\n"
	$(ARM_CXX) $(CFLAGS_ARM) -L$(PB_ARM_DIR)/lib -I$(PB_ARM_DIR)/include -L$(OPENCV_DIR)/lib -I$(OPENCV_DIR)/include $(IDIRS) $(LDIRS) $(LLIBS) $(OPENCV_LIBS) $^ -o $@
	@echo


sw_layer_plugin.elf : ../example/sw_layer_plugin.cpp

	@echo -e "\n## Compiling Custom Layer example\n"
	$(ARM_CXX) $(CFLAGS_ARM) -L$(PB_ARM_DIR)/lib -I$(PB_ARM_DIR)/include -L$(OPENCV_DIR)/lib -I$(OPENCV_DIR)/include $(IDIRS) $(LDIRS) $(LLIBS) $(OPENCV_LIBS) $^ -o $@
	@echo	
	
eval.elf : ../example/eval_ex.cpp

	@echo -e "\n## Compiling Custom Layer example\n"
	$(ARM_CXX) $(CFLAGS_ARM) -L$(PB_ARM_DIR)/lib -I$(PB_ARM_DIR)/include -L$(OPENCV_DIR)/lib -I$(OPENCV_DIR)/include $(IDIRS) $(LDIRS) $(LLIBS) $(OPENCV_LIBS) $^ -o $@
	@echo

copy :
	cp *.elf $(ELF_DIR)

clean :
	rm -rf *.elf
