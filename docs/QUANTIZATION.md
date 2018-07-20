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
	<td  align="center">Quantization User Guide for CHaiDNN</td>
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
	<td  align="center" colspan="2"><a href="../docs/HW_SW_PARTITIONING.md">Hardware-Software Partioning for Performance</a></td>

</tr>  
</table>

## Quantization User Guide for CHaiDNN

Design goal of a deep neural network is to achieve best accuracy with maximum performance. CHaiDNN works in fixed point domain for better performance. All the feature maps and trained parameters are converted from single precision to fixed point before the computation starts. The precision parameters can vary a lot depending upon the network, datasets, or even across layers in the same network. Accuracy of a network depends on the precision parameters used to represent the feature maps and trained parameters. Well-crafted precision parameters might give accuracy similar to accuracy obtained from a single precision model.

CHaiDNN supports 2 different types of quantization modes:
1. <a name="xilinx_quant"></a> **Xilinx's Quantization**

    This quantization technique produces an optimal target quantization from a given network (deploy prototxt and caffemodel) and calibration set (unlabeled input images) without requiring hours of retraining or a labeled dataset for most of the use-cases.

1. <a name="dynamic_fixed"></a> **Dynamic Fixed Point Quantization**

    This refers to a fixed point representation where the number of bits for the integer and the fractional part is dynamic in nature. NOTE: For CHai, The total bitwidths(Sign + Integer + Fractional) required should be either 8bit or 6bit for the weights/ input or output activations.

CHaiDNN expects the precision parameters specified in the `deploy.prototxt`. The original input caffemodel can be used for the inference. There are two approaches to generate CHai `deploy.prototxt`:

<details>
<summary><strong><big>Approach-1: Generating prototxt using XportDNN</big></strong></summary>

