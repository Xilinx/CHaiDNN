# **Create SDx GUI Project for CHaiDNN**
---
## Follow the steps carefully to build `CHaiDNN` using SDx GUI

1. Launch the **SDx development environment** using the desktop icon or the Start menu.

    > The **Workspace Launcher** dialog appears

2.  Click `Browse` to enter a workspace folder used to store your projects (you can use workspace folders to organize your work), then click `OK` to dismiss the Workspace Launcher dialog.

    > The SDx development environment window opens with the **Welcome** tab visible when you create a new workspace. The Welcome tab can be closed by clicking the `X` icon or minimized if you do not wish to use it.

3.  Select `File` → `New` → `Xilinx SDx Project` from the SDx development environment menu bar.
    
    > The **New SDx Project** dialog box opens.
    Select `Application Project` and Click `Next`.

4. Specify the name of the project. For example `CHaiDNN`.

5. Click `Next`.

    > The **Choose Hardware platform** page appears.

6. From the **Choose Hardware platform** page, select `zcu102`. Click `Next`.

    > The **System configuration** page appears.
 
7. Use default settings and click `Next`.

8. Select `Empty Application` and click `Finish`.

9. From `Project Explorer` Pane, Right Click on `src` directory under the created project & Click `Import`.

    > The **Import wizard** appears.

10. Expand `General` option and select `File System`. Click `Next`.

11. Browse & Select the local copy of `CHaiDNN` Git repository. Click `OK`. 

12. Expand `CHaiDNN` and Select `software` & `design` directories. Click `Finish`.

13. All the source files needed to build `CHaiDNN`, can be seen under `src` directory. 

14. Right click on project and select `C/C++ Build Settings`

15. In `SDS++ Compiler` add the following in the `command`
```
sds++ -xp param:compiler.skipTimingCheckAndFrequencyScaling=1 -hls-tcl <path to design>/scripts/kernel/config_core2.tcl
```
> **Note :** `<path to design>` is the path to the project's `src/design` folder.

16. In `Symbols`, add symbol `__SDSOC`

17. In `Directories`, add opencv and protobuf include paths
```
<path to libraries>/opencv_arm64/include
<path to libraries>/protobuf_arm64/include
```
18. In `SDS++ Linker` add the following in the `command`
```
sds++ -xp param:compiler.skipTimingCheckAndFrequencyScaling=1
``` 
19. In `SDS++ Linker`, Select `Libraries` & add the following libs
```
opencv_core;lzma;tiff;png16;z;jpeg;opencv_imgproc;opencv_imgcodecs;dl;rt;webp;protobuf
```  
20. Add library paths
```   
<path to libraries>/opencv_arm64/lib
<path to libraries>/protobuf_arm64/lib
```   
> **Note :** `<path to libraries>` is the path to `SD_Card` directory in local `CHaiDNN` repository.

21. Save and apply changes. Close the settings window.

22. Navigate to `src/software/init` folder in `Project Explorer` Pane. Right click on `xiInit.cpp` and open the properties. 

    > In Optimization, change the `Optimization Level` to None (-O0).
   
23. Apply changes and close the window.
 
24.	Select the HW functions.
    > Navigate to `src/design/conv/example/xi_convolution.cpp` file using SDx explorer, right click on `XiConvolutionTop` and select Toggle HW/SW.
	
    > Navigate to `src/design/gemv/example/au_sgemv_top.cpp` file using SDx explorer, right click on `auSgemvTop` and select Toggle HW/SW.
	
    > Navigate to `src/design/pool/example/pooling_layer_dp_2xio_top.cpp` file using SDx explorer, right click on `PoolTop` and select Toggle HW/SW.
    
    > Navigate to `src/design/deconv/example/xi_deconv_top.cpp` file using SDx explorer, right click on `XiDeconvTop` and select Toggle HW/SW.
	
25. Open `project.sdx` file, change the `Data motion network clock frequency (MHz)` to 200.00 and change the `Hardware Functions clock frequency (MHz)` to 200.00
	
26. Build project.

**IMPORTANT :** Initial release only supports SDx GUI build on Linux machines.