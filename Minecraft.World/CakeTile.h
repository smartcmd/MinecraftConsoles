#pragma once
#include "Tile.h"
#include "Material.h"
#include "Definitions.h"

class Player;
class Random;
class Level;
class ChunkRebuildData;

class CakeTile : public Tile
{
	friend class Tile;
	friend class ChunkRebuildData;

private:
	Icon *iconTop;
	Icon *iconBottom;
	Icon *iconInner;

protected:
	CakeTile(int id);
    virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
    virtual void updateDefaultShape();
    virtual AABB *getAABB(Level *level, int x, int y, int z);
    virtual AABB *getTileAABB(Level *level, int x, int y, int z);
    virtual Icon *getTexture(int face, int data);
	//@Override
	void registerIcons(IconRegister *iconRegister);
    virtual bool isCubeShaped();
    virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool TestUse();
    virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
    virtual void attack(Level *level, int x, int y, int z, shared_ptr<Player> player);
private:
	void eat(Level *level, int x, int y, int z, shared_ptr<Player> player);
public:
	virtual bool mayPlace(Level *level, int x, int y, int z);
    virtual void neighborChanged(Level *level, int x, int y, int z, int type);
    virtual bool canSurvive(Level *level, int x, int y, int z);
    virtual int getResourceCount(Random *random);
    virtual int getResource(int data, Random *random, int playerBonusLevel);
	int cloneTileId(Level *level, int x, int y, int z);
};