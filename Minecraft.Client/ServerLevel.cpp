#include "stdafx.h"
#include "ServerLevel.h"

#include <memory>
#include "MinecraftServer.h"
#include "ServerChunkCache.h"
#include "PlayerList.h"
#include "ServerPlayer.h"
#include "PlayerConnection.h"
#include "EntityTracker.h"
#include "ServerScoreboard.h"
#include "..\Minecraft.World\ScoreboardSaveData.h"
#include "..\Minecraft.World\net.minecraft.world.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\net.minecraft.world.entity.ai.village.h"
#include "..\Minecraft.World\net.minecraft.world.entity.global.h"
#include "..\Minecraft.World\net.minecraft.world.entity.npc.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.biome.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "..\Minecraft.World\net.minecraft.world.level.levelgen.feature.h"
#include "..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\Minecraft.World\net.minecraft.world.scores.h"
#include "..\Minecraft.World\ItemEntity.h"
#include "..\Minecraft.World\Arrow.h"
#include "..\Minecraft.World\PrimedTnt.h"
#include "..\Minecraft.World\FallingTile.h"
#include "..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\Minecraft.World\Mth.h"
#include "..\Minecraft.World\StructurePiece.h"
#include "..\Minecraft.Client\ServerLevelListener.h"
#include "..\Minecraft.World\WeighedTreasure.h"
#include "TexturePackRepository.h"
#include "DLCTexturePack.h"
#include "..\Minecraft.World\ProgressListener.h"
#include "PS3\PS3Extras\ShutdownManager.h"
#include "PlayerChunkMap.h"

WeighedTreasureArray ServerLevel::RANDOM_BONUS_ITEMS;

C4JThread* ServerLevel::m_updateThread = nullptr;
C4JThread::EventArray* ServerLevel::m_updateTrigger;
CRITICAL_SECTION ServerLevel::m_updateCS[3];

Level *ServerLevel::m_level[3];
int	   ServerLevel::m_updateChunkX[3][LEVEL_CHUNKS_TO_UPDATE_MAX];
int	   ServerLevel::m_updateChunkZ[3][LEVEL_CHUNKS_TO_UPDATE_MAX];
int	   ServerLevel::m_updateChunkCount[3];
int	   ServerLevel::m_updateTileX[3][MAX_UPDATES];
int	   ServerLevel::m_updateTileY[3][MAX_UPDATES];
int	   ServerLevel::m_updateTileZ[3][MAX_UPDATES];
int	   ServerLevel::m_updateTileCount[3];
int	   ServerLevel::m_randValue[3];

void ServerLevel::staticCtor()
{
	m_updateTrigger  = new C4JThread::EventArray(3);
	InitializeCriticalSection(&m_updateCS[0]);
	InitializeCriticalSection(&m_updateCS[1]);
	InitializeCriticalSection(&m_updateCS[2]);

	m_updateThread = new C4JThread(runUpdate, nullptr, "Tile update");
	m_updateThread->SetProcessor(CPU_CORE_TILE_UPDATE);
#ifdef __ORBIS__
	m_updateThread->SetPriority(THREAD_PRIORITY_BELOW_NORMAL);	// On Orbis, this core is also used for Matching 2, and that priority of that seems to be always at default no matter what we set it to. Prioritise this below Matching 2.
#endif
	m_updateThread->Run();

	RANDOM_BONUS_ITEMS =  WeighedTreasureArray(20);

	RANDOM_BONUS_ITEMS[0] = new WeighedTreasure(Item::stick_Id, 0, 1, 3, 10);
	RANDOM_BONUS_ITEMS[1] = new WeighedTreasure(Tile::wood_Id, 0, 1, 3, 10);
	RANDOM_BONUS_ITEMS[2] = new WeighedTreasure(Tile::treeTrunk_Id, 0, 1, 3, 10);
	RANDOM_BONUS_ITEMS[3] = new WeighedTreasure(Item::hatchet_stone_Id, 0, 1, 1, 3);
	RANDOM_BONUS_ITEMS[4] = new WeighedTreasure(Item::hatchet_wood_Id, 0, 1, 1, 5);
	RANDOM_BONUS_ITEMS[5] = new WeighedTreasure(Item::pickAxe_stone_Id, 0, 1, 1, 3);
	RANDOM_BONUS_ITEMS[6] = new WeighedTreasure(Item::pickAxe_wood_Id, 0, 1, 1, 5);
	RANDOM_BONUS_ITEMS[7] = new WeighedTreasure(Item::apple_Id, 0, 2, 3, 5);
	RANDOM_BONUS_ITEMS[8] = new WeighedTreasure(Item::bread_Id, 0, 2, 3, 3);
	// 4J-PB - new items
	RANDOM_BONUS_ITEMS[9] = new WeighedTreasure(Tile::sapling_Id, 0, 4, 4, 2);
	RANDOM_BONUS_ITEMS[10] = new WeighedTreasure(Tile::sapling_Id, 1, 4, 4, 2);
	RANDOM_BONUS_ITEMS[11] = new WeighedTreasure(Tile::sapling_Id, 2, 4, 4, 2);
	RANDOM_BONUS_ITEMS[12] = new WeighedTreasure(Tile::sapling_Id, 3, 4, 4, 4);
	RANDOM_BONUS_ITEMS[13] = new WeighedTreasure(Item::seeds_melon_Id, 0, 1, 2, 3);
	RANDOM_BONUS_ITEMS[14] = new WeighedTreasure(Item::seeds_pumpkin_Id, 0, 1, 2, 3);
	RANDOM_BONUS_ITEMS[15] = new WeighedTreasure(Tile::cactus_Id, 0, 1, 2, 3);
	RANDOM_BONUS_ITEMS[16] = new WeighedTreasure(Item::dye_powder_Id, DyePowderItem::BROWN, 1, 2, 2);
	RANDOM_BONUS_ITEMS[17] = new WeighedTreasure(Item::potato_Id, 0, 1, 2, 3);
	RANDOM_BONUS_ITEMS[18] = new WeighedTreasure(Item::carrots_Id, 0, 1, 2, 3);
	RANDOM_BONUS_ITEMS[19] = new WeighedTreasure(Tile::mushroom_brown_Id, 0, 1, 2, 2);

};

