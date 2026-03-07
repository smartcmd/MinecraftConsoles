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

#include "LevelRenderer_cull.h"
#include "../Common/DmaData.h"
#include <vectormath/c/vectormath_aos_v.h>



// #define SPU_HEAPSIZE (128*1024)
// #define SPU_STACKSIZE (16*1024)
// 
// CELL_SPU_LS_PARAM(128*1024, 16*1024);	// can't use #defines here as it seems to  create an asm instruction


static const bool sc_verbose = false;

CellSpursJobContext2* g_pSpursJobContext;


// The flag definitions
static const int    CHUNK_FLAG_COMPILED		= 0x01;
static const int    CHUNK_FLAG_DIRTY		= 0x02;
static const int    CHUNK_FLAG_EMPTY0		= 0x04;
static const int    CHUNK_FLAG_EMPTY1		= 0x08;
static const int	CHUNK_FLAG_EMPTYBOTH	= 0x0c;
static const int    CHUNK_FLAG_NOTSKYLIT	= 0x10;
static const int	CHUNK_FLAG_REF_MASK		= 0x07;
static const int	CHUNK_FLAG_REF_SHIFT	= 5;


bool inline clip(float *bb, float *frustum)
{
	for (int i = 0; i < 6; ++i, frustum += 4)
	{
		if (frustum[0] * (bb[0]) + frustum[1] * (bb[1]) + frustum[2] * (bb[2]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[3]) + frustum[1] * (bb[1]) + frustum[2] * (bb[2]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[0]) + frustum[1] * (bb[4]) + frustum[2] * (bb[2]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[3]) + frustum[1] * (bb[4]) + frustum[2] * (bb[2]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[0]) + frustum[1] * (bb[1]) + frustum[2] * (bb[5]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[3]) + frustum[1] * (bb[1]) + frustum[2] * (bb[5]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[0]) + frustum[1] * (bb[4]) + frustum[2] * (bb[5]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[3]) + frustum[1] * (bb[4]) + frustum[2] * (bb[5]) + frustum[3] > 0) continue;
		return false;
	}
	return true;
}

class PPUStoreArray
{
	static const int sc_cacheSize = 128;
	int m_localCache[128];
	int* m_pDataPPU;
	int m_cachePos;
	int m_ppuPos;

public:
	PPUStoreArray(uintptr_t pDataPPU) { m_pDataPPU = (int*)pDataPPU; m_cachePos = 0; m_ppuPos = 0;}

	void store(int val)
	{
		m_localCache[m_cachePos] = val;
		m_cachePos++;
		if(m_cachePos >= sc_cacheSize)
			flush();
	}

	void flush()
	{
		if(m_cachePos > 0)
		{
			// dma the local cache back to PPU and start again
// 			spu_print("DMAing %d bytes from 0x%08x(SPU) to 0x%08x(PPU)\n",(int)( m_cachePos*sizeof(int)), (int)m_localCache, (int)&m_pDataPPU[m_ppuPos]);
			DmaData_SPU::put(m_localCache, (uintptr_t)&m_pDataPPU[m_ppuPos], DmaData_SPU::roundUpDMASize(m_cachePos*sizeof(int)));
			m_ppuPos += m_cachePos;
			m_cachePos = 0;
		}
	}
	int getSize() { return m_ppuPos; }
};

