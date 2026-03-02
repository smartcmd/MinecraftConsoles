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

#include "LevelRenderer_FindNearestChunk.h"
#include "..\Common\DmaData.h"
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


bool LevelRenderer_FindNearestChunk_DataIn::MultiplayerChunkCache::getChunkEmpty(int lowerOffset, int upperOffset, int x, int y, int z)
{
	x>>=4;
	z>>=4;
	int ix = x + XZOFFSET;
	int iz = z + XZOFFSET;
	// Check we're in range of the stored level
	if( ( ix < 0 ) || ( ix >= XZSIZE ) ) return false; // ( waterChunk ? waterChunk : emptyChunk );
	if( ( iz < 0 ) || ( iz >= XZSIZE ) ) return false; //( waterChunk ? waterChunk : emptyChunk );
	int idx = ix * XZSIZE + iz;

// 	spu_print("grabbing pointer idx %d from 0x%08x", idx, (uintptr_t)&cache[idx]);
	uint32_t chunkPointer = DmaData_SPU::getValue32((uintptr_t)&cache[idx]);
// 	spu_print(" value - 0x%08x\n", chunkPointer);

	if( chunkPointer == NULL )
	{
		return false;
	}
	else
	{
		CompressedTileStorage blocks;
		uintptr_t pBlocks;
		// using a class structure offset here as we don't want to be compiling LevelChunk on SPU
		int chunkY = y;
		if( y >= 128 )
		{
			pBlocks = DmaData_SPU::getValue32((uintptr_t)(chunkPointer+upperOffset));
			chunkY -= 128;
		}
		else
		{
			pBlocks = DmaData_SPU::getValue32((uintptr_t)(chunkPointer+lowerOffset));
		}
		DmaData_SPU::getAndWaitUnaligned(&blocks, pBlocks, sizeof(CompressedTileStorage));
		return blocks.isRenderChunkEmpty(chunkY);
	}
}


bool LevelRenderer_FindNearestChunk_DataIn::CompressedTileStorage::isRenderChunkEmpty(int y)	// y == 0, 16, 32... 112 (representing a 16 byte range) 
{
	int blockIdx;
	unsigned short *blockIndices = (unsigned short *)indicesAndData;

	for( int x = 0; x < 16; x += 4 )
	{
		for( int z = 0; z < 16; z += 4 )
		{
			getBlock(&blockIdx, x, y, z);
			uint16_t comp;
			comp = DmaData_SPU::getValue16((uintptr_t)&blockIndices[blockIdx]);
			if( comp != 0x0007 ) return false;
			comp = DmaData_SPU::getValue16((uintptr_t)&blockIndices[blockIdx+1]);
			if( comp != 0x0007 ) return false;
			comp = DmaData_SPU::getValue16((uintptr_t)&blockIndices[blockIdx+2]);
			if( comp != 0x0007 ) return false;
			comp = DmaData_SPU::getValue16((uintptr_t)&blockIndices[blockIdx+3]);
			if( comp != 0x0007 ) return false;
		}
	}
	return true;
}


