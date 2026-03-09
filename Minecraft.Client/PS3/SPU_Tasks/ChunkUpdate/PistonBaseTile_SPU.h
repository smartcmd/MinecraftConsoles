#pragma once
#include "Tile_SPU.h"

// TileRenderer not implemented, so minimum of stuff here
class PistonBaseTile_SPU : public Tile_SPU
{

public:
	PistonBaseTile_SPU(int id) : Tile_SPU(id) {}
//	virtual void updateShape(float x0, float y0, float z0, float x1, float y1, float z1);
	virtual Icon_SPU *getTexture(int face, int data) { return nullptr; }
    virtual int getRenderShape() { return SHAPE_PISTON_BASE; }
    virtual bool isSolidRender(bool isServerLevel = false) { return false; }

	//     virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
//     virtual void updateDefaultShape();

};