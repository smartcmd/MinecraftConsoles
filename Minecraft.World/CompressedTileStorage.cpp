#include "stdafx.h"
#include "CompressedTileStorage.h"

#ifdef __PSVITA__
#define PSVITA_PRECOMPUTED_TABLE
#endif

#ifdef __PS3__
#include "..\SPU_Tasks\CompressedTileStorage_compress\CompressedTileStorage_compress.h"
#include "C4JSpursJob.h"
static const int sc_maxCompressTiles = 64;
static CompressedTileStorage_compress_dataIn g_compressTileDataIn[sc_maxCompressTiles] __attribute__((__aligned__(16)));
static int g_currentCompressTiles = 0;
//#define DISABLE_SPU_CODE
#endif //__PS3__

// Note: See header for an overview of this class

int CompressedTileStorage::deleteQueueIndex;
XLockFreeStack <unsigned char> CompressedTileStorage::deleteQueue[3];

CRITICAL_SECTION CompressedTileStorage::cs_write;

#ifdef PSVITA_PRECOMPUTED_TABLE
// AP - this will create a precomputed table to speed up getData
static int *CompressedTile_StorageIndexTable = NULL;

void CompressedTileStorage_InitTable()
{
	if( CompressedTile_StorageIndexTable == NULL )
	{
		CompressedTile_StorageIndexTable = (int*) malloc(sizeof(int) * 64);
		for(int j = 0;j < 64;j += 1 )
		{
			int index = ( ( j & 0x30 ) << 7) | ( ( j & 0x0c ) << 5 ) |  ( j & 0x03 );
			CompressedTile_StorageIndexTable[j] = index;
		}
	}
}
#endif

CompressedTileStorage::CompressedTileStorage()
{
	indicesAndData = NULL;
	allocatedSize = 0;

#ifdef PSVITA_PRECOMPUTED_TABLE
	CompressedTileStorage_InitTable();
#endif
}

CompressedTileStorage::CompressedTileStorage(CompressedTileStorage *copyFrom)
{
	EnterCriticalSection(&cs_write);
	allocatedSize = copyFrom->allocatedSize;
	if(allocatedSize > 0)
	{
		indicesAndData = (unsigned char *)XPhysicalAlloc(allocatedSize, MAXULONG_PTR, 4096, PAGE_READWRITE);//(unsigned char *)malloc(allocatedSize);
		XMemCpy(indicesAndData, copyFrom->indicesAndData, allocatedSize);
	}
	else
	{
		indicesAndData = NULL;
	}
	LeaveCriticalSection(&cs_write);

#ifdef PSVITA_PRECOMPUTED_TABLE
	CompressedTileStorage_InitTable();
#endif
}

CompressedTileStorage::CompressedTileStorage(byteArray initFrom, unsigned int initOffset)
{
	indicesAndData = NULL;
	allocatedSize = 0;

	// We need 32768 bytes for a fully uncompressed chunk, plus 1024 for the index. Rounding up to nearest 4096 bytes for allocation
	indicesAndData = (unsigned char *)XPhysicalAlloc(32768+4096, MAXULONG_PTR, 4096, PAGE_READWRITE);

	unsigned short *indices = (unsigned short *)indicesAndData;
	unsigned char *data = indicesAndData + 1024;

	int offset = 0;
	for( int i = 0; i < 512; i++ )
	{
		indices[i] = INDEX_TYPE_0_OR_8_BIT | ( offset << 1 );

		if( initFrom.data )
		{
			for( int j = 0; j < 64; j++ )
			{
				*data++ = initFrom[getIndex(i,j) + initOffset];
			}
		}
		else
		{
			for( int j = 0; j < 64; j++ )
			{
				*data++ = 0;
			}
		}

		offset += 64;
	}

	allocatedSize = 32768 + 1024;		// This is used for copying (see previous ctor), and as such it only needs to be the actual size of the data used rather than the one rounded up to a page size actually allocated

#ifdef PSVITA_PRECOMPUTED_TABLE
	CompressedTileStorage_InitTable();
#endif
}

bool CompressedTileStorage::isCompressed()
{
	return allocatedSize != 32768 + 1024;
}

CompressedTileStorage::CompressedTileStorage(bool isEmpty)
{
	indicesAndData = NULL;
	allocatedSize = 0;

	// Empty and already compressed, so we only need 1K. Rounding up to nearest 4096 bytes for allocation
#ifdef __PS3__
	// XPhysicalAlloc just maps to malloc on PS3, so allocate the smallest amount
	indicesAndData = (unsigned char *)XPhysicalAlloc(1024, MAXULONG_PTR, 4096, PAGE_READWRITE);
#else
	indicesAndData = (unsigned char *)XPhysicalAlloc(4096, MAXULONG_PTR, 4096, PAGE_READWRITE);
#endif //__PS3__
	unsigned short *indices = (unsigned short *)indicesAndData;
	//unsigned char *data = indicesAndData + 1024;

	//int offset = 0;
	for( int i = 0; i < 512; i++ )
	{
		indices[i] = INDEX_TYPE_0_OR_8_BIT | INDEX_TYPE_0_BIT_FLAG;
	}

	allocatedSize = 1024;		// This is used for copying (see previous ctor), and as such it only needs to be the actual size of the data used rather than the one rounded up to a page size actually allocated

#ifdef PSVITA_PRECOMPUTED_TABLE
	CompressedTileStorage_InitTable();
#endif
}

