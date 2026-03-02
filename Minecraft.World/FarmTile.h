#pragma once
#include "Tile.h"
#include "Definitions.h"

class Random;
class ChunkRebuildData;

class FarmTile : public Tile
{
	friend class Tile;
	friend class ChunkRebuildData;
private:
	Icon *iconWet;
	Icon *iconDry;

protected:
	FarmTile(int id);
public:
    virtual void updateDefaultShape(); // 4J Added override
    virtual AABB *getAABB(Level *level, int x, int y, int z);
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool isCubeShaped();
    virtual Icon *getTexture(int face, int data);
    virtual void tick(Level *level, int x, int y, int z, Random *random);
    virtual void fallOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity, float fallDistance);
private:
	virtual bool isUnderCrops(Level *level, int x, int y, int z);
	virtual bool isNearWater(Level *level, int x, int y, int z);
public:
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
    virtual bool blocksLight();
    virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int cloneTileId(Level *level, int x, int y, int z);
	//@Override
    void registerIcons(IconRegister *iconRegister);
};
