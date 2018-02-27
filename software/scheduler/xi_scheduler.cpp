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

#include "xi_scheduler.hpp"

//# Checks Dependence of a layer
bool chkDeps(std::vector<bool> &layerDone, std::vector<layerID> &previous)
{
	bool retFlag = true;
	uint16_t nProds = previous.size();
#if ENABLE_CONSOLE_TEXT
	std::cout << "[DEBUG] Previous ID : " << previous[0].seqidx << std::endl;
#endif
	if(previous[0].seqidx == -1)
		return true;
	for(uint16_t prod = 0; prod < nProds; ++prod)
		retFlag &= layerDone[previous[prod].seqidx];
	return retFlag;
}


//# Wrapper function for FCForward
void FcForwardWrapper(
		CHAR_TYPE *A1, CHAR_TYPE *A2,
		SHORT_TYPE *in1, SHORT_TYPE *in2,
		SHORT_TYPE *in3, SHORT_TYPE *y_in,
		SHORT_TYPE *y_out, int *scalar_fc_args,
		int fc_id, int reD, int reH, int reW
)
{
	short *fc_in;
	if(fc_id == 0)
	{
		//# Re-arrange input data
		fc_inputdatawrite(in1, in2, reH, reW, reD, 0, in3, NULL);
		fc_in = in3;
#if ENABLE_CONSOLE_TEXT
		std::cout << "\n[DEBUG] Re-arrange: done !" << std::endl;
#endif
	}
	else
	{
		fc_in = in1;
	}

	//# Call FCForward
	FcForward(A1, A2, fc_in, y_in, y_out, scalar_fc_args);

}

//# Wrapper function for DeconvForward
void DeconvForwardWrapper(short* in1, short* in2, short* in3, short* weights, short* bias, int * out, int *scalars)
{ 
	//# Re-arrage previous layer output for Deconv input
	DeconvInReArrange(in1, in2, in3, scalars[1], scalars[2], scalars[0]);
	//# Call Deconv
	DeconvForward(in3, weights, bias, (unsigned long long int*)out, scalars);
}

//# Scheduler for all the layers/tasks in the network in optimal way
//# to the PL or PS

