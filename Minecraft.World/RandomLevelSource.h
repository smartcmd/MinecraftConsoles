#pragma once
using namespace std;

#include "ChunkSource.h"

class ProgressListener;
class LargeFeature;
class StrongholdFeature;
class VillageFeature;
class MineShaftFeature;
class PerlinNoise;
class RandomScatteredLargeFeature;

class RandomLevelSource : public ChunkSource
{
public:
    static const double SNOW_CUTOFF;
    static const double SNOW_SCALE;
    static const bool FLOATING_ISLANDS = false;
    static const int CHUNK_HEIGHT = 8;
    static const int CHUNK_WIDTH = 4;

private:
    Random *random;
	Random *pprandom;	// 4J - added

    PerlinNoise *lperlinNoise1;
    PerlinNoise *lperlinNoise2;
    PerlinNoise *perlinNoise1;
    PerlinNoise *perlinNoise3;

public:
    PerlinNoise *scaleNoise;
    PerlinNoise *depthNoise;

private:
    PerlinNoise *floatingIslandScale;
    PerlinNoise *floatingIslandNoise;

public:
    PerlinNoise *forestNoise;

private:
	Level *level;
	const bool generateStructures;

	floatArray pows;

public:
	RandomLevelSource(Level *level, __int64 seed, bool generateStructures);
	~RandomLevelSource();

public:
	void prepareHeights(int xOffs, int zOffs, byteArray blocks);

public:
    void buildSurfaces(int xOffs, int zOffs, byteArray blocks, BiomeArray biomes);

private:
	LargeFeature *caveFeature;
	StrongholdFeature *strongholdFeature;
	VillageFeature *villageFeature;
	MineShaftFeature *mineShaftFeature;
	RandomScatteredLargeFeature *scatteredFeature;
	LargeFeature *canyonFeature;
private:
    virtual LevelChunk *create(int x, int z);

public:
    virtual LevelChunk *getChunk(int xOffs, int zOffs);
	virtual void lightChunk(LevelChunk *lc);	// 4J added

private:
	doubleArray getHeights(doubleArray buffer, int x, int y, int z, int xSize, int ySize, int zSize, BiomeArray& biomes);

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
