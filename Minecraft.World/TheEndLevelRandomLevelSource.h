#pragma once

#include "ChunkSource.h"
class PerlinNoise;

class TheEndLevelRandomLevelSource : public ChunkSource
{
public:
	static const double SNOW_CUTOFF;
	static const double SNOW_SCALE;
	static const bool FLOATING_ISLANDS;

	static const int CHUNK_HEIGHT = 4;
	static const int CHUNK_WIDTH = 8;
private:
	Random *random;
	Random *pprandom;

private:
	PerlinNoise *lperlinNoise1;
	PerlinNoise *lperlinNoise2;
	PerlinNoise *perlinNoise1;
public:
	PerlinNoise *scaleNoise;
	PerlinNoise *depthNoise;
	PerlinNoise *forestNoise;


private:
	Level *level;

public:
	TheEndLevelRandomLevelSource(Level *level, int64_t seed);
	~TheEndLevelRandomLevelSource();

	void prepareHeights(int xOffs, int zOffs, byteArray blocks, BiomeArray biomes);
	void buildSurfaces(int xOffs, int zOffs, byteArray blocks, BiomeArray biomes);

public:
	virtual LevelChunk *create(int x, int z);
	virtual LevelChunk *getChunk(int xOffs, int zOffs);

private:
	doubleArray getHeights(doubleArray buffer, int x, int y, int z, int xSize, int ySize, int zSize);

public:
	virtual bool hasChunk(int x, int y);
private:
	void calcWaterDepths(ChunkSource *parent, int xt, int zt);
public:
	virtual void postProcess(ChunkSource *parent, int xt, int zt);
	virtual bool save(bool force, ProgressListener *progressListener);
	virtual bool tick();
	virtual bool shouldSave();
	virtual wstring gatherStats();

public:
	virtual vector<Biome::MobSpawnerData *> *getMobsAt(MobCategory *mobCategory, int x, int y, int z);
	virtual TilePos *findNearestMapFeature(Level *level, const wstring& featureName, int x, int y, int z);
	virtual void recreateLogicStructuresForChunk(int chunkX, int chunkZ);
};
