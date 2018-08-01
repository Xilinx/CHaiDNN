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

#include <string.h>
#include "../include/hw_settings.h"
#include "checkers.hpp"
#include "checkers_utils.hpp"


//# Checks funtionality
int errorCheck(
		xChangeLayer inLayer
)
{
	std :: cout << std :: endl;
	
#if LAYERWISE_OUTPUT_WRITE
	std :: cout << "[CHECKS] Start : Output Write\n" << std :: endl;
#else
	std :: cout << "[CHECKS] Start : errorCheck()\n" << std :: endl;
#endif

	int status = 0;

	const char *ref_path = inLayer.ref_path;
	const char *out_path = inLayer.out_path;

	char network_name[100];
	for(int i=0;i<100;i++)
	{
		network_name[i]=0;
	}
	{
		char *path = inLayer.out_path;
		char *ssc;
		ssc = strstr(path, "models");
		//ssc = strstr(path, "2017.4");
		int l = strlen(ssc) + 1;
		path = &path[strlen(path)-l+8];
		ssc = strstr(path, "/");
		l = strlen(ssc) + 1;
		int l1 = strlen(path)+1;
		for(int i=0;i<(l1-l);i++)
		{
			network_name[i]=path[i];
		}
		//fprintf(stderr,"%s\n",network_name);
	}

	FILE *csv_fp;

#if EN_HISTOGRAM
	char csv_path[500];
	//char *csv_path = (char *)malloc(500*sizeof(char));
#ifdef __SDSOC
	sprintf(csv_path, "logs/%s_histogram.csv", network_name);
#else
	sprintf(csv_path, "%s/out/%s_histogram.csv", inLayer.base_path, network_name);
#endif

	csv_fp = fopen(csv_path, "a+");
	if(csv_fp == NULL)
	{
		fprintf(stderr, "can't create csv file - %s\n", csv_path);
		return -1;
	}

	int previous = inLayer.previous[0].seqidx;
	if(previous == -1)
	{
		fprintf(csv_fp, "layer name, straddle, slk, err_0_10, err_10_90, err_90_100, Batch Status\n");
	}

	std :: cout << "[PATH] csv_path : " << csv_path << std :: endl;
#endif

	FILE *error_fp;

#if EN_FILE_WRITE
	char error_path[500];
#ifdef __SDSOC
	sprintf(error_path, "logs/%s_error.txt", network_name);
#else
	sprintf(error_path, "%s/out/%s_error.txt", inLayer.base_path, network_name);
#endif

	error_fp = fopen(error_path, "a+");
	if(error_fp == NULL)
	{
		fprintf(stderr, "can't create error log file - %s\n", error_path);
		return -1;
	}

	fprintf(error_fp, "\n\n[PATH] ref_path : %s\n", ref_path);
	fprintf(error_fp, "[PATH] out_path : %s\n", out_path);

	for(uint8_t idx = 0; idx < MAX_PARAM_SIZE; ++idx)
		fprintf(error_fp, "%d ", ((int*)inLayer.params)[idx]);
	fprintf(error_fp, "\n");
#endif  //#if EN_FILE_WRITE

#if !LAYERWISE_OUTPUT_WRITE
	std :: cout << "[REPATH] ref_path : " << ref_path << std :: endl;
#endif

	std :: cout << "[WRPATH] out_path : " << out_path << std :: endl;

	if(ref_path == NULL)
	{
		std :: cout << "Incorrect ref path : " << ref_path << std :: endl;
		return -1;
	}

	if(inLayer.kernType == CONV)
	{
#if PACKED_INOUT
		status = cpCheck_packed(inLayer);
#else
		int *params = (int*)inLayer.params;
		//if(((int*)inLayer.params)[34] == OPCODE_FC2CONV)
		if(params[34] == OPCODE_FC2CONV)
		{
			status = fcCheck(inLayer, csv_fp, error_fp);
		}
		else
			status = cpCheck(inLayer, csv_fp, error_fp);
#endif
	}
	else if(inLayer.kernType == POOL)
	{
#if PACKED_INOUT
		status = cpCheck_packed(inLayer);
#else
		status = cpCheck(inLayer, csv_fp, error_fp);
#endif
	}
	else if(inLayer.kernType == FC_LAYER)
	{
		//status = fcCheck(inLayer, csv_fp, error_fp);
		status = swfcCheck(inLayer, csv_fp, error_fp);
	}
	else if(inLayer.kernType == SOFTMAX)
	{
		status = swSoftmaxCheck(inLayer, csv_fp, error_fp);
	}
	else if(inLayer.kernType == CROP)
	{
		status = cropCheck(inLayer, error_fp);
	}
	else if(inLayer.kernType == DECONV)
	{
		status = deconvCheck(inLayer, error_fp);
	}
	else if(inLayer.kernType == NORM)
	{
#if PACKED_INOUT
		status = cpCheck_packed(inLayer);
#else
		//status = normCheck(inLayer, csv_fp, error_fp);
		status = cpCheck(inLayer, csv_fp, error_fp);
#endif
	}
	else if(inLayer.kernType == PERMUTE)
	{
		status = permuteCheck(inLayer, csv_fp, error_fp);
	}
	else if(inLayer.kernType == NMS)
	{
		status = nmsCheck(inLayer, csv_fp, error_fp);
	}
	else if(inLayer.kernType == XCUSTOM)
	{
		status = xcustCheck(inLayer, error_fp);
	}
#if EN_HISTOGRAM
	fclose(csv_fp);
#endif

#if EN_FILE_WRITE
	fclose(error_fp);
#endif
	//free(network_name);
	//free(csv_path);

	
#if LAYERWISE_OUTPUT_WRITE
	std :: cout << "\n[CHECKS] End : Output Write\n" << std :: endl;
#else
	std :: cout << "\n[CHECKS] End : errorCheck()\n" << std :: endl;
#endif

	if(status != 0)
		return -1;
	else
		return 0;

}
//errorCheck

//# Enable Output file write
int outFileWrite(
		xChangeLayer inLayer,
		const char *img_path
)
{
	//# Fetch image name
	const char *n_path = img_path;

	for(short i = strlen(img_path)-1; i; i--)
	{
		if (img_path[i] == '/')
		{
			n_path = img_path+i+1;
			break;
		}
	}

	int cnt = 0;
	int n = strlen(n_path);
	for(short i = 0; i < strlen(n_path)-1; i++)
	{
		if(n_path[i] == '.')
		{
			break;
		}
		else
		{
			cnt++;
		}
	}

	char tmp[100];
	strncpy(tmp, n_path, cnt);
	tmp[cnt] = '\0';

#if STR
	//# Replace the '.' with '_'
	std :: string input1 = (std :: string)n_path;
	replace(input1.begin(), input1.end(), '.', '_');
	const char *path1 = input1.c_str();
	//fprintf(stderr, "%s\n", path1);
#endif

	char outPath[500];
	sprintf(outPath, "%s/out/%s_out.txt", inLayer.base_path, tmp);

	std :: cout << "[OUTWR] Saving " << outPath << std :: endl;

	int status;
	const char *out_path = outPath;

	if(inLayer.kernType == SOFTMAX)
	{
		status = swSoftmaxWrite(inLayer, out_path);
	}
	else if(inLayer.kernType == CROP)
	{
		status = cropWrite(inLayer, out_path);
	}
	else if(inLayer.kernType == NMS)
	{
		status = nmsWrite(inLayer, out_path);
	}

	//	std :: cout << "[SAVING] Done " << std :: endl;

	if(status != 0)
		return -1;
	else
		return 0;


}
