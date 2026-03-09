#pragma once
#include "LevelChunk.h"
#include "Definitions.h"

class Level;

class WaterLevelChunk:  public LevelChunk
{
public:
	using LevelChunk::getEntities;
	using LevelChunk::getEntitiesOfClass;

	WaterLevelChunk(Level *level, byteArray blocks, int x, int z);
	bool isAt(int x, int z);
	void recalcBlockLights();
	void recalcHeightmapOnly();
	void recalcHeightmap();
	void lightLava();
	bool setTileAndData(int x, int y, int z, int _tile, int _data);
	bool setTile(int x, int y, int z, int _tile);
	bool setData(int x, int y, int z, int val, int mask, bool *maskedBitsChanged);		// 4J added mask
	void setBrightness(LightLayer::variety layer, int x, int y, int z, int brightness);
	void setLevelChunkBrightness(LightLayer::variety layer, int x, int y, int z, int brightness);		// 4J added - calls the setBrightness method of the parent class
	void addEntity(shared_ptr<Entity> e);
	void removeEntity(shared_ptr<Entity> e);
	void removeEntity(shared_ptr<Entity> e, int yc);
	void skyBrightnessChanged();
	shared_ptr<TileEntity> getTileEntity(int x, int y, int z);
	void addTileEntity(shared_ptr<TileEntity> te);
	void setTileEntity(int x, int y, int z, shared_ptr<TileEntity> tileEntity);
	void removeTileEntity(int x, int y, int z);
	void load();
	void unload(bool unloadTileEntities) ;	// 4J - added parameter
	bool containsPlayer(); // 4J added
	void markUnsaved();
	void getEntities(shared_ptr<Entity> except, AABB bb, vector<shared_ptr<Entity> > &es);
	void getEntitiesOfClass(const type_info& ec, AABB bb, vector<shared_ptr<Entity> > &es);
	int countEntities();
	bool shouldSave(bool force);
	void setBlocks(byteArray newBlocks, int sub);
	int setBlocksAndData(byteArray data, int x0, int y0, int z0, int x1, int y1, int z1, int p, bool includeLighting = true);	// 4J - added includeLighting parameter;
	bool testSetBlocksAndData(byteArray data, int x0, int y0, int z0, int x1, int y1, int z1, int p);
	Random *getRandom(int64_t l);
	virtual Biome *getBiome(int x, int z, BiomeSource *biomeSource);
	virtual void reSyncLighting() {};	// 4J added
};
