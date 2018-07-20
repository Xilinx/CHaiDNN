/*----------------------------------------------------
Copyright 2017 Xilinx, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
----------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "../include/hw_settings.h"

double exp_lookup_table_float[1000]={
		0.000045, 0.000046, 0.000046, 0.000047, 0.000047, 0.000048, 0.000048, 0.000049, 0.000049, 0.000050,
		0.000050, 0.000051, 0.000051, 0.000052, 0.000052, 0.000053, 0.000053, 0.000054, 0.000054, 0.000055,
		0.000055, 0.000056, 0.000057, 0.000057, 0.000058, 0.000058, 0.000059, 0.000059, 0.000060, 0.000061,
		0.000061, 0.000062, 0.000063, 0.000063, 0.000064, 0.000064, 0.000065, 0.000066, 0.000066, 0.000067,
		0.000068, 0.000068, 0.000069, 0.000070, 0.000070, 0.000071, 0.000072, 0.000073, 0.000073, 0.000074,
		0.000075, 0.000076, 0.000076, 0.000077, 0.000078, 0.000079, 0.000079, 0.000080, 0.000081, 0.000082,
		0.000083, 0.000084, 0.000084, 0.000085, 0.000086, 0.000087, 0.000088, 0.000089, 0.000090, 0.000091,
		0.000091, 0.000092, 0.000093, 0.000094, 0.000095, 0.000096, 0.000097, 0.000098, 0.000099, 0.000100,
		0.000101, 0.000102, 0.000103, 0.000104, 0.000105, 0.000106, 0.000107, 0.000108, 0.000109, 0.000111,
		0.000112, 0.000113, 0.000114, 0.000115, 0.000116, 0.000117, 0.000119, 0.000120, 0.000121, 0.000122,
		0.000123, 0.000125, 0.000126, 0.000127, 0.000128, 0.000130, 0.000131, 0.000132, 0.000134, 0.000135,
		0.000136, 0.000138, 0.000139, 0.000141, 0.000142, 0.000143, 0.000145, 0.000146, 0.000148, 0.000149,
		0.000151, 0.000152, 0.000154, 0.000155, 0.000157, 0.000158, 0.000160, 0.000162, 0.000163, 0.000165,
		0.000167, 0.000168, 0.000170, 0.000172, 0.000173, 0.000175, 0.000177, 0.000179, 0.000180, 0.000182,
		0.000184, 0.000186, 0.000188, 0.000190, 0.000192, 0.000194, 0.000195, 0.000197, 0.000199, 0.000201,
		0.000203, 0.000206, 0.000208, 0.000210, 0.000212, 0.000214, 0.000216, 0.000218, 0.000220, 0.000223,
		0.000225, 0.000227, 0.000229, 0.000232, 0.000234, 0.000236, 0.000239, 0.000241, 0.000244, 0.000246,
		0.000249, 0.000251, 0.000254, 0.000256, 0.000259, 0.000261, 0.000264, 0.000267, 0.000269, 0.000272,
		0.000275, 0.000277, 0.000280, 0.000283, 0.000286, 0.000289, 0.000292, 0.000295, 0.000298, 0.000301,
		0.000304, 0.000307, 0.000310, 0.000313, 0.000316, 0.000319, 0.000322, 0.000326, 0.000329, 0.000332,
		0.000335, 0.000339, 0.000342, 0.000346, 0.000349, 0.000353, 0.000356, 0.000360, 0.000363, 0.000367,
		0.000371, 0.000374, 0.000378, 0.000382, 0.000386, 0.000390, 0.000394, 0.000398, 0.000402, 0.000406,
		0.000410, 0.000414, 0.000418, 0.000422, 0.000426, 0.000431, 0.000435, 0.000439, 0.000444, 0.000448,
		0.000453, 0.000457, 0.000462, 0.000467, 0.000471, 0.000476, 0.000481, 0.000486, 0.000491, 0.000495,
		0.000500, 0.000506, 0.000511, 0.000516, 0.000521, 0.000526, 0.000531, 0.000537, 0.000542, 0.000548,
		0.000553, 0.000559, 0.000564, 0.000570, 0.000576, 0.000581, 0.000587, 0.000593, 0.000599, 0.000605,
		0.000611, 0.000617, 0.000624, 0.000630, 0.000636, 0.000643, 0.000649, 0.000656, 0.000662, 0.000669,
		0.000676, 0.000682, 0.000689, 0.000696, 0.000703, 0.000710, 0.000717, 0.000725, 0.000732, 0.000739,
		0.000747, 0.000754, 0.000762, 0.000769, 0.000777, 0.000785, 0.000793, 0.000801, 0.000809, 0.000817,
		0.000825, 0.000833, 0.000842, 0.000850, 0.000859, 0.000867, 0.000876, 0.000885, 0.000894, 0.000903,
		0.000912, 0.000921, 0.000930, 0.000940, 0.000949, 0.000959, 0.000968, 0.000978, 0.000988, 0.000998,
		0.001008, 0.001018, 0.001028, 0.001039, 0.001049, 0.001060, 0.001070, 0.001081, 0.001092, 0.001103,
		0.001114, 0.001125, 0.001136, 0.001148, 0.001159, 0.001171, 0.001183, 0.001195, 0.001207, 0.001219,
		0.001231, 0.001243, 0.001256, 0.001268, 0.001281, 0.001294, 0.001307, 0.001320, 0.001334, 0.001347,
		0.001360, 0.001374, 0.001388, 0.001402, 0.001416, 0.001430, 0.001445, 0.001459, 0.001474, 0.001489,
		0.001504, 0.001519, 0.001534, 0.001549, 0.001565, 0.001581, 0.001597, 0.001613, 0.001629, 0.001645,
		0.001662, 0.001678, 0.001695, 0.001712, 0.001730, 0.001747, 0.001764, 0.001782, 0.001800, 0.001818,
		0.001836, 0.001855, 0.001874, 0.001892, 0.001911, 0.001931, 0.001950, 0.001970, 0.001989, 0.002009,
		0.002030, 0.002050, 0.002071, 0.002091, 0.002112, 0.002134, 0.002155, 0.002177, 0.002199, 0.002221,
		0.002243, 0.002266, 0.002288, 0.002311, 0.002335, 0.002358, 0.002382, 0.002406, 0.002430, 0.002454,
		0.002479, 0.002504, 0.002529, 0.002554, 0.002580, 0.002606, 0.002632, 0.002659, 0.002685, 0.002712,
		0.002740, 0.002767, 0.002795, 0.002823, 0.002852, 0.002880, 0.002909, 0.002938, 0.002968, 0.002998,
		0.003028, 0.003058, 0.003089, 0.003120, 0.003151, 0.003183, 0.003215, 0.003247, 0.003280, 0.003313,
		0.003346, 0.003380, 0.003414, 0.003448, 0.003483, 0.003518, 0.003553, 0.003589, 0.003625, 0.003661,
		0.003698, 0.003735, 0.003773, 0.003811, 0.003849, 0.003888, 0.003927, 0.003966, 0.004006, 0.004047,
		0.004087, 0.004128, 0.004170, 0.004212, 0.004254, 0.004297, 0.004340, 0.004384, 0.004428, 0.004472,
		0.004517, 0.004562, 0.004608, 0.004655, 0.004701, 0.004749, 0.004796, 0.004845, 0.004893, 0.004942,
		0.004992, 0.005042, 0.005093, 0.005144, 0.005196, 0.005248, 0.005301, 0.005354, 0.005408, 0.005462,
		0.005517, 0.005573, 0.005629, 0.005685, 0.005742, 0.005800, 0.005858, 0.005917, 0.005977, 0.006037,
		0.006097, 0.006159, 0.006221, 0.006283, 0.006346, 0.006410, 0.006474, 0.006540, 0.006605, 0.006672,
		0.006739, 0.006806, 0.006875, 0.006944, 0.007014, 0.007084, 0.007155, 0.007227, 0.007300, 0.007373,
		0.007447, 0.007522, 0.007598, 0.007674, 0.007751, 0.007829, 0.007908, 0.007987, 0.008068, 0.008149,
		0.008231, 0.008313, 0.008397, 0.008481, 0.008567, 0.008653, 0.008740, 0.008828, 0.008916, 0.009006,
		0.009096, 0.009188, 0.009280, 0.009373, 0.009468, 0.009563, 0.009659, 0.009756, 0.009854, 0.009953,
		0.010053, 0.010154, 0.010256, 0.010359, 0.010463, 0.010569, 0.010675, 0.010782, 0.010890, 0.011000,
		0.011110, 0.011222, 0.011335, 0.011449, 0.011564, 0.011680, 0.011797, 0.011916, 0.012036, 0.012157,
		0.012279, 0.012402, 0.012527, 0.012653, 0.012780, 0.012908, 0.013038, 0.013169, 0.013302, 0.013435,
		0.013570, 0.013707, 0.013844, 0.013984, 0.014124, 0.014266, 0.014409, 0.014554, 0.014701, 0.014848,
		0.014998, 0.015148, 0.015301, 0.015454, 0.015610, 0.015767, 0.015925, 0.016085, 0.016247, 0.016410,
		0.016575, 0.016741, 0.016910, 0.017080, 0.017251, 0.017425, 0.017600, 0.017777, 0.017955, 0.018136,
		0.018318, 0.018502, 0.018688, 0.018876, 0.019066, 0.019257, 0.019451, 0.019646, 0.019844, 0.020043,
		0.020245, 0.020448, 0.020654, 0.020861, 0.021071, 0.021283, 0.021497, 0.021713, 0.021931, 0.022151,
		0.022374, 0.022599, 0.022826, 0.023055, 0.023287, 0.023521, 0.023757, 0.023996, 0.024237, 0.024481,
		0.024727, 0.024975, 0.025226, 0.025480, 0.025736, 0.025995, 0.026256, 0.026520, 0.026786, 0.027056,
		0.027327, 0.027602, 0.027880, 0.028160, 0.028443, 0.028729, 0.029017, 0.029309, 0.029603, 0.029901,
		0.030202, 0.030505, 0.030812, 0.031121, 0.031434, 0.031750, 0.032069, 0.032391, 0.032717, 0.033046,
		0.033378, 0.033713, 0.034052, 0.034394, 0.034740, 0.035089, 0.035442, 0.035798, 0.036158, 0.036521,
		0.036888, 0.037259, 0.037633, 0.038012, 0.038394, 0.038779, 0.039169, 0.039563, 0.039961, 0.040362,
		0.040768, 0.041177, 0.041591, 0.042009, 0.042432, 0.042858, 0.043289, 0.043724, 0.044163, 0.044607,
		0.045055, 0.045508, 0.045966, 0.046427, 0.046894, 0.047365, 0.047841, 0.048322, 0.048808, 0.049298,
		0.049794, 0.050294, 0.050800, 0.051310, 0.051826, 0.052347, 0.052873, 0.053404, 0.053941, 0.054483,
		0.055031, 0.055584, 0.056142, 0.056707, 0.057277, 0.057852, 0.058434, 0.059021, 0.059614, 0.060213,
		0.060818, 0.061430, 0.062047, 0.062671, 0.063300, 0.063937, 0.064579, 0.065228, 0.065884, 0.066546,
		0.067215, 0.067890, 0.068572, 0.069262, 0.069958, 0.070661, 0.071371, 0.072088, 0.072813, 0.073545,
		0.074284, 0.075030, 0.075784, 0.076546, 0.077315, 0.078092, 0.078877, 0.079670, 0.080471, 0.081279,
		0.082096, 0.082921, 0.083755, 0.084596, 0.085447, 0.086305, 0.087173, 0.088049, 0.088934, 0.089827,
		0.090730, 0.091642, 0.092563, 0.093493, 0.094433, 0.095382, 0.096341, 0.097309, 0.098287, 0.099275,
		0.100272, 0.101280, 0.102298, 0.103326, 0.104365, 0.105413, 0.106473, 0.107543, 0.108624, 0.109715,
		0.110818, 0.111932, 0.113057, 0.114193, 0.115341, 0.116500, 0.117671, 0.118853, 0.120048, 0.121254,
		0.122473, 0.123704, 0.124947, 0.126203, 0.127471, 0.128752, 0.130046, 0.131353, 0.132673, 0.134007,
		0.135354, 0.136714, 0.138088, 0.139476, 0.140877, 0.142293, 0.143723, 0.145168, 0.146627, 0.148100,
		0.149589, 0.151092, 0.152611, 0.154144, 0.155694, 0.157258, 0.158839, 0.160435, 0.162048, 0.163676,
		0.165321, 0.166983, 0.168661, 0.170356, 0.172068, 0.173797, 0.175544, 0.177308, 0.179090, 0.180890,
		0.182708, 0.184544, 0.186399, 0.188272, 0.190165, 0.192076, 0.194006, 0.195956, 0.197925, 0.199915,
		0.201924, 0.203953, 0.206003, 0.208073, 0.210164, 0.212277, 0.214410, 0.216565, 0.218741, 0.220940,
		0.223160, 0.225403, 0.227668, 0.229956, 0.232268, 0.234602, 0.236960, 0.239341, 0.241747, 0.244176,
		0.246630, 0.249109, 0.251612, 0.254141, 0.256695, 0.259275, 0.261881, 0.264513, 0.267171, 0.269856,
		0.272568, 0.275308, 0.278075, 0.280869, 0.283692, 0.286543, 0.289423, 0.292332, 0.295270, 0.298237,
		0.301235, 0.304262, 0.307320, 0.310409, 0.313528, 0.316679, 0.319862, 0.323077, 0.326324, 0.329603,
		0.332916, 0.336262, 0.339641, 0.343055, 0.346502, 0.349985, 0.353502, 0.357055, 0.360643, 0.364268,
		0.367929, 0.371627, 0.375361, 0.379134, 0.382944, 0.386793, 0.390680, 0.394607, 0.398573, 0.402578,
		0.406624, 0.410711, 0.414839, 0.419008, 0.423219, 0.427472, 0.431768, 0.436108, 0.440491, 0.444918,
		0.449389, 0.453906, 0.458467, 0.463075, 0.467729, 0.472430, 0.477178, 0.481974, 0.486817, 0.491710,
		0.496652, 0.501643, 0.506685, 0.511777, 0.516921, 0.522116, 0.527363, 0.532663, 0.538016, 0.543424,
		0.548885, 0.554401, 0.559973, 0.565601, 0.571285, 0.577027, 0.582826, 0.588684, 0.594600, 0.600576,
		0.606612, 0.612708, 0.618866, 0.625086, 0.631368, 0.637713, 0.644123, 0.650596, 0.657135, 0.663739,
		0.670410, 0.677147, 0.683953, 0.690827, 0.697770, 0.704782, 0.711865, 0.719020, 0.726246, 0.733545,
		0.740917, 0.748363, 0.755885, 0.763481, 0.771155, 0.778905, 0.786733, 0.794640, 0.802626, 0.810692,
		0.818840, 0.827070, 0.835382, 0.843778, 0.852258, 0.860823, 0.869474, 0.878213, 0.887039, 0.895954,
		0.904958, 0.914053, 0.923240, 0.932518, 0.941890, 0.951357, 0.960918, 0.970575, 0.980330, 0.990182
};


// function to return the maximum of an array
template<typename inType>
inType getMaxVal(inType* start, int size, int batch_size, int batch_id)
{
	// TODO : Abid : Need a better value for maxVal
	inType maxVal = -255;//-9999999;

	for(int i=0; i<size; i++)
	{
		inType input = start[i*batch_size + batch_id];
		maxVal = input < maxVal ? maxVal : input;
	}

	return maxVal;
}

// function to return the maximum of an array
template<typename inType>
inType getMaxVal_float(inType* start, int size)
{
	// TODO : Abid : Need a better value for maxVal
	inType maxVal = -255;//-9999999;

	for(int i=0; i<size; i++)
	{
		inType input = start[i];
		maxVal = input < maxVal ? maxVal : input;
	}

	return maxVal;
}

//#define IO_DATA_TYPE short//char
//#define IO_DATA_TYPE1 float//char

#define SUB_DATA_TYPE short
#define SUB_DATA_FLOAT_TYPE float

// Function : SwSoftmax Layer
// inArray : Input array of size [nboxes x nclasses] eg : [7308 x 21] for SSD-300-PascalVOC
// outArray : Result array with same size
// nclasses : Number of classes in training (eg : 21 for PascalVOC, 1000 for ImageNet)
// nboxes : Number of boxes detected
// TODO : ABID : Better subtract the maxVal for numerical stability
//template <typename inType, typename outType>
void SwSoftmaxWrapper_prevlayerfcfloat(IO_DATA_TYPE1* inArray, IO_DATA_TYPE1* sumBuffer, IO_DATA_TYPE1* outArray, int *scalar_softmax_args)
{

	int nclasses  = scalar_softmax_args[0];
	int nboxes    = scalar_softmax_args[1];
	int batch_size= scalar_softmax_args[3];
	int ip_fbits  = scalar_softmax_args[4];

	int quant_scheme_flag	  = scalar_softmax_args[7];
	int prev_layer_type = scalar_softmax_args[2];

	//# Read scale factor for input
	float *sf_in_ptr 	  = (float*)(&scalar_softmax_args[8]);
	float sf_in = sf_in_ptr[0];

	int batch_size_one_enable =  scalar_softmax_args[5];
	batch_size = XBATCH_SIZE;
	int batch_size_loopcnt;
	if(batch_size_one_enable)
	{
		batch_size_loopcnt = 1;
	}
	else
	{
		batch_size_loopcnt = batch_size;
	}

	float fval1;
	int table_idx;
	float fract_val;
	int fract_id;

#if SOFTMAX_OPT
	const float SOFTMAX_THRESHOLD = -6.0;
#endif

#if 0
	FILE *frefout=fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/GoogleNetWithoutLRN_New/caffe_ref/loss3_classifier_out.txt","r");
	if(frefout == NULL)
	{
		//std :: cout << "File not found - " << ref_path << std :: endl;
		//return -1;
		printf("file open error");
	}

	float *ref_output		= (float*)malloc(nclasses*nboxes*batch_size*sizeof(float));
	for(int i=0; i< nclasses*nboxes;i++)
	{
		fscanf(frefout, "%f ", &ref_output[2*i]);
		ref_output[2*i+1] = ref_output[2*i];
	}
	fclose(frefout);
#endif


	// Buffer to keep the sum of exponents for each box
	//outType* sumBuffer = (outType*) malloc ( nboxes * sizeof(outType) );

	// printf("SwSoftmax Layer : Time for exp sum : ");
	// TIME_STAMP_INIT

	IO_DATA_TYPE* inArray_8bit = (IO_DATA_TYPE*) inArray;

	IO_DATA_TYPE1 fval;
	//IO_DATA_TYPE1 fval1;
	IO_DATA_TYPE maxVal_8bit;
	IO_DATA_TYPE1 maxVal;
	SUB_DATA_TYPE fxval;


	//if(prev_layer_type == 4)//FC
	{

		for(int i=0; i<nboxes; i++)
		{

			//for(int batch_id = 0; batch_id < batch_size; batch_id++)
			for(int batch_id = 0; batch_id < batch_size_loopcnt; batch_id++)
			{

				int in_offset = i*nclasses*batch_size;

				maxVal = getMaxVal<IO_DATA_TYPE1>(inArray + in_offset, nclasses, batch_size, batch_id);


				float sumVal = 0.0f;
				for(int j=0; j<nclasses; j++)
				{
					//if(j==232)
					//fprintf(stderr, "stop");

					int tmpIdx = i*nclasses*batch_size + j*batch_size + batch_id;

					{
						fval = inArray[tmpIdx] - maxVal;
					}


#if 0
					fval1 = ref_output[tmpIdx] - maxVal1;

					if(fval != fval1)
					{
						fprintf(stderr, "j=%d fval=%f fval1=%f \n", j, fval, fval1);
					}
#endif
					//IO_DATA_TYPE1 fval = inArray[tmpIdx] - maxVal;

#if SOFTMAX_OPT
					float outVal = fval < SOFTMAX_THRESHOLD ? 0.0 : expf(fval);
#else
					//IO_DATA_TYPE1 outVal = expf(fval);
#if 1
					IO_DATA_TYPE1 outVal;
					if(fval > 0)
					{
						outVal = expf(fval);
					}
					else if(fval==0)
					{
						outVal = 1;
					}
					else if(fval < -10)
					{
						outVal = 0;
					}
					else if( (fval > -10) && (fval < 0) )
					{
						fval1 = -fval;
						table_idx = 1000 - (int(fval1 * 100));
						outVal = exp_lookup_table_float[table_idx];
					}
					else
					{
						outVal = 0;
					}

#endif
#if 0
					float outVal1 = expf(fval);

					//if(outVal1 != outVal)
					//fprintf(stderr, "j = %d  fval = %f  outVal=%f\n", j, fval, outVal1);
#endif
#endif

					//if(j==354)
					{
						//printf("stop");
					}
					outArray[tmpIdx] = outVal;
					sumVal += outVal;
					//outArray[tmpIdx] = outVal1;
					//sumVal += outVal1;
				}
				sumBuffer[i] = 1.0f/sumVal;											// Optim : Take reciprocal

				// Now outArray contains element-wise exponent. Now normalize them
				// printf("SwSoftmax Layer : Time for Normalization : ");
				IO_DATA_TYPE1 normParam = sumBuffer[i];
				for(int j=0; j<nclasses; j++)
				{
					int tmpIdx = i*nclasses*batch_size + j*batch_size + batch_id;
					outArray[tmpIdx] *= normParam;							// Optim : Multplify vs Divide

					//fprintf(stderr, "j = %d  outVal=%f\n", j, outArray[tmpIdx]);
				}
			}  //for(int i=0; i<nboxes; i++)

		}  //for(int batch_id = 0; batch_id < batch_size; batch_id++)
		// TIME_STAMP

		//free(sumBuffer);
	}
}

void SwSoftmaxWrapper_prevlayerpermute(IO_DATA_TYPE1* inArray, IO_DATA_TYPE1* sumBuffer, IO_DATA_TYPE1* outArray, int *scalar_softmax_args)
{

	int nclasses  = scalar_softmax_args[0];
	int nboxes    = scalar_softmax_args[1];
	int batch_size= scalar_softmax_args[3];
	int ip_fbits  = scalar_softmax_args[4];

	int quant_scheme_flag	  = scalar_softmax_args[7];
	int prev_layer_type = scalar_softmax_args[2];

	//# Read scale factor for input
	float *sf_in_ptr 	  = (float*)(&scalar_softmax_args[8]);
	float sf_in = sf_in_ptr[0];

	int batch_size_one_enable =  scalar_softmax_args[5];
	batch_size = XBATCH_SIZE;
	int batch_size_loopcnt;
	if(batch_size_one_enable)
	{
		batch_size_loopcnt = 1;
	}
	else
	{
		batch_size_loopcnt = batch_size;
	}

	float fval1;
	int table_idx;
	float fract_val;
	int fract_id;

#if SOFTMAX_OPT
	const float SOFTMAX_THRESHOLD = -6.0;
#endif

#if 0
	FILE *frefout=fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/GoogleNetWithoutLRN_New/caffe_ref/loss3_classifier_out.txt","r");
	if(frefout == NULL)
	{
		//std :: cout << "File not found - " << ref_path << std :: endl;
		//return -1;
		printf("file open error");
	}

	float *ref_output		= (float*)malloc(nclasses*nboxes*batch_size*sizeof(float));
	for(int i=0; i< nclasses*nboxes;i++)
	{
		fscanf(frefout, "%f ", &ref_output[2*i]);
		ref_output[2*i+1] = ref_output[2*i];
	}
	fclose(frefout);
#endif


	// Buffer to keep the sum of exponents for each box
	//outType* sumBuffer = (outType*) malloc ( nboxes * sizeof(outType) );

	// printf("SwSoftmax Layer : Time for exp sum : ");
	// TIME_STAMP_INIT

	IO_DATA_TYPE* inArray_8bit = (IO_DATA_TYPE*) inArray;

	IO_DATA_TYPE1 fval;
	//IO_DATA_TYPE1 fval1;
	IO_DATA_TYPE maxVal_8bit;
	IO_DATA_TYPE1 maxVal;
	SUB_DATA_TYPE fxval;


	//if(prev_layer_type == 6)//PERMUTE)
	{
		for(int i=0; i<nboxes; i++)
		{

			//for(int batch_id = 0; batch_id < batch_size; batch_id++)
			for(int batch_id = 0; batch_id < batch_size_loopcnt; batch_id++)
			{

				int in_offset = i*nclasses*batch_size;
				maxVal_8bit = getMaxVal<IO_DATA_TYPE>(inArray_8bit + in_offset, nclasses, batch_size, batch_id);

				float sumVal = 0.0f;
				for(int j=0; j<nclasses; j++)
				{
					//if(j==232)
					//fprintf(stderr, "stop");

					int tmpIdx = i*nclasses*batch_size + j*batch_size + batch_id;


					fxval = inArray_8bit[tmpIdx] - maxVal_8bit;

					//if(quant_scheme_flag == 1)
					{
						fval = ((IO_DATA_TYPE1)(fxval*sf_in));
					}
					/*
					else
					{
						fval = ((IO_DATA_TYPE1)(fxval))/(1 << ip_fbits);
					}
					 */


