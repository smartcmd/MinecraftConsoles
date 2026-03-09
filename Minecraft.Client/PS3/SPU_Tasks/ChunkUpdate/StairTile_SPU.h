#pragma once

#include "Tile_SPU.h"

class StairTile_SPU : public Tile_SPU
{
public:
	static const int UPSIDEDOWN_BIT = 4;

	// the direction is the way going up (for normal non-upsidedown stairs)
	static const int DIR_EAST = 0;
	static const int DIR_WEST = 1;
	static const int DIR_SOUTH = 2;
	static const int DIR_NORTH = 3;

public:
	StairTile_SPU(int id) : Tile_SPU(id) {}
	void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr);	// 4J added forceData, forceEntity param
	bool isSolidRender(bool isServerLevel = false);
	int getRenderShape();
	void setBaseShape(ChunkRebuildData *level, int x, int y, int z);
	static bool isStairs(int id);

private:
	bool isLockAttached(ChunkRebuildData *level, int x, int y, int z, int data);

public:
	bool setStepShape(ChunkRebuildData *level, int x, int y, int z);
	bool setInnerPieceShape(ChunkRebuildData *level, int x, int y, int z);
};