ServerLevel::ServerLevel(MinecraftServer *server, shared_ptr<LevelStorage>levelStorage, const wstring& levelName, int dimension, LevelSettings *levelSettings) : Level(levelStorage, levelName, levelSettings, Dimension::getNew(dimension), false)
{
	InitializeCriticalSection(&m_limiterCS);
	InitializeCriticalSection(&m_tickNextTickCS);
	InitializeCriticalSection(&m_csQueueSendTileUpdates);
	m_fallingTileCount = 0;
	m_primedTntCount = 0;

	// 4J - this this used to be called in parent ctor via a virtual fn
	chunkSource = createChunkSource();
	// 4J - optimisation - keep direct reference of underlying cache here
	chunkSourceCache = chunkSource->getCache();
	chunkSourceXZSize = chunkSource->m_XZSize;

	// 4J - The listener used to be added in MinecraftServer::loadLevel but we need it to be set up before we do the next couple of things, or else chunks get loaded before we have the entity tracker set up to listen to them
	this->server = server;
	server->setLevel(dimension, this);		// The listener needs the server to have the level set up... this will be set up anyway on return of this ctor but setting up early here
	addListener(new ServerLevelListener(server, this));

	tracker = new EntityTracker(this);
	chunkMap = new PlayerChunkMap(this, dimension, server->getPlayers()->getViewDistance());

	mobSpawner = new MobSpawner();
	portalForcer = new PortalForcer(this);
	scoreboard = new ServerScoreboard(server);

	//shared_ptr<ScoreboardSaveData> scoreboardSaveData = dynamic_pointer_cast<ScoreboardSaveData>( savedDataStorage->get(typeid(ScoreboardSaveData), ScoreboardSaveData::FILE_ID) );
	//if (scoreboardSaveData == nullptr)
	//{
	//	scoreboardSaveData = shared_ptr<ScoreboardSaveData>( new ScoreboardSaveData() );
	//	savedDataStorage->set(ScoreboardSaveData::FILE_ID, scoreboardSaveData);
	//}
	//scoreboardSaveData->setScoreboard(scoreboard);
	//((ServerScoreboard *) scoreboard)->setSaveData(scoreboardSaveData);

	// This also used to be called in parent ctor, but can't be called until chunkSource is created. Call now if required.
	if (!levelData->isInitialized())
	{
		initializeLevel(levelSettings);
		levelData->setInitialized(true);
	}
	else if ( (dimension==0) && levelData->getSpawnBonusChest() ) // 4J-JEV, still would like bonus chests to respawn.
	{
		// 4J - added isFindingSpawn as we want any chunks we are looking in here for suitable locations for the bonus chest to actually create those chunks rather than just get emptychunks if they aren't loaded
		isFindingSpawn = true;
		generateBonusItemsNearSpawn();
		isFindingSpawn = false;
	}

	// 4J - added initialisers
	// 4J Stu - Allowing spawn edit for our game, and consider a better solution for the possible griefing
	canEditSpawn = true; //false;
	noSave = false;
	allPlayersSleeping = false;
	m_bAtLeastOnePlayerSleeping = false;
	emptyTime = 0;
	activeTileEventsList = 0;

#ifdef _LARGE_WORLDS
	saveInterval = 3;
#else
	saveInterval = 20 * 2;
#endif
}

ServerLevel::~ServerLevel()
{
	delete portalForcer;
	delete mobSpawner;

	EnterCriticalSection(&m_csQueueSendTileUpdates);
	for(Pos* p : m_queuedSendTileUpdates)
	{
		delete p;
	}
	m_queuedSendTileUpdates.clear();

	delete this->tracker; // MGH - added, we were losing about 500K going in and out the menus
	delete this->chunkMap;

	LeaveCriticalSection(&m_csQueueSendTileUpdates);
	DeleteCriticalSection(&m_csQueueSendTileUpdates);
	DeleteCriticalSection(&m_limiterCS);
	DeleteCriticalSection(&m_tickNextTickCS);

	// Make sure that the update thread isn't actually doing any updating
	EnterCriticalSection(&m_updateCS[0]);
	LeaveCriticalSection(&m_updateCS[0]);
	EnterCriticalSection(&m_updateCS[1]);
	LeaveCriticalSection(&m_updateCS[1]);
	EnterCriticalSection(&m_updateCS[2]);
	LeaveCriticalSection(&m_updateCS[2]);
	m_updateTrigger->ClearAll();
}


void ServerLevel::tick()
{
	Level::tick();
	if (getLevelData()->isHardcore() && difficulty < 3)
	{
		difficulty = 3;
	}

	dimension->biomeSource->update();

	if (allPlayersAreSleeping())
	{
		if (getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT))
		{
			// skip time until new day
			int64_t newTime = levelData->getDayTime() + TICKS_PER_DAY;

			// 4J : WESTY : Changed so that time update goes through stats tracking update code.
			//levelData->setTime(newTime - (newTime % TICKS_PER_DAY));
			setDayTime(newTime - (newTime % TICKS_PER_DAY));
		}
		awakenAllPlayers();
	}

	PIXBeginNamedEvent(0,"Mob spawner tick");
	// for Minecraft 1.8, spawn friendlies really rarely	- 4J - altered from once every 400 ticks to 40 ticks as we depend on this a more than the original since we don't have chunk post-process spawning
	if (getGameRules()->getBoolean(GameRules::RULE_DOMOBSPAWNING))
	{
		// Note - these flags are used logically in an inverted way. Mob spawning is not performed if:
		// (1) finalSpawnEnemies isn't set, and mob category isn't friendly
		// (2) finalSpawnFriendlies isn't set, and mob category is friendly
		// (3) finalSpawnPersistent isn't set, and mob category is persistent
		bool finalSpawnEnemies = spawnEnemies && ((levelData->getGameTime() % 2) == 0);					// Spawn enemies every other tick
		bool finalSpawnFriendlies = spawnFriendlies && ((levelData->getGameTime() % 40) == 0);			// Spawn friendlies once per 40 ticks
		bool finalSpawnPersistent = finalSpawnFriendlies && ((levelData->getGameTime() % 80) == 0);		// All persistents are also friendly - do them once every other friendly spawning, ie once per 80 ticks
		mobSpawner->tick(this, finalSpawnEnemies, finalSpawnFriendlies, finalSpawnPersistent);
	}
	PIXEndNamedEvent();
	PIXBeginNamedEvent(0,"Chunk source tick");
	chunkSource->tick();
	PIXEndNamedEvent();
	int newDark = getOldSkyDarken(1);
	if (newDark != skyDarken)
	{
		skyDarken = newDark;
		if (!SharedConstants::TEXTURE_LIGHTING)	// 4J - change brought forward from 1.8.2
		{
			for (auto& listener : listeners)
			{
				listener->skyColorChanged();
			}
		}
	}

	//4J - temporarily disabling saves as they are causing gameplay to generally stutter quite a lot

	int64_t time = levelData->getGameTime() + 1;
	// 4J Stu - Putting this back in, but I have reduced the number of chunks that save when not forced
#ifdef _LARGE_WORLDS
	if (time % (saveInterval) == (dimension->id + 1))
#else
	if (time % (saveInterval) == (dimension->id * dimension->id * (saveInterval/2)))
#endif
	{
		PIXBeginNamedEvent(0,"Incremental save");
		save(false, nullptr);
		PIXEndNamedEvent();
	}

	// 4J : WESTY : Changed so that time update goes through stats tracking update code.
	//levelData->setTime(time);
	setGameTime(levelData->getGameTime() + 1);
	if (getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT))
	{
		// 4J: Debug setting added to keep it at day time
#ifndef _FINAL_BUILD
		bool freezeTime = app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_FreezeTime);
		if (!freezeTime)
#endif
		{
			setDayTime(levelData->getDayTime() + 1);
		}
	}

	PIXBeginNamedEvent(0,"Tick pending ticks");
	// if (tickCount % 5 == 0) {
	tickPendingTicks(false);
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Tick tiles");
	MemSect(18);
	tickTiles();
	MemSect(0);
	PIXEndNamedEvent();

	chunkMap->tick();

	PIXBeginNamedEvent(0,"Tick villages");
	//MemSect(18);
	villages->tick();
	villageSiege->tick();
	//MemSect(0);
	PIXEndNamedEvent();

	PIXBeginNamedEvent(0,"Tick portal forcer");
	portalForcer->tick(getGameTime());
	PIXEndNamedEvent();

	// repeat after tile ticks
	PIXBeginNamedEvent(0,"runTileEvents");
	runTileEvents();
	PIXEndNamedEvent();

	// 4J Added
	runQueuedSendTileUpdates();
}

