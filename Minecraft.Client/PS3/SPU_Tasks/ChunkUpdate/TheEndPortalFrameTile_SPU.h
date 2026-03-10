#pragma once
#include "Tile_SPU.h"

class TheEndPortalFrameTile_SPU : public Tile_SPU
{
public:
    TheEndPortalFrameTile_SPU(int id) : Tile_SPU(id) {}
	virtual Icon_SPU *getTexture(int face, int data) { return nullptr; }
    virtual bool isSolidRender(bool isServerLevel = false) { return false; }
    virtual int getRenderShape() { return SHAPE_PORTAL_FRAME; }
//    virtual void updateDefaultShape();
};