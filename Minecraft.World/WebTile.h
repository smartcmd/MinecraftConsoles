#pragma once

#include "Tile.h"

class WebTile : public Tile
{

public:
	WebTile(int id);


public:
	void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);


public:
	bool isSolidRender(bool isServerLevel = false);

public:
	AABB *getAABB(Level *level, int x, int y, int z);

public:
	int getRenderShape();

	bool blocksLight();
	bool isCubeShaped();
	virtual int getResource(int data, Random *random, int playerBonusLevel);

protected:
	bool isSilkTouchable();
};