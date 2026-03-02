#pragma once
#include "EntityTile.h"

class Random;

class MobSpawnerTile : public EntityTile
{
	friend class Tile;
protected:
	MobSpawnerTile(int id);
public:
    virtual shared_ptr<TileEntity> newTileEntity(Level *level);
	virtual int getResource(int data, Random *random, int playerBonusLevel);
    virtual int getResourceCount(Random *random);
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool blocksLight();
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel);
	virtual int cloneTileId(Level *level, int x, int y, int z);
};