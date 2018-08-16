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

//# Scheduler for all the layers/tasks in the network in optimal way
//# to the PL or PS
void xiExec(void *handle, vector<void *> input, vector<void *> output)
{
	if(handle == NULL)
	{
		fprintf(stderr, "Failed to read handle\n");
	}
	
	chaihandle_t *chaihandle_info = (chaihandle*)handle;
	std::vector<xChangeLayer> *hwQueue = chaihandle_info->JobQueue;
	
    //# Number of layers to be scheduled
    uint16_t totalLayers = hwQueue[0].size();
    
	//# Number of layers to be scheduled
	// uint16_t totalLayers = hwQueue[0].size();
	if(totalLayers <= 0)
	{
		std::cerr << "\n[ERROR] Invalid Queue size !" << std::endl;
		return;
	}
	else
	{
#if ENABLE_ERROR_CHECKS
		std::cerr << "\n[INFOx] Total Layers : " << totalLayers << std::endl << std::endl;
#endif
	}

	/* Assigning user's input and output pointers to scheduler jobqueue */
	if((hwQueue[0][0].kernType == CONV))// && (layer1_or_not == 1))
	{
		hwQueue[0][0].in_ptrs[2] = (IO_DATA_TYPE *)input[0];
	}
	else
	{
		for(int i = 0; i < input.size(); i++)
		{
			hwQueue[0][0].in_ptrs[i] = (IO_DATA_TYPE *)input[i];
		}
	}

	//# Last layer index
	uint16_t lastLayerIdx = totalLayers - 1;

	for(int i = 0; i < output.size(); i++)
	{
		hwQueue[0][lastLayerIdx].out_ptrs[i] = output[i];
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

	std::vector<int> xcustomSeq;
	std::vector<int> xpackSeq;
	std::vector<int> eltwaddSeq;

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
		case XCUSTOM:	xcustomSeq.push_back(idx);	break;
		case XPACK: 	xpackSeq.push_back(idx);	break;
		case ELTWISEADD:eltwaddSeq.push_back(idx);	break;
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

	uint16_t txCustomLayers	= xcustomSeq.size();
	uint16_t txPackLayers	= xpackSeq.size();
	uint16_t tEltwaddLayers	= eltwaddSeq.size();

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
	std::cout << "[INFOx] Total xCustom Layers : " << txCustomLayers << std::endl;
	std::cout << "[INFOx] Total xPack Layers :   " << txPackLayers << std::endl;
	std::cout << "[INFOx] Total Eltwadd Layers :   " << tEltwaddLayers << std::endl;
#endif

	//# Counter for all the layers
	uint16_t convCnt[NUM_IMG]   = {0},
			poolCnt[NUM_IMG]    = {0},
			fcCnt[NUM_IMG]      = {0},
			softmaxCnt[NUM_IMG] = {0},
			deconvCnt[NUM_IMG]  = {0},
			normCnt[NUM_IMG]    = {0},
			nmsCnt[NUM_IMG]     = {0},
			permuteCnt[NUM_IMG] = {0},
			cropCnt[NUM_IMG]    = {0},
			xcustomCnt[NUM_IMG] = {0},
			xpackCnt[NUM_IMG]   = {0},
			eltwaddCnt[NUM_IMG] = {0};

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

	bool xcustomInUse 	= false;
	bool xpackInUse 	= false;
	bool eltwaddInUse 	= false;

	bool ImreadInUse	= false;

	//# Image IDs for different layers
	int convImgId, poolImgId, deconvImgId, fcImgId,
	softmaxImgId, normImgId, permuteImgId, nmsImgId,
	cropImgId,xcustomImgId,xpackImgId,eltwaddImgId;

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

	uint16_t ImageDoneCount = 0;				//# Number of images completed
	uint16_t ImageDispatchCount = NUM_IMG;		//# Number of images dispatched
	int ImgId;									//# Image ID for parallel image execution

	//# Software thread & done flags
	pthread_t softmaxThread, normThread, nmsThread, permuteThread, imageReadThread,
	          cropThread, xcustomThread, xpackThread, eltwaddThread;

	uint8_t normThreadDone 		= 0;
	uint8_t nmsThreadDone  		= 0;
	uint8_t softmaxThreadDone	= 0;
	uint8_t permuteThreadDone	= 0;
	uint8_t cropThreadDone 		= 0;

	uint8_t xcustomThreadDone	= 0;
	uint8_t	xpackThreadDone		= 0;
	uint8_t	eltwaddThreadDone	= 0;

	//# Check flags for all individual layers done
	bool allPoolDone, allConvDone, allFCDone, allSoftMaxDone,
	allPermuteDone, allDeconvDone, allNmsDone, allNormDone,
	allCropDone,allxCustomDone,allxPackDone,allEltwaddDone;

	int totalImages = NUM_IMG;//total_layers;
	//int totalImages = 20;

	//# Create Crop thread argument structure
	CropThreadArgs cropArgs;
	xCustomThreadArgs xcustomArgs;
	//Custom Xcustom_obj;
	//xcustomArgs.Xcustom_obj = Xcustom_obj;
	xPackThreadArgs xpackArgs;
	//# Initialize
	cropArgs.cropThreadDone = &cropThreadDone;

	xcustomArgs.xcustomThreadDone = &xcustomThreadDone;
	xpackArgs.xpackThreadDone = &xpackThreadDone;


#if ENABLE_SCHEDULER
	//# Scheduler Entry Point ################################################
	while(1)
	{
#if ENABLE_CONSOLE_TEXT
		std::cout << "[DEBUG] while(1)" << std::endl;
#endif



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

#if LAYERWISE_PERFORMANCE
				hwQueue[ImgId][whichConv].startclk = sds_clock_counter();
#endif

					ConvolutionForward(
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].wts_ptrs[0], (CHAR_TYPE*)hwQueue[ImgId][whichConv].wts_ptrs[1],
#if (KER_PROC==16 || (PORT_BITWIDTH_64BIT==1 && KER_PROC==8))
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].wts_ptrs[2], (CHAR_TYPE*)hwQueue[ImgId][whichConv].wts_ptrs[3],
#endif
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].out_ptrs[0],
#if !SINGLE_IO_PORT
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].out_ptrs[1],
#endif
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].in_ptrs[0],
#if !SINGLE_IO_PORT
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].in_ptrs[1],
#endif
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].in_ptrs[2],
							(SHORT_TYPE*)hwQueue[ImgId][whichConv].bias_ptr,
