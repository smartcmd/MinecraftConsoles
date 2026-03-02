#pragma once
#include "Tile_SPU.h"

class LeverTile_SPU : public Tile_SPU
{
public:
	LeverTile_SPU(int id) : Tile_SPU(id) {}
    virtual bool blocksLight() { return false; }
    virtual bool isSolidRender(bool isServerLevel = false) { return false; }
	virtual int getRenderShape() { return Tile_SPU::SHAPE_LEVER; }
// 	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
};
