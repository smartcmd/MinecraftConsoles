#pragma once
using namespace std;

#include "ChunkSource.h"

#ifndef _CONTENT_PACKAGE
#define _OVERRIDE_HEIGHTMAP
#endif

class ProgressListener;
class LargeFeature;
class StrongholdFeature;
class VillageFeature;
class MineShaftFeature;

class CustomLevelSource : public ChunkSource
{
public:
    static const double SNOW_CUTOFF;
    static const double SNOW_SCALE;
    static const bool FLOATING_ISLANDS = false;
    static const int CHUNK_HEIGHT = 8;
    static const int CHUNK_WIDTH = 4;

private:

#ifdef _OVERRIDE_HEIGHTMAP
	Random *random;
	Random *pprandom;	// 4J - added
    PerlinNoise *perlinNoise3;
	LargeFeature *caveFeature;
 	StrongholdFeature *strongholdFeature;
 	VillageFeature *villageFeature;
 	MineShaftFeature *mineShaftFeature;
 	LargeFeature *canyonFeature;
	Level *level;
#endif

	byteArray m_heightmapOverride;
	byteArray m_waterheightOverride;

private:

	const bool generateStructures;

public:
	CustomLevelSource(Level *level, __int64 seed, bool generateStructures);
	~CustomLevelSource();

public:
	void prepareHeights(int xOffs, int zOffs, byteArray blocks);

public:
    void buildSurfaces(int xOffs, int zOffs, byteArray blocks, BiomeArray biomes);

private:
    virtual LevelChunk *create(int x, int z);

public:
    virtual LevelChunk *getChunk(int xOffs, int zOffs);
	virtual void lightChunk(LevelChunk *lc);	// 4J added

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
};
