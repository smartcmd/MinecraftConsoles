#pragma once
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "..\Minecraft.World\RandomLevelSource.h"
#include <unordered_map>
#include <deque>

using namespace std;
class ServerChunkCache;

// 4J - various alterations here to make this thread safe
// Modified for infinite worlds: flat array cache replaced with unordered_map
class MultiPlayerChunkCache : public ChunkSource
{
	friend class LevelRenderer;
private:
	LevelChunk *emptyChunk;
	LevelChunk *waterChunk;

	vector<LevelChunk *> loadedChunkList;

	unordered_map<int64_t, LevelChunk *> m_chunkMap;
	unordered_map<int64_t, bool> m_hasDataMap;
	// 4J - added for multithreaded support
	CRITICAL_SECTION m_csLoadCreate;

#ifdef _LARGE_WORLDS
	bool m_isInfinite;	// true when m_XZSize >= LEVEL_MAX_WIDTH (infinite world)
#endif

	// Deferred deletion: chunks removed from the map but kept alive briefly
	// so any in-flight rebuild thread that already obtained the pointer can finish.
	// Each entry stores {chunk, tickAtWhichToDelete}.
	static const int DELETE_DELAY_TICKS = 20;	// ~1 second at 20 tps
	deque<pair<LevelChunk *, int>> m_pendingDelete;
	int m_tickCount;

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
	virtual void dataReceived(int x, int z);	// 4J added

	// getCache() is no longer meaningful for infinite worlds; returns NULL
	virtual LevelChunk **getCache() { return NULL; }

	// Expose loaded chunk list for iteration (infinite-worlds replacement for coordinate scanning)
	const vector<LevelChunk *>& getLoadedChunkList() const { return loadedChunkList; }
};