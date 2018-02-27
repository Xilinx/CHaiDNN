.PHONY : clean

ARM_CXX = aarch64-linux-gnu-g++
CLIBS   = -lprotobuf -lpthread  
CFLAGS  = -g -O2

libparser_arm.so : libcaffeparser_arm.o libxgraph_arm.o libcaffeproto_arm.o libxifuncs_arm.o xtract_opt_arm.o hw_settings_arm.o
	$(ARM_CXX) $(CFLAGS) -shared -fPIC -Wl,-soname,libparser_arm.so -o $@ $^

libcaffeproto_arm.o : ../xtract/caffe.pb.cc ../xtract/caffe.pb.h
	$(ARM_CXX) $(CFLAGS) -c -fPIC -L$(PB_ARM_DIR)/lib -I$(PB_ARM_DIR)/include $< -o $@ $(CLIBS)
	
libcaffeparser_arm.o : ../xtract/caffe_network_parser.cpp
	$(ARM_CXX) $(CFLAGS) -c -fPIC -L$(PB_ARM_DIR)/lib -I$(PB_ARM_DIR)/include $< -o $@ $(CLIBS)
	
hw_settings_arm.o : ../xtract/hw_settings.cpp  	
	$(ARM_CXX) $(CFLAGS) -c -fPIC $< -o $@

xtract_opt_arm.o : ../xtract/xtract_opt.cpp  	
	$(ARM_CXX) $(CFLAGS) -c -fPIC $< -o $@

libxgraph_arm.o : ../xtract/xgraph.cpp ../xtract/xgraph.hpp ../xtract/xparameter.hpp
	$(ARM_CXX) $(CFLAGS) -c -fPIC $< -o $@
	
libxifuncs_arm.o : ../xtract/xi_funcs.cpp ../xtract/xi_funcs.hpp
	$(ARM_CXX) $(CFLAGS) -c -fPIC $< -o $@

copy :
	cp libparser_arm.so ../../SD_Card/lib/
	
clean :
	rm -rf *.o

ultraclean :
	rm -rf *.o *.so
