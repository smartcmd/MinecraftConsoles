#pragma once
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "..\Minecraft.World\RandomLevelSource.h"

using namespace std;
class ServerChunkCache;

// 4J - various alterations here to make this thread safe, and operate as a fixed sized cache
class MultiPlayerChunkCache : public ChunkSource
{
	friend class LevelRenderer;
private:
	LevelChunk *emptyChunk;
	LevelChunk *waterChunk;

	vector<LevelChunk *> loadedChunkList;

	LevelChunk **cache;
	// 4J - added for multithreaded support
	CRITICAL_SECTION m_csLoadCreate;
	// 4J - size of cache is defined by size of one side - must be even
	int XZSIZE;
	int XZOFFSET;
	bool *hasData;

	Level *level;

public:
	MultiPlayerChunkCache(Level *level);
	~MultiPlayerChunkCache();
	virtual bool hasChunk(int x, int z);
	virtual bool reallyHasChunk(int x, int z);
	virtual void drop(int x, int z);
	virtual LevelChunk *create(int x, int z);
	virtual LevelChunk *getChunk(int x, int z);
	virtual bool save(bool force, ProgressListener *progressListener);
	virtual bool tick();
	virtual bool shouldSave();
	virtual void postProcess(ChunkSource *parent, int x, int z);
	virtual wstring gatherStats();
	virtual vector<Biome::MobSpawnerData *> *getMobsAt(MobCategory *mobCategory, int x, int y, int z);
	virtual TilePos *findNearestMapFeature(Level *level, const wstring &featureName, int x, int y, int z);
	virtual void recreateLogicStructuresForChunk(int chunkX, int chunkZ);
	virtual void dataReceived(int x, int z);	// 4J added

	virtual LevelChunk **getCache() { return cache; }		// 4J added
};