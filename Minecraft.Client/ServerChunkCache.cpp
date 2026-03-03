#include "stdafx.h"
#include "ServerChunkCache.h"
#include "ServerLevel.h"
#include "MinecraftServer.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "..\Minecraft.World\Pos.h"
#include "..\Minecraft.World\ProgressListener.h"
#include "..\Minecraft.World\ThreadName.h"
#include "..\Minecraft.World\compression.h"
#include "..\Minecraft.World\OldChunkStorage.h"

ServerChunkCache::ServerChunkCache(ServerLevel *level, ChunkStorage *storage, ChunkSource *source)
{
	autoCreate = false;	// 4J added
    
	emptyChunk = new EmptyLevelChunk(level, byteArray( Level::CHUNK_TILE_COUNT ), 0, 0);

    this->level = level;
    this->storage = storage;
    this->source = source;
	// For infinite worlds, m_XZSize is no longer used for cache sizing.
	// Keep it at a large value so code that reads it (e.g. dimension getXZSize) still works.
	this->m_XZSize = source->m_XZSize;
#ifdef _LARGE_WORLDS
	this->m_isInfinite = isInfiniteWorld(this->m_XZSize);
#endif

	InitializeCriticalSectionAndSpinCount(&m_csLoadCreate,4000);
}

// 4J-PB added
ServerChunkCache::~ServerChunkCache()
{
	delete emptyChunk;
	delete source;

#ifdef _LARGE_WORLDS
	for (auto &kv : m_unloadedMap)
		delete kv.second;
	m_unloadedMap.clear();
#endif

	AUTO_VAR(itEnd, m_loadedChunkList.end());
	for (AUTO_VAR(it, m_loadedChunkList.begin()); it != itEnd; it++)
		delete *it;
	DeleteCriticalSection(&m_csLoadCreate);
}

bool ServerChunkCache::hasChunk(int x, int z)
{
#ifdef _LARGE_WORLDS
	// For finite worlds, out-of-bounds coordinates are treated as "exists" (sea/empty edge)
	if (!m_isInfinite)
	{
		int half = m_XZSize / 2;
		if (x < -half || x >= half || z < -half || z >= half) return true;
	}
#endif
	// Check if chunk is loaded in the map
	EnterCriticalSection(&m_csLoadCreate);
	auto it = m_chunkMap.find(chunkKey(x, z));
	bool result = (it != m_chunkMap.end() && it->second != NULL);
	LeaveCriticalSection(&m_csLoadCreate);
	return result;
}

vector<LevelChunk *> *ServerChunkCache::getLoadedChunkList()
{
	return &m_loadedChunkList;
}

void ServerChunkCache::drop(int x, int z)
{
#ifdef _LARGE_WORLDS
	int64_t key = chunkKey(x, z);
	auto it = m_chunkMap.find(key);
	if (it != m_chunkMap.end() && it->second != NULL)
	{
		m_toDrop.push_back(it->second);
	}
#endif
}

void ServerChunkCache::dropAll()
{
#ifdef _LARGE_WORLDS
	for (LevelChunk *chunk : m_loadedChunkList)
	{
		drop(chunk->x, chunk->z);
}
#endif
}

// 4J - this is the original (and virtual) interface to create
LevelChunk *ServerChunkCache::create(int x, int z)
{
	return create(x, z, false);
}