#if !DISABLE_BN
							//# New args added
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].in_ptrs[3], (CHAR_TYPE*)hwQueue[ImgId][whichConv].in_ptrs[4],
#endif

#if !SINGLE_IO_PORT
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].out_ptrs[2],
#else
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].out_ptrs[1],
#endif

#if !SINGLE_IO_PORT
							(CHAR_TYPE*)hwQueue[ImgId][whichConv].out_ptrs[3],
#endif
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
#if LAYERWISE_PERFORMANCE
				hwQueue[ImgId][whichPool].startclk = sds_clock_counter();
#endif
					
					//# Call Pool wrapper
					PoolForward(
							(SHORT_TYPE*)hwQueue[ImgId][whichPool].in_ptrs[0], (SHORT_TYPE*)hwQueue[ImgId][whichPool].out_ptrs[0],
							(SHORT_TYPE*)hwQueue[ImgId][whichPool].in_ptrs[1], (SHORT_TYPE*)hwQueue[ImgId][whichPool].out_ptrs[1],
							(CHAR_TYPE*)hwQueue[ImgId][whichPool].wts_ptrs[0],
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
#if LAYERWISE_PERFORMANCE
				hwQueue[ImgId][whichFc].startclk = sds_clock_counter();
#endif

#if 0
					//# Call FC wrapper
					FcForward(
							(CHAR_TYPE*)hwQueue[ImgId][whichFc].wts_ptrs[0],
							(CHAR_TYPE*)hwQueue[ImgId][whichFc].wts_ptrs[1],
							(SHORT_TYPE *)hwQueue[ImgId][whichFc].in_ptrs[0],
							(SHORT_TYPE *)hwQueue[ImgId][whichFc].in_ptrs[1],
							(SHORT_TYPE *)hwQueue[ImgId][whichFc].in_ptrs[2],
							(SHORT_TYPE *)hwQueue[ImgId][whichFc].bias_ptr,
							(SHORT_TYPE *)hwQueue[ImgId][whichFc].out_ptrs[0],
							(INT_TYPE*)hwQueue[ImgId][whichFc].params);
#endif


					//# Call FC wrapper
					SwFcForward(
							(IO_DATA_TYPE*)hwQueue[0][whichFc].in_ptrs[0],
							(IO_DATA_TYPE*)hwQueue[0][whichFc].in_ptrs[1],
							(SW_FC_DATA_TYPE *)hwQueue[0][whichFc].in_ptrs[2],
							(SW_FC_DATA_TYPE *)hwQueue[0][whichFc].wts_ptrs[0],
							(SW_FC_DATA_TYPE *)hwQueue[0][whichFc].bias_ptr,
							(SW_FC_DATA_TYPE *)hwQueue[0][whichFc].out_ptrs[0],
							(INT_TYPE *)hwQueue[0][whichFc].params
					);

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
#if LAYERWISE_PERFORMANCE
				hwQueue[ImgId][whichDeconv].startclk = sds_clock_counter();
#endif
					
					DeconvForward(
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

#if LAYERWISE_PERFORMANCE
				hwQueue[ImgId][whichNorm].startclk = sds_clock_counter();
#endif

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
#if LAYERWISE_PERFORMANCE
				hwQueue[ImgId][whichPermute].startclk = sds_clock_counter();
#endif
					
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
#if LAYERWISE_PERFORMANCE
				hwQueue[ImgId][whichSoftmax].startclk = sds_clock_counter();
#endif					
					//# Call SoftMax wrapper
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
#if LAYERWISE_PERFORMANCE
				hwQueue[ImgId][whichNms].startclk = sds_clock_counter();
#endif	
				
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
#if LAYERWISE_PERFORMANCE
				hwQueue[ImgId][whichCrop].startclk = sds_clock_counter();
#endif					
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

#if NEEDED_XCUSTOM
		if((xcustomInUse == false) && txCustomLayers)
		{
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] xcustomInUse == false " << std::endl;
#endif
			for(ImgId = 0; ImgId < NUM_IMG; ++ImgId)
			{
				allxCustomDone = (xcustomCnt[ImgId] == txCustomLayers) ? true : false;
				//# Get the Custom layer ID
				uint16_t whichxcustom = xcustomSeq[xcustomCnt[ImgId]];
				//# Check dependencies are satisfied or not
				if(!allxCustomDone && (chkDeps(layerDone[ImgId], hwQueue[ImgId][whichxcustom].previous)))
				{
					//cout<<"exec "<<"ImgId "<<ImgId<<"whichxcustom "<<whichxcustom<<endl;
					//# Call Custom layer
					//std::cout << "\n\n[DEBUG] Calling Thread : Custom" << std::endl;
#if LAYERWISE_PERFORMANCE
				hwQueue[ImgId][whichxcustom].startclk = sds_clock_counter();
#endif

					xcustomArgs.Layer = &hwQueue[ImgId][whichxcustom];

					//xcustomRoutine(&xcustomArgs);
					pthread_create(&xcustomThread, NULL, xcustomRoutine, &xcustomArgs);
					//std::cout << "\n\n[DEBUG] Calling Thread : Custom : Done" << std::endl;
					xcustomImgId = ImgId;
					xcustomInUse = true;
					break;
				}
			}
		}
#endif//NEEDED_XCUSTOM

#if NEEDED_XPACK
		if((xpackInUse == false) && txPackLayers)
		{
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] xpackInUse == false " << std::endl;
#endif
			for(ImgId = 0; ImgId < NUM_IMG; ++ImgId)
			{
				allxPackDone = (xpackCnt[ImgId] == txPackLayers) ? true : false;
				//# Get the Pack layer ID
				uint16_t whichxpack = xpackSeq[xpackCnt[ImgId]];
				//# Check dependencies are satisfied or not
				if(!allxPackDone && (chkDeps(layerDone[ImgId], hwQueue[ImgId][whichxpack].previous)))
				{
#if LAYERWISE_PERFORMANCE
				hwQueue[ImgId][whichxpack].startclk = sds_clock_counter();
#endif
					
					//# Call Pack layer
					//std::cout << "\n\n[DEBUG] Calling Thread : Pack" << std::endl;
					xpackArgs.Layer = &hwQueue[ImgId][whichxpack];
					pthread_create(&xpackThread, NULL, xpackRoutine, &xpackArgs);
					//std::cout << "\n\n[DEBUG] Calling Thread : Pack : Done" << std::endl;
					xpackImgId = ImgId;
					xpackInUse = true;
					break;
				}
			}
		}