unsigned short int xiExec(std::vector<xChangeLayer> hwQueue[NUM_IMG], void *inptr1, void *outptr1, int inp_bytes)
{
	//# Number of layers to be scheduled
	uint16_t totalLayers = hwQueue[0].size();
	if(totalLayers <= 0)
	{
		std::cerr << "\n[ERROR] Invalid Queue size !" << std::endl;
		return totalLayers;
	}
	else
	{
#if ENABLE_ERROR_CHECKS
		std::cerr << "\n[INFOx] Total Layers : " << totalLayers << std::endl << std::endl;
#endif
	}

	//# Layer-wise Sequence IDs
	std::vector<int> convSeq;
	std::vector<int> poolSeq;
	std::vector<int> fcSeq;
	std::vector<int> softmaxSeq;
	std::vector<int> deconvSeq;
	std::vector<int> nmsSeq;
	std::vector<int> permuteSeq;
	std::vector<int> normSeq;
	std::vector<int> cropSeq;

	//# Initialize layer-wise sequence
	for(uint16_t idx = 0; idx < totalLayers; ++idx)
	{
		kernel_type_e layerType = hwQueue[0][idx].kernType;
		switch (layerType) 
		{
		case CONV: 		convSeq.push_back(idx); 	break;
		case POOL: 		poolSeq.push_back(idx); 	break;
		case FC_LAYER: 	fcSeq.push_back(idx); 		break;
		case SOFTMAX: 	softmaxSeq.push_back(idx); 	break;
		case DECONV: 	deconvSeq.push_back(idx); 	break;
		case NORM: 		normSeq.push_back(idx); 	break;
		case NMS: 		nmsSeq.push_back(idx); 		break;
		case PERMUTE: 	permuteSeq.push_back(idx); 	break;
		case CROP: 		cropSeq.push_back(idx); 	break;
		}
#if ENABLE_ERROR_CHECKS
		hwQueue[0][idx].DisplayParams(idx);
#endif
	}

	//# Count of Different layers
	uint16_t tConvLayers 	= convSeq.size();
	uint16_t tPoolLayers 	= poolSeq.size();
	uint16_t tFCLayers		= fcSeq.size();
	uint16_t tSoftMaxLayers = softmaxSeq.size();
	uint16_t tDeconvLayers	= deconvSeq.size();
	uint16_t tNormLayers	= normSeq.size();
	uint16_t tPermuteLayers = permuteSeq.size();
	uint16_t tNmsLayers		= nmsSeq.size();
	uint16_t tCropLayers	= cropSeq.size();

#if ENABLE_ERROR_CHECKS
	std::cout << "[INFOx] Total Conv Layers :    " << tConvLayers << std::endl;
	std::cout << "[INFOx] Total Pool Layers :    " << tPoolLayers << std::endl;
	std::cout << "[INFOx] Total FC Layers :      " << tFCLayers << std::endl;
	std::cout << "[INFOx] Total Norm Layers :    " << tNormLayers << std::endl;
	std::cout << "[INFOx] Total Permute Layers : " << tPermuteLayers << std::endl;
	std::cout << "[INFOx] Total SoftMax Layers : " << tSoftMaxLayers << std::endl;
	std::cout << "[INFOx] Total NMS Layers :     " << tNmsLayers << std::endl;
	std::cout << "[INFOx] Total Deconv Layers :  " << tDeconvLayers << std::endl;
	std::cout << "[INFOx] Total Crop Layers :    " << tCropLayers << std::endl;
#endif

	//# Counter for all the layers
	uint16_t convCnt[NUM_IMG] = {0},
			poolCnt[NUM_IMG] = {0},
			fcCnt[NUM_IMG] = {0},
			softmaxCnt[NUM_IMG] = {0},
			deconvCnt[NUM_IMG] = {0},
			normCnt[NUM_IMG] = {0},
			nmsCnt[NUM_IMG] = {0},
			permuteCnt[NUM_IMG] = {0},
			cropCnt[NUM_IMG] = {0};

	//# In-use flags
	bool convInUse 		= false;
	bool poolInUse 		= false;
	bool fcInUse 		= false;
	bool softmaxInUse 	= false;
	bool deconvInUse 	= false;
	bool nmsInUse 		= false;
	bool permuteInUse 	= false;
	bool normInUse 		= false;
	bool cropInUse 		= false;
	bool ImreadInUse	= false;

	//# Image IDs for different layers
	int convImgId, poolImgId, deconvImgId, fcImgId,
	softmaxImgId, normImgId, permuteImgId, nmsImgId,
	cropImgId;

	//# Done flags for all the layers
	std::vector<bool> layerDone[NUM_IMG];

	//# Reset flags before scheduling
	for(uint16_t nImg = 0; nImg < NUM_IMG; ++nImg)
	{
		//# Initialize done flags
		for(uint16_t idx = 0; idx < totalLayers; ++idx)
		{
			layerDone[nImg].push_back(false);
		}
	}

#if MANUAL_LAYER_ENABLE
	totalLayers = total_layers;
#endif
	uint16_t lastLayerIdx = totalLayers - 1; 	//# Last layer index
	uint16_t ImageDoneCount = 0;				//# Number of images completed
	uint16_t ImageDispatchCount = NUM_IMG;		//# Number of images dispatched
	int ImgId;									//# Image ID for parallel image execution

	//# Software thread & done flags
	pthread_t softmaxThread, normThread, nmsThread, permuteThread, imageReadThread,
	cropThread;

	uint8_t normThreadDone 		= 0;
	uint8_t nmsThreadDone  		= 0;
	uint8_t softmaxThreadDone	= 0;
	uint8_t permuteThreadDone	= 0;
	uint8_t cropThreadDone 		= 0;
	uint8_t imageReadDone 		= 0;

#if LIST_INPUT
	//# Read Images
	FILE * ImageListFp	= fopen(ImageListPath, "r");
	if(ImageListFp == NULL)
	{
		std::cout << "\n[ERROR] Can not find Image List File : " << ImageListPath;
		std::cout << std::endl << std::endl;
		return -1;
	}

	//# Create Image Read Thread argument structure
	ImgReadThreadArgs imreadArg;
	imreadArg.ImageReadDone = &imageReadDone;

	//# Read n-images prior to scheduling
	//# Initialize
	imreadArg.fp = ImageListFp;
	inputImageRead(&ImageListFp, &hwQueue[0][0]);
#if (NUM_IMG==2)
	inputImageRead(&ImageListFp, &hwQueue[1][0]);
#endif

#else //#LIST_INPUT
	if(inp_bytes == 2)
	{
		//	loadData(ImageListPath, &hwQueue[0][0]);
		loadDatafromBuffptr((short *)inptr1, &hwQueue[0][0]);
#if (NUM_IMG==2)
		//	loadData(ImageListPath, &hwQueue[1][0]);
		loadDatafromBuffptr((short *)inptr2, &hwQueue[1][0]);
#endif
	}
	else
	{
		loadImagefromBuffptr((unsigned char *)inptr1, &hwQueue[0][0]);
		//	inputImageRead(ImageListPath, &hwQueue[0][0]);
#if (NUM_IMG==2)
		//	inputImageRead(ImageListPath, &hwQueue[1][0]);
		loadImagefromBuffptr((unsigned char *)inptr2, &hwQueue[1][0]);
#endif
	}
#endif  //#if LIST_INPUT


	//# Initiate Image Read flags 
	bool CanReadImage[NUM_IMG];
	for(uint8_t img = 0; img < NUM_IMG; ++img)
		CanReadImage[img] = true;

	//# Check flags for all individual layers done
	bool allPoolDone, allConvDone, allFCDone, allSoftMaxDone,
	allPermuteDone, allDeconvDone, allNmsDone, allNormDone,
	allCropDone;

	int totalImages = NUM_IMG;//total_layers;
	//int totalImages = 20;

	//# Create Crop thread argument structure
	CropThreadArgs cropArgs;
	//# Initialize
	cropArgs.cropThreadDone = &cropThreadDone;

	int reH, reW, reD;
	if(!fcSeq.empty())
	{
		int prevLayer = hwQueue[0][fcSeq[0]].previous[0].seqidx;
		reH = *((int*)(hwQueue[0][prevLayer].params) + 2);
		reW = *((int*)(hwQueue[0][prevLayer].params) + 3);
		reD = *((int*)(hwQueue[0][prevLayer].params) + 4) << 1;
	}

#if ENABLE_SCHEDULER
	//# Scheduler Entry Point ################################################
	while(1)
	{
#if ENABLE_CONSOLE_TEXT
		std::cout << "[DEBUG] while(1)" << std::endl;
#endif

#if ENABLE_IMAGE_READ_THREAD
		//#TODO : Write conditional code to enable thread for Image read: Stop Reading based on number of images dispatched
		if((ImreadInUse == false) && (ImageDispatchCount < totalImages))
		{
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] ImreadInUse == false " << std::endl;
#endif
			for(ImgId = 0; ImgId < NUM_IMG; ++ImgId)
			{
				if(CanReadImage[ImgId] && layerDone[ImgId][0])
				{
					imreadArg.firstLayer = &hwQueue[ImgId][0];
					pthread_create(&imageReadThread, NULL, imageReadRoutine, &imreadArg);
					ImreadInUse = true;
					CanReadImage[ImgId] = false;
					++ImageDispatchCount;
					break;
				}
			}
		}
#endif//ENABLE_IMAGE_READ_THREAD

#if NEEDED_CONV
		if((convInUse == false) && tConvLayers)
		{
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] convInUse == false " << std::endl;
#endif
			for(ImgId = 0; ImgId < NUM_IMG; ++ImgId)
			{
				//# Check all conv layers are done or not
				allConvDone = (convCnt[ImgId] == tConvLayers) ? true : false;
				//# Get the convolution layer ID
				uint16_t whichConv = convSeq[convCnt[ImgId]];
				//# Check dependencies are satisfied or not
				if(!allConvDone && (chkDeps(layerDone[ImgId], hwQueue[ImgId][whichConv].previous)))
				{
#if ENABLE_CONSOLE_TEXT
					std::cout << "[DEBUG] Weights Ptrs : " << hwQueue[ImgId][whichConv].wts_ptrs[0] << " , " << hwQueue[ImgId][whichConv].wts_ptrs[1] << std::endl; 
#endif								
					//# Call Conv wrapper
					ConvolutionForward(
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].wts_ptrs[0], (CHAR_TYPE*)hwQueue[ImgId][whichConv].wts_ptrs[1],
							(SHORT_TYPE*)hwQueue[ImgId][whichConv].out_ptrs[0], (SHORT_TYPE*)hwQueue[ImgId][whichConv].out_ptrs[1],
							(SHORT_TYPE*)hwQueue[ImgId][whichConv].in_ptrs[0], (SHORT_TYPE*)hwQueue[ImgId][whichConv].in_ptrs[1],
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].in_ptrs[2],
							(SHORT_TYPE*)hwQueue[ImgId][whichConv].bias_ptr,
							//# New args added
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].in_ptrs[3], (CHAR_TYPE*)hwQueue[ImgId][whichConv].in_ptrs[4],		
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].out_ptrs[0], (CHAR_TYPE*)hwQueue[ImgId][whichConv].out_ptrs[1],
							(INT_TYPE *)hwQueue[ImgId][whichConv].params
					);

					convImgId = ImgId; 
					convInUse = true; 
