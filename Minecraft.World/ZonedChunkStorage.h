#pragma once
#include "File.h"
#include "ChunkStorage.h"
#include "LevelChunk.h"

// 4J Stu - There are changes to this class for 1.8.2, but since we never use it anyway lets not worry about it

using namespace std;

class ZoneFile;
class ZoneIo;

class ZonedChunkStorage : public ChunkStorage
{
public:
    static const int BIT_TERRAIN_POPULATED;

    static const int CHUNKS_PER_ZONE_BITS; // = 32
    static const int CHUNKS_PER_ZONE; // ^2

    static const int CHUNK_WIDTH;

    static const int CHUNK_HEADER_SIZE;
    static const int CHUNK_SIZE;
    static const int CHUNK_LAYERS ;
    static const int CHUNK_SIZE_BYTES;

    static const ByteOrder BYTEORDER;

    File dir;

private:
    unordered_map<int64_t, ZoneFile *> zoneFiles;
    int64_t tickCount;

public:
	ZonedChunkStorage(File dir);
private:
	int getSlot(int x, int z);
    ZoneFile *getZoneFile(int x, int z, bool create);
    ZoneIo *getBuffer(int x, int z, bool create);

public:
	LevelChunk *load(Level *level, int x, int z);
    void save(Level *level, LevelChunk *lc);

    void tick();

    void flush();

    void loadEntities(Level *level, LevelChunk *lc);
    void saveEntities(Level *level, LevelChunk *lc);
};