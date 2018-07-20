<table style="width:100%">
<tr>
<th width="100%" colspan="6"><img src="https://www.xilinx.com/content/dam/xilinx/imgs/press/media-kits/corporate/xilinx-logo.png" width="30%"/><h1>CHaiDNN-v2</h2>
</th>
</tr>
  <tr>
    <th rowspan="6" width="17%">Analysis and Eval</th>
   </tr>
<tr>
	<td width="40%" align="center" colspan="2">Supported Layers</td>
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
	<td  align="center"><a href="../docs/RUN_NEW_NETWORK.md">Running Interface on new Network</a></td>
</tr>
  <tr></tr>
<tr>
	<td  align="center"><a href="../docs/BUILD_USING_SDX_GUI.md">Creating SDx GUI Project</a></td>
	<td  align="center"><a href="../docs/CONFIGURABLE_PARAMS.md">Configurable Parameters</a></td>
	<td  align="center"><a href="../docs/CUSTOM_PLATFORM_GEN.md">Custom Platform Generation</a></td>
	<td  align="center"><a href="../docs/SOFTWARE_LAYER_PLUGIN.md">Software Layer Plugin</a></td>
</tr>
  <tr></tr>
<tr>
	<td  align="center" colspan="2"><a href="https://www.xilinx.com/support/documentation/sw_manuals/xilinx2017_4/ug1027-sdsoc-user-guide.pdf">SDSoC Environment User Guide</a></td>	
	<td align="center" colspan="2"><a href="../docs/HW_SW_PARTITIONING.md">Hardware-Software Partioning for Performance</a></td>

</tr>  
</table>

## DNN Layer Support

Supported Layers||||
:--------------:|:---------------:|:---------------:|:---------------:|
Convolution     | BatchNorm       | Power           | Scale           |
Deconvolution   | ReLU            | Pooling(Max, Avg)         | InnerProduct    |
Dropout         | Softmax         | Crop            | Concat          |
Permute         | Normalize(L2 Norm)       | Argmax          | Flatten         |
PriorBox        | Reshape         | NMS             | Eltwise         |
CReLU<sup>* </sup> | Depthwise Separable Convolution | Software Layer Plugin<sup>** </sup>|Input/ Data|
Dilated Convolution||||         |

<sup>*  refers CReLU supported as a composition operation, i.e., Concat(Convolution, Power(Convolution, -1)), where Power(Convolution, -1) is expected to perform invert operation by multiplying input with -1. </sup>

<sup>** refers to [CHai Software-layer-Plugin](./SOFTWARE_LAYER_PLUGIN.md) </sup>


<details>
<summary><strong><big>Hardware Accelerated Layers<big></strong></summary>

The following table describes the hardware accelerated layers.

|        Layer Name        |  Hardware Kernel |                                                                                                                 Notes/Restrictions                                                                                                                 |
|:------------------------:|:----------------:|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Convolution              | Convolution      |  Filter sizes: 1x1, 3x3, 5x5, 7x7, 11x11. Horizontal and vertical strides must be same.  Number of Input and output feature maps must be less than 4096.                                                                                           |
| Dilated Convolution      | Convolution      | Dilation factor: 6                                                                                                                                                                                                                                 |
| Batch Normalization      | Convolution      | Number of Input and output feature maps must be less than 2048.                                                                                                                                                                                    |
| Scale and Bias           | Convolution      | Number of Input and output feature maps must be less than 2048.                                                                                                                                                                                    |
| Element-wise addition    | Convolution      |                                                                                                                                                                                                                                                    |
| Pooling (Max, Average)   | Convolution/Pool | Number of Input and output feature maps must be less than 4096.                                                                                                                                                                                    |
| Deconvolution            | Deconvolution    | 16-bit only                                                                                                                                                                                                                                        |
| Depthwise Separable Convolution | Convolution      | Number of Input and output feature maps must be less than 4096.                                                                                                                                                                                    |
| ReLU                     | Convolution      | ReLU operation is performed in-line with other supported operations. The fusion of ReLU is supported for the below Layers: Convolution, Dilated Convolution, Batch Normalization, Scale and Bias, 3D separable Convolution, Element-wise Addition  |
</details>
<details>
<summary><big><strong>Software Optimized Layers</strong></big></summary>

The following table describes the software optimized layers.

| Layer Name       | Software Kernel | Notes/Restrictions                                                                                  |
|------------------|-----------------|-----------------------------------------------------------------------------------------------------|
| L2-Normalization | Norm            | This layer works if it lies between two Hardware convolution layers (as present in VGGSSD network). |
| Permute          | Permute         | Input is in packed format. This works for the order=[0,2,3,1] only (as present in VGGSSD network).  |
| Inner Product    | CBLAS GEMV      | Using CBLAS library function.                                                                       |
| Softmax          | Softmax         |                                                                                                     |
| NMS              | NMS             | Max box count 200                                                                                   |

</details>


<hr/>
<p align="center"><sup>Copyright&copy; 2018 Xilinx</sup></p>
