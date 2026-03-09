#pragma once
#include "ChunkStorage.h"

class Level;

class ChunkStorageProfilerDecorator : public ChunkStorage
{
private:
	ChunkStorage *capsulated;

	int64_t timeSpentLoading;
	int64_t loadCount;
	int64_t timeSpentSaving;
	int64_t saveCount;

	int counter;

public:
    ChunkStorageProfilerDecorator(ChunkStorage *capsulated);
    LevelChunk *load(Level *level, int x, int z);
    void save(Level *level, LevelChunk *levelChunk);
    void saveEntities(Level *level, LevelChunk *levelChunk);
    void tick();
    void flush();
};
