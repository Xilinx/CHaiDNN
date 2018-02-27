# (c) Copyright 2013 - 2016 Xilinx, Inc. All rights reserved. 
# 
# This file contains confidential and proprietary information of Xilinx, Inc. and
# is protected under U.S. and international copyright and other intellectual
# property laws.
# 
# DISCLAIMER 
# This disclaimer is not a license and does not grant any rights to the materials
# distributed herewith. Except as otherwise provided in a valid license issued to
# you by Xilinx, and to the maximum extent permitted by applicable law: (1) THESE
# MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS, AND XILINX HEREBY
# DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY,
# INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, OR
# FITNESS FOR ANY PARTICULAR PURPOSE; and (2) Xilinx shall not be liable (whether
# in contract or tort, including negligence, or under any other theory of
# liability) for any loss or damage of any kind or nature related to, arising
# under or in connection with these materials, including for any direct, or any
# indirect, special, incidental, or consequential loss or damage (including loss
# of data, profits, goodwill, or any type of loss or damage suffered as a result
# of any action brought by a third party) even if such damage or loss was
# reasonably foreseeable or Xilinx had been advised of the possibility of the
# same.
# 
# CRITICAL APPLICATIONS
# Xilinx products are not designed or intended to be fail-safe, or for use in any
# application requiring fail-safe performance, such as life-support or safety
# devices or systems, Class III medical devices, nuclear facilities, applications
# related to the deployment of airbags, or any other applications that could lead
# to death, personal injury, or severe property or environmental damage
# (individually and collectively, "Critical Applications"). Customer assumes the
# sole risk and liability of any use of Xilinx products in Critical Applications,
# subject only to applicable laws and regulations governing limitations on product
# liability.
# 
# THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT
# ALL TIMES. 

LIBRARY = libxlnxdnn.so
  
PLATFORM = zcu102
#VERBOSE = -verbose

SDSFLAGS = -sds-pf ${PLATFORM} -xp param:compiler.skipTimingCheckAndFrequencyScaling=1 \
	-sds-hw XiConvolutionTop ../../src/conv/example/xi_convolution_top.cpp -clkid 3 -hls-tcl ./config_core2.tcl -sds-end -dmclkid 3 \
	-sds-hw PoolTop ../../src/pool/example/pooling_layer_dp_2xio_top.cpp -clkid 3 -sds-end -dmclkid 3 \
	-sds-hw xiSgemvTop ../../src/gemv/example/xi_sgemv_top.cpp -clkid 3 -sds-end -dmclkid 3 \
	-sds-hw XiDeconvTop ../../src/deconv/example/xi_deconv_top.cpp -clkid 3 -sds-end -dmclkid 3 \
#${VERBOSE} 

CC = sds++ ${SDSFLAGS}

IDIRS = -I. -I../../include
CFLAGS = -Wall -O3 -c ${IDIRS} -D__SDSOC=1 
LFLAGS = -O3

.PHONY: all

all: ${LIBRARY}

xi_convolution_top.o: ../../src/conv/example/xi_convolution_top.cpp
	${CC} ${CFLAGS} -fPIC $< -o $@

pooling_layer_dp_2xio_top.o: ../../src/pool/example/pooling_layer_dp_2xio_top.cpp
	${CC} ${CFLAGS} -fPIC $< -o $@

xi_sgemv_top.o: ../../src/gemv/example/xi_sgemv_top.cpp
	${CC} ${CFLAGS} -fPIC $< -o $@
	
xi_deconv_top.o: ../../src/deconv/example/xi_deconv_top.cpp
	${CC} ${CFLAGS} -fPIC $< -o $@

dnn_wrapper.o: ../../utils/common/dnn_wrapper.cpp
	${CC} ${CFLAGS} -fPIC $< -o $@


${LIBRARY}: xi_convolution_top.o pooling_layer_dp_2xio_top.o xi_sgemv_top.o xi_deconv_top.o dnn_wrapper.o
	${CC} -shared $^ -o $@

copy:
	cp sd_card/libxlnxdnn.so ../../../SD_Card/lib/libxlnxdnn.so
	cp sd_card/BOOT.BIN ../../../SD_Card/BOOT.BIN
	cp sd_card/image.ub ../../../SD_Card/image.ub
	cp -r sd_card/_sds ../../../SD_Card/

clean:
	${RM} ${LIBRARY} xi_convolution_top.o pooling_layer_dp_2xio_top.o xi_sgemv_top.o xi_deconv_top.o dnn_wrapper.o

ultraclean: clean
	${RM} ${LIBRARY}.bit 
	${RM} -rf _sds sd_card
