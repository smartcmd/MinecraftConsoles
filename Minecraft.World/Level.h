#pragma once
using namespace std;
#include "LevelSource.h"
#include "LightLayer.h"
#include "ChunkPos.h"
#include "TickNextTickData.h"
#include "SavedData.h"
#include "Definitions.h"
#include "ParticleTypes.h"
#include "biome.h"
#include "C4JThread.h"

#ifdef __PSVITA__
#include "..\Minecraft.Client\PSVita\PSVitaExtras\CustomSet.h"
#endif

// 4J Stu - This value should be big enough that we don't get any crashes causes by memory overwrites,
// however it does seem way too large for what is actually needed. Needs further investigation
#ifdef MINECRAFT_SERVER_BUILD
// fixes a crash when 8+ players are present
#define LEVEL_CHUNKS_TO_UPDATE_MAX (32*32*8)
#else
#define LEVEL_CHUNKS_TO_UPDATE_MAX (19*19*8)
#endif

class Vec3;
class ChunkSource;
class LevelListener;
class Explosion;
class Dimension;
class Material;
class TileEntity;
class AABB;
class Entity;
class SavedData;
class Pos;
class Player;
class LevelData;
class ProgressListener;
class Random;
class LevelStorage;
class SavedDataStorage;
class HitResult;
class Path;
class LevelSettings;
class Biome;
class Villages;
class VillageSiege;
class Tickable;
class Minecart;
class EntitySelector;
class Scoreboard;
class GameRules;

class Level : public LevelSource
{
public:
	static const int MAX_TICK_TILES_PER_TICK = 1000;

	// 4J Added
	static const int MAX_GRASS_TICKS = 100;
	static const int MAX_LAVA_TICKS = 100;


public:
	static const int MAX_XBOX_BOATS = 40;										// Max number of boats
	static const int MAX_CONSOLE_MINECARTS = 40;
	static const int MAX_DISPENSABLE_FIREBALLS = 200;
	static const int MAX_DISPENSABLE_PROJECTILES = 300;

	static const int MAX_LEVEL_SIZE = 30000000;
	static const int maxMovementHeight = 512;	// 4J added

	static const int minBuildHeight = 0;		// 4J - brought forward from 1.2.3
	static const int maxBuildHeight = 256;		// 4J - brought forward from 1.2.3
	static const int genDepthBits = 7;
	static const int genDepthBitsPlusFour = genDepthBits + 4;
	static const int genDepth = 1 << genDepthBits;
	static const int genDepthMinusOne = genDepth - 1;
	static const int constSeaLevel = genDepth / 2 - 1;

	static const int CHUNK_TILE_COUNT = maxBuildHeight * 16 * 16;
	static const int HALF_CHUNK_TILE_COUNT = CHUNK_TILE_COUNT/2;
	static const int COMPRESSED_CHUNK_SECTION_HEIGHT = 128;
	static const int COMPRESSED_CHUNK_SECTION_TILES = COMPRESSED_CHUNK_SECTION_HEIGHT * 16 * 16; // 4J Stu - Fixed size

	int seaLevel;

	// 4J - added, making instaTick flag use TLS so we can set it in the chunk rebuilding thread without upsetting the main game thread
	static DWORD tlsIdx;
	static DWORD tlsIdxLightCache;
	static void enableLightingCache();
	static void destroyLightingCache();
	static bool getCacheTestEnabled();
	static bool getInstaTick();
	static void setInstaTick(bool enable);
	//	bool instaTick;		// 4J - removed

	static const int MAX_BRIGHTNESS = 15;
	static const int TICKS_PER_DAY = 20 * 60 * 20; // ORG:20*60*20

public:
	CRITICAL_SECTION				m_entitiesCS;		// 4J added

