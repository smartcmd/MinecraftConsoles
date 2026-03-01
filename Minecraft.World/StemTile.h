#pragma once
#include "Tile.h"
#include "Bush.h"

class ChunkRebuildData;
class StemTile : public Bush
{
	friend class ChunkRebuildData;
public:
	static const wstring TEXTURE_ANGLED;

private:
	Tile *fruit;
	Icon *iconAngled;

public:
	StemTile(int id, Tile *fruit);

	virtual bool mayPlaceOn(int tile);
public:
	virtual void tick(Level *level, int x, int y, int z, Random *random);
	virtual void growCrops(Level *level, int x, int y, int z);

private:
	float getGrowthSpeed(Level *level, int x, int y, int z);

public:
	using Tile::getColor;
	int getColor(int data);

	virtual int getColor(LevelSource *level, int x, int y, int z);
	virtual void updateDefaultShape();
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
	virtual int getRenderShape();

	int getConnectDir(LevelSource *level, int x, int y, int z);

	/**
	* Using this method instead of destroy() to determine if seeds should be
	* dropped
	*/
	virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus);

	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual int getResourceCount(Random *random);
	virtual int cloneTileId(Level *level, int x, int y, int z);
	void registerIcons(IconRegister *iconRegister);
	Icon *getAngledTexture();
};