void cull(LevelRenderer_cull_DataIn* dataIn, LevelRenderer_cull_DataIn* eaDataIn)
{
	ClipChunk_SPU chunkDoubleBuffer[2];		
	int chunkIdx = 0;
 	unsigned char* globalChunkFlags = (unsigned char*)alloca(dataIn->numGlobalChunks);		// 164K !!!
 	DmaData_SPU::getAndWait(globalChunkFlags, (uintptr_t)dataIn->pGlobalChunkFlags, sizeof(unsigned char)*dataIn->numGlobalChunks);

	PPUStoreArray layer0List((uintptr_t)dataIn->listArray_layer0);
	PPUStoreArray layer1List((uintptr_t)dataIn->listArray_layer1);

	PPUStoreArray layer0ZDepth((uintptr_t)dataIn->zDepth_layer0);
	PPUStoreArray layer1ZDepth((uintptr_t)dataIn->zDepth_layer1);
	float xOff = dataIn->clipMat[3][0];
	float yOff = dataIn->clipMat[3][1];
	float zOff = dataIn->clipMat[3][2];
	dataIn->clipMat[3][0] = 0;
	dataIn->clipMat[3][1] = 0;
	dataIn->clipMat[3][2] = 0;




	// DMA up the first chunk
	if(dataIn->numClipChunks > 0)
		DmaData_SPU::get(&chunkDoubleBuffer[chunkIdx], (uintptr_t)&dataIn->pClipChunks[0], sizeof(ClipChunk_SPU));

	for(int i=0;i<dataIn->numClipChunks; i++)
	{
		DmaData_SPU::wait();// wait for the last chunk to have been uploaded
		ClipChunk_SPU& chunk = chunkDoubleBuffer[chunkIdx];
		chunkIdx ^= 1;
		// queue up the next chunk
		DmaData_SPU::get(&chunkDoubleBuffer[chunkIdx], (uintptr_t)&dataIn->pClipChunks[i+1], sizeof(ClipChunk_SPU));

		Vectormath::Aos::Vector4 pos(chunk.aabb[0] + xOff, chunk.aabb[1] + yOff, chunk.aabb[2]  + zOff, 1.0f);
		Vectormath::Aos::Vector4 transPos = dataIn->clipMat * pos;
		float zDepth = -transPos.getZ();
// 		spu_print("z val = %f : maxDepth = %f\n", zDepth, dataIn->maxDepthRender);
		if(zDepth > dataIn->maxDepthRender || chunk.aabb[1] < dataIn->maxHeightRender )
			chunk.visible = false;
		else
		{
			unsigned char flags = chunk.globalIdx == -1 ? 0 : globalChunkFlags[ chunk.globalIdx ];
			chunk.visible = false;
			if ( (flags & CHUNK_FLAG_COMPILED ) && ( ( flags & CHUNK_FLAG_EMPTYBOTH ) != CHUNK_FLAG_EMPTYBOTH ) )
			{
				chunk.visible = clip(chunk.aabb, dataIn->fdraw);
			}
		}
		// write the visible flag directly into the chunk structure on PPU
		DmaData_SPU::put(&chunk, (uintptr_t)&dataIn->pClipChunks[i], 16);// only DMA the first 16 bytes, as they contain the visible flag we need to pass back

		// add the data for renderChunks
		if( !chunk.visible ) 
			continue;													// This will be set if the chunk isn't visible, or isn't compiled, or has both empty flags set
		if( chunk.globalIdx == -1 ) 
			continue;												// Not sure if we should ever encounter this... TODO check
		int layer = 0;
// 		zDepth /= transPos.getW(); 
		if( ( globalChunkFlags[chunk.globalIdx] & CHUNK_FLAG_EMPTY0 ) == 0)	// Check that this particular layer isn't empty
		{
			// List can be calculated directly from the chunk's global index
			int list = chunk.globalIdx * 2 + layer;
			list += dataIn->chunkLists;
			layer0List.store(list);
			layer0ZDepth.store(*((int*)&zDepth));
		}

		layer = 1;
		if( ( globalChunkFlags[chunk.globalIdx] & CHUNK_FLAG_EMPTY1 ) == 0)	// Check that this particular layer isn't empty
		{
			// List can be calculated directly from the chunk's global idex
			int list = chunk.globalIdx * 2 + layer;
			list += dataIn->chunkLists;
			layer1List.store(list);
			layer0ZDepth.store(*((int*)&zDepth));
		}
	}
	layer0List.flush();
	layer1List.flush();
	layer0ZDepth.flush();
	cellDmaPutUint32(layer0List.getSize(), (uintptr_t)&eaDataIn->numToRender_layer0, 0, 0, 0);
	cellDmaPutUint32(layer1List.getSize(), (uintptr_t)&eaDataIn->numToRender_layer1, 0, 0, 0);

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

	if(sc_verbose)
		spu_print("LevelRenderer_cull [SPU#%u] DMA'd input data, ready to cull %d chunks\n", idSpu, dataIn.numClipChunks);

	int numForDMA = dataIn.numClipChunks;
	if(numForDMA % 16)
	{
		numForDMA &= ~0x0f;
		numForDMA += 0x10;
	}

	cull(&dataIn, (LevelRenderer_cull_DataIn*)eaDataIn);
// 	DmaData_SPU::putAndWait(outputData, eaDataOut, sizeof(char) * numForDMA);


	if(sc_verbose)
		spu_print("LevelRenderer_cull [SPU#%u] exit\n", idSpu);
}