	vector<shared_ptr<Entity> > entities;

protected:
	vector<shared_ptr<Entity> > entitiesToRemove;
public:
	bool							hasEntitiesToRemove();	// 4J added
	bool							m_bDisableAddNewTileEntities; // 4J Added
	CRITICAL_SECTION				m_tileEntityListCS;		// 4J added
	vector<shared_ptr<TileEntity> > tileEntityList;
private:
	vector<shared_ptr<TileEntity> > pendingTileEntities;
	vector<shared_ptr<TileEntity> > tileEntitiesToUnload;
	bool updatingTileEntities;
public:
	vector<shared_ptr<Player> > players;
	vector<shared_ptr<Entity> > globalEntities;

private:
	int cloudColor;

public:
	int skyDarken;

protected:
	int randValue;

public:
	int addend;

protected:
	float oRainLevel, rainLevel;
	float oThunderLevel, thunderLevel;

public:
	int skyFlashTime;

	int difficulty;
	Random *random;
	bool isNew;
	Dimension *dimension;

protected:
	vector<LevelListener *> listeners;

public:
	ChunkSource *chunkSource;		// 4J - changed to public
protected:
	// This is the only shared_ptr ref to levelStorage - we need to keep this as long as at least one Level references it,
	// to be able to cope with moving from dimension to dimension where the Level(Level *level, Dimension *dimension) ctor is used
	shared_ptr<LevelStorage> levelStorage;

	LevelData *levelData;

public:
	bool isFindingSpawn;
	SavedDataStorage *savedDataStorage;
	shared_ptr<Villages> villages;
	VillageSiege *villageSiege;

private:
	// 4J - Calendar is now static
	// Calendar *calendar;

protected:
	Scoreboard *scoreboard;

public:
	Biome *getBiome(int x, int z);				// 4J - brought forward from 1.2.3
	virtual BiomeSource *getBiomeSource();

private:

	// 4J Stu - Added these ctors to handle init of member variables
	void _init();
	void _init(shared_ptr<LevelStorage>levelStorage, const wstring& levelName, LevelSettings *levelSettings, Dimension *fixedDimension, bool doCreateChunkSource = true);

public:
	Level(shared_ptr<LevelStorage>levelStorage, const wstring& name, Dimension *dimension, LevelSettings *levelSettings, bool doCreateChunkSource = true);
	Level(shared_ptr<LevelStorage>levelStorage, const wstring& levelName, LevelSettings *levelSettings);
	Level(shared_ptr<LevelStorage>levelStorage, const wstring& levelName, LevelSettings *levelSettings, Dimension *fixedDimension, bool doCreateChunkSource = true);

	virtual ~Level();

protected:
	virtual ChunkSource *createChunkSource() = 0;

	virtual void initializeLevel(LevelSettings *settings);

public:
	virtual bool AllPlayersAreSleeping() { return false; } // 4J Added

