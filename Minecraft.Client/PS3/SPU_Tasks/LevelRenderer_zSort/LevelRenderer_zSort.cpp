/* SCE CONFIDENTIAL
PlayStation(R)3 Programmer Tool Runtime Library 430.001
* Copyright (C) 2007 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

/* common headers */
#include <stdint.h>
#include <stdlib.h>
#include <alloca.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>
#include <cell/dma.h>
#include <cell/spurs/job_queue.h>

#include "LevelRenderer_zSort.h"
#include "../Common/DmaData.h"
#include <vectormath/c/vectormath_aos_v.h>



// #define SPU_HEAPSIZE (128*1024)
// #define SPU_STACKSIZE (16*1024)
// 
// CELL_SPU_LS_PARAM(128*1024, 16*1024);	// can't use #defines here as it seems to  create an asm instruction


static const bool sc_verbose = false;

CellSpursJobContext2* g_pSpursJobContext;

struct ListData
{
	int index;
	float zDepth;
};

int zGreater(const void * a, const void * b)
{
	if ( ((ListData*)a)->zDepth <  ((ListData*)b)->zDepth ) return -1;
	if ( ((ListData*)a)->zDepth == ((ListData*)b)->zDepth ) return 0;
	if ( ((ListData*)a)->zDepth >  ((ListData*)b)->zDepth ) return 1;
}

int zLess(const void * a, const void * b)
{
	if ( ((ListData*)a)->zDepth >  ((ListData*)b)->zDepth ) return -1;
	if ( ((ListData*)a)->zDepth == ((ListData*)b)->zDepth ) return 0;
	if ( ((ListData*)a)->zDepth <  ((ListData*)b)->zDepth ) return 1;
}

void zSort(LevelRenderer_cull_DataIn* dataIn, LevelRenderer_cull_DataIn* eaDataIn)
{
	int num = dataIn->numToRender_layer0;
	int* listArray_layer0 = (int*)alloca(DmaData_SPU::roundUpDMASize(num * sizeof(int)));
	float* zDepth_layer0 = (float*)alloca(DmaData_SPU::roundUpDMASize(num * sizeof(float)));
	ListData* list = (ListData*)alloca(DmaData_SPU::roundUpDMASize(num * sizeof(ListData)));
	DmaData_SPU::getAndWait(listArray_layer0, (uintptr_t)dataIn->listArray_layer0, DmaData_SPU::roundUpDMASize(sizeof(int)*num));
	DmaData_SPU::getAndWait(zDepth_layer0, (uintptr_t)dataIn->zDepth_layer0, DmaData_SPU::roundUpDMASize(sizeof(int)*num));

	for(int i=0;i<num;i++)
	{
		list[i].index = listArray_layer0[i];
		list[i].zDepth = zDepth_layer0[i];
	}

// 	spu_print("qsorting %d values\n", num);
	std::qsort(list, num, sizeof(ListData), zLess);

	for(int i=0;i<num;i++)
	{
		listArray_layer0[i] = list[i].index;
		zDepth_layer0[i] = list[i].zDepth;
// 		spu_print("i %d : %f\n", i, list[i].zDepth);
	}
	DmaData_SPU::putAndWait(listArray_layer0, (uintptr_t)dataIn->listArray_layer0, DmaData_SPU::roundUpDMASize(sizeof(int)*num));
	DmaData_SPU::putAndWait(zDepth_layer0, (uintptr_t)dataIn->zDepth_layer0, DmaData_SPU::roundUpDMASize(sizeof(float)*num));


}




void cellSpursJobQueueMain(CellSpursJobContext2 *pContext, CellSpursJob256 *pJob)
{
	// 	CellSpursTaskId idTask = cellSpursGetTaskId();
	unsigned int idSpu = cellSpursGetCurrentSpuId();

	if(sc_verbose)
		spu_print("LevelRenderer_cull [SPU#%u] start\n", idSpu);

	g_pSpursJobContext = pContext;
	uint32_t eaDataIn = pJob->workArea.userData[0];						
// 	uint32_t eaDataOut =pJob->workArea.userData[1];

	LevelRenderer_cull_DataIn dataIn;
	DmaData_SPU::getAndWait(&dataIn, eaDataIn, sizeof(LevelRenderer_cull_DataIn));


	zSort(&dataIn, (LevelRenderer_cull_DataIn*)eaDataIn);

	if(sc_verbose)
		spu_print("LevelRenderer_cull [SPU#%u] exit\n", idSpu);
}

