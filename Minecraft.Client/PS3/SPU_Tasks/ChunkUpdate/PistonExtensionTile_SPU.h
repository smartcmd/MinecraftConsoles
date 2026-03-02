#pragma once
#include "Tile_SPU.h"

class PistonExtensionTile_SPU : public Tile_SPU
{

public:
	PistonExtensionTile_SPU(int id) : Tile_SPU(id) {}
	virtual Icon_SPU *getTexture(int face, int data) { return NULL; }
    virtual int getRenderShape() { return SHAPE_PISTON_EXTENSION; }
    virtual bool isSolidRender(bool isServerLevel = false) { return false; }
//    virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
};