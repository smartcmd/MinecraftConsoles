#pragma once
#include "EntityTile.h"

class Mob;
class Player;
class Random;
class ChunkRebuildData;

class FurnaceTile : public EntityTile
{
	friend class Tile;
	friend class ChunkRebuildData;
private:
	Random *random;
    bool lit;
    static bool noDrop;
	Icon *iconTop;
	Icon *iconFront;

protected:
	FurnaceTile(int id, bool lit);
public:
	virtual int getResource(int data, Random *random, int playerBonusLevel);
    virtual void onPlace(Level *level, int x, int y, int z);
private:
	void recalcLockDir(Level *level, int x, int y, int z);
public:
	Icon *getTexture(int face, int data);
    void registerIcons(IconRegister *iconRegister);
    virtual void animateTick(Level *level, int xt, int yt, int zt, Random *random);
	virtual bool TestUse();
    virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
    static void setLit(bool lit, Level *level, int x, int y, int z);
protected:
	virtual shared_ptr<TileEntity> newTileEntity(Level *level);
public:
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by);
    virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
};