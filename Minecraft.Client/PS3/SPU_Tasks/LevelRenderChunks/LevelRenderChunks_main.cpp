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
#include <spu_printf.h>
#include <cell/dma.h>
#include <cell/gcm_spu.h>
#include <cell/gcm_spu.h>


#include "..\Common\DmaData.h"
#include "LevelRenderChunks.h"
#include <cell/gcm_spu.h>

#define SPU_HEAPSIZE (128*1024)
#define SPU_STACKSIZE (16*1024)

CELL_SPU_LS_PARAM(128*1024, 16*1024);	// can't use #defines here as it seems to  create an asm instruction


static const bool sc_verbose = false;



static int32_t gcmReserveFailed(CellGcmContextData *context, uint32_t count)
{
	(void)count;
	spu_assert(0);
	context->current = context->begin;				// Back to the beginning so we don't trample memory.
	return CELL_OK;
}

	
int cellSpursTaskMain(qword argTask, uint64_t argTaskset)
{
	(void)argTaskset;
	int ret;
	CellSpursTaskId idTask = cellSpursGetTaskId();
	unsigned int idSpu = cellSpursGetCurrentSpuId();

// 	if(sc_verbose)
		spu_printf("[Task#%02u][SPU#%u] start\n", idTask, idSpu);

	void* pVolatileMem = NULL;
	uint32_t volatileSize = 0;
	ret = cellSpursGetTaskVolatileArea(&pVolatileMem, &volatileSize);
	spu_printf(	"----------------- LevelRenderChunks SPU Memory ------------------\n"
				"Stack : %dKb\n"
				"Heap  : %dKb\n"
				"Prog  : %dKb\n"
				"Free  : %dKb\n"
				"-------------------------------------------------------------\n", 
				SPU_STACKSIZE/1024,
				SPU_HEAPSIZE/1024,
				256 - ((SPU_HEAPSIZE+SPU_STACKSIZE+volatileSize)/1024),
				volatileSize/1024);
																				
	uint32_t eaEventFlag = spu_extract((vec_uint4)argTask, 0);					
	uint32_t eaDataIn = spu_extract((vec_uint4)argTask, 1);						
	uint32_t eaDataOut = spu_extract((vec_uint4)argTask, 2);

 	cell::Spurs::EventFlagStub eventFlag;
 	eventFlag.setObject(eaEventFlag);
 
	RenderChunksSpuDataIn* pDataIn = new RenderChunksSpuDataIn;
	RenderChunksSpuDataOut* pDataOut = new RenderChunksSpuDataOut;
	CellGcmContextData gcmContext;
	cellGcmSetupContextData(&gcmContext, pDataOut->m_commandBuffer, sizeof(pDataOut->m_commandBuffer), gcmReserveFailed);

 
 	while(1)
 	{
 		// wait for the chunk rebuild thread to signal us
 		uint16_t mask = 0x1;
 		if(sc_verbose)
 			spu_printf("[Task#%02u][SPU#%u] waiting for event flag#0, mask = 0x%04x\n", idTask, idSpu, mask);
 		ret = eventFlag.wait(&mask, CELL_SPURS_EVENT_FLAG_AND);
 		if (ret) {
 			spu_printf("[Task#%02u][SPU#%u] eventFlag0.wait() failed : %x\n", idTask, idSpu, ret);
 			abort();
 		}
 
 		if(sc_verbose)
 			spu_printf("[Task#%02u][SPU#%u] woken up\n", idTask, idSpu);
 		eventFlag.clear(0x01);
 
 		DmaData_SPU::getAndWait(pDataIn, eaDataIn, sizeof(RenderChunksSpuDataIn));

		for(int i=0;i<pDataIn->m_numLists; i++)
		{
			cellGcmSetCallCommand(&gcmContext, pDataIn->m_listArray[i]);
		}

		pDataOut->m_commandBufferSize = ((unsigned int)gcmContext.current) - ((unsigned int)gcmContext.begin);

		DmaData_SPU::putAndWait(pDataOut, eaDataOut, sizeof(RenderChunksSpuDataOut));

		mask = 0x2;
		if(sc_verbose)
			spu_printf("[Task#%02u][SPU#%u] set event flag#1, mask = 0x%04x\n", idTask, idSpu, mask);
		ret = eventFlag.set(mask);
		if (ret) {
			spu_printf("[Task#%02u][SPU#%u] eventFlag1.set(mask) failed : %x\n", idTask, idSpu, ret);
			abort();
		}
	}
	spu_printf("[Task#%02u][SPU#%u] exit\n", idTask, idSpu);
	return 0;
}