#if 0
					fval1 = ref_output[tmpIdx] - maxVal1;

					if(fval != fval1)
					{
						fprintf(stderr, "j=%d fval=%f fval1=%f \n", j, fval, fval1);
					}
#endif
					//IO_DATA_TYPE1 fval = inArray[tmpIdx] - maxVal;

#if SOFTMAX_OPT
					float outVal = fval < SOFTMAX_THRESHOLD ? 0.0 : expf(fval);
#else
					//IO_DATA_TYPE1 outVal = expf(fval);
#if 1
					IO_DATA_TYPE1 outVal;
					if(fval > 0)
					{
						outVal = expf(fval);
					}
					else if(fval==0)
					{
						outVal = 1;
					}
					else if(fval < -10)
					{
						outVal = 0;
					}
					else //if( (fval > -10) && (fval < 0) )
					{
						fval1 = -fval;
						table_idx = 1000 - (int(fval1 * 100));
						outVal = exp_lookup_table_float[table_idx];
					}
#endif
#if 0
					float outVal1 = expf(fval);

					//if(outVal1 != outVal)
					//fprintf(stderr, "j = %d  fval = %f  outVal=%f\n", j, fval, outVal1);
#endif
#endif

					//if(j==354)
					{
						//printf("stop");
					}
					outArray[tmpIdx] = outVal;
					sumVal += outVal;
					//outArray[tmpIdx] = outVal1;
					//sumVal += outVal1;
				}
				sumBuffer[i] = 1.0f/sumVal;											// Optim : Take reciprocal

				// Now outArray contains element-wise exponent. Now normalize them
				// printf("SwSoftmax Layer : Time for Normalization : ");
				IO_DATA_TYPE1 normParam = sumBuffer[i];
				for(int j=0; j<nclasses; j++)
				{
					int tmpIdx = i*nclasses*batch_size + j*batch_size + batch_id;
					outArray[tmpIdx] *= normParam;							// Optim : Multplify vs Divide

					//fprintf(stderr, "j = %d  outVal=%f\n", j, outArray[tmpIdx]);
				}
			}  //for(int i=0; i<nboxes; i++)

		}  //for(int batch_id = 0; batch_id < batch_size; batch_id++)
		// TIME_STAMP

		//free(sumBuffer);
	}

}