Biome::MobSpawnerData *ServerLevel::getRandomMobSpawnAt(MobCategory *mobCategory, int x, int y, int z)
{
	vector<Biome::MobSpawnerData *> *mobList = getChunkSource()->getMobsAt(mobCategory, x, y, z);
	if (mobList == nullptr || mobList->empty()) return nullptr;

	return static_cast<Biome::MobSpawnerData *>(WeighedRandom::getRandomItem(random, (vector<WeighedRandomItem *> *)mobList));
}

void ServerLevel::updateSleepingPlayerList()
{
	allPlayersSleeping = !players.empty();
	m_bAtLeastOnePlayerSleeping = false;

	for (auto& player : players)
	{
		if (!player->isSleeping())
		{
			allPlayersSleeping = false;
			//break;
		}
		else
		{
			m_bAtLeastOnePlayerSleeping = true;
		}
		if(m_bAtLeastOnePlayerSleeping && !allPlayersSleeping) break;
	}
}

void ServerLevel::awakenAllPlayers()
{
	allPlayersSleeping = false;
	m_bAtLeastOnePlayerSleeping = false;

	for (auto& player : players)
	{
		if (player->isSleeping())
		{
			player->stopSleepInBed(false, false, true);
		}
	}

	stopWeather();
}

void ServerLevel::stopWeather()
{
	levelData->setRainTime(0);
	levelData->setRaining(false);
	levelData->setThunderTime(0);
	levelData->setThundering(false);
}

bool ServerLevel::allPlayersAreSleeping()
{
	if (allPlayersSleeping && !isClientSide)
	{
		// all players are sleeping, but have they slept long enough?
		for (auto& player : players)
		{
			//                System.out.println(player->entityId + ": " + player->getSleepTimer());
			if (!player->isSleepingLongEnough())
			{
				return false;
			}
		}
		// yep
		return true;
	}
	return false;
}

void ServerLevel::validateSpawn()
{
	if (levelData->getYSpawn() <= 0)
	{
		levelData->setYSpawn(genDepth / 2);
	}
	int xSpawn = levelData->getXSpawn();
	int zSpawn = levelData->getZSpawn();
	int tries = 0;
	while (getTopTile(xSpawn, zSpawn) == 0)
	{
		xSpawn += random->nextInt(8) - random->nextInt(8);
		zSpawn += random->nextInt(8) - random->nextInt(8);
		if (++tries == 10000) break;
	}
	levelData->setXSpawn(xSpawn);
	levelData->setZSpawn(zSpawn);
}

// 4J - Changes made here to move a section of code (which randomly determines which tiles in the current chunks to tick, and is very cache unfriendly by nature)
// This code now has a thread of its own so it can wait all it wants on the cache without holding the main game thread up. This slightly changes how things are
// processed, as we now tick the tiles that were determined in the previous tick. Have also limited the amount of tiles to be ticked to 256 (it never seemed to creep
// up much beyond this in normal play anyway, and we need some finite limit).
void ServerLevel::tickTiles()
{
	// Index into the arrays used by the update thread
	int iLev = 0;
	if( dimension->id == -1 )
	{
		iLev = 1;
	}
	else if( dimension->id == 1 )
	{
		iLev = 2;
	}
	chunksToPoll.clear();

	unsigned int tickCount = 0;

	EnterCriticalSection(&m_updateCS[iLev]);
	// This section processes the tiles that need to be ticked, which we worked out in the previous tick (or haven't yet, if this is the first frame)
	/*int grassTicks = 0;
	int lavaTicks = 0;
	int otherTicks = 0;*/
	for( int i = 0; i < m_updateTileCount[iLev]; i++ )
	{
		int x = m_updateTileX[iLev][i];
		int y = m_updateTileY[iLev][i];
		int z = m_updateTileZ[iLev][i];
		if( hasChunkAt(x,y,z) )
		{
			int id = getTile(x,y,z);
			if (Tile::tiles[id] != nullptr && Tile::tiles[id]->isTicking())
			{
				/*if(id == 2) ++grassTicks;
				else if(id == 11) ++lavaTicks;
				else ++otherTicks;*/
				Tile::tiles[id]->tick(this, x, y, z, random);
			}
		}
	}
	//printf("Total ticks - Grass: %d, Lava: %d, Other: %d, Total: %d\n", grassTicks, lavaTicks, otherTicks, grassTicks + lavaTicks + otherTicks);
	m_updateTileCount[iLev] = 0;
	m_updateChunkCount[iLev] = 0;
	LeaveCriticalSection(&m_updateCS[iLev]);

	Level::tickTiles();

	// AP moved this outside of the loop
	int prob = 100000;
	if(app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_RegularLightning)) prob = 100;

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

		// 4J added - don't let this actually load/create any chunks, we'll let the normal updateDirtyChunks etc. processes do that, so it can happen on another thread
		if( !this->hasChunk(cp.x, cp.z) ) continue;

		// 4J Stu - When adding a 5th player to the game, the number of chunksToPoll is greater than the size of
		// the m_updateChunkX & m_updateChunkZ arrays (19*19*4 at time of writing). It doesn't seem like there should
		// ever be that many chunks needing polled, so this needs looked at in more detail. For now I have enlarged
		// the size of the array to 19*19*8 but this seems way to big for our needs.

		// The cause of this is largely because the chunksToPoll vector does not enforce unique elements
		// The java version used a HashSet which would, although if our world size gets a lot larger
		// then we may have no overlaps of players surrounding chunks
		//assert(false);

		// If you hit this assert, then a memory overwrite will occur when you continue
		assert(m_updateChunkCount[iLev] < LEVEL_CHUNKS_TO_UPDATE_MAX);

		m_updateChunkX[iLev][m_updateChunkCount[iLev]] = cp.x;
		m_updateChunkZ[iLev][m_updateChunkCount[iLev]++] = cp.z;

		LevelChunk *lc = getChunk(cp.x, cp.z);
		tickClientSideTiles(xo, zo, lc);

		if (random->nextInt(prob) == 0 && isRaining() && isThundering())
		{
			randValue = randValue * 3 + addend;
			int val = (randValue >> 2);
			int x = xo + (val & 15);
			int z = zo + ((val >> 8) & 15);
			int y = getTopRainBlock(x, z);

			if (isRainingAt(x, y, z))
			{
				addGlobalEntity(std::make_shared<LightningBolt>(this, x, y, z));
			}
		}

		// 4J - changes here brought forrward from 1.2.3
		if (random->nextInt(16) == 0)
		{
			randValue = randValue * 3 + addend;
			int val = (randValue >> 2);
			int x = (val & 15);
			int z = ((val >> 8) & 15);
			int yy = getTopRainBlock(x + xo, z + zo);
			if (shouldFreeze(x + xo, yy - 1, z + zo))
			{
				setTileAndUpdate(x + xo, yy - 1, z + zo, Tile::ice_Id);
			}
			if (isRaining() && shouldSnow(x + xo, yy, z + zo))
			{
				setTileAndUpdate(x + xo, yy, z + zo, Tile::topSnow_Id);
			}
			if (isRaining())
			{
				Biome *b = getBiome(x + xo, z + zo);
				if (b->hasRain())
				{
					int tile = getTile(x + xo, yy - 1, z + zo);
					if (tile != 0)
					{
						Tile::tiles[tile]->handleRain(this, x + xo, yy - 1, z + zo);
					}
				}
			}
		}

		// 4J - lighting change brought forward from 1.8.2
		checkLight(xo + random->nextInt(16), random->nextInt(128), zo + random->nextInt(16));
	}

	m_level[iLev] = this;
	m_randValue[iLev] = randValue;
	// We've set up everything that the udpate thread needs, so kick it off
	m_updateTrigger->Set(iLev);
}

