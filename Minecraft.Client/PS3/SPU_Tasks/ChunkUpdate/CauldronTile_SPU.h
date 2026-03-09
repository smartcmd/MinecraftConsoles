#pragma once
#include "Tile_SPU.h"


class CauldronTile_SPU : public Tile_SPU
{
public:
	CauldronTile_SPU(int id) : Tile_SPU(id) {}
	virtual Icon_SPU *getTexture(int face, int data) { return nullptr; }
	//@Override
// 	virtual void updateDefaultShape();
	virtual bool isSolidRender(bool isServerLevel = false) { return false; }
	virtual int getRenderShape() { return SHAPE_CAULDRON; }
};