#endif//NEEDED_XPACK

#if NEEDED_ELTWADD
		if((eltwaddInUse == false) && tEltwaddLayers)
		{
#if ENABLE_CONSOLE_TEXT
			std::cout << "[DEBUG] eltwaddInUse == false " << std::endl;
#endif

			for(ImgId = 0; ImgId < NUM_IMG; ++ImgId)
			{
				allEltwaddDone = (eltwaddCnt[ImgId] == tEltwaddLayers) ? true : false;
				uint16_t whichEltwadd = eltwaddSeq[eltwaddCnt[ImgId]];
				if(!allEltwaddDone && (chkDeps(layerDone[ImgId], hwQueue[ImgId][whichEltwadd].previous)))
				{
#if LAYERWISE_PERFORMANCE
				hwQueue[ImgId][whichEltwadd].startclk = sds_clock_counter();
#endif
					//# Call Eltwadd wrapper
#if PTHREAD
					pthread_create(&eltwaddThread, NULL, eltwaddRoutine, (void *)&hwQueue[ImgId][whichEltwadd]);
#else
					eltwaddRoutine((void *)&hwQueue[ImgId][whichEltwadd]);
#endif
					eltwaddImgId = ImgId;
					eltwaddInUse = true;
#if ENABLE_CONSOLE_TEXT
					std::cout << "[DEBUG] eltwaddForward : " << eltwaddImgId << ", " << allEltwaddDone << std::endl;
#endif
					break;
				}
			}
		}
