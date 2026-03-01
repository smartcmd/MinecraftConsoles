#pragma once

#include "LevelSource.h"

class Material;
class TileEntity;
class BiomeSource;

class Region : public LevelSource
{
private:
	int xc1, zc1;
	LevelChunk2DArray *chunks;
	Level *level;
	bool allEmpty;

	// AP - added a caching system for Chunk::rebuild to take advantage of
	int xcCached, zcCached;
	unsigned char *CachedTiles;

public:
	Region(Level *level, int x1, int y1, int z1, int x2, int y2, int z2, int r);
	virtual ~Region();
	bool isAllEmpty();
	int getTile(int x, int y, int z);
	shared_ptr<TileEntity> getTileEntity(int x, int y, int z);
	float getBrightness(int x, int y, int z, int emitt);
	float getBrightness(int x, int y, int z);
	int getLightColor(int x, int y, int z, int emitt, int tileId = -1);	// 4J - change brought forward from 1.8.2
	int getRawBrightness(int x, int y, int z);
	int getRawBrightness(int x, int y, int z, bool propagate);
	int getData(int x, int y, int z);
	Material *getMaterial(int x, int y, int z);
	BiomeSource *getBiomeSource();
	Biome *getBiome(int x, int z);
	bool isSolidRenderTile(int x, int y, int z);
	bool isSolidBlockingTile(int x, int y, int z);
	bool isTopSolidBlocking(int x, int y, int z);
	bool isEmptyTile(int x, int y, int z);

	// 4J - changes brought forward from 1.8.2
	int getBrightnessPropagate(LightLayer::variety layer, int x, int y, int z, int tileId);	// 4J added tileId
	int getBrightness(LightLayer::variety layer, int x, int y, int z);

	int getMaxBuildHeight();
	int getDirectSignal(int x, int y, int z, int dir);

	LevelChunk* getLevelChunk(int x, int y, int z);

	// AP - added a caching system for Chunk::rebuild to take advantage of
	void setCachedTiles(unsigned char *tiles, int xc, int zc);
};