LevelChunk *ServerChunkCache::create(int x, int z, bool asyncPostProcess)	// 4J - added extra parameter
{
#ifdef _LARGE_WORLDS
	// For finite worlds, refuse to create chunks outside the world boundary
	if (!m_isInfinite)
	{
		int half = m_XZSize / 2;
		if (x < -half || x >= half || z < -half || z >= half) return emptyChunk;
	}
#endif

	int64_t key = chunkKey(x, z);

	EnterCriticalSection(&m_csLoadCreate);

	// Check under lock
	{
		auto it = m_chunkMap.find(key);
		if (it != m_chunkMap.end() && it->second != NULL)
		{
			LevelChunk *existing = it->second;
			LeaveCriticalSection(&m_csLoadCreate);
			return existing;
		}
	}

    LevelChunk *chunk = load(x, z);
    if (chunk == NULL)
	{
        if (source == NULL)
		{
            chunk = emptyChunk;
        }
		else
		{
            chunk = source->getChunk(x, z);
        }
    }
	if (chunk != NULL)
	{
		chunk->load();
	}

	m_chunkMap[key] = chunk;

	// 4J - added - this will run a recalcHeightmap if source is a randomlevelsource, which has been split out from source::getChunk so that
	// we are doing it after the chunk has been added to the cache - otherwise a lot of the lighting fails as lights aren't added if the chunk
	// they are in fail ServerChunkCache::hasChunk.
	source->lightChunk(chunk);

	// Prevent cascading chunk creation: when isFindingSpawn or autoCreate is true,
	// getChunk() auto-creates missing chunks. Post-processing and updatePostProcessFlags
	// call getChunk() for neighbors, which would recursively create() those neighbors,
	// whose post-processing creates more neighbors, flood-filling the entire world.
	// Temporarily disable auto-creation so these internal calls return emptyChunk
	// for missing neighbors instead of cascading.
	bool savedFindingSpawn = level->isFindingSpawn;
	bool savedAutoCreate = autoCreate;
	level->isFindingSpawn = false;
	autoCreate = false;

	updatePostProcessFlags( x, z );

	m_loadedChunkList.push_back(chunk);

	// 4J - If post-processing is to be async, then let the server know about requests rather than processing directly here.
	if( asyncPostProcess )
	{
		// 4J Stu - TODO This should also be calling the same code as chunk->checkPostProcess, but then we cannot guarantee we are in the server add the post-process request
		if ( ( (chunk->terrainPopulated & LevelChunk::sTerrainPopulatedFromHere) == 0) && hasChunk(x + 1, z + 1) && hasChunk(x, z + 1) && hasChunk(x + 1, z)) MinecraftServer::getInstance()->addPostProcessRequest(this, x, z);
		if (hasChunk(x - 1, z) && ((getChunk(x - 1, z)->terrainPopulated & LevelChunk::sTerrainPopulatedFromHere ) == 0 ) && hasChunk(x - 1, z + 1) && hasChunk(x, z + 1) && hasChunk(x - 1, z)) MinecraftServer::getInstance()->addPostProcessRequest(this, x - 1, z);
		if (hasChunk(x, z - 1) && ((getChunk(x, z - 1)->terrainPopulated & LevelChunk::sTerrainPopulatedFromHere ) == 0 ) && hasChunk(x + 1, z - 1) && hasChunk(x, z - 1) && hasChunk(x + 1, z)) MinecraftServer::getInstance()->addPostProcessRequest(this, x, z - 1);
		if (hasChunk(x - 1, z - 1) && ((getChunk(x - 1, z - 1)->terrainPopulated & LevelChunk::sTerrainPopulatedFromHere ) == 0 ) && hasChunk(x - 1, z - 1) && hasChunk(x, z - 1) && hasChunk(x - 1, z)) MinecraftServer::getInstance()->addPostProcessRequest(this, x - 1, z - 1);
	}
	else
	{
		chunk->checkPostProcess(this, this, x, z);
	}

	// 4J - Now try and fix up any chests that were saved pre-1.8.2.
	if( hasChunk( x - 1, z ) && hasChunk( x - 2, z ) && hasChunk( x - 1, z + 1 ) && hasChunk( x - 1, z - 1 ) ) chunk->checkChests( this, x - 1, z );
	if( hasChunk( x, z + 1) && hasChunk( x , z + 2 ) && hasChunk( x - 1, z + 1 ) && hasChunk( x + 1, z + 1 ) ) chunk->checkChests( this, x, z + 1);
	if( hasChunk( x + 1, z ) && hasChunk( x + 2, z ) && hasChunk( x + 1, z + 1 ) && hasChunk( x + 1, z - 1 ) ) chunk->checkChests( this, x + 1, z );
	if( hasChunk( x, z - 1) && hasChunk( x , z - 2 ) && hasChunk( x - 1, z - 1 ) && hasChunk( x + 1, z - 1 ) ) chunk->checkChests( this, x, z - 1);
	if( hasChunk( x - 1, z ) && hasChunk( x + 1, z ) && hasChunk ( x, z - 1 ) && hasChunk( x, z + 1 ) ) chunk->checkChests( this, x, z );

	// Restore auto-creation flags
	level->isFindingSpawn = savedFindingSpawn;
	autoCreate = savedAutoCreate;

	LeaveCriticalSection(&m_csLoadCreate);

#ifdef __PS3__
	Sleep(1);
#endif // __PS3__
    return chunk;

}

