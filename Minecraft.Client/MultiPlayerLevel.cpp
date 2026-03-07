#include "stdafx.h"
#include "MultiPlayerLevel.h"
#include "MultiPlayerLocalPlayer.h"
#include "ClientConnection.h"
#include "MultiPlayerChunkCache.h"
#include "../Minecraft.World/net.minecraft.world.level.storage.h"
#include "../Minecraft.World/net.minecraft.world.level.dimension.h"
#include "../Minecraft.World/Pos.h"
#include "MinecraftServer.h"
#include "ServerLevel.h"
#include "Minecraft.h"
#include "FireworksParticles.h"
#include "../Minecraft.World/PrimedTnt.h"
#include "../Minecraft.World/Tile.h"
#include "../Minecraft.World/TileEntity.h"
#include "../Minecraft.World/JavaMath.h"

MultiPlayerLevel::ResetInfo::ResetInfo(int x, int y, int z, int tile, int data)
{
	this->x = x;
	this->y = y;
	this->z = z;
	ticks = TICKS_BEFORE_RESET;
	this->tile = tile;
	this->data = data;
}

MultiPlayerLevel::MultiPlayerLevel(ClientConnection *connection, LevelSettings *levelSettings, int dimension, int difficulty)
	: Level(shared_ptr<MockedLevelStorage >(new MockedLevelStorage()), L"MpServer", Dimension::getNew(dimension), levelSettings, false)
{
	minecraft = Minecraft::GetInstance();

	// 4J - this this used to be called in parent ctor via a virtual fn
	chunkSource = createChunkSource();
	// 4J - optimisation - keep direct reference of underlying cache here
	chunkSourceCache = chunkSource->getCache();
	chunkSourceXZSize = chunkSource->m_XZSize;

	// This also used to be called in parent ctor, but can't be called until chunkSource is created. Call now if required.
	if (!levelData->isInitialized())
	{
		initializeLevel(levelSettings);
		levelData->setInitialized(true);
	}

	if(connection !=NULL)
	{
		this->connections.push_back( connection );
	}
	this->difficulty = difficulty;
	// Fix for #62566 - TU7: Content: Gameplay: Compass needle stops pointing towards the original spawn point, once the player has entered the Nether.
	// 4J Stu - We should never be setting a specific spawn position for a multiplayer, this should only be set by receiving a packet from the server
	// (which happens when a player logs in)
	//setSpawnPos(new Pos(8, 64, 8));
	// The base ctor already has made some storage, so need to delete that
	if( this->savedDataStorage ) delete savedDataStorage;
	if(connection !=NULL)
	{
		savedDataStorage = connection->savedDataStorage;
	}
	unshareCheckX = 0;
	unshareCheckZ = 0;
	compressCheckX = 0;
	compressCheckZ = 0;

	// 4J Added, as there are some times when we don't want to add tile updates to the updatesToReset vector
	m_bEnableResetChanges = true;
}

MultiPlayerLevel::~MultiPlayerLevel()
{
	// Don't let the base class delete this, it comes from the connection for multiplayerlevels, and we'll delete there
	this->savedDataStorage = NULL;
}

void MultiPlayerLevel::unshareChunkAt(int x, int z)
{
	if( g_NetworkManager.IsHost() )
	{
		Level::getChunkAt(x,z)->stopSharingTilesAndData();
	}
}

void MultiPlayerLevel::shareChunkAt(int x, int z)
{
	if( g_NetworkManager.IsHost() )
	{
		Level::getChunkAt(x,z)->startSharingTilesAndData();
	}
}


void MultiPlayerLevel::tick()
{
	PIXBeginNamedEvent(0,"Sky color changing");
	setGameTime(getGameTime() + 1);
	if (getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT))
	{
		// 4J: Debug setting added to keep it at day time
#ifndef _FINAL_BUILD
		bool freezeTime = app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_FreezeTime);
		if (!freezeTime)