#if ENABLE_CONSOLE_TEXT
					std::cout << "[DEBUG] convForward : " << convImgId << ", " << allConvDone << std::endl;
#endif
					break;
				}
			}
		}
#endif//NEEDED_CONV

#if NEEDED_POOL
		if((poolInUse == false) && tPoolLayers)
		{
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] poolInUse == false " << std::endl;
#endif
			for(ImgId = 0; ImgId < NUM_IMG; ++ImgId)
			{
				allPoolDone = (poolCnt[ImgId] == tPoolLayers) ? true : false;
				uint16_t whichPool = poolSeq[poolCnt[ImgId]];
				if(!allPoolDone && (chkDeps(layerDone[ImgId], hwQueue[ImgId][whichPool].previous)))
				{
					//# Call Pool wrapper
					PoolForward(
							(SHORT_TYPE*)hwQueue[ImgId][whichPool].in_ptrs[0], (SHORT_TYPE*)hwQueue[ImgId][whichPool].out_ptrs[0],
							(SHORT_TYPE*)hwQueue[ImgId][whichPool].in_ptrs[1], (SHORT_TYPE*)hwQueue[ImgId][whichPool].out_ptrs[1],
							(INT_TYPE*)hwQueue[ImgId][whichPool].params
					);

					poolImgId = ImgId;
					poolInUse = true; 
#if ENABLE_CONSOLE_TEXT
					std::cout << "[DEBUG] poolForward : " << poolImgId << std::endl;
#endif
					break;
				}
			}
		}
#endif//NEEDED_POOL

#if NEEDED_FC
		if((fcInUse == false) && tFCLayers)
		{
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] fcInUse == false " << std::endl;
#endif
			for(ImgId = 0; ImgId < NUM_IMG; ++ImgId)
			{
				uint16_t fcCount = fcCnt[ImgId];

				allFCDone = (fcCount == tFCLayers) ? true : false;
				uint16_t whichFc = fcSeq[fcCount];
				if(!allFCDone && (chkDeps(layerDone[ImgId], hwQueue[ImgId][whichFc].previous)))
				{
					//# Call FC wrapper
					FcForwardWrapper(
							(CHAR_TYPE*)hwQueue[ImgId][whichFc].wts_ptrs[0],
							(CHAR_TYPE*)hwQueue[ImgId][whichFc].wts_ptrs[1],
							(SHORT_TYPE *)hwQueue[ImgId][whichFc].in_ptrs[0],
							(SHORT_TYPE *)hwQueue[ImgId][whichFc].in_ptrs[1],
							(SHORT_TYPE *)hwQueue[ImgId][whichFc].in_ptrs[2],
							(SHORT_TYPE *)hwQueue[ImgId][whichFc].bias_ptr,
							(SHORT_TYPE *)hwQueue[ImgId][whichFc].out_ptrs[0],
							(INT_TYPE*)hwQueue[ImgId][whichFc].params,
							fcCnt[ImgId], reD, reH, reW);
					fcImgId = ImgId; 
					fcInUse = true; 
#if ENABLE_CONSOLE_TEXT
					std::cout << "[DEBUG] FCForward : " << fcImgId << std::endl;
#endif
					break;
				}
			}
		}
