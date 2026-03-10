#pragma once
#include "Tile_SPU.h"

class FenceGateTile_SPU : public Tile_SPU
{
private:
	static const int DIRECTION_MASK = 3;
    static const int OPEN_BIT = 4;

public:
	FenceGateTile_SPU(int id) : Tile_SPU(id) {}
	Icon_SPU *getTexture(int face, int data) { 	return TileRef_SPU(wood_Id)->getTexture(face); }
	static int getDirection(int data)	{ return (data & DIRECTION_MASK); }

	virtual void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr) // 4J added forceData, forceEntity param // Brought forward from 1.2.3
	{
		int data = getDirection(level->getData(x, y, z));
		if (data == Direction::NORTH || data == Direction::SOUTH)
		{
			setShape(0, 0, 6.0f / 16.0f, 1, 1.0f, 10.0f / 16.0f);
		}
		else
		{
			setShape(6.0f / 16.0f, 0, 0, 10.0f / 16.0f, 1.0f, 1);
		}
	}
    virtual bool blocksLight() { return false; }
    virtual bool isSolidRender(bool isServerLevel = false) { return false; }
    virtual int getRenderShape() { return Tile_SPU::SHAPE_FENCE_GATE; }
	virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face){	return true;	}
	static bool isOpen(int data) { 	return (data & OPEN_BIT) != 0; }
};