// 4J Stu - Split out this function so that we get a chunk without loading entities
// This is used when sharing server chunk data on the main thread
LevelChunk *ServerChunkCache::getChunk(int x, int z)
{
#ifdef _LARGE_WORLDS
	// For finite worlds, out-of-bounds coordinates return the empty chunk
	if (!m_isInfinite)
	{
		int half = m_XZSize / 2;
		if (x < -half || x >= half || z < -half || z >= half) return emptyChunk;
	}
#endif

	EnterCriticalSection(&m_csLoadCreate);
	auto it = m_chunkMap.find(chunkKey(x, z));
	LevelChunk *chunk = (it != m_chunkMap.end() && it->second != NULL) ? it->second : NULL;
	LeaveCriticalSection(&m_csLoadCreate);

	if (chunk != NULL) return chunk;
	if (level->isFindingSpawn || autoCreate)
		return create(x, z);
	return emptyChunk;
}

#ifdef _LARGE_WORLDS
// 4J added - this special variation on getChunk also checks the unloaded chunk cache. It is called on a host machine from the client-side level when:
// (1) Trying to determine whether the client blocks and data are the same as those on the server, so we can start sharing them
// (2) Trying to resync the lighting data from the server to the client
// As such it is really important that we don't return emptyChunk in these situations, when we actually still have the block/data/lighting in the unloaded cache
LevelChunk *ServerChunkCache::getChunkLoadedOrUnloaded(int x, int z)
{
	int64_t key = chunkKey(x, z);

	EnterCriticalSection(&m_csLoadCreate);
	auto it = m_chunkMap.find(key);
	if (it != m_chunkMap.end() && it->second != NULL)
	{
		LevelChunk *chunk = it->second;
		LeaveCriticalSection(&m_csLoadCreate);
		return chunk;
	}

	auto it2 = m_unloadedMap.find(key);
	if (it2 != m_unloadedMap.end() && it2->second != NULL)
	{
		LevelChunk *chunk = it2->second;
		LeaveCriticalSection(&m_csLoadCreate);
		return chunk;
	}
	LeaveCriticalSection(&m_csLoadCreate);

	if( level->isFindingSpawn || autoCreate )
		return create(x, z);

	return emptyChunk;
}
#endif

// 4J Added //
#ifdef _LARGE_WORLDS
void ServerChunkCache::dontDrop(int x, int z)
{
	LevelChunk *chunk = getChunk(x, z);
	m_toDrop.erase(std::remove(m_toDrop.begin(), m_toDrop.end(), chunk), m_toDrop.end());
}
#endif

LevelChunk *ServerChunkCache::load(int x, int z)
{
    if (storage == NULL) return NULL;

    LevelChunk *levelChunk = NULL;

#ifdef _LARGE_WORLDS
	// Check the in-memory unloaded cache first before going to disk
	int64_t key = chunkKey(x, z);
	auto it = m_unloadedMap.find(key);
	if (it != m_unloadedMap.end())
	{
		levelChunk = it->second;
		m_unloadedMap.erase(it);
	}
	if (levelChunk == NULL)
#endif
	{
		levelChunk = storage->load(level, x, z);
	}
    if (levelChunk != NULL)
	{
        levelChunk->lastSaveTime = level->getTime();
    }
    return levelChunk;
}

void ServerChunkCache::saveEntities(LevelChunk *levelChunk)
{
    if (storage == NULL) return;

    storage->saveEntities(level, levelChunk);
}

void ServerChunkCache::save(LevelChunk *levelChunk)
{
	if (storage == NULL) return;

	levelChunk->lastSaveTime = level->getTime();
	storage->save(level, levelChunk);
}

// 4J added
void ServerChunkCache::updatePostProcessFlag(short flag, int x, int z, int xo, int zo, LevelChunk *lc)
{
	if( hasChunk( x + xo, z + zo ) )
	{
		LevelChunk *lc2 = getChunk(x + xo, z + zo);
		if( lc2 != emptyChunk )		// Will only be empty chunk of this is the edge (we've already checked hasChunk so won't just be a missing chunk)
		{
			if( lc2->terrainPopulated & LevelChunk::sTerrainPopulatedFromHere )
			{
				lc->terrainPopulated |= flag;
			}
		}
		else
		{
			// The edge - always consider as post-processed
			lc->terrainPopulated |= flag;
		}
	}
}