bool ServerLevel::isTileToBeTickedAt(int x, int y, int z, int tileId)
{
	TickNextTickData td = TickNextTickData(x, y, z, tileId);
	return std::find(toBeTicked.begin(), toBeTicked.end(), td) != toBeTicked.end();
}

void ServerLevel::addToTickNextTick(int x, int y, int z, int tileId, int tickDelay)
{
	addToTickNextTick(x, y, z, tileId, tickDelay, 0);
}

void ServerLevel::addToTickNextTick(int x, int y, int z, int tileId, int tickDelay, int priorityTilt)
{
	MemSect(27);
	TickNextTickData td = TickNextTickData(x, y, z, tileId);
	int r = 0;
	if (getInstaTick() && tileId > 0)
	{
		if(Tile::tiles[tileId]->canInstantlyTick())
		{
			r = 8;
			if (hasChunksAt(td.x - r, td.y - r, td.z - r, td.x + r, td.y + r, td.z + r))
			{
				int id = getTile(td.x, td.y, td.z);
				if (id == td.tileId && id > 0)
				{
					Tile::tiles[id]->tick(this, td.x, td.y, td.z, random);
				}
			}
			MemSect(0);
			return;
		}
		else
		{
			tickDelay = 1;
		}
	}

	if (hasChunksAt(x - r, y - r, z - r, x + r, y + r, z + r))
	{
		if (tileId > 0)
		{
			td.delay(tickDelay + levelData->getGameTime());
			td.setPriorityTilt(priorityTilt);
		}
		EnterCriticalSection(&m_tickNextTickCS);
		if ( tickNextTickSet.find(td) == tickNextTickSet.end() )
		{
			tickNextTickSet.insert(td);
			tickNextTickList.insert(td);
		}
		LeaveCriticalSection(&m_tickNextTickCS);
	}
	MemSect(0);
}

void ServerLevel::forceAddTileTick(int x, int y, int z, int tileId, int tickDelay, int prioTilt)
{
	TickNextTickData td = TickNextTickData(x, y, z, tileId);
	td.setPriorityTilt(prioTilt);

	if (tileId > 0)
	{
		td.delay(tickDelay + levelData->getGameTime());
	}
	EnterCriticalSection(&m_tickNextTickCS);
	if ( tickNextTickSet.find(td) == tickNextTickSet.end() )
	{
		tickNextTickSet.insert(td);
		tickNextTickList.insert(td);
	}
	LeaveCriticalSection(&m_tickNextTickCS);
}

void ServerLevel::tickEntities()
{
	if (players.empty())
	{
		if (emptyTime++ >= EMPTY_TIME_NO_TICK)
		{
			return;
		}
	}
	else
	{
		resetEmptyTime();
	}

	Level::tickEntities();
}

void ServerLevel::resetEmptyTime()
{
	emptyTime = 0;
}

bool ServerLevel::tickPendingTicks(bool force)
{
	EnterCriticalSection(&m_tickNextTickCS);
	int count = static_cast<int>(tickNextTickList.size());
	int count2 = static_cast<int>(tickNextTickSet.size());
	if (count != tickNextTickSet.size())
	{
		// TODO 4J Stu - Add new exception types
		//throw new IllegalStateException("TickNextTick list out of synch");
	}
	if (count > MAX_TICK_TILES_PER_TICK) count = MAX_TICK_TILES_PER_TICK;

    auto itTickList = tickNextTickList.begin();
    for (int i = 0; i < count; i++)
	{
		TickNextTickData td = *(itTickList);
		if (!force && td.m_delay > levelData->getGameTime())
		{
			break;
		}

		itTickList = tickNextTickList.erase(itTickList);
		tickNextTickSet.erase(td);
		toBeTicked.push_back(td);
	}

    for (auto it = toBeTicked.begin(); it != toBeTicked.end();)
    {
		TickNextTickData td = *it;
		it = toBeTicked.erase(it);

		int r = 0;
		if (hasChunksAt(td.x - r, td.y - r, td.z - r, td.x + r, td.y + r, td.z + r))
		{
			int id = getTile(td.x, td.y, td.z);
			if (id > 0 && Tile::isMatching(id, td.tileId))
			{
				Tile::tiles[id]->tick(this, td.x, td.y, td.z, random);
			}
		}
		else
		{
			addToTickNextTick(td.x, td.y, td.z, td.tileId, 0);
		}
	}

	toBeTicked.clear();

	int count3 = static_cast<int>(tickNextTickList.size());
	int count4 = static_cast<int>(tickNextTickSet.size());

	bool retval = tickNextTickList.size() != 0;
	LeaveCriticalSection(&m_tickNextTickCS);

	return retval;
}

vector<TickNextTickData> *ServerLevel::fetchTicksInChunk(LevelChunk *chunk, bool remove)
{
	EnterCriticalSection(&m_tickNextTickCS);
	vector<TickNextTickData> *results = new vector<TickNextTickData>;

	ChunkPos *pos = chunk->getPos();
	int xMin = (pos->x << 4) - 2;
	int xMax = (xMin + 16) + 2;
	int zMin = (pos->z << 4) - 2;
	int zMax = (zMin + 16) + 2;
	delete pos;

	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
		{
            for (auto it = tickNextTickList.begin(); it != tickNextTickList.end();)
            {
				TickNextTickData td = *it;

				if (td.x >= xMin && td.x < xMax && td.z >= zMin && td.z < zMax)
				{
					if (remove)
					{
						tickNextTickSet.erase(td);
						it = tickNextTickList.erase(it);
					}
					else
					{
						it++;
					}

					results->push_back(td);
				}
				else
				{
					it++;
				}
			}
		}
		else
		{
			if (!toBeTicked.empty())
			{
				app.DebugPrintf("To be ticked size: %d\n",toBeTicked.size());
			}
            for (auto it = toBeTicked.begin(); it != toBeTicked.end();)
            {
				TickNextTickData td = *it;

				if (td.x >= xMin && td.x < xMax && td.z >= zMin && td.z < zMax)
				{
					if (remove)
					{
						tickNextTickList.erase(td);
						it = toBeTicked.erase(it);
					}
					else
					{
						it++;
					}

					results->push_back(td);
				}
				else
				{
					it++;
				}
			}
		}
	}

	LeaveCriticalSection(&m_tickNextTickCS);
	return results;
}

void ServerLevel::tick(shared_ptr<Entity> e, bool actual)
{
	if ( !server->isAnimals() && (e->instanceof(eTYPE_ANIMAL) || e->instanceof(eTYPE_WATERANIMAL)) )
	{
		e->remove();
	}
	if (!server->isNpcsEnabled() && (dynamic_pointer_cast<Npc>(e) != nullptr))
	{
		e->remove();
	}
	Level::tick(e, actual);
}

void ServerLevel::forceTick(shared_ptr<Entity> e, bool actual)
{
	Level::tick(e, actual);
}

ChunkSource *ServerLevel::createChunkSource()
{
	ChunkStorage *storage = levelStorage->createChunkStorage(dimension);
	cache = new ServerChunkCache(this, storage, dimension->createRandomLevelSource());
	return cache;
}

