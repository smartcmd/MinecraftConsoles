#include "stdafx.h"
#include "CompressedTileStorage_SPU.h"
#ifdef SN_TARGET_PS3_SPU
#include "../Common/DmaData.h"
#else
#include "../../../../Minecraft.World/Tile.h"
#include "../../../../Minecraft.World/TilePos.h"
#include "../../../../Minecraft.World/LevelChunk.h"
#endif

#include <stdlib.h>

#ifdef SN_TARGET_PS3_SPU
TileCompressData_SPU::OutputData TileCompressData_SPU::m_OutputData;
CompressedTileStorage_SPU* TileCompressData_SPU::m_pTileStorage;
SparseLightStorage_SPU* TileCompressData_SPU::m_pLightStorage;
SparseDataStorage_SPU* TileCompressData_SPU::m_pDataStorage;
#endif

// Note: See header for an overview of this class

// int CompressedTileStorage::deleteQueueIndex;
// XLockFreeStack <unsigned char> CompressedTileStorage::deleteQueue[3];
// 
// CRITICAL_SECTION CompressedTileStorage::cs_write;

CompressedTileStorage_SPU::CompressedTileStorage_SPU(unsigned char* data)
{
	indicesAndData = data;
	allocatedSize = 0;
}


CompressedTileStorage_SPU::~CompressedTileStorage_SPU()
{
}

// Get an index into the normal ordering of tiles for the java game, given a block index (0 to 511) and a tile index (0 to 63)
inline int CompressedTileStorage_SPU::getIndex(int block, int tile)
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

inline void CompressedTileStorage_SPU::getBlockAndTile(int *block, int *tile, int x, int y, int z)
{
	*block = ( ( x  & 0x0c ) << 5 ) | ( ( z & 0x0c ) << 3 ) | ( y >> 2 );
	*tile = ( ( x & 0x03 ) << 4 ) | ( ( z & 0x03 ) << 2 ) | ( y & 0x03 );
}


