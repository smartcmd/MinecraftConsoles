#pragma once
#include "EntityTile.h"

class Player;
class Mob;
class ChunkRebuildData;
class DispenserTile : public EntityTile
{
	friend class Tile;
	friend class ChunkRebuildData;

private:
	static const int DISPENSE_ITEM = 0;
    static const int REMOVE_ITEM = 1;
    static const int LEAVE_ITEM = 2;

public:
	static const int FACING_MASK = 0x7;

protected:
	Random *random;

	Icon *iconTop;
	Icon *iconFront;
	Icon *iconFrontVertical;

protected:
	DispenserTile(int id);

public:
	virtual int getTickDelay();
    virtual int getResource(int data, Random *random, int playerBonusLevel);
    virtual void onPlace(Level *level, int x, int y, int z);

private:
	void recalcLockDir(Level *level, int x, int y, int z);

public:
	virtual Icon *getTexture(int face, int data);
	//@Override
	void registerIcons(IconRegister *iconRegister);
	virtual bool TestUse();
    virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param

private:
	void fireArrow(Level *level, int x, int y, int z, Random *random);

public:
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
    virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual shared_ptr<TileEntity> newTileEntity(Level *level);
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by);
    virtual void onRemove(Level *level, int x, int y, int z, int id, int data);

private:
	static void throwItem(Level *level, shared_ptr<ItemInstance> item, Random *random, int accuracy, int xd, int zd, double xp, double yp, double zp);
	static int dispenseItem(shared_ptr<DispenserTileEntity> trap, Level *level, shared_ptr<ItemInstance> item, Random *random, int x, int y, int z, int xd, int zd, double xp, double yp, double zp);
};