void SwSoftmaxWrapper_flmode(IO_DATA_TYPE1* inArray, IO_DATA_TYPE1* sumBuffer, IO_DATA_TYPE1* outArray, int *scalar_softmax_args)
{

	int nclasses  = scalar_softmax_args[0];
	int nboxes    = scalar_softmax_args[1];
	int batch_size= scalar_softmax_args[3];
	int ip_fbits  = scalar_softmax_args[4];

	int quant_scheme_flag	  = scalar_softmax_args[7];
	int prev_layer_type = scalar_softmax_args[2];

	//# Read scale factor for input
	float *sf_in_ptr 	  = (float*)(&scalar_softmax_args[8]);
	float sf_in = sf_in_ptr[0];

	int batch_size_one_enable =  scalar_softmax_args[5];
	batch_size = XBATCH_SIZE;
	int batch_size_loopcnt;
	if(batch_size_one_enable)
	{
		batch_size_loopcnt = 1;
	}
	else
	{
		batch_size_loopcnt = batch_size;
	}

	float fval1;
	int table_idx;
	float fract_val;
	int fract_id;

#if SOFTMAX_OPT
	const float SOFTMAX_THRESHOLD = -6.0;
#endif

#if 0
	FILE *frefout=fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/GoogleNetWithoutLRN_New/caffe_ref/loss3_classifier_out.txt","r");
	if(frefout == NULL)
	{
		//std :: cout << "File not found - " << ref_path << std :: endl;
		//return -1;
		printf("file open error");
	}

	float *ref_output		= (float*)malloc(nclasses*nboxes*batch_size*sizeof(float));
	for(int i=0; i< nclasses*nboxes;i++)
	{
		fscanf(frefout, "%f ", &ref_output[2*i]);
		ref_output[2*i+1] = ref_output[2*i];
	}
	fclose(frefout);
#endif


	// Buffer to keep the sum of exponents for each box
	//outType* sumBuffer = (outType*) malloc ( nboxes * sizeof(outType) );

	// printf("SwSoftmax Layer : Time for exp sum : ");
	// TIME_STAMP_INIT

	IO_DATA_TYPE* inArray_8bit = (IO_DATA_TYPE*) inArray;

	IO_DATA_TYPE1 fval;
	//IO_DATA_TYPE1 fval1;
	IO_DATA_TYPE maxVal_8bit;
	IO_DATA_TYPE1 maxVal;
	SUB_DATA_TYPE fxval;


	//if(prev_layer_type == 6)//PERMUTE)
	{
		for(int i=0; i<nboxes; i++)
		{

			//for(int batch_id = 0; batch_id < batch_size; batch_id++)
			for(int batch_id = 0; batch_id < batch_size_loopcnt; batch_id++)
			{

				int in_offset = i*nclasses*batch_size;
				maxVal_8bit = getMaxVal<IO_DATA_TYPE>(inArray_8bit + in_offset, nclasses, batch_size, batch_id);

				float sumVal = 0.0f;
				for(int j=0; j<nclasses; j++)
				{
					//if(j==232)
					//fprintf(stderr, "stop");

					int tmpIdx = i*nclasses*batch_size + j*batch_size + batch_id;


					fxval = inArray_8bit[tmpIdx] - maxVal_8bit;


					{
						fval = ((IO_DATA_TYPE1)(fxval))/(1 << ip_fbits);
					}


#if 0
					fval1 = ref_output[tmpIdx] - maxVal1;

					if(fval != fval1)
					{
						fprintf(stderr, "j=%d fval=%f fval1=%f \n", j, fval, fval1);
					}
#endif
					//IO_DATA_TYPE1 fval = inArray[tmpIdx] - maxVal;

#if SOFTMAX_OPT
					float outVal = fval < SOFTMAX_THRESHOLD ? 0.0 : expf(fval);
#else
					//IO_DATA_TYPE1 outVal = expf(fval);
#if 1
					IO_DATA_TYPE1 outVal;
					if(fval > 0)
					{
						outVal = expf(fval);
					}
					else if(fval==0)
					{
						outVal = 1;
					}
					else if(fval < -10)
					{
						outVal = 0;
					}
					else //if( (fval > -10) && (fval < 0) )
					{
						fval1 = -fval;
						table_idx = 1000 - (int(fval1 * 100));
						outVal = exp_lookup_table_float[table_idx];
					}
#endif
#if 0
					float outVal1 = expf(fval);

					//if(outVal1 != outVal)
					//fprintf(stderr, "j = %d  fval = %f  outVal=%f\n", j, fval, outVal1);
#endif
#endif

					if(j==354)
					{
						//printf("stop");
					}
					outArray[tmpIdx] = outVal;
					sumVal += outVal;
					//outArray[tmpIdx] = outVal1;
					//sumVal += outVal1;
				}
				sumBuffer[i] = 1.0f/sumVal;											// Optim : Take reciprocal

				// Now outArray contains element-wise exponent. Now normalize them
				// printf("SwSoftmax Layer : Time for Normalization : ");
				IO_DATA_TYPE1 normParam = sumBuffer[i];
				for(int j=0; j<nclasses; j++)
				{
					int tmpIdx = i*nclasses*batch_size + j*batch_size + batch_id;
					outArray[tmpIdx] *= normParam;							// Optim : Multplify vs Divide

					//fprintf(stderr, "j = %d  outVal=%f\n", j, outArray[tmpIdx]);
				}
			}  //for(int i=0; i<nboxes; i++)

		}  //for(int batch_id = 0; batch_id < batch_size; batch_id++)
		// TIME_STAMP

		//free(sumBuffer);
	}

}

