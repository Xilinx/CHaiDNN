.PHONY: all

LIBRARY = libxstack.so

SDSFLAGS = -D__SDSOC=1

IDIRS = -I$(OPENCV_INC) -I$(ARM_INC) -I$(SDX_INC)

OLEVEL = -O3

CFLAGS  = -c ${IDIRS} ${OLEVEL} 
CFLAGS1 = -c ${IDIRS}

CFLAGS += $(WRAPPER_FLAGS) ${SDSFLAGS}
CFLAGS1 += $(WRAPPER_FLAGS) ${SDSFLAGS}

all: ${LIBRARY}

scheduler.o: ../scheduler/xi_scheduler.cpp
	${ARM_CXX} ${CFLAGS1} -fPIC $< -o $@

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

xi_fc_top.o: ../swkernels/xi_fc_top.cpp
	${ARM_CXX} ${CFLAGS} -fPIC -I$(BLAS_ARM_DIR)/include -L$(BLAS_ARM_DIR)/lib $< -o $@ -lpthread -lopenblas

xi_interface.o: ../interface/xi_interface.cpp
	$(ARM_CXX) ${CFLAGS} -c -fPIC -I$(PB_ARM_DIR)/include $(IDIRS) $< -o $@

xi_readwrite_util.o: ../interface/xi_readwrite_util.cpp
	$(ARM_CXX) ${CFLAGS} -c -fPIC -I$(PB_ARM_DIR)/include $(IDIRS) $< -o $@

kernelinfo_class.o: ../common/kernelinfo_class.cpp
	$(ARM_CXX) ${CFLAGS} -c -fPIC -I$(PB_ARM_DIR)/include $(IDIRS) $< -o $@

custom_class.o: ../custom/custom_class.cpp
	$(ARM_CXX) ${CFLAGS} -c -fPIC -I$(PB_ARM_DIR)/include $(IDIRS) $< -o $@

xi_format_converter_top.o: ../swkernels/xi_format_converter_top.cpp
	$(ARM_CXX) ${CFLAGS} -c -fPIC -I$(PB_ARM_DIR)/include $(IDIRS) $< -o $@
	
${LIBRARY}: scheduler.o xi_utils.o xi_thread_routines.o xi_kernels.o xi_buff_mgmt.o xi_init.o checkers.o xchange_structs.o xi_nms_top.o xi_norm_top.o xi_permute_top.o xi_swsoftmax_top.o xi_crop.o xi_fc_top.o xi_interface.o xi_readwrite_util.o kernelinfo_class.o custom_class.o xi_format_converter_top.o
	${ARM_CXX} ${OLEVEL} -shared $^ -o $@

copy :
	cp *.so $(LIB_DIR)

clean:
	${RM} ${LIBRARY} *.o *.so
