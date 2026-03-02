#pragma once
#include "Tile.h"
#include "Definitions.h"

class HellSandTile : public Tile
{
public:
	HellSandTile(int id);
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
};