void SwSoftmaxWrapper(IO_DATA_TYPE1* inArray, IO_DATA_TYPE1* sumBuffer, IO_DATA_TYPE1* outArray, int *scalar_softmax_args)
{

	int nclasses  = scalar_softmax_args[0];
	int nboxes    = scalar_softmax_args[1];
	int batch_size= scalar_softmax_args[3];
	int ip_fbits  = scalar_softmax_args[4];

	int quant_scheme_flag	  = scalar_softmax_args[7];
	int prev_layer_type = scalar_softmax_args[2];


	if(prev_layer_type == 6)//PERMUTE)
	{
		if(quant_scheme_flag == 1)
		{

			SwSoftmaxWrapper_prevlayerpermute(inArray, sumBuffer, outArray, scalar_softmax_args);
		}
		else
		{
			SwSoftmaxWrapper_flmode(inArray, sumBuffer, outArray, scalar_softmax_args);
		}

	}
	else
	{
		SwSoftmaxWrapper_prevlayerfcfloat(inArray, sumBuffer, outArray, scalar_softmax_args);
	}



}
// Function : SwSoftmax Layer
// inArray : Input array of size [nboxes x nclasses] eg : [7308 x 21] for SSD-300-PascalVOC
// outArray : Result array with same size
// nclasses : Number of classes in training (eg : 21 for PascalVOC, 1000 for ImageNet)
// nboxes : Number of boxes detected
// TODO : ABID : Better subtract the maxVal for numerical stability
//template <typename inType, typename outType>
void SwSoftmaxWrapper_float(IO_DATA_TYPE1* inArray, IO_DATA_TYPE1* sumBuffer, IO_DATA_TYPE1* outArray, int *scalar_softmax_args)
{

	int nclasses  = scalar_softmax_args[0];
	int nboxes    = scalar_softmax_args[1];
	int batch_size= scalar_softmax_args[3];
	int ip_fbits  = scalar_softmax_args[4];

	int quant_scheme_flag	  = scalar_softmax_args[8];

	//# Read scale factor for input
	float *sf_in_ptr 	  = (float*)(&scalar_softmax_args[7]);
	float sf_in = sf_in_ptr[0];

	int batch_size_one_enable =  scalar_softmax_args[5];
	batch_size = XBATCH_SIZE;
	int batch_size_loopcnt;
	if(batch_size_one_enable)
	{
		batch_size_loopcnt = 1;
	}
	else
	{
		batch_size_loopcnt = batch_size;
	}

	float fval1;
	int table_idx;
	float fract_val;
	int fract_id;

#if SOFTMAX_OPT
	const float SOFTMAX_THRESHOLD = -6.0;
#endif

#if 0
	FILE *frefout=fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/GoogleNetWithoutLRN_New/caffe_ref/loss3_classifier_out.txt","r");
	if(frefout == NULL)
	{
		//std :: cout << "File not found - " << ref_path << std :: endl;
		//return -1;
		printf("file open error");
	}

	float *ref_output		= (float*)malloc(nclasses*nboxes*batch_size*sizeof(float));
	for(int i=0; i< nclasses*nboxes;i++)
	{
		fscanf(frefout, "%f ", &ref_output[2*i]);
		ref_output[2*i+1] = ref_output[2*i];
	}
	fclose(frefout);
#endif


	// Buffer to keep the sum of exponents for each box
	//outType* sumBuffer = (outType*) malloc ( nboxes * sizeof(outType) );

	// printf("SwSoftmax Layer : Time for exp sum : ");
	// TIME_STAMP_INIT

	IO_DATA_TYPE1 fval;
	//IO_DATA_TYPE1 fval1;

	for(int i=0; i<nboxes; i++)
	{

		//for(int batch_id = 0; batch_id < batch_size; batch_id++)
		for(int batch_id = 0; batch_id < batch_size_loopcnt; batch_id++)
		{

			int in_offset = i*nclasses*batch_size;
			//IO_DATA_TYPE maxVal = getMaxVal<IO_DATA_TYPE>(inArray + in_offset, nclasses, batch_size, batch_id);
			IO_DATA_TYPE1 maxVal = getMaxVal<IO_DATA_TYPE1>(inArray + in_offset, nclasses, batch_size, batch_id);

			float sumVal = 0.0f;
			for(int j=0; j<nclasses; j++)
			{
				//if(j==232)
				//fprintf(stderr, "stop");

				int tmpIdx = i*nclasses*batch_size + j*batch_size + batch_id;

				//SUB_DATA_TYPE fxval = inArray[tmpIdx] - maxVal;
				fval = inArray[tmpIdx] - maxVal;

#if 0
				if(quant_scheme_flag == 1)
				{
					fval = ((IO_DATA_TYPE1)(fxval*sf_in));
				}
				else
				{
					fval = ((IO_DATA_TYPE1)(fxval))/(1 << ip_fbits);
				}
#endif

#if 0
				fval1 = ref_output[tmpIdx] - maxVal1;

				if(fval != fval1)
				{
					fprintf(stderr, "j=%d fval=%f fval1=%f \n", j, fval, fval1);
				}
#endif
				//IO_DATA_TYPE1 fval = inArray[tmpIdx] - maxVal;

#if SOFTMAX_OPT
				float outVal = fval < SOFTMAX_THRESHOLD ? 0.0 : expf(fval);
#else
				//IO_DATA_TYPE1 outVal = expf(fval);
#if 1
				IO_DATA_TYPE1 outVal;
				if(fval > 0)
				{
					outVal = expf(fval);
				}
				else if(fval==0)
				{
					outVal = 1;
				}
				else if(fval < -10)
				{
					outVal = 0;
				}
				else //if( (fval > -10) && (fval < 0) )
				{
					fval1 = -fval;
					table_idx = 1000 - (int(fval1 * 100));
					outVal = exp_lookup_table_float[table_idx];
				}
#endif
#if 0
				float outVal1 = expf(fval);

				//if(outVal1 != outVal)
				//fprintf(stderr, "j = %d  fval = %f  outVal=%f\n", j, fval, outVal1);
#endif
#endif

				if(j==354)
				{
					//printf("stop");
				}
				outArray[tmpIdx] = outVal;
				sumVal += outVal;
				//outArray[tmpIdx] = outVal1;
				//sumVal += outVal1;
			}
			sumBuffer[i] = 1.0f/sumVal;											// Optim : Take reciprocal

			// Now outArray contains element-wise exponent. Now normalize them
			// printf("SwSoftmax Layer : Time for Normalization : ");
			IO_DATA_TYPE1 normParam = sumBuffer[i];
			for(int j=0; j<nclasses; j++)
			{
				int tmpIdx = i*nclasses*batch_size + j*batch_size + batch_id;
				outArray[tmpIdx] *= normParam;							// Optim : Multplify vs Divide

				//fprintf(stderr, "j = %d  outVal=%f\n", j, outArray[tmpIdx]);
			}
		}  //for(int i=0; i<nboxes; i++)

	}  //for(int batch_id = 0; batch_id < batch_size; batch_id++)
	// TIME_STAMP

	//free(sumBuffer);
}