vector<shared_ptr<TileEntity> > *ServerLevel::getTileEntitiesInRegion(int x0, int y0, int z0, int x1, int y1, int z1)
{
	vector<shared_ptr<TileEntity> > *result = new vector<shared_ptr<TileEntity> >;
	for (unsigned int i = 0; i < tileEntityList.size(); i++)
	{
		shared_ptr<TileEntity> te = tileEntityList[i];
		if (te->x >= x0 && te->y >= y0 && te->z >= z0 && te->x < x1 && te->y < y1 && te->z < z1)
		{
			result->push_back(te);
		}
	}
	return result;
}

bool ServerLevel::mayInteract(shared_ptr<Player> player, int xt, int yt, int zt, int content)
{
	// 4J-PB - This will look like a bug to players, and we really should have a message to explain why we're not allowing lava to be placed at or near a spawn point
	// We'll need to do this in a future update

	// 4J-PB - Let's allow water near the spawn point, but not lava
	if(content!=Tile::lava_Id)
	{
		// allow this to be used
		return true;
	}
	else if(dimension->id == 0) // 4J Stu - Only limit this in the overworld
	{
		return !server->isUnderSpawnProtection(this, xt, yt, zt, player);
	}
	return true;
}

void ServerLevel::initializeLevel(LevelSettings *settings)
{
	setInitialSpawn(settings);

	Level::initializeLevel(settings);
}

/**
* Sets the initial spawn, created this method so we could do a special
* location for the demo version.
*/
void ServerLevel::setInitialSpawn(LevelSettings *levelSettings)
{
	if (!dimension->mayRespawn())
	{
		levelData->setSpawn(0, dimension->getSpawnYPosition(), 0);
		return;
	}

	isFindingSpawn = true;

	BiomeSource *biomeSource = dimension->biomeSource;
	vector<Biome *> playerSpawnBiomes = biomeSource->getPlayerSpawnBiomes();
	Random random(getSeed());

	TilePos *findBiome = biomeSource->findBiome(0, 0, 16 * 16, playerSpawnBiomes, &random);

	int xSpawn = 0; // (Level.MAX_LEVEL_SIZE - 100) * 0;
	int ySpawn = dimension->getSpawnYPosition();
	int zSpawn = 0; // (Level.MAX_LEVEL_SIZE - 100) * 0;
	int minXZ = - (dimension->getXZSize() * 16 ) / 2;
	int maxXZ = (dimension->getXZSize() * 16 ) / 2 - 1;

	if (findBiome != nullptr)
	{
		xSpawn = findBiome->x;
		zSpawn = findBiome->z;
		delete findBiome;
	}
	else
	{
		app.DebugPrintf("Level::setInitialSpawn - Unable to find spawn biome\n");
	}

	int tries = 0;

	while (!dimension->isValidSpawn(xSpawn, zSpawn))
	{
		// 4J-PB changed to stay within our level limits
		xSpawn += random.nextInt(64) - random.nextInt(64);
		if(xSpawn>maxXZ) xSpawn=0;
		if(xSpawn<minXZ) xSpawn=0;
		zSpawn += random.nextInt(64) - random.nextInt(64);
		if(zSpawn>maxXZ) zSpawn=0;
		if(zSpawn<minXZ) zSpawn=0;

		if (++tries == 1000) break;
	}

	levelData->setSpawn(xSpawn, ySpawn, zSpawn);
	if (levelSettings->hasStartingBonusItems())
	{
		generateBonusItemsNearSpawn();
	}
	isFindingSpawn = false;
}

// 4J - brought forward from 1.3.2
void ServerLevel::generateBonusItemsNearSpawn()
{
	// once we've found the initial spawn, try to find a location for the
	// starting bonus chest
	// 4J - added - scan the spawn area first to see if there's already a chest near here

	static const int r = 20;
	int xs = levelData->getXSpawn();
	int zs = levelData->getZSpawn();
	for( int xx = -r; xx <= r; xx++ )
		for( int zz = -r; zz <= r; zz++ )
		{
			int x = xx + xs;
			int z = zz + zs;
			int y = getTopSolidBlock( x, z ) - 1;

			if( getTile( x, y, z ) == Tile::chest_Id )
			{
				shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity>(getTileEntity(x, y, z));
				if (chest != nullptr)
				{
					if( chest->isBonusChest )
					{
						return;
					}
				}
			}
		}

		BonusChestFeature *feature = new BonusChestFeature(RANDOM_BONUS_ITEMS, 16);
		for (int attempt = 0; attempt < 16; attempt++)
		{
			int x = levelData->getXSpawn() + random->nextInt(6) - random->nextInt(6);
			int z = levelData->getZSpawn() + random->nextInt(6) - random->nextInt(6);
			int y = getTopSolidBlock(x, z) + 1;

			if (feature->place(this, random, x, y, z, (attempt == 15) ))
			{
				break;
			}
		}
		delete feature;
}

Pos *ServerLevel::getDimensionSpecificSpawn()
{
	return dimension->getSpawnPos();
}

// 4j Added for XboxOne PLM
void ServerLevel::Suspend()
{
	if(StorageManager.GetSaveDisabled()) return;
	saveLevelData();
	chunkSource->saveAllEntities();
}

void ServerLevel::save(bool force, ProgressListener *progressListener, bool bAutosave)
{
	if (!chunkSource->shouldSave()) return;

	// 4J-PB - check that saves are enabled
	if(StorageManager.GetSaveDisabled()) return;


	if (progressListener != nullptr)
	{
		if(bAutosave)
		{
			progressListener->progressStartNoAbort(IDS_PROGRESS_AUTOSAVING_LEVEL);
		}
		else
		{
			progressListener->progressStartNoAbort(IDS_PROGRESS_SAVING_LEVEL);
		}

	}
	PIXBeginNamedEvent(0,"Saving level data");
	saveLevelData();
	PIXEndNamedEvent();

	if (progressListener != nullptr) progressListener->progressStage(IDS_PROGRESS_SAVING_CHUNKS);

#if defined(_XBOX_ONE) || defined(__ORBIS__)
	// Our autosave is a minimal save. All the chunks are saves by the constant save process
	if(bAutosave)
	{
		chunkSource->saveAllEntities();
	}
	else
#endif
	{
		chunkSource->save(force, progressListener);

#ifdef _LARGE_WORLDS
		// 4J Stu - Only do this if there are players in the level
		if(chunkMap->players.size() > 0)
		{
			// 4J Stu - This will come in a later change anyway
			// clean cache
			vector<LevelChunk *> *loadedChunkList = cache->getLoadedChunkList();
			for ( LevelChunk *lc : *loadedChunkList )
			{
				if ( lc && !chunkMap->hasChunk(lc->x, lc->z) )
				{
					cache->drop(lc->x, lc->z);
				}
			}
		}
#endif
	}

	//if( force && !isClientSide )
	//{
	//	if (progressListener != nullptr) progressListener->progressStage(IDS_PROGRESS_SAVING_TO_DISC);
	//	levelStorage->flushSaveFile();
	//}
}

