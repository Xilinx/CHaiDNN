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
#include <iostream>
#include <math.h>               // For math ops like sqrt, exp etc.
#include <string.h>
#include <algorithm>            // For sorting

#include "../include/hw_settings.h"

using namespace std;

// Structure to pack serialNo and score of a box together
// Helps to simplify the sort function
typedef struct idScore_
{
	int id;                     // index of the item
	float score;                // score of the item
} idScore;

// Structure to pack serialNo, label and score of a box together
typedef struct idLabelScore_
{
	int id;                     // id and score
	int label;                  // class label
	float score;                // score of the box
} idLabelScore;

// Structure for a box with topleft, bottomright coords
typedef struct box_
{
	float xmin;
	float ymin;
	float xmax;
	float ymax;
} Box;

// Structure to pack serialNo, label, score and box coords
// TODO : Should we add some pad to match size to cache size ?
typedef struct fullBox_
{
	int id;
	int label;
	float score;
	Box box;
} fullBox;

int idLabelScoreDescend(const void* a, const void* b)
{
	float valA = ((idLabelScore*)a)->score;
	float valB = ((idLabelScore*)b)->score;

	if(valA < valB)             return 1;
	else if(valA == valB)       return 0;
	else                        return -1;
}

//int box_cnt = 0;
int fullBoxDescend(const void* a, const void* b)
{
	float valA = ((fullBox*)a)->score;
	float valB = ((fullBox*)b)->score;
	//fprintf(stderr, "box_cnt:%d\n", box_cnt);
	//box_cnt++;
	if(valA < valB)             return 1;
	else if(valA == valB)       return 0;
	else                        return -1;
}


