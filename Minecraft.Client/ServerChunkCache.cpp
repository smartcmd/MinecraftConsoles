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
#include "..\Minecraft.World\Tile.h"

ServerChunkCache::ServerChunkCache(ServerLevel *level, ChunkStorage *storage, ChunkSource *source)
{
	XZSIZE = source->m_XZSize; // 4J Added
	XZOFFSET = XZSIZE/2; // 4J Added

	autoCreate = false;	// 4J added

	emptyChunk = new EmptyLevelChunk(level, byteArray( Level::CHUNK_TILE_COUNT ), 0, 0);

    this->level = level;
    this->storage = storage;
    this->source = source;
	this->m_XZSize = source->m_XZSize;

	this->cache = new LevelChunk *[XZSIZE * XZSIZE];
	memset(this->cache, 0, XZSIZE * XZSIZE * sizeof(LevelChunk *));

#ifdef _LARGE_WORLDS
	m_unloadedCache = new LevelChunk *[XZSIZE * XZSIZE];
	memset(m_unloadedCache, 0, XZSIZE * XZSIZE * sizeof(LevelChunk *));
#endif

	InitializeCriticalSectionAndSpinCount(&m_csLoadCreate,4000);
}

// 4J-PB added
ServerChunkCache::~ServerChunkCache()
{
	storage->WaitForAll();		// MGH -  added to fix crash bug 175183
	delete emptyChunk;
	delete cache;
	delete source;

#ifdef _LARGE_WORLDS
	for(unsigned int i = 0; i < XZSIZE * XZSIZE; ++i)
	{
		delete m_unloadedCache[i];
	}
	delete m_unloadedCache;
#endif

	for (auto& it : m_loadedChunkList)
		delete it;
	DeleteCriticalSection(&m_csLoadCreate);
}

bool ServerChunkCache::hasChunk(int x, int z)
{
	int ix = x + XZOFFSET;
	int iz = z + XZOFFSET;
	// Check we're in range of the stored level
	// 4J Stu - Request for chunks outside the range always return an emptyChunk, so just return true here to say we have it
	// If we return false entities less than 2 chunks from the edge do not tick properly due to them requiring a certain radius
	// of chunks around them when they tick
	if( ( ix < 0 ) || ( ix >= XZSIZE ) ) return true;
	if( ( iz < 0 ) || ( iz >= XZSIZE ) ) return true;
	int idx = ix * XZSIZE + iz;
	LevelChunk *lc = cache[idx];
	if( lc == NULL ) return false;
	return true;
}

vector<LevelChunk *> *ServerChunkCache::getLoadedChunkList()
{
	return &m_loadedChunkList;
}