#endif  //NEEDED_ELTWADD

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
				//((int*)hwQueue[convImgId][whichConv].params)[11] = 1;

				int *params = (int*)hwQueue[0][whichConv].params;

				/* when group=2 */
				params[11] = 1;
				params[78] = params[120];
				params[79] = params[121];
				params[80] = params[122];
				params[81] = params[123];
				params[82] = params[124];
				params[102] = params[125];

#if ENABLE_CONSOLE_TEXT
				std::cout << "[DEBUG] convForward, Group : " << convImgId << std::endl;
#endif

				ConvolutionForward(
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].wts_ptrs[0], (CHAR_TYPE*)hwQueue[convImgId][whichConv].wts_ptrs[1],
#if (KER_PROC==16 || (PORT_BITWIDTH_64BIT==1 && KER_PROC==8))
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].wts_ptrs[2], (CHAR_TYPE*)hwQueue[convImgId][whichConv].wts_ptrs[3],
#endif
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].out_ptrs[0],
#if !SINGLE_IO_PORT
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].out_ptrs[1],
#endif
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].in_ptrs[0],
#if !SINGLE_IO_PORT
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].in_ptrs[1],
#endif
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].in_ptrs[2],
						(SHORT_TYPE*)hwQueue[convImgId][whichConv].bias_ptr,
#if !DISABLE_BN
						//# New args added
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].in_ptrs[3], (CHAR_TYPE*)hwQueue[convImgId][whichConv].in_ptrs[4],
#endif

#if !SINGLE_IO_PORT
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].out_ptrs[2],
#else
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].out_ptrs[1],
#endif

#if !SINGLE_IO_PORT
						(CHAR_TYPE*)hwQueue[convImgId][whichConv].out_ptrs[3],
#endif
						(INT_TYPE *)hwQueue[convImgId][whichConv].params
				);

#ifdef __SDSOC
				sds_wait(1);
#endif

				params[11] = 0;
				params[78] = 0;
				params[79] = 0;
				params[80] = 0;
				params[81] = 0;
				params[82] = 0;
				params[102] = params[119];
			}
			
#if LAYERWISE_PERFORMANCE
			hwQueue[convImgId][whichConv].endclk = sds_clock_counter();
#endif
			