// DecodeBoxes function
// box - float array - set of prior boxes, shape = nboxes x 4
// loc - float array - set of trained offsets, shape = nboxes x 4
// var - float array - variance for each coordinate, shape = 4
// nboxes - number of boxes in the box array
// filteredBoxes - thresholded boxes from box array, shape= nFilteredBoxes
// nFilteredBoxes - number of filtered boxes
// decodedBoxes - Output array
void decodeBoxes(float* box, IO_DATA_TYPE* loc, float* var, int nboxes,
		idLabelScore* filteredBoxes, int nFilteredBoxes, fullBox* decodedBoxes, int batch_size, int batch_id, int ip_fbits, float sf_in, int quant_scheme_flag)
{
	//float var0 = var[0], var1 = var[1];
	//float var2 = var[2], var3 = var[3];

	float prior_xmin, prior_ymin, prior_xmax, prior_ymax;
	float bbox_xmin, bbox_ymin, bbox_xmax, bbox_ymax;
	float prior_width, prior_height, prior_center_x, prior_center_y;

	float decode_bbox_center_x, decode_bbox_center_y;
	float decode_bbox_width, decode_bbox_height;

	fullBox decodedBox;

	batch_size = XBATCH_SIZE;

	// TODO : Abid K : Apply Vijay's technique to avoid some computes
	for(int i=0; i<nFilteredBoxes; i++)
	{
		float var0 = var[i*4+0];
		float var1 = var[i*4+1];
		float var2 = var[i*4+2];
		float var3 = var[i*4+3];
		idLabelScore ID = filteredBoxes[i];
		int id = ID.id;
		//        cout << "ID = " << id << endl;
		prior_xmin = box[id*4];
		prior_ymin = box[id*4+1];
		prior_xmax = box[id*4+2];
		prior_ymax = box[id*4+3];


#if 0
		bbox_xmin = loc[(id*4+0)*batch_size+batch_id];
		bbox_ymin = loc[(id*4+1)*batch_size+batch_id];
		bbox_xmax = loc[(id*4+2)*batch_size+batch_id];
		bbox_ymax = loc[(id*4+3)*batch_size+batch_id];
#endif

#if 1
		IO_DATA_TYPE fxval;
		IO_DATA_TYPE1 fval;

		if(quant_scheme_flag == 1)
		{
			fxval = loc[(id*4+0)*batch_size+batch_id];
			fval = ((IO_DATA_TYPE1)(fxval*sf_in));
			bbox_xmin = fval;//loc[id*4];
			fxval = loc[(id*4+1)*batch_size+batch_id];
			fval = ((IO_DATA_TYPE1)(fxval*sf_in));
			bbox_ymin = fval;//loc[id*4+1];
			fxval = loc[(id*4+2)*batch_size+batch_id];
			fval = ((IO_DATA_TYPE1)(fxval*sf_in));
			bbox_xmax = fval;//loc[id*4+2];
			fxval = loc[(id*4+3)*batch_size+batch_id];
			fval = ((IO_DATA_TYPE1)(fxval*sf_in));
			bbox_ymax = fval;//loc[id*4+3];
		}
		else
		{
			fxval = loc[(id*4+0)*batch_size+batch_id];
			fval = ((IO_DATA_TYPE1)(fxval))/(1 << ip_fbits);
			bbox_xmin = fval;//loc[id*4];
			fxval = loc[(id*4+1)*batch_size+batch_id];
			fval = ((IO_DATA_TYPE1)(fxval))/(1 << ip_fbits);
			bbox_ymin = fval;//loc[id*4+1];
			fxval = loc[(id*4+2)*batch_size+batch_id];
			fval = ((IO_DATA_TYPE1)(fxval))/(1 << ip_fbits);
			bbox_xmax = fval;//loc[id*4+2];
			fxval = loc[(id*4+3)*batch_size+batch_id];
			fval = ((IO_DATA_TYPE1)(fxval))/(1 << ip_fbits);
			bbox_ymax = fval;//loc[id*4+3];
		}
#endif

		prior_width = prior_xmax - prior_xmin;
		prior_height = prior_ymax - prior_ymin;
		prior_center_x = (prior_xmin + prior_xmax) / 2.;
		prior_center_y = (prior_ymin + prior_ymax) / 2.;

		// Apply the variance
		decode_bbox_center_x = var0 * bbox_xmin * prior_width + prior_center_x;
		decode_bbox_center_y = var1 * bbox_ymin * prior_height + prior_center_y;
		decode_bbox_width    = exp(var2 * bbox_xmax) * prior_width;
		decode_bbox_height   = exp(var3 * bbox_ymax) * prior_height;

		// Update the box
		decodedBox.id       = ID.id;
		decodedBox.label    = ID.label;
		decodedBox.score    = ID.score;
		decodedBox.box.xmin = (decode_bbox_center_x - decode_bbox_width / 2.);
		decodedBox.box.ymin = (decode_bbox_center_y - decode_bbox_height / 2.);
		decodedBox.box.xmax = (decode_bbox_center_x + decode_bbox_width / 2.);
		decodedBox.box.ymax = (decode_bbox_center_y + decode_bbox_height / 2.);

		//        cout << "box : " << box[id*4] << " " << box[id*4+1] << " "
		//                        << box[id*4+2] << " " << box[id*4+3] << endl;
		//        cout << "loc : " << loc[id*4] << " " << loc[id*4+1] << " "
		//                        << loc[id*4+2] << " " << loc[id*4+3] << endl;
		//        cout << "decode: " << decodedBox.box.xmin << " " << decodedBox.box.ymin << " "
		//                        << decodedBox.box.xmax << " " << decodedBox.box.ymax << endl;

		// Put it in the list
		decodedBoxes[i] = decodedBox;
	}

}


float intersectArea(const fullBox& bbox1, const fullBox& bbox2)
{
	// Return [0, 0, 0, 0] if there is no intersection.
	if ((bbox2.box.xmin > bbox1.box.xmax) || (bbox2.box.xmax < bbox1.box.xmin) ||
			(bbox2.box.ymin > bbox1.box.ymax) || (bbox2.box.ymax < bbox1.box.ymin))
	{
		// cout << "Inside If part " << endl;
		return 0;
	}
	else
	{
		float xmin = (std::max(bbox1.box.xmin, bbox2.box.xmin));
		float ymin = (std::max(bbox1.box.ymin, bbox2.box.ymin));
		float xmax = (std::min(bbox1.box.xmax, bbox2.box.xmax));
		float ymax = (std::min(bbox1.box.ymax, bbox2.box.ymax));
		// cout << xmin << " " << ymin << " " << xmax << " " << ymax << endl;
		return (xmax-xmin) * (ymax-ymin);
	}
}