#endif
		{
			setDayTime(getDayTime() + 1);
		}
	}
	/* 4J - change brought forward from 1.8.2
	int newDark = this->getSkyDarken(1);
	if (newDark != skyDarken)
	{
	skyDarken = newDark;
	for (unsigned int i = 0; i < listeners.size(); i++)
	{
	listeners[i]->skyColorChanged();
	}
	}*/
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Entity re-entry");
	EnterCriticalSection(&m_entitiesCS);
	for (int i = 0; i < 10 && !reEntries.empty(); i++)
	{
		shared_ptr<Entity> e = *(reEntries.begin());

		if (find(entities.begin(), entities.end(), e) == entities.end() ) addEntity(e);
	}
	LeaveCriticalSection(&m_entitiesCS);
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Connection ticking");
	// 4J HEG - Copy the connections vector to prevent crash when moving to Nether
	vector<ClientConnection *> connectionsTemp = connections;
    for (auto connection : connectionsTemp )
    {
		if ( connection )
			connection->tick();
	}
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Updating resets");
	unsigned int lastIndexToRemove = 0;
	bool eraseElements = false;
	for (unsigned int i = 0; i < updatesToReset.size(); i++)
	{
		ResetInfo& r = updatesToReset[i];
		if (--r.ticks == 0)
		{
			Level::setTileAndData(r.x, r.y, r.z, r.tile, r.data, Tile::UPDATE_ALL);
			Level::sendTileUpdated(r.x, r.y, r.z);

			//updatesToReset.erase(updatesToReset.begin()+i);
			eraseElements = true;
			lastIndexToRemove = 0;

			i--;
		}
	}
	// 4J Stu - As elements in the updatesToReset vector are inserted with a fixed initial lifetime, the elements at the front should always be the oldest
	// Therefore we can always remove from the first element
	if(eraseElements)
	{
		updatesToReset.erase(updatesToReset.begin(), updatesToReset.begin()+lastIndexToRemove);
	}
	PIXEndNamedEvent();

	chunkCache->tick();
	tickTiles();

	// 4J - added this section. Each tick we'll check a different block, and force it to share data if it has been
	// more than 2 minutes since we last wanted to unshare it. This shouldn't really ever happen, and is added
	// here as a safe guard against accumulated memory leaks should a lot of chunks become unshared over time.

	int ls = dimension->getXZSize();
	if( g_NetworkManager.IsHost() )
	{
		if( Level::reallyHasChunk(unshareCheckX - ( ls / 2), unshareCheckZ - ( ls / 2 ) ) )
		{
			LevelChunk *lc = Level::getChunk(unshareCheckX - ( ls / 2), unshareCheckZ - ( ls / 2 ));
			if( g_NetworkManager.IsHost() )
			{
				lc->startSharingTilesAndData(1000 * 60 * 2);
			}
		}

		unshareCheckX++;
		if( unshareCheckX >= ls )
		{
			unshareCheckX = 0;
			unshareCheckZ++;
			if( unshareCheckZ >= ls )
			{
				unshareCheckZ = 0;
			}
		}
	}

	// 4J added - also similar thing tosee if we can compress the lighting in any of these chunks. This is slightly different
	// as it does try to make sure that at least one chunk has something done to it.

	// At most loop round at least one row the chunks, so we should be able to at least find a non-empty chunk to do something with in 2.7 seconds of ticks, and process the whole thing in about 2.4 minutes.
	for( int i = 0; i < ls; i++ )
	{
		compressCheckX++;
		if( compressCheckX >= ls )
		{
			compressCheckX = 0;
			compressCheckZ++;
			if( compressCheckZ >= ls )
			{
				compressCheckZ = 0;
			}
		}

		if( Level::reallyHasChunk(compressCheckX - ( ls / 2), compressCheckZ - ( ls / 2 ) ) )
		{
			LevelChunk *lc = Level::getChunk(compressCheckX - ( ls / 2), compressCheckZ - ( ls / 2 ));
			lc->compressLighting();
			lc->compressBlocks();
			lc->compressData();
			break;
		}
	}

