#pragma once
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\JavaIntHash.h"
class ServerChunkCache;
class MinecraftServer;
class Node;
class EntityTracker;
class PlayerChunkMap;
using namespace std;

class ServerLevel : public Level
{
private:
	static const int EMPTY_TIME_NO_TICK = SharedConstants::TICKS_PER_SECOND * 3;

	MinecraftServer *server;
	EntityTracker *tracker;
	PlayerChunkMap *chunkMap;

	CRITICAL_SECTION				m_tickNextTickCS;	// 4J added
	set<TickNextTickData, TickNextTickDataKeyCompare> tickNextTickList; // 4J Was TreeSet
	unordered_set<TickNextTickData, TickNextTickDataKeyHash, TickNextTickDataKeyEq> tickNextTickSet; // 4J Was HashSet

	vector<Pos *> m_queuedSendTileUpdates; // 4J added
	CRITICAL_SECTION m_csQueueSendTileUpdates;

protected:
	int saveInterval;

public:
	ServerChunkCache *cache;
    bool canEditSpawn;
    bool noSave;
private:
	bool allPlayersSleeping;
	int emptyTime;
	bool m_bAtLeastOnePlayerSleeping; // 4J Added
	static WeighedTreasureArray RANDOM_BONUS_ITEMS;	// 4J - brought forward from 1.3.2

	vector<TileEventData> tileEvents[2];
	int activeTileEventsList;
public:
	static void staticCtor();
	ServerLevel(MinecraftServer *server, shared_ptr<LevelStorage>levelStorage, const wstring& levelName, int dimension, LevelSettings *levelSettings);
	~ServerLevel();
	void tick();
	Biome::MobSpawnerData *getRandomMobSpawnAt(MobCategory *mobCategory, int x, int y, int z);
	void updateSleepingPlayerList();
protected:
	void awakenAllPlayers();

private:
	void stopWeather();

public:
	bool allPlayersAreSleeping();
	void validateSpawn();

protected:
	void tickTiles();

public:
	void addToTickNextTick(int x, int y, int z, int tileId, int tickDelay);
	void forceAddTileTick(int x, int y, int z, int tileId, int tickDelay);
	void tickEntities();
	bool tickPendingTicks(bool force);
	vector<TickNextTickData> *fetchTicksInChunk(LevelChunk *chunk, bool remove);
    virtual void tick(shared_ptr<Entity> e, bool actual);
    void forceTick(shared_ptr<Entity> e, bool actual);
	bool AllPlayersAreSleeping()			{ return allPlayersSleeping;} // 4J added for a message to other players
	bool isAtLeastOnePlayerSleeping()		{ return m_bAtLeastOnePlayerSleeping;}
protected:
	ChunkSource *createChunkSource();	// 4J - was virtual, but was called from parent ctor
public:
	vector<shared_ptr<TileEntity> > *getTileEntitiesInRegion(int x0, int y0, int z0, int x1, int y1, int z1);
    virtual bool mayInteract(shared_ptr<Player> player, int xt, int yt, int zt, int id);
protected:
	virtual void initializeLevel(LevelSettings *settings);
	virtual void setInitialSpawn(LevelSettings *settings);
	void generateBonusItemsNearSpawn();	// 4J - brought forward from 1.3.2

public:
	Pos *getDimensionSpecificSpawn();

	void Suspend(); // 4j Added for XboxOne PLM

	void save(bool force, ProgressListener *progressListener, bool bAutosave=false);
	void saveToDisc(ProgressListener *progressListener, bool autosave); // 4J Added

private:
	void saveLevelData();

	typedef unordered_map<int, shared_ptr<Entity> , IntKeyHash2, IntKeyEq> intEntityMap;
	intEntityMap entitiesById;	// 4J - was IntHashMap, using same hashing function as this uses
protected:
	virtual void entityAdded(shared_ptr<Entity> e);
    virtual void entityRemoved(shared_ptr<Entity> e);
public:
	shared_ptr<Entity> getEntity(int id);
    virtual bool addGlobalEntity(shared_ptr<Entity> e);
    void broadcastEntityEvent(shared_ptr<Entity> e, byte event);
    virtual shared_ptr<Explosion> explode(shared_ptr<Entity> source, double x, double y, double z, float r, bool fire, bool destroyBlocks);
    virtual void tileEvent(int x, int y, int z, int tile, int b0, int b1);

private:
	void runTileEvents();
	bool doTileEvent(TileEventData *te);

public:
    void closeLevelStorage();
protected:
	virtual void tickWeather();

public:
	MinecraftServer *getServer();
	EntityTracker *getTracker();
	void setTimeAndAdjustTileTicks(__int64 newTime);
	PlayerChunkMap *getChunkMap();

	void queueSendTileUpdate(int x, int y, int z); // 4J Added
private:
	void runQueuedSendTileUpdates();// 4J Added

	// 4J - added for implementation of finite limit to number of item entities, tnt and falling block entities
public:

	static const int MAX_HANGING_ENTITIES = 400;
	static const int MAX_ITEM_ENTITIES = 200;
	static const int MAX_ARROW_ENTITIES = 200;
	static const int MAX_EXPERIENCEORB_ENTITIES = 50;
	static const int MAX_PRIMED_TNT = 20;
	static const int MAX_FALLING_TILE = 20;

	int							m_primedTntCount;
	int							m_fallingTileCount;
	CRITICAL_SECTION			m_limiterCS;
	list< shared_ptr<Entity> >	m_itemEntities;
	list< shared_ptr<Entity> >	m_hangingEntities;
	list< shared_ptr<Entity> >	m_arrowEntities;
	list< shared_ptr<Entity> >	m_experienceOrbEntities;

	virtual bool addEntity(shared_ptr<Entity> e);
	void entityAddedExtra(shared_ptr<Entity> e);
	void entityRemovedExtra(shared_ptr<Entity> e);
	
	virtual bool newPrimedTntAllowed();
	virtual bool newFallingTileAllowed();

	void flagEntitiesToBeRemoved(unsigned int *flags, bool *removedFound);		// 4J added

	// 4J added
	static const int MAX_UPDATES = 256;

	// Each of these need to be duplicated for each level in the current game. As we currently only have 2 (over/nether), making this constant
	static Level 				*m_level[3];
	static int					m_updateChunkX[3][LEVEL_CHUNKS_TO_UPDATE_MAX];
	static int					m_updateChunkZ[3][LEVEL_CHUNKS_TO_UPDATE_MAX];
	static int					m_updateChunkCount[3];
	static int					m_updateTileX[3][MAX_UPDATES];
	static int					m_updateTileY[3][MAX_UPDATES];
	static int					m_updateTileZ[3][MAX_UPDATES];
	static int					m_updateTileCount[3];
	static int					m_randValue[3];

	static C4JThread::EventArray*	m_updateTrigger;
	static CRITICAL_SECTION		m_updateCS[3];

	static C4JThread*			m_updateThread;
	static int 	runUpdate(void* lpParam);

};