	virtual void validateSpawn();
	int getTopTile(int x, int z);

public:
	virtual int getTile(int x, int y, int z);
	virtual int getTileLightBlock(int x, int y, int z);
	bool isEmptyTile(int x, int y, int z);
	virtual bool isEntityTile(int x, int y, int z);
	int getTileRenderShape(int x, int y, int z);
	int getTileRenderShape(int t); // 4J Added to slightly optimise and avoid getTile call if we already know the tile
	bool hasChunkAt(int x, int y, int z);
	bool hasChunksAt(int x, int y, int z, int r);
	bool hasChunksAt(int x0, int y0, int z0, int x1, int y1, int z1);
	bool reallyHasChunkAt(int x, int y, int z);									// 4J added
	bool reallyHasChunksAt(int x, int y, int z, int r);							// 4J added
	bool reallyHasChunksAt(int x0, int y0, int z0, int x1, int y1, int z1);		// 4J added

public:
	bool hasChunk(int x, int z);
	bool reallyHasChunk(int x, int z );	// 4J added

public:
	LevelChunk *getChunkAt(int x, int z);
	LevelChunk *getChunk(int x, int z);
	virtual bool setTileAndData(int x, int y, int z, int tile, int data, int updateFlags);
	Material *getMaterial(int x, int y, int z);
	virtual int getData(int x, int y, int z);
	virtual bool setData(int x, int y, int z, int data, int updateFlags, bool forceUpdate =false);	// 4J added forceUpdate
	virtual bool removeTile(int x, int y, int z);
	virtual bool destroyTile(int x, int y, int z, bool dropResources);
	virtual bool setTileAndUpdate(int x, int y, int z, int tile);
	virtual void sendTileUpdated(int x, int y, int z);

public:
	virtual void tileUpdated(int x, int y, int z, int tile);
	void lightColumnChanged(int x, int z, int y0, int y1);
	void setTileDirty(int x, int y, int z);
	void setTilesDirty(int x0, int y0, int z0, int x1, int y1, int z1);
	void updateNeighborsAt(int x, int y, int z, int tile);
	void updateNeighborsAtExceptFromFacing(int x, int y, int z, int tile, int skipFacing);
	void neighborChanged(int x, int y, int z, int type);
	virtual bool isTileToBeTickedAt(int x, int y, int z, int tileId);
	bool canSeeSky(int x, int y, int z);
	int getDaytimeRawBrightness(int x, int y, int z);
	int getRawBrightness(int x, int y, int z);
	int getRawBrightness(int x, int y, int z, bool propagate);
	bool isSkyLit(int x, int y, int z);
	int getHeightmap(int x, int z);
	int getLowestHeightmap(int x, int z);
	void updateLightIfOtherThan(LightLayer::variety layer, int x, int y, int z, int expected);
	int getBrightnessPropagate(LightLayer::variety layer, int x, int y, int z, int tileId);	// 4J added tileId
	void getNeighbourBrightnesses(int *brightnesses, LightLayer::variety layer, int x, int y, int z);	// 4J added
	int getBrightness(LightLayer::variety layer, int x, int y, int z);
	void setBrightness(LightLayer::variety layer, int x, int y, int z, int brightness, bool noUpdateOnClient=false);		// 4J added noUpdateOnClient
	void setBrightnessNoUpdateOnClient(LightLayer::variety layer, int x, int y, int z, int brightness);	// 4J added

#ifdef _LARGE_WORLDS
	typedef uint64_t lightCache_t;
#else
	typedef unsigned int lightCache_t;
#endif
	inline void setBrightnessCached(lightCache_t *cache, uint64_t *cacheUse, LightLayer::variety layer, int x, int y, int z, int brightness);
	inline int getBrightnessCached(lightCache_t *cache, LightLayer::variety layer, int x, int y, int z);
	inline int getEmissionCached(lightCache_t *cache, int ct, int x, int y, int z);
	inline int getBlockingCached(lightCache_t *cache, LightLayer::variety layer, int *ct, int x, int y, int z);
	void initCachePartial(lightCache_t *cache, int xc, int yc, int zc);
	void initCacheComplete(lightCache_t *cache, int xc, int yc, int zc);
	void flushCache(lightCache_t *cache, uint64_t cacheUse, LightLayer::variety layer);

	bool cachewritten;
	static const int LIGHTING_SHIFT		= 24;
	static const int BLOCKING_SHIFT		= 20;
	static const int EMISSION_SHIFT		= 16;
#ifdef _LARGE_WORLDS
	static const int64_t LIGHTING_WRITEBACK = 0x80000000LL;
	static const int64_t EMISSION_VALID		= 0x40000000LL;
	static const int64_t BLOCKING_VALID	    = 0x20000000LL;
	static const int64_t LIGHTING_VALID		= 0x10000000LL;
	static const lightCache_t POSITION_MASK		= 0xffffffff0000ffffLL;
#else
	static const int LIGHTING_WRITEBACK = 0x80000000;
	static const int EMISSION_VALID		= 0x40000000;
	static const int BLOCKING_VALID	    = 0x20000000;
	static const int LIGHTING_VALID		= 0x10000000;
	static const lightCache_t POSITION_MASK		= 0x0000ffff;
#endif