#if ENABLE_ERROR_CHECKS || LAYERWISE_OUTPUT_WRITE
			if(convImgId == 0)
			{
				int convErr = errorCheck(hwQueue[convImgId][whichConv]);
				
#if !LAYERWISE_OUTPUT_WRITE
				if(convErr)
					std::cout << "\n[ERROR] Conv Layer : " << whichConv << " Image : " << convImgId << " Fail !" << std::endl;
				else
					std::cout << "\n[ERROR] Conv Layer : " << whichConv << " Image : " << convImgId << " Pass !" << std::endl;
#endif
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

#if LAYERWISE_PERFORMANCE
			hwQueue[poolImgId][poolSeq[poolCnt[poolImgId]]].endclk = sds_clock_counter();
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
			if(1)//sds_try_wait(3))
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
#if LAYERWISE_PERFORMANCE
			hwQueue[fcImgId][fcSeq[fcCnt[fcImgId]]].endclk = sds_clock_counter();
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
			if(sds_try_wait(3))
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
#if LAYERWISE_PERFORMANCE
			hwQueue[deconvImgId][deconvSeq[deconvCnt[deconvImgId]]].endclk = sds_clock_counter();
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

#if LAYERWISE_PERFORMANCE
			hwQueue[normImgId][whichNorm].endclk = sds_clock_counter();
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
#if LAYERWISE_PERFORMANCE
			hwQueue[permuteImgId][whichPermute].endclk = sds_clock_counter();
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
#if LAYERWISE_PERFORMANCE
			hwQueue[softmaxImgId][whichSoftmax].endclk = sds_clock_counter();
#endif
				layerDone[softmaxImgId][softmaxSeq[softmaxCnt[softmaxImgId]]] = true;
				softmaxCnt[softmaxImgId]++;
				softmaxInUse = false;
			}
		}
#endif//NEEDED_SOFTMAX

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
				hwQueue[nmsImgId][whichNms].layer_done[0] = 0;
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

#if LAYERWISE_PERFORMANCE
			hwQueue[nmsImgId][whichNms].endclk = sds_clock_counter();
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
#if LAYERWISE_PERFORMANCE
			hwQueue[cropImgId][cropSeq[cropCnt[cropImgId]]].endclk = sds_clock_counter();
#endif				
				layerDone[cropImgId][cropSeq[cropCnt[cropImgId]]] = true;
				cropCnt[cropImgId]++; 
				cropInUse = false;
			}
		}
#endif//NEEDED_CROP

#if NEEDED_XCUSTOM
		if(xcustomInUse == true)
		{
			//# Check for thread completion
			if(xcustomThreadDone)
			{
				//# Join thread
				int xcustomRet = pthread_join(xcustomThread, NULL);
				if(xcustomRet != 0)
				{ std::cerr << "\n[ERROR] xcustomThread Fail ! " << "Image : " << xcustomImgId << ", Layer ID : " << xcustomSeq[xcustomCnt[xcustomImgId]] << std::endl; }
#if ENABLE_ERROR_CHECKS
				if(cropImgId == 0){
					uint16_t whichxcustom = xcustomSeq[xcustomCnt[xcustomImgId]];
					int cropErr = errorCheck(hwQueue[xcustomImgId][whichxcustom]);
					if(cropErr)
						std::cout << "\n[ERROR] xcustom Layer : " << whichxcustom << " Image : " << xcustomImgId << " Fail !" << std::endl;
					else
						std::cout << "\n[ERROR] xcustom Layer : " << whichxcustom << " Image : " << xcustomImgId << " Pass !" << std::endl;
				}
#endif
				xcustomThreadDone = 0;
#if LAYERWISE_PERFORMANCE
			hwQueue[xcustomImgId][xcustomSeq[xcustomCnt[xcustomImgId]]].endclk = sds_clock_counter();
#endif				
				layerDone[xcustomImgId][xcustomSeq[xcustomCnt[xcustomImgId]]] = true;
				xcustomCnt[xcustomImgId]++;
				xcustomInUse = false;
			}
		}
#endif//NEEDED_XCUSTOM

#if NEEDED_XPACK
		if(xpackInUse == true)
		{
			//# Check for thread completion
			if(xpackThreadDone)
			{
				//# Join thread
				int xpackRet = pthread_join(xpackThread, NULL);
				if(xpackRet != 0)
				{ std::cerr << "\n[ERROR] xpackThread Fail ! " << "Image : " << xpackImgId << ", Layer ID : " << xpackSeq[xpackCnt[xpackImgId]] << std::endl; }
#if ENABLE_ERROR_CHECKS
				if(cropImgId == 0){
					uint16_t whichxpack = xpackSeq[xpackCnt[xpackImgId]];
					int cropErr = errorCheck(hwQueue[xpackImgId][whichxpack]);
					if(cropErr)
						std::cout << "\n[ERROR] Pack/Unpack Layer : " << whichxpack << " Image : " << xpackImgId << " Fail !" << std::endl;
					else
						std::cout << "\n[ERROR] Pack/Unpack Layer : " << whichxpack << " Image : " << xpackImgId << " Pass !" << std::endl;
				}
#endif
				xpackThreadDone = 0;
#if LAYERWISE_PERFORMANCE
			hwQueue[xpackImgId][xpackSeq[xpackCnt[xpackImgId]]].endclk = sds_clock_counter();
#endif				
				layerDone[xpackImgId][xpackSeq[xpackCnt[xpackImgId]]] = true;
				xpackCnt[xpackImgId]++;
				xpackInUse = false;
			}
		}
