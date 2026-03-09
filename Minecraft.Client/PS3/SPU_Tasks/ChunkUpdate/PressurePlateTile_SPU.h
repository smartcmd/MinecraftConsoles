#pragma once
#include "Tile_SPU.h"


class PressurePlateTile_SPU : public Tile_SPU
{
public:
    enum Sensitivity
	{
        everything,
		mobs,
		players
    };

	PressurePlateTile_SPU(int id) : Tile_SPU(id) {}
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool blocksLight();

	virtual void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr);	// 4J added forceData, forceEntity param
    virtual void updateDefaultShape();
};