	int cacheminx, cachemaxx, cacheminy, cachemaxy, cacheminz, cachemaxz;
	void setTileBrightnessChanged(int x, int y, int z);
	virtual int getLightColor(int x, int y, int z, int emitt, int tileId = -1);		// 4J - brought forward from 1.8.2
	virtual float getBrightness(int x, int y, int z, int emitt);
	virtual float getBrightness(int x, int y, int z);
	bool isDay();
	HitResult *clip(Vec3 *a, Vec3 *b);
	HitResult *clip(Vec3 *a, Vec3 *b, bool liquid);
	HitResult *clip(Vec3 *a, Vec3 *b, bool liquid, bool solidOnly);

	virtual void playEntitySound(shared_ptr<Entity> entity, int iSound, float volume, float pitch);
	virtual void playPlayerSound(shared_ptr<Player> entity, int iSound, float volume, float pitch);
	virtual void playSound(double x, double y, double z, int iSound, float volume, float pitch, float fClipSoundDist=16.0f);

	virtual void playLocalSound(double x, double y, double z, int iSound, float volume, float pitch, bool distanceDelay, float fClipSoundDist=16.0f);

	void playStreamingMusic(const wstring& name, int x, int y, int z);
	void playMusic(double x, double y, double z, const wstring& string, float volume);
	// 4J removed - void addParticle(const wstring& id, double x, double y, double z, double xd, double yd, double zd);
	void addParticle(ePARTICLE_TYPE id, double x, double y, double z, double xd, double yd, double zd); // 4J added
	virtual bool addGlobalEntity(shared_ptr<Entity> e);
	virtual bool addEntity(shared_ptr<Entity> e);

protected:
	virtual void entityAdded(shared_ptr<Entity> e);
	virtual void entityRemoved(shared_ptr<Entity> e);
	virtual void playerRemoved(shared_ptr<Entity> e);	// 4J added

public:
	virtual void removeEntity(shared_ptr<Entity> e);
	void removeEntityImmediately(shared_ptr<Entity> e);
	void addListener(LevelListener *listener);
	void removeListener(LevelListener *listener);

private:
	AABBList boxes;

public:
	AABBList *getCubes(shared_ptr<Entity> source, AABB *box, bool noEntities = false, bool blockAtEdge = false); // 4J: Added noEntities & blockAtEdge parameters
	AABBList *getTileCubes(AABB *box, bool blockAtEdge = false); // 4J: Added noEntities & blockAtEdge parameters
	int getOldSkyDarken(float a);		// 4J - change brought forward from 1.8.2
	float getSkyDarken(float a);		// 4J - change brought forward from 1.8.2
	Vec3 *getSkyColor(shared_ptr<Entity> source, float a);
	float getTimeOfDay(float a);
	int getMoonPhase();
	float getMoonBrightness();
	float getSunAngle(float a);
	Vec3 *getCloudColor(float a);
	Vec3 *getFogColor(float a);
	int getTopRainBlock(int x, int z);
	int getTopSolidBlock(int x, int z);
	bool biomeHasRain(int x, int z);	// 4J added
	bool biomeHasSnow(int x, int z);	// 4J added
	int getLightDepth(int x, int z);
	float getStarBrightness(float a);
	virtual void addToTickNextTick(int x, int y, int z, int tileId, int tickDelay);
	virtual void addToTickNextTick(int x, int y, int z, int tileId, int tickDelay, int priorityTilt);
	virtual void forceAddTileTick(int x, int y, int z, int tileId, int tickDelay, int prioTilt);
	virtual void tickEntities();
	void addAllPendingTileEntities(vector< shared_ptr<TileEntity> >& entities);
	void tick(shared_ptr<Entity> e);
	virtual void tick(shared_ptr<Entity> e, bool actual);
	bool isUnobstructed(AABB *aabb);
	bool isUnobstructed(AABB *aabb, shared_ptr<Entity> ignore);
	bool containsAnyBlocks(AABB *box);
	bool containsAnyLiquid(AABB *box);
	bool containsAnyLiquid_NoLoad(AABB *box);	// 4J added
	bool containsFireTile(AABB *box);
	bool checkAndHandleWater(AABB *box, Material *material, shared_ptr<Entity> e);
	bool containsMaterial(AABB *box, Material *material);
	bool containsLiquid(AABB *box, Material *material);
	// 4J Stu - destroyBlocks param brought forward as part of fix for tnt cannons
	shared_ptr<Explosion> explode(shared_ptr<Entity> source, double x, double y, double z, float r, bool destroyBlocks);
	virtual shared_ptr<Explosion> explode(shared_ptr<Entity> source, double x, double y, double z, float r, bool fire, bool destroyBlocks);
	float getSeenPercent(Vec3 *center, AABB *bb);
	bool extinguishFire(shared_ptr<Player> player, int x, int y, int z, int face);
	wstring gatherStats();
	wstring gatherChunkSourceStats();
	virtual shared_ptr<TileEntity> getTileEntity(int x, int y, int z);
	void setTileEntity(int x, int y, int z, shared_ptr<TileEntity> tileEntity);
	void removeTileEntity(int x, int y, int z);
	void markForRemoval(shared_ptr<TileEntity> entity);
	virtual bool isSolidRenderTile(int x, int y, int z);
	virtual bool isSolidBlockingTile(int x, int y, int z);
	bool isSolidBlockingTileInLoadedChunk(int x, int y, int z, bool valueIfNotLoaded);
	bool isFullAABBTile(int x, int y, int z);
	virtual bool isTopSolidBlocking(int x, int y, int z);	// 4J - brought forward from 1.3.2
	bool isTopSolidBlocking(Tile *tile, int data);

protected:
	bool spawnEnemies;
	bool spawnFriendlies;

public:
	// int xxo, yyo, zzo;

