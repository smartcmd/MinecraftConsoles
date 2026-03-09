
#pragma once
#include "EntityTile_SPU.h"


class PistonMovingPiece_SPU : public EntityTile_SPU
{
public:
	PistonMovingPiece_SPU(int id) : EntityTile_SPU(id) {}

    virtual int getRenderShape() { return SHAPE_INVISIBLE; }
    virtual bool isSolidRender(bool isServerLevel = false) { return false; }
    virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr)	// 4J added forceData, forceEntity param
	{
		// should never get here.
	}
};