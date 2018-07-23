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
	<td  align="center"><a href="../docs/BUILD_USING_SDX_GUI.md">Creating SDx GUI Project</a></td>
	<td  align="center"><a href="../docs/CONFIGURABLE_PARAMS.md">Configurable Parameters</a></td>
	<td  align="center"><a href="../docs/CUSTOM_PLATFORM_GEN.md">Custom Platform Generation</a></td>
	<td  align="center">Software Layer Plugin</td>
</tr>
  <tr></tr>
<tr>
	<td  align="center" colspan="2"><a href="https://www.xilinx.com/support/documentation/sw_manuals/xilinx2017_4/ug1027-sdsoc-user-guide.pdf">SDSoC Environment User Guide</a></td>	
	<td align="center" colspan="2"><a href="../docs/HW_SW_PARTITIONING.md">Hardware-Software Partitioning for Performance</a></td>

</tr>  
</table>

## **Software Layer Plugin in CHaiDNN**

CHaiDNN supports layers which are commonly used in various image classification, object detection based neural networks. If a user network has any unsupported layers, CHaiDNN provides a feature to add unsupported layers as custom layers and run the network inference.

ChaiDNN custom layer expects all the input data, output data and trained parameters in the Caffe format, i.e. single precision data packed in [NCHW](http://caffe.berkeleyvision.org/tutorial/net_layer_blob.html) format. Inside the custom layer, the user is free to convert it into any suitable format. Also, custom layer can take multiple bottom blobs, but only one top blob.

If this function is very critical in terms of performance, feel free to contact CHaiDNN team for hardware or software acceleration of this layer in CHaiDNN. 

This feature enables quick evaluation of a user network. Do not use the ChaiDNN performance number for benchmarking this network. For the purpose of execution of custom layer CHai inserts unpack and pack layers acoross the custom layer. The unpack layer converts the input data into Caffe format and the pack layer performs the pack action on the output. 

Post execution of the network, the performance numbers of the custom layer and pack/unpack layers can be obtained using the call described below

```c++
//# Display latency of custom and Pack/Unpack layers
getPerfInfo(chaihandle);
```
Please note below the performance numbers of pack and unpack layers for execution of fully connected layer as a custom layer in GoogleNet.  
```sh
[PERFM] Performance with Batching : 49.54 Images/second

[INFOx] Total number of Custom layers :  1
[PERFM] Custom Layer - 1     : loss3/calssifier
[PERFM] Unpack layer Latency : 0.31 ms
[PERFM] Custom layer Latency : 30.27 ms
[PERFM] Pack layer Latency   : 0.0 ms
```


This tutorial walks through the steps required to add a custom layer to CHaiDNN.Below are the steps described in detail with GoogleNet as an example where fully-connected layer is flagged as a custom layer.

>**:pushpin: NOTE:**  The caffe model and prototxt for this network is available at [ModelZoo](./MODELZOO.md).


<details>
<summary><strong><big>Step-1: Mark custom layer in the caffe prototxt</big></strong></summary>

The following is the syntax of the fully-connected layer as given in the prototxt of GoogleNet. Layer-type is given as `gemm` which CHaiDNN doesn't support natively.

```protobuf
layer {
  name: "loss3/classifier"
  type: "gemm"
  bottom: "pool5/7x7_s1"
  top: "loss3/classifier"
  inner_product_param {
    num_output: 1000
  }
}
```
The following sections provides information on the layer description that CHaiDNN expects.

##### 1. Mark it as a custom layer

The layer should be marked as a custom layer by adding following line to the layer  description.
```protobuf
user_defined_layer : true
```

##### 2. Add xcustom_param

You should add a new section, **xcustom_param**, to the layer. This section communicates with all the parameters of a layer and layer output dimensions.

All numerical parameters will have same label named **float_args**. This will be passed to the custom layer function as a float array with the same order as provided in the prototxt so that custom layer plugin can access them by array indexing. In the case of ‘gemm’ there is only one argument `num_output` which will be communicated as
```protobuf
float_args = 1000
```
CHaiDNN pre-computes the output size of each supported layer. But for custom layer, it should be provided by the user since CHaiDNN doesn't know underlying implementation. This is added as **top_dim** sequence and it should match the Caffe blob dimensions.

##### 3. Add precision parameters

User needs to provide the precision parameters which are used to convert data between integer and floating point format. Finally, the layer description looks like this:

```protobuf
layer {
  name: "loss3/classifier"
  type: "gemm"
  bottom: "pool5/7x7_s1"
  top: "loss3/classifier"

  user_defined_layer : true

  xcustom_param {
    float_args : 1000
    top_dim : 1
    top_dim : 1000     
  }

  precision_param {
    bw_layer_in: 8
    bw_layer_out: 8
    fl_layer_in: 3
    fl_layer_out: 3
  }
}
```
</details>

<details>
<summary><strong><big>Step-2: Create software layer plugin</big></strong></summary>

Every framework requires its functions to be created in a standard format, so does CHaiDNN. This section explains the standard format of a custom layer in CHaiDNN. The entire custom layer function definition should be kept in `custom/custom_class.cpp` file.

##### 1. Function Prototype
CHaiDNN custom layer has a very simple interface. Every custom layer is a method of `class Custom`. So first, declare it as a public method in `Custom class` in **custom/custom.hpp**

```c++
// Declare Custom layer in custom/custom_class.hpp file
class Custom
{
  public:
  // ... other custom functions
  void custom_gemm(const xChangeLayer *xchange);
};
```

Every custom layer function takes only one input namely the `xChangeLayer` object. The `xChangeLayer` object has the required details needed for the layer execution

Function return type is always `void`.

Now its implementation should be added to **custom/custom.cpp**.

```c++
// Add the implementation to custom/custom.cpp
void Custom::custom_gemm(const xChangeLayer *xchange) {
  // Implementation comes here
}
```

##### 2. Create Custom Layer Definition

Below section explains use of information from `xChangeLayer` to create a custom layer function definition.

###### 2.1. Input and Output Dimensions
Dimensions of the first bottom blob and the top blob can be accessed as follows:

```c++
  // Input Dimensions
  vector<int> in_dim = xchange->input_dims[0];
  int M = in_dim.at(0);   // Batch Size
  int N = in_dim.at(1);   // Input vector size

  // Output Dimensions  
  vector<int> out_dim = xchange->output_dims[0];
  int O = out_dim.at(1);   // Output vector size
  ```

###### 2.2 Accessing Layer Parameters in Prototxt

As mentioned before, the entire parameters specified in prototxt as `float_args` field are stored as a single float array inside xChangeLayer. the actual names of these fields are not stored. The user is expected to know the order in which these variables are specified in the prototxt, so that he can access them by specifying proper indices into the float_args array.

For example, in this case, custom layer has only one parameter, `num_output`. So user can access it as follows:

```c++
  // Numerical Parameters in Prototxt
  int num_output = (int)xchange->float_args[0];
```


###### 2.3. Accessing Trained Parameters in Caffemodel

If a custom layer has trained parameters which are stored in caffemodel file,the entire data is transferred to xChangeLayer along with its dimensions in the same order as it appears inside caffemodel file.

For example, custom(fully-connected) layer has both weights and bias stored in caffemodel file. So below code snippet shows how to access them from xChangeLayer:

```c++
  // Extract weights and bias
  vector<float> weights = xchange->custom_float_params[0];
  vector<float> bias    = xchange->custom_float_params[1];

  // Extract the dimensions of weights and bias
  vector<int> weights_dim = xchange->params_dims[0];
  vector<int> bias_dim    = xchange->params_dims[1];
```

###### 2.4. Custom Layer Functionality
Now that all the required data is available, you can plugin the core functionality as part of the custom layer. CHaiDNN processes images in batches and so the custom layer function should also support batch processing.

>**:pushpin: NOTE:**  CHaiDNN pre-allocates all the input and output buffers for custom layer function.

If the custom function has multiple bottom blobs, each blob will have a corresponding input buffer and the buffers will be arranged in the same order as the blobs in the prototxt.

```c++
  int batch = M;
  for(int batch_num=0; batch_num<batch; batch_num++) {

    // Access the input pointer
    float* In_ptr = (float*)xchange->in_ptrs[0] + batch_num * N;

    // Access the output pointer
    float* out_ptr = (float*)xchange->out_ptrs[0] + batch_num * O;

    // Do the computation
    Frcnn_gemm(In_ptr, weights_transpose, bias, out_ptr, 1, N, O);
  }
```
</details>

<details>
<summary><strong><big>Step-3: Register the software layer plugin</big></strong></summary>

You need to register the custom layer to CHaiDNN. `Custom class` keeps a registry of all custom layer functions against its **layer type**.

Insert custom function in map object as registry process with key "gemm" inside as shown below.

```c++
// Register the Custom Layer in custom/custom_class.cpp file
Custom::Custom()
{
  // ... Other registered custom layers
  custom_func_keys["gemm"] = &custom::custom_gemm;
}
```
</details>

<details>
<summary><strong><big>Step-4: Re-build the host libraries</big></strong></summary>

Re-build the host libraries using the instructions provided in the [Building CHaiDNN Software from Source](../README.md#2-build-chaidnn-software-from-source)section. Replace the libraries in the SDCARD for the changes to take effect.
</details>

>**:pushpin: NOTE:**  Detailed information on the implementation can viewed [here](../software/custom).

<hr/>
<p align="center"><sup>Copyright&copy; 2018 Xilinx</sup></p>
