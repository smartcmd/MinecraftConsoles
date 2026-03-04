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

#include "Texture_blit.h"
#include "..\Common\DmaData.h"
#include <vectormath/c/vectormath_aos_v.h>



static const bool sc_verbose = false;

CellSpursJobContext2* g_pSpursJobContext;


void CopyPPUMemory(void* pSrc, void* pDst, int size)
{
	static const int bufferSize = 16384;
	char spuBuffer[bufferSize];
	int dataLeft = size;
	char* pSrcPos = (char*)pSrc;
	char* pDstPos = (char*)pDst;
	while(dataLeft > 0)
	{
		int sizeToDma = dataLeft;
		if(sizeToDma > bufferSize)
			sizeToDma = bufferSize;

		DmaData_SPU::getAndWait(spuBuffer, (uintptr_t)pSrcPos, sizeToDma);
		DmaData_SPU::putAndWait(spuBuffer, (uintptr_t)pDstPos, sizeToDma);
		pSrcPos += sizeToDma;
		pDstPos += sizeToDma;
		dataLeft -= sizeToDma;
	}
}







void blit(Texture_blit_DataIn& dataIn)
{
	int yy = dataIn.yy;
	int xx = dataIn.xx;
// 	int hh = dataIn.hh;
	int ww = dataIn.ww;
	int shh = dataIn.shh;
	int sww = dataIn.sww;
	bool rotated = dataIn.rotated;

	for (int srcY = 0; srcY < shh; srcY++)
	{
		int dstY = yy + srcY;
		int srcLine = srcY * sww * 4;
		int dstLine = dstY * ww * 4;

		if (rotated)
		{
			dstY = yy + (shh - srcY);
		}

		for (int srcX = 0; srcX < sww; srcX++)
		{
			int dstPos = dstLine + (srcX + xx) * 4;
			int srcPos = srcLine + srcX * 4;

			if (rotated)
			{
				dstPos = (xx + srcX * ww * 4) + dstY * 4;
			}

			uint32_t val = DmaData_SPU::getValue32((uintptr_t)&dataIn.pSrcData[srcPos]);
			DmaData_SPU::putValue32(val, (uintptr_t)&dataIn.pDstData[dstPos]);

// 			data[level]->put(dstPos + 0, srcBuffer->get(srcPos + 0));
// 			data[level]->put(dstPos + 1, srcBuffer->get(srcPos + 1));
// 			data[level]->put(dstPos + 2, srcBuffer->get(srcPos + 2));
// 			data[level]->put(dstPos + 3, srcBuffer->get(srcPos + 3));
		}
	}
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

	Texture_blit_DataIn dataIn;
	DmaData_SPU::getAndWait(&dataIn, eaDataIn, sizeof(Texture_blit_DataIn));

	if(sc_verbose)
		spu_print("Texture_blit [SPU#%u] \n", idSpu);

	blit(dataIn);


	if(sc_verbose)
		spu_print("Texture_blit [SPU#%u] exit\n", idSpu);
}