void ServerChunkCache::drop(int x, int z)
{
	// 4J - we're not dropping things anymore now that we have a fixed sized cache
#ifdef _LARGE_WORLDS

	bool canDrop = false;
//	if (level->dimension->mayRespawn())
//	{
//		Pos *spawnPos = level->getSharedSpawnPos();
//		int xd = x * 16 + 8 - spawnPos->x;
//		int zd = z * 16 + 8 - spawnPos->z;
//		delete spawnPos;
//		int r = 128;
//		if (xd < -r || xd > r || zd < -r || zd > r)
//		{
//			canDrop = true;
//}
//	}
//	else
	{
		canDrop = true;
	}
	if(canDrop)
	{
		int ix = x + XZOFFSET;
		int iz = z + XZOFFSET;
		// Check we're in range of the stored level
		if( ( ix < 0 ) || ( ix >= XZSIZE ) ) return;
		if( ( iz < 0 ) || ( iz >= XZSIZE ) ) return;
		int idx = ix * XZSIZE + iz;
		LevelChunk *chunk = cache[idx];

		if(chunk)
		{
			m_toDrop.push_back(chunk);
		}
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
	int ix = x + XZOFFSET;
	int iz = z + XZOFFSET;
	// Check we're in range of the stored level
	if( ( ix < 0 ) || ( ix >= XZSIZE ) ) return emptyChunk;
	if( ( iz < 0 ) || ( iz >= XZSIZE ) ) return emptyChunk;
	int idx = ix * XZSIZE + iz;

	LevelChunk *chunk = cache[idx];
	LevelChunk *lastChunk = chunk;

	if( ( chunk == NULL ) || ( chunk->x != x ) || ( chunk->z != z ) )
	{
		EnterCriticalSection(&m_csLoadCreate);
        chunk = load(x, z);
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

		LeaveCriticalSection(&m_csLoadCreate);

#if ( defined _WIN64 || defined __LP64__ )
		if( InterlockedCompareExchangeRelease64((LONG64 *)&cache[idx],(LONG64)chunk,(LONG64)lastChunk) == (LONG64)lastChunk )
#else
		if( InterlockedCompareExchangeRelease((LONG *)&cache[idx],(LONG)chunk,(LONG)lastChunk) == (LONG)lastChunk )
#endif // _DURANGO
		{
			// Successfully updated the cache
			EnterCriticalSection(&m_csLoadCreate);
			// 4J - added - this will run a recalcHeightmap if source is a randomlevelsource, which has been split out from source::getChunk so that
			// we are doing it after the chunk has been added to the cache - otherwise a lot of the lighting fails as lights aren't added if the chunk
			// they are in fail ServerChunkCache::hasChunk.
			source->lightChunk(chunk);

			updatePostProcessFlags( x, z );

			m_loadedChunkList.push_back(chunk);

			// 4J - If post-processing is to be async, then let the server know about requests rather than processing directly here. Note that
			// these hasChunk() checks appear to be incorrect - the chunks checked by these map out as:
			//
			// 1.		2.		3.		4.
			// oxx		xxo		ooo		ooo
			// oPx		Poo		oox		xoo
			// ooo		ooo		oPx		Pxo
			//
			// where P marks the chunk that is being considered for postprocessing, and x marks chunks that needs to be loaded. It would seem that the
			// chunks which need to be loaded should stay the same relative to the chunk to be processed, but the hasChunk checks in 3 cases check again
			// the chunk which is to be processed itself rather than (what I presume to be) the correct position.
			// Don't think we should change in case it alters level creation.

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

			// 4J - Now try and fix up any chests that were saved pre-1.8.2. We don't want to do this to this particular chunk as we don't know if all its neighbours are loaded yet, and we
			// need the neighbours to be able to work out the facing direction for the chests. Therefore process any neighbouring chunk that loading this chunk would be the last neighbour for.
			// 5 cases illustrated below, where P is the chunk to be processed, T is this chunk, and x are other chunks that need to be checked for being present

			// 1.		2.		3.		4.		5.
			// ooooo	ooxoo	ooooo	ooooo	ooooo
			// oxooo	oxPxo	oooxo	ooooo	ooxoo
			// xPToo	ooToo	ooTPx	ooToo	oxPxo	(in 5th case P and T are same)
			// oxooo	ooooo	oooxo	oxPxo	ooxoo
			// ooooo	ooooo	ooooo	ooxoo	ooooo

			if( hasChunk( x - 1, z ) && hasChunk( x - 2, z ) && hasChunk( x - 1, z + 1 ) && hasChunk( x - 1, z - 1 ) ) chunk->checkChests( this, x - 1, z );
			if( hasChunk( x, z + 1) && hasChunk( x , z + 2 ) && hasChunk( x - 1, z + 1 ) && hasChunk( x + 1, z + 1 ) ) chunk->checkChests( this, x, z + 1);
			if( hasChunk( x + 1, z ) && hasChunk( x + 2, z ) && hasChunk( x + 1, z + 1 ) && hasChunk( x + 1, z - 1 ) ) chunk->checkChests( this, x + 1, z );
			if( hasChunk( x, z - 1) && hasChunk( x , z - 2 ) && hasChunk( x - 1, z - 1 ) && hasChunk( x + 1, z - 1 ) ) chunk->checkChests( this, x, z - 1);
			if( hasChunk( x - 1, z ) && hasChunk( x + 1, z ) && hasChunk ( x, z - 1 ) && hasChunk( x, z + 1 ) ) chunk->checkChests( this, x, z );

			LeaveCriticalSection(&m_csLoadCreate);
		}
		else
		{
			// Something else must have updated the cache. Return that chunk and discard this one
			chunk->unload(true);
			delete chunk;
			return cache[idx];
		}
    }

#ifdef __PS3__
	Sleep(1);
#endif // __PS3__
    return chunk;

}

// 4J Stu - Split out this function so that we get a chunk without loading entities
// This is used when sharing server chunk data on the main thread
LevelChunk *ServerChunkCache::getChunk(int x, int z)
{
	int ix = x + XZOFFSET;
	int iz = z + XZOFFSET;
	// Check we're in range of the stored level
	if( ( ix < 0 ) || ( ix >= XZSIZE ) ) return emptyChunk;
	if( ( iz < 0 ) || ( iz >= XZSIZE ) ) return emptyChunk;
	int idx = ix * XZSIZE + iz;

	LevelChunk *lc = cache[idx];
	if( lc )
	{
		return lc;
	}

	if( level->isFindingSpawn || autoCreate )
	{
		return create(x, z);
	}

	return emptyChunk;
}

#ifdef _LARGE_WORLDS
// 4J added - this special variation on getChunk also checks the unloaded chunk cache. It is called on a host machine from the client-side level when:
// (1) Trying to determine whether the client blocks and data are the same as those on the server, so we can start sharing them
// (2) Trying to resync the lighting data from the server to the client
// As such it is really important that we don't return emptyChunk in these situations, when we actually still have the block/data/lighting in the unloaded cache
LevelChunk *ServerChunkCache::getChunkLoadedOrUnloaded(int x, int z)
{
	int ix = x + XZOFFSET;
	int iz = z + XZOFFSET;
	// Check we're in range of the stored level
	if( ( ix < 0 ) || ( ix >= XZSIZE ) ) return emptyChunk;
	if( ( iz < 0 ) || ( iz >= XZSIZE ) ) return emptyChunk;
	int idx = ix * XZSIZE + iz;

	LevelChunk *lc = cache[idx];
	if( lc )
	{
		return lc;
	}

	lc = m_unloadedCache[idx];
	if( lc )
	{
		return lc;
}

	if( level->isFindingSpawn || autoCreate )
	{
		return create(x, z);
	}

	return emptyChunk;
}
#endif


// 4J MGH added, for expanding worlds, to kill any player changes and reset the chunk
#ifdef _LARGE_WORLDS
void ServerChunkCache::overwriteLevelChunkFromSource(int x, int z)
{
	int ix = x + XZOFFSET;
	int iz = z + XZOFFSET;
	// Check we're in range of the stored level
	if( ( ix < 0 ) || ( ix >= XZSIZE ) ) assert(0);
	if( ( iz < 0 ) || ( iz >= XZSIZE ) ) assert(0);
	int idx = ix * XZSIZE + iz;

	LevelChunk *chunk = NULL;
	chunk = source->getChunk(x, z);
	assert(chunk);
	if(chunk)
	{
		save(chunk);
	}
}



void ServerChunkCache::updateOverwriteHellChunk(LevelChunk* origChunk, LevelChunk* playerChunk, int xMin, int xMax, int zMin, int zMax)
{

	// replace a section of the chunk with the original source data, if it hasn't already changed
	for(int x=xMin;x<xMax;x++)
	{
		for(int z=zMin;z<zMax;z++)
		{
			for(int y=0;y<256;y++)
			{
				int playerTile = playerChunk->getTile(x,y,z);
				if(playerTile == Tile::unbreakable_Id)	// if the tile is still unbreakable, the player hasn't changed it, so we can replace with the source
					playerChunk->setTileAndData(x, y, z, origChunk->getTile(x,y,z), origChunk->getData(x,y,z));
			}
		}
	}
}

void ServerChunkCache::overwriteHellLevelChunkFromSource(int x, int z, int minVal, int maxVal)
{
	int ix = x + XZOFFSET;
	int iz = z + XZOFFSET;
	// Check we're in range of the stored level
	if( ( ix < 0 ) || ( ix >= XZSIZE ) ) assert(0);
	if( ( iz < 0 ) || ( iz >= XZSIZE ) ) assert(0);
	int idx = ix * XZSIZE + iz;
	autoCreate = true;
	LevelChunk* playerChunk = getChunk(x, z);
	autoCreate = false;
	LevelChunk* origChunk = source->getChunk(x, z);
	assert(origChunk);
	if(playerChunk!= emptyChunk)
	{
		if(x == minVal)
			updateOverwriteHellChunk(origChunk, playerChunk, 0, 4, 0, 16);
		if(x == maxVal)
			updateOverwriteHellChunk(origChunk, playerChunk, 12, 16, 0, 16);
		if(z == minVal)
			updateOverwriteHellChunk(origChunk, playerChunk, 0, 16, 0, 4);
		if(z == maxVal)
			updateOverwriteHellChunk(origChunk, playerChunk, 0, 16, 12, 16);
	}
	save(playerChunk);
}

#endif

// 4J Added //
#ifdef _LARGE_WORLDS
void ServerChunkCache::dontDrop(int x, int z)
{
	LevelChunk *chunk = getChunk(x,z);
	m_toDrop.erase(std::remove(m_toDrop.begin(), m_toDrop.end(), chunk), m_toDrop.end());
}
#endif

LevelChunk *ServerChunkCache::load(int x, int z)
{
    if (storage == NULL) return NULL;

    LevelChunk *levelChunk = NULL;

#ifdef _LARGE_WORLDS
	int ix = x + XZOFFSET;
	int iz = z + XZOFFSET;
	int idx = ix * XZSIZE + iz;
	levelChunk = m_unloadedCache[idx];
	m_unloadedCache[idx] = NULL;
	if(levelChunk == NULL)
#endif
	{
		levelChunk = storage->load(level, x, z);
	}
    if (levelChunk != NULL)
	{
        levelChunk->lastSaveTime = level->getGameTime();
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

	levelChunk->lastSaveTime = level->getGameTime();
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

		// If we are an edge chunk, fill in missing flags from sides that will never post-process
		if(x == -XZOFFSET)						// Furthest west
		{
			chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromW;
			chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromSW;
			chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromNW;
		}
		if(x == (XZOFFSET - 1 ))				// Furthest east
		{
			chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromE;
			chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromSE;
			chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromNE;
		}
		if(z == -XZOFFSET)						// Furthest south
		{
			chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromS;
			chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromSW;
			chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromSE;
		}
		if(z == (XZOFFSET - 1))					// Furthest north
		{
			chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromN;
			chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromNW;
			chunk->terrainPopulated |= LevelChunk::sTerrainPopulatedFromNE;
		}

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
	for(auto& it : m_loadedChunkList)
	{
		storage->saveEntities(level, it);
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
    if (progressListener)
	{
		for (LevelChunk *chunk : m_loadedChunkList)
		{
			if ( chunk && chunk->shouldSave(force))
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
					// Don't unload a chunk that contains a player, as this will cause their entity to be removed from the level itself and they will never tick again.
					// This can happen if a player moves a long distance in one tick, for example when the server thread has locked up doing something for a while whilst a player
					// kept moving. In this case, the player is moved in the player chunk map (driven by the network packets being processed for their new position) before the
					// player's tick is called to remove them from the chunk they used to be in, and add them to their current chunk. This will only be a temporary state and
					// we should be able to unload the chunk on the next call to this tick.
					if( !chunk->containsPlayer() )
					{
						save(chunk);
						saveEntities(chunk);
						chunk->unload(true);

						//loadedChunks.remove(cp);
						//loadedChunkList.remove(chunk);
                        auto it = std::find(m_loadedChunkList.begin(), m_loadedChunkList.end(), chunk);
                        if(it != m_loadedChunkList.end()) m_loadedChunkList.erase(it);

						int ix = chunk->x + XZOFFSET;
						int iz = chunk->z + XZOFFSET;
						int idx = ix * XZSIZE + iz;
						m_unloadedCache[idx] = chunk;
						cache[idx] = NULL;
					}
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
	 return L"ServerChunkCache: ";// + std::to_wstring(loadedChunks.size()) + L" Drop: " + std::to_wstring(toDrop.size());
}

vector<Biome::MobSpawnerData *> *ServerChunkCache::getMobsAt(MobCategory *mobCategory, int x, int y, int z)
{
	return source->getMobsAt(mobCategory, x, y, z);
}

TilePos *ServerChunkCache::findNearestMapFeature(Level *level, const wstring &featureName, int x, int y, int z)
{
	return source->findNearestMapFeature(level, featureName, x, y, z);
}

void ServerChunkCache::recreateLogicStructuresForChunk(int chunkX, int chunkZ)
{
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
