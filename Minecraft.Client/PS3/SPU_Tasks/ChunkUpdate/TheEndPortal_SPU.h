#pragma once
#include "EntityTile_SPU.h"

class TheEndPortal_SPU : public EntityTile_SPU
{
public:
	TheEndPortal_SPU(int id) : EntityTile_SPU(id) {}

//     virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
    virtual bool shouldRenderFace(ChunkRebuildData *level, int x, int y, int z, int face)
	{
		if (face != 0) return false;
		return EntityTile_SPU::shouldRenderFace(level, x, y, z, face);
	}
    virtual bool isSolidRender(bool isServerLevel = false) { return false; }
    virtual int getRenderShape() { return SHAPE_INVISIBLE; }
};