// Function : SwSoftmax Layer
// inArray : Input array of size [nboxes x nclasses] eg : [7308 x 21] for SSD-300-PascalVOC
// outArray : Result array with same size
// nclasses : Number of classes in training (eg : 21 for PascalVOC, 1000 for ImageNet)
// nboxes : Number of boxes detected
// TODO : ABID : Better subtract the maxVal for numerical stability
//template <typename inType, typename outType>
void SwSoftmaxWrapper1(IO_DATA_TYPE1* inArray, IO_DATA_TYPE1* sumBuffer, IO_DATA_TYPE1* outArray, int *scalar_softmax_args)
{

	int nclasses  = scalar_softmax_args[0];
	int nboxes    = scalar_softmax_args[1];
	int batch_size= scalar_softmax_args[3];
	int ip_fbits  = scalar_softmax_args[4];

	int quant_scheme_flag	  = scalar_softmax_args[8];

	//# Read scale factor for input
	float *sf_in_ptr 	  = (float*)(&scalar_softmax_args[7]);
	float sf_in = sf_in_ptr[0];

	int batch_size_one_enable =  scalar_softmax_args[5];
	batch_size = XBATCH_SIZE;
	int batch_size_loopcnt;
	if(batch_size_one_enable)
	{
		batch_size_loopcnt = 1;
	}
	else
	{
		batch_size_loopcnt = batch_size;
	}

	float fval1;
	int table_idx;
	float fract_val;
	int fract_id;

#if SOFTMAX_OPT
	const float SOFTMAX_THRESHOLD = -6.0;
#endif

#if 0
	FILE *frefout=fopen("/proj/sdxapps/refdes/API_MIGRATE/sd_card/quant_models/GoogleNetWithoutLRN_New/caffe_ref/loss3_classifier_out.txt","r");
	if(frefout == NULL)
	{
		//std :: cout << "File not found - " << ref_path << std :: endl;
		//return -1;
		printf("file open error");
	}

	float *ref_output		= (float*)malloc(nclasses*nboxes*batch_size*sizeof(float));
	for(int i=0; i< nclasses*nboxes;i++)
	{
		fscanf(frefout, "%f ", &ref_output[2*i]);
		ref_output[2*i+1] = ref_output[2*i];
	}
	fclose(frefout);
#endif


	// Buffer to keep the sum of exponents for each box
	//outType* sumBuffer = (outType*) malloc ( nboxes * sizeof(outType) );

	// printf("SwSoftmax Layer : Time for exp sum : ");
	// TIME_STAMP_INIT

	IO_DATA_TYPE1 fval;
	//IO_DATA_TYPE1 fval1;

	for(int i=0; i<nboxes; i++)
	{

		//for(int batch_id = 0; batch_id < batch_size; batch_id++)
		for(int batch_id = 0; batch_id < batch_size_loopcnt; batch_id++)
		{

			int in_offset = i*nclasses*batch_size;
			//IO_DATA_TYPE maxVal = getMaxVal<IO_DATA_TYPE>(inArray + in_offset, nclasses, batch_size, batch_id);
			IO_DATA_TYPE1 maxVal = getMaxVal<IO_DATA_TYPE1>(inArray + in_offset, nclasses, batch_size, batch_id);

			float sumVal = 0.0f;
			for(int j=0; j<nclasses; j++)
			{
				//if(j==232)
				//fprintf(stderr, "stop");

				int tmpIdx = i*nclasses*batch_size + j*batch_size + batch_id;

				//SUB_DATA_TYPE fxval = inArray[tmpIdx] - maxVal;
				fval = inArray[tmpIdx] - maxVal;

#if 0
				if(quant_scheme_flag == 1)
				{
					fval = ((IO_DATA_TYPE1)(fxval*sf_in));
				}
				else
				{
					fval = ((IO_DATA_TYPE1)(fxval))/(1 << ip_fbits);
				}
#endif

#if 0
				fval1 = ref_output[tmpIdx] - maxVal1;

				if(fval != fval1)
				{
					fprintf(stderr, "j=%d fval=%f fval1=%f \n", j, fval, fval1);
				}
#endif
				//IO_DATA_TYPE1 fval = inArray[tmpIdx] - maxVal;

#if SOFTMAX_OPT
				float outVal = fval < SOFTMAX_THRESHOLD ? 0.0 : expf(fval);
#else
				//IO_DATA_TYPE1 outVal = expf(fval);
#if 1
				IO_DATA_TYPE1 outVal;
				if(fval > 0)
				{
					outVal = expf(fval);
				}
				else if(fval==0)
				{
					outVal = 1;
				}
				else if(fval < -10)
				{
					outVal = 0;
				}
				else //if( (fval > -10) && (fval < 0) )
				{
					fval1 = -fval;
					table_idx = 1000 - (int(fval1 * 100));
					outVal = exp_lookup_table_float[table_idx];
				}
#endif
#if 0
				float outVal1 = expf(fval);

				//if(outVal1 != outVal)
				//fprintf(stderr, "j = %d  fval = %f  outVal=%f\n", j, fval, outVal1);
#endif
#endif

				if(j==354)
				{
					//printf("stop");
				}
				outArray[tmpIdx] = outVal;
				sumVal += outVal;
				//outArray[tmpIdx] = outVal1;
				//sumVal += outVal1;
			}
			sumBuffer[i] = 1.0f/sumVal;											// Optim : Take reciprocal

			// Now outArray contains element-wise exponent. Now normalize them
			// printf("SwSoftmax Layer : Time for Normalization : ");
			IO_DATA_TYPE1 normParam = sumBuffer[i];
			for(int j=0; j<nclasses; j++)
			{
				int tmpIdx = i*nclasses*batch_size + j*batch_size + batch_id;
				outArray[tmpIdx] *= normParam;							// Optim : Multplify vs Divide

				//fprintf(stderr, "j = %d  outVal=%f\n", j, outArray[tmpIdx]);
			}
		}  //for(int i=0; i<nboxes; i++)

	}  //for(int batch_id = 0; batch_id < batch_size; batch_id++)
	// TIME_STAMP

	//free(sumBuffer);
}



