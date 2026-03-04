#pragma once

#include <stdint.h>


#ifndef SN_TARGET_PS3_SPU
#include "..\..\..\stdafx.h"
#include "..\..\..\..\Minecraft.World\Region.h"
#endif // SN_TARGET_PS3_SPU

//#include "xmcore.h"

// This class is used for the compressed storage of tile data. Unlike the SparseLightingStorage class, data is split into 512 blocks of 4x4x4 tiles. Then within each block, the
// data is compressed as described below, with a selection of bits per tile available, in a method similar to a palettised image.

// There are two elements to the storage... an index array (fixed size), and the actual storage required...

// The index:
// (1) Each index takes up 2 bytes, one for each 4x4x4 block ie 512 X 2 = 1024 bytes in total
// (2) The type of index is determined by the least significant 2 bits, the other 14 bits represent an offset for the data, stored divided by 2
//			0 - the data for this block is represented at 1 bit per tile. Data pointed to is 2 bytes describing the 2 possible tiles stored in this block, followed by 32 bytes of data (total 34 bytes)
//			1 - the data for this block is represented at 2 bit per tile. Data pointed to is 4 bytes describing the 4 possible tiles stored in this block, followed by 64 bytes of data (total 68 bytes)
//			2 - the data for this block is represented at 4 bit per tile. Data pointed to is 16 bytes describing the 16 possible tiles stored in this block, followed by 128 bytes of data (total 144 bytes)
//	        3 - if bit 2 is 0, then this block is represented at 8 bits per tile. Data pointed to is 64 bytes, offset must be a multiple of 4 (since bit 2 can also be thought of as being
//				the low bit of the offset (divided by 2 as in the other cases), and is zero)
//			  - if bit 2 is 1, then this block is represented at 0 bits per tile. The upper 8 bits of the index store the tile value that is used by the entire block.
// So:
//     oooooooooooooo00		- 1 bit per tile,  offset oooooooooooooo0
//     oooooooooooooo01     - 2 bits per tile, offset oooooooooooooo0
//     oooooooooooooo10     - 4 bits per tile, offset oooooooooooooo0
//     ooooooooooooo011     - 8 bits per tile, offset ooooooooooooo00
//     tttttttt-----111     - 0 bits per tile - tile is tttttttt

// Some notes on the logic of all of this...
// (1) Large numbers of blocks in the world really don't need to be stored at a full 8 bits per tile.
// In a worst-case scenario, all planes would be 256 bytes and we'd have to store offsets of up to 32704 ( 64 x 511). This would require 15 bits per offset to store, but since in all cases
// the data can be stored with a 2 byte alignment, we can store offsets divided by 2, freeing up 2 bits to store the type of index for each plane. This allows us to encode 4 types, but we really have
// 5 types (0, 1, 2, 4 or 8 bits per tile). Since the 8-bit per tile planes are likely to be very rare, we can free up an extra bit in those by making their offset 4-byte aligned, and
// then use the extra bit to determine whether its a 0 or 8-bit per tile index. In the 0 bit case, we can use the bits used for the offset to store the actual tile value represented throughout the plane.
// (2) The compression is done per 4x4x4 block rather than planes like the lighting, as that gives many more regions that have a small number of tile types than per plane, and can therefore
// be compressed using less bits per tile. This is at the expense of a larger index, and more overhead from storing the tile types in each block (since there are more blocks than planes). However
// on balance this still was found to give much better compression - around 12.5% vs 19% by doing things per plane.
// (3) Another compromise is being made on how the memory is allocated. This is all currently done with physical allocations to bypass the general heap manager, in particular to allow the freeing
// of memory to actually free whole memory pages cleanly rather than leaving them as managed by the heap manager. The downside to this is that all storage is done in whole 4K pages. Annoyingly,
// a lot of our compressed chunks are just on the edge of fitting in 4K, so an awful lot of them end up being 8K when they are just a small amount over. However, in testing absolutely no chunks
// were seen that got close to going over 8K compressed, so doing things this way then we at least know that we are reliably getting 25% compression, and freeing things up cleanly.
// Note: see the comments on the getIndex and getBlockAndTile for an explanation of how the blocks themselves are organised in terms of mapping a chunk-wide x/y/z into a block and tile index.

//#define BLOCK_COMPRESSION_STATS
class CompressedTileStorage_SPU
{
private:
	unsigned char	*indicesAndData;
public:
	int				allocatedSize;
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

	static const unsigned int MM_PHYSICAL_4KB_BASE = 0xE0000000;		// Start of where 4KB page sized physical allocations are made
public:
	CompressedTileStorage_SPU(unsigned char* data);
	~CompressedTileStorage_SPU();
private:
	inline static int getIndex(int block, int tile);
	inline static void getBlockAndTile(int *block, int *tile, int x, int y, int z);
public:
	unsigned char* getDataPtr() { return indicesAndData; }
    int  get(int x, int y, int z);							// Get an individual tile value
};

class TileCompressData_SPU
{
	static const int sc_size = 20;

public:
	int m_x0;
	int m_y0;
	int m_z0;
	void* m_lowerBlocks[9];
	int m_lowerBlocksSize[9];
	void* m_upperBlocks[9];
	int m_upperBlocksSize[9];

	int64_t			m_lowerSkyLight[9];							// Contains packed-together data pointer (lower 32-bits), and count of lines used (upper 32-bits)
	int64_t			m_lowerBlockLight[9];							// Contains packed-together data pointer (lower 32-bits), and count of lines used (upper 32-bits)
	int64_t			m_upperSkyLight[9];							// Contains packed-together data pointer (lower 32-bits), and count of lines used (upper 32-bits)
	int64_t			m_upperBlockLight[9];							// Contains packed-together data pointer (lower 32-bits), and count of lines used (upper 32-bits)

	int64_t			m_lowerData[9];							// Contains packed-together data pointer (lower 32-bits), and count of lines used (upper 32-bits)
	int64_t			m_upperData[9];							// Contains packed-together data pointer (lower 32-bits), and count of lines used (upper 32-bits)

#ifdef SN_TARGET_PS3_SPU
	void*			m_pRegion;

	void uncompress(uint32_t eaOutData);

	void uncompressTiles(int x0, int z0, int x1, int z1, bool upper);
	void clearTiles(int x0, int z0, int x1, int z1, bool upper);

	void uncompressLights(int x0, int z0, int x1, int z1, bool upper, bool skyLight);
	void clearLights(int x0, int z0, int x1, int z1, bool upper, bool skyLight);

	void uncompressData(int x0, int z0, int x1, int z1, bool upper);
	void clearData(int x0, int z0, int x1, int z1, bool upper);

	void dmaSparseStorage(int64_t dataAndSize, unsigned char* pDest);

	void loadAndUncompressLowerSection(int block, int x0, int z0, int x1, int z1);
	void loadAndUncompressUpperSection(int block, int x0, int z0, int x1, int z1);

	bool validY(int yVal, bool upper)
	{
		if(upper)
		{
			if(yVal >= 128)		return true;
		}
		else
		{
			if(yVal <= 127)		return true;
		}
		return false;
	}

	class OutputData
	{
	public:
		unsigned char	m_tileIds[16*16*256];			// byte
	};

	static OutputData m_OutputData;
	static CompressedTileStorage_SPU* m_pTileStorage;
#else
	Region*			m_pRegion;

	void setForChunk( Region* region, int x0, int y0, int z0 );
#endif // SN_TARGET_PS3_SPU

	int padding[3];

};
