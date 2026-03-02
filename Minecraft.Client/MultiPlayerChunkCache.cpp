#include "stdafx.h"
#include "MultiPlayerChunkCache.h"
#include "ServerChunkCache.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "..\Minecraft.World\Arrays.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "MinecraftServer.h"
#include "ServerLevel.h"
#include "..\Minecraft.World\Tile.h"
#include "..\Minecraft.World\WaterLevelChunk.h"

// Pack (x,z) chunk coords into a single int64 key
static inline int64_t mpKey(int x, int z) {
	return ((int64_t)(unsigned int)x << 32) | (unsigned int)z;
}

MultiPlayerChunkCache::MultiPlayerChunkCache(Level *level)
{
	// For infinite worlds, m_XZSize is kept for compatibility but the cache is unbounded
	m_XZSize = level->dimension->getXZSize();

	emptyChunk = new EmptyLevelChunk(level, byteArray(16 * 16 * Level::maxBuildHeight), 0, 0);

	// For normal world dimension, create a chunk that can be used to create the illusion of infinite water at the edge of the world
	if( level->dimension->id == 0 )
	{
		byteArray bytes = byteArray(16 * 16 * 128);

		// Superflat.... make grass, not water...
		if(level->getLevelData()->getGenerator() == LevelType::lvl_flat)
		{
			for( int x = 0; x < 16; x++ )
				for( int y = 0; y < 128; y++ )
					for( int z = 0; z < 16; z++ )
					{
						unsigned char tileId = 0;
						if( y == 3 ) tileId = Tile::grass_Id;
						else if( y <= 2 ) tileId = Tile::dirt_Id;

						bytes[x << 11 | z << 7 | y] = tileId;
					}
		}
		else
		{
			for( int x = 0; x < 16; x++ )
				for( int y = 0; y < 128; y++ )
					for( int z = 0; z < 16; z++ )
					{
						unsigned char tileId = 0;
						if( y <= ( level->getSeaLevel() - 10 ) ) tileId = Tile::rock_Id;
						else if( y < level->getSeaLevel() ) tileId = Tile::calmWater_Id;

						bytes[x << 11 | z << 7 | y] = tileId;
					}
		}

		waterChunk = new WaterLevelChunk(level, bytes, 0, 0);

		delete[] bytes.data;

		if(level->getLevelData()->getGenerator() == LevelType::lvl_flat)
		{
			for( int x = 0; x < 16; x++ )
				for( int y = 0; y < 128; y++ )
					for( int z = 0; z < 16; z++ )
					{
						if( y >= 3 )
						{
							((WaterLevelChunk *)waterChunk)->setLevelChunkBrightness(LightLayer::Sky,x,y,z,15);
						}
					}
		}
		else
		{
			for( int x = 0; x < 16; x++ )
				for( int y = 0; y < 128; y++ )
					for( int z = 0; z < 16; z++ )
					{
						if( y >= ( level->getSeaLevel() - 1 ) )
						{
							((WaterLevelChunk *)waterChunk)->setLevelChunkBrightness(LightLayer::Sky,x,y,z,15);
						}
						else
						{
							((WaterLevelChunk *)waterChunk)->setLevelChunkBrightness(LightLayer::Sky,x,y,z,2);
						}
					}
		}
	}
	else
	{
		waterChunk = NULL;
	}

	this->level = level;
	m_tickCount = 0;

	InitializeCriticalSectionAndSpinCount(&m_csLoadCreate,4000);
}

MultiPlayerChunkCache::~MultiPlayerChunkCache()
{
	delete emptyChunk;
	delete waterChunk;

	AUTO_VAR(itEnd, loadedChunkList.end());
	for (AUTO_VAR(it, loadedChunkList.begin()); it != itEnd; it++)
		delete *it;

	// Delete any chunks still waiting in the deferred-delete queue
	for (auto &pd : m_pendingDelete)
		delete pd.first;
	m_pendingDelete.clear();

	DeleteCriticalSection(&m_csLoadCreate);
}


bool MultiPlayerChunkCache::hasChunk(int x, int z)
{
	// This cache always claims to have chunks, although it might actually just return empty data if it doesn't have anything
	return true;
}

// 4J  added - find out if we actually really do have a chunk in our cache
bool MultiPlayerChunkCache::reallyHasChunk(int x, int z)
{
	int64_t key = mpKey(x, z);
	EnterCriticalSection(&m_csLoadCreate);
	auto it = m_chunkMap.find(key);
	if (it == m_chunkMap.end() || it->second == NULL)
	{
		LeaveCriticalSection(&m_csLoadCreate);
		return false;
	}
	auto itd = m_hasDataMap.find(key);
	bool result = (itd != m_hasDataMap.end() && itd->second);
	LeaveCriticalSection(&m_csLoadCreate);
	return result;
}

