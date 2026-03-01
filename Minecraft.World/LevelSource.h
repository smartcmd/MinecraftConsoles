#pragma once

class BiomeSource;
class Material;
class TileEntity;

#include "LightLayer.h"

class LevelSource
{
public:
	virtual int getTile(int x, int y, int z) = 0;
	virtual shared_ptr<TileEntity> getTileEntity(int x, int y, int z) = 0;
	virtual int getLightColor(int x, int y, int z, int emitt, int tileId = -1) = 0;		// 4J - brought forward from 1.8.2, added tileId
	virtual float getBrightness(int x, int y, int z, int emitt) = 0;
	virtual float getBrightness(int x, int y, int z) = 0;
	virtual int getData(int x, int y, int z) = 0;
	virtual Material *getMaterial(int xx, int i, int zz) = 0;
	virtual bool isSolidRenderTile(int x, int y, int z) = 0;
	virtual bool isSolidBlockingTile(int x, int y, int z) = 0;
	virtual bool isEmptyTile(int x, int y, int z) = 0;
	virtual Biome *getBiome(int x, int z) = 0;
	virtual BiomeSource *getBiomeSource() = 0;
	virtual int getBrightness(LightLayer::variety layer, int x, int y, int z) = 0;
	virtual int getMaxBuildHeight() = 0;
	virtual bool isAllEmpty() = 0;
	virtual bool isTopSolidBlocking(int x, int y, int z) = 0;
	virtual int getDirectSignal(int x, int y, int z, int dir) = 0;
	virtual ~LevelSource() {}
};