#endif//NEEDED_FC

#if NEEDED_DECONV
		if((deconvInUse == false) && tDeconvLayers)
		{
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] deconvInUse == false " << std::endl;
#endif
			for(ImgId = 0; ImgId < NUM_IMG; ++ImgId)
			{
				allDeconvDone = (deconvCnt[ImgId] == tDeconvLayers) ? true : false;
				uint16_t whichDeconv = deconvSeq[deconvCnt[ImgId]];
				if(!allDeconvDone && (chkDeps(layerDone[ImgId], hwQueue[ImgId][whichDeconv].previous)))
				{
					DeconvForwardWrapper(
							(SHORT_TYPE*)hwQueue[ImgId][whichDeconv].in_ptrs[0], (SHORT_TYPE*)hwQueue[ImgId][whichDeconv].in_ptrs[1],
							(SHORT_TYPE*)hwQueue[ImgId][whichDeconv].in_ptrs[2], (SHORT_TYPE*)hwQueue[ImgId][whichDeconv].wts_ptrs[0],
							(SHORT_TYPE*)hwQueue[ImgId][whichDeconv].bias_ptr, (INT_TYPE*)hwQueue[ImgId][whichDeconv].out_ptrs[0],
							(INT_TYPE*)hwQueue[ImgId][whichDeconv].params);
					deconvImgId = ImgId; 
					deconvInUse = true; break;
				}
			}
		}
#endif//NEEDED_DECONV

#if NEEDED_NORM
		if((normInUse == false) && tNormLayers)
		{
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] normInUse == false " << std::endl;
#endif
			for(ImgId = 0; ImgId < NUM_IMG; ++ImgId)
			{
				allNormDone = (normCnt[ImgId] == tNormLayers) ? true : false;
				uint16_t whichNorm = normSeq[normCnt[ImgId]];
				if(!allNormDone && (chkDeps(layerDone[ImgId], hwQueue[ImgId][whichNorm].previous)))
				{
#if ENABLE_CONSOLE_TEXT
					std::cout << "[DEBUG] normForward" << std::endl;
#endif
					//# TODO: 
					//pthread_create(&imageReadThread, NULL, imageReadRoutine, &imreadArg);
#if PTHREAD
					pthread_create(&normThread, NULL, normRoutine, (void *)&hwQueue[ImgId][whichNorm]);
#else
					normRoutine((void *)&hwQueue[ImgId][whichNorm]);
#endif
					normImgId = ImgId; 
					normInUse = true;
					break;
				}
			}

		}
#endif//NEEDED_NORM

#if NEEDED_PERMUTE	
		if((permuteInUse == false) && tPermuteLayers)
		{
			for(ImgId = 0; ImgId < NUM_IMG; ++ImgId)
			{
				allPermuteDone = (permuteCnt[ImgId] == tPermuteLayers) ? true : false;
				uint16_t whichPermute = permuteSeq[permuteCnt[ImgId]];
				if(!allPermuteDone && (chkDeps(layerDone[ImgId], hwQueue[ImgId][whichPermute].previous)))
				{
					//# TODO: 
					//pthread_create(&permuteThread, NULL, permuteRoutine, &permuteThreadDone);
#if PTHREAD
					pthread_create(&permuteThread, NULL, permuteRoutine, (void *)&hwQueue[ImgId][whichPermute]);
#else
					permuteRoutine((void *)&hwQueue[ImgId][whichPermute]);
#endif

					permuteImgId = ImgId; 
					permuteInUse = true;
					break;
				}
			}

		}
#endif//NEEDED_PERMUTE

#if NEEDED_SOFTMAX
		if((softmaxInUse == false) && tSoftMaxLayers)
		{
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] softmaxInUse == false " << std::endl;
#endif

			for(ImgId = 0; ImgId < NUM_IMG; ++ImgId)
			{
				allSoftMaxDone = (softmaxCnt[ImgId] == tSoftMaxLayers) ? true : false;
				uint16_t whichSoftmax = softmaxSeq[softmaxCnt[ImgId]];
				if(!allSoftMaxDone && (chkDeps(layerDone[ImgId], hwQueue[ImgId][whichSoftmax].previous)))
				{
					//# Call SoftMax wrapper
					//pthread_create(&softmaxThread, NULL, softmaxRoutine, &softmaxThreadDone);
#if PTHREAD
					pthread_create(&softmaxThread, NULL, softmaxRoutine, (void *)&hwQueue[ImgId][whichSoftmax]);
#else
					softmaxRoutine((void *)&hwQueue[ImgId][whichSoftmax]);
#endif
					softmaxImgId = ImgId;
					softmaxInUse = true;
#if ENABLE_CONSOLE_TEXT
					std::cout << "[DEBUG] softmaxForward : " << softmaxImgId << ", " << allSoftMaxDone << std::endl;
#endif
					break;
				}
			}
		}