// 4J Added
void ServerLevel::saveToDisc(ProgressListener *progressListener, bool autosave)
{
	// 4J-PB - check that saves are enabled
	if(StorageManager.GetSaveDisabled()) return;

	// Check if we are using a trial version of a texture pack (which will be the case for going into the mash-up pack world with a trial version)
	if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
	{
		TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
		DLCTexturePack *pDLCTexPack=static_cast<DLCTexturePack *>(tPack);

		DLCPack * pDLCPack=pDLCTexPack->getDLCInfoParentPack();

		if(!pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
		{
			return;
		}
	}

	if (progressListener != nullptr) progressListener->progressStage(IDS_PROGRESS_SAVING_TO_DISC);
	levelStorage->flushSaveFile(autosave);
}

void ServerLevel::saveLevelData()
{
	checkSession();

	levelStorage->saveLevelData(levelData, &players);
	savedDataStorage->save();
}

void ServerLevel::entityAdded(shared_ptr<Entity> e)
{
	Level::entityAdded(e);
	entitiesById[e->entityId] = e;
	vector<shared_ptr<Entity> > *es = e->getSubEntities();
	if (es)
	{
		for(auto& i : *es)
		{
			entitiesById.emplace(i->entityId, i);
		}
	}
	entityAddedExtra(e);	// 4J added
}

void ServerLevel::entityRemoved(shared_ptr<Entity> e)
{
	Level::entityRemoved(e);
	entitiesById.erase(e->entityId);
	vector<shared_ptr<Entity> > *es = e->getSubEntities();
	if (es)
	{
		for(auto& i : *es)
		{
			entitiesById.erase(i->entityId);
		}
	}
	entityRemovedExtra(e);		// 4J added
}

shared_ptr<Entity> ServerLevel::getEntity(int id)
{
    auto it = entitiesById.find(id);
    if (it != entitiesById.end())
    {
        return it->second;
    }
    return nullptr;
}

bool ServerLevel::addGlobalEntity(shared_ptr<Entity> e)
{
	if (Level::addGlobalEntity(e))
	{
		server->getPlayers()->broadcast(e->x, e->y, e->z, 512, dimension->id, std::make_shared<AddGlobalEntityPacket>(e));
		return true;
	}
	return false;
}

void ServerLevel::broadcastEntityEvent(shared_ptr<Entity> e, byte event)
{
	shared_ptr<Packet> p = std::make_shared<EntityEventPacket>(e->entityId, event);
	server->getLevel(dimension->id)->getTracker()->broadcastAndSend(e, p);
}

shared_ptr<Explosion> ServerLevel::explode(shared_ptr<Entity> source, double x, double y, double z, float r, bool fire, bool destroyBlocks)
{
	// instead of calling super, we run the same explosion code here except
	// we don't generate any particles
	shared_ptr<Explosion> explosion = std::make_shared<Explosion>(this, source, x, y, z, r);
	explosion->fire = fire;
	explosion->destroyBlocks = destroyBlocks;
	explosion->explode();
	explosion->finalizeExplosion(false);

	if (!destroyBlocks)
	{
		explosion->toBlow.clear();
	}

	vector<shared_ptr<ServerPlayer> > sentTo;
	for(auto& it : players)
	{
		shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(it);
		if (player->dimension != dimension->id) continue;

		bool knockbackOnly = false;
		if( sentTo.size() )
		{
			INetworkPlayer *thisPlayer = player->connection->getNetworkPlayer();
			if( thisPlayer == nullptr )
			{
				continue;
			}
			else
			{
				for(auto& player2 : sentTo)
				{
					INetworkPlayer *otherPlayer = player2->connection->getNetworkPlayer();
					if( otherPlayer != nullptr && thisPlayer->IsSameSystem(otherPlayer) )
					{
						knockbackOnly = true;
					}
				}
			}
		}

		if (player->distanceToSqr(x, y, z) < 64 * 64)
		{
			Vec3 *knockbackVec = explosion->getHitPlayerKnockback(player);
			//app.DebugPrintf("Sending %s with knockback (%f,%f,%f)\n", knockbackOnly?"knockbackOnly":"allExplosion",knockbackVec->x,knockbackVec->y,knockbackVec->z);
			// If the player is not the primary on the system, then we only want to send info for the knockback
			player->connection->send(std::make_shared<ExplodePacket>(x, y, z, r, &explosion->toBlow, knockbackVec, knockbackOnly));
			sentTo.push_back( player );
		}
	}

	return explosion;
}

void ServerLevel::tileEvent(int x, int y, int z, int tile, int b0, int b1)
{
	//        super.tileEvent(x, y, z, b0, b1);
	//        server.getPlayers().broadcast(x, y, z, 64, dimension.id, new TileEventPacket(x, y, z, b0, b1));
	TileEventData newEvent(x, y, z, tile, b0, b1);
	//for (TileEventData te : tileEvents[activeTileEventsList])
	for(auto& it : tileEvents[activeTileEventsList])
	{
		if (it.equals(newEvent))
		{
			return;
		}
	}
	tileEvents[activeTileEventsList].push_back(newEvent);
}

void ServerLevel::runTileEvents()
{
	// use two lists until both are empty, intended to avoid concurrent
	// modifications
	while (!tileEvents[activeTileEventsList].empty())
	{
		int runList = activeTileEventsList;
		activeTileEventsList ^= 1;

		for(auto& it : tileEvents[runList])
		{
			if (doTileEvent(&it))
			{
				TileEventData te = it;
				server->getPlayers()->broadcast(te.getX(), te.getY(), te.getZ(), 64, dimension->id, std::make_shared<TileEventPacket>(te.getX(), te.getY(), te.getZ(), te.getTile(), te.getParamA(), te.getParamB()));
			}
		}
		tileEvents[runList].clear();
	}
}

bool ServerLevel::doTileEvent(TileEventData *te)
{
	int t = getTile(te->getX(), te->getY(), te->getZ());
	if (t == te->getTile()) {
		return Tile::tiles[t]->triggerEvent(this, te->getX(), te->getY(), te->getZ(), te->getParamA(), te->getParamB());
	}
	return false;
}

void ServerLevel::closeLevelStorage()
{
	levelStorage->closeAll();
}

void ServerLevel::tickWeather()
{
	bool wasRaining = isRaining();
	Level::tickWeather();

	if (wasRaining != isRaining())
	{
		if (wasRaining)
		{
			server->getPlayers()->broadcastAll(std::make_shared<GameEventPacket>(GameEventPacket::STOP_RAINING, 0));
		}
		else
		{
			server->getPlayers()->broadcastAll(std::make_shared<GameEventPacket>(GameEventPacket::START_RAINING, 0));
		}
	}

}

MinecraftServer *ServerLevel::getServer()
{
	return server;
}

EntityTracker *ServerLevel::getTracker()
{
	return tracker;
}

void ServerLevel::setTimeAndAdjustTileTicks(int64_t newTime)
{
	int64_t delta = newTime - levelData->getGameTime();
	// 4J - can't directly adjust m_delay in a set as it has a const interator, since changing values in here might change the ordering of the elements in the set.
	// Instead move to a vector, do the adjustment, put back in the set.
	vector<TickNextTickData> temp;
	for(const auto& it : tickNextTickList)
	{
		temp.push_back(it);
		temp.back().m_delay += delta;
	}
	tickNextTickList.clear();
	for(const auto& it : temp)
	{
		tickNextTickList.insert(it);
	}
	setGameTime(newTime);
}

PlayerChunkMap *ServerLevel::getChunkMap()
{
	return chunkMap;
}

PortalForcer *ServerLevel::getPortalForcer()
{
	return portalForcer;
}

void ServerLevel::sendParticles(const wstring &name, double x, double y, double z, int count)
{
	sendParticles(name, x + 0.5f, y + 0.5f, z + 0.5f, count, 0.5f, 0.5f, 0.5f, 0.02f);
}

void ServerLevel::sendParticles(const wstring &name, double x, double y, double z, int count, double xDist, double yDist, double zDist, double speed)
{
	shared_ptr<Packet> packet = std::make_shared<LevelParticlesPacket>( name, static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(xDist), static_cast<float>(yDist), static_cast<float>(zDist), static_cast<float>(speed), count );

	for(auto& it : players)
	{
		shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(it);
		player->connection->send(packet);
	}
}

// 4J Stu - Sometimes we want to update tiles on the server from the main thread (eg SignTileEntity when string verify returns)
void ServerLevel::queueSendTileUpdate(int x, int y, int z)
{
	EnterCriticalSection(&m_csQueueSendTileUpdates);
	m_queuedSendTileUpdates.push_back( new Pos(x,y,z) );
	LeaveCriticalSection(&m_csQueueSendTileUpdates);
}

void ServerLevel::runQueuedSendTileUpdates()
{
	EnterCriticalSection(&m_csQueueSendTileUpdates);
	for(Pos* p : m_queuedSendTileUpdates)
	{
		sendTileUpdated(p->x, p->y, p->z);
		delete p;
	}
	m_queuedSendTileUpdates.clear();
	LeaveCriticalSection(&m_csQueueSendTileUpdates);
}

// 4J - added special versions of addEntity and extra processing on entity removed and added so we can limit the number of itementities created
bool ServerLevel::addEntity(shared_ptr<Entity> e)
{
	// If its an item entity, and we've got to our capacity, delete the oldest
	if( e->instanceof(eTYPE_ITEMENTITY) )
	{
		//		printf("Adding item entity count %d\n",m_itemEntities.size());
		EnterCriticalSection(&m_limiterCS);
		if( m_itemEntities.size() >= MAX_ITEM_ENTITIES )
		{
			//			printf("Adding - doing remove\n");
			removeEntityImmediately(m_itemEntities.front());
		}
		LeaveCriticalSection(&m_limiterCS);
	}
	// If its an hanging entity, and we've got to our capacity, delete the oldest
	else if( e->instanceof(eTYPE_HANGING_ENTITY) )
	{
		//		printf("Adding item entity count %d\n",m_itemEntities.size());
		EnterCriticalSection(&m_limiterCS);
		if( m_hangingEntities.size() >= MAX_HANGING_ENTITIES )
		{
			//			printf("Adding - doing remove\n");

			// 4J-PB - refuse to add the entity, since we'll be removing one already there, and it may be an item frame with something in it.
			LeaveCriticalSection(&m_limiterCS);
			return FALSE;

			//removeEntityImmediately(m_hangingEntities.front());
		}
		LeaveCriticalSection(&m_limiterCS);
	}
	// If its an arrow entity, and we've got to our capacity, delete the oldest
	else if( e->instanceof(eTYPE_ARROW) )
	{
		//		printf("Adding arrow entity count %d\n",m_arrowEntities.size());
		EnterCriticalSection(&m_limiterCS);
		if( m_arrowEntities.size() >= MAX_ARROW_ENTITIES )
		{
			//			printf("Adding - doing remove\n");
			removeEntityImmediately(m_arrowEntities.front());
		}
		LeaveCriticalSection(&m_limiterCS);
	}
	// If its an experience orb entity, and we've got to our capacity, delete the oldest
	else if( e->instanceof(eTYPE_EXPERIENCEORB) )
	{
		//		printf("Adding arrow entity count %d\n",m_arrowEntities.size());
		EnterCriticalSection(&m_limiterCS);
		if( m_experienceOrbEntities.size() >= MAX_EXPERIENCEORB_ENTITIES )
		{
			//			printf("Adding - doing remove\n");
			removeEntityImmediately(m_experienceOrbEntities.front());
		}
		LeaveCriticalSection(&m_limiterCS);
	}
	return Level::addEntity(e);
}

// 4J: Returns true if the level is at its limit for this type of entity (only checks arrows, hanging, item and experience orbs)
bool ServerLevel::atEntityLimit(shared_ptr<Entity> e)
{
	// TODO: This duplicates code from addEntity above, fix

	bool atLimit = false;

	if( e->instanceof(eTYPE_ITEMENTITY) )
	{
		EnterCriticalSection(&m_limiterCS);
		atLimit = m_itemEntities.size() >= MAX_ITEM_ENTITIES;
		LeaveCriticalSection(&m_limiterCS);
	}
	else if( e->instanceof(eTYPE_HANGING_ENTITY) )
	{
		EnterCriticalSection(&m_limiterCS);
		atLimit = m_hangingEntities.size() >= MAX_HANGING_ENTITIES;
		LeaveCriticalSection(&m_limiterCS);
	}
	else if( e->instanceof(eTYPE_ARROW) )
	{
		EnterCriticalSection(&m_limiterCS);
		atLimit = m_arrowEntities.size() >= MAX_ARROW_ENTITIES;
		LeaveCriticalSection(&m_limiterCS);
	}
	else if( e->instanceof(eTYPE_EXPERIENCEORB) )
	{
		EnterCriticalSection(&m_limiterCS);
		atLimit = m_experienceOrbEntities.size() >= MAX_EXPERIENCEORB_ENTITIES;
		LeaveCriticalSection(&m_limiterCS);
	}

	return atLimit;
}

// Maintain a cound of primed tnt & falling tiles in this level
void ServerLevel::entityAddedExtra(shared_ptr<Entity> e)
{
	if( e->instanceof(eTYPE_ITEMENTITY) )
	{
		EnterCriticalSection(&m_limiterCS);
		m_itemEntities.push_back(e);
		//		printf("entity added: item entity count now %d\n",m_itemEntities.size());
		LeaveCriticalSection(&m_limiterCS);
	}
	else if( e->instanceof(eTYPE_HANGING_ENTITY) )
	{
		EnterCriticalSection(&m_limiterCS);
		m_hangingEntities.push_back(e);
		//		printf("entity added: item entity count now %d\n",m_itemEntities.size());
		LeaveCriticalSection(&m_limiterCS);
	}
	else if( e->instanceof(eTYPE_ARROW) )
	{
		EnterCriticalSection(&m_limiterCS);
		m_arrowEntities.push_back(e);
		//		printf("entity added: arrow entity count now %d\n",m_arrowEntities.size());
		LeaveCriticalSection(&m_limiterCS);
	}
	else if( e->instanceof(eTYPE_EXPERIENCEORB) )
	{
		EnterCriticalSection(&m_limiterCS);
		m_experienceOrbEntities.push_back(e);
		//		printf("entity added: experience orb entity count now %d\n",m_arrowEntities.size());
		LeaveCriticalSection(&m_limiterCS);
	}
	else if( e->instanceof(eTYPE_PRIMEDTNT) )
	{
		EnterCriticalSection(&m_limiterCS);
		m_primedTntCount++;
		LeaveCriticalSection(&m_limiterCS);
	}
	else if( e->instanceof(eTYPE_FALLINGTILE) )
	{
		EnterCriticalSection(&m_limiterCS);
		m_fallingTileCount++;
		LeaveCriticalSection(&m_limiterCS);
	}
}

// Maintain a cound of primed tnt & falling tiles in this level, and remove any item entities from our list
void ServerLevel::entityRemovedExtra(shared_ptr<Entity> e)
{
	if( e->instanceof(eTYPE_ITEMENTITY) )
	{
		EnterCriticalSection(&m_limiterCS);
		//		printf("entity removed: item entity count %d\n",m_itemEntities.size());
        auto it = find(m_itemEntities.begin(), m_itemEntities.end(), e);
        if( it != m_itemEntities.end() )
		{
			//			printf("Item to remove found\n");
			m_itemEntities.erase(it);
		}
		//		printf("entity removed: item entity count now %d\n",m_itemEntities.size());
		LeaveCriticalSection(&m_limiterCS);
	}
	else if( e->instanceof(eTYPE_HANGING_ENTITY) )
	{
		EnterCriticalSection(&m_limiterCS);
		//		printf("entity removed: item entity count %d\n",m_itemEntities.size());
        auto it = find(m_hangingEntities.begin(), m_hangingEntities.end(), e);
        if( it != m_hangingEntities.end() )
		{
			//			printf("Item to remove found\n");
			m_hangingEntities.erase(it);
		}
		//		printf("entity removed: item entity count now %d\n",m_itemEntities.size());
		LeaveCriticalSection(&m_limiterCS);
	}
	else if( e->instanceof(eTYPE_ARROW) )
	{
		EnterCriticalSection(&m_limiterCS);
		//		printf("entity removed: arrow entity count %d\n",m_arrowEntities.size());
        auto it = find(m_arrowEntities.begin(), m_arrowEntities.end(), e);
        if( it != m_arrowEntities.end() )
		{
			//			printf("Item to remove found\n");
			m_arrowEntities.erase(it);
		}
		//		printf("entity removed: arrow entity count now %d\n",m_arrowEntities.size());
		LeaveCriticalSection(&m_limiterCS);
	}
	else if( e->instanceof(eTYPE_EXPERIENCEORB) )
	{
		EnterCriticalSection(&m_limiterCS);
		//		printf("entity removed: experience orb entity count %d\n",m_arrowEntities.size());
        auto it = find(m_experienceOrbEntities.begin(), m_experienceOrbEntities.end(), e);
        if( it != m_experienceOrbEntities.end() )
		{
			//			printf("Item to remove found\n");
			m_experienceOrbEntities.erase(it);
		}
		//		printf("entity removed: experience orb entity count now %d\n",m_arrowEntities.size());
		LeaveCriticalSection(&m_limiterCS);
	}
	else if( e->instanceof(eTYPE_PRIMEDTNT) )
	{
		EnterCriticalSection(&m_limiterCS);
		m_primedTntCount--;
		LeaveCriticalSection(&m_limiterCS);
	}
	else if( e->instanceof(eTYPE_FALLINGTILE) )
	{
		EnterCriticalSection(&m_limiterCS);
		m_fallingTileCount--;
		LeaveCriticalSection(&m_limiterCS);
	}
}

bool ServerLevel::newPrimedTntAllowed()
{
	EnterCriticalSection(&m_limiterCS);
	bool retval = m_primedTntCount < MAX_PRIMED_TNT;
	LeaveCriticalSection(&m_limiterCS);
	return retval;
}

bool ServerLevel::newFallingTileAllowed()
{
	EnterCriticalSection(&m_limiterCS);
	bool retval = m_fallingTileCount < MAX_FALLING_TILE;
	LeaveCriticalSection(&m_limiterCS);
	return retval;
}

int ServerLevel::runUpdate(void* lpParam)
{
	ShutdownManager::HasStarted(ShutdownManager::eRunUpdateThread,m_updateTrigger);
	while(ShutdownManager::ShouldRun(ShutdownManager::eRunUpdateThread))
	{
		m_updateTrigger->WaitForAll(INFINITE);

		if(!ShutdownManager::ShouldRun(ShutdownManager::eRunUpdateThread)) break;

		PIXBeginNamedEvent(0,"Updating tiles to be ticked");
		// 4J Stu - Grass and Lava ticks currently take up the majority of all tile updates, so I am limiting them
		int grassTicks = 0;
		int lavaTicks = 0;
		for( unsigned int iLev = 0; iLev < 3; ++iLev )
		{
			EnterCriticalSection(&m_updateCS[iLev]);
			for( int i = 0; i < m_updateChunkCount[iLev]; i++ )
			{
				// 4J - some of these tile ticks will check things in neighbouring tiles, causing chunks to load/create that aren't yet in memory.
				// Try and avoid this by limiting the min/max x & z values that we will try and inspect in this chunk according to what surround chunks are loaded
				int cx = m_updateChunkX[iLev][i];
				int cz = m_updateChunkZ[iLev][i];
				int minx = 0;
				int maxx = 15;
				int minz = 0;
				int maxz = 15;
				if( !m_level[iLev]->hasChunk(cx, cz) ) continue;

				if( !m_level[iLev]->hasChunk(cx + 1, cz + 0) ) { maxx = 11; }
				if( !m_level[iLev]->hasChunk(cx + 0, cz + 1) ) { maxz = 11; }
				if( !m_level[iLev]->hasChunk(cx - 1, cz + 0) ) { minx = 4; }
				if( !m_level[iLev]->hasChunk(cx + 0, cz - 1) ) { minz = 4; }
				if( !m_level[iLev]->hasChunk(cx + 1, cz + 1) ) { maxx = 11; maxz = 11; }
				if( !m_level[iLev]->hasChunk(cx + 1, cz - 1) ) { maxx = 11; minz = 4; }
				if( !m_level[iLev]->hasChunk(cx - 1, cz - 1) ) { minx = 4;  minz = 4; }
				if( !m_level[iLev]->hasChunk(cx - 1, cz + 1) ) { minx = 4;  maxz = 11; }

				LevelChunk *lc = m_level[iLev]->getChunk(cx, cz);

				for (int j = 0; j < 80; j++)
				{
					m_randValue[iLev] = m_randValue[iLev] * 3 + m_level[iLev]->addend;
					int val = (m_randValue[iLev] >> 2);
					int x = (val & 15);
					if( ( x < minx ) || ( x > maxx ) ) continue;
					int z = ((val >> 8) & 15);
					if( ( z < minz ) || ( z > maxz ) ) continue;
					int y = ((val >> 16) & (Level::maxBuildHeight - 1));

					// This array access is a cache miss pretty much every time
					int id = lc->getTile(x,y,z);
					if( m_updateTileCount[iLev] >= MAX_UPDATES ) break;

					// 4J Stu - Grass and Lava ticks currently take up the majority of all tile updates, so I am limiting them
					if( (id == Tile::grass_Id && grassTicks >= MAX_GRASS_TICKS) || (id == Tile::calmLava_Id && lavaTicks >= MAX_LAVA_TICKS) ) continue;

					// 4J Stu - Added shouldTileTick as some tiles won't even do anything if they are set to tick and use up one of our updates
					if (Tile::tiles[id] != nullptr && Tile::tiles[id]->isTicking() && Tile::tiles[id]->shouldTileTick(m_level[iLev],x + (cx * 16), y, z + (cz * 16) ) )
					{
						if(id == Tile::grass_Id) ++grassTicks;
						else if(id == Tile::calmLava_Id) ++lavaTicks;
						m_updateTileX[iLev][m_updateTileCount[iLev]] = x + (cx * 16);
						m_updateTileY[iLev][m_updateTileCount[iLev]] = y;
						m_updateTileZ[iLev][m_updateTileCount[iLev]] = z + (cz * 16);

						m_updateTileCount[iLev]++;
					}
				}
			}
			LeaveCriticalSection(&m_updateCS[iLev]);
		}
		PIXEndNamedEvent();
#ifdef __PS3__
		Sleep(10);
#endif //__PS3__
	}

	ShutdownManager::HasFinished(ShutdownManager::eRunUpdateThread);

	return 0;
}

void ServerLevel::flagEntitiesToBeRemoved(unsigned int *flags, bool *removedFound)
{
	if( chunkMap )
	{
		chunkMap->flagEntitiesToBeRemoved(flags, removedFound);
	}
}