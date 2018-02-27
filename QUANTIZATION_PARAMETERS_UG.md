## **CHaiDNN Quantization Parameters Details**

Design goal of a deep neural network is to achieve best accuracy with maximum performance. CHaiDNN works in fixed point 
domain for better performance. All the feature maps and trained parameters are converted from single precision to fixed
point before the computation starts. The precision parameters can vary a lot depending upon the network, datasets, or 
even across layers in the same network. Accuracy of a network depends on the precision parameters used to represent the 
feature maps and trained parameters. Well-crafted precision parameters might give accuracy similar to accuracy obtained
from a single precision model. 

Considering these wide variety of choices, CHaiDNN provides users the flexibility to use their own precision parameters
to achieve the best accuracy. Precision parameters are to be provided in the deploy.prototxt by the user. 

### **How to Add Precision Parameters**

CHaiDNN provides a new field 'quantization_params' for every layer to provide various precision parameters. 
Precision parameter has two parts. Total Bitwidth (bw) and Fractional Bits (fl). Precision Parameters should be 
mentioned for input/output feature maps as well as the trained parameters. 

Total bitwidths for each of these parameters are given in the below table:

Item | Network with batchnorm | Network without batchnorm
-------|------------------------|-------------------------
I/O Feature Maps|8|16
Trained Pramaters|8|8

### **How to Specify I/O Precision Parameters**

CHaiDNN expects the precision parameters for Input and Output feature maps for Convolution, InnerProduct (FC), 
Average Pooling, Eltwise, BatchNorm and Scale layers. 
```
bw_layer_in : Total bitwidth required for Input Feature Maps
fl_layer_in : Fractional bits required for Input Feature Maps
bw_layer_out : Total bitwidth required for Output Feature Maps
fl_layer_out : Fractional bits required for Output Feature Maps
```
```
Example : Eltwise Layer
layer {
	bottom: "group3/block1/conv3"
	bottom: "group3/block0/eltwise"
	top: "group3/block1/eltwise"
	name: "group3/block1/eltwise"
	type: "Eltwise"
    quantization_param {
		bw_layer_in: 8
		bw_layer_out: 8
		fl_layer_in: 3
		fl_layer_out: 3
	}  
}
```
**NOTE**: Eltwise Layer doesn't have any trained parameters, so we don't have to specify anything. But CHaiDNN expects the precision parameters for trained parameters of Convolution, InnerProduct (FC), BatchNorm and Scale layers.

### **For weights of Convolution and InnerProduct Layers**

```
bw_params : Total bitwidth required for weights of Convolution and InnerProduct
fl_params : Fractional bits required for weights of Convolution and InnerProduct
```
**NOTE** : Bias precision parameters are internally taken care by CHaiDNN.

```
Example:
layer {
  name: "conv1/7x7_s2"
  type: "Convolution"
  bottom: "data"
  top: "conv1/7x7_s2"
  convolution_param {
    num_output: 64
    pad: 3
    kernel_size: 7
    stride: 2
  }
  quantization_param {
    bw_layer_in: 16
    bw_layer_out: 16
    bw_params: 8 
    fl_layer_in: 8
    fl_layer_out: 4
    fl_params: 7 
  }
}
```

### **For Mean and Variance of BatchNorm Layer**
```
batchnorm_mean_bw : Total bitwidth required for mean of BatchNorm Layer
batchnorm_mean_fl : Fractional bits required for mean of BatchNorm Layer
batchnorm_variance_bw : Total bitwidth required for variance of BatchNorm Layer
batchnorm_variance_fl : Fractional bits required for variance of BatchNorm Layer
```
```
Example: 
layer {
	bottom: "conv0"
	top: "conv0/bn/mv"
	name: "conv0/bn/mv"
	type: "BatchNorm"
    quantization_param {
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

### **For gamma and beta of Scale Layer**
```
scale_gamma_bw : Total bitwidth required for gamma of Scale Layer
scale_gamma_fl : Fractional bits required for gamma of Scale Layer
scale_beta_bw : Total bitwidth required for beta of Scale Layer
scale_beta_fl : Fractional bits required for beta of Scale Layer
```

CHaiDNN fuses BatchNorm layer and Scale Layer into one single operation. So it requires the precision parameters for 
combined "gamma/sqrt(variance+eps)".

```
scale_gamma_by_std_bw : Total bitwidth required for 'gamma/sqrt(variance+eps)'
scale_gamma_by_std_fl : Fractional bits required for 'gamma/sqrt(variance+eps)'
```

**NOTE** : scale_gamma_by_std_bw & scale_gamma_by_std_bw are mandatory even if scale layer appears alone 
(without preceding batchnorm layer). In that case you can put any arbitrary values.

```
Example: 
layer {
	bottom: "conv0/bn/mv"
	top: "conv0/bn/bg"
	name: "conv0/bn/bg"
	type: "Scale"
    quantization_param {
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
