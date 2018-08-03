<table style="width:100%">
<tr>
<th width="100%" colspan="6"><img src="https://www.xilinx.com/content/dam/xilinx/imgs/press/media-kits/corporate/xilinx-logo.png" width="30%"/><h1>CHaiDNN-v2</h2>
</th>
</tr>
  <tr>
    <th rowspan="6" width="17%">Analysis and Eval</th>
   </tr>
<tr>
	<td align="center" colspan="2"><a href="../docs/SUPPORTED_LAYERS.md">Supported Layers</a></td>
	<td align="center" colspan="2"><a href="../docs/PERFORMANCE_SNAPSHOT.md">Performance/Resource Utilization</a></td>
</tr>
  <tr></tr>
<tr>
	<td align="center" colspan="4"><a href="../docs/PERFORMANCE_EVAL.md">Performance Eval</a></td>	
</tr>
<tr></tr>
    <tr></tr>
  <tr><th colspan="6"></th></tr>

  <tr></tr>
  <tr>
     <th rowspan="7" width="17%">Design and Development</th>
   </tr>

<tr>
	<td  align="center"><a href="../docs/API.md">API Reference</a></td>
	<td  align="center"><a href="../docs/QUANTIZATION.md">Quantization User Guide for CHaiDNN</a></td>
	<td  align="center"><a href="../docs/MODELZOO.md">Model Zoo</a></td>
	<td  align="center"><a href="../docs/RUN_NEW_NETWORK.md">Running Inference on new Network</a></td>
</tr>
  <tr></tr>
<tr>
	<td  align="center">Creating SDx GUI Project</td>
	<td  align="center"><a href="../docs/CONFIGURABLE_PARAMS.md">Configurable Parameters</a></td>
	<td  align="center"><a href="../docs/CUSTOM_PLATFORM_GEN.md">Custom Platform Generation</a></td>
	<td  align="center"><a href="../docs/SOFTWARE_LAYER_PLUGIN.md">Software Layer Plugin</a></td>
</tr>
  <tr></tr>
<tr>
	<td  align="center" colspan="2"><a href="https://www.xilinx.com/support/documentation/sw_manuals/xilinx2017_4/ug1027-sdsoc-user-guide.pdf">SDSoC Environment User Guide</a></td>	
	<td align="center" colspan="2"><a href="../docs/HW_SW_PARTITIONING.md">Hardware-Software Partitioning for Performance</a></td>

</tr>  
</table>

## **Creating SDx GUI Project**

To build `CHaiDNN` using the Xilinx&reg; SDx Development Environment, perform the following steps.

1. Launch the **SDx development environment** using the desktop icon or the Start menu.
   The **Workspace Launcher** dialog appears

2. Click **Browse** to enter a workspace folder used to store your projects (you can use workspace folders to organize your work), then click **OK** to dismiss the **Workspace Launcher** dialog.
    The SDx development environment window opens with the **Welcome** tab visible when you create a new workspace. The Welcome tab can be closed by clicking the **X** icon or minimized if you do not wish to use it.

3. Select **File → New → Xilinx SDx Project** from the SDx development environment menu bar.
    The **New SDx Project** dialog box opens.

4. Select **Application** and Click **Next**.

5. Specify the name of the project. For example `CHaiDNN`.

6. Click **Next**.
   The **Choose Hardware platform** page appears.

7. From the **Choose Hardware platform** page, select `zcu102`. Click **Next**. To use a custom platform, click on **Add Custom Platform** and give the path to custom platform and click **OK**.
   The **System Configuration** page appears.

7. Use default settings and click **Next**.

8. Select **Empty Application** and click **Finish**.

9. From **Project Explorer** Pane, Right Click on the `src` directory under the created project and click **Import**.
   The **Import wizard** appears.

10. Expand the `General` option and select **File System**. Click **Next**.

11. Browse & Select the local copy of `CHaiDNN` Git repository. Click **OK**.

12. Expand `CHaiDNN` and Select the `software` and the `design` directories. Click **Finish**.

13. All the source files needed to build `CHaiDNN`, can be seen under `src` directory.

    >**:pushpin: NOTE:**  The `software/example` folder contains example files. Using SDx GUI, only 1 of them can be built at once. Exclude other example files from the project except the one you wish to build.  

14. Right click on project and select **C/C++ Build Settings**. In the **Configuration** drop down menu, Select **Release** as an active configuration.

15. In `SDS++ Compiler` add the following in the `command`
    ```
    sds++ -sds-hw XiConvolutionTop  <path to design>/src/design/conv/src/xi_convolution_top.cpp -clkid 1 -hls-tcl  <path to design>/src/conv/scripts/config_core.tcl -sds-end

    ```
    >**:pushpin: NOTE:**   `<path to design>` is the path to the project's `src/design` folder.

    >**:pushpin: NOTE:**  If custom platform is used as described in the document ["CUSTOM PLATFORM GENERATION"](CUSTOM_PLATFORM_GEN.md), then `clkid` must be set to 1.