void LevelRenderer_FindNearestChunk_DataIn::findNearestChunk()
{
 	unsigned char* globalChunkFlags = (unsigned char*)alloca(numGlobalChunks);		// 164K !!!
 	DmaData_SPU::getAndWait(globalChunkFlags, (uintptr_t)pGlobalChunkFlags, sizeof(unsigned char)*numGlobalChunks);

	
	nearChunk = NULL;			// Nearest chunk that is dirty
	veryNearCount = 0;
	int minDistSq = 0x7fffffff;		// Distances to this chunk

	
	// Find nearest chunk that is dirty
	for( int p = 0; p < 4; p++ )
	{
		// It's possible that the localplayers member can be set to NULL on the main thread when a player chooses to exit the game
		// So take a reference to the player object now. As it is a shared_ptr it should live as long as we need it
		PlayerData* player = &playerData[p]; 
		if( player->bValid == NULL ) continue;
		if( chunks[p] == NULL ) continue;
		if( level[p] == NULL ) continue;
		if( chunkLengths[p] != xChunks * zChunks * CHUNK_Y_COUNT ) continue;
		int px = (int)player->x;
		int py = (int)player->y;
		int pz = (int)player->z;		

		ClipChunk clipChunk[512];

		for( int z = 0; z < zChunks; z++ )
		{ 
			uintptr_t ClipChunkX_PPU = (uintptr_t)&chunks[p][(z * yChunks + 0) * xChunks + 0];
			DmaData_SPU::getAndWait(&clipChunk[0], ClipChunkX_PPU, sizeof(ClipChunk) * xChunks*CHUNK_Y_COUNT);
			for( int y = 0; y < CHUNK_Y_COUNT; y++ )
			{
				for( int x = 0; x < xChunks; x++ )
				{
					ClipChunk *pClipChunk = &clipChunk[(y) * xChunks + x];

					// Get distance to this chunk - deliberately not calling the chunk's method of doing this to avoid overheads (passing entitie, type conversion etc.) that this involves
					int xd = pClipChunk->xm - px;
					int yd = pClipChunk->ym - py;		
					int zd = pClipChunk->zm - pz;
					int distSq = xd * xd + yd * yd + zd * zd;
					int distSqWeighted = xd * xd + yd * yd * 4 + zd * zd;  // Weighting against y to prioritise things in same x/z plane as player first

					if( globalChunkFlags[ pClipChunk->globalIdx ] & CHUNK_FLAG_DIRTY )
					{
						if( (!onlyRebuild) ||
							globalChunkFlags[ pClipChunk->globalIdx ] & CHUNK_FLAG_COMPILED ||
							( distSq < 20 * 20 ) )	// Always rebuild really near things or else building (say) at tower up into empty blocks when we are low on memory will not create render data
						{
							// Is this chunk nearer than our nearest?
							if( distSqWeighted < minDistSq )
							{
								// At this point we've got a chunk that we would like to consider for rendering, at least based on its proximity to the player(s).
								// Its *quite* quick to generate empty render data for render chunks, but if we let the rebuilding do that then the after rebuilding we will have
								// to start searching for the next nearest chunk from scratch again. Instead, its better to detect empty chunks at this stage, flag them up as not dirty
								// (and empty), and carry on. The levelchunk's isRenderChunkEmpty method can be quite optimal as it can make use of the chunk's data compression to detect
								// emptiness without actually testing as many data items as uncompressed data would.
 								Chunk chunk;
								DmaData_SPU::getAndWait(&chunk, (uintptr_t)pClipChunk->chunk, sizeof(Chunk));
								if(!multiplayerChunkCache[p].getChunkEmpty(lowerOffset, upperOffset, chunk.x, y*16, chunk.z))
 								{
									uintptr_t ClipChunkPPU = (uintptr_t)&chunks[p][(z * yChunks + y) * xChunks + x];
									nearChunk = (ClipChunk*)ClipChunkPPU;
 									minDistSq = distSqWeighted;
 								}
 								else
 								{
 									globalChunkFlags[ pClipChunk->globalIdx ] &= ~CHUNK_FLAG_DIRTY;
  									globalChunkFlags[ pClipChunk->globalIdx ] |= CHUNK_FLAG_EMPTYBOTH;
 								}
							}

							if( distSq < 20 * 20 )
							{
								veryNearCount++;
							}
						}
					}
				}
			}
		}
	}

 	DmaData_SPU::putAndWait(globalChunkFlags, (uintptr_t)pGlobalChunkFlags, sizeof(unsigned char)*numGlobalChunks);

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

	LevelRenderer_FindNearestChunk_DataIn dataIn;
	DmaData_SPU::getAndWait(&dataIn, eaDataIn, sizeof(LevelRenderer_FindNearestChunk_DataIn));
	
	dataIn.findNearestChunk();

 	DmaData_SPU::putAndWait(&dataIn, eaDataIn, sizeof(LevelRenderer_FindNearestChunk_DataIn));


	if(sc_verbose)
		spu_print("LevelRenderer_cull [SPU#%u] exit\n", idSpu);
}

