#pragma once
#include "ChunkStorage.h"

class Level;

class ChunkStorageProfilerDecorator : public ChunkStorage
{
private:
	ChunkStorage *capsulated;

	__int64 timeSpentLoading;
	__int64 loadCount;
	__int64 timeSpentSaving;
	__int64 saveCount;

	int counter;

public:
    ChunkStorageProfilerDecorator(ChunkStorage *capsulated);
    LevelChunk *load(Level *level, int x, int z);
    void save(Level *level, LevelChunk *levelChunk);
    void saveEntities(Level *level, LevelChunk *levelChunk);
    void tick();
    void flush();
};