16. In `Symbols`, add symbols `__SDSOC`,  `__CONV_ENABLE__`,  `__DSP48E2__`,  `__SYNTHESIS__` , `__POOL_ENABLE__`  , `__DECONV_ENABLE__`.

    >**:pushpin: NOTE:**  When building `DietChai` don't add `__POOL_ENABLE__`,  `__DECONV_ENABLE__`.   

17. In `Optimization` set the Optimization Level to None (-O0).

18. In `Directories`, add below paths
    ```
    <path to libraries>/opencv/arm64/include
    <path to libraries>/protobuf/arm64/include
    <path to libraries>/cblas/arm64/include
    <path to project>/src/design/conv/include
    <path to project>/src/design/conv/src
    <path to project>/src/design/pool/include
    <path to project>/src/design/pool/src
    <path to project>/src/design/deconv/include
    <path to project>/src/design/deconv/src
    ```
    >**:pushpin: NOTE:**  `<path to libraries>` is the path to `SD_Card` directory in local `CHaiDNN` repository. `<path to project>` is the path to SDx project.

19. In `SDS++ Linker` add the following in the `command`
    ```
    sds++ -xp param:compiler.skipTimingCheckAndFrequencyScaling=1 -xp "vivado_prop:run.impl_1.{STEPS.OPT_DESIGN.ARGS.MORE OPTIONS}={-directive Explore}"  -xp "vivado_prop:run.impl_1.{STEPS.PLACE_DESIGN.ARGS.MORE OPTIONS}={-directive Explore}" -xp "vivado_prop:run.impl_1.STEPS.PHYS_OPT_DESIGN.IS_ENABLED=1" -xp "vivado_prop:run.impl_1.{STEPS.PHYS_OPT_DESIGN.ARGS.MORE OPTIONS}={-directive Explore}" -xp "vivado_prop:run.impl_1.{STEPS.ROUTE_DESIGN.ARGS.MORE OPTIONS}={-directive Explore}" -xp "vivado_prop:run.synth_1.{STEPS.SYNTH_DESIGN.TCL.PRE}={<path to design>/src/conv/scripts/mcps.tcl}" -xp "vivado_prop:run.impl_1.{STEPS.PLACE_DESIGN.TCL.PRE}={<path to design>/src/conv/scripts/mcps.tcl}" -Wno-unused-label
    ```
 20. In `SDS++ Linker`, Select `Libraries` and add the following libs
     ```
     opencv_core;lzma;tiff;png16;z;jpeg;opencv_imgproc;opencv_imgcodecs;dl;rt;webp;protobuf;openblas
     ```

 21. Add library paths
     ```   
     <path to libraries>/opencv/arm64/lib
     <path to libraries>/protobuf/arm64/lib
     <path to libraries>/cblas/arm64/lib
     ```

     >**:pushpin: NOTE:**  **Note:** `<path to libraries>` is the path to `SD_Card` directory in local `CHaiDNN` repository.

22. Save and apply changes. Close the settings window.

23. Navigate to `<project name>/src/software/init` folder in `Project Explorer` Pane. Right click on `xi_init.cpp` and open the properties.
    In Optimization, change the `Optimization Level` to None (-O0).

24. Apply changes and close the window.

25. Open `<project name>/src/design/scripts/mcps.tcl` file and modify the path of `xdc` file as shown below.
```
    read_xdc <path to project>/src/design/conv/scripts/mcp_const.xdc
```
>**:pushpin: NOTE:**   `<path to project>` is the path to SDx project.

26.	Select the Hardware functions.

    - Navigate to `src/design/src/pool/src/pooling_layer_dp_2xio_top.cpp` file using SDx explorer, right click on `PoolTop` and select Toggle HW/SW.

    - Navigate to `src/design/src/deconv/src/xi_deconv_top.cpp` file using SDx explorer, right click on `XiDeconvTop` and select Toggle HW/SW.

     >**:pushpin: NOTE:**  When building `DietChai` don't map any function to HW. `XiConvolutionTop` will be mapped to HW by default.

27. Open `project.sdx` file, change the `Data motion network clock frequency (MHz)` to 100.00 and change the `Hardware Functions clock frequency (MHz)` to 100.00.

28. Build the project.

>**:information_source: NOTE:**   Current CHaiDNN release only supports SDx GUI build on Linux machines.

<hr/>
<p align="center"><sup>Copyright&copy; 2018 Xilinx</sup></p>