	void updateSkyBrightness();
	void setSpawnSettings(bool spawnEnemies, bool spawnFriendlies);
	virtual void tick();

private:
	void prepareWeather();

protected:
	virtual void tickWeather();

private:
	void stopWeather();

public:
	void toggleDownfall();

protected:
#ifdef __PSVITA__
	// AP - See CustomSet.h for an explanation of this
	CustomSet chunksToPoll;
#else
	unordered_set<ChunkPos,ChunkPosKeyHash,ChunkPosKeyEq> chunksToPoll;
#endif

private:
	int delayUntilNextMoodSound;
	static const int CHUNK_POLL_RANGE = 9;
	static const int CHUNK_TILE_TICK_COUNT = 80;
	static const int CHUNK_SECTION_TILE_TICK_COUNT = (CHUNK_TILE_TICK_COUNT / 8) + 1;

protected:
	virtual void buildAndPrepareChunksToPoll();
	virtual void tickClientSideTiles(int xo, int zo, LevelChunk *lc);
	virtual void tickTiles();

	// 4J - snow & ice checks brought forward from 1.2.3
public:
	bool shouldFreezeIgnoreNeighbors(int x, int y, int z);
	bool shouldFreeze(int x, int y, int z);
	bool shouldFreeze(int x, int y, int z, bool checkNeighbors);
	bool shouldSnow(int x, int y, int z);
	void checkLight(int x, int y, int z, bool force = false, bool rootOnlyEmissive = false);		// 4J added force, rootOnlySource parameters
private:
	int *toCheckLevel;
	int getExpectedLight(lightCache_t *cache, int x, int y, int z, LightLayer::variety layer, bool propagatedOnly);
public:
	void checkLight(LightLayer::variety layer, int xc, int yc, int zc, bool force = false, bool rootOnlyEmissive = false);		// 4J added force, rootOnlySource parameters

public:
	virtual bool tickPendingTicks(bool force);
	virtual vector<TickNextTickData> *fetchTicksInChunk(LevelChunk *chunk, bool remove);

private:
	vector<shared_ptr<Entity> > es;

public:
	bool isClientSide;