// 4J added - normally we try and set these flags when a chunk is post-processed. However, when setting in a north or easterly direction the
// affected chunks might not themselves exist, so we need to check the flags also when creating new chunks.
void ServerChunkCache::updatePostProcessFlags(int x, int z)
{
	LevelChunk *lc = getChunk(x, z);
	if( lc != emptyChunk )
	{
		// First check if any of our neighbours are post-processed, that should affect OUR flags
		updatePostProcessFlag( LevelChunk::sTerrainPopulatedFromS,  x, z,  0, -1, lc );
		updatePostProcessFlag( LevelChunk::sTerrainPopulatedFromSW, x, z, -1, -1, lc );
		updatePostProcessFlag( LevelChunk::sTerrainPopulatedFromW,  x, z, -1,  0, lc );
		updatePostProcessFlag( LevelChunk::sTerrainPopulatedFromNW, x, z, -1,  1, lc );
		updatePostProcessFlag( LevelChunk::sTerrainPopulatedFromN,  x, z,  0,  1, lc );
		updatePostProcessFlag( LevelChunk::sTerrainPopulatedFromNE, x, z,  1,  1, lc );
		updatePostProcessFlag( LevelChunk::sTerrainPopulatedFromE,  x, z,  1,  0, lc );
		updatePostProcessFlag( LevelChunk::sTerrainPopulatedFromSE, x, z,  1, -1, lc );

		// Then, if WE are post-processed, check that our neighbour's flags are also set
		if( lc->terrainPopulated & LevelChunk::sTerrainPopulatedFromHere )
		{
			flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromW,  x + 1, z + 0 );
			flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromSW, x + 1, z + 1 );
			flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromS,  x + 0, z + 1 );
			flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromSE, x - 1, z + 1 );
			flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromE,  x - 1, z + 0 );
			flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromNE, x - 1, z - 1 );
			flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromN,  x + 0, z - 1 );
			flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromNW, x + 1, z - 1 );
		}
	}

	flagPostProcessComplete(0, x, z);
}

// 4J added - add a flag to a chunk to say that one of its neighbours has completed post-processing. If this completes the set of
// chunks which can actually set tile tiles in this chunk (sTerrainPopulatedAllAffecting), then this is a good point to compress this chunk.
// If this completes the set of all 8 neighbouring chunks that have been fully post-processed, then this is a good time to fix up some
// lighting things that need all the tiles to be in place in the region into which they might propagate.
void ServerChunkCache::flagPostProcessComplete(short flag, int x, int z)
{
	// Set any extra flags for this chunk to indicate which neighbours have now had their post-processing done
	if( !hasChunk(x, z) ) return;

	LevelChunk *lc = level->getChunk( x, z );
	if( lc == emptyChunk ) return;

	lc->terrainPopulated |= flag;

	// Are all neighbouring chunks which could actually place tiles on this chunk complete? (This is ones to W, SW, S)
	if( ( lc->terrainPopulated & LevelChunk::sTerrainPopulatedAllAffecting ) == LevelChunk::sTerrainPopulatedAllAffecting )
	{
		// Do the compression of data & lighting at this point
		PIXBeginNamedEvent(0,"Compressing lighting/blocks");

		// Check, using lower blocks as a reference, if we've already compressed - no point doing this multiple times, which
		// otherwise we will do as we aren't checking for the flags transitioning in the if statement we're in here
		if( !lc->isLowerBlockStorageCompressed() )
			lc->compressBlocks();
		if( !lc->isLowerBlockLightStorageCompressed() )
			lc->compressLighting();
		if( !lc->isLowerDataStorageCompressed() )
			lc->compressData();
		
		PIXEndNamedEvent();
	}

	// Are all neighbouring chunks And this one now post-processed?
	if( lc->terrainPopulated == LevelChunk::sTerrainPopulatedAllNeighbours )
	{
		// Special lighting patching for schematics first
		app.processSchematicsLighting(lc);

		// This would be a good time to fix up any lighting for this chunk since all the geometry that could affect it should now be in place
		PIXBeginNamedEvent(0,"Recheck gaps");
		if( lc->level->dimension->id != 1 )
		{
			lc->recheckGaps(true);
		}
		PIXEndNamedEvent();

		// Do a checkLight on any tiles which are lava.
		PIXBeginNamedEvent(0,"Light lava (this)");
		lc->lightLava();
		PIXEndNamedEvent();

		// Flag as now having this post-post-processing stage completed
		lc->terrainPopulated |= LevelChunk::sTerrainPostPostProcessed;
	}
}

