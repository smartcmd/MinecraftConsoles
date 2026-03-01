#pragma once
#include "BaseEntityTile.h"
#include "Material.h"

class Player;
class Random;

class ChestTile : public BaseEntityTile
{
	friend class Tile;
	friend class Minecraft;

public:
	static const int TYPE_BASIC = 0;
	static const int TYPE_TRAP = 1;

	static const int EVENT_SET_OPEN_COUNT = 1;

private:
	Random *random;

public:
	int type;

protected:
	ChestTile(int id, int type);
	~ChestTile();

public:
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual int getRenderShape();
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());
	virtual void onPlace(Level *level, int x, int y, int z);
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	void recalcLockDir(Level *level, int x, int y, int z);
	virtual bool mayPlace(Level *level, int x, int y, int z);

private:
	bool isFullChest(Level *level, int x, int y, int z);

public:
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
	virtual bool TestUse();
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	virtual shared_ptr<Container> getContainer(Level *level, int x, int y, int z);
	virtual shared_ptr<TileEntity> newTileEntity(Level *level);
	virtual bool isSignalSource();
	virtual int getSignal(LevelSource *level, int x, int y, int z, int dir);
	virtual int getDirectSignal(LevelSource *level, int x, int y, int z, int dir);

private:
	bool isCatSittingOnChest(Level *level, int x, int y, int z);

public:
	virtual bool hasAnalogOutputSignal();
	virtual int getAnalogOutputSignal(Level *level, int x, int y, int z, int dir);
	virtual void registerIcons(IconRegister *iconRegister);
};