#endif//NEEDED_SOFTMAX

#if NEEDED_NMS
		if((nmsInUse == false) && tNmsLayers)
		{

			for(ImgId = 0; ImgId < NUM_IMG; ++ImgId)
			{
				allNmsDone = (nmsCnt[ImgId] == tNmsLayers) ? true : false;
				uint16_t whichNms = nmsSeq[nmsCnt[ImgId]];
				if(!allNmsDone && (chkDeps(layerDone[ImgId], hwQueue[ImgId][whichNms].previous)))
				{
					//# TODO:
					//pthread_create(&nmsThread, NULL, nmsRoutine, &nmsThreadDone);
#if PTHREAD
					pthread_create(&nmsThread, NULL, nmsRoutine, (void *)&hwQueue[ImgId][whichNms]);
#else
					nmsRoutine((void *)&hwQueue[ImgId][whichNms]);
#endif
					nmsImgId = ImgId;
					nmsInUse = true; break;
				}
			}

		}
#endif//NEEDED_NMS

#if NEEDED_CROP
		if((cropInUse == false) && tCropLayers)
		{
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] cropInUse == false " << std::endl;
#endif
			for(ImgId = 0; ImgId < NUM_IMG; ++ImgId)
			{
				allCropDone = (cropCnt[ImgId] == tCropLayers) ? true : false;
				//# Get the Crop layer ID
				uint16_t whichCrop = cropSeq[cropCnt[ImgId]];
				//# Check dependencies are satisfied or not
				if(!allCropDone && (chkDeps(layerDone[ImgId], hwQueue[ImgId][whichCrop].previous)))
				{
					//# Call Crop layer
					//std::cout << "\n\n[DEBUG] Calling Thread : Crop" << std::endl;
					cropArgs.Layer = &hwQueue[ImgId][whichCrop];
					pthread_create(&cropThread, NULL, cropRoutine, &cropArgs);
					//std::cout << "\n\n[DEBUG] Calling Thread : Crop : Done" << std::endl;
					cropImgId = ImgId; 
					cropInUse = true; 
					break;
				}
			}
		}
#endif//NEEDED_CROP

		//#######################################################################################//

#if NEEDED_CONV
		if(convInUse == true)
		{
			//# wait for conv to be completed
#ifdef __SDSOC
			sds_wait(1);
#endif

			//# condition for groups
			uint16_t whichConv = convSeq[convCnt[convImgId]];
			int group_enable = *(((int*)hwQueue[convImgId][whichConv].params) + 30);
			if(group_enable)
			{
				((int*)hwQueue[convImgId][whichConv].params)[11] = 1;

#if ENABLE_CONSOLE_TEXT
				std::cout << "[DEBUG] convForward, Group : " << convImgId << std::endl;
#endif

				ConvolutionForward(
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].wts_ptrs[0], (CHAR_TYPE*)hwQueue[convImgId][whichConv].wts_ptrs[1],
						(SHORT_TYPE*)hwQueue[convImgId][whichConv].out_ptrs[0], (SHORT_TYPE*)hwQueue[convImgId][whichConv].out_ptrs[1],
						(SHORT_TYPE*)hwQueue[convImgId][whichConv].in_ptrs[0], (SHORT_TYPE*)hwQueue[convImgId][whichConv].in_ptrs[1],
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].in_ptrs[2],
						(SHORT_TYPE*)hwQueue[convImgId][whichConv].bias_ptr,
						//# New args added
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].in_ptrs[3], (CHAR_TYPE*)hwQueue[convImgId][whichConv].in_ptrs[4],
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].out_ptrs[0], (CHAR_TYPE*)hwQueue[convImgId][whichConv].out_ptrs[1],
						(INT_TYPE *)hwQueue[convImgId][whichConv].params
				);
#ifdef __SDSOC
				sds_wait(1);
#endif
				((int*)hwQueue[convImgId][whichConv].params)[11] = 0;
			}
#if ENABLE_ERROR_CHECKS
			if(convImgId == 0)
			{
				int convErr = errorCheck(hwQueue[convImgId][whichConv]);
				if(convErr)
					std::cout << "\n[ERROR] Conv Layer : " << whichConv << " Image : " << convImgId << " Fail !" << std::endl;
				else
					std::cout << "\n[ERROR] Conv Layer : " << whichConv << " Image : " << convImgId << " Pass !" << std::endl;
			}
#endif		
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] convForward : Done : Image : " << convImgId << " Layer : " << convCnt[convImgId] << std::endl;
#endif
			layerDone[convImgId][whichConv] = true;
			convCnt[convImgId]++;
			convInUse = false;

		}
#endif//NEEDED_CONV

#if NEEDED_POOL
		if(poolInUse == true)
		{
#ifdef __SDSOC
			if(sds_try_wait(2))
#else
				if(1)
#endif
				{
#if ENABLE_CONSOLE_TEXT
					std::cout << "[DEBUG] poolForward : Done : Image : " << poolImgId << " Layer : " << poolCnt[poolImgId] << std::endl;
#endif
#if ENABLE_ERROR_CHECKS
					if(poolImgId == 0)
					{
						int poolErr = errorCheck(hwQueue[poolImgId][poolSeq[poolCnt[poolImgId]]]);
						if(poolErr)
							std::cout << "\n[ERROR] Pool Layer : " << poolSeq[poolCnt[poolImgId]] << " Image : " << poolImgId << " Fail !" << std::endl;
						else
							std::cout << "\n[ERROR] Pool Layer : " << poolSeq[poolCnt[poolImgId]] << " Image : " << poolImgId << " Pass !" << std::endl;
					}
#endif
					layerDone[poolImgId][poolSeq[poolCnt[poolImgId]]] = true;
					poolCnt[poolImgId]++;
					poolInUse = false;
				}
				else
				{
					poolInUse = true;
				}
		}
