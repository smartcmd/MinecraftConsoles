/* SCE CONFIDENTIAL
PlayStation(R)3 Programmer Tool Runtime Library 430.001
* Copyright (C) 2007 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

/* common headers */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>
#include <spu_printf.h>
#include <cell/dma.h>
#include <cell/spurs/job_queue.h>

#include "../Common/DmaData.h"

#include "CompressedTileStorage_getData.h"

static const bool sc_verbose = false;

CellSpursJobContext2* g_pSpursJobContext;
	



class CCompressedTileStorage_getData
{
public:
	unsigned char	indicesAndData[32768+4096];
	int				allocatedSize;
	int				newAllocatedSize;
	uint32_t		newIndicesPPU;
private:

	static const int INDEX_OFFSET_MASK			= 0x7ffe;
	static const int INDEX_OFFSET_SHIFT			= 1;
	static const int INDEX_TILE_MASK			= 0x00ff;
	static const int INDEX_TILE_SHIFT			= 8;
	static const int INDEX_TYPE_MASK			= 0x0003;
	static const int INDEX_TYPE_1_BIT			= 0x0000;
	static const int INDEX_TYPE_2_BIT			= 0x0001;
	static const int INDEX_TYPE_4_BIT			= 0x0002;
	static const int INDEX_TYPE_0_OR_8_BIT		= 0x0003;
	static const int INDEX_TYPE_0_BIT_FLAG		= 0x0004;

public:
 	CCompressedTileStorage_getData(unsigned char* idxAndData, int dataSize) 
	{
		allocatedSize = dataSize; 
		spu_assert(allocatedSize < (int)sizeof(indicesAndData));
		DmaData_SPU::getAndWait(indicesAndData, (uintptr_t)idxAndData, DmaData_SPU::roundUpDMASize(allocatedSize));
	}
	void getData(uint8_t* retArray, unsigned int retOffset);

	// Get an index into the normal ordering of tiles for the java game, given a block index (0 to 511) and a tile index (0 to 63)
	int getIndex(int block, int tile)
	{
		// bits for index into data is: xxxxzzzzyyyyyyy
		// we want block(b) & tile(t) spread out as:
		//			from:		______bbbbbbbbb
		//          to:			bb__bb__bbbbb__
		//
		//			from:		_________tttttt
		//			to:			__tt__tt_____tt

		int index = ( ( block & 0x180) << 6 ) | ( ( block & 0x060 ) << 4 ) | ( ( block & 0x01f ) << 2 );
		index |= ( ( tile & 0x30 ) << 7) | ( ( tile & 0x0c ) << 5 ) |  ( tile & 0x03 );

		return index;
	}

};

void CCompressedTileStorage_getData::getData(uint8_t* retArray, unsigned int retOffset)
{
	unsigned short *blockIndices = (unsigned short *)indicesAndData;
	unsigned char *data = indicesAndData + 1024;

	for( int i = 0; i < 512; i++ )
	{
		int indexType = blockIndices[i] & INDEX_TYPE_MASK;
		if( indexType == INDEX_TYPE_0_OR_8_BIT )
		{
			if( blockIndices[i] & INDEX_TYPE_0_BIT_FLAG )
			{
				for( int j = 0; j < 64; j++ )
				{
					retArray[getIndex(i,j) + retOffset] = ( blockIndices[i] >> INDEX_TILE_SHIFT ) & INDEX_TILE_MASK;
				}
			}
			else
			{
				// 8-bit reads are just directly read from the 64 long array of values stored for the block
				unsigned char *packed = data + ( ( blockIndices[i] >> INDEX_OFFSET_SHIFT ) & INDEX_OFFSET_MASK );

				for( int j = 0; j < 64; j++ )
				{
					retArray[getIndex(i,j) + retOffset] = packed[j];
				}
			}
		}
		else
		{
			// 1, 2, or 4 bits per block packed format

			int bitspertile = 1 << indexType;			// will be 1, 2 or 4 (from index values of 0, 1, 2)
			int tiletypecount = 1 << bitspertile;		// will be 2, 4 or 16 (from index values of 0, 1, 2)
			int tiletypemask = tiletypecount - 1;		// will be 1, 3 or 15 (from index values of 0, 1, 2)
			int indexshift = 3 - indexType;				// will be 3, 2 or 1 (from index values of 0, 1, 2)
			int indexmask_bits = 7 >> indexType;		// will be 7, 3 or 1 (from index values of 0, 1, 2)
			int indexmask_bytes = 62 >> indexshift;		// will be 7, 15 or 31 (from index values of 0, 1, 2)

			unsigned char *tile_types = data + ( ( blockIndices[i] >> INDEX_OFFSET_SHIFT ) & INDEX_OFFSET_MASK );
			unsigned char *packed = tile_types + tiletypecount;

			for( int j = 0; j < 64; j++ )
			{
				int idx = ( j >> indexshift ) & indexmask_bytes;
				int bit = ( j & indexmask_bits ) * bitspertile;
				retArray[getIndex(i,j) + retOffset] = tile_types[( packed[idx] >> bit ) & tiletypemask];
			}
		}
	}
}



void cellSpursJobQueueMain(CellSpursJobContext2 *pContext, CellSpursJob256 *pJob)
{
// 	CellSpursTaskId idTask = cellSpursGetTaskId();
	unsigned int idSpu = cellSpursGetCurrentSpuId();

	if(sc_verbose)
		spu_printf("CompressedTileStorage_getData [SPU#%u] start\n", idSpu);

	g_pSpursJobContext = pContext;

	unsigned char* pIdxAndData = (unsigned char*)pJob->workArea.userData[0];						
	int dataSize = (int)pJob->workArea.userData[1];						
	unsigned char* pDst = (unsigned char*)pJob->workArea.userData[2];						
	unsigned int retOffset = (unsigned int)pJob->workArea.userData[3];	

	CCompressedTileStorage_getData c(pIdxAndData, dataSize);

	unsigned char	retArray[32768];

	c.getData(retArray, retOffset);

	DmaData_SPU::putAndWait(retArray, (uintptr_t)pDst, 32768);

	if(sc_verbose)
	 	spu_printf("CompressedTileStorage_getData [SPU#%u] exit\n", idSpu);
}

