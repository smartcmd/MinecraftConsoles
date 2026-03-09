#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "ReadOnlyChunkCache.h"
#include "Biome.h"

ReadOnlyChunkCache::ReadOnlyChunkCache(Level *level, ChunkStorage *storage)
{
	chunks = LevelChunkArray(LEN * LEN);
	emptyPixels = byteArray(Level::CHUNK_TILE_COUNT);

    this->level = level;
    this->storage = storage;
}

ReadOnlyChunkCache::~ReadOnlyChunkCache()
{	
	for(unsigned int i = 0; i < chunks.length; ++i)
		delete chunks[i];

	delete[] chunks.data;

	delete[] emptyPixels.data;
}

bool ReadOnlyChunkCache::hasChunk(int x, int z)
{
    int slot = (x & LEN_MASK) | ((z & LEN_MASK) * LEN);
    return chunks[slot] != nullptr && (chunks[slot]->isAt(x, z));
}

LevelChunk *ReadOnlyChunkCache::create(int x, int z)
{
	return getChunk(x, z);
}

LevelChunk *ReadOnlyChunkCache::getChunk(int x, int z)
{
    int slot = (x & LEN_MASK) | ((z & LEN_MASK) * LEN);
	// 4J - removed try/catch
//    try {
        if (!hasChunk(x, z))
		{
            LevelChunk *newChunk = load(x, z);
            if (newChunk == nullptr)
			{
                newChunk = new EmptyLevelChunk(level, emptyPixels, x, z);
            }
            chunks[slot] = newChunk;
        }
        return chunks[slot];
//    } catch (Exception e) {
//        e.printStackTrace();
//        return null;
//    }
}

LevelChunk *ReadOnlyChunkCache::load(int x, int z)
{
	// 4J - remove try/catch
//    try {
        return storage->load(level, x, z);
//    } catch (IOException e) {
//        e.printStackTrace();
//        return null;
//    }
}
	// 4J - TODO - was synchronized
void ReadOnlyChunkCache::postProcess(ChunkSource *parent, int x, int z)
{
}

bool ReadOnlyChunkCache::save(bool force, ProgressListener *progressListener)
{
	return true;
}

bool ReadOnlyChunkCache::tick()
{
	return false;
}

bool ReadOnlyChunkCache::shouldSave()
{
	return false;
}

wstring ReadOnlyChunkCache::gatherStats()
{
	return L"ReadOnlyChunkCache";
}

vector<Biome::MobSpawnerData *> *ReadOnlyChunkCache::getMobsAt(MobCategory *mobCategory, int x, int y, int z)
{
	return nullptr;
}

TilePos *ReadOnlyChunkCache::findNearestMapFeature(Level *level, const wstring& featureName, int x, int y, int z)
{
	return nullptr;
}
