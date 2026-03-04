/* SCE CONFIDENTIAL
PlayStation(R)3 Programmer Tool Runtime Library 430.001
* Copyright (C) 2007 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

/* common headers */
#include <stdint.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>
#include <cell/dma.h>

#include "ChunkRebuildData.h"
#include "TileRenderer_SPU.h"
#include "..\Common\DmaData.h"

// #define SPU_HEAPSIZE (0*1024)
// #define SPU_STACKSIZE (64*1024)
// 
// CELL_SPU_LS_PARAM(0*1024, 64*1024);	// can't use #defines here as it seems to  create an asm instruction


static const bool sc_verbose = false;
	
int g_lastHitBlockX = 0;
int g_lastHitBlockY = 0;
int g_lastHitBlockZ = 0;

CellSpursJobContext2* g_pSpursJobContext;

void cellSpursJobQueueMain(CellSpursJobContext2 *pContext, CellSpursJob256 *pJob)
{
	CellSpursTaskId idTask = cellSpursGetTaskId();
	unsigned int idSpu = cellSpursGetCurrentSpuId();

 	if(sc_verbose)
		spu_print("ChunkUpdate [SPU#%u] start\n", idSpu);
	
	g_pSpursJobContext = pContext;

// 	void* pVolatileMem = NULL;
// 	uint32_t volatileSize = 0;
// 	ret = cellSpursGetTaskVolatileArea(&pVolatileMem, &volatileSize);
// 	spu_print(	"----------------- ChunkUpdate ------------------\n"
// 				"Stack : %dKb\n"
// 				"Heap  : %dKb\n"
// 				"Prog  : %dKb\n"
// 				"Free  : %dKb\n"
// 				"-------------------------------------------------------------\n", 
// 				SPU_STACKSIZE/1024,
// 				SPU_HEAPSIZE/1024,
// 				256 - ((SPU_HEAPSIZE+SPU_STACKSIZE+volatileSize)/1024),
// 				volatileSize/1024);

// 	uint32_t eaEventFlag = spu_extract((vec_uint4)argTask, 0);
		uint32_t eaDataIn = pJob->workArea.userData[0];						
		uint32_t eaDataOut =pJob->workArea.userData[1];


	ChunkRebuildData rebuildData;
	TileRenderer_SPU tileRenderer(&rebuildData);
	unsigned int arrayData[4096+32];
	intArray_SPU tesselatorArray(arrayData);
	Tile_SPU::initTilePointers();

	DmaData_SPU::getAndWait(&rebuildData, eaDataIn, sizeof(ChunkRebuildData));
	g_lastHitBlockX = rebuildData.m_lastHitBlockX;
	g_lastHitBlockY = rebuildData.m_lastHitBlockY;
	g_lastHitBlockZ = rebuildData.m_lastHitBlockZ;

	rebuildData.m_tesselator._array = &tesselatorArray;
	rebuildData.disableUnseenTiles();
	rebuildData.tesselateAllTiles(&tileRenderer);
	DmaData_SPU::putAndWait(&rebuildData, eaDataOut, sizeof(ChunkRebuildData));

	if(sc_verbose)
	spu_print("ChunkUpdate [SPU#%u] exit\n", idTask, idSpu);
}

