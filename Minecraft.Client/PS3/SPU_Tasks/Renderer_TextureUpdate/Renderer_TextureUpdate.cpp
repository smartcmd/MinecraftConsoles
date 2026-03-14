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

#include "Renderer_TextureUpdate.h"
#include "../Common/DmaData.h"
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


void TextureDataUpdate(int xoffset, int yoffset, int width, int height, int pitch, void *pSrcData, void* pDstData, int level)
{
	int colourDepth = 4;
	char* pTexData = (char*)pDstData;
	char* pSrc = (char*) pSrcData;
	int srcOffset = 0;
	int dstOffset = 0;
	for(int i=0; i<height;i++)
	{
		CopyPPUMemory(&pSrc[srcOffset], &pTexData[dstOffset], pitch*colourDepth);
		srcOffset += width*colourDepth;
		dstOffset += pitch;
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

	Renderer_TextureUpdate_DataIn dataIn;
	DmaData_SPU::getAndWait(&dataIn, eaDataIn, sizeof(Renderer_TextureUpdate_DataIn));

	if(sc_verbose)
		spu_print("Renderer_TextureDataUpdate [SPU#%u] \n", idSpu);

	TextureDataUpdate(dataIn.xoffset, dataIn.yoffset, dataIn.width, dataIn.height, dataIn.pitch, dataIn.pSrcData, dataIn.pDstData, dataIn.level );


	if(sc_verbose)
		spu_print("Renderer_TextureDataUpdate [SPU#%u] exit\n", idSpu);
}