#endif//NEEDED_POOL

#if NEEDED_FC
		if(fcInUse == true)
		{
#ifdef __SDSOC
			if(sds_try_wait(3))
#else
				if(1)
#endif
				{
#if ENABLE_CONSOLE_TEXT
					std::cout << "[DEBUG] fcForward : Done " << fcImgId << std::endl;
#endif
#if ENABLE_ERROR_CHECKS
					if(fcImgId == 0){
						int fcErr = errorCheck(hwQueue[fcImgId][fcSeq[fcCnt[fcImgId]]]);
						if(fcErr)
							std::cout << "\n[ERROR] FC Layer : " << fcSeq[fcCnt[fcImgId]] << " Image : " << fcImgId << " Fail !" << std::endl;
						else
							std::cout << "\n[ERROR] FC Layer : " << fcSeq[fcCnt[fcImgId]] << " Image : " << fcImgId << " Pass !" << std::endl;
					}
#endif
					layerDone[fcImgId][fcSeq[fcCnt[fcImgId]]] = true;
					fcCnt[fcImgId]++;
					fcInUse = false;
				}
				else
				{
					fcInUse = true;
				}
		}
#endif//NEEDED_FC

#if NEEDED_DECONV
		if(deconvInUse == true)
		{
#ifdef __SDSOC
			if(sds_try_wait(5))
#else
				if(1)
#endif
				{
#if ENABLE_ERROR_CHECKS
					if(deconvImgId == 0){
						uint16_t whichDeconv = deconvSeq[deconvCnt[deconvImgId]];
						int deconvErr = errorCheck(hwQueue[deconvImgId][whichDeconv]);
						if(deconvErr)
							std::cout << "\n[ERROR] Deconv Layer : " << whichDeconv << " Image : " << deconvImgId << " Fail !" << std::endl;
						else
							std::cout << "\n[ERROR] Deconv Layer : " << whichDeconv << " Image : " << deconvImgId << " Pass !" << std::endl;
					}
#endif	
					layerDone[deconvImgId][deconvSeq[deconvCnt[deconvImgId]]] = true;
					deconvCnt[deconvImgId]++;
					deconvInUse = false;
				}
				else
				{
					deconvInUse = true;
				}
		}
#endif//NEEDED_DECONV

#if NEEDED_NORM
		if(normInUse == true)
		{
			uint16_t whichNorm = normSeq[normCnt[normImgId]];
			normThreadDone = hwQueue[normImgId][whichNorm].layer_done[0];

			//# Check for thread completion
			if(normThreadDone)
			{
#if PTHREAD
				int normRet = pthread_join(normThread, NULL);
#else
				int normRet = 0;
#endif

#if ENABLE_CONSOLE_TEXT
				if(normRet != 0)
				{ std::cerr << "\n[ERROR] normThread Fail ! " << "Image : " << normImgId << ", Layer ID : " << normSeq[normCnt[normImgId]] << std::endl; }
				else
				{ std::cout << "** normForward : Done" << std::endl; }
#endif
				hwQueue[normImgId][whichNorm].layer_done[0] = 0;
				normThreadDone = 0;

#if ENABLE_ERROR_CHECKS
				if(normImgId == 0){
					int normErr = errorCheck(hwQueue[normImgId][whichNorm]);
					if(normErr)
						std::cout << "\n[ERROR] Norm Layer : " << whichNorm << " Image : " << normImgId << " Fail !" << std::endl;
					else
						std::cout << "\n[ERROR] Norm Layer : " << whichNorm << " Image : " << normImgId << " Pass !" << std::endl;
				}
#endif
				layerDone[normImgId][normSeq[normCnt[normImgId]]] = true;
				normCnt[normImgId]++; 
				normInUse = false;
			}
		}
#endif//NEEDED_NORM

#if NEEDED_PERMUTE
		if(permuteInUse == true)
		{
			uint16_t whichPermute = permuteSeq[permuteCnt[permuteImgId]];
			permuteThreadDone = hwQueue[permuteImgId][whichPermute].layer_done[0];

			//# Check for thread completion
			if(permuteThreadDone)
			{
#if PTHREAD
				int permuteRet = pthread_join(permuteThread, NULL);
#else
				int permuteRet = 0;
#endif
#if ENABLE_CONSOLE_TEXT
				if(permuteRet != 0)
				{ std::cerr << "\n[ERROR] permuteThread Fail ! " << "Image : " << permuteImgId << ", Layer ID : " << permuteSeq[permuteCnt[permuteImgId]] << std::endl; }
				else
				{ std::cout << "** permuteForward : Done" << std::endl; }
#endif
				hwQueue[permuteImgId][whichPermute].layer_done[0] = 0;
				permuteThreadDone = 0;

#if ENABLE_ERROR_CHECKS
				if(permuteImgId == 0){
					int permuteErr = errorCheck(hwQueue[permuteImgId][whichPermute]);
					if(permuteErr)
						std::cout << "\n[ERROR] Permute Layer : " << whichPermute << " Image : " << permuteImgId << " Fail !" << std::endl;
					else
						std::cout << "\n[ERROR] Permute Layer : " << whichPermute << " Image : " << permuteImgId << " Pass !" << std::endl;
				}
#endif
				layerDone[permuteImgId][permuteSeq[permuteCnt[permuteImgId]]] = true;
				permuteCnt[permuteImgId]++; 
				permuteInUse = false;
			}
		}