void ServerChunkCache::postProcess(ChunkSource *parent, int x, int z )
{
#ifdef _LARGE_WORLDS
	// Don't run decoration/structures at out-of-bounds coordinates
	if (!m_isInfinite)
	{
		int half = m_XZSize / 2;
		if (x < -half || x >= half || z < -half || z >= half) return;
	}
#endif

    LevelChunk *chunk = getChunk(x, z);
    if ( (chunk->terrainPopulated & LevelChunk::sTerrainPopulatedFromHere) == 0 )
	{	
		if (source != NULL)
		{
			PIXBeginNamedEvent(0,"Main post processing");
            source->postProcess(parent, x, z);
			PIXEndNamedEvent();

            chunk->markUnsaved();
        }

		// Flag not only this chunk as being post-processed, but also all the chunks that this post-processing might affect. We can guarantee that these
		// chunks exist as that's determined before post-processing can even run
		chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromHere;

#ifdef _LARGE_WORLDS
		// For finite worlds, edge chunks have missing neighbours that will never post-process,
		// so fill in the appropriate flags. For infinite worlds, no edges exist.
		if (!m_isInfinite)
		{
			int half = m_XZSize / 2;
			if(x == -half)						// Furthest west
			{
				chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromW;
				chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromSW;
				chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromNW;
			}
			if(x == (half - 1))					// Furthest east
			{
				chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromE;
				chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromSE;
				chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromNE;
			}
			if(z == -half)						// Furthest south
			{
				chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromS;
				chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromSW;
				chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromSE;
			}
			if(z == (half - 1))					// Furthest north
			{
				chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromN;
				chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromNW;
				chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromNE;
			}
		}
#endif

		// Set flags for post-processing being complete for neighbouring chunks. This also performs actions if this post-processing completes
		// a full set of post-processing flags for one of these neighbours.
		flagPostProcessComplete(0, x, z );
		flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromW,  x + 1, z + 0 );
		flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromSW, x + 1, z + 1 );
		flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromS,  x + 0, z + 1 );
		flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromSE, x - 1, z + 1 );
		flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromE,  x - 1, z + 0 );
		flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromNE, x - 1, z - 1 );
		flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromN,  x + 0, z - 1 );
		flagPostProcessComplete(LevelChunk::sTerrainPopulatedFromNW, x + 1, z - 1 );
    }
}

// 4J Added for suspend
bool ServerChunkCache::saveAllEntities()
{
	PIXBeginNamedEvent(0, "Save all entities");

	PIXBeginNamedEvent(0, "saving to NBT");
	EnterCriticalSection(&m_csLoadCreate);
	for(AUTO_VAR(it,m_loadedChunkList.begin()); it != m_loadedChunkList.end(); ++it)
	{
		storage->saveEntities(level, *it);
	}
	LeaveCriticalSection(&m_csLoadCreate);
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Flushing");
	storage->flush();
	PIXEndNamedEvent();

	PIXEndNamedEvent();
	return true;
}

