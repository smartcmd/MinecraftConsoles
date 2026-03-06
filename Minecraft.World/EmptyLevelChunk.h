#pragma once
#include "LevelChunk.h"
#include "Definitions.h"

class Level;

class EmptyLevelChunk: public LevelChunk
{
public:
	using LevelChunk::getBlocksAndData;
	using LevelChunk::getEntities;
	using LevelChunk::getEntitiesOfClass;

	EmptyLevelChunk(Level *level, int x, int z);
	EmptyLevelChunk(Level *level, byteArray blocks, int x, int z);
	bool isAt(int x, int z);
	int getHeightmap(int x, int z);
	void recalcBlockLights();
	void recalcHeightmapOnly();
	void recalcHeightmap();
	void lightLava();
	int getTile(int x, int y, int z);
	bool setTileAndData(int x, int y, int z, int _tile, int _data);
	bool setTile(int x, int y, int z, int _tile);
	int getData(int x, int y, int z);
	bool setData(int x, int y, int z, int val, int mask, bool *maskedBitsChanged);		// 4J added mask
	int getBrightness(LightLayer::variety layer, int x, int y, int z);
	void getNeighbourBrightnesses(int *brightnesses, LightLayer::variety layer, int x, int y, int z);	// 4J added
	void setBrightness(LightLayer::variety layer, int x, int y, int z, int brightness);
	int getRawBrightness(int x, int y, int z, int skyDampen);
	void addEntity(shared_ptr<Entity> e);
	void removeEntity(shared_ptr<Entity> e);
	void removeEntity(shared_ptr<Entity> e, int yc);
	bool isSkyLit(int x, int y, int z);
	void skyBrightnessChanged();
	shared_ptr<TileEntity> getTileEntity(int x, int y, int z);
	void addTileEntity(shared_ptr<TileEntity> te);
	void setTileEntity(int x, int y, int z, shared_ptr<TileEntity> tileEntity);
	void removeTileEntity(int x, int y, int z);
	void load();
	void unload(bool unloadTileEntities) ;	// 4J - added parameter
	bool containsPlayer();	// 4J added
	void markUnsaved();
	void getEntities(shared_ptr<Entity> except, AABB bb, vector<shared_ptr<Entity> > &es, EntitySelector *selector);
	void getEntitiesOfClass(const type_info& ec, AABB bb, vector<shared_ptr<Entity> > &es, EntitySelector *selector);
	int countEntities();
	bool shouldSave(bool force);
	void setBlocks(byteArray newBlocks, int sub);
	int getBlocksAndData(byteArray data, int x0, int y0, int z0, int x1, int y1, int z1, int p, bool includeLighting = true);	// 4J - added includeLighting parameter
	int setBlocksAndData(byteArray data, int x0, int y0, int z0, int x1, int y1, int z1, int p, bool includeLighting = true);	// 4J - added includeLighting parameter
	bool testSetBlocksAndData(byteArray data, int x0, int y0, int z0, int x1, int y1, int z1, int p);	// 4J added
	Random *getRandom(int64_t l);
	bool isEmpty();
	virtual void reSyncLighting() {};	// 4J added
};