void MultiPlayerChunkCache::drop(int x, int z)
{
	// 4J Stu - We do want to drop any entities in the chunks, especially for the case when a player is dead as they will
	// not get the RemoveEntity packet if an entity is removed.
	EnterCriticalSection(&m_csLoadCreate);
	int64_t key = mpKey(x, z);
	auto it = m_chunkMap.find(key);
	if (it != m_chunkMap.end() && it->second != NULL)
	{
		LevelChunk *chunk = it->second;
		if (!chunk->isEmpty())
		{
			// Added parameter here specifies that we don't want to delete tile entities, as they won't get recreated unless they've got update packets
			// The tile entities are in general only created on the client by virtue of the chunk rebuild
			chunk->unload(false);
		}

		// Remove from maps so getChunk() will return emptyChunk for this position
		m_chunkMap.erase(it);
		m_hasDataMap.erase(key);

		auto lit = std::find(loadedChunkList.begin(), loadedChunkList.end(), chunk);
		if (lit != loadedChunkList.end()) loadedChunkList.erase(lit);

		// Defer actual deletion: rebuild threads may still hold a raw LevelChunk* from
		// a getChunkAt() call that returned this chunk just before we removed it.
		// The chunk stays alive for DELETE_DELAY_TICKS so any in-flight rebuild finishes safely.
		m_pendingDelete.push_back(std::make_pair(chunk, m_tickCount + DELETE_DELAY_TICKS));
	}
	LeaveCriticalSection(&m_csLoadCreate);
}

LevelChunk *MultiPlayerChunkCache::create(int x, int z)
{
	int64_t key = mpKey(x, z);

	EnterCriticalSection(&m_csLoadCreate);
	{
		auto it = m_chunkMap.find(key);
		if (it != m_chunkMap.end() && it->second != NULL)
		{
			LevelChunk *existing = it->second;
			existing->load();
			LeaveCriticalSection(&m_csLoadCreate);
			return existing;
		}
	}

	LevelChunk *chunk = NULL;

	if( g_NetworkManager.IsHost() )		// force here to disable sharing of data
	{
		// 4J-JEV: We are about to use shared data, abort if the server is stopped and the data is deleted.
		if (MinecraftServer::getInstance()->serverHalted())
		{
			LeaveCriticalSection(&m_csLoadCreate);
			return NULL;
		}

		// If we're the host, then don't create the chunk, share data from the server's copy 
#ifdef _LARGE_WORLDS
		LevelChunk *serverChunk = MinecraftServer::getInstance()->getLevel(level->dimension->id)->cache->getChunkLoadedOrUnloaded(x,z);
#else
		LevelChunk *serverChunk = MinecraftServer::getInstance()->getLevel(level->dimension->id)->cache->getChunk(x,z);
#endif
		chunk = new LevelChunk(level, x, z, serverChunk);
		// Let renderer know that this chunk has been created - it might have made render data from the EmptyChunk if it got to a chunk before the server sent it
		level->setTilesDirty( x * 16 , 0 , z * 16 , x * 16 + 15, 127, z * 16 + 15);
		m_hasDataMap[key] = true;
	}
	else
	{
		// Passing an empty array into the LevelChunk ctor, which it now detects and sets up the chunk as compressed & empty
		byteArray bytes;

		chunk = new LevelChunk(level, bytes, x, z);

		// 4J - changed to use new methods for lighting
		chunk->setSkyLightDataAllBright(); 
	}
	
	chunk->loaded = true;

	// Insert into hash map
	m_chunkMap[key] = chunk;

	// If we're sharing with the server, we'll need to calculate our heightmap now, which isn't shared.
	if( g_NetworkManager.IsHost() )
	{
		chunk->recalcHeightmapOnly();
	}

	loadedChunkList.push_back(chunk);

	LeaveCriticalSection(&m_csLoadCreate);

	return chunk;
}

LevelChunk *MultiPlayerChunkCache::getChunk(int x, int z)
{
	EnterCriticalSection(&m_csLoadCreate);
	auto it = m_chunkMap.find(mpKey(x, z));
	LevelChunk *chunk = (it != m_chunkMap.end() && it->second != NULL) ? it->second : NULL;
	LeaveCriticalSection(&m_csLoadCreate);
	return chunk != NULL ? chunk : emptyChunk;
}

bool MultiPlayerChunkCache::save(bool force, ProgressListener *progressListener)
{
	return true;
}

bool MultiPlayerChunkCache::tick()
{
	m_tickCount++;
	while (!m_pendingDelete.empty() && m_pendingDelete.front().second <= m_tickCount)
	{
		delete m_pendingDelete.front().first;
		m_pendingDelete.pop_front();
	}

	return false;
}

bool MultiPlayerChunkCache::shouldSave()
{
	return false;
}

void MultiPlayerChunkCache::postProcess(ChunkSource *parent, int x, int z)
{
}

vector<Biome::MobSpawnerData *> *MultiPlayerChunkCache::getMobsAt(MobCategory *mobCategory, int x, int y, int z)
{
	return NULL;
}

TilePos *MultiPlayerChunkCache::findNearestMapFeature(Level *level, const wstring &featureName, int x, int y, int z)
{
	return NULL;
}

wstring MultiPlayerChunkCache::gatherStats()
{
	EnterCriticalSection(&m_csLoadCreate);
	int size = (int)loadedChunkList.size();
	LeaveCriticalSection(&m_csLoadCreate);
	return L"MultiplayerChunkCache: " + _toString<int>(size);
	
}

void MultiPlayerChunkCache::dataReceived(int x, int z)
{
	EnterCriticalSection(&m_csLoadCreate);
	m_hasDataMap[mpKey(x, z)] = true;
	LeaveCriticalSection(&m_csLoadCreate);
}