bool ServerChunkCache::save(bool force, ProgressListener *progressListener)
{
	EnterCriticalSection(&m_csLoadCreate);
    int saves = 0;

	// 4J - added this to support progressListner
	int count = 0;
    if (progressListener != NULL)
	{
        AUTO_VAR(itEnd, m_loadedChunkList.end());
		for (AUTO_VAR(it, m_loadedChunkList.begin()); it != itEnd; it++)
		{
			LevelChunk *chunk = *it;
            if (chunk->shouldSave(force))
			{
                count++;
            }
        }
    }
    int cc = 0;

	bool maxSavesReached = false;

	if(!force)
	{
		//app.DebugPrintf("Unsaved chunks = %d\n", level->getUnsavedChunkCount() );
		// Single threaded implementation for small saves
		for (unsigned int i = 0; i < m_loadedChunkList.size(); i++)
		{
			LevelChunk *chunk = m_loadedChunkList[i];
#ifndef SPLIT_SAVES
			if (force && !chunk->dontSave) saveEntities(chunk);
#endif
			if (chunk->shouldSave(force))
			{
				save(chunk);
				chunk->setUnsaved(false);
				if (++saves == MAX_SAVES && !force)
				{
					LeaveCriticalSection(&m_csLoadCreate);
					return false;
				}

				// 4J - added this to support progressListener
				if (progressListener != NULL)
				{
					if (++cc % 10 == 0)
					{
						progressListener->progressStagePercentage(cc * 100 / count);
					}
				}
			}
		}
	}
	else
	{
#if 1 //_LARGE_WORLDS
		// 4J Stu - We have multiple for threads for all saving as part of the storage, so use that rather than new threads here

		// Created a roughly sorted list to match the order that the files were created in 	McRegionChunkStorage::McRegionChunkStorage.
		// This is to minimise the amount of data that needs to be moved round when creating a new level.

		vector<LevelChunk *> sortedChunkList;

		for( int i = 0; i < m_loadedChunkList.size(); i++ )
		{
			if( ( m_loadedChunkList[i]->x < 0 ) && ( m_loadedChunkList[i]->z < 0 ) ) sortedChunkList.push_back(m_loadedChunkList[i]);
		}
		for( int i = 0; i < m_loadedChunkList.size(); i++ )
		{
			if( ( m_loadedChunkList[i]->x >= 0 ) && ( m_loadedChunkList[i]->z < 0 ) ) sortedChunkList.push_back(m_loadedChunkList[i]);
		}
		for( int i = 0; i < m_loadedChunkList.size(); i++ )
		{
			if( ( m_loadedChunkList[i]->x >= 0 ) && ( m_loadedChunkList[i]->z >= 0 ) ) sortedChunkList.push_back(m_loadedChunkList[i]);
		}
		for( int i = 0; i < m_loadedChunkList.size(); i++ )
		{
			if( ( m_loadedChunkList[i]->x < 0 ) && ( m_loadedChunkList[i]->z >= 0 ) ) sortedChunkList.push_back(m_loadedChunkList[i]);
		}

		// Push all the chunks to be saved to the compression threads
		for (unsigned int i = 0; i < sortedChunkList.size();++i)
		{
			LevelChunk *chunk = sortedChunkList[i];
			if (force && !chunk->dontSave) saveEntities(chunk);
			if (chunk->shouldSave(force))
			{
				save(chunk);
				chunk->setUnsaved(false);
				if (++saves == MAX_SAVES && !force)
				{
					LeaveCriticalSection(&m_csLoadCreate);
					return false;
				}

				// 4J - added this to support progressListener
				if (progressListener != NULL)
				{
					if (++cc % 10 == 0)
					{
						progressListener->progressStagePercentage(cc * 100 / count);
					}
				}
			}
			// Wait if we are building up too big a queue of chunks to be written - on PS3 this has been seen to cause so much data to be queued that we run out of
			// out of memory when saving after exploring a full map
			storage->WaitIfTooManyQueuedChunks();
		}

		// Wait for the storage threads to be complete
		storage->WaitForAll();
#else
		// Multithreaded implementation for larger saves

		C4JThread::Event *wakeEvent[3]; // This sets off the threads that are waiting to continue
		C4JThread::Event *notificationEvent[3]; // These are signalled by the threads to let us know they are complete
		C4JThread *saveThreads[3];
		DWORD threadId[3];
		SaveThreadData threadData[3];
		ZeroMemory(&threadData[0], sizeof(SaveThreadData));
		ZeroMemory(&threadData[1], sizeof(SaveThreadData));
		ZeroMemory(&threadData[2], sizeof(SaveThreadData));

		for(unsigned int i = 0; i < 3; ++i)
		{
			saveThreads[i] = NULL;
		
			threadData[i].cache = this;

			wakeEvent[i] = new C4JThread::Event(); //CreateEvent(NULL,FALSE,FALSE,NULL);
			threadData[i].wakeEvent = wakeEvent[i];
			
			notificationEvent[i] = new C4JThread::Event(); //CreateEvent(NULL,FALSE,FALSE,NULL);
			threadData[i].notificationEvent = notificationEvent[i];

			if(i==0) threadData[i].useSharedThreadStorage = true;
			else threadData[i].useSharedThreadStorage = false;
		}

		LevelChunk *chunk = NULL;
		byte workingThreads;
		bool chunkSet = false;

		// Created a roughly sorted list to match the order that the files were created in 	McRegionChunkStorage::McRegionChunkStorage.
		// This is to minimise the amount of data that needs to be moved round when creating a new level.

		vector<LevelChunk *> sortedChunkList;

		for( int i = 0; i < m_loadedChunkList.size(); i++ )
		{
			if( ( m_loadedChunkList[i]->x < 0 ) && ( m_loadedChunkList[i]->z < 0 ) ) sortedChunkList.push_back(m_loadedChunkList[i]);
		}
		for( int i = 0; i < m_loadedChunkList.size(); i++ )
		{
			if( ( m_loadedChunkList[i]->x >= 0 ) && ( m_loadedChunkList[i]->z < 0 ) ) sortedChunkList.push_back(m_loadedChunkList[i]);
		}
		for( int i = 0; i < m_loadedChunkList.size(); i++ )
		{
			if( ( m_loadedChunkList[i]->x >= 0 ) && ( m_loadedChunkList[i]->z >= 0 ) ) sortedChunkList.push_back(m_loadedChunkList[i]);
		}
		for( int i = 0; i < m_loadedChunkList.size(); i++ )
		{
			if( ( m_loadedChunkList[i]->x < 0 ) && ( m_loadedChunkList[i]->z >= 0 ) ) sortedChunkList.push_back(m_loadedChunkList[i]);
		}
		
		for (unsigned int i = 0; i < sortedChunkList.size();)
		{
			workingThreads = 0;
			PIXBeginNamedEvent(0,"Setting tasks for save threads\n");
			for(unsigned int j = 0; j < 3; ++j)
			{
				chunkSet = false;

				while(!chunkSet && i < sortedChunkList.size())
				{
					chunk = sortedChunkList[i];

					threadData[j].saveEntities = (force && !chunk->dontSave);

					if (chunk->shouldSave(force) || threadData[j].saveEntities)
					{
						chunkSet = true;
						++workingThreads;

						threadData[j].chunkToSave = chunk;

						//app.DebugPrintf("Chunk to save set for thread %d\n", j);

						if(saveThreads[j] == NULL)
						{
							char threadName[256];
							sprintf(threadName,"Save thread %d\n",j);
							SetThreadName(threadId[j], threadName);

							//saveThreads[j] = CreateThread(NULL,0,runSaveThreadProc,&threadData[j],CREATE_SUSPENDED,&threadId[j]);
							saveThreads[j] = new C4JThread(runSaveThreadProc,(void *)&threadData[j],threadName);


							//app.DebugPrintf("Created new thread: %s\n",threadName);

							// Threads 1,3 and 5 are generally idle so use them (this call waits on thread 2)
							if(j == 0) saveThreads[j]->SetProcessor(CPU_CORE_SAVE_THREAD_A); //XSetThreadProcessor( saveThreads[j], 1);
							else if(j == 1) saveThreads[j]->SetProcessor(CPU_CORE_SAVE_THREAD_B); //XSetThreadProcessor( saveThreads[j], 3);
							else if(j == 2) saveThreads[j]->SetProcessor(CPU_CORE_SAVE_THREAD_C); //XSetThreadProcessor( saveThreads[j], 5);

							//ResumeThread( saveThreads[j] );
							saveThreads[j]->Run();
						}

						if (++saves == MAX_SAVES && !force)
						{
							maxSavesReached = true;
							break;

							//LeaveCriticalSection(&m_csLoadCreate);
							// TODO Should we be returning from here? Probably not
							//return false;
						}

						// 4J - added this to support progressListener
						if (progressListener != NULL)
						{
							if (count > 0 && ++cc % 10 == 0)
							{
								progressListener->progressStagePercentage(cc * 100 / count);
							}
						}
					}

					++i;
				}

				if( !chunkSet )
				{
					threadData[j].chunkToSave = NULL;
					//app.DebugPrintf("No chunk to save set for thread %d\n",j);
				}
			}
			PIXEndNamedEvent();
			PIXBeginNamedEvent(0,"Waking save threads\n");
			// Start the worker threads going
			for(unsigned int k = 0; k < 3; ++k)
			{
				//app.DebugPrintf("Waking save thread %d\n",k);
				threadData[k].wakeEvent->Set(); //SetEvent(threadData[k].wakeEvent);
			}
			PIXEndNamedEvent();
			PIXBeginNamedEvent(0,"Waiting for completion of save threads\n");
			//app.DebugPrintf("Waiting for %d save thread(s) to complete\n", workingThreads);

			// Wait for the worker threads to complete
			//WaitForMultipleObjects(workingThreads,notificationEvent,TRUE,INFINITE);
			// 4J Stu - TODO This isn't ideal as it's not a perfect re-implmentation of the Xbox behaviour
			for(unsigned int k = 0; k < workingThreads; ++k)
			{
				threadData[k].notificationEvent->WaitForSignal(INFINITE);
			}
			PIXEndNamedEvent();
			if( maxSavesReached ) break;
		}

		//app.DebugPrintf("Clearing up worker threads\n");
		// Stop all the worker threads by giving them nothing to process then telling them to start
		unsigned char validThreads = 0;
		for(unsigned int i = 0; i < 3; ++i)
		{
			//app.DebugPrintf("Settings chunk to NULL for save thread %d\n", i);
			threadData[i].chunkToSave = NULL;

			//app.DebugPrintf("Setting wake event for save thread %d\n",i);
			threadData[i].wakeEvent->Set(); //SetEvent(threadData[i].wakeEvent);

			if(saveThreads[i] != NULL) ++validThreads;
		}

		//WaitForMultipleObjects(validThreads,saveThreads,TRUE,INFINITE);
		// 4J Stu - TODO This isn't ideal as it's not a perfect re-implmentation of the Xbox behaviour
		for(unsigned int k = 0; k < validThreads; ++k)
		{
			saveThreads[k]->WaitForCompletion(INFINITE);;
		}

		for(unsigned int i = 0; i < 3; ++i)
		{
			//app.DebugPrintf("Closing handles for save thread %d\n", i);
			delete threadData[i].wakeEvent; //CloseHandle(threadData[i].wakeEvent);
			delete threadData[i].notificationEvent; //CloseHandle(threadData[i].notificationEvent);
			delete saveThreads[i]; //CloseHandle(saveThreads[i]);
		}
#endif
	}

    if (force)
	{
        if (storage == NULL)
		{
			LeaveCriticalSection(&m_csLoadCreate);
			return true;
		}
        storage->flush();
    }

	LeaveCriticalSection(&m_csLoadCreate);
    return !maxSavesReached;

}

