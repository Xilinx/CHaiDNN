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

#include "checkers.hpp"
#include "checkers_utils.hpp"
#include "../include/hw_settings.h"



//# Checks funtionality
int errorCheck(
		xChangeLayer inLayer
)
{
	std :: cout << std :: endl;
	std :: cout << "[CHECKS] *****************************	start : errorCheck()" << std :: endl;

	int status = 0;

	const char *ref_path = inLayer.ref_path;
	const char *out_path = inLayer.out_path;

	//char csv_path[500];
	//char *path = inLayer.base_path;
	//sprintf(csv_path, "%s/histogram.csv", path);
	//std :: cout << "csv_path : " << csv_path << std :: endl;
	char network_name[100];
	{
		char *path = inLayer.out_path;
		char *ssc;
		ssc = strstr(path, "models");
		int l = strlen(ssc) + 1;
		path = &path[strlen(path)-l+8];
		ssc = strstr(path, "/");
		l = strlen(ssc) + 1;
		int l1 = strlen(path)+1;
		for(int i=0;i<(l1-l);i++)
		{
			network_name[i]=path[i];
		}
		fprintf(stderr,"%s\n",network_name);
	}

	FILE *csv_fp;

#if EN_HISTOGRAM
	char csv_path[100];
	//char *csv_path = "histogram.csv";
	sprintf(csv_path, "%s_histogram.csv",network_name);
	csv_fp = fopen(csv_path, "a+");
	if(csv_fp == NULL)
	{
		fprintf(stderr, "can't create csv file - %s\n", "histogram.csv");//csv_path);
	}
	
	int previous = inLayer.previous[0].seqidx;
	if(previous == -1)
	{
		fprintf(csv_fp, "err_0_10, err_10_90, err_90_100");
	}
	
	std :: cout << "[PATH] csv_path : " << csv_path << std :: endl;
#endif

//#if EN_HISTOGRAM
	char csv_path[100];
	//char *csv_path = "histogram.csv";
	sprintf(csv_path, "error.txt");
	FILE *error_fp = fopen(csv_path, "a+");
	if(error_fp == NULL)
	{
		fprintf(stderr, "can't create csv file - %s\n", csv_path);//csv_path);
	}
//#endif

	std :: cout << "[PATH] ref_path : " << ref_path << std :: endl;
	std :: cout << "[PATH] out_path : " << out_path << std :: endl;

	fprintf(error_fp, "[PATH] ref_path : %s\n", ref_path);
	fprintf(error_fp, "[PATH] out_path : %s\n", out_path);

	if(ref_path == NULL)
	{
		std :: cout << "Incorrect ref path : " << ref_path << std :: endl;
		return -1;
	}

	if(inLayer.kernType == CONV)
	{
#if PACKED_INOUT
		status = cpCheck_packed(inLayer, ref_path, out_path);
#else
		status = cpCheck(inLayer, ref_path, out_path, csv_fp);//, error_fp);
#endif
	}
	else if(inLayer.kernType == POOL)
	{
#if PACKED_INOUT
		status = cpCheck_packed(inLayer, ref_path, out_path);
#else
		status = cpCheck(inLayer, ref_path, out_path, csv_fp);//, error_fp);
#endif
	}
	else if(inLayer.kernType == FC_LAYER)
	{
		status = fcCheck(inLayer, ref_path, out_path, csv_fp);
	}
	else if(inLayer.kernType == SOFTMAX)
	{
		//softmaxCheck(inLayer, ref_path, out_path);
		status = swSoftmaxCheck(inLayer, ref_path, out_path, csv_fp);
	}
	else if(inLayer.kernType == CROP)
	{
		status = cropCheck(inLayer, ref_path, out_path);
	}
	else if(inLayer.kernType == NORM)
	{
#if PACKED_INOUT
		status = cpCheck_packed(inLayer, ref_path, out_path);
#else
		status = normCheck(inLayer, ref_path, out_path, csv_fp);
#endif
	}
	else if(inLayer.kernType == PERMUTE)
	{
		status = permuteCheck(inLayer, ref_path, out_path, csv_fp);
	}
	else if(inLayer.kernType == NMS)
	{
		status = nmsCheck(inLayer, ref_path, out_path, csv_fp);
	}

#if EN_HISTOGRAM
	fclose(csv_fp);
#endif

	fclose(error_fp);

	std :: cout << "[CHECKS] *****************************	End : errorCheck()" << std :: endl;

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