#ifdef LIGHT_COMPRESSION_STATS
	static int updateTick = 0;

	if( ( updateTick % 60 ) == 0 )
	{
		unsigned int totalBLu = 0;
		unsigned int totalBLl = 0;
		unsigned int totalSLu = 0;
		unsigned int totalSLl = 0;
		unsigned int totalChunks = 0;

		for( int lcs_x = 0; lcs_x < ls; lcs_x++ )
			for( int lcs_z = 0; lcs_z < ls; lcs_z++ )
			{
				if( Level::reallyHasChunk(lcs_x - ( ls / 2), lcs_z - ( ls / 2 ) ) )
				{
					LevelChunk *lc = Level::getChunk(lcs_x - ( ls / 2), lcs_z - ( ls / 2 ));
					totalChunks++;
					totalBLu += lc->getBlockLightPlanesUpper();
					totalBLl += lc->getBlockLightPlanesLower();
					totalSLu += lc->getSkyLightPlanesUpper();
					totalSLl += lc->getSkyLightPlanesLower();
				}
			}
			if( totalChunks )
			{
				MEMORYSTATUS memStat;
				GlobalMemoryStatus(&memStat);

				unsigned int totalBL = totalBLu + totalBLl;
				unsigned int totalSL = totalSLu + totalSLl;
				printf("%d: %d chunks, %d BL (%d + %d), %d SL (%d + %d ) (out of %d) - total %d %% (%dMB mem free)\n",
					dimension->id, totalChunks, totalBL, totalBLu, totalBLl, totalSL, totalSLu, totalSLl, totalChunks * 256, ( 100 * (totalBL + totalSL) ) / ( totalChunks * 256 * 2),memStat.dwAvailPhys/(1024*1024) );
			}
	}
	updateTick++;

#endif

#ifdef DATA_COMPRESSION_STATS
	static int updateTick = 0;

	if( ( updateTick % 60 ) == 0 )
	{
		unsigned int totalData = 0;
		unsigned int totalChunks = 0;

		for( int lcs_x = 0; lcs_x < ls; lcs_x++ )
			for( int lcs_z = 0; lcs_z < ls; lcs_z++ )
			{
				if( Level::reallyHasChunk(lcs_x - ( ls / 2), lcs_z - ( ls / 2 ) ) )
				{
					LevelChunk *lc = Level::getChunk(lcs_x - ( ls / 2), lcs_z - ( ls / 2 ));
					totalChunks++;
					totalData += lc->getDataPlanes();
				}
			}
			if( totalChunks )
			{
				MEMORYSTATUS memStat;
				GlobalMemoryStatus(&memStat);

				printf("%d: %d chunks, %d data (out of %d) - total %d %% (%dMB mem free)\n",
					dimension->id, totalChunks, totalData, totalChunks * 128, ( 100 * totalData)/ ( totalChunks * 128),memStat.dwAvailPhys/(1024*1024) );
			}
	}
	updateTick++;

#endif

#ifdef BLOCK_COMPRESSION_STATS
	static int updateTick = 0;

	if( ( updateTick % 60 ) == 0 )
	{
		unsigned int total = 0;
		unsigned int totalChunks = 0;
		unsigned int total0 = 0, total1 = 0, total2 = 0, total4 = 0, total8 = 0;

		printf("*****************************************************************************************************************************************\n");
		printf("TODO: Report upper chunk data as well\n");
		for( int lcs_x = 0; lcs_x < ls; lcs_x++ )
			for( int lcs_z = 0; lcs_z < ls; lcs_z++ )
			{
				if( Level::reallyHasChunk(lcs_x - ( ls / 2), lcs_z - ( ls / 2 ) ) )
				{
					LevelChunk *lc = Level::getChunk(lcs_x - ( ls / 2), lcs_z - ( ls / 2 ));
					totalChunks++;
					int i0, i1, i2, i4, i8;
					int thisSize = lc->getBlocksAllocatedSize(&i0, &i1, &i2, &i4, &i8);
					total0 += i0;
					total1 += i1;
					total2 += i2;
					total4 += i4;
					total8 += i8;
					printf("%d ",thisSize);
					thisSize = ( thisSize + 0xfff ) & 0xfffff000;		// round to 4096k blocks for actual memory consumption
					total += thisSize;
				}
			}
			printf("\n*****************************************************************************************************************************************\n");
			if( totalChunks )
			{
				printf("%d (0) %d (1) %d (2) %d (4) %d (8)\n",total0/totalChunks,total1/totalChunks,total2/totalChunks,total4/totalChunks,total8/totalChunks);
				MEMORYSTATUS memStat;
				GlobalMemoryStatus(&memStat);

				printf("%d: %d chunks, %d KB (out of %dKB) : %d %% (%dMB mem free)\n",
					dimension->id, totalChunks, total/1024, totalChunks * 32, ( ( total / 1024 ) * 100 ) / ( totalChunks * 32),memStat.dwAvailPhys/(1024*1024) );
			}
	}
	updateTick++;
