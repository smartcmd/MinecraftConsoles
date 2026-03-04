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

#include "PerlinNoiseJob.h"
#include "..\Common\DmaData.h"
#include <vectormath/c/vectormath_aos_v.h>


static const bool sc_verbose = false;
CellSpursJobContext2* g_pSpursJobContext;


void cellSpursJobQueueMain(CellSpursJobContext2 *pContext, CellSpursJob256 *pJob)
{
	// 	CellSpursTaskId idTask = cellSpursGetTaskId();
	unsigned int idSpu = cellSpursGetCurrentSpuId();

	if(sc_verbose)
		spu_print("PerlinNoiseJob [SPU#%u] start\n", idSpu);

	g_pSpursJobContext = pContext;
	uint32_t eaDataIn = pJob->workArea.userData[0];						
	// 	uint32_t eaDataOut =pJob->workArea.userData[1];

	PerlinNoise_DataIn dataIn;
	DmaData_SPU::getAndWait(&dataIn, eaDataIn, sizeof(PerlinNoise_DataIn));


	dataIn.m_perlinData.getRegion(	dataIn.m_outputBuffer, 
									dataIn.x, dataIn.y, dataIn.z, 
									dataIn.xSize, dataIn.ySize, dataIn.zSize, 
									dataIn.xScale, dataIn.yScale, dataIn.zScale);
	


	if(sc_verbose)
		spu_print("PerlinNoiseJob [SPU#%u] exit\n", idSpu);
}