bool ServerChunkCache::tick()
{
    if (!level->noSave)
	{
#ifdef _LARGE_WORLDS
		for (int i = 0; i < 100; i++)
		{
			if (!m_toDrop.empty())
			{
				LevelChunk *chunk = m_toDrop.front();
				if(!chunk->isUnloaded())
				{
				save(chunk);
				saveEntities(chunk);
				chunk->unload(true);

				EnterCriticalSection(&m_csLoadCreate);
				AUTO_VAR(it, std::find( m_loadedChunkList.begin(), m_loadedChunkList.end(), chunk) );
				if(it != m_loadedChunkList.end()) m_loadedChunkList.erase(it);

				int64_t key = chunkKey(chunk->x, chunk->z);
				// Move from live map to unloaded map; data stays in RAM
				m_unloadedMap[key] = chunk;
				m_chunkMap.erase(key);
				LeaveCriticalSection(&m_csLoadCreate);
				}
				m_toDrop.pop_front();
			}
		}
#endif
        if (storage != NULL) storage->tick();
    }

    return source->tick();

}

bool ServerChunkCache::shouldSave()
{
	return !level->noSave;
}

wstring ServerChunkCache::gatherStats()
{
	 return L"ServerChunkCache: ";// + _toString<int>(loadedChunks.size()) + L" Drop: " + _toString<int>(toDrop.size());
}