bool CompressedTileStorage::isRenderChunkEmpty(int y)	// y == 0, 16, 32... 112 (representing a 16 byte range)
{
	int block;
	unsigned char *localIndicesAndData = indicesAndData;
	if(!localIndicesAndData) return true;
	unsigned short *blockIndices = (unsigned short *)localIndicesAndData;

	for( int x = 0; x < 16; x += 4 )
		for( int z = 0; z < 16; z += 4 )
	{
		getBlock(&block, x, y, z);
		uint64_t *comp = (uint64_t *)&blockIndices[block];
		// Are the 4 y regions stored here all zero? (INDEX_TYPE_0_OR_8_BIT | INDEX_TYPE_0_BIT_FLAG )
		if( ( *comp ) != 0x0007000700070007L ) return false;
	}
	return true;
}

bool CompressedTileStorage::isSameAs(CompressedTileStorage *other)
{
	EnterCriticalSection(&cs_write);
	if( allocatedSize != other->allocatedSize )
	{
		LeaveCriticalSection(&cs_write);
		return false;
	}

	// Attempt to compare as much as we can in 64-byte chunks (8 groups of 8 bytes)
	int quickCount = allocatedSize / 64;
	int64_t *pOld = (int64_t *)indicesAndData;
	int64_t *pNew = (int64_t *)other->indicesAndData;
	for( int i = 0; i < quickCount; i++ )
	{
		int64_t d0 = pOld[0] ^ pNew[0];
		int64_t d1 = pOld[1] ^ pNew[1];
		int64_t d2 = pOld[2] ^ pNew[2];
		int64_t d3 = pOld[3] ^ pNew[3];
		int64_t d4 = pOld[4] ^ pNew[4];
		int64_t d5 = pOld[5] ^ pNew[5];
		int64_t d6 = pOld[6] ^ pNew[6];
		int64_t d7 = pOld[7] ^ pNew[7];
		d0 |= d1;
		d2 |= d3;
		d4 |= d5;
		d6 |= d7;
		d0 |= d2;
		d4 |= d6;
		if( d0 | d4 )
		{
			LeaveCriticalSection(&cs_write);
			return false;
		}
		pOld += 8;
		pNew += 8;
	}

	// Now test anything remaining just byte at a time
	unsigned char *pucOld = (unsigned char *)pOld;
	unsigned char *pucNew = (unsigned char *)pNew;
	for( int i = 0; i < allocatedSize - (quickCount * 64); i++ )
	{
		if( *pucOld++ != *pucNew++ )
		{
			LeaveCriticalSection(&cs_write);
			return false;
		}
	}

	LeaveCriticalSection(&cs_write);
	return true;
}

CompressedTileStorage::~CompressedTileStorage()
{
#if 1
	if(indicesAndData) XPhysicalFree(indicesAndData);
#else
	if( (unsigned int)indicesAndData >= MM_PHYSICAL_4KB_BASE )
	{
		if(indicesAndData) XPhysicalFree(indicesAndData);
	}
	else
	{
		if(indicesAndData) free(indicesAndData);
	}
#endif
}

// Get an index into the normal ordering of tiles for the java game, given a block index (0 to 511) and a tile index (0 to 63)
inline int CompressedTileStorage::getIndex(int block, int tile)
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

// Get the block and tile (reversing getIndex above) for a given x, y, z coordinate
//
// bits for index into data is: xxxxzzzzyyyyyyy
//                              bbttbbttbbbbbtt
//
// so x is:						___________xxxx
// and maps to this bit of b	______bb_______
//         and this bit of t	_________tt____
//
// y is:						________yyyyyyy
// and maps to this bit of b	__________bbbbb
//         and this bit of t	_____________tt
//
// and z is:					___________zzzz
// and maps to this bit of b	________bb_____
//         and this bit of t    ___________tt__
//

inline void CompressedTileStorage::getBlockAndTile(int *block, int *tile, int x, int y, int z)
{
	*block = ( ( x  & 0x0c ) << 5 ) | ( ( z & 0x0c ) << 3 ) | ( y >> 2 );
	*tile = ( ( x & 0x03 ) << 4 ) | ( ( z & 0x03 ) << 2 ) | ( y & 0x03 );
}

inline void CompressedTileStorage::getBlock(int *block, int x, int y, int z)
{
	*block = ( ( x  & 0x0c ) << 5 ) | ( ( z & 0x0c ) << 3 ) | ( y >> 2 );
}

