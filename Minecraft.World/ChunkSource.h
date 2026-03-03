#pragma once

#include "Biome.h"
class ProgressListener;
class TilePos;

// The maximum number of chunks for the world.
// For infinite worlds (_LARGE_WORLDS), this is kept at a very large value
// so that structure generation code (villages, strongholds, etc.) still works.
// The actual chunk cache is now unbounded (hash map); this constant is only
// used for structure placement limits and similar range checks.
#ifdef _LARGE_WORLDS
#define LEVEL_MAX_WIDTH (1875000)  // 30,000,000 blocks / 16 = effectively infinite kinda :3
#define LEVEL_LARGE_WIDTH (5 * 64) // 320 chunks (~5120 blocks radius) for the "Large" bounded option
#else
#define LEVEL_MAX_WIDTH 54
#endif
#define LEVEL_MIN_WIDTH 54
#define LEVEL_LEGACY_WIDTH 54

#ifdef _LARGE_WORLDS
// Anything >= LEVEL_MAX_WIDTH is treated as an infinite world
inline bool isInfiniteWorld(int xzSize) { return xzSize >= LEVEL_MAX_WIDTH; }
#endif

// Scale was 8 in the Java game, but that would make our nether tiny
// Every 1 block you move in the nether maps to HELL_LEVEL_SCALE blocks in the overworld
#ifdef _LARGE_WORLDS
#define HELL_LEVEL_MAX_SCALE 8
#else
#define HELL_LEVEL_MAX_SCALE 3
#endif
#define HELL_LEVEL_MIN_SCALE 3
#define HELL_LEVEL_LEGACY_SCALE 3

#define HELL_LEVEL_MAX_WIDTH (LEVEL_MAX_WIDTH / HELL_LEVEL_MAX_SCALE)
#define HELL_LEVEL_MIN_WIDTH 18

#define END_LEVEL_SCALE 3
// 4J Stu - Fix the size of the end for all platforms
// 54 / 3 = 18
#define END_LEVEL_MAX_WIDTH 18
#define END_LEVEL_MIN_WIDTH 18
//#define END_LEVEL_MAX_WIDTH (LEVEL_MAX_WIDTH / END_LEVEL_SCALE)

class ChunkSource
{
public:
	// 4J Added so that we can store the maximum dimensions of this world
	int m_XZSize;

public:
	virtual ~ChunkSource() {}

    virtual bool hasChunk(int x, int y) = 0;
	virtual bool reallyHasChunk(int x, int y) { return hasChunk(x,y); }	// 4J added
    virtual LevelChunk *getChunk(int x, int z) = 0;
	virtual void lightChunk(LevelChunk *lc) {}	// 4J added
    virtual LevelChunk *create(int x, int z) = 0;
    virtual void postProcess(ChunkSource *parent, int x, int z) = 0;
	virtual bool saveAllEntities() { return false; } // 4J Added
    virtual bool save(bool force, ProgressListener *progressListener) = 0;
    virtual bool tick() = 0;
    virtual bool shouldSave() = 0;

	virtual LevelChunk **getCache() { return NULL; }		// 4J added
	virtual void dataReceived(int x, int z) {}				// 4J added

    /**
     * Returns some stats that are rendered when the user holds F3.
     */
    virtual wstring gatherStats() = 0;

	virtual vector<Biome::MobSpawnerData *> *getMobsAt(MobCategory *mobCategory, int x, int y, int z) = 0;
    virtual TilePos *findNearestMapFeature(Level *level, const wstring& featureName, int x, int y, int z) = 0;
};