vector<Biome::MobSpawnerData *> *ServerChunkCache::getMobsAt(MobCategory *mobCategory, int x, int y, int z)
{
	return source->getMobsAt(mobCategory, x, y, z);
}

TilePos *ServerChunkCache::findNearestMapFeature(Level *level, const wstring &featureName, int x, int y, int z)
{
	return source->findNearestMapFeature(level, featureName, x, y, z);
}

int ServerChunkCache::runSaveThreadProc(LPVOID lpParam)
{
	SaveThreadData *params = (SaveThreadData *)lpParam;

	if(params->useSharedThreadStorage)
	{
		Compression::UseDefaultThreadStorage();
		OldChunkStorage::UseDefaultThreadStorage();
	}
	else
	{
		Compression::CreateNewThreadStorage();
		OldChunkStorage::CreateNewThreadStorage();
	}

	// Wait for the producer thread to tell us to start
	params->wakeEvent->WaitForSignal(INFINITE); //WaitForSingleObject(params->wakeEvent,INFINITE);

	//app.DebugPrintf("Save thread has started\n");

	while(params->chunkToSave != NULL)
	{
		PIXBeginNamedEvent(0,"Saving entities");
		//app.DebugPrintf("Save thread has started processing a chunk\n");
		if (params->saveEntities) params->cache->saveEntities(params->chunkToSave);
		PIXEndNamedEvent();
		PIXBeginNamedEvent(0,"Saving chunk");

		params->cache->save(params->chunkToSave);
		params->chunkToSave->setUnsaved(false);

		PIXEndNamedEvent();
		PIXBeginNamedEvent(0,"Notifying and waiting for next chunk");

		// Inform the producer thread that we are done with this chunk
		params->notificationEvent->Set(); //SetEvent(params->notificationEvent);

		//app.DebugPrintf("Save thread has alerted producer that it is complete\n");

		// Wait for the producer thread to tell us to go again
		params->wakeEvent->WaitForSignal(INFINITE); //WaitForSingleObject(params->wakeEvent,INFINITE);
		PIXEndNamedEvent();
	}

	//app.DebugPrintf("Thread is exiting as it has no chunk to process\n");

	if(!params->useSharedThreadStorage)
	{
		Compression::ReleaseThreadStorage();
		OldChunkStorage::ReleaseThreadStorage();
	}

	return 0;
}