	vector<shared_ptr<Entity> > *getEntities(shared_ptr<Entity> except, AABB *bb);
	vector<shared_ptr<Entity> > *getEntities(shared_ptr<Entity> except, AABB *bb, const EntitySelector *selector);
	vector<shared_ptr<Entity> > *getEntitiesOfClass(const type_info& baseClass, AABB *bb);
	vector<shared_ptr<Entity> > *getEntitiesOfClass(const type_info& baseClass, AABB *bb, const EntitySelector *selector);
	shared_ptr<Entity> getClosestEntityOfClass(const type_info& baseClass, AABB *bb, shared_ptr<Entity> source);
	virtual shared_ptr<Entity> getEntity(int entityId) = 0;
	vector<shared_ptr<Entity> > getAllEntities();
	void tileEntityChanged(int x, int y, int z, shared_ptr<TileEntity> te);
	//	unsigned int countInstanceOf(BaseObject::Class *clas);
	unsigned int countInstanceOf(eINSTANCEOF clas, bool singleType, unsigned int *protectedCount = nullptr, unsigned int *couldWanderCount = nullptr);			// 4J added
	unsigned int countInstanceOfInRange(eINSTANCEOF clas, bool singleType, int range, int x, int y, int z);													// 4J Added
	void addEntities(vector<shared_ptr<Entity> > *list);
	virtual void removeEntities(vector<shared_ptr<Entity> > *list);
	bool mayPlace(int tileId, int x, int y, int z, bool ignoreEntities, int face, shared_ptr<Entity> ignoreEntity, shared_ptr<ItemInstance> item);
	int getSeaLevel();
	Path *findPath(shared_ptr<Entity> from, shared_ptr<Entity> to, float maxDist, bool canPassDoors, bool canOpenDoors, bool avoidWater, bool canFloat);
	Path *findPath(shared_ptr<Entity> from, int xBest, int yBest, int zBest, float maxDist, bool canPassDoors, bool canOpenDoors, bool avoidWater, bool canFloat);
	int getDirectSignal(int x, int y, int z, int dir);
	int getDirectSignalTo(int x, int y, int z);
	bool hasSignal(int x, int y, int z, int dir);
	int getSignal(int x, int y, int z, int dir);
	bool hasNeighborSignal(int x, int y, int z);
	int getBestNeighborSignal(int x, int y, int z);
	// 4J Added maxYDist param
	shared_ptr<Player> getNearestPlayer(shared_ptr<Entity> source, double maxDist, double maxYDist = -1);
	shared_ptr<Player> getNearestPlayer(double x, double y, double z, double maxDist, double maxYDist = -1);
	shared_ptr<Player> getNearestPlayer(double x, double z, double maxDist);
	shared_ptr<Player> getNearestAttackablePlayer(shared_ptr<Entity> source, double maxDist);
	shared_ptr<Player> getNearestAttackablePlayer(double x, double y, double z, double maxDist);