#endif//NEEDED_PERMUTE

#if NEEDED_SOFTMAX
		if(softmaxInUse == true)
		{
			uint16_t whichSoftmax = softmaxSeq[softmaxCnt[softmaxImgId]];
			softmaxThreadDone = hwQueue[softmaxImgId][whichSoftmax].layer_done[0];

			//# Check for thread completion
			if(softmaxThreadDone)
			{
#if PTHREAD
				int softmaxRet = pthread_join(softmaxThread, NULL);
#else
				int softmaxRet = 0;
#endif
#if ENABLE_CONSOLE_TEXT
				if(softmaxRet != 0)
				{ std::cerr << "\n[ERROR] softmaxThread Fail ! " << "Image : " << softmaxImgId << ", Layer ID : " << softmaxSeq[softmaxCnt[softmaxImgId]] << std::endl; }
				else
				{ std::cout << "** softmaxForward : Done" << std::endl; }
#endif
				hwQueue[softmaxImgId][whichSoftmax].layer_done[0] = 0;
				softmaxThreadDone = 0;

#if ENABLE_ERROR_CHECKS
				if(softmaxImgId == 0){
					int softmaxErr = errorCheck(hwQueue[softmaxImgId][whichSoftmax]);
					if(softmaxErr)
						std::cout << "\n[ERROR] Softmax Layer : " << whichSoftmax << " Image : " << softmaxImgId << " Fail !" << std::endl;
					else
						std::cout << "\n[ERROR] Softmax Layer : " << whichSoftmax << " Image : " << softmaxImgId << " Pass !" << std::endl;
				}
#endif
				layerDone[softmaxImgId][softmaxSeq[softmaxCnt[softmaxImgId]]] = true;
				softmaxCnt[softmaxImgId]++;
				softmaxInUse = false;
			}
		}
#endif//NEEDED_NORM

#if NEEDED_NMS
		if(nmsInUse == true)
		{
			uint16_t whichNms = nmsSeq[nmsCnt[nmsImgId]];
			nmsThreadDone = hwQueue[nmsImgId][whichNms].layer_done[0];

			//# Check for thread completion
			if(nmsThreadDone)
			{
#if PTHREAD
				int nmsRet = pthread_join(nmsThread, NULL);
#else
				int nmsRet = 0;
#endif
#if ENABLE_CONSOLE_TEXT
				if(nmsRet != 0)
				{ std::cerr << "\n[ERROR] nmsThread Fail ! " << "Image : " << nmsImgId << ", Layer ID : " << nmsSeq[nmsCnt[nmsImgId]] << std::endl; }
				else
				{ std::cout << "** nmsForward : Done" << std::endl; }
#endif
				hwQueue[normImgId][whichNms].layer_done[0] = 0;
				nmsThreadDone = 0;

#if ENABLE_ERROR_CHECKS
				if(nmsImgId == 0){
					int nmsErr = errorCheck(hwQueue[nmsImgId][whichNms]);
					if(nmsErr)
						std::cout << "\n[ERROR] Nms Layer : " << whichNms << " Image : " << nmsImgId << " Fail !" << std::endl;
					else
						std::cout << "\n[ERROR] Nms Layer : " << whichNms << " Image : " << nmsImgId << " Pass !" << std::endl;
				}
#endif
				layerDone[nmsImgId][nmsSeq[nmsCnt[nmsImgId]]] = true;
				nmsCnt[nmsImgId]++;
				nmsInUse = false;
			}
		}
#endif//NEEDED_NMS

#if NEEDED_CROP
		if(cropInUse == true)
		{
			//# Check for thread completion
			if(cropThreadDone)
			{
				//# Join thread
				int cropRet = pthread_join(cropThread, NULL);
				if(cropRet != 0)
				{ std::cerr << "\n[ERROR] cropThread Fail ! " << "Image : " << cropImgId << ", Layer ID : " << cropSeq[cropCnt[cropImgId]] << std::endl; }
#if ENABLE_ERROR_CHECKS
				if(cropImgId == 0){
					uint16_t whichCrop = cropSeq[cropCnt[cropImgId]];
					int cropErr = errorCheck(hwQueue[cropImgId][whichCrop]);
					if(cropErr)
						std::cout << "\n[ERROR] Crop Layer : " << whichCrop << " Image : " << cropImgId << " Fail !" << std::endl;
					else
						std::cout << "\n[ERROR] Crop Layer : " << whichCrop << " Image : " << cropImgId << " Pass !" << std::endl;
				}
#endif
				cropThreadDone = 0;
				layerDone[cropImgId][cropSeq[cropCnt[cropImgId]]] = true;
				cropCnt[cropImgId]++; 
				cropInUse = false;
			}
		}
#endif//NEEDED_CROP