// Set all tile values from a data array of length 32768 (128 x 16 x 16).
void CompressedTileStorage::setData(byteArray dataIn, unsigned int inOffset)
{
	unsigned short _blockIndices[512];

	EnterCriticalSection(&cs_write);
	unsigned char *data = dataIn.data + inOffset;

	// Is the destination fully uncompressed? If so just write our data in - this happens when writing schematics and we don't want this setting of data to trigger compression
	if( allocatedSize == ( 32768 + 1024 ) )
	{
		//unsigned short *indices = (unsigned short *)indicesAndData;
		unsigned char *dataOut = indicesAndData + 1024;

		for( int i = 0; i < 512; i++ )
		{
			for( int j = 0; j < 64; j++ )
			{
				*dataOut++ = data[getIndex(i,j)];
			}
		}
		LeaveCriticalSection(&cs_write);
		return;
	}

	int offsets[512];
	int memToAlloc = 0;
//	static int type0 = 0, type1 = 0, type2 = 0, type4 = 0, type8 = 0, chunkTotal = 0;

	// Loop round all blocks
	for( int i = 0; i < 512; i++ )
	{
		offsets[i] = memToAlloc;
		// Count how many unique tile types are in the block - if unpacked_data isn't set then there isn't any data so we can't compress any further and require no storage.
		// Store flags for each tile type used in an array of 4 64-bit flags.

#ifdef __PSVITA__
		// AP - Vita isn't so great at shifting 64bits. The top biggest CPU time sink after profiling is __ashldi3 (64bit shift) at 3%
		// Let's use 32bit instead
		unsigned int usedFlags[8] = {0,0,0,0,0,0,0,0};
		__int32 i32_1 = 1;
		for( int j = 0; j < 64; j++ )			// This loop of 64 is to go round the 4 x 4 tiles in the block
		{
			int tile = data[getIndex(i,j)];
			if( tile < (64<<2) )
			{
				usedFlags[tile & 7] |= ( i32_1 << ( tile >> 3 ) );
			}
		}
		int count = 0;
		for( int tile = 0; tile < 256; tile++ )	// This loop of 256 is to go round the 256 possible values that the tiles might have had to find how many are actually used
		{
			if( usedFlags[tile & 7] & ( i32_1 << ( tile >> 3 ) ) )
			{
				count++;
			}
		}
#else
		uint64_t usedFlags[4] = {0,0,0,0};
		int64_t i64_1 = 1;	// MGH - instead of 1i64, which is MS specific
		for( int j = 0; j < 64; j++ )			// This loop of 64 is to go round the 4 x 4 tiles in the block
		{
			int tile = data[getIndex(i,j)];

			usedFlags[tile & 3] |= ( i64_1 << ( tile >> 2 ) );
		}
		int count = 0;
		for( int tile = 0; tile < 256; tile++ )	// This loop of 256 is to go round the 256 possible values that the tiles might have had to find how many are actually used
		{
			if( usedFlags[tile & 3] & ( i64_1 << ( tile >> 2 ) ) )
			{
				count++;
			}
		}
#endif
		if( count == 1 )
		{
			_blockIndices[i] = INDEX_TYPE_0_OR_8_BIT | INDEX_TYPE_0_BIT_FLAG;
//			type0++;
		}
		else if( count == 2 )
		{
			_blockIndices[i] = INDEX_TYPE_1_BIT;
			memToAlloc += 10;		// 8 bytes + 2 tile index
//			type1++;
		}
		else if ( count <= 4 )
		{
			_blockIndices[i] = INDEX_TYPE_2_BIT;
			memToAlloc += 20;		// 16 bytes + 4 tile index
//			type2++;
		}
		else if ( count <= 16 )
		{
			_blockIndices[i] = INDEX_TYPE_4_BIT;
			memToAlloc += 48;		// 32 bytes + 16 tile index
//			type4++;
		}
		else
		{
			_blockIndices[i] = INDEX_TYPE_0_OR_8_BIT;
			memToAlloc = ( memToAlloc + 3 ) & 0xfffc;		// Make sure we are 4-byte aligned for 8-bit storage
			memToAlloc += 64;
//			type8++;
		}
	}

//	chunkTotal++;
//	printf("%d: %d (0) %d (1) %d (2) %d (4) %d (8)\n", chunkTotal, type0 / chunkTotal, type1 / chunkTotal, type2 / chunkTotal, type4 / chunkTotal, type8 / chunkTotal);

	memToAlloc += 1024; // For the indices
	unsigned char *newIndicesAndData = (unsigned char *)XPhysicalAlloc(memToAlloc, MAXULONG_PTR, 4096, PAGE_READWRITE);//(unsigned char *)malloc( memToAlloc );
	unsigned char *pucData = newIndicesAndData + 1024;
	unsigned short usDataOffset = 0;
	unsigned short *newIndices = (unsigned short *) newIndicesAndData;

	// Now pass through again actually making the final compressed data
	for( int i = 0; i < 512; i++ )
	{
		unsigned short indexTypeNew = _blockIndices[i] & INDEX_TYPE_MASK;
		newIndices[i] = indexTypeNew;

		if( indexTypeNew == INDEX_TYPE_0_OR_8_BIT )
		{
			if( _blockIndices[i] & INDEX_TYPE_0_BIT_FLAG )
			{
				newIndices[i] = INDEX_TYPE_0_OR_8_BIT | INDEX_TYPE_0_BIT_FLAG | (((unsigned short)data[getIndex(i,0)]) << INDEX_TILE_SHIFT);
			}
			else
			{
				usDataOffset = (usDataOffset + 3 ) & 0xfffc;
				for( int j = 0; j < 64; j++ ) // This loop of 64 is to go round the 4 x 4 x 4 tiles in the block
				{
					pucData[usDataOffset + j] = data[getIndex(i,j)];
				}
				newIndices[i] |= ( usDataOffset & INDEX_OFFSET_MASK) << INDEX_OFFSET_SHIFT;
				usDataOffset += 64;
			}
		}
		else
		{
			// Need to repack - TODO - from here onwards!
			unsigned char ucMappings[256] = {0};
			for( int j = 0; j < 256; j++ )
			{
				ucMappings[j] = 255;
			}

			unsigned char *repacked = NULL;

			int bitspertile = 1 << indexTypeNew;		// will be 1, 2 or 4 (from index values of 0, 1, 2)
			int tiletypecount = 1 << bitspertile;		// will be 2, 4 or 16 (from index values of 0, 1, 2)
			//int tiletypemask = tiletypecount - 1;		// will be 1, 3 or 15 (from index values of 0, 1, 2)
			int tiledatasize = 8 << indexTypeNew;		// will be 8, 16 or 32 (from index values of 0, 1, 2)
			int indexshift = 3 - indexTypeNew;			// will be 3, 2 or 1 (from index values of 0, 1, 2)
			int indexmask_bits = 7 >> indexTypeNew;		// will be 7, 3 or 1 (from index values of 0, 1, 2)
			int indexmask_bytes = 62 >> indexshift;	    // will be 7, 15 or 31 (from index values of 0, 1, 2)

			unsigned char *tile_types = pucData + usDataOffset;
			repacked = tile_types + tiletypecount;
			XMemSet(tile_types, 255, tiletypecount);
			XMemSet(repacked, 0,tiledatasize);
			newIndices[i] |= ( usDataOffset & INDEX_OFFSET_MASK) << INDEX_OFFSET_SHIFT;
			usDataOffset += tiletypecount + tiledatasize;
			int count = 0;
			for( int j = 0; j < 64; j++ )
			{
				int tile = data[getIndex(i,j)];
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

	if( indicesAndData )
	{
		queueForDelete( indicesAndData );
	}
	indicesAndData = newIndicesAndData;
	allocatedSize = memToAlloc;
	LeaveCriticalSection(&cs_write);
}

#ifdef PSVITA_PRECOMPUTED_TABLE

// AP - When called in pairs from LevelChunk::getBlockData this version of getData reduces the time from ~5.2ms to ~1.6ms on the Vita
// Gets all tile values into an array of length 32768.
void CompressedTileStorage::getData(byteArray retArray, unsigned int retOffset)
{
	unsigned short *blockIndices = (unsigned short *)indicesAndData;
	unsigned char *data = indicesAndData + 1024;

	int k = 0;
	unsigned char *Array = &retArray.data[retOffset];
	int *Table = CompressedTile_StorageIndexTable;
	for( int i = 0; i < 512; i++ )
	{
		int indexType = blockIndices[i] & INDEX_TYPE_MASK;

		int index = ( ( i & 0x180) << 6 ) | ( ( i & 0x060 ) << 4 ) | ( ( i & 0x01f ) << 2 );
		unsigned char *NewArray = &Array[index];

		if( indexType == INDEX_TYPE_0_OR_8_BIT )
		{
			if( blockIndices[i] & INDEX_TYPE_0_BIT_FLAG )
			{
				unsigned char val = ( blockIndices[i] >> INDEX_TILE_SHIFT ) & INDEX_TILE_MASK;
				for( int j = 0; j < 64; j++ )
				{
					NewArray[Table[j]] = val;
				}
			}
			else
			{
				// 8-bit reads are just directly read from the 64 long array of values stored for the block
				unsigned char *packed = data + ( ( blockIndices[i] >> INDEX_OFFSET_SHIFT ) & INDEX_OFFSET_MASK );

				for( int j = 0; j < 64; j++ )
				{
					NewArray[Table[j]] = packed[j];
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
				int bit = ( j & indexmask_bits ) << indexType;
				NewArray[Table[j]] = tile_types[( packed[idx] >> bit ) & tiletypemask];
			}
		}
	}
}

#else

// Gets all tile values into an array of length 32768.
void CompressedTileStorage::getData(byteArray retArray, unsigned int retOffset)
{
	// Snapshot pointer to avoid race with compress() swapping indicesAndData
	unsigned char *localIndicesAndData = indicesAndData;
	if(!localIndicesAndData) return;

	unsigned short *blockIndices = (unsigned short *)localIndicesAndData;
	unsigned char *data = localIndicesAndData + 1024;

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

#endif

// Get an individual tile value
int  CompressedTileStorage::get(int x, int y, int z)
{
	// Take a local snapshot of indicesAndData to avoid a race with compress() which swaps the pointer.
	// Both blockIndices and data must reference the same buffer, otherwise index offsets won't match.
	unsigned char *localIndicesAndData = indicesAndData;
	if(!localIndicesAndData) return 0;

	unsigned short *blockIndices = (unsigned short *)localIndicesAndData;
	unsigned char *data = localIndicesAndData + 1024;

	int block, tile;
	getBlockAndTile( &block, &tile, x, y, z );
	int indexType = blockIndices[block] & INDEX_TYPE_MASK;

	if( indexType == INDEX_TYPE_0_OR_8_BIT )
	{
		if( blockIndices[block] & INDEX_TYPE_0_BIT_FLAG )
		{
			// 0 bit reads are easy - the value is packed in the index
			return ( blockIndices[block] >> INDEX_TILE_SHIFT ) & INDEX_TILE_MASK;
		}
		else
		{
			// 8-bit reads are just directly read from the 64 long array of values stored for the block
			unsigned char *packed = data + ( ( blockIndices[block] >> INDEX_OFFSET_SHIFT ) & INDEX_OFFSET_MASK );
			return packed[tile];
		}
	}
	else
	{
		int bitspertile = 1 << indexType;			// will be 1, 2 or 4 (from index values of 0, 1, 2)
		int tiletypecount = 1 << bitspertile;		// will be 2, 4 or 16 (from index values of 0, 1, 2)
		int tiletypemask = tiletypecount - 1;		// will be 1, 3 or 15 (from index values of 0, 1, 2)
		int indexshift = 3 - indexType;				// will be 3, 2 or 1 (from index values of 0, 1, 2)
		int indexmask_bits = 7 >> indexType;		// will be 7, 3 or 1 (from index values of 0, 1, 2)
		int indexmask_bytes = 62 >> indexshift;		// will be 7, 15 or 31 (from index values of 0, 1, 2)

		unsigned char *tile_types = data + ( ( blockIndices[block] >> INDEX_OFFSET_SHIFT ) & INDEX_OFFSET_MASK );
		unsigned char *packed = tile_types + tiletypecount;
		int idx = ( tile >> indexshift ) & indexmask_bytes;
		int bit = ( tile & indexmask_bits ) * bitspertile;
		return tile_types[( packed[idx] >> bit ) & tiletypemask];
	}
	return 0;
}

// Set an individual tile value
void CompressedTileStorage::set(int x, int y, int z, int val)
{
	EnterCriticalSection(&cs_write);
	assert(val !=255 );
	int block, tile;
	getBlockAndTile( &block, &tile, x, y, z );

	// 2 passes - first pass will try and store within the current levels of compression, then if that fails will
	// upgrade the block we are writing to (so more bits can be stored) to achieve the storage required
	for( int pass = 0; pass < 2; pass++ )
	{
		unsigned short *blockIndices = (unsigned short *)indicesAndData;
		unsigned char *data = indicesAndData + 1024;

		int indexType = blockIndices[block] & INDEX_TYPE_MASK;

		if( indexType == INDEX_TYPE_0_OR_8_BIT )
		{
			if( blockIndices[block] & INDEX_TYPE_0_BIT_FLAG )
			{
				// 0 bits - if its the value already, we're done, otherwise continue on to upgrade storage
				if ( val == ( ( blockIndices[block] >> INDEX_TILE_SHIFT ) & INDEX_TILE_MASK ) )
				{
					LeaveCriticalSection(&cs_write);
					return;
				}
			}
			else
			{
				// 8 bits - just store directly and we're done
				unsigned char *packed = data + ( ( blockIndices[block] >> INDEX_OFFSET_SHIFT ) & INDEX_OFFSET_MASK );
				packed[ tile ] = val;
				LeaveCriticalSection(&cs_write);
				return;
			}
		}
		else
		{
			int bitspertile = 1 << indexType;			// will be 1, 2 or 4 (from index values of 0, 1, 2)
			int tiletypecount = 1 << bitspertile;		// will be 2, 4 or 16 (from index values of 0, 1, 2)
			int tiletypemask = tiletypecount - 1;		// will be 1, 3 or 15 (from index values of 0, 1, 2)
			int indexshift = 3 - indexType;				// will be 3, 2 or 1 (from index values of 0, 1, 2)
			int indexmask_bits = 7 >> indexType;		// will be 7, 3 or 1 (from index values of 0, 1, 2)
			int indexmask_bytes = 62 >> indexshift;		// will be 7, 15 or 31 (from index values of 0, 1, 2)

			unsigned char *tile_types = data + ( ( blockIndices[block] >> INDEX_OFFSET_SHIFT ) & INDEX_OFFSET_MASK );

			for( int i = 0; i < tiletypecount; i++ )
			{
				if( ( tile_types[i] == val ) || ( tile_types[i] == 255 ) )
				{
					tile_types[i] = val;
					unsigned char *packed = tile_types + tiletypecount;
					int idx = ( tile >> indexshift ) & indexmask_bytes;
					int bit = ( tile & indexmask_bits ) * bitspertile;
					packed[idx] &= ~( tiletypemask << bit );
					packed[idx] |= i << bit;
					LeaveCriticalSection(&cs_write);
					return;
				}
			}
		}
		if( pass == 0 )
		{
			compress(block);
		}
	};
	LeaveCriticalSection(&cs_write);
}

// Sets a region of tile values with the data at offset position in the array dataIn - external ordering compatible with java DataLayer
int  CompressedTileStorage::setDataRegion(byteArray dataIn, int x0, int y0, int z0, int x1, int y1, int z1, int offset, tileUpdatedCallback callback, void *param, int yparam)
{
	unsigned char *pucIn = &dataIn.data[offset];

	if( callback )
	{
		for( int x = x0; x < x1; x++ )
		{
			for( int z = z0; z < z1; z++ )
			{
				for( int y = y0; y < y1; y++ )
				{
					if(get(x, y, z) != *pucIn)
					{
						set(x, y, z, *pucIn);
						callback(x, y, z, param, yparam);
					}
					pucIn++;
				}
			}
		}
	}
	else
	{
		for( int x = x0; x < x1; x++ )
		{
			for( int z = z0; z < z1; z++ )
			{
				for( int y = y0; y < y1; y++ )
				{
					set(x, y, z, *pucIn++);
				}
			}
		}
	}
	ptrdiff_t count = pucIn - &dataIn.data[offset];

	return (int)count;
}

// Tests whether setting data would actually change anything
bool  CompressedTileStorage::testSetDataRegion(byteArray dataIn, int x0, int y0, int z0, int x1, int y1, int z1, int offset)
{
	unsigned char *pucIn = &dataIn.data[offset];
	for( int x = x0; x < x1; x++ )
	{
		for( int z = z0; z < z1; z++ )
		{
			for( int y = y0; y < y1; y++ )
			{
				if(get(x, y, z) != *pucIn++)
				{
					return true;
				}
			}
		}
	}
	return false;
}

// Updates the data at offset position dataInOut with a region of tile information - external ordering compatible with java DataLayer
int  CompressedTileStorage::getDataRegion(byteArray dataInOut, int x0, int y0, int z0, int x1, int y1, int z1, int offset)
{
	unsigned char *pucOut = &dataInOut.data[offset];
	for( int x = x0; x < x1; x++ )
	{
		for( int z = z0; z < z1; z++ )
		{
			for( int y = y0; y < y1; y++ )
			{
				*pucOut++ = get(x, y, z);
			}
		}
	}
	ptrdiff_t count = pucOut - &dataInOut.data[offset];

	return (int)count;
}

void CompressedTileStorage::staticCtor()
{
	InitializeCriticalSectionAndSpinCount(&cs_write, 5120);
	for( int i = 0; i < 3; i++ )
	{
		deleteQueue[i].Initialize();
	}
}

void CompressedTileStorage::queueForDelete(unsigned char *data)
{
	// Add this into a queue for deleting. This shouldn't be actually deleted until tick has been called twice from when
	// the data went into the queue.
	if( data )
	{
		deleteQueue[deleteQueueIndex].Push( data );
	}
}

void CompressedTileStorage::tick()
{
	// We have 3 queues for deleting. Always delete from the next one after where we are writing to, so it should take 2 ticks
	// before we ever delete something, from when the request to delete it came in
	int freeIndex = ( deleteQueueIndex + 1 ) % 3;

//	printf("Free queue: %d, %d\n",deleteQueue[freeIndex].GetEntryCount(),deleteQueue[freeIndex].GetAllocated());
	unsigned char *toFree = NULL;
	do
	{
		toFree = deleteQueue[freeIndex].Pop();
//		if( toFree ) printf("Deleting 0x%x\n", toFree);
#if 1
		if( toFree ) XPhysicalFree(toFree);
#else
		// Determine correct means to free this data - could have been allocated either with XPhysicalAlloc or malloc
		if( (unsigned int)toFree >= MM_PHYSICAL_4KB_BASE )
		{
			XPhysicalFree(toFree);
		}
		else
		{
			free(toFree);
		}
#endif
	} while( toFree );

	deleteQueueIndex = ( deleteQueueIndex + 1 ) % 3;
}

#ifdef __PS3__
void  CompressedTileStorage::compress_SPU(int upgradeBlock/*=-1*/)
{
	EnterCriticalSection(&cs_write);
	static unsigned char compBuffer[32768+4096]  __attribute__((__aligned__(16)));
	CompressedTileStorage_compress_dataIn& dataIn = g_compressTileDataIn[0];
	dataIn.allocatedSize = allocatedSize;
	dataIn.indicesAndData = indicesAndData;
	dataIn.newIndicesAndData = compBuffer;
	dataIn.upgradeBlock = upgradeBlock;

	static C4JSpursJobQueue::Port p("CompressedTileStorage::compress_SPU");
	C4JSpursJob_CompressedTileStorage_compress compressJob(&dataIn);
	p.submitJob(&compressJob);
	p.waitForCompletion();

	if(dataIn.neededCompressed)
	{
		unsigned char *newIndicesAndData = (unsigned char *)XPhysicalAlloc(dataIn.newAllocatedSize, MAXULONG_PTR, 4096, PAGE_READWRITE);//(unsigned char *)malloc( memToAlloc );
		memcpy(newIndicesAndData, compBuffer, dataIn.newAllocatedSize);
		queueForDelete( indicesAndData );
		indicesAndData = newIndicesAndData;
		allocatedSize = dataIn.newAllocatedSize;
	}

	LeaveCriticalSection(&cs_write);

}
#endif

// Compresses the data currently stored in one of two ways:
// (1) Attempt to compresses every block as much as possible (if upgradeBlock is -1)
// (2) Copy all blocks as-is apart from the block specified by upgradeBlock ( if > -1 ), which is changed to be the next-most-accomodating storage from its current state
void  CompressedTileStorage::compress(int upgradeBlock/*=-1*/)
{
#if defined __PS3__ && !defined DISABLE_SPU_CODE
	compress_SPU(upgradeBlock);
		return;
#endif

	unsigned char tempdata[64];
	unsigned short _blockIndices[512];

	// If this is already fully compressed, early out
	if( ( allocatedSize == 1024 ) && ( upgradeBlock == -1 ) ) return;

	bool needsCompressed = ( upgradeBlock > -1 );	// If an upgrade block is specified, we'll always need to recompress - otherwise default to false

	EnterCriticalSection(&cs_write);

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

#ifdef __PSVITA__
				// AP - Vita isn't so great at shifting 64bits. The top biggest CPU time sink after profiling is __ashldi3 (64bit shift) at 3%
				// lets use 32bit values instead
				unsigned int usedFlags[8] = {0,0,0,0,0,0,0,0};
				__int32 i32_1 = 1;
				for( int j = 0; j < 64; j++ )			// This loop of 64 is to go round the 4x4x4 tiles in the block
				{
					int tiletype = unpacked_data[j];
					usedFlags[tiletype & 7] |= ( i32_1 << ( tiletype >> 3 ) );
				}
				// count the number of bits set using the 'Hammering Weight' method. This reduces ::compress total thread cpu consumption from 10% to 4%
				unsigned int count = 0;
				for( int Index = 0;Index < 8;Index += 1 )
				{
					unsigned int i = usedFlags[Index];
					i = i - ((i >> 1) & 0x55555555);
					i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
					count += (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
				}
#else

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
#endif

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
		unsigned char *newIndicesAndData = (unsigned char *)XPhysicalAlloc(memToAlloc, MAXULONG_PTR, 4096, PAGE_READWRITE);//(unsigned char *)malloc( memToAlloc );
		if( newIndicesAndData == NULL )
		{
			DWORD lastError = GetLastError();
#ifndef _DURANGO
			MEMORYSTATUS memStatus;
			GlobalMemoryStatus(&memStatus);
			__debugbreak();
#endif
		}
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
							XMemCpy( pucData + usDataOffset, packed_data, 64 );
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
					XMemCpy( pucData + usDataOffset, packed_data, dataSize );
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
						XMemSet( tempdata, value, 64 );
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
#ifdef __PSVITA__
				memset(ucMappings, 255, 256);
#else
				for( int j = 0; j < 256; j++ )
				{
					ucMappings[j] = 255;
				}
#endif

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
						XMemCpy( pucData + usDataOffset, unpacked_data, 64 );
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
					XMemSet(tile_types, 255, tiletypecount);
					XMemSet(repacked, 0,tiledatasize);
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

		queueForDelete( indicesAndData );
		indicesAndData = newIndicesAndData;
		allocatedSize = memToAlloc;
	}
	LeaveCriticalSection(&cs_write);
}

int CompressedTileStorage::getAllocatedSize(int *count0, int *count1, int *count2, int *count4, int *count8)
{
	*count0 = 0;
	*count1 = 0;
	*count2 = 0;
	*count4 = 0;
	*count8 = 0;

	// Volatile read: compress() can swap indicesAndData concurrently outside cs_write
	unsigned char *localIndicesAndData = *(unsigned char *volatile *)&indicesAndData;
	if(!localIndicesAndData) return 0;
	unsigned short *blockIndices = (unsigned short *)localIndicesAndData;
	for(int i = 0; i < 512; i++ )
	{
		unsigned short idxType = blockIndices[i] & INDEX_TYPE_MASK;
		if( idxType == INDEX_TYPE_1_BIT )
		{
			(*count1)++;
		}
		else if( idxType == INDEX_TYPE_2_BIT )
		{
			(*count2)++;
		}
		else if( idxType == INDEX_TYPE_4_BIT )
		{
			(*count4)++;
		}
		else if( idxType == INDEX_TYPE_0_OR_8_BIT )
		{
			if( blockIndices[i] & INDEX_TYPE_0_BIT_FLAG )
			{
				(*count0)++;
			}
			else
			{
				(*count8)++;
			}
		}
	}
	return allocatedSize;
}

int CompressedTileStorage::getHighestNonEmptyY()
{
	unsigned char *localIndicesAndData = indicesAndData;
	if(!localIndicesAndData) return -1;
	unsigned short *blockIndices = (unsigned short *)localIndicesAndData;
	unsigned int highestYBlock = 0;
	bool found = false;

	// The 512 "blocks" (4x4x4 tiles) are arranged in 32 layers
	for(int yBlock = 31; yBlock >= 0; --yBlock)
	{
		// Each layer has 16 blocks
		for(unsigned int xzBlock = 0; xzBlock < 16; ++xzBlock)
		{
			// Blocks are ordered in columns
			int index = yBlock + (xzBlock * 32);

			int indexType = blockIndices[index] & INDEX_TYPE_MASK;
			if( indexType == INDEX_TYPE_0_OR_8_BIT && blockIndices[index] & INDEX_TYPE_0_BIT_FLAG )
			{
				int val = ( blockIndices[index] >> INDEX_TILE_SHIFT ) & INDEX_TILE_MASK;
				if(val != 0)
				{
					highestYBlock = yBlock;
					found = true;
					break;
				}
			}
			else
			{
				highestYBlock = yBlock;
				found = true;
				break;
			}
		}

		if(found) break;
	}

	int highestNonEmptyY = -1;
	if(found)
	{
		// Multiply by the number of vertical tiles in a block, and then add that again to be at the top of the block
		highestNonEmptyY = (highestYBlock * 4) + 4;
	}
	else
	{
		app.DebugPrintf("[CTS-WARN] getHighestNonEmptyY() returned -1! allocatedSize=%d indicesAndData=%p\n",
			allocatedSize, indicesAndData);
	}
	return highestNonEmptyY;
}

void CompressedTileStorage::write(DataOutputStream *dos)
{
	dos->writeInt(allocatedSize);
	// Volatile read: compress() can swap indicesAndData concurrently outside cs_write
	unsigned char *localIndicesAndData = *(unsigned char *volatile *)&indicesAndData;
	if(localIndicesAndData)
	{
		if(LOCALSYTEM_ENDIAN == BIGENDIAN)
		{
			// The first 1024 bytes are an array of shorts, so we need to reverse the endianness
			byteArray indicesCopy(1024);
			memcpy(indicesCopy.data, localIndicesAndData, 1024);
			reverseIndices(indicesCopy.data);
			dos->write(indicesCopy);
			delete [] indicesCopy.data;

			// Write the rest of the data
			if(allocatedSize > 1024)
			{
				byteArray dataWrapper(localIndicesAndData + 1024, allocatedSize - 1024);
				dos->write(dataWrapper);
			}
		}
		else
		{
			byteArray wrapper(localIndicesAndData, allocatedSize);
			dos->write(wrapper);
		}
	}
}

void CompressedTileStorage::read(DataInputStream *dis)
{
	allocatedSize = dis->readInt();
	if(allocatedSize > 0)
	{
		// This delete should be safe to do in a non-thread safe way as the chunk is fully read before any external reference is available to it from another thread
		if( indicesAndData )
		{
			XPhysicalFree(indicesAndData);
		}
		indicesAndData = (unsigned char *)XPhysicalAlloc(allocatedSize, MAXULONG_PTR, 4096, PAGE_READWRITE);

		byteArray wrapper(indicesAndData, allocatedSize);
		dis->readFully(wrapper);
		if(LOCALSYTEM_ENDIAN == BIGENDIAN)
		{
			reverseIndices(indicesAndData);
		}

		compress();
	}
}

void CompressedTileStorage::reverseIndices(unsigned char *indices)
{
	unsigned short *blockIndices = (unsigned short *)indices;
	for( int i = 0; i < 512; i++ )
	{
		System::ReverseUSHORT(&blockIndices[i]);
	}
}