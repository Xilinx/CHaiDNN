<table style="width:100%">
<tr>
<th width="100%" colspan="6"><img src="https://www.xilinx.com/content/dam/xilinx/imgs/press/media-kits/corporate/xilinx-logo.png" width="30%"/><h1>CHaiDNN-v2</h2>
</th>
</tr>

</table>

# Setting up the XportDNN tool for CHaiDNN

To set up the XportDNN tool for CHaiDNN, perform the following steps:


1. Extract the `tools.zip` archive file. The file contains tools_binaries, example network required for [XportDNN](../docs/QUANTIZATION.md#xport).
   ```
   $ unzip tools.zip
   ```

   The folder should look as below:
   ```
   $ ls
   setup_chai_tools.sh  tools.zip			 XportDNN.pyc
   tools_binaries	     xilinx_quant_example_model
   ```

3. Run `setup_chai_tools.sh` to prepare the environment for the binaries to run successfully. The script downloads the dependency packages from the internet and installs it within the `CHaiDNN` virtual-environment.

	>**:pushpin: NOTE:** Ensure that the system is connected to the internet.
   ```
   $ bash setup_chai_tools.sh
   ```

4. Activate the `chaidnn_tools_ENV` virtualenv.
   ```
   $ source ./chaidnn_tools_ENV/bin/activate
   ```

5. Run `XportDNN.pyc` using the required quantization mode. For more details on usage, see [Quantization User Guide for CHaiDNN](../docs/QUANTIZATION.md#xport).


### Tested Environment
The tool has been tested for its working in the following test environment:
- CHaiDNN(v2)
- OS: Ubuntu 16.04 LTS, 64bit
- x86 CPU
- python 2.7
- pip 10.0.1  
- virtualenv 15.1.0

### Shared Library Dependencies
Use the following syntax to view a list of shared library dependencies for `/bin/bash`.
```
$ ldd /bin/bash
	linux-vdso.so.1 =>  (0x00007ffe73dbb000)
	libtinfo.so.5 => /lib/x86_64-linux-gnu/libtinfo.so.5 (0x00007f6f45220000)
	libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f6f4501c000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f6f44c52000)
	/lib64/ld-linux-x86-64.so.2 (0x0000556bd7301000)
```

<hr/>
<p align="center"><sup>Copyright&copy; 2018 Xilinx</sup></p>
