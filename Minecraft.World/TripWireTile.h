#pragma once

#include "Tile.h"

class TripWireTile : public Tile
{
	using Tile::getTickDelay;
public:
	static const int MASK_POWERED = 0x1;
	static const int MASK_SUSPENDED = 0x2;
	static const int MASK_ATTACHED = 0x4;
	static const int MASK_DISARMED = 0x8;

	TripWireTile(int id);

	int getTickDelay(Level *level);
	AABB *getAABB(Level *level, int x, int y, int z);
	bool blocksLight();
	bool isSolidRender(bool isServerLevel = false);
	bool isCubeShaped();
	int getRenderLayer();
	int getRenderShape();
	int getResource(int data, Random *random, int playerBonusLevel);
	int cloneTileId(Level *level, int x, int y, int z);
	void neighborChanged(Level *level, int x, int y, int z, int type);
	void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());
	void onPlace(Level *level, int x, int y, int z);
	void onRemove(Level *level, int x, int y, int z, int id, int data);
	void playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player);

private:
	void updateSource(Level *level, int x, int y, int z, int data);

public:
	void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
	void tick(Level *level, int x, int y, int z, Random *random);

private:
	void checkPressed(Level *level, int x, int y, int z);

public:
	static bool shouldConnectTo(LevelSource *level, int x, int y, int z, int data, int dir);
};