#if 0
void SwSoftmaxWrapper_prevlayerfc(IO_DATA_TYPE1* inArray1, IO_DATA_TYPE1* sumBuffer, IO_DATA_TYPE1* outArray1, int *scalar_softmax_args)
{

	int nclasses  = scalar_softmax_args[0];
	int nboxes    = scalar_softmax_args[1];
	int batch_size= scalar_softmax_args[3];
	int ip_fbits  = scalar_softmax_args[4];

	int quant_scheme_flag	  = scalar_softmax_args[8];

	//# Read scale factor for input
	float *sf_in_ptr 	  = (float*)(&scalar_softmax_args[7]);
	float sf_in = sf_in_ptr[0];

	int batch_size_one_enable =  scalar_softmax_args[5];
	batch_size = XBATCH_SIZE;
	int batch_size_loopcnt;
	if(batch_size_one_enable)
	{
		batch_size_loopcnt = 1;
	}
	else
	{
		batch_size_loopcnt = batch_size;
	}

	float fval1;
	int table_idx;
	float fract_val;
	int fract_id;

#if SOFTMAX_OPT
	const float SOFTMAX_THRESHOLD = -6.0;
#endif



	// Buffer to keep the sum of exponents for each box
	//outType* sumBuffer = (outType*) malloc ( nboxes * sizeof(outType) );

	// printf("SwSoftmax Layer : Time for exp sum : ");
	// TIME_STAMP_INIT

	IO_DATA_TYPE1 fval;

	//for(int i=0; i<nboxes; i++)
	for(int batch_id = 0; batch_id < batch_size_loopcnt; batch_id++)
	{

		IO_DATA_TYPE1* inArray = &inArray1[batch_id*nboxes*nclasses]
										   //for(int batch_id = 0; batch_id < batch_size; batch_id++)
										   //for(int batch_id = 0; batch_id < batch_size_loopcnt; batch_id++)
										   for(int i=0; i<nboxes; i++)
										   {

											   int in_offset = i*nclasses*batch_size;
											   IO_DATA_TYPE1 maxVal = getMaxVal_float<IO_DATA_TYPE1>(inArray, nclasses);

											   float sumVal = 0.0f;
											   for(int j=0; j<nclasses; j++)
											   {
												   //if(j==232)
												   //fprintf(stderr, "stop");

												   int tmpIdx = i*nclasses + j;
												   fval = inArray[tmpIdx] - maxVal;

												   //IO_DATA_TYPE1 fval = inArray[tmpIdx] - maxVal;

#if SOFTMAX_OPT
												   float outVal = fval < SOFTMAX_THRESHOLD ? 0.0 : expf(fval);
#else
												   //IO_DATA_TYPE1 outVal = expf(fval);
#if 1
												   IO_DATA_TYPE1 outVal;
												   if(fval > 0)
												   {
													   outVal = expf(fval);
												   }
												   else if(fval==0)
												   {
													   outVal = 1;
												   }
												   else if(fval < -10)
												   {
													   outVal = 0;
												   }
												   else //if( (fval > -10) && (fval < 0) )
												   {
													   fval1 = -fval;
													   table_idx = 1000 - (int(fval1 * 100));
													   outVal = exp_lookup_table_float[table_idx];
												   }
#endif
#if 0
												   float outVal1 = expf(fval);

												   //if(outVal1 != outVal)
												   fprintf(stderr, "j = %d  fval = %f  outVal=%f\n", j, fval, outVal1);
#endif
#endif
												   outArray[tmpIdx] = outVal;
												   sumVal += outVal;
											   }
											   sumBuffer[i] = 1.0f/sumVal;											// Optim : Take reciprocal

											   // Now outArray contains element-wise exponent. Now normalize them
											   // printf("SwSoftmax Layer : Time for Normalization : ");
											   IO_DATA_TYPE1 normParam = sumBuffer[i];
											   for(int j=0; j<nclasses; j++)
											   {
												   int tmpIdx = i*nclasses + j;
												   outArray[tmpIdx] *= normParam;							// Optim : Multplify vs Divide
											   }
										   }  //for(int i=0; i<nboxes; i++)

	}  //for(int batch_id = 0; batch_id < batch_size; batch_id++)
	// TIME_STAMP

	//free(sumBuffer);
}
#endif

