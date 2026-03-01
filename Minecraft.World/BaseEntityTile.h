#pragma once
#include "Tile.h"
#include "EntityTile.h"

class TileEntity;

class BaseEntityTile : public Tile, public EntityTile
{
protected:
	BaseEntityTile(int id, Material *material, bool isSolidRender = true);
public:
	virtual void onPlace(Level *level, int x, int y, int z);
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	virtual bool triggerEvent(Level *level, int x, int y, int z, int b0, int b1);
};