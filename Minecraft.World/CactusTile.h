#pragma once
#include "Tile.h"
#include "Material.h"
#include "Definitions.h"

class Random;
class Level;
class ChunkRebuildData;

class CactusTile : public Tile
{
	friend class Tile;
	friend class ChunkRebuildData;

private:
	Icon *iconTop;
    Icon *iconBottom;

protected:
	CactusTile(int id);

public:
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual AABB *getTileAABB(Level *level, int x, int y, int z);
	virtual Icon *getTexture(int face, int data);
	virtual bool isCubeShaped();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual int getRenderShape();
	virtual bool mayPlace(Level *level, int x, int y, int z);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual bool canSurvive(Level *level, int x, int y, int z);
	virtual void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
	//@Override
    void registerIcons(IconRegister *iconRegister);
	
	// 4J Added so we can check before we try to add a tile to the tick list if it's actually going to do seomthing
	virtual bool shouldTileTick(Level *level, int x,int y,int z);
};