#if 0
// Function : SwSoftmax Layer
// inArray : Input array of size [nboxes x nclasses] eg : [7308 x 21] for SSD-300-PascalVOC
// outArray : Result array with same size
// nclasses : Number of classes in training (eg : 21 for PascalVOC, 1000 for ImageNet)
// nboxes : Number of boxes detected
// TODO : ABID : Better subtract the maxVal for numerical stability
//template <typename inType, typename outType>
void SwSoftmaxWrapper(float* inArray, float* sumBuffer, float* outArray, int *scalar_softmax_args)
{

	int nclasses  = scalar_softmax_args[0];
	int nboxes    = scalar_softmax_args[1];

#if SOFTMAX_OPT
	const float SOFTMAX_THRESHOLD = -6.0;
#endif

	// Buffer to keep the sum of exponents for each box
	//outType* sumBuffer = (outType*) malloc ( nboxes * sizeof(outType) );

	// printf("SwSoftmax Layer : Time for exp sum : ");
	// TIME_STAMP_INIT
	for(int i=0; i<nboxes; i++)
	{
		float maxVal = getMaxVal<float>(inArray + i*nclasses, nclasses);

		float sumVal = 0.0f;
		for(int j=0; j<nclasses; j++)
		{
			int tmpIdx = i*nclasses + j;
			float fval = inArray[tmpIdx] - maxVal;
#if SOFTMAX_OPT
			float outVal = fval < SOFTMAX_THRESHOLD ? 0.0 : expf(fval);
#else
			float outVal;
			if(fval<-10)
			{
				outVal = 0;
			}
			else
			{
				int table_idx = int(fval);
				outVal = exp_lookup_table_float[table_idx*100];
			}
			//float outVal = expf(fval);
#endif
			outArray[tmpIdx] = outVal;
			sumVal += outVal;
		}
		sumBuffer[i] = 1.0f/sumVal;											// Optim : Take reciprocal

		float normParam = sumBuffer[i];
		for(int j=0; j<nclasses; j++)
		{
			outArray[i*nclasses + j] *= normParam;							// Optim : Multplify vs Divide
		}

	}
	// TIME_STAMP

#if 0
	// Now outArray contains element-wise exponent. Now normalize them
	// printf("SwSoftmax Layer : Time for Normalization : ");
	// TIME_STAMP_INIT
	for(int i=0; i<nboxes; i++)
	{
		float normParam = sumBuffer[i];
		for(int j=0; j<nclasses; j++)
		{
			outArray[i*nclasses + j] *= normParam;							// Optim : Multplify vs Divide
		}
	}
	// TIME_STAMP
#endif

	//free(sumBuffer);
}

#endif