#endif

	// super.tick();

}

void MultiPlayerLevel::clearResetRegion(int x0, int y0, int z0, int x1, int y1, int z1)
{
	for (unsigned int i = 0; i < updatesToReset.size(); i++)
	{
		ResetInfo& r = updatesToReset[i];
		if (r.x >= x0 && r.y >= y0 && r.z >= z0 && r.x <= x1 && r.y <= y1 && r.z <= z1)
		{
			updatesToReset.erase(updatesToReset.begin()+i);
			i--;
		}
	}
}

ChunkSource *MultiPlayerLevel::createChunkSource()
{
	chunkCache = new MultiPlayerChunkCache(this);

	return chunkCache;
}

void MultiPlayerLevel::validateSpawn()
{
	// Fix for #62566 - TU7: Content: Gameplay: Compass needle stops pointing towards the original spawn point, once the player has entered the Nether.
	// 4J Stu - We should never be setting a specific spawn position for a multiplayer, this should only be set by receiving a packet from the server
	// (which happens when a player logs in)
	//setSpawnPos(new Pos(8, 64, 8));
}

void MultiPlayerLevel::tickTiles()
{
	chunksToPoll.clear();	// 4J - added or else we don't reset this set at all in a multiplayer level... think current java now resets in buildAndPrepareChunksToPoll rather than the calling functions

	PIXBeginNamedEvent(0,"Ticking tiles (multiplayer)");
	PIXBeginNamedEvent(0,"buildAndPrepareChunksToPoll");
	Level::tickTiles();
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Ticking client side tiles");
#ifdef __PSVITA__
	// AP - see CustomSet.h for and explanation
	for( int i = 0;i < chunksToPoll.end();i += 1 )
	{
		ChunkPos cp = chunksToPoll.get(i);
#else
	for (ChunkPos cp : chunksToPoll)
	{
#endif
		int xo = cp.x * 16;
		int zo = cp.z * 16;

		LevelChunk *lc = getChunk(cp.x, cp.z);

		tickClientSideTiles(xo, zo, lc);
	}
	PIXEndNamedEvent();
	PIXEndNamedEvent();
}

void MultiPlayerLevel::setChunkVisible(int x, int z, bool visible)
{
	if (visible)
	{
		chunkCache->create(x, z);
	}
	else
	{
		chunkCache->drop(x, z);
	}
	if (!visible)
	{
		setTilesDirty(x * 16, 0, z * 16, x * 16 + 15, Level::maxBuildHeight, z * 16 + 15);
	}

}

bool MultiPlayerLevel::addEntity(shared_ptr<Entity> e)
{
	bool ok = Level::addEntity(e);
	forced.insert(e);

	if (!ok)
	{
		reEntries.insert(e);
	}

	return ok;
}

void MultiPlayerLevel::removeEntity(shared_ptr<Entity> e)
{
	// 4J Stu - Add this remove from the reEntries collection to stop us continually removing and re-adding things,
	// in particular the MultiPlayerLocalPlayer when they die
    auto it = reEntries.find(e);
    if (it!=reEntries.end())
	{
		reEntries.erase(it);
	}

	Level::removeEntity(e);
	forced.erase(e);
}

void MultiPlayerLevel::entityAdded(shared_ptr<Entity> e)
{
	Level::entityAdded(e);
    auto it = reEntries.find(e);
    if (it!=reEntries.end())
	{
		reEntries.erase(it);
	}
}

void MultiPlayerLevel::entityRemoved(shared_ptr<Entity> e)
{
	Level::entityRemoved(e);
    auto it = forced.find(e);
    if (it!=forced.end())
	{
		reEntries.insert(e);
	}
}

void MultiPlayerLevel::putEntity(int id, shared_ptr<Entity> e)
{
	shared_ptr<Entity> old = getEntity(id);
	if (old != NULL)
	{
		removeEntity(old);
	}

	forced.insert(e);
	e->entityId = id;
	if (!addEntity(e))
	{
		reEntries.insert(e);
	}
	entitiesById[id] = e;
}

shared_ptr<Entity> MultiPlayerLevel::getEntity(int id)
{
    auto it = entitiesById.find(id);
    if( it == entitiesById.end() ) return nullptr;
	return it->second;
}

shared_ptr<Entity> MultiPlayerLevel::removeEntity(int id)
{
	shared_ptr<Entity> e;
    auto it = entitiesById.find(id);
    if( it != entitiesById.end() )
	{
		e = it->second;
		entitiesById.erase(it);
		forced.erase(e);
		removeEntity(e);
	}
	else
	{
	}
	return e;
}

// 4J Added to remove the entities from the forced list
// This gets called when a chunk is unloaded, but we only do half an unload to remove entities slightly differently
void MultiPlayerLevel::removeEntities(vector<shared_ptr<Entity> > *list)
{
	if ( list )
	{
		for (auto& e : *list )
		{
			auto reIt = reEntries.find(e);
			if (reIt!=reEntries.end())
			{
				reEntries.erase(reIt);
			}

			forced.erase(e);
		}
		Level::removeEntities(list);
	}
}

bool MultiPlayerLevel::setData(int x, int y, int z, int data, int updateFlags, bool forceUpdate/*=false*/)	// 4J added forceUpdate)
{
	// First check if this isn't going to do anything, because if it isn't then the next stage (of unsharing data) is really quite
	// expensive so far better to early out here
	int d = getData(x, y, z);

	if( ( d == data ) )
	{
		// If we early-out, its important that we still do a checkLight here (which would otherwise have happened as part of Level::setTileAndDataNoUpdate)
		// This is because since we are potentially sharing tile/data but not lighting data, it is possible that the server might tell a client
		// of a lighting update that doesn't need actioned on the client just because the chunk's data was being shared with the server when it was set. However,
		// the lighting data will potentially now be out of sync on the client.
		checkLight(x,y,z);
		return false;
	}
	// 4J - added - if this is the host, then stop sharing block data with the server at this point
	unshareChunkAt(x,z);

	if (Level::setData(x, y, z, data, updateFlags, forceUpdate))
	{
		//if(m_bEnableResetChanges) updatesToReset.push_back(ResetInfo(x, y, z, t, d));
		return true;
	}
	// Didn't actually need to stop sharing
	shareChunkAt(x,z);
	return false;
}

bool MultiPlayerLevel::setTileAndData(int x, int y, int z, int tile, int data, int updateFlags)
{
	// First check if this isn't going to do anything, because if it isn't then the next stage (of unsharing data) is really quite
	// expensive so far better to early out here
	int t = getTile(x, y, z);
	int d = getData(x, y, z);

	if( ( t == tile ) && ( d == data ) )
	{
		// If we early-out, its important that we still do a checkLight here (which would otherwise have happened as part of Level::setTileAndDataNoUpdate)
		// This is because since we are potentially sharing tile/data but not lighting data, it is possible that the server might tell a client
		// of a lighting update that doesn't need actioned on the client just because the chunk's data was being shared with the server when it was set. However,
		// the lighting data will potentially now be out of sync on the client.
		checkLight(x,y,z);
		return false;
	}
	// 4J - added - if this is the host, then stop sharing block data with the server at this point
	unshareChunkAt(x,z);

	if (Level::setTileAndData(x, y, z, tile, data, updateFlags))
	{
		//if(m_bEnableResetChanges) updatesToReset.push_back(ResetInfo(x, y, z, t, d));
		return true;
	}
	// Didn't actually need to stop sharing
	shareChunkAt(x,z);
	return false;
}


bool MultiPlayerLevel::doSetTileAndData(int x, int y, int z, int tile, int data)
{
	clearResetRegion(x, y, z, x, y, z);

	// 4J - Don't bother setting this to dirty if it isn't going to visually change - we get a lot of
	// water changing from static to dynamic for instance. Note that this is only called from a client connection,
	// and so the thing being notified of any update through tileUpdated is the renderer
	int prevTile = getTile(x, y, z);
	bool visuallyImportant = (!( ( ( prevTile == Tile::water_Id ) && ( tile == Tile::calmWater_Id ) )   ||
		( ( prevTile == Tile::calmWater_Id )  && ( tile == Tile::water_Id ) )	||
		( ( prevTile == Tile::lava_Id )		&& ( tile == Tile::calmLava_Id ) )	||
		( ( prevTile == Tile::calmLava_Id )		&& ( tile == Tile::calmLava_Id ) )	||
		( ( prevTile == Tile::calmLava_Id )	&& ( tile == Tile::lava_Id ) ) ) );
	// If we're the host, need to tell the renderer for updates even if they don't change things as the host
	// might have been sharing data and so set it already, but the renderer won't know to update
	if( (Level::setTileAndData(x, y, z, tile, data, Tile::UPDATE_ALL) || g_NetworkManager.IsHost() ) )
	{
		if( g_NetworkManager.IsHost() && visuallyImportant )
		{
			// 4J Stu - This got removed from the tileUpdated function in TU14. Adding it back here as we need it
			// to handle the cases where the chunk data is shared so the normal paths never call this
			sendTileUpdated(x,y,z);

			tileUpdated(x, y, z, tile);
		}
		return true;
	}
	return false;
}

void MultiPlayerLevel::disconnect(bool sendDisconnect /*= true*/)
{
	if( sendDisconnect )
	{
        for (auto& it : connections )
        {
			if ( it )
				it->sendAndDisconnect( shared_ptr<DisconnectPacket>( new DisconnectPacket(DisconnectPacket::eDisconnect_Quitting) ) );
		}
	}
	else
	{
		for (auto& it : connections )
		{
			if ( it )
				it->close();
		}
	}
}

Tickable *MultiPlayerLevel::makeSoundUpdater(shared_ptr<Minecart> minecart)
{
	return NULL; //new MinecartSoundUpdater(minecraft->soundEngine, minecart, minecraft->player);
}

void MultiPlayerLevel::tickWeather()
{
	if (dimension->hasCeiling) return;

	oRainLevel = rainLevel;
	if (levelData->isRaining())
	{
		rainLevel += 0.01;
	}
	else
	{
		rainLevel -= 0.01;
	}
	if (rainLevel < 0) rainLevel = 0;
	if (rainLevel > 1) rainLevel = 1;

	oThunderLevel = thunderLevel;
	if (levelData->isThundering())
	{
		thunderLevel += 0.01;
	}
	else
	{
		thunderLevel -= 0.01;
	}
	if (thunderLevel < 0) thunderLevel = 0;
	if (thunderLevel > 1) thunderLevel = 1;

}

void MultiPlayerLevel::animateTick(int xt, int yt, int zt)
{
	// Get 8x8x8 chunk (ie not like the renderer or game chunks... maybe we need another word here...) that the player is in
	// We then want to add a 3x3 region of chunks into a set that we'll be ticking over. Set is stored as unsigned ints which encode
	// this chunk position
	int cx = xt >> 3;
	int cy = yt >> 3;
	int cz = zt >> 3;

	for( int xx = -1; xx <= 1; xx++ )
		for( int yy = -1; yy <= 1; yy++ )
			for( int zz = -1; zz <= 1; zz++ )
			{
				if( ( cy + yy ) < 0 ) continue;
				if( ( cy + yy ) > 15 ) continue;
				// Note - LEVEL_MAX_WIDTH is in game (16) tile chunks, and so our level goes from -LEVEL_MAX_WIDTH to LEVEL_MAX_WIDTH of our half-sized chunks
				if( ( cx + xx ) >= LEVEL_MAX_WIDTH ) continue;
				if( ( cx + xx ) < -LEVEL_MAX_WIDTH ) continue;
				if( ( cz + zz ) >= LEVEL_MAX_WIDTH ) continue;
				if( ( cz + zz ) < -LEVEL_MAX_WIDTH ) continue;
				chunksToAnimate.insert( ( ( ( cx + xx ) & 0xff ) << 16 ) | ( ( ( cy + yy ) & 0xff ) << 8 ) | ( ( ( cz + zz ) & 0xff ) ) );
			}
}

// 4J - the game used to tick 1000 tiles in a random region +/- 16 units round the player. We've got a 3x3 region of 8x8x8 chunks round each
// player. So the original game was ticking 1000 things in a 32x32x32 region ie had about a 1 in 32 chance of updating any one tile per tick.
// We're not dealing with quite such a big region round each player (24x24x24) but potentially we've got 4 players. Ultimately, we could end
// up ticking anywhere between 432 and 1728 tiles depending on how many players we've got, which seems like a good tradeoff from the original.
void MultiPlayerLevel::animateTickDoWork()
{
	const int ticksPerChunk = 16;		// This ought to give us roughly the same 1000/32768 chance of a tile being animated as the original

	// Horrible hack to communicate with the level renderer, which is just attached as a listener to this level. This let's the particle
	// rendering know to use this level (rather than try to work it out from the current player), and to not bother distance clipping particles
	// which would again be based on the current player.
	Minecraft::GetInstance()->animateTickLevel = this;

	MemSect(31);
	Random *animateRandom = new Random();
	MemSect(0);

	for( int i = 0; i < ticksPerChunk; i++ )
	{
		for(int packed : chunksToAnimate)
		{
			int cx = ( packed << 8 ) >> 24;
			int cy = ( packed << 16 ) >> 24;
			int cz = ( packed << 24 ) >> 24;
			cx <<= 3;
			cy <<= 3;
			cz <<= 3;
			int x = cx + random->nextInt(8);
			int y = cy + random->nextInt(8);
			int z = cz + random->nextInt(8);
			int t = getTile(x, y, z);
			if (random->nextInt(8) > y && t == 0 && dimension->hasBedrockFog())			// 4J - test for bedrock fog brought forward from 1.2.3
			{
				addParticle(eParticleType_depthsuspend, x + random->nextFloat(), y + random->nextFloat(), z + random->nextFloat(), 0, 0, 0);
			}
			else if (t > 0)
			{
				Tile::tiles[t]->animateTick(this, x, y, z, animateRandom);
			}
		}
	}

	Minecraft::GetInstance()->animateTickLevel = NULL;
	delete animateRandom;

	chunksToAnimate.clear();

}

void MultiPlayerLevel::playSound(shared_ptr<Entity> entity, int iSound, float volume, float pitch)
{
	playLocalSound(entity->x, entity->y - entity->heightOffset, entity->z, iSound, volume, pitch);
}

void MultiPlayerLevel::playLocalSound(double x, double y, double z, int iSound, float volume, float pitch, bool distanceDelay/*= false */, float fClipSoundDist)
{
	//float dd = 16;
	if (volume > 1) fClipSoundDist *= volume;

	// 4J - find min distance to any players rather than just the current one
	float minDistSq = FLT_MAX;
	for( int i = 0; i < XUSER_MAX_COUNT; i++ )
	{
		if( minecraft->localplayers[i] )
		{
			float distSq = minecraft->localplayers[i]->distanceToSqr(x, y, z );
			if( distSq < minDistSq )
			{
				minDistSq = distSq;
			}
		}
	}

	if (minDistSq < fClipSoundDist * fClipSoundDist)
	{
		if (distanceDelay && minDistSq > 10 * 10)
		{
			// exhaggerate sound speed effect by making speed of sound ~=
			// 40 m/s instead of 300 m/s
			double delayInSeconds = sqrt(minDistSq) / 40.0;
			minecraft->soundEngine->schedule(iSound, (float) x, (float) y, (float) z, volume, pitch, (int) Math::round(delayInSeconds * SharedConstants::TICKS_PER_SECOND));
		}
		else
		{
			minecraft->soundEngine->play(iSound, (float) x, (float) y, (float) z, volume, pitch);
		}
	}
}

void MultiPlayerLevel::createFireworks(double x, double y, double z, double xd, double yd, double zd, CompoundTag *infoTag)
{
	minecraft->particleEngine->add(shared_ptr<FireworksParticles::FireworksStarter>(new FireworksParticles::FireworksStarter(this, x, y, z, xd, yd, zd, minecraft->particleEngine, infoTag)));
}

void MultiPlayerLevel::setScoreboard(Scoreboard *scoreboard)
{
	this->scoreboard = scoreboard;
}

void MultiPlayerLevel::setDayTime(int64_t newTime)
{
	// 4J: We send daylight cycle rule with host options so don't need this
	/*if (newTime < 0)
	{
		newTime = -newTime;
		getGameRules()->set(GameRules::RULE_DAYLIGHT, L"false");
	}
	else
	{
		getGameRules()->set(GameRules::RULE_DAYLIGHT, L"true");
	}*/

	Level::setDayTime(newTime);
}

void MultiPlayerLevel::removeAllPendingEntityRemovals()
{
	//entities.removeAll(entitiesToRemove);

	EnterCriticalSection(&m_entitiesCS);
    for (auto it = entities.begin(); it != entities.end();)
    {
		bool found = false;
		for(auto & it2 : entitiesToRemove)
		{
			if( (*it) == it2 )
			{
				found = true;
				break;
			}
		}
		if( found )
		{
			it = entities.erase(it);
		}
		else
		{
			it++;
		}
	}
	LeaveCriticalSection(&m_entitiesCS);

	for (auto& e : entitiesToRemove)
	{
		if ( e )
		{
			int xc = e->xChunk;
			int zc = e->zChunk;
			if (e->inChunk && hasChunk(xc, zc))
			{
				getChunk(xc, zc)->removeEntity(e);
			}
		}
	}

	// 4J Stu - Is there a reason do this in a separate loop? Thats what the Java does...
	for (auto& it : entitiesToRemove)
	{
		if ( it )
			entityRemoved(it);
	}
	entitiesToRemove.clear();

	//for (int i = 0; i < entities.size(); i++)
	EnterCriticalSection(&m_entitiesCS);
	auto it = entities.begin();
	while(  it != entities.end() )
	{
		shared_ptr<Entity> e = *it;//entities.at(i);

		if (e->riding != NULL)
		{
			if (e->riding->removed || e->riding->rider.lock() != e)
			{
				e->riding->rider = weak_ptr<Entity>();
				e->riding = nullptr;
			}
			else
			{
				++it;
				continue;
			}
		}

		if (e->removed)
		{
			int xc = e->xChunk;
			int zc = e->zChunk;
			if (e->inChunk && hasChunk(xc, zc))
			{
				getChunk(xc, zc)->removeEntity(e);
			}
			//entities.remove(i--);

			it = entities.erase( it );
			entityRemoved(e);
		}
		else
		{
			it++;
		}
	}
	LeaveCriticalSection(&m_entitiesCS);
}

void MultiPlayerLevel::removeClientConnection(ClientConnection *c, bool sendDisconnect)
{
	if( sendDisconnect )
	{
		c->sendAndDisconnect( shared_ptr<DisconnectPacket>( new DisconnectPacket(DisconnectPacket::eDisconnect_Quitting) ) );
	}

    auto it = find(connections.begin(), connections.end(), c);
    if( it != connections.end() )
	{
		connections.erase( it );
	}
}

void MultiPlayerLevel::tickAllConnections()
{
	PIXBeginNamedEvent(0,"Connection ticking");
    for (auto& it : connections )
    {
		if ( it )
			it->tick();
	}
	PIXEndNamedEvent();
}

void MultiPlayerLevel::dataReceivedForChunk(int x, int z)
{
	chunkCache->dataReceived(x, z);
}

// 4J added - removes all tile entities in the given region from both level & levelchunks
void MultiPlayerLevel::removeUnusedTileEntitiesInRegion(int x0, int y0, int z0, int x1, int y1, int z1)
{
	EnterCriticalSection(&m_tileEntityListCS);

	for (unsigned int i = 0; i < tileEntityList.size();)
	{
		bool removed = false;
		shared_ptr<TileEntity> te = tileEntityList[i];
		if (te->x >= x0 && te->y >= y0 && te->z >= z0 && te->x < x1 && te->y < y1 && te->z < z1)
		{
			LevelChunk *lc = getChunk(te->x >> 4, te->z >> 4);
			if (lc != NULL)
			{
				// Only remove tile entities where this is no longer a tile entity
				int tileId = lc->getTile(te->x & 15, te->y, te->z & 15 );
				if( Tile::tiles[tileId] == NULL || !Tile::tiles[tileId]->isEntityTile())
				{
					tileEntityList[i] = tileEntityList.back();
					tileEntityList.pop_back();

					// 4J Stu - Chests can create new tile entities when being removed, so disable this
					m_bDisableAddNewTileEntities = true;
					lc->removeTileEntity(te->x & 15, te->y, te->z & 15);
					m_bDisableAddNewTileEntities = false;
					removed = true;
				}
			}
		}
		if( !removed ) i++;
	}

	LeaveCriticalSection(&m_tileEntityListCS);
}