#if ENABLE_IMAGE_READ_THREAD
		if(ImreadInUse == true)
		{
			if(imageReadDone)
			{
				//#TODO: 
				int imreadRet = pthread_join(imageReadThread, NULL);
#if ENABLE_CONSOLE_TEXT
				if(imreadRet != 0){ 
					std::cerr << "\n[ERROR] imageReadThread Fail !" << std::endl;
				}
				else { 
					std::cout << "[DEBUG] imreadForward : Done : Image : "<< ImageDispatchCount << std::endl; 
				}
#endif
				imageReadDone = 0;
				ImreadInUse = false;
			}
		}
#endif//ENABLE_IMAGE_READ_THREAD

#if RESET_DONE_FLAGS
		//# Check Last Layer : Done ?
		if(layerDone[0][lastLayerIdx] == true)
		{
			++ImageDoneCount;
#if ENABLE_CONSOLE_TEXT			
			std::cout << "\n[DEBUG] Image Done Count : " << ImageDoneCount << std::endl;
#endif
			//# Re-initialize done flags
			for(uint16_t idx = 0; idx < totalLayers; ++idx)
			{
				layerDone[0][idx] = false;
			}
			poolCnt[0] = 0; fcCnt[0] = 0; softmaxCnt[0] = 0; 
			deconvCnt[0] = 0; normCnt[0] = 0; nmsCnt[0] = 0; permuteCnt[0] = 0; cropCnt[0] = 0;
			CanReadImage[0] = true;
			if(ImageDoneCount >= totalImages-1)
				convCnt[0] = tConvLayers;
			else
				convCnt[0]  = 0;
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] Image Done Count : " << ImageDoneCount << "ConvCnt[0] : " << convCnt[0] << std::endl;
#endif
		}
#if (NUM_IMG==2)
		if(layerDone[1][lastLayerIdx] == true)
		{
			++ImageDoneCount;
#if ENABLE_CONSOLE_TEXT			
			std::cout << "\n[DEBUG] Image Done Count : " << ImageDoneCount << std::endl;
#endif
			//# Re-initialize done flags
			for(uint16_t idx = 0; idx < totalLayers; ++idx)
			{
				layerDone[1][idx] = false;
			}
			poolCnt[1] = 0; fcCnt[1] = 0; softmaxCnt[1] = 0; 
			deconvCnt[1] = 0; normCnt[1] = 0; nmsCnt[1] = 0; permuteCnt[1] = 0; cropCnt[1] = 0;
			CanReadImage[1] = true;
			if(ImageDoneCount >= totalImages-1)
				convCnt[1] = tConvLayers;
			else
				convCnt[1]  = 0;
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] Image Done Count : " << ImageDoneCount << "ConvCnt[1] : " << convCnt[1] << std::endl;
#endif

		}
#endif
#endif//RESET_DONE_FLAGS

		//# Break scheduler loop based on number of images
		if(ImageDoneCount == totalImages)
			break;

	}//# while(1) ############################################################

#if 1
	//# For Classification Networks
	if(hwQueue[0][lastLayerIdx].kernType == SOFTMAX)
	{
		//# Output Buffer Write
		float *hls_out1	= (float*)hwQueue[0][totalLayers-1].out_ptrs[0];
		int    n_elems  = ((int*)hwQueue[0][totalLayers-1].params)[0];

		memcpy(((float*)outptr1), hls_out1, n_elems*sizeof(float));
		
#if NUM_IMG==2
		float *hls_out2	= (float*)hwQueue[1][totalLayers-1].out_ptrs[0];
		memcpy(((float*)outptr2), hls_out2, n_elems*sizeof(float));
#endif
	}
	//# For Detection Networks
	else if (hwQueue[0][lastLayerIdx].kernType == NMS)
	{
		//# Output Buffer Write
		float *hls_out1	= (float*)hwQueue[0][totalLayers-1].xtra_ptrs[2];
		int *nms_cnt1 = (int*)hwQueue[0][totalLayers-1].out_ptrs[1];
		int BoxCount1 = nms_cnt1[0];
		memcpy((int*)outptr1, &BoxCount1, sizeof(int));
		memcpy(((float*)outptr1)+1, hls_out1, BoxCount1*7*sizeof(float));

#if NUM_IMG==2		
		float *hls_out2	= (float*)hwQueue[1][totalLayers-1].xtra_ptrs[2];
		int *nms_cnt2 = (int*)hwQueue[1][totalLayers-1].out_ptrs[1];
		int BoxCount2 = nms_cnt2[0];
		memcpy((int*)outptr2, &BoxCount2, sizeof(int));	
		memcpy(((float*)outptr2)+1, hls_out2, BoxCount2*7*sizeof(float));
#endif
	}
	//# For Segmentation Networks
	else if (hwQueue[0][lastLayerIdx].kernType == CROP)
	{
		int *hls_out1	= (int*)hwQueue[0][totalLayers-1].out_ptrs[0];
		int *params		= (int*)hwQueue[0][totalLayers-1].params;
		
		int outH = params[4];
		int outW = params[5];

		memcpy(((int*)outptr1), hls_out1, outH*outW*sizeof(int));
		
#if NUM_IMG==2
		int *hls_out2	= (int*)hwQueue[1][totalLayers-1].out_ptrs[0];
		memcpy(((int*)outptr2), hls_out2, outH*outW*sizeof(int));
#endif
	}
#endif

#endif//ENABLE_SCHEDULER

	return 0;
}
