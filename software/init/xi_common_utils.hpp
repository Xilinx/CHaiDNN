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

#ifndef _XCHANGE_INIT_UTILS_HPP_
#define _XCHANGE_INIT_UTILS_HPP_

//void Trim2FixedPoint(vector<float>& data, const int bw, const int fl, RoundingMethod rounding )
void Trim2FixedPoint(float *data, const int bw, const int fl, RoundingMethod rounding, int data_size )
{
	float pow_bw_minus_1 = pow(2, bw-1);
	float pow_minus_fl = pow(2, -fl);
	float pow_plus_fl = pow(2, fl);
	float max_data = (pow_bw_minus_1 - 1) * pow_minus_fl;
	float min_data = -pow_bw_minus_1 * pow_minus_fl;

	for (int index = 0; index < data_size; ++index)
	{
		// Saturate data
		data[index] = std::max(std::min(data[index], max_data), min_data);

		// Round data
		data[index] /= pow_minus_fl;
		switch (rounding) {
		case ROUND_NEAREST:
			data[index] = round(data[index]);
			break;
			// case ROUND_STOCHASTIC:
				//     data[index] = floor(data[index] + RandUniform_cpu());
			//     break;
		default:
			break;
		}
		data[index] *= pow_minus_fl;
	}
}

#endif //_XCHANGE_INIT_UTILS_HPP_