##### <a name="xport"></a> Export DNNs to CHai compatible formats for various quantization modes using XportDNN
XportDNN is an unified tool provided to the CHai-users to be able to quickly produce CHai prototxt with appropriate precision parameters specified for various supported layers. CHaiDNN supports [Caffe Deep Learning Framework](http://caffe.berkeleyvision.org/).

###### XportDNN Features:

Feature | Xilinx Quantizer Mode | Dynamic Fixed Mode
-------|------------------------|-------------------------
Precision/ threshold deduction|Yes| N.A.
Auto CHai prototxt Generation|Yes| Yes
User configurable Bitwidth | Yes| Yes
User configurable Q.F format| N.A.| Yes

To run the XportDNN, use the command `python XportDNN.pyc` with the instructions below.

###### List of available Arguments:

- `[-h]`
- `[--quant_type]` - Specify the quantization mode {'Xilinx' or 'DynamicFixed'}, default 'Xilinx'

Arguments required for both quantization modes:

- `[--deploy_model DEPLOY_MODEL]` - Input deploy prototxt
- `[--weights WEIGHTS]` - FP32 pretrained caffe model
- `[--quantized_deploy_model QUANTIZED_TRAIN_VAL_MODEL]` - Output file name for CHaiDNN deploy prototxt
- `[--bitwidths BITWIDTHS] ` - Bit widths for input,params,output default: 6,6,6

Arguments for Xilinx-Quantization mode ONLY:

- `[--calibration_directory CALIBRATION_DIRECTORY]` - Dir of dataset of original images
- `[--calibration_size CALIBRATION_SIZE]` - Number of images to use for calibration, default is 8
- `[--dims DIMS]`            - Dimensions for first layer, default 3,224,224
- `[--transpose TRANSPOSE] ` - Passed to caffe.io.Transformer function set_transpose, default 2,0,1
- `[--channel_swap CHANNEL_SWAP]` - Passed to caffe.io.Transformer function set_channel_swap, default 2,1,0
- `[--raw_scale RAW_SCALE] ` - Passed to caffe.io.Transformer function set_raw_scale, default 255.0
- `[--mean_value MEAN_VALUE]` - Passed to caffe.io.Transformer function set_mean, default 104,117,123

Arguments for DynamicFixed mode ONLY:

- `[--fl_bitwidths FL_BITWIDTHS] ` - Bit widths for Fl-part of input,params,output default: 2,5,2. Required for "DynamicFixed" quant_mode ONLY.

**Points to Remember**

- XportDNN expects the input/ data to be defined as a layer.
  >**:page_with_curl: Example:**  

  ```sh
  layer {
    name: "data"
    type: "Input"
    top: "data"
    input_param {
      shape { #As required
        dim: 1
        dim: 3
        dim: 224  
        dim: 224
      }
    }
  }
  ```
- Arguments that are not passed will be set with a default value.
- Threshold values are generated for only [supported layers](./SUPPORTED_LAYERS.md) and a warning is generated for non-supported layers.
- Finetuning/ re-training feature, which could help users improve the accuracy of the DNNs, will be provided soon.



##### Quantizing a network using the Xilinx's Quantizer  Mode
>**:page_with_curl: GoogLeNet(Without LRN) v1 Example:**  
> Pre-requisites:
> * Pre-trained Caffemodel file
> * Associated deploy prototxt
> * Calibration dataset(Images). For details, refer to [Imagenet](http://www.image-net.org/download-imageurls) for downloading ILSVRC files.

Quantize a network by inference thresholding against a calibration set:

```
$ python XportDNN.pyc --quant_type "Xilinx" \
--deploy_model ./models/bvlc_googlenet_without_lrn/bvlc_googlenet_without_lrn_deploy.prototxt \
--weights ./models/bvlc_googlenet_without_lrn/bvlc_googlenet_without_lrn.caffemodel \
--quantized_deploy_model ./models/bvlc_googlenet_without_lrn/RistrettoDemo/bvlc_googlenet_without_lrn_quantized_deploy.prototxt \
--calibration_directory ./data/ilsvrc12/ILSVRC2012_img_val --calibration_size 32 \
--bitwidths 6,6,6 --dims 3,224,224 --transpose 2,0,1 \
--channel_swap 2,1,0 --raw_scale 255.0 \
--mean_value 104,117,123 --input_scale 1.0
```

The output of the above step is a CHaiDNN compatible prototxt


#### Creating a CHai deploy prototxt for Dynamic-Fixed  Mode
>**:page_with_curl: GoogLeNet(Without LRN) v1 Example:**
>Pre-requisites:
> * Pre-trained Caffemodel file
> * Associated deploy prototxt


XportDNN produces the CHai prototxt with the specified precision parameters for required layers. User is expected to set appropriate precision details(Q.F format) via `XportDNN.pyc` arguments `--bitwidths` and `--fl_bitwidths` for best accuracy
```
$ python XportDNN.pyc --quant_type "DynamicFixed" \
--deploy_model ./models/bvlc_googlenet_without_lrn/bvlc_googlenet_without_lrn_deploy.prototxt \
--weights ./models/bvlc_googlenet_without_lrn/bvlc_googlenet_without_lrn.caffemodel \
--quantized_deploy_model ./models/bvlc_googlenet_without_lrn/RistrettoDemo/bvlc_googlenet_without_lrn_quantized_deploy.prototxt\
--bitwidths 6,6,6 \
--fl_bitwidths 2,5,2
```
The output of the above step is a CHaiDNN compatible prototxt.
</details>



<details>
<summary><strong><big>Approach-2: Generating prototxt manually</big></strong></summary>

#### <a name="manual"></a> Precision details needed for CHai (Required when user manually creates CHai-Prototxt)

CHaiDNN provides a new field `precision_param` for every layer to provide various precision parameters. XportDNN will be able to add automatically these `precision_param{}` blocks to required layers in the prototxt. Another option would be the user adding appropriate extra fields in the prototxt manually based on the details provided below.

Precision Parameters should be mentioned for input/output feature maps as well as the trained parameters. Total supported bit-widths for each of these parameters are given in the below table:

Item | I/O Feature Maps |Trained Parameters
-------|------------------------|------
Option#1 | 8 | 8
Option#2 | 6 | 6

Whether or not `precision_param{}` block needs to be specified for a layer-type is as given below:

Layer | Xilinx Quant Mode | DynamicFixed Quant Mode
-------|------------------------|---------------
Convolution|Yes| Yes
BatchNorm|Yes|Yes
Power|No|No
Scale|Yes|Yes
InnerProduct|No|No
Pooling(Max, Avg)|Yes|Yes
ReLU|Yes|No
Deconvolution|Yes|No
Concat|Yes|No
Crop|No|No
Softmax|No|No
Dropout|No|No
Permute|Yes|Yes
Normalize|Yes|No
Argmax|No|No
Flatten|No|No
PriorBox|No|No
Reshape|No|No
NMS|No|No
Eltwise|Yes|Yes
Depthwise Separable Convolution|Yes|Yes
Input/ Data|Yes|No

The terminology for the various precision_params are:

Item | Xilinx Quantization | Dynamic fixed point Quantization | Comments
------------------|---------------------|------------------|---|
Total Bitwidth: weights|bw_params | bw_params| Single value per layer
fl-bits: weights| -- | fl_params|Single value per layer
Threshold: weights|th_params|--|Single value per channel
Total Bitwidth: Input Activations|bw_layer_in |bw_layer_in |Single value per layer
fl-bits: Input Activations|--|fl_layer_in|Single value per layer
Threshold: Input Activations|th_layer_in|--|Single value per layer
Total Bitwidth: Output Activations|bw_layer_out|bw_layer_out|Single value per layer
fl-bits: Output Activations|--|fl_layer_out|Single value per layer
Threshold: Output Activations|th_layer_out|--|Single value per layer

##### Specifying the I/O Precision Parameters

CHaiDNN expects the precision parameters for Input and Output feature maps for Convolution, InnerProduct (FC or Fully-Connected),
Average Pooling, Eltwise, BatchNorm and Scale layers.
```
bw_layer_in : Total bitwidth required for Input Feature Maps
fl_layer_in : Fractional bits required for Input Feature Maps
bw_layer_out : Total bitwidth required for Output Feature Maps
fl_layer_out : Fractional bits required for Output Feature Maps
th_layer_in : Threshold for the Input Feature Maps
th_layer_out : Threshold for the Output Feature maps
```
>**:page_with_curl: Eltwise Layer ("DynamicFixed" quant mode) Example:**
```
layer {
	bottom: "group3/block1/conv3"
	bottom: "group3/block0/eltwise"
	top: "group3/block1/eltwise"
	name: "group3/block1/eltwise"
	type: "Eltwise"
    precision_param {
        quant_type: "DynamicFixed"
		bw_layer_in: 8
		bw_layer_out: 8
		fl_layer_in: 3
		fl_layer_out: 3
	}  
}
```
>**:page_with_curl: Eltwise Layer ("Xilinx" quant mode) Example:**
```
layer {
	bottom: "group3/block1/conv3"
	bottom: "group3/block0/eltwise"
	top: "group3/block1/eltwise"
	name: "group3/block1/eltwise"
	type: "Eltwise"
    precision_param {
         quant_type: "Xilinx"
	 bwlayer_in: 8
	 bw_layer_out: 8
         th_layer_in: 752.14132257
         th_layer_out: 1106.06941786
	}  
}
```

	>**:pushpin: NOTE:**  Eltwise Layer doesn't have any trained parameters, so we don't have to specify `precision_param` block. But CHaiDNN expects the precision parameters for trained parameters of Convolution, InnerProduct (FC), BatchNorm and Scale layers.


##### **For weights of Convolution and InnerProduct Layers**

```
bw_params : Total bitwidth required for weights of Convolution and InnerProduct
fl_params : Fractional bits required for weights of Convolution and InnerProduct
```
>**:pushpin: NOTE:**  Bias precision parameters are internally taken care by CHaiDNN.

>**:page_with_curl: Convolution Layer ("DynamicFixed" quant mode) Example:**
```
layer {
  name: "conv1/7x7_s2"
  type: "Convolution"
  bottom: "data"
  top: "conv1/7x7_s2"
  convolution_param {
    num_output: 4
    pad: 3
    kernel_size: 7
    stride: 2
  }
  precision_param {
    quant_type: "DynamicFixed"
    bw_layer_in: 6
    bw_layer_out: 6
    bw_params: 6
    fl_layer_in: 2
    fl_layer_out: 2
    fl_params: 5
  }
}
```
>**:page_with_curl: Convolution Layer ("Xilinx" quant mode) Example:**
```
layer {
  name: "conv1/7x7_s2"
  type: "Convolution"
  bottom: "data"
  top: "conv1/7x7_s2"
  convolution_param {
    num_output: 4
    pad: 3
    kernel_size: 7
    stride: 2
  }
  precision_param {
    quant_type: "Xilinx"
    bw_layer_in: 8
    bw_layer_out: 8
    bw_params: 8
    th_layer_in: 150.866221005
    th_layer_out: 752.14132257
    th_params: 0.63660389185
    th_params: 0.37376704812
    th_params: 0.594033837318
    th_params: 0.400175541639
  }
}
```

#### Special case(BatchNorm & Scale layers) for `DynamicFixed` mode ONLY
##### **Mean and Variance of BatchNorm Layer**
```
batchnorm_mean_bw : Total bitwidth required for mean of BatchNorm Layer
batchnorm_mean_fl : Fractional bits required for mean of BatchNorm Layer
batchnorm_variance_bw : Total bitwidth required for variance of BatchNorm Layer
batchnorm_variance_fl : Fractional bits required for variance of BatchNorm Layer
```
>**:page_with_curl: Mean and Variance of BatchNorm Layer Example:**
```
layer {
	bottom: "conv0"
	top: "conv0/bn/mv"
	name: "conv0/bn/mv"
	type: "BatchNorm"
    precision_param {
      bw_layer_in: 8
      bw_layer_out: 8
      fl_layer_in: 3
      fl_layer_out: 3
      batchnorm_mean_fl: 4
      batchnorm_variance_fl: 3
      batchnorm_mean_bw: 8
      batchnorm_variance_bw: 8
    }
}

```

##### **Gamma and Beta of Scale Layer**
```
scale_gamma_bw : Total bitwidth required for gamma of Scale Layer
scale_gamma_fl : Fractional bits required for gamma of Scale Layer
scale_beta_bw : Total bitwidth required for beta of Scale Layer
scale_beta_fl : Fractional bits required for beta of Scale Layer
```

CHaiDNN fuses BatchNorm layer and Scale Layer into one single operation. So it requires the precision parameters for combined "gamma/sqrt(variance+eps)".

```
scale_gamma_by_std_bw : Total bitwidth required for 'gamma/sqrt(variance+eps)'
scale_gamma_by_std_fl : Fractional bits required for 'gamma/sqrt(variance+eps)'
```

>**:pushpin: NOTE:**  `scale_gamma_by_std_bw` & `scale_gamma_by_std_bw` are mandatory even if scale layer appears alone (without preceding batchnorm layer). In that case, you can put any arbitrary values.
>
>**:page_with_curl: Gamma and Beta of Scale Layer Example:**
```
layer {
	bottom: "conv0/bn/mv"
	top: "conv0/bn/bg"
	name: "conv0/bn/bg"
	type: "Scale"
    precision_param {
      bw_layer_in: 8
      bw_layer_out: 8
      fl_layer_in: 3
      fl_layer_out: 3
      scale_gamma_fl: 5
      scale_beta_fl: 5
      scale_gamma_bw: 8
      scale_beta_bw: 8
      scale_gamma_by_std_bw: 8
      scale_gamma_by_std_fl: 2  
    }
}
```
</details>

<hr/>
<p align="center"><sup>Copyright&copy; 2018 Xilinx</sup></p>
