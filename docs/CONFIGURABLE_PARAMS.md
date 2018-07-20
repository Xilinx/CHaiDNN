<table style="width:100%">
<tr>
<th width="100%" colspan="6"><img src="https://www.xilinx.com/content/dam/xilinx/imgs/press/media-kits/corporate/xilinx-logo.png" width="30%"/><h1>CHaiDNN-v2</h2>
</th>
</tr>
  <tr>
    <th rowspan="2" width="17%">Analysis</th>
   </tr>
<tr>
	<td width="40%" align="center" colspan="2"><a href="../docs/SUPPORTED_LAYERS.md">Supported Layers</a></td>
	<td width="50%" align="center" colspan="2"><a href="../docs/PERFORMANCE_SNAPSHOT.md">Performance/Resource Utilization</a></td>

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
	<td  align="center"><a href="../docs/RUN_NEW_NETWORK.md">Running Interface on new Network</a></td>
</tr>
  <tr></tr>
<tr>
	<td  align="center"><a href="../docs/BUILD_USING_SDX_GUI.md">Creating SDx GUI Project</a></td>
	<td  align="center">Configurable Parameters</td>
	<td  align="center"><a href="../docs/CUSTOM_PLATFORM_GEN.md">Custom Platform Generation</a></td>
	<td  align="center"><a href="../docs/SOFTWARE_LAYER_PLUGIN.md">Software Layer Plugin</a></td>
</tr>
  <tr></tr>
<tr>
	<td  align="center" colspan="2"><a href="https://www.xilinx.com/support/documentation/sw_manuals/xilinx2017_4/ug1027-sdsoc-user-guide.pdf">SDSoC Environment User Guide</a></td>
	<td  align="center" colspan="2"><a href="../docs/HW_SW_PARTITIONING.md">Hardware-Software Partioning for Performance</a></td>

</tr>  
</table>

## Configurable Parameters

CHaiDNN provides various configurable parameters to be able to port the design to various Xilinx devices.

<details>
<summary><strong><big>Hardware Parameters</big></strong></summary>

The following table describes the available configurable parameters. The below parameters have to be changed in the two files namely [xi_conv_config.h](../design/conv/include/xi_conv_config.h) and [hw_settings.h](../software/include/hw_settings.h).

>**:pushpin: NOTE:**  Both the design and the software has to be rebuilt for expected functionality of the new configuration.

| Parameter        | Description           |
| ------------- |:-------------|
| XI_DIET_CHAI_Z      | Set to 1 to configure the design to use 128 DSPs for compute and 64-bit AXI interface |
| XI_DIET_CHAI_ZUPLUS      |   Set to 1 to configure the design to use 128 DSPs for compute and 128-bit AXI interface      |
| XI_KER_PROC | Sets the number of output feature maps to be generated in parallel. The valid values: 8, 16      |
| XI_PIX_PROC | Sets the number of output pixels to be generated in parallel. Valid values: 4,8,16, 32, 64      |
| XI_ISTAGEBUFF_DEPTH | Configures the depth of Istage Buffer. Valid Values: 1024, 2048, 4096, 8192      |
| XI_OSTAGEBUFF_DEPTH | Configures the depth of Ostage Buffer. Valid Values: 1024, 2048, 4096      |
| XI_WEIGHTBUFF_DEPTH | Configures the depth of Weights Buffer. Valid Values: 1024, 2048, 4096      |

The below parameters have to be changed in thr [xi_conv_config.h](../design/conv/include/xi_conv_config.h) file.

>**:pushpin: NOTE:**  The design has to be rebuilt for expected functionality of the new configuration


| Parameter        | Description           |
| ------------- |:-------------|
| XI_BIAS_URAM_EN | Set to 1 to map Bias buffer to URAM      |
| XI_WTS_URAM_EN | Set to 1 to map Weights buffer to URAM      |
| XI_ISTG_URAM_EN | Set to 1 to map Istage buffer to URAM      |
| XI_OSTG_URAM_EN | Set to 1 to map Ostage buffer to URAM      |
| XI_FEED_URAM_EN | Set to 1 to map Feeding buffer to URAM      |
| XI_SCALE_URAM_EN | Set to 1 to map Scale buffer to URAM      |

</details>

<details>
<summary><strong><big>Supported Configurations</big></strong></summary>

The following table shows the configurations for XI_KER_PROC and XI_PIX_PROC for required number of compute DSPs.

<table class="tg">
  <tr>
    <th class="tg-us36">DSPs</th>
    <th class="tg-us36" colspan="2">128</th>
    <th class="tg-us36" colspan="2">256</th>
    <th class="tg-us36" colspan="2">512</th>
    <th class="tg-us36" colspan="2">1024</th>
  </tr>
  <tr>
    <td class="tg-us36">XI_KER_PROC</td>
    <td class="tg-us36">8</td>
    <td class="tg-us36">16</td>
    <td class="tg-us36">8</td>
    <td class="tg-us36">16</td>
    <td class="tg-us36">8</td>
    <td class="tg-us36">16</td>
    <td class="tg-us36">8</td>
    <td class="tg-us36">16</td>
  </tr>
  <tr>
    <td class="tg-yw4l">XI_PIX_PROC</td>
    <td class="tg-yw4l">8</td>
    <td class="tg-yw4l">4</td>
    <td class="tg-yw4l">16</td>
    <td class="tg-yw4l">8</td>
    <td class="tg-yw4l">32</td>
    <td class="tg-yw4l">16</td>
    <td class="tg-yw4l">64</td>
    <td class="tg-yw4l">32</td>
  </tr>
</table>

</details>


<hr/>
<p align="center"><sup>Copyright&copy; 2018 Xilinx</sup></p>
