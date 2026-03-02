#pragma once
#include "EntityTile.h"
#include "Material.h"

class Player;
class Random;

class ChestTile : public EntityTile
{
	friend class Tile;
public:
	static const int EVENT_SET_OPEN_COUNT = 1;
private:
	Random *random;
protected:
	ChestTile(int id);
	~ChestTile();
public:
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual int getRenderShape();
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());
	virtual void onPlace(Level *level, int x, int y, int z);
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by);
	void recalcLockDir(Level *level, int x, int y, int z);
    virtual bool mayPlace(Level *level, int x, int y, int z);
private:
	bool isFullChest(Level *level, int x, int y, int z);
	bool isCatSittingOnChest(Level *level, int x, int y, int z);
public:
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	virtual bool TestUse();
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
    shared_ptr<TileEntity> newTileEntity(Level *level);
	//@Override
	void registerIcons(IconRegister *iconRegister);
};