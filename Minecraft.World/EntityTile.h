#pragma once
#include "Tile.h"

class TileEntity;

class EntityTile : public Tile
{
protected:
	EntityTile(int id, Material *material, bool isSolidRender = true);
public:
    virtual void onPlace(Level *level, int x, int y, int z);
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	virtual shared_ptr<TileEntity> newTileEntity(Level *level) = 0;
	virtual void triggerEvent(Level *level, int x, int y, int z, int b0, int b1);
};