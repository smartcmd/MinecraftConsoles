#pragma once
#include "Tile.h"
#include "Definitions.h"

class SoulSandTile : public Tile
{
public:
	SoulSandTile(int id);
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
};