#pragma once

#include "BaseEntityTile.h"

class SkullTileEntity;

class SkullTile : public BaseEntityTile
{
	friend class Tile;
public:
	static const int MAX_SKULL_TILES = 40;
public:
	static const int PLACEMENT_MASK = 0x7;
	static const int NO_DROP_BIT = 0x8;

	SkullTile(int id);

public:
	using BaseEntityTile::onRemove;

	int getRenderShape();
	bool isSolidRender(bool isServerLevel = false);
	bool isCubeShaped();
	void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());
	AABB *getAABB(Level *level, int x, int y, int z);
	void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by);
	shared_ptr<TileEntity> newTileEntity(Level *level);
	int cloneTileId(Level *level, int x, int y, int z);
	int cloneTileData(Level *level, int x, int y, int z);
	int getSpawnResourcesAuxValue(int data);
	void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel);
	void playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player);
	void onRemove(Level *level, int x, int y, int z, int id, int data);
	int getResource(int data, Random *random, int playerBonusLevel);
	void checkMobSpawn(Level *level, int x, int y, int z, shared_ptr<SkullTileEntity> placedSkull);

private:
	bool isSkullAt(Level *level, int x, int y, int z, int skullType);

public:
	void registerIcons(IconRegister *iconRegister);
	Icon *getTexture(int face, int data);
	wstring getTileItemIconName();
};
