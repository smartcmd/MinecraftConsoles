#pragma once
#include "Tile_SPU.h"
#include "Direction_SPU.h"

class VineTile_SPU : public Tile_SPU
{
public:
	static const int VINE_SOUTH = 1 << Direction::SOUTH;
	static const int VINE_NORTH = 1 << Direction::NORTH;
	static const int VINE_EAST = 1 << Direction::EAST;
	static const int VINE_WEST = 1 << Direction::WEST;

public:
	VineTile_SPU(int id) : Tile_SPU(id) {} 
    virtual void updateDefaultShape() { setShape(0, 0, 0, 1, 1, 1); }
    virtual int getRenderShape() { return SHAPE_VINE; }
    virtual bool isSolidRender(bool isServerLevel = false) { return false; }
    virtual bool isCubeShaped() { return false; }

	float _max(float a, float b)	{		return a > b ? a : b;	}
	float _min(float a, float b)	{		return a < b ? a : b;	}
	bool isAcceptableNeighbor(int id)
	{
		if (id == 0) return false;
		TileRef_SPU tile(id);
		if (tile->isCubeShaped() && tile->getMaterial()->blocksMotion()) return true;
		return false;
	}

    virtual void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr)	// 4J added forceData, forceEntity param
	{
		const float thickness = 1.0f / 16.0f;

		int facings = level->getData(x, y, z);

		float minX = 1;
		float minY = 1;
		float minZ = 1;
		float maxX = 0;
		float maxY = 0;
		float maxZ = 0;
		bool hasWall = facings > 0;

		if ((facings & VINE_WEST) != 0)
		{
			maxX = _max(maxX, thickness);
			minX = 0;
			minY = 0;
			maxY = 1;
			minZ = 0;
			maxZ = 1;
			hasWall = true;
		}
		if ((facings & VINE_EAST) != 0)
		{
			minX = _min(minX, 1 - thickness);
			maxX = 1;
			minY = 0;
			maxY = 1;
			minZ = 0;
			maxZ = 1;
			hasWall = true;
		}
		if ((facings & VINE_NORTH) != 0)
		{
			maxZ = _max(maxZ, thickness);
			minZ = 0;
			minX = 0;
			maxX = 1;
			minY = 0;
			maxY = 1;
			hasWall = true;
		}
		if ((facings & VINE_SOUTH) != 0)
		{
			minZ = _min(minZ, 1 - thickness);
			maxZ = 1;
			minX = 0;
			maxX = 1;
			minY = 0;
			maxY = 1;
			hasWall = true;
		}
		if (!hasWall && isAcceptableNeighbor(level->getTile(x, y + 1, z)))
		{
			minY = _min(minY, 1 - thickness);
			maxY = 1;
			minX = 0;
			maxX = 1;
			minZ = 0;
			maxZ = 1;
		}
		setShape(minX, minY, minZ, maxX, maxY, maxZ);
	}

public:
	virtual int getColor(ChunkRebuildData *level, int x, int y, int z, int data) { 	return getColor(level, x, y, z); }	// 4J added
	virtual int getColor(ChunkRebuildData *level, int x, int y, int z){ return level->getFoliageColor(x, z); } 
};