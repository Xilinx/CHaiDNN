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
	<td  align="center">Model Zoo</td>
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
	<td  align="center" colspan="2"><a href="../docs/HW_SW_PARTITIONING.md">Hardware-Software Partioning for Performance</a></td>

</tr>  
</table>

## ModelZoo

CHaiDNN ModelZoo contains a collection of pre-trained reduced precision Convolutional Neural Networks. These models were carefully analyzed to find the best precision details per layer.

CHai ModelZoo consists of 3 sets of example-models:
1. **Xilinx's Quantizer Models**: Models generated using the technique described [here](./QUANTIZATION.md#xilinx_quant)
2. **Dynamic Fixed Models**: Models generated using the technique described [here](./QUANTIZATION.md#dynamic_fixed)
3. **User Defined Software Layer Model**: CHai supports software layer plug-in. For more details, see [Software Layer Plugin](./SOFTWARE_LAYER_PLUGIN.md).


### Xilinx's Quantizer Model Details

Models for 8-bit and 6-bit bit-width.

DNN model |  Single Precision Top-1 (Top-5) | S/W<sup>***</sup> 8Bit Top-1 (Top-5) | S/W<sup>***</sup> 6Bit Top-1 (Top-5)|  
-------|---------|---------|---------|
[AlexNet_NoLRN](https://www.xilinx.com/member/forms/download/xef.html?filename=AlexNet_XQ.zip) |  55.54 (78.75) | 55.18 (78.60) |  55.24 (78.44) <sup>*</sup> |
[AlexNet_FCN](https://www.xilinx.com/member/forms/download/xef.html?filename=AlexNetFCN_XQ.zip) |  39.74 (MeanIOU)| 39.14 (MeanIOU) | 35.84 (MeanIOU) <sup>**</sup>|
[VGG-16](https://www.xilinx.com/member/forms/download/xef.html?filename=VGG-16_XQ.zip) | 68.3 (88.4) | 68.18 (88.3) |  70.99 (90.23) <sup>*</sup>
[VGG-SSD300](https://www.xilinx.com/member/forms/download/xef.html?filename=VGG_SSD_300_XQ.zip) |  78.21 (mAP) | 78.06 (mAP) | 77.28 (mAP)|
[GoogleNet_V1_NoLRN](https://www.xilinx.com/member/forms/download/xef.html?filename=GoogleNetWithoutLRN_XQ.zip) | 67.35 (88.18) | 67.09 (87.97) | 67.27 (87.99) <sup>*</sup> |
[ResNet-50-BNfused](https://www.xilinx.com/member/forms/download/xef.html?filename=ResNet-50-Legacy_NoBN_XQ.zip)| 72.86 (91.11) | 73.4 (91.47) <sup>*</sup> | 68.8 (89.04)<sup>**</sup>|

<sup><strong>* </strong> indicates accuracies for the shared models is obtained by re-trained/finetuned to re-gain lost accuracy due to quantization.</sup>
<sup><strong>** </strong> indicates accuracies for the shared models can be improved with retraining. </sup>
<sup><strong>*** </strong> refers to the Caffe + Xilinx quantization.</sup>


### Dynamic Fixed Model Details

Bitwidth specifies the weights/ activations precisions.

DNN model | Bitwidth 8-bit or 6-bit
-------|------------------------
[GoogleNet_V1_NoLRN](https://www.xilinx.com/member/forms/download/xef.html?filename=GoogleNetWithoutLRN_DynamicFixed.zip) | Yes


### Custom Layer Model Details

DNN model | Bitwidth 8-bit or 6-bit  | Custom Layers
-------|------------------------|-------------------------
[GoogleNet_V1_NoLRN](https://www.xilinx.com/member/forms/download/xef.html?filename=GoogleNetWithoutLRN_SoftwareLayerPlugin.zip) | Yes | Fully-connected/ Innerproduct layer



<hr/>
<p align="center"><sup>Copyright&copy; 2018 Xilinx</sup></p>
