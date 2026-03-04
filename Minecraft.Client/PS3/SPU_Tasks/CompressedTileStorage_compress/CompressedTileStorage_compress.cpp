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
#include <cell/dma.h>
#include <cell/spurs/job_queue.h>

#include "..\Common\DmaData.h"

#include "CompressedTileStorage_compress.h"

static const bool sc_verbose = false;

CellSpursJobContext2* g_pSpursJobContext;
	



class CCompressedTileStorage_compress
{
public:
	unsigned char	indicesAndData[32768+4096];
	unsigned char   newIndicesAndData[32768+4096];
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
 	CCompressedTileStorage_compress(CompressedTileStorage_compress_dataIn& dataIn) 
	{
		allocatedSize = dataIn.allocatedSize; 
		newIndicesPPU = (uint32_t)dataIn.newIndicesAndData;
		spu_assert(allocatedSize < (int)sizeof(indicesAndData));
		DmaData_SPU::getAndWait(indicesAndData, (uintptr_t)dataIn.indicesAndData, DmaData_SPU::roundUpDMASize(allocatedSize));
	}
	bool compress(int upgradeBlock = -1);
};

// Compresses the data currently stored in one of two ways:
// (1) Attempt to compresses every block as much as possible (if upgradeBlock is -1)
// (2) Copy all blocks as-is apart from the block specified by upgradeBlock ( if > -1 ), which is changed to be the next-most-accomodating storage from its current state
bool  CCompressedTileStorage_compress::compress(int upgradeBlock/*=-1*/)
{
	unsigned char tempdata[64];
	unsigned short _blockIndices[512];
	bool needsCompressed = ( upgradeBlock > -1 );	// If an upgrade block is specified, we'll always need to recompress - otherwise default to false


	unsigned short *blockIndices = (unsigned short *)indicesAndData;
	unsigned char *data = indicesAndData + 1024;

	int memToAlloc = 0;
	for( int i = 0; i < 512; i++ )
	{
		unsigned short indexType = blockIndices[i] & INDEX_TYPE_MASK;

		unsigned char *unpacked_data = NULL;
		unsigned char *packed_data;

		// First task is to find out what type of storage each block needs. Need to unpack each where required. 
		// Note that we don't need to fully unpack the data at this stage since we are only interested in working out how many unique types of tiles are in each block, not
		// what those actual tile ids are.
		if( upgradeBlock == -1 )
		{
			if( indexType == INDEX_TYPE_0_OR_8_BIT )
			{
				// Note that we are only interested in data that can be packed further, so we don't need to consider things that are already at their most compressed
				// (ie with INDEX_TYPE_0_BIT_FLAG set)
				if( ( blockIndices[i] & INDEX_TYPE_0_BIT_FLAG ) == 0 )
				{
					unpacked_data = data + ( ( blockIndices[i] >> INDEX_OFFSET_SHIFT ) & INDEX_OFFSET_MASK );
				}
			}
			else
			{
				int bitspertile = 1 << indexType;			// will be 1, 2 or 4 (from index values of 0, 1, 2)
				int tiletypecount = 1 << bitspertile;		// will be 2, 4 or 16
				int tiletypemask = tiletypecount - 1;		// will be 1, 3 or 15
				int indexshift = 3 - indexType;				// will be 3, 2 or 1 (from index values of 0, 1, 2)
				int indexmask_bits = 7 >> indexType;		// will be 7, 3 or 1 (from index values of 0, 1, 2)
				int indexmask_bytes = 62 >> indexshift;		// will be 7, 15 or 31 (from index values of 0, 1, 2)

				unpacked_data = tempdata;
				packed_data = data + ( ( blockIndices[i] >> INDEX_OFFSET_SHIFT ) & INDEX_OFFSET_MASK ) + tiletypecount;

				for( int j = 0; j < 64; j++ )
				{
					int idx = (j >> indexshift) & indexmask_bytes;
					int bit = ( j & indexmask_bits ) * bitspertile;

					unpacked_data[j] = ( packed_data[idx] >> bit ) & tiletypemask;	// Doesn't need the actual data for each tile, just unique values
				}
			}

			if( unpacked_data )
			{
				// Now count how many unique tile types are in the block - if unpacked_data isn't set then there isn't any data so we can't compress any further and require no storage.
				// Store flags for each tile type used in an array of 4 64-bit flags.

				uint64_t usedFlags[4] = {0,0,0,0};

				int64_t i64_1 = 1;	// MGH - instead of 1i64, which is MS specific
				for( int j = 0; j < 64; j++ )			// This loop of 64 is to go round the 4x4x4 tiles in the block
				{
					int tiletype = unpacked_data[j];
					usedFlags[tiletype & 3] |= ( i64_1 << ( tiletype >> 2 ) );
				}
				int count = 0;
				for( int tiletype = 0; tiletype < 256; tiletype++ )	// This loop of 256 is to go round the 256 possible values that the tiles might have had to find how many are actually used
				{
					if( usedFlags[tiletype & 3] & ( i64_1 << ( tiletype >> 2 ) ) )
					{
						count++;
					}
				}
				if( count == 1 )
				{
					_blockIndices[i] = INDEX_TYPE_0_OR_8_BIT | INDEX_TYPE_0_BIT_FLAG;

					// We'll need to compress if this isn't the same type as before. If it *was* a 0-bit one though, then unpacked_data wouldn't have been set and we wouldn't be here
					needsCompressed = true;
				}
				else if( count == 2 )
				{
					_blockIndices[i] = INDEX_TYPE_1_BIT;
					if( indexType != INDEX_TYPE_1_BIT ) needsCompressed = true;
					memToAlloc += 10;		// 8 bytes + 2 tile index
				}
				else if ( count <= 4 )
				{
					_blockIndices[i] = INDEX_TYPE_2_BIT;
					if( indexType != INDEX_TYPE_2_BIT ) needsCompressed = true;
					memToAlloc += 20;		// 16 bytes + 4 tile index
				}
				else if ( count <= 16 )
				{
					_blockIndices[i] = INDEX_TYPE_4_BIT;
					if( indexType != INDEX_TYPE_4_BIT ) needsCompressed = true;
					memToAlloc += 48;		// 32 bytes + 16 tile index
				}
				else
				{
					_blockIndices[i] = INDEX_TYPE_0_OR_8_BIT;
					memToAlloc = ( memToAlloc + 3 ) & 0xfffc;		// Make sure we are 4-byte aligned for 8-bit storage
					memToAlloc += 64;
				}
			}
			else
			{
				// Already will be 0 bits, so we can't do any further compression - just copy the index over.
				_blockIndices[i] = blockIndices[i];
			}
		}
		else
		{
			if( i == upgradeBlock )
			{
				// INDEX_TYPE_1_BIT (0) -> INDEX_TYPE_2_BIT (1)
				// INDEX_TYPE_2_BIT (1) -> INDEX_TYPE_4_BIT (2)
				// INDEX_TYPE_4_BIT (2) -> INDEX_TYPE_0_OR_8_BIT (3)	(new will be 8-bit)
				// INDEX_TYPE_0_OR_8_BIT (3) -> INDEX_TYPE_1_BIT (0)	(assuming old was 0-bit)
				_blockIndices[i] = ( ( blockIndices[i] & INDEX_TYPE_MASK ) + 1 ) & INDEX_TYPE_MASK;
			}
			else
			{
				// Copy over the index, without the offset.
				_blockIndices[i] = blockIndices[i] & INDEX_TYPE_MASK;
				if( _blockIndices[i] == INDEX_TYPE_0_OR_8_BIT )
				{
					_blockIndices[i] |= ( blockIndices[i] & INDEX_TYPE_0_BIT_FLAG );
				}
			}
			switch(_blockIndices[i])
			{				
			case INDEX_TYPE_1_BIT:
				memToAlloc += 10;
				break;
			case INDEX_TYPE_2_BIT:
				memToAlloc += 20;
				break;
			case INDEX_TYPE_4_BIT:
				memToAlloc += 48;
				break;
			case INDEX_TYPE_0_OR_8_BIT:
				memToAlloc = ( memToAlloc + 3 ) & 0xfffc;		// Make sure we are 4-byte aligned for 8-bit storage
				memToAlloc += 64;
				break;
				// Note that INDEX_TYPE_8_BIT|INDEX_TYPE_0_BIT_FLAG not in here as it doesn't need any further allocation
			}
		}
	}

	// If we need to do something here, then lets allocate some memory
	if( needsCompressed )
	{
		memToAlloc += 1024; // For the indices
		unsigned char *pucData = newIndicesAndData + 1024;
		unsigned short usDataOffset = 0;
		unsigned short *newIndices = (unsigned short *) newIndicesAndData;

		// Now pass through again actually making the final compressed data
		for( int i = 0; i < 512; i++ )
		{
			unsigned short indexTypeNew = _blockIndices[i] & INDEX_TYPE_MASK;
			unsigned short indexTypeOld = blockIndices[i] & INDEX_TYPE_MASK;
			newIndices[i] = indexTypeNew;

			// Is the type unmodifed? Then can just copy over
			bool done = false;
			if( indexTypeOld == indexTypeNew )
			{
				unsigned char *packed_data;
				if( indexTypeOld == INDEX_TYPE_0_OR_8_BIT )
				{
					if( ( blockIndices[i] & INDEX_TYPE_0_BIT_FLAG ) == ( _blockIndices[i] & INDEX_TYPE_0_BIT_FLAG ) )
					{
						if( blockIndices[i] & INDEX_TYPE_0_BIT_FLAG )
						{
							newIndices[i] = blockIndices[i];
						}
						else
						{
							packed_data = data + ( ( blockIndices[i] >> INDEX_OFFSET_SHIFT ) & INDEX_OFFSET_MASK);
							usDataOffset = (usDataOffset + 3 ) & 0xfffc;
							memcpy( pucData + usDataOffset, packed_data, 64 );
							newIndices[i] |= ( usDataOffset & INDEX_OFFSET_MASK) << INDEX_OFFSET_SHIFT;
							usDataOffset += 64;
						}
						done = true;
					}
				}
				else
				{
					packed_data = data + ( ( blockIndices[i] >> INDEX_OFFSET_SHIFT ) & INDEX_OFFSET_MASK);

					int dataSize = 8 << indexTypeOld;		// 8, 16 or 32 bytes of per-tile storage
					dataSize += 1 << ( 1 << indexTypeOld );	// 2, 4 or 16 bytes to store each tile type
					newIndices[i] |= ( usDataOffset & INDEX_OFFSET_MASK) << INDEX_OFFSET_SHIFT;
					memcpy( pucData + usDataOffset, packed_data, dataSize );
					usDataOffset += dataSize;
					done = true;
				}
			}


			// If we're not done, then we actually need to recompress this block. First of all decompress from its current format.
			if( !done )
			{
				unsigned char *unpacked_data = NULL;
				unsigned char *tile_types = NULL;
				unsigned char *packed_data = NULL;
				if( indexTypeOld == INDEX_TYPE_0_OR_8_BIT )
				{
					if( blockIndices[i] & INDEX_TYPE_0_BIT_FLAG )
					{
						unpacked_data  = tempdata;
						int value = ( blockIndices[i] >> INDEX_TILE_SHIFT ) & INDEX_TILE_MASK;
						memset( tempdata, value, 64 );
					}
					else
					{
						unpacked_data = data + ( ( blockIndices[i] >> INDEX_OFFSET_SHIFT ) & INDEX_OFFSET_MASK );
					}
				}
				else
				{
					int bitspertile = 1 << indexTypeOld;		// will be 1, 2 or 4 (from index values of 0, 1, 2)
					int tiletypecount = 1 << bitspertile;		// will be 2, 4 or 16
					int tiletypemask = tiletypecount - 1;		// will be 1, 3 or 15
					int indexshift = 3 - indexTypeOld;			// will be 3, 2 or 1 (from index values of 0, 1, 2)
					int indexmask_bits = 7 >> indexTypeOld;		// will be 7, 3 or 1 (from index values of 0, 1, 2)
					int indexmask_bytes = 62 >> indexshift;		// will be 7, 15 or 31 (from index values of 0, 1, 2)

					unpacked_data = tempdata;
					tile_types = data + ( ( blockIndices[i] >> INDEX_OFFSET_SHIFT ) & INDEX_OFFSET_MASK );
					packed_data = tile_types + tiletypecount;
					for( int j = 0; j < 64; j++ )
					{
						int idx = ( j >> indexshift ) & indexmask_bytes;
						int bit = ( j & indexmask_bits ) * bitspertile;

						unpacked_data[j] = tile_types[(packed_data[idx] >> bit) & tiletypemask];
					}
				}

				// And finally repack
				unsigned char ucMappings[256] = {0};
				for( int j = 0; j < 256; j++ )
				{
					ucMappings[j] = 255;
				}

				unsigned char *repacked = NULL;

				if( indexTypeNew == INDEX_TYPE_0_OR_8_BIT )
				{
					if( _blockIndices[i] & INDEX_TYPE_0_BIT_FLAG )
					{
						newIndices[i] = INDEX_TYPE_0_OR_8_BIT | INDEX_TYPE_0_BIT_FLAG | (((unsigned short)unpacked_data[0]) << INDEX_TILE_SHIFT);
					}
					else
					{
						usDataOffset = (usDataOffset + 3 ) & 0xfffc;									// Make sure offset is 4 byte aligned
						memcpy( pucData + usDataOffset, unpacked_data, 64 );
						newIndices[i] |= ( usDataOffset & INDEX_OFFSET_MASK) << INDEX_OFFSET_SHIFT;
						usDataOffset += 64;
					}
				}
				else
				{
					int bitspertile = 1 << indexTypeNew;		// will be 1, 2 or 4 (from index values of 0, 1, 2)
					int tiletypecount = 1 << bitspertile;		// will be 2, 4 or 16 (from index values of 0, 1, 2)
					int tiletypemask = tiletypecount - 1;		// will be 1, 3 or 15 (from index values of 0, 1, 2)
					int tiledatasize = 8 << indexTypeNew;		// will be 8, 16 or 32 (from index values of 0, 1, 2)
					int indexshift = 3 - indexTypeNew;			// will be 3, 2 or 1 (from index values of 0, 1, 2)
					int indexmask_bits = 7 >> indexTypeNew;		// will be 7, 3 or 1 (from index values of 0, 1, 2)
					int indexmask_bytes = 62 >> indexshift;	// will be 7, 15 or 31 (from index values of 0, 1, 2)

					tile_types = pucData + usDataOffset;
					repacked = tile_types + tiletypecount;
					memset(tile_types, 255, tiletypecount);
					memset(repacked, 0,tiledatasize);
					newIndices[i] |= ( usDataOffset & INDEX_OFFSET_MASK) << INDEX_OFFSET_SHIFT;
					usDataOffset += tiletypecount + tiledatasize;
					int count = 0;
					for( int j = 0; j < 64; j++ )
					{
						int tile = unpacked_data[j];
						if( ucMappings[tile] == 255 )
						{
							ucMappings[tile] = count;
							tile_types[count++] = tile;
						}
						int idx = (j >> indexshift) & indexmask_bytes;
						int bit = ( j & indexmask_bits ) * bitspertile;
						repacked[idx] |= ucMappings[tile] << bit;
					}
				}
			}
		}

		newAllocatedSize = memToAlloc;

		DmaData_SPU::putAndWait(&newIndicesAndData, newIndicesPPU, DmaData_SPU::roundUpDMASize(newAllocatedSize));
		return true;
	}
	return false;
}




void cellSpursJobQueueMain(CellSpursJobContext2 *pContext, CellSpursJob256 *pJob)
{
// 	CellSpursTaskId idTask = cellSpursGetTaskId();
	unsigned int idSpu = cellSpursGetCurrentSpuId();

	if(sc_verbose)
		spu_print("CompressedTileStorage_compress [SPU#%u] start\n", idSpu);

	g_pSpursJobContext = pContext;

	uint32_t eaDataIn = pJob->workArea.userData[0];						
 
 	CompressedTileStorage_compress_dataIn compressDataIn;
 	DmaData_SPU::getAndWait(&compressDataIn, eaDataIn, sizeof(CompressedTileStorage_compress_dataIn));
	CCompressedTileStorage_compress c(compressDataIn);

	compressDataIn.neededCompressed = c.compress(compressDataIn.upgradeBlock);
	compressDataIn.newAllocatedSize = c.newAllocatedSize;

	DmaData_SPU::putAndWait(&compressDataIn, eaDataIn, sizeof(CompressedTileStorage_compress_dataIn));

	if(sc_verbose)
	 	spu_print("CompressedTileStorage_compress [SPU#%u] exit\n", idSpu);
}