float BBoxSize(const fullBox& bbox)
{
	// If bbox is invalid (e.g. xmax < xmin or ymax < ymin), return 0.
	if (bbox.box.xmax < bbox.box.xmin || bbox.box.ymax < bbox.box.ymin)
	{
		return 0.0;
	}
	else
	{
		const float width  = bbox.box.xmax - bbox.box.xmin;
		const float height = bbox.box.ymax - bbox.box.ymin;
		return width * height;
	}
}

float JaccardOverlap(const fullBox& bbox1, const fullBox& bbox2)
{
	float intersect_size = intersectArea(bbox1, bbox2);
	if (intersect_size > 0.0)
	{
		float bbox1_size = BBoxSize(bbox1);
		float bbox2_size = BBoxSize(bbox2);
		return intersect_size / (bbox1_size + bbox2_size - intersect_size);
	}
	else
	{
		return 0.;
	}
}



// NMS Layer
// TODO : Abid K : Instead of score_threshold, directly use final_threshold. Reduce the boxes from 169 -> 2
// TODO : Abid K : Instead of considering all the classes for each rectangle, consider only highest class.
// score : float array : all the softmax scores with shape (nboxes x nclasses)
// nboxes : Total number of boxes
// nclasses : total number of classes (21 for Pascal VOC)
// score_threshold : float value : All scores less than this value is discarded
// nms_overlap : float value : Any box with overlap more than this with another reference box is merged to ref box
// nms_top_k : integer : Keep only nms_top_k number of highest scoring boxes for each class
// keep_top_k : integer : Keep only keep_top_k number of highest scoring boxes at the end
void NMSWrapper(int* nms_finalboxcount, int *nms_id, int *nms_label, float *nms_box, float *nms_score, float* conf, float* pbox, IO_DATA_TYPE* loc, float* var, int *scalar_nms_args )
{

	int nboxes     = scalar_nms_args[0];
	int nclasses   = scalar_nms_args[1];
	int nms_top_k  = scalar_nms_args[2];
	int keep_top_k = scalar_nms_args[3];
	int batch_size = XBATCH_SIZE;//scalar_nms_args[6];
	int ip_fbits   = scalar_nms_args[7];//8;

	//# Read scale factor for input
	//float *sf_in_ptr 	  = (float*)(&scalar_nms_args[9]);
	//float sf_in = sf_in_ptr[0];

	int quant_scheme_flag = scalar_nms_args[10];

	//# Read scale factor for input
	float *score_threshold_ptr 	  = (float*)(&scalar_nms_args[11]);
	float score_threshold = score_threshold_ptr[0];

	float *nms_overlap_ptr 	  = (float*)(&scalar_nms_args[12]);
	float nms_overlap = nms_overlap_ptr[0];

	float *sf_in_ptr 	  = (float*)(&scalar_nms_args[13]);
	float sf_in = sf_in_ptr[0];


	int batch_size_one_enable =  scalar_nms_args[8];
	int batch_size_loopcnt;
	if(batch_size_one_enable)
	{
		batch_size_loopcnt = 1;
	}
	else
	{
		batch_size_loopcnt = batch_size;
	}

#if 1
	//float score_threshold = 0.01;//var[4];
	//float nms_overlap = 0.45;//var[5];

	//nboxes = nboxes+1;

	//fullBox* finalBoxes = (fullBox*) malloc (nboxes * sizeof(fullBox));

	// Create a vector for selected Boxes along with their id, label, score
	// Here the ID corresponds to index in the overall boxes
	// TODO : Abid K : I am assuming for each box, maximum 3 scores will be filtered so that
	// all selected boxes will fit in the final array. Make it more reasonable.
	// TODO : Abid K : Instead of keeping separate arrays for final selected boxes and class-wise
	// temporary buffers for boxes, put everything in the final array, and adjust sort accordingly
	// It saves some memory, may be some performance also
	//idLabelScore* filteredBoxes = (idLabelScore*) malloc (nboxes * nclasses * sizeof(idLabelScore));
	// Create a tmp array for keeping classwise boxes to sort it later
	// ID assigned to each box is index in the pbox array
	idLabelScore* tmpIDLabelScore[XBATCH_SIZE];
	fullBox* decodedBoxes[XBATCH_SIZE];
	fullBox* finalBoxes_cid[XBATCH_SIZE];
	fullBox* finalBoxes[XBATCH_SIZE];
	int* finalBoxesID[XBATCH_SIZE];
	idLabelScore* filteredBoxes[XBATCH_SIZE];
	int finalCount[XBATCH_SIZE];

	for(int i=0; i<XBATCH_SIZE; i++)
	{
		tmpIDLabelScore[i] = (idLabelScore*) malloc (nboxes * sizeof(idLabelScore));
		decodedBoxes[i] = (fullBox*) malloc (nboxes * sizeof(fullBox));
		finalBoxes_cid[i] = (fullBox*) malloc (nboxes * sizeof(fullBox));
		finalBoxesID[i] = (int*) malloc (nboxes * sizeof(int));
		filteredBoxes[i] = (idLabelScore*) malloc (nboxes * nclasses * sizeof(idLabelScore));
		finalBoxes[i] = (fullBox*) malloc (nboxes * sizeof(fullBox));
		finalCount[i] = 0;
	}


	int nFilteredBoxes = 0;
	int nms_offset = 0;

	//for(int batch_id = 0; batch_id < XBATCH_SIZE; batch_id++)
	//{
	// Consider each class seperately
	// TODO : Abid K : Skip the background class (classID = 0), so start with c=1
	for (int cid=1; cid<nclasses; cid++)
	{
		// cout << "#--------------------------------------#" << endl;
		// cout << "#---------  Class " << c << "   ----------#" << endl;
		// cout << "#--------------------------------------#" << endl;
		int cwCount[XBATCH_SIZE];// = {0};//,0};
		for(int i=0; i<batch_size_loopcnt; i++)
		{
			cwCount[i] = 0;
		}

		int cwCount2 = 0;
		for (int bid=0; bid<nboxes; bid++)
		{
			int scoreIndex = bid*nclasses*batch_size + cid*batch_size;// + batch_id;
			for(int batch_id = 0; batch_id < batch_size_loopcnt; batch_id++)
			{
				// Filter the boxes based on the score_threshold
				//int scoreIndex = i*nclasses + c;

				//cout << "DEBUG : " << i << " " << scoreIndex << " " << c << " " << score[scoreIndex] << endl;

				if (conf[scoreIndex+batch_id] > score_threshold)
				{
					idLabelScore tmp = {bid, cid, conf[scoreIndex+batch_id]};
					// cout << "Before sort : " << i << " " << scoreIndex+1 << " " << c << " " << score[scoreIndex] << endl;
					tmpIDLabelScore[batch_id][cwCount[batch_id]] = tmp;
					cwCount[batch_id]++;
				}
			}

		}

		// Now we have all the boxes from a particular class, Let's sort it
		// TODO : Abid K : std::sort() beats qsort() in performance. Why not try it and see?
		for(int batch_id = 0; batch_id < batch_size_loopcnt; batch_id++)
		{
			qsort(tmpIDLabelScore[batch_id], cwCount[batch_id], sizeof(idLabelScore), idLabelScoreDescend);
			//sort(tmpIDLabelScore[batch_id], tmpIDLabelScore[batch_id]+cwCount[batch_id], idLabelScoreDescend);


			// for (int ww = 0; ww<min(5, cwCount); ww++)
			//     cout << tmpIDLabelScore[ww].score << endl;

			// Now copy only highest scoring boxes depending on value of nms_top_k
			// TODO : Abid K : may be we can avoid this copy, sort directly on tmpIDLabelScore
			int reqBoxes = nms_top_k > 0 ? min(nms_top_k, cwCount[batch_id]) : cwCount[batch_id];
			for (int j=0; j< reqBoxes; j++)
			{
				filteredBoxes[batch_id][j] = tmpIDLabelScore[batch_id][j];
			}
			nFilteredBoxes = reqBoxes;

			// Now we decode only the required boxes
			decodeBoxes(pbox, loc, var, nboxes, filteredBoxes[batch_id], reqBoxes, decodedBoxes[batch_id], batch_size_loopcnt, batch_id, ip_fbits, sf_in, quant_scheme_flag);

			int nDecodedBoxes = reqBoxes;
			// for (int ww = 0; ww<reqBoxes; ww++)
			// {
			//     cout
			//                 << decodedBoxes[ww].box.xmin    << " "
			//                 << decodedBoxes[ww].box.ymin    << " "
			//                 << decodedBoxes[ww].box.xmax    << " "
			//                 << decodedBoxes[ww].box.ymax    << " "
			//                 << decodedBoxes[ww].score       << " "
			//                 << decodedBoxes[ww].id          << " "
			//                 << endl;
			// }

			// Now is the time to do actual NMS. For that sort the decodedBoxes in terms of score
			// qsort(decodedBoxes, nFilteredBoxes, sizeof(fullBox), fullBoxDescend);

			// Non-maximum Suppression starts here
			int scanID = 0;
			// Just save the index of boxes w.ref.t decodedBoxes
			int cwIDcount = 0;
			while (scanID < nDecodedBoxes)
			{
				fullBox tmpBox = decodedBoxes[batch_id][scanID];
				bool keep = true;
				for (int k = 0; k < cwIDcount; ++k)
				{
					if (keep)
					{
						// Compare this box with all the boxes in output list
						// Only if it doesn't overlap with all other boxes in output, keep it.
						int refBoxID = finalBoxesID[batch_id][k];
						fullBox refBox = decodedBoxes[batch_id][refBoxID];
						// cout << "tmpBox : " << tmpBox.id << " " <<  tmpBox.score << " " << tmpBox.box.xmin << " "
						//                     << tmpBox.box.ymin << " "
						//                     << tmpBox.box.xmax << " "
						//                     << tmpBox.box.ymax << " " << endl;
						// cout << "refBox : " << refBox.id << " " << refBox.score << " " << refBox.box.xmin << " "
						//                     << refBox.box.ymin << " "
						//                     << refBox.box.xmax << " "
						//                     << refBox.box.ymax << " " << endl;
						float overlap = JaccardOverlap(tmpBox, decodedBoxes[batch_id][refBoxID]);
						// cout << "Jaccard Overlap : " << overlap << endl;
						keep = overlap <= nms_overlap;
					}
					else
					{
						break;
					}
				}
				if (keep)
				{
					if(finalCount[batch_id] < nboxes)
					{
						finalBoxesID[batch_id][cwIDcount++] = scanID;
						finalBoxes_cid[batch_id][finalCount[batch_id]++] = tmpBox;
					}
					else
					{
						break;
					}
				}
				scanID++;
			}
			//cout << "class : " << c << " count after NMS: " << finalCount << endl;
		}  //for(int batch_id = 0; batch_id < batch_size_loopcnt; batch_id++)

	} //for (int c=1; c<nclasses; c++) // Got all the required box IDs

#if 1
	for(int batch_id = 0; batch_id < batch_size_loopcnt; batch_id++)
	{
		// Do a final sort to get top scoring boxes in the front
		//qsort(finalBoxes_, finalCount, sizeof(fullBox), fullBoxDescend);
		qsort(finalBoxes_cid[batch_id], finalCount[batch_id], sizeof(fullBox), fullBoxDescend);
		//sort(finalBoxes_cid[batch_id], finalBoxes_cid[batch_id]+finalCount[batch_id], fullBoxDescend);

		if (keep_top_k > 0 && keep_top_k <= finalCount[batch_id])
		{
			//qsort(finalBoxes_cid[batch_id], finalCount[batch_id], sizeof(fullBox), fullBoxDescend);
			finalCount[batch_id] = keep_top_k;
		}

		finalBoxes[batch_id] = finalBoxes_cid[batch_id];
		*nms_finalboxcount = finalCount[batch_id];

#if 0
		//int *nms_id, int *nms_label, float *box_xyval, f hgloat *score, int* finalCount, float* score,
		nms_id[0] = finalCount;
		for(int i=0;i<finalCount;i++)
		{
			nms_id[i+1]    = finalBoxes[i].id;
			nms_label[i]   = finalBoxes[i].label;

			nms_score[i]   =  finalBoxes[i].score;

			nms_box[4*i+0] = (finalBoxes[i].box.xmin);
			nms_box[4*i+1] = (finalBoxes[i].box.ymin);
			nms_box[4*i+2] = (finalBoxes[i].box.xmax);
			nms_box[4*i+3] = (finalBoxes[i].box.ymax);

		}
#endif

#if 0
		//int *nms_id, int *nms_label, float *box_xyval, f hgloat *score, int* finalCount, float* score,
		nms_id[0] = finalCount;
		for(int i=0;i<finalCount;i++)
		{
			nms_id[i+1]    = finalBoxes[i].id;
			nms_label[i]   = finalBoxes[i].label;

			nms_box[5*i+0] =  finalBoxes[i].score;

			nms_box[5*i+1] = (finalBoxes[i].box.xmin);
			nms_box[5*i+2] = (finalBoxes[i].box.ymin);
			nms_box[5*i+3] = (finalBoxes[i].box.xmax);
			nms_box[5*i+4] = (finalBoxes[i].box.ymax);

		}

#endif

#if 0
		for(int i=0;i<finalCount;i++)
		{
			nms_score[i*7]    = (float)finalBoxes[i].id;
			nms_score[i*7+1]  = (float)finalBoxes[i].label;

			nms_score[i*7+2]  =  finalBoxes[i].score;

			nms_score[i*7+3]  = finalBoxes[i].box.xmin;
			nms_score[i*7+4]  = finalBoxes[i].box.ymin;
			nms_score[i*7+5]  = finalBoxes[i].box.xmax;
			nms_score[i*7+6]  = finalBoxes[i].box.ymax;
		}
#endif

#if 0
		//int *nms_id, int *nms_label, float *box_xyval, f hgloat *score, int* finalCount, float* score,
		nms_id[0] = finalCount;

		for(int i=0;i<finalCount;i++)
		{
			int nms_score_index = i*7*batch_size + batch_id;
			nms_score[i*7*batch_size+0*batch_size+batch_id]  = 0;//(float)finalBoxes[i].id;
			nms_score[i*7*batch_size+1*batch_size+batch_id]  = (float)finalBoxes[i].label;
			nms_score[i*7*batch_size+2*batch_size+batch_id]  =  finalBoxes[i].score;

			nms_score[i*7*batch_size+3*batch_size+batch_id]  = finalBoxes[i].box.xmin;
			nms_score[i*7*batch_size+4*batch_size+batch_id]  = finalBoxes[i].box.ymin;
			nms_score[i*7*batch_size+5*batch_size+batch_id]  = finalBoxes[i].box.xmax;
			nms_score[i*7*batch_size+6*batch_size+batch_id]  = finalBoxes[i].box.ymax;
		}
#endif

#if 1
		//int *nms_id, int *nms_label, float *box_xyval, f hgloat *score, int* finalCount, float* score,
		nms_score[nms_offset+0] = (float)finalCount[batch_id];
		nms_offset += 1;
		for(int i=0;i<finalCount[batch_id];i++)
		{
			nms_score[nms_offset+i*7+0]  = 0;//(float)finalBoxes[i].id;
			nms_score[nms_offset+i*7+1]  = (float)finalBoxes[batch_id][i].label;
			nms_score[nms_offset+i*7+2]  =  finalBoxes[batch_id][i].score;

			nms_score[nms_offset+i*7+3]  = finalBoxes[batch_id][i].box.xmin;
			nms_score[nms_offset+i*7+4]  = finalBoxes[batch_id][i].box.ymin;
			nms_score[nms_offset+i*7+5]  = finalBoxes[batch_id][i].box.xmax;
			nms_score[nms_offset+i*7+6]  = finalBoxes[batch_id][i].box.ymax;
		}
		nms_offset += (finalCount[batch_id]* 7);
#endif

	} //for(int batch_id = 0; batch_id < batch_size_loopcnt; batch_id++)
#endif

#if 1
	for(int i=0; i<batch_size_loopcnt; i++)
	{
		free(tmpIDLabelScore[i]);
		free(decodedBoxes[i]);
		free(finalBoxes_cid[i]);
		free(finalBoxesID[i]);
		free(filteredBoxes[i]);
		//free(finalBoxes[i]);
	}
/*
	free(finalBoxes);
	free(filteredBoxes);
	free(tmpIDLabelScore);
	free(decodedBoxes);
	//free(finalBoxes_);
	free(finalBoxesID);
*/
#endif
#endif

}