	shared_ptr<Player> getPlayerByName(const wstring& name);
	shared_ptr<Player> getPlayerByUUID(const wstring& name); // 4J Added
	byteArray getBlocksAndData(int x, int y, int z, int xs, int ys, int zs, bool includeLighting = true);
	void setBlocksAndData(int x, int y, int z, int xs, int ys, int zs, byteArray data, bool includeLighting = true);
	virtual void disconnect(bool sendDisconnect = true);
	void checkSession();
	void setGameTime(int64_t time);
	int64_t getSeed();
	int64_t getGameTime();
	int64_t getDayTime();
	void setDayTime(int64_t newTime);
	Pos *getSharedSpawnPos();
	void setSpawnPos(int x, int y, int z);
	void setSpawnPos(Pos *spawnPos);
	void ensureAdded(shared_ptr<Entity> entity);
	virtual bool mayInteract(shared_ptr<Player> player, int xt, int yt, int zt, int content);
	virtual void broadcastEntityEvent(shared_ptr<Entity> e, byte event);
	ChunkSource *getChunkSource();
	virtual void tileEvent(int x, int y, int z, int tile, int b0, int b1);
	LevelStorage *getLevelStorage();
	LevelData *getLevelData();
	GameRules *getGameRules();
	virtual void updateSleepingPlayerList();
	bool useNewSeaLevel();	// 4J added
	bool getHasBeenInCreative(); // 4J Added
	bool isGenerateMapFeatures(); // 4J Added
	int getSaveVersion();
	int getOriginalSaveVersion();
	float getThunderLevel(float a);
	float getRainLevel(float a);
	void setRainLevel(float rainLevel);
	bool isThundering();
	bool isRaining();
	bool isRainingAt(int x, int y, int z);
	bool isHumidAt(int x, int y, int z);
	void setSavedData(const wstring& id, shared_ptr<SavedData> data);
	shared_ptr<SavedData> getSavedData(const type_info& clazz, const wstring& id);
	int getFreeAuxValueFor(const wstring& id);
	void globalLevelEvent(int type, int sourceX, int sourceY, int sourceZ, int data);
	void levelEvent(int type, int x, int y, int z, int data);
	void levelEvent(shared_ptr<Player> source, int type, int x, int y, int z, int data);
	int getMaxBuildHeight();
	int getHeight();
	virtual Tickable *makeSoundUpdater(shared_ptr<Minecart> minecart);
	Random *getRandomFor(int x, int z, int blend);
	virtual bool isAllEmpty();
	double getHorizonHeight() ;
	void destroyTileProgress(int id, int x, int y, int z, int progress);
	//  Calendar *getCalendar(); // 4J - Calendar is now static
	virtual void createFireworks(double x, double y, double z, double xd, double yd, double zd, CompoundTag *infoTag);
	virtual Scoreboard *getScoreboard();
	virtual void updateNeighbourForOutputSignal(int x, int y, int z, int source);
	virtual float getDifficulty(double x, double y, double z);
	virtual float getDifficulty(int x, int y, int z);
	TilePos *findNearestMapFeature(const wstring& featureName, int x, int y, int z);

	// 4J Added
	int getAuxValueForMap(PlayerUID xuid, int dimension, int centreXC, int centreZC, int scale);

	// 4J - optimisation - keep direct reference of underlying cache here
	LevelChunk					**chunkSourceCache;
	int							chunkSourceXZSize;

	// 4J - added for implementation of finite limit to number of item entities, tnt and falling block entities
public:
	virtual bool newPrimedTntAllowed() { return true; }
	virtual bool newFallingTileAllowed() { return true; }

	// 4J - added for new lighting from 1.8.2
	CRITICAL_SECTION			m_checkLightCS;

private:
	int m_iHighestY;		// 4J-PB - for the end portal in The End
public:
	int GetHighestY()			{ return m_iHighestY;}
	void SetHighestY(int iVal)	{ m_iHighestY=iVal;}

	bool isChunkFinalised(int x, int z);		// 4J added
	bool isChunkPostPostProcessed(int x, int z);	// 4J added

private:
	int m_unsavedChunkCount;

public:
	int getUnsavedChunkCount();
	void incrementUnsavedChunkCount(); // 4J Added
	void decrementUnsavedChunkCount(); // 4J Added

	enum ESPAWN_TYPE
	{
		eSpawnType_Egg,
		eSpawnType_Breed,
		eSpawnType_Portal,
	};

	bool canCreateMore(eINSTANCEOF type, ESPAWN_TYPE spawnType);
};