#endif//NEEDED_XPACK

#if NEEDED_ELTWADD
		if(eltwaddInUse == true)
		{
			uint16_t whichEltwadd = eltwaddSeq[eltwaddCnt[eltwaddImgId]];
			eltwaddThreadDone = hwQueue[eltwaddImgId][whichEltwadd].layer_done[0];

			//# Check for thread completion
			if(eltwaddThreadDone)
			{
#if PTHREAD
				int eltwaddRet = pthread_join(eltwaddThread, NULL);
#else
				int eltwaddRet = 0;
#endif

#if ENABLE_CONSOLE_TEXT
				if(eltwaddRet != 0)
				{ std::cerr << "\n[ERROR] eltwaddThread Fail ! " << "Image : " << eltwaddImgId << ", Layer ID : " << eltwaddSeq[eltwaddCnt[eltwaddImgId]] << std::endl; }
				else
				{ std::cout << "** eltwaddForward : Done" << std::endl; }
#endif
				hwQueue[eltwaddImgId][whichEltwadd].layer_done[0] = 0;
				eltwaddThreadDone = 0;

#if ENABLE_ERROR_CHECKS
				if(eltwaddImgId == 0){
					int eltwaddErr = errorCheck(hwQueue[eltwaddImgId][whichEltwadd]);
					if(eltwaddErr)
						std::cout << "\n[ERROR] Eltwadd Layer : " << whichEltwadd << " Image : " << eltwaddImgId << " Fail !" << std::endl;
					else
						std::cout << "\n[ERROR] Eltwadd Layer : " << whichEltwadd << " Image : " << eltwaddImgId << " Pass !" << std::endl;
				}
#endif
#if LAYERWISE_PERFORMANCE
			hwQueue[eltwaddImgId][whichEltwadd].endclk = sds_clock_counter();
#endif
				layerDone[eltwaddImgId][eltwaddSeq[eltwaddCnt[eltwaddImgId]]] = true;
				eltwaddCnt[eltwaddImgId]++;
				eltwaddInUse = false;
			}
		}
#endif//NEEDED_ELTWADD

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
			deconvCnt[0] = 0; normCnt[0] = 0; nmsCnt[0] = 0;
			permuteCnt[0] = 0; cropCnt[0] = 0; eltwaddCnt[0] = 0;

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
			deconvCnt[1] = 0; normCnt[1] = 0; nmsCnt[1] = 0;
			permuteCnt[1] = 0; cropCnt[1] = 0; eltwaddCnt[1] = 0;
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


#endif//ENABLE_SCHEDULER

#if OUTPUT_WRITE
	//# Output data write
	int status = outFileWrite(hwQueue[0][totalLayers-1], ImageListPath);

	if(status)
		std::cout << "\n[ERROR] Output file write failed " << std::endl;
	/*else
		std::cout << "[INFOx] File write End " << std::endl;
	 */
	//std::cout << std::endl;
#endif

	//# TODO: Can be removed based on need
#if 0//ENABLE_ERROR_CHECKS1
	for(int img = 0; img < NUM_IMG; ++img)
	{
		uint16_t whichLayer = totalLayers-1;
		int nmsErr = errorCheck(hwQueue[img][whichLayer]);
		if(nmsErr)
			std::cout << "\n[ERROR] Final Layer : " << whichLayer << " Image : " << img << " Fail !" << std::endl;
		else
			std::cout << "\n[ERROR] Final Layer : " << whichLayer << " Image : " << img << " Pass !" << std::endl;

		std::cout << std::endl;
	}

#endif

	//output[0]	= hwQueue[0][totalLayers-1].out_ptrs[0];

	return;
}