// Get an individual tile value
int  CompressedTileStorage_SPU::get(int x, int y, int z)
{
	if(y<0)
		return 0;
	if(!indicesAndData) return 0;

	unsigned short *blockIndices = (unsigned short *)indicesAndData;
	unsigned char *data = indicesAndData + 1024;

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

#ifdef SN_TARGET_PS3_SPU




void TileCompressData_SPU::uncompressTiles(int x0, int z0, int x1, int z1, bool upper)
{
	int y0 = -2;
	int y1 = 18;
	for(int iY=y0;iY<y1;iY++)
	{
		if(validY(iY+m_y0, upper))
		{
			for(int iX=x0;iX<x1;iX++)
			{
				for(int iZ=z0;iZ<z1;iZ++)
				{
					int index = ((iX+2)*sc_size*sc_size) + ((iY+2)*sc_size) + (iZ+2);
					int sectionX = (iX+m_x0) & 0xf;
					int sectionY = (iY+m_y0);
					if(upper)
						sectionY -= 128;
					int sectionZ = (iZ+m_z0) & 0xf;
					if((iY+m_y0)<0 || (iY+m_y0)>255)
					{
						m_OutputData.m_tileIds[index] = 0;
					}
					else
					{
						m_OutputData.m_tileIds[index] = m_pTileStorage->get(sectionX, sectionY, sectionZ);
					}
				}
			}
		}
	}
}


void TileCompressData_SPU::clearTiles(int x0, int z0, int x1, int z1, bool upper)
{
	int y0 = -2;
	int y1 = 18;
	for(int iY=y0;iY<y1;iY++)
	{
		if(validY((iY+m_y0), upper))
		{
			for(int iX=x0;iX<x1;iX++)
			{
				for(int iZ=z0;iZ<z1;iZ++)
				{
					int index = ((iX+2)*sc_size*sc_size) + ((iY+2)*sc_size) + (iZ+2);
					m_OutputData.m_tileIds[index] = 0;
				}
			}
		}
	}
}


void TileCompressData_SPU::uncompressLights(int x0, int z0, int x1, int z1, bool upper, bool skyLight)
{
	int y0 = -2;
	int y1 = 18;
	for(int iY=y0;iY<y1;iY++)
	{
		if(validY(iY+m_y0, upper))
		{
			for(int iX=x0;iX<x1;iX++)
			{
				for(int iZ=z0;iZ<z1;iZ++)
				{
					int index = ((iX+2)*sc_size*sc_size) + ((iY+2)*sc_size) + (iZ+2);
					int sectionX = (iX+m_x0) & 0xf;
					int sectionY = (iY+m_y0);
					if(upper)
						sectionY -= 128;
					int sectionZ = (iZ+m_z0) & 0xf;
					if((iY+m_y0)<0 || (iY+m_y0)>255)
					{
						m_OutputData.m_brightness[index] = 0;
					}
					else if(skyLight)
					{
						m_OutputData.m_brightness[index] = m_pLightStorage->get(sectionX, sectionY, sectionZ);
					}
					else
					{
						m_OutputData.m_brightness[index] |= m_pLightStorage->get(sectionX, sectionY, sectionZ) << 4;
					}
				}
			}
		}
	}
}

void TileCompressData_SPU::clearLights(int x0, int z0, int x1, int z1, bool upper, bool skyLight)
{
	int y0 = -2;
	int y1 = 18;
	for(int iY=y0;iY<y1;iY++)
	{
		if(validY((iY+m_y0), upper))
		{
			for(int iX=x0;iX<x1;iX++)
			{
				for(int iZ=z0;iZ<z1;iZ++)
				{
					int index = ((iX+2)*sc_size*sc_size) + ((iY+2)*sc_size) + (iZ+2);
					if(skyLight)
						m_OutputData.m_tileIds[index] &= 0xf0;
					else
						m_OutputData.m_tileIds[index] &= 0x0f;
				}
			}
		}
	}
}

void TileCompressData_SPU::uncompressData(int x0, int z0, int x1, int z1, bool upper)
{
	int y0 = -2;
	int y1 = 18;
	for(int iY=y0;iY<y1;iY++)
	{
		if(validY(iY+m_y0, upper))
		{
			for(int iX=x0;iX<x1;iX++)
			{
				for(int iZ=z0;iZ<z1;iZ++)
				{
					int index = ((iX+2)*sc_size*sc_size) + ((iY+2)*sc_size) + (iZ+2);
					int sectionX = (iX+m_x0) & 0xf;
					int sectionY = (iY+m_y0);
					if(upper)
						sectionY -= 128;
					int sectionZ = (iZ+m_z0) & 0xf;
					if((iY+m_y0)<0 || (iY+m_y0)>255)
					{
						m_OutputData.m_data_flags[index] = 0;
					}
					else
					{
						m_OutputData.m_data_flags[index] = m_pDataStorage->get(sectionX, sectionY, sectionZ);
					}
				}
			}
		}
	}
}

void TileCompressData_SPU::clearData(int x0, int z0, int x1, int z1, bool upper)
{
	int y0 = -2;
	int y1 = 18;
	for(int iY=y0;iY<y1;iY++)
	{
		if(validY((iY+m_y0), upper))
		{
			for(int iX=x0;iX<x1;iX++)
			{
				for(int iZ=z0;iZ<z1;iZ++)
				{
					int index = ((iX+2)*sc_size*sc_size) + ((iY+2)*sc_size) + (iZ+2);
					m_OutputData.m_data_flags[index] = 0;
				}
			}
		}
	}
}


void TileCompressData_SPU::dmaSparseStorage(int64_t dataAndSize, unsigned char* pDest)
{
	uint32_t loadFrom = (uint32_t)dataAndSize & 0x00000000ffffffffL;
	unsigned int loadSize = (uint32_t)((dataAndSize >> 48)*128)+128;
	DmaData_SPU::getAndWait(pDest, loadFrom, loadSize);
}

int padTo16(int size) 
{
	if(size & 0x0f)
	{
		size &= ~0x0f;
		size += 0x10;
	}
	return size;
}
void TileCompressData_SPU::loadAndUncompressLowerSection(int block, int x0, int z0, int x1, int z1)
{

	// tile IDs first
	// ---------------------------
	if(m_lowerBlocks[block] != nullptr)
	{
		int dmaSize = padTo16(m_lowerBlocksSize[block]);
		DmaData_SPU::getAndWait(m_pTileStorage->getDataPtr(), (uint32_t)m_lowerBlocks[block], dmaSize);
// 		spu_print("Grabbed %d of data\n", m_lowerBlocksSize[block]);
		uncompressTiles(x0, z0, x1, z1, false);
	}
	else
	{
		clearTiles(x0, z0, x1, z1, false);
	}

	// Sky Lights
	// ---------------------------
	if(m_lowerSkyLight[block] != 0)
	{
		dmaSparseStorage(m_lowerSkyLight[block], m_pLightStorage->getDataPtr());
		uncompressLights(x0, z0, x1, z1, false, true);
	}
	else
	{
		clearLights(x0, z0, x1, z1, false, true);
	}

	// Block Lights
	// ---------------------------
	if(m_lowerBlockLight[block] != 0)
	{
		dmaSparseStorage(m_lowerBlockLight[block],  m_pLightStorage->getDataPtr());
		uncompressLights(x0, z0, x1, z1, false, false);
	}
	else
	{
		clearLights(x0, z0, x1, z1, false, false);
	}

	// Data
	// ---------------------------
	if(m_lowerData[block] != 0)
	{
		dmaSparseStorage(m_lowerData[block],  m_pDataStorage->getDataPtr());
 		uncompressData(x0, z0, x1, z1, false);
	}
	else
	{
 		clearData(x0, z0, x1, z1, false);
	}
}

void TileCompressData_SPU::loadAndUncompressUpperSection(int block, int x0, int z0, int x1, int z1)
{
	if(m_upperBlocks[block] != nullptr)
	{
		int dmaSize = padTo16(m_upperBlocksSize[block]);
		DmaData_SPU::getAndWait(m_pTileStorage->getDataPtr(), (uint32_t)m_upperBlocks[block], dmaSize);
		uncompressTiles(x0, z0, x1, z1, true);
	}
	else
	{
		clearTiles(x0, z0, x1, z1, true);
	}

	// Sky Lights
	// ---------------------------
	if(m_upperSkyLight[block] != 0)
	{
		dmaSparseStorage(m_upperSkyLight[block],  m_pLightStorage->getDataPtr());
		uncompressLights(x0, z0, x1, z1, true, true);
	}
	else
	{
		clearLights(x0, z0, x1, z1, true, true);
	}

	// Block Lights
	// ---------------------------
	if(m_upperBlockLight[block] != 0)
	{
		dmaSparseStorage(m_upperBlockLight[block],  m_pLightStorage->getDataPtr());
		uncompressLights(x0, z0, x1, z1, true, false);
	}
	else
	{
		clearLights(x0, z0, x1, z1, true, false);
	}

	// Data
	// ---------------------------
	if(m_upperData[block] != 0)
	{
		dmaSparseStorage(m_upperData[block],  m_pDataStorage->getDataPtr());
 		uncompressData(x0, z0, x1, z1, true);
	}
	else
	{
 		clearData(x0, z0, x1, z1, true);
	}
}


void TileCompressData_SPU::uncompress( uint32_t eaDataOut )
{
	unsigned char pScratchArea[33*1024];
	int outDataSize = sc_size*sc_size*sc_size*3;
	CompressedTileStorage_SPU ts(pScratchArea);
	SparseLightStorage_SPU ls(pScratchArea);
	SparseDataStorage_SPU ds(pScratchArea);


	for(int i=0;i<sc_size*sc_size*sc_size;i++)
	{
		m_OutputData.m_tileIds[i] = 0;
		m_OutputData.m_brightness[i] = 0;
		m_OutputData.m_data_flags[i] = 0;
	}

	m_pTileStorage = &ts;
	m_pLightStorage = &ls;
	m_pDataStorage = &ds;

	// there may be an overlap here on the 128 boundary here we have to uncompressed upper and lower
	if(m_y0 <= 127+16)
	{
		loadAndUncompressLowerSection(0,	-2,-2,	0,0);
		loadAndUncompressLowerSection(1,	-2,0,	0,16);
		loadAndUncompressLowerSection(2,	-2,16,	0,18);

		loadAndUncompressLowerSection(3,	0,-2,	16,0);
		loadAndUncompressLowerSection(4,	0,0,	16,16);
		loadAndUncompressLowerSection(5,	0,16,	16,18);

		loadAndUncompressLowerSection(6,	16,-2,	18,0);
		loadAndUncompressLowerSection(7,	16,0,	18,16);
		loadAndUncompressLowerSection(8,	16,16,	18,18);
	}
	if(m_y0 >= 128-16)
	{
		loadAndUncompressUpperSection(0,	-2,-2,	0,0);
		loadAndUncompressUpperSection(1,	-2,0,	0,16);
		loadAndUncompressUpperSection(2,	-2,16,	0,18);

		loadAndUncompressUpperSection(3,	0,-2,	16,0);
		loadAndUncompressUpperSection(4,	0,0,	16,16);
		loadAndUncompressUpperSection(5,	0,16,	16,18);

		loadAndUncompressUpperSection(6,	16,-2,	18,0);
		loadAndUncompressUpperSection(7,	16,0,	18,16);
		loadAndUncompressUpperSection(8,	16,16,	18,18);
	}

// 	for(int i=0;i<20*20*20; i++)
// 	{
// 		m_OutputData.m_data_flags[i] = 0xEE;
// 		m_OutputData.m_data_flags[i] = 0xEE;
// 		m_OutputData.m_data_flags[i] = 0xEE;
// 
// 		if(m_OutputData.m_data_flags[i] == 32)
// 		{
// 			spu_print("Help! 32 in flags\n");
// 		}
// 	}
	DmaData_SPU::putAndWait(m_OutputData.m_tileIds, eaDataOut, outDataSize);
}
	
#else


void TileCompressData_SPU::setForChunk( Region* region, int x0, int y0, int z0 )
{
	m_x0 = x0;
	m_y0 = y0;
	m_z0 = z0;

	// we have to grab a 20x20x20 section, so we need 9 chunks in total, the centre chunk and all neighbours in x and z
	int offsets[3] = {-2, 0, 18};
	for(int i=0;i<3;i++)
	{
		for(int j=0; j<3;j++)
		{
			if(y0 <= 127+16)
			{
				LevelChunk* pLevelChunk = region->getLevelChunk(m_x0+offsets[i], 0, m_z0+offsets[j]);
				if(pLevelChunk && !pLevelChunk->isEmpty())
				{
					m_lowerBlocks[i*3+j] = pLevelChunk->lowerBlocks->indicesAndData;
					m_lowerBlocksSize[i*3+j] = pLevelChunk->lowerBlocks->allocatedSize;
					m_lowerSkyLight[i*3+j] = pLevelChunk->lowerSkyLight->dataAndCount;
					m_lowerBlockLight[i*3+j] = pLevelChunk->lowerBlockLight->dataAndCount;
					m_lowerData[i*3+j] = pLevelChunk->lowerData->dataAndCount;
				}
				else
				{
					m_lowerBlocks[i*3+j] = nullptr;
					m_lowerBlocksSize[i*3+j] = 0;
					m_lowerSkyLight[i*3+j] = 0;
					m_lowerBlockLight[i*3+j] = 0;
					m_lowerData[i*3+j] = 0;
				}
			}
			if(y0 >= 128-16)
			{
				LevelChunk* pLevelChunk = region->getLevelChunk(m_x0+offsets[i], 128, m_z0+offsets[j]);
				if(pLevelChunk && !pLevelChunk->isEmpty())
				{
					m_upperBlocks[i*3+j] = pLevelChunk->upperBlocks->indicesAndData;
					m_upperBlocksSize[i*3+j] = pLevelChunk->upperBlocks->allocatedSize;
					m_upperSkyLight[i*3+j] = pLevelChunk->upperSkyLight->dataAndCount;
					m_upperBlockLight[i*3+j] = pLevelChunk->upperBlockLight->dataAndCount;
					m_upperData[i*3+j] = pLevelChunk->upperData->dataAndCount;
			}
				else
				{
					m_upperBlocks[i*3+j] = nullptr;
					m_upperBlocksSize[i*3+j] = 0;
					m_upperSkyLight[i*3+j] = 0;
					m_upperBlockLight[i*3+j] = 0;
					m_upperData[i*3+j] = 0;
				}
			}

		}
	}
}



#endif