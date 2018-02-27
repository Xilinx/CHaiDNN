.PHONY: all

LIBRARY = libxstack.so

SDSFLAGS = -D__SDSOC=1

ARM_CXX = aarch64-linux-gnu-g++ ${SDSFLAGS}

IDIRS = -I$(SDx_BUILD_PATH)/target/aarch64-linux/include/ -I$(OPENCV_DIR)/include/ -I$(SDx_BUILD_PATH)/../../Vivado/2017.4/include

OLEVEL = -O3

CFLAGS  = -c ${IDIRS} ${OLEVEL} 
CFLAGS1 = -c ${IDIRS}

all: ${LIBRARY}

scheduler.o: ../scheduler/xi_scheduler.cpp
	${ARM_CXX} ${CFLAGS} -fPIC $< -o $@

xi_utils.o: ../scheduler/xi_utils.cpp
	${ARM_CXX} ${CFLAGS} -fPIC $< -o $@

xi_image_read.o: ../imageread/xi_input_image.cpp
	${ARM_CXX} ${CFLAGS} -fPIC $< -o $@
	
xi_buff_mgmt.o: ../bufmgmt/xi_buf_mgmt.cpp
	${ARM_CXX} ${CFLAGS} -fPIC $< -o $@
	
xi_init.o: ../init/xi_init.cpp
	${ARM_CXX} ${CFLAGS1} -fPIC $< -o $@

checkers.o: ../checkers/checkers.cpp
	${ARM_CXX} ${CFLAGS} -fPIC $< -o $@
	
xi_thread_routines.o: ../scheduler/xi_thread_routines.cpp
	${ARM_CXX} ${CFLAGS} -fPIC $< -o $@

xi_kernels.o: ../common/xi_kernels.cpp
	${ARM_CXX} ${CFLAGS} -fPIC $< -o $@

xchange_structs.o: ../include/xchange_structs.cpp
	${ARM_CXX} ${CFLAGS} -fPIC $< -o $@
	
xi_nms_top.o: ../swkernels/xi_nms_top.cpp
	${ARM_CXX} ${CFLAGS} -fPIC $< -o $@
	
xi_norm_top.o: ../swkernels/xi_norm_top.cpp
	${ARM_CXX} ${CFLAGS} -fPIC $< -o $@

xi_permute_top.o: ../swkernels/xi_permute_top.cpp
	${ARM_CXX} ${CFLAGS} -fPIC $< -o $@

xi_swsoftmax_top.o: ../swkernels/xi_swsoftmax_top.cpp
	${ARM_CXX} ${CFLAGS} -fPIC $< -o $@
	
xi_crop.o: ../swkernels/xi_crop.cpp
	${ARM_CXX} ${CFLAGS} -fPIC $< -o $@

xi_interface.o: ../interface/xi_interface.cpp
	$(ARM_CXX) -c -fPIC -I$(PB_ARM_DIR)/include $(IDIRS) $< -o $@

${LIBRARY}: scheduler.o xi_utils.o xi_thread_routines.o xi_image_read.o xi_kernels.o xi_buff_mgmt.o xi_init.o checkers.o xchange_structs.o xi_nms_top.o xi_norm_top.o xi_permute_top.o xi_swsoftmax_top.o xi_crop.o xi_interface.o
	${ARM_CXX} ${OLEVEL} -shared $^ -o $@

copy :
	cp libxstack.so ../../SD_Card/lib/

clean:
	${RM} ${LIBRARY} *.o *.so
