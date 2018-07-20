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
	<td align="center" colspan="4">Performance Eval</td>	
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


## Quick Performance Evaluation
CHai-v2 provides support for a variety of networks for classification, object detection and segmentation. Please refer to [Model Zoo](./MODELZOO.md) for list of networks and [Supported layers](./SUPPORTED_LAYERS.md) for list of layers that are required for these networks. While we believe that we have addressed most of the frequently-used heavy-lifting layers required in networks, we also understand that there might be some networks which might require support for some additional layers. We encourage users to add support for these layers and you could use the [software plugin](./SOFTWARE_LAYER_PLUGIN.md) methodology or the methodology described [here](./HW_SW_PARTITIONING.md) for efficient hardware software partitioning. But from a system design perspective, we understand that it might be useful to understand the performance of CHai on the layers that are already supported for a given network. We describe an API in this section which can be used to achieve this. For example, if a network is built with 50 layers and the support is missing for only two layers on CHai, then you could get a ball-park estimate on the latency of the 48 layers which are supported. Please note that these latency numbers are only an estimate and they could be optimistic or pessimistic based on the structure of the network. The reason for this is that the runtime of the accelerator fuses some layers for latency optimization.

<details>
<summary><big><strong>API</strong></big></summary>

</details>

<details>
<summary><big><strong>Example</strong></big></summary>

</details>

<details>
<summary><big><strong>Benchmarks</strong></big></summary>

</details>




