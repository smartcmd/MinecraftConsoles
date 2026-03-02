#pragma once
using namespace std;

#include "ChunkSource.h"

class ProgressListener;
class LargeFeature;
class StrongholdFeature;
class VillageFeature;
class MineShaftFeature;
class PerlinNoise;

class FlatLevelSource : public ChunkSource
{
public:
		static const int CHUNK_HEIGHT = 8;
		static const int CHUNK_WIDTH = 4;

private:
	Level *level;
	Random *random;
	Random *pprandom;

	boolean generateStructures;
	VillageFeature *villageFeature;// = new VillageFeature(1);

public: 
	FlatLevelSource(Level *level, __int64 seed, bool generateStructures);
	~FlatLevelSource();

private: void prepareHeights(byteArray blocks); 

public: 
	virtual LevelChunk *create(int x, int z);
	virtual LevelChunk *getChunk(int xOffs, int zOffs);
	virtual bool hasChunk(int x, int y);
	virtual void postProcess(ChunkSource *parent, int xt, int zt);
	virtual bool save(bool force, ProgressListener *progressListener);
	virtual bool tick();
	virtual bool shouldSave();
	virtual wstring gatherStats();
	virtual vector<Biome::MobSpawnerData *> *getMobsAt(MobCategory *mobCategory, int x, int y, int z);
    virtual TilePos *findNearestMapFeature(Level *level, const wstring& featureName, int x, int y, int z);
};
