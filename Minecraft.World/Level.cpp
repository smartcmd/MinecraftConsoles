#include "stdafx.h"
#include "System.h"
#include "BasicTypeContainers.h"
#include "File.h"
#include "ProgressListener.h"
#include "net.minecraft.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.global.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.chunk.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.levelgen.h"
#include "net.minecraft.world.level.storage.h"
#include "net.minecraft.world.level.pathfinder.h"
#include "net.minecraft.world.phys.h"
#include "ChunkPos.h"
#include "Explosion.h"
#include "LevelListener.h"
#include "LightLayer.h"
#include "MobSpawner.h"
#include "Region.h"
#include "TickNextTickData.h"
#include "Level.h"
#include "ThreadName.h"
#include "WeighedRandom.h"

#include "ConsoleSaveFile.h"
#include <xuiapp.h>
#include "..\Minecraft.Client\Minecraft.h"
#include "..\Minecraft.Client\LevelRenderer.h"
#include "SoundTypes.h"
#include "SparseLightStorage.h"
#include "..\Minecraft.Client\Textures.h"
#include "..\Minecraft.Client\TexturePackRepository.h"
#include "..\Minecraft.Client\DLCTexturePack.h"
#include "..\Minecraft.Client\Common\DLC\DLCPack.h"
#include "..\Minecraft.Client\PS3\PS3Extras\ShutdownManager.h"


DWORD Level::tlsIdx = TlsAlloc();
DWORD Level::tlsIdxLightCache = TlsAlloc();

// 4J : WESTY : Added for time played stats.
#include "net.minecraft.stats.h"

// 4J - Caching of lighting data added. This is implemented as a 16x16x16 cache of ints (ie 16K storage in total). The index of the element to be used in the array is determined by the lower
// four bits of each x/y/z position, and the upper 7/4/7 bits of the x/y/z positions are stored within the element itself along with the cached values etc. The cache can be enabled per thread by
// calling enableLightingCache, otherwise standard non-cached accesses are performed. General method for using caching if enabled on a thread is:
// (1) Call initCache, this invalidates any previous data in the cache
// (2) Use setBrightnessCached, getBrightnessCached, getEmissionCached, getBlockingCached methods to get and set data
// (3) Call flushCache, which writes through any dirty values in cache

#ifdef _LARGE_WORLDS
// Packing for cache entries in large worlds is as follows ( 64 bits per entry)
// Add the extra x and z data into the top 32 bits, to keep all the masks and code for everything else the same
// xxxxxxxxxxxxxxxxzzzzzzzzzzzzzzzzWEBLllllbbbbeeeexxxxxxyyyyzzzzzz
//
// xxxxxx  - middle 6 bits of x position
// yyyy	   - top 4 bits of y position
// zzzzzz  - middle 6 bits of z position
// eeee    - light emission
// bbbb	   - light blocking
// llll	   - light level
// L	   - light value valid
// B       - blocking value valid
// E	   - emission value valid
// W       - lighting value requires write
// xxxxxxxxxxxxxxxx - top 16 bits of x position
// zzzzzzzzzzzzzzzz - top 16 bits of z position
#else
// Packing for cache entries is as follows ( 32 bits per entry)
// WEBLllllbbbbeeeexxxxxxyyyyzzzzzz
//
// xxxxxx  - top 6 bits of x position
// yyyy	   - top 4 bits of y position
// zzzzzz  - top 6 bits of z position
// eeee    - light emission
// bbbb	   - light blocking
// llll	   - light level
// L	   - light value valid
// B       - blocking value valid
// E	   - emission value valid
// W       - lighting value requires write
#endif


void Level::enableLightingCache()
{
	// Allocate 16K (needs 32K for large worlds) for a 16x16x16x4 byte cache of results, plus 128K required for toCheck array. Rounding up to 256 to keep as multiple of alignement - aligning to 128K boundary for possible cache locking.
	void *cache = (unsigned char *)XPhysicalAlloc(256 * 1024, MAXULONG_PTR, 128 * 1024, PAGE_READWRITE | MEM_LARGE_PAGES);
	TlsSetValue(tlsIdxLightCache,cache);
}

void Level::destroyLightingCache()
{
	lightCache_t *cache = (lightCache_t *)TlsGetValue(tlsIdxLightCache);
	XPhysicalFree(cache);
}

void Level::initCache(lightCache_t *cache)
{
	cachewritten = false;
	if( cache == NULL ) return;

	XMemSet128(cache,0,16*16*16*sizeof(lightCache_t));
}

// Set a brightness value, going through the cache if  enabled for this thread
void inline Level::setBrightnessCached(lightCache_t *cache, __uint64 *cacheUse, LightLayer::variety layer, int x, int y, int z, int brightness)
{
	if( cache == NULL )
	{
		setBrightness(layer, x, y, z, brightness, true);
		return;
	}
	if( y & 0xffffff00 ) return;		// Eliminate -ve ys and values > 255

	int idx = ( ( x & 15 ) << 8 ) |
			  ( ( y & 15 ) << 4 ) |
			  ( z & 15 );
	lightCache_t posbits = ( ( x & 0x3f0 ) << 6 ) |
				  ( ( y & 0x0f0 ) << 2 ) |
				  ( ( z & 0x3f0 ) >> 4 );
#ifdef _LARGE_WORLDS
	// Add in the higher bits for x and z
	posbits |=  ( ( ((__uint64)x) & 0x3FFFC00L) << 38) |
				( ( ((__uint64)z) & 0x3FFFC00L) << 22);
#endif

	lightCache_t cacheValue = cache[idx];

	// If this cache entry doesn't refer to the same thing...
	if( ( cacheValue & POSITION_MASK ) != posbits )
	{
		/// and it has been written to...
		if( cacheValue & LIGHTING_WRITEBACK )
		{
			// Then we need to flush
			int val = ( cacheValue >> LIGHTING_SHIFT ) & 15;
			int xx = ( (cacheValue >> 6 ) & 0x3f0 ) | ( x & 15 );
#ifdef _LARGE_WORLDS
			xx |= ( (cacheValue >> 38) & 0x3FFFC00);
			xx = ( xx << 6 ) >> 6;	// sign extend
#else
			xx = ( xx << 22 ) >> 22;	// sign extend
#endif
			int yy = ( (cacheValue >> 2 ) & 0x0f0 ) | ( y & 15 );
			int zz = ( (cacheValue << 4 ) & 0x3f0 ) | ( z & 15 );
#ifdef _LARGE_WORLDS
			zz |= ( (cacheValue >> 22) & 0x3FFFC00);
			zz = ( zz << 6 ) >> 6;	// sign extend
#else
			zz = ( zz << 22 ) >> 22;	// sign extend
#endif
			setBrightness(layer, xx, yy, zz, val, true);
		}
		cacheValue = posbits;
	}

	// Just written to it, so value is valid & requires writing back
	cacheValue &= ~(15 << LIGHTING_SHIFT );
	cacheValue |= brightness << LIGHTING_SHIFT;
	cacheValue |= ( LIGHTING_WRITEBACK | LIGHTING_VALID );

	// cacheUse has a single bit for each x, y and z to say whether anything with that x, y or z has been written to
	(*cacheUse) |= ( ( 1LL << ( x & 15 ) ) | ( 0x10000LL << ( y & 15 ) ) | ( 0x100000000LL << ( z & 15 ) ) );

	cache[idx] = cacheValue;
}

// Get a brightness value, going through the cache if  enabled for this thread
inline int Level::getBrightnessCached(lightCache_t *cache, LightLayer::variety layer, int x, int y, int z)
{
	if( cache == NULL ) return getBrightness(layer, x, y, z);
	if( y & 0xffffff00 ) return getBrightness(layer, x, y, z);		// Fall back on original method for out-of-bounds y

	int idx = ( ( x & 15 ) << 8 ) |
			  ( ( y & 15 ) << 4 ) |
			  ( z & 15 );
	lightCache_t posbits = ( ( x & 0x3f0 ) << 6 ) |
				  ( ( y & 0x0f0 ) << 2 ) |
				  ( ( z & 0x3f0 ) >> 4 );
#ifdef _LARGE_WORLDS
	// Add in the higher bits for x and z
	posbits |=  ( ( ((__uint64)x) & 0x3FFFC00L) << 38) |
				( ( ((__uint64)z) & 0x3FFFC00L) << 22);
#endif

	lightCache_t cacheValue = cache[idx];

	if( ( cacheValue & POSITION_MASK ) != posbits )
	{
		// Position differs - need to evict this cache entry
		if( cacheValue & LIGHTING_WRITEBACK )
		{
			// Then we need to flush
			int val = ( cacheValue >> LIGHTING_SHIFT ) & 15;
			int xx = ( (cacheValue >> 6 ) & 0x3f0 ) | ( x & 15 );
#ifdef _LARGE_WORLDS
			xx |= ( (cacheValue >> 38) & 0x3FFFC00);
			xx = ( xx << 6 ) >> 6;	// sign extend
#else
			xx = ( xx << 22 ) >> 22;	// sign extend
#endif
			int yy = ( (cacheValue >> 2 ) & 0x0f0 ) | ( y & 15 );
			int zz = ( (cacheValue << 4 ) & 0x3f0 ) | ( z & 15 );
#ifdef _LARGE_WORLDS
			zz |= ( (cacheValue >> 22) & 0x3FFFC00);
			zz = ( zz << 6 ) >> 6;	// sign extend
#else
			zz = ( zz << 22 ) >> 22;	// sign extend
#endif
			setBrightness(layer, xx, yy, zz, val, true);
		}
		cacheValue = posbits | LIGHTING_VALID;
		int val = getBrightness(layer, x, y, z);
		cacheValue |= val << LIGHTING_SHIFT;
	}
	else
	{
		// The position matches - will incurr a read miss if the lighting value isn't valid
		if( ( cacheValue & LIGHTING_VALID ) == 0 )
		{
			int val = getBrightness(layer, x, y, z);
			cacheValue |= val << LIGHTING_SHIFT;
			cacheValue |= LIGHTING_VALID;
		}
		else
		{
			// All valid - just return value
			return ( cacheValue >> LIGHTING_SHIFT ) & 15;
		}
	}

	cache[idx] = cacheValue;
	return ( cacheValue >> LIGHTING_SHIFT ) & 15;
}

// Get a block emission value, going through the cache if  enabled for this thread
inline int Level::getEmissionCached(lightCache_t *cache, int ct, int x, int y, int z)
{
	if( cache == NULL )	return Tile::lightEmission[ct];

	int idx = ( ( x & 15 ) << 8 ) |
			  ( ( y & 15 ) << 4 ) |
			  ( z & 15 );
	lightCache_t posbits = ( ( x & 0x3f0 ) << 6 ) |
				  ( ( y & 0x0f0 ) << 2 ) |
				  ( ( z & 0x3f0 ) >> 4 );
#ifdef _LARGE_WORLDS
	// Add in the higher bits for x and z
	posbits |=  ( ( ((__uint64)x) & 0x3FFFC00) << 38) |
				( ( ((__uint64)z) & 0x3FFFC00) << 22);
#endif

	lightCache_t cacheValue = cache[idx];

	if( ( cacheValue & POSITION_MASK ) != posbits )
	{
		// Position differs - need to evict this cache entry
		if( cacheValue & LIGHTING_WRITEBACK )
		{
			// Then we need to flush
			int val = ( cacheValue >> LIGHTING_SHIFT ) & 15;
			int xx = ( (cacheValue >> 6 ) & 0x3f0 ) | ( x & 15 );
#ifdef _LARGE_WORLDS
			xx |= ( (cacheValue >> 38) & 0x3FFFC00);
			xx = ( xx << 6 ) >> 6;	// sign extend
#else
			xx = ( xx << 22 ) >> 22;	// sign extend
#endif
			int yy = ( (cacheValue >> 2 ) & 0x0f0 ) | ( y & 15 );
			int zz = ( (cacheValue << 4 ) & 0x3f0 ) | ( z & 15 );
#ifdef _LARGE_WORLDS
			zz |= ( (cacheValue >> 22) & 0x3FFFC00);
			zz = ( zz << 6 ) >> 6;	// sign extend
#else
			zz = ( zz << 22 ) >> 22;	// sign extend
#endif
			setBrightness(LightLayer::Block, xx, yy, zz, val, true);
		}
		
		// Update both emission & blocking values whilst we are here
		cacheValue = posbits | EMISSION_VALID | BLOCKING_VALID;
		int t = getTile(x,y,z);
		cacheValue |= ( Tile::lightEmission[t] & 15 )  << EMISSION_SHIFT;
		cacheValue |= ( Tile::lightBlock[t] & 15 ) << BLOCKING_SHIFT;
	}
	else
	{
		// The position matches - will incurr a read miss if the lighting value isn't valid
		if( ( cacheValue & EMISSION_VALID ) == 0 )
		{
			// Update both emission & blocking values whilst we are here
			cacheValue |= EMISSION_VALID | BLOCKING_VALID;
			int t = getTile(x,y,z);
			cacheValue |= ( Tile::lightEmission[t] & 15 )  << EMISSION_SHIFT;
			cacheValue |= ( Tile::lightBlock[t] & 15 ) << BLOCKING_SHIFT;
		}
		else
		{
			// All valid - just return value
			return ( cacheValue >> EMISSION_SHIFT ) & 15;
		}
	}
	cache[idx] = cacheValue;
	return ( cacheValue >> EMISSION_SHIFT ) & 15;
}

// Get a tile light blocking value, going through cache if enabled for this thread
inline int Level::getBlockingCached(lightCache_t *cache, LightLayer::variety layer, int *ct, int x, int y, int z)
{
	if( cache == NULL )
	{
		int t = getTile(x,y,z);
		if(ct) 	*ct = t;
		return Tile::lightBlock[t];
	}

	int idx = ( ( x & 15 ) << 8 ) |
			  ( ( y & 15 ) << 4 ) |
			  ( z & 15 );
	lightCache_t posbits = ( ( x & 0x3f0 ) << 6 ) |
				  ( ( y & 0x0f0 ) << 2 ) |
				  ( ( z & 0x3f0 ) >> 4 );
#ifdef _LARGE_WORLDS
	// Add in the higher bits for x and z
	posbits |=  ( ( ((__uint64)x) & 0x3FFFC00L) << 38) |
				( ( ((__uint64)z) & 0x3FFFC00L) << 22);
#endif

	lightCache_t cacheValue = cache[idx];

	if( ( cacheValue & POSITION_MASK ) != posbits )
	{
		// Position differs - need to evict this cache entry
		if( cacheValue & LIGHTING_WRITEBACK )
		{
			// Then we need to flush
			int val = ( cacheValue >> LIGHTING_SHIFT ) & 15;
			int xx = ( (cacheValue >> 6 ) & 0x3f0 ) | ( x & 15 );
#ifdef _LARGE_WORLDS
			xx |= ( (cacheValue >> 38) & 0x3FFFC00);
			xx = ( xx << 6 ) >> 6;	// sign extend
#else
			xx = ( xx << 22 ) >> 22;	// sign extend
#endif
			int yy = ( (cacheValue >> 2 ) & 0x0f0 ) | ( y & 15 );
			int zz = ( (cacheValue << 4 ) & 0x3f0 ) | ( z & 15 );
#ifdef _LARGE_WORLDS
			zz |= ( (cacheValue >> 22) & 0x3FFFC00);
			zz = ( zz << 6 ) >> 6;	// sign extend
#else
			zz = ( zz << 22 ) >> 22;	// sign extend
#endif
			setBrightness(layer, xx, yy, zz, val, true);
		}
		
		// Update both emission & blocking values whilst we are here
		cacheValue = posbits | EMISSION_VALID | BLOCKING_VALID;
		int t = getTile(x,y,z);
		cacheValue |= ( Tile::lightEmission[t] & 15 )  << EMISSION_SHIFT;
		cacheValue |= ( Tile::lightBlock[t] & 15 ) << BLOCKING_SHIFT;
	}
	else
	{
		// The position matches - will incurr a read miss if the lighting value isn't valid
		if( ( cacheValue & EMISSION_VALID ) == 0 )
		{
			// Update both emission & blocking values whilst we are here
			cacheValue |= EMISSION_VALID | BLOCKING_VALID;
			int t = getTile(x,y,z);
			cacheValue |= ( Tile::lightEmission[t] & 15 )  << EMISSION_SHIFT;
			cacheValue |= ( Tile::lightBlock[t] & 15 ) << BLOCKING_SHIFT;
		}
		else
		{
			// All valid - just return value
			return ( cacheValue >> BLOCKING_SHIFT ) & 15;
		}
	}

	cache[idx] = cacheValue;
	return ( cacheValue >> BLOCKING_SHIFT ) & 15;
}

// Write back any dirty entries in the lighting cache. Also calls the setTilesDirty method on the region which has been updated during this lighting update, since
// this hasn't been updated (for client threads) for each individual lighting update as would have been the case with the non-cached lighting. There's two reasons for this
// (1) it's more efficient, since we aren't doing so many individual calls to the level listener to let the renderer know what has been updated
// (2) it lets the lighting actually complete before we get any visual representation of the update, otherwise we end up seeing some strange partial updates
void Level::flushCache(lightCache_t *cache, __uint64 cacheUse, LightLayer::variety layer)
{
	// cacheUse has a single bit for each x, y and z to say whether anything with that x, y or z has been written to
	if( cacheUse == 0 ) return;
	if( cache )
	{
		lightCache_t *pcache = cache;
		for( int x = 0; x < 16; x++ )
		{
			if( ( cacheUse & ( 1LL << x ) ) == 0 )
			{
				pcache += 16 * 16;
				continue;
			}
			for( int y = 0; y < 16; y++ )
			{
				if( ( cacheUse & ( 0x10000LL << y ) ) == 0 )
				{
					pcache += 16;
					continue;
				}
				for( int z = 0; z < 16; z++ )
				{
					if( ( cacheUse & ( 0x100000000LL << z ) ) == 0 )
					{
						pcache++;
						continue;
					}
					lightCache_t cacheValue = *pcache++;
					if( cacheValue & LIGHTING_WRITEBACK )
					{
						int val = ( cacheValue >> LIGHTING_SHIFT ) & 15;
						int xx = ( (cacheValue >> 6 ) & 0x3f0 ) | ( x & 15 );
#ifdef _LARGE_WORLDS
						xx |= ( (cacheValue >> 38) & 0x3FFFC00);
						xx = ( xx << 6 ) >> 6;	// sign extend
#else
						xx = ( xx << 22 ) >> 22;	// sign extend
#endif
						int yy = ( (cacheValue >> 2 ) & 0x0f0 ) | ( y & 15 );
						int zz = ( (cacheValue << 4 ) & 0x3f0 ) | ( z & 15 );
#ifdef _LARGE_WORLDS
						zz |= ( (cacheValue >> 22) & 0x3FFFC00);
						zz = ( zz << 6 ) >> 6;	// sign extend
#else
						zz = ( zz << 22 ) >> 22;	// sign extend
#endif
						setBrightness(layer, xx, yy, zz, val, true);
					}
				}
			}
		}
	}
	// For client side (which has the renderer attached) we haven't been updating with each individual update, but have been gathering them up.
	// Let the renderer know now the region that has been updated.
	if( isClientSide && cachewritten)
	{
		setTilesDirty(cacheminx, cacheminy, cacheminz,cachemaxx,cachemaxy,cachemaxz);
	}
}

// 4J - added following 2 functions to move instaBuild flag from being a class member, to TLS
bool Level::getInstaTick()
{
	return ((size_t)TlsGetValue(tlsIdx)) != 0;
}

void Level::setInstaTick(bool enable)
{
	void *value = 0;
	if( enable ) value = (void *)1;
	TlsSetValue(tlsIdx,value);
}

// 4J - added
bool Level::hasEntitiesToRemove()
{
	return !entitiesToRemove.empty();
}

void Level::_init()
{
	cloudColor = 0xffffff;

	skyDarken = 0;

	randValue = (new Random())->nextInt();

	addend = 1013904223;

	oRainLevel = rainLevel = 0.0f;

	oThunderLevel = thunderLevel = 0.0f;

	lightningTime = 0;

	lightningBoltTime = 0;

	noNeighborUpdate = false;

	difficulty = 0;

	random = new Random();
	isNew = false;

	dimension = NULL;

	chunkSource = NULL;

	levelStorage = nullptr;

	levelData = NULL;

	isFindingSpawn = false;

	savedDataStorage = NULL;

	spawnEnemies = true;

	spawnFriendlies = true;

	delayUntilNextMoodSound = random->nextInt(20 * 60 * 10);

	isClientSide = false;

	InitializeCriticalSection(&m_entitiesCS);
	InitializeCriticalSection(&m_tileEntityListCS);

	m_timeOfDayOverride = -1;

	updatingTileEntities = false;

	villageSiege = new VillageSiege(this);

	toCheckLevel = new int[ 32 * 32 * 32];	// 4J - brought forward from 1.8.2
	InitializeCriticalSectionAndSpinCount(&m_checkLightCS, 5120);	// 4J - added for 1.8.2 lighting

	// 4J Added
	m_bDisableAddNewTileEntities = false;
	m_iHighestY=-1000;
	m_unsavedChunkCount = 0;
}

// 4J - brought forward from 1.8.2
Biome *Level::getBiome(int x, int z)
{
    if (hasChunkAt(x, 0, z))
	{
        LevelChunk *lc = getChunkAt(x, z);
        if (lc != NULL)
		{
			// Water chunks at the edge of the world return NULL for their biome as they can't store it, so should fall back on the normal method below
			Biome *biome = lc->getBiome(x & 0xf, z & 0xf, dimension->biomeSource);
	        if( biome ) return biome;
        }
    }
    return dimension->biomeSource->getBiome(x, z);
}

BiomeSource *Level::getBiomeSource()
{
	return dimension->biomeSource;
}

Level::Level(shared_ptr<LevelStorage> levelStorage, const wstring& name, Dimension *dimension, LevelSettings *levelSettings, bool doCreateChunkSource)
	: seaLevel(constSeaLevel)
{
	_init();
	this->levelStorage = levelStorage;//shared_ptr<LevelStorage>(levelStorage);
	this->dimension = dimension;
	this->levelData = new LevelData(levelSettings, name);
	if( !this->levelData->useNewSeaLevel() ) seaLevel = Level::genDepth / 2;		// 4J added - sea level is one unit lower since 1.8.2, maintain older height for old levels
	this->savedDataStorage = new SavedDataStorage(levelStorage.get());

	shared_ptr<Villages> savedVillages = dynamic_pointer_cast<Villages>(savedDataStorage->get(typeid(Villages), Villages::VILLAGE_FILE_ID));
	if (savedVillages == NULL)
	{
		villages = shared_ptr<Villages>(new Villages(this));
		savedDataStorage->set(Villages::VILLAGE_FILE_ID, villages);
	}
	else
	{
		villages = savedVillages;
		villages->setLevel(this);
	}

	dimension->init(this);
	chunkSource = NULL;	// 4J - added flag so chunk source can be called from derived class instead

	updateSkyBrightness();
	prepareWeather();
}


Level::Level(Level *level, Dimension *dimension)
	:seaLevel( constSeaLevel )
{
	_init();
	this->levelStorage = level->levelStorage;
	this->levelData = new LevelData(level->levelData);
	if( !this->levelData->useNewSeaLevel() ) seaLevel = Level::genDepth / 2;		// 4J added - sea level is one unit lower since 1.8.2, maintain older height for old levels
	this->savedDataStorage = new SavedDataStorage( levelStorage.get() );
	
	shared_ptr<Villages> savedVillages = dynamic_pointer_cast<Villages>(savedDataStorage->get(typeid(Villages), Villages::VILLAGE_FILE_ID));
	if (savedVillages == NULL)
	{
		villages = shared_ptr<Villages>(new Villages(this));
		savedDataStorage->set(Villages::VILLAGE_FILE_ID, villages);
	}
	else
	{
		villages = savedVillages;
		villages->setLevel(this);
	}

	this->dimension = dimension;
	dimension->init(this);
	chunkSource = NULL;
	updateSkyBrightness();
	prepareWeather();
}


Level::Level(shared_ptr<LevelStorage>levelStorage, const wstring& levelName, LevelSettings *levelSettings)
	: seaLevel( constSeaLevel )
{
	_init(levelStorage, levelName, levelSettings, NULL, true);
}


Level::Level(shared_ptr<LevelStorage>levelStorage, const wstring& levelName, LevelSettings *levelSettings, Dimension *fixedDimension, bool doCreateChunkSource)
	: seaLevel( constSeaLevel )
{
	_init( levelStorage, levelName, levelSettings, fixedDimension, doCreateChunkSource );
}

void Level::_init(shared_ptr<LevelStorage>levelStorage, const wstring& levelName, LevelSettings *levelSettings, Dimension *fixedDimension, bool doCreateChunkSource)
{
	_init();
	this->levelStorage = levelStorage;//shared_ptr<LevelStorage>(levelStorage);
	this->savedDataStorage = new SavedDataStorage(levelStorage.get());

	shared_ptr<Villages> savedVillages = dynamic_pointer_cast<Villages>(savedDataStorage->get(typeid(Villages), Villages::VILLAGE_FILE_ID));
	if (savedVillages == NULL)
	{
		villages = shared_ptr<Villages>(new Villages(this));
		savedDataStorage->set(Villages::VILLAGE_FILE_ID, villages);
	}
	else
	{
		villages = savedVillages;
		villages->setLevel(this);
	}

	levelData = levelStorage->prepareLevel();
	isNew = levelData == NULL;

	if (fixedDimension != NULL)
	{
		dimension = fixedDimension;
	}
	// 4J Remove TU9 as getDimensions was never accurate. This path was never used anyway as we always set fixedDimension
	//else if (levelData != NULL && levelData->getDimension() != 0)
	//{
	//	dimension = Dimension::getNew(levelData->getDimension());
	//}
	else 
	{
		dimension = Dimension::getNew(0);
	}

	if (levelData == NULL)
	{
		levelData = new LevelData(levelSettings, levelName);
	}
	else
	{
		levelData->setLevelName(levelName);
	}
	if( !this->levelData->useNewSeaLevel() ) seaLevel = Level::genDepth / 2;		// 4J added - sea level is one unit lower since 1.8.2, maintain older height for old levels

	((Dimension *) dimension)->init( this );
	
	chunkSource = doCreateChunkSource ? createChunkSource() : NULL;	// 4J - added flag so chunk source can be called from derived class instead

	// 4J Stu- Moved to derived classes
	//if (!levelData->isInitialized())
	//{
	//	initializeLevel(levelSettings);
	//	levelData->setInitialized(true);
	//}

	updateSkyBrightness();
	prepareWeather();

}

Level::~Level()
{
	delete random;
	delete dimension;
	delete chunkSource;
	delete levelData;
	delete toCheckLevel;

	if( !isClientSide )
	{
		NotGateTile::removeLevelReferences(this);	// 4J added
	}

	DeleteCriticalSection(&m_checkLightCS);

	// 4J-PB - savedDataStorage is shared between overworld and nether levels in the server, so it will already have been deleted on the first level delete
	if(savedDataStorage!=NULL) delete savedDataStorage;		

	DeleteCriticalSection(&m_entitiesCS);
	DeleteCriticalSection(&m_tileEntityListCS);

	// 4J Stu - At least one of the listeners is something we cannot delete, the LevelRenderer
	/*
	for(int i = 0; i < listeners.size(); i++)
		delete listeners[i];
		*/
}

void Level::initializeLevel(LevelSettings *settings)
{
	levelData->setInitialized(true);
}

void Level::validateSpawn()
{
	setSpawnPos(8, 64, 8);
}

int Level::getTopTile(int x, int z)
{
	// 4J added - was breaking spawning as not finding ground in superflat worlds
	if( levelData->getGenerator() == LevelType::lvl_flat )
	{
		return Tile::grass_Id;
	}

	int y = seaLevel;
	while (!isEmptyTile(x, y + 1, z))
	{
		y++;
	}
	return getTile(x, y, z);
}
int Level::getTile(int x, int y, int z)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z >= MAX_LEVEL_SIZE)
	{
		return 0;
	}
	if (y < minBuildHeight) return 0;
	if (y >= maxBuildHeight) return 0;
	return getChunk(x >> 4, z >> 4)->getTile(x & 15, y, z & 15);
}

int Level::getTileLightBlock(int x, int y, int z)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z >= MAX_LEVEL_SIZE)
	{
		return 0;
	}
	if (y < minBuildHeight) return 0;
	if (y >= maxBuildHeight) return 0;
	return getChunk(x >> 4, z >> 4)->getTileLightBlock(x & 15, y, z & 15);
}

bool Level::isEmptyTile(int x, int y, int z)
{
	return getTile(x, y, z) == 0;
}

bool Level::isEntityTile(int x, int y, int z)
{
	int t = getTile(x, y, z);
	if (Tile::tiles[t] != NULL && Tile::tiles[t]->isEntityTile())
	{
		return true;
	}
	return false;
}

int Level::getTileRenderShape(int x, int y, int z)
{
	int t = getTile(x, y, z);
	if (Tile::tiles[t] != NULL)
	{
		return Tile::tiles[t]->getRenderShape();
	}
	return Tile::SHAPE_INVISIBLE;
}

bool Level::hasChunkAt(int x, int y, int z)
{
	if (y < minBuildHeight || y >= maxBuildHeight) return false;
	return hasChunk(x >> 4, z >> 4);
}

// 4J added
bool Level::reallyHasChunkAt(int x, int y, int z)
{
	if (y < minBuildHeight || y >= maxBuildHeight) return false;
	return reallyHasChunk(x >> 4, z >> 4);
}

bool Level::hasChunksAt(int x, int y, int z, int r)
{
	return hasChunksAt(x - r, y - r, z - r, x + r, y + r, z + r);
}

// 4J added
bool Level::reallyHasChunksAt(int x, int y, int z, int r)
{
	return reallyHasChunksAt(x - r, y - r, z - r, x + r, y + r, z + r);
}


bool Level::hasChunksAt(int x0, int y0, int z0, int x1, int y1, int z1)
{
	if (y1 < minBuildHeight || y0 >= maxBuildHeight) return false;

	x0 >>= 4;
	z0 >>= 4;
	x1 >>= 4;
	z1 >>= 4;

	for (int x = x0; x <= x1; x++)
		for (int z = z0; z <= z1; z++)
			if (!hasChunk(x, z)) return false;

	return true;
}

// 4J added
bool Level::reallyHasChunksAt(int x0, int y0, int z0, int x1, int y1, int z1)
{
	x0 >>= 4;
	z0 >>= 4;
	x1 >>= 4;
	z1 >>= 4;

	for (int x = x0; x <= x1; x++)
		for (int z = z0; z <= z1; z++)
			if (!reallyHasChunk(x, z)) return false;

	return true;
}

bool Level::hasChunk(int x, int z)
{
	return this->chunkSource->hasChunk(x, z);
}

// 4J added
bool Level::reallyHasChunk(int x, int z)
{
	return this->chunkSource->reallyHasChunk(x, z);
}


LevelChunk *Level::getChunkAt(int x, int z)
{
	return getChunk(x >> 4, z >> 4);
}


LevelChunk *Level::getChunk(int x, int z)
{
	return this->chunkSource->getChunk(x, z);
}


bool Level::setTileAndDataNoUpdate(int x, int y, int z, int tile, int data)
{
	return setTileAndDataNoUpdate(x, y, z, tile, data, true);
}

bool Level::setTileAndDataNoUpdate(int x, int y, int z, int tile, int data, bool informClients)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z >= MAX_LEVEL_SIZE)
	{
		return false;
	}
	if (y < 0) return false;
	if (y >= maxBuildHeight) return false;
	LevelChunk *c = getChunk(x >> 4, z >> 4);
	// 4J - changes for lighting brought forward from 1.8.2
	bool result;
#ifndef _CONTENT_PACKAGE
	int old = c->getTile(x & 15, y, z & 15);
	int olddata = c->getData( x & 15, y, z & 15);
#endif
	result = c->setTileAndData(x & 15, y, z & 15, tile, data);
#ifndef _CONTENT_PACKAGE
	PIXBeginNamedEvent(0,"Checking light %d %d %d",x,y,z);
	PIXBeginNamedEvent(0,"was %d, %d now %d, %d",old,olddata,tile,data);
#endif
	this->checkLight(x, y, z);
	PIXEndNamedEvent();
	PIXEndNamedEvent();
	if (informClients && result && (isClientSide || c->seenByPlayer)) sendTileUpdated(x, y, z);
	return result;
}


bool Level::setTileNoUpdate(int x, int y, int z, int tile)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z >= MAX_LEVEL_SIZE)
	{
		return false;
	}
	if (y < 0) return false;
	if (y >= maxBuildHeight) return false;
	LevelChunk *c = getChunk(x >> 4, z >> 4);
	// 4J - changes for lighting brought forward from 1.8.2
	bool result = c->setTile(x & 15, y, z & 15, tile);
	this->checkLight(x, y, z);
	if (result && (isClientSide || c->seenByPlayer)) sendTileUpdated(x, y, z);
	return result;
}

bool Level::setTileNoUpdateNoLightCheck(int x, int y, int z, int tile)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z >= MAX_LEVEL_SIZE)
	{
		return false;
	}
	if (y < 0) return false;
	if (y >= maxBuildHeight) return false;
	LevelChunk *c = getChunk(x >> 4, z >> 4);
	// 4J - changes for lighting brought forward from 1.8.2
	bool result = c->setTile(x & 15, y, z & 15, tile);
	return result;
}


Material *Level::getMaterial(int x, int y, int z)
{
	int t = getTile(x, y, z);
	if (t == 0) return Material::air;
	return Tile::tiles[t]->material;
}


int Level::getData(int x, int y, int z)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z >= MAX_LEVEL_SIZE)
	{
		return 0;
	}
	if (y < 0) return 0;
	if (y >= maxBuildHeight) return 0;
	LevelChunk *c = getChunk(x >> 4, z >> 4);
	x &= 15;
	z &= 15;
	return c->getData(x, y, z);
}


void Level::setData(int x, int y, int z, int data, bool forceUpdate/*=false*/)	// 4J added forceUpdate
{
	if (setDataNoUpdate(x, y, z, data) || forceUpdate)
	{
		tileUpdated(x, y, z, getTile(x, y, z));
	}
}

bool Level::setDataNoUpdate(int x, int y, int z, int data)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z >= MAX_LEVEL_SIZE)
	{
		return false;
	}
	if (y < 0) return false;
	if (y >= maxBuildHeight) return false;
	LevelChunk *c = getChunk(x >> 4, z >> 4);
	int cx = x & 15;
	int cz = z & 15;
	// 4J - have changed _sendTileData to encode a bitfield of which bits are important to be sent. This will be zero where the original flag was false, and non-zero where the original
	// flag was true - hence recreating the original flag as sendTileData here. For nearly all tiles this will be 15 for the case where this used to be true (ie all bits are important) so
	// there should be absolutely to change in behaviour. However, for leaf tiles, bits have been masked so we don't bother doing sendTileUpdated if a non-visual thing has changed in the data
	unsigned char importantMask = Tile::_sendTileData[c->getTile(cx, y, cz) & Tile::TILE_NUM_MASK];
	bool sendTileData = importantMask != 0;

	bool maskedBitsChanged;
	bool result = c->setData(cx, y, cz, data, importantMask, &maskedBitsChanged);
	if (result && (isClientSide || (c->seenByPlayer && sendTileData && maskedBitsChanged))) sendTileUpdated(x, y, z);
	return result;
}


bool Level::setTile(int x, int y, int z, int tile)
{
	if (setTileNoUpdate(x, y, z, tile))
	{
		tileUpdated(x, y, z, tile);
		return true;
	}
	return false;
}


bool Level::setTileAndData(int x, int y, int z, int tile, int data)
{
	if (setTileAndDataNoUpdate(x, y, z, tile, data))
	{
		tileUpdated(x, y, z, tile);
		return true;
	}
	return false;
}

void Level::sendTileUpdated(int x, int y, int z)
{
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
	{
		(*it)->tileChanged(x, y, z);
	}
}

void Level::tileUpdated(int x, int y, int z, int tile)
{
	this->updateNeighborsAt(x, y, z, tile);
}


void Level::lightColumnChanged(int x, int z, int y0, int y1)
{
	PIXBeginNamedEvent(0,"LightColumnChanged (%d,%d) %d to %d",x,z,y0,y1);
	if (y0 > y1)
	{
		int tmp = y1;
		y1 = y0;
		y0 = tmp;
	}

	if (!dimension->hasCeiling)
	{
		PIXBeginNamedEvent(0,"Checking lights");
		for (int y = y0; y <= y1; y++)
		{
			PIXBeginNamedEvent(0,"Checking light %d", y);
			checkLight(LightLayer::Sky, x, y, z);
			PIXEndNamedEvent();
		}
		PIXEndNamedEvent();
	}
	PIXBeginNamedEvent(0,"Setting tiles dirty");
	setTilesDirty(x, y0, z, x, y1, z);
	PIXEndNamedEvent();
	PIXEndNamedEvent();
}


void Level::setTileDirty(int x, int y, int z)
{
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
	{
		(*it)->setTilesDirty(x, y, z, x, y, z, this);
	}
}


void Level::setTilesDirty(int x0, int y0, int z0, int x1, int y1, int z1)
{
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
	{
		(*it)->setTilesDirty(x0, y0, z0, x1, y1, z1, this);
	}
}


void Level::swap(int x1, int y1, int z1, int x2, int y2, int z2)
{
	int t1 = getTile(x1, y1, z1);
	int d1 = getData(x1, y1, z1);
	int t2 = getTile(x2, y2, z2);
	int d2 = getData(x2, y2, z2);

	setTileAndDataNoUpdate(x1, y1, z1, t2, d2);
	setTileAndDataNoUpdate(x2, y2, z2, t1, d1);

	updateNeighborsAt(x1, y1, z1, t2);
	updateNeighborsAt(x2, y2, z2, t1);
}


void Level::updateNeighborsAt(int x, int y, int z, int tile)
{
	neighborChanged(x - 1, y, z, tile);
	neighborChanged(x + 1, y, z, tile);
	neighborChanged(x, y - 1, z, tile);
	neighborChanged(x, y + 1, z, tile);
	neighborChanged(x, y, z - 1, tile);
	neighborChanged(x, y, z + 1, tile);
}


void Level::neighborChanged(int x, int y, int z, int type)
{
	if (noNeighborUpdate || isClientSide) return;
	Tile *tile = Tile::tiles[getTile(x, y, z)];
	if (tile != NULL) tile->neighborChanged(this, x, y, z, type);
}



bool Level::canSeeSky(int x, int y, int z)
{
	return getChunk(x >> 4, z >> 4)->isSkyLit(x & 15, y, z & 15);
}


int Level::getDaytimeRawBrightness(int x, int y, int z)
{
	if (y < 0) return 0;
	if (y >= maxBuildHeight) y = maxBuildHeight - 1;
	return getChunk(x >> 4, z >> 4)->getRawBrightness(x & 15, y, z & 15, 0);
}


int Level::getRawBrightness(int x, int y, int z)
{
	return getRawBrightness(x, y, z, true);
}


int Level::getRawBrightness(int x, int y, int z, bool propagate)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z >= MAX_LEVEL_SIZE)
	{
		return MAX_BRIGHTNESS;
	}

	if (propagate)
	{
		int id = getTile(x, y, z);
		switch(id)
		{
		case Tile::stoneSlabHalf_Id:
		case Tile::woodSlabHalf_Id:
		case Tile::farmland_Id:
		case Tile::stairs_stone_Id:
		case Tile::stairs_wood_Id:
		{
			int br = getRawBrightness(x, y + 1, z, false);
			int br1 = getRawBrightness(x + 1, y, z, false);
			int br2 = getRawBrightness(x - 1, y, z, false);
			int br3 = getRawBrightness(x, y, z + 1, false);
			int br4 = getRawBrightness(x, y, z - 1, false);
			if (br1 > br) br = br1;
			if (br2 > br) br = br2;
			if (br3 > br) br = br3;
			if (br4 > br) br = br4;
			return br;
		}
		break;
		}
	}

	if (y < 0) return 0;
	if (y >= maxBuildHeight) y = maxBuildHeight - 1;

	LevelChunk *c = getChunk(x >> 4, z >> 4);
	x &= 15;
	z &= 15;
	return c->getRawBrightness(x, y, z, skyDarken);
}


bool Level::isSkyLit(int x, int y, int z)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z >= MAX_LEVEL_SIZE)
	{
		return false;
	}
	if (dimension->hasCeiling) return false;

	if (y < 0) return false;
	if (y >= maxBuildHeight) return true;
	if (!hasChunk(x >> 4, z >> 4)) return false;

	LevelChunk *c = getChunk(x >> 4, z >> 4);
	x &= 15;
	z &= 15;
	return c->isSkyLit(x, y, z);
}


int Level::getHeightmap(int x, int z)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z >= MAX_LEVEL_SIZE)
	{
		return 0;
	}
	if (!hasChunk(x >> 4, z >> 4)) return 0;

	LevelChunk *c = getChunk(x >> 4, z >> 4);
	return c->getHeightmap(x & 15, z & 15);
}


void Level::updateLightIfOtherThan(LightLayer::variety layer, int x, int y, int z, int expected)
{
	if (dimension->hasCeiling && layer == LightLayer::Sky) return;

	if (!hasChunkAt(x, y, z)) return;

	if (layer == LightLayer::Sky)
	{
		if (isSkyLit(x, y, z)) expected = 15;
	}
	else if (layer == LightLayer::Block)
	{
		int t = getTile(x, y, z);
		if (Tile::lightEmission[t] > expected) expected = Tile::lightEmission[t];
	}

	if (getBrightness(layer, x, y, z) != expected)
	{
		setBrightness(layer, x, y, z, expected);
	}
}

// 4J - update brought forward from 1.8.2
int Level::getBrightnessPropagate(LightLayer::variety layer, int x, int y, int z, int tileId)
{
	if (dimension->hasCeiling && layer == LightLayer::Sky) return 0;

    if (y < 0) y = 0;
    if (y >= maxBuildHeight && layer == LightLayer::Sky)
	{
		// 4J Stu - The java LightLayer was an enum class type with a member "surrounding" which is what we
		// were returning here. Surrounding has the same value as the enum value in our C++ code, so just cast
		// it to an int
		return (int)layer;
    }
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z >= MAX_LEVEL_SIZE)
	{
		// 4J Stu - The java LightLayer was an enum class type with a member "surrounding" which is what we
		// were returning here. Surrounding has the same value as the enum value in our C++ code, so just cast
		// it to an int
		return (int)layer;
    }
    int xc = x >> 4;
    int zc = z >> 4;
    if (!hasChunk(xc, zc)) return (int)layer;

    {
		int id = tileId > -1 ? tileId : getTile(x,y,z);
		if (Tile::propagate[id])
		{
            int br = getBrightness(layer, x, y + 1, z);
            int br1 = getBrightness(layer, x + 1, y, z);
            int br2 = getBrightness(layer, x - 1, y, z);
            int br3 = getBrightness(layer, x, y, z + 1);
            int br4 = getBrightness(layer, x, y, z - 1);
            if (br1 > br) br = br1;
            if (br2 > br) br = br2;
            if (br3 > br) br = br3;
            if (br4 > br) br = br4;
            return br;
        }
    }

    LevelChunk *c = getChunk(xc, zc);
    return c->getBrightness(layer, x & 15, y, z & 15);
}

int Level::getBrightness(LightLayer::variety layer, int x, int y, int z)
{
	// 4J - optimised. Not doing checks on x/z that are no longer necessary, and directly checking the cache within
	// the ServerChunkCache/MultiplayerChunkCache rather than going through wrappers & virtual functions.
	int xc = x >> 4;
	int zc = z >> 4;

	int ix = xc + (chunkSourceXZSize/2);
	int iz = zc + (chunkSourceXZSize/2);

	if( ( ix < 0 ) || ( ix >= chunkSourceXZSize ) ) return 0;
	if( ( iz < 0 ) || ( iz >= chunkSourceXZSize ) ) return 0;
	int idx = ix * chunkSourceXZSize + iz;
	LevelChunk *c = chunkSourceCache[idx];

	if( c == NULL ) return (int)layer;

    if (y < 0) y = 0;
    if (y >= maxBuildHeight) y = maxBuildHeight - 1;

	return c->getBrightness(layer, x & 15, y, z & 15);
}

// 4J added as optimisation - if all the neighbouring brightesses are going to be in the one chunk, just get
// the level chunk once
void Level::getNeighbourBrightnesses(int *brightnesses, LightLayer::variety layer, int x, int y, int z)
{
	if( ( ( ( x & 15 ) == 0 ) || ( ( x & 15 ) == 15 ) ) || 
		( ( ( z & 15 ) == 0 ) || ( ( z & 15 ) == 15 ) ) ||
		( ( y <= 0 ) || ( y >= 127 ) ) )
	{
		// We're spanning more than one chunk, just fall back on original java method here
		brightnesses[0] = getBrightness(layer, x - 1, y, z);
		brightnesses[1] = getBrightness(layer, x + 1, y, z);
		brightnesses[2] = getBrightness(layer, x, y - 1, z);
		brightnesses[3] = getBrightness(layer, x, y + 1, z);
		brightnesses[4] = getBrightness(layer, x, y, z - 1);
		brightnesses[5] = getBrightness(layer, x, y, z + 1);
	}
	else
	{
		// All in one chunk - just get the chunk once, and do a single call to get the results
		int xc = x >> 4;
		int zc = z >> 4;

		int ix = xc + (chunkSourceXZSize/2);
		int iz = zc + (chunkSourceXZSize/2);

		// 4J Stu - The java LightLayer was an enum class type with a member "surrounding" which is what we
		// were returning here. Surrounding has the same value as the enum value in our C++ code, so just cast
		// it to an int
		if( ( ( ix < 0 ) || ( ix >= chunkSourceXZSize ) ) ||
			( ( iz < 0 ) || ( iz >= chunkSourceXZSize ) ) )
		{
			for( int i = 0; i < 6; i++ )
			{
				brightnesses[i] = (int)layer;
			}
			return;
		}

		int idx = ix * chunkSourceXZSize + iz;
		LevelChunk *c = chunkSourceCache[idx];

		// 4J Stu - The java LightLayer was an enum class type with a member "surrounding" which is what we
		// were returning here. Surrounding has the same value as the enum value in our C++ code, so just cast
		// it to an int
		if( c == NULL )
		{
			for( int i = 0; i < 6; i++ )
			{
				brightnesses[i] = (int)layer;
			}
			return;
		}

		// Single call to the levelchunk too to avoid overhead of virtual fn calls
		c->getNeighbourBrightnesses(brightnesses, layer, x & 15, y, z & 15);
	}
}

void Level::setBrightness(LightLayer::variety layer, int x, int y, int z, int brightness, bool noUpdateOnClient/*=false*/)		// 4J added noUpdateOnClient
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z >= MAX_LEVEL_SIZE)
	{
		return;
	}
	if (y < 0) return;
	if (y >= maxBuildHeight) return;
	if (!hasChunk(x >> 4, z >> 4)) return;
	LevelChunk *c = getChunk(x >> 4, z >> 4);

	c->setBrightness(layer, x & 15, y, z & 15, brightness);

	// 4J added
	if( isClientSide && noUpdateOnClient )
	{
		if( cachewritten )
		{
			if( x < cacheminx ) cacheminx = x;
			if( x > cachemaxx ) cachemaxx = x;
			if( y < cacheminy ) cacheminy = y;
			if( y > cachemaxy ) cachemaxy = y;
			if( z < cacheminz ) cacheminz = z;
			if( z > cachemaxz ) cachemaxz = z;
		}
		else
		{
			cachewritten = true;
			cacheminx = x;
			cachemaxx = x;
			cacheminy = y;
			cachemaxy = y;
			cacheminz = z;
			cachemaxz = z;
		}
	}
	else
	{
		AUTO_VAR(itEnd, listeners.end());
		for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
		{
			(*it)->tileLightChanged(x, y, z);
		}
	}
}

void Level::setTileBrightnessChanged(int x, int y, int z)
{
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
	{
		(*it)->tileLightChanged(x, y, z);
	}
}

int Level::getLightColor(int x, int y, int z, int emitt, int tileId/*=-1*/)
{
	int s = getBrightnessPropagate(LightLayer::Sky, x, y, z, tileId);
    int b = getBrightnessPropagate(LightLayer::Block, x, y, z, tileId);
    if (b < emitt) b = emitt;
    return s << 20 | b << 4;
}

float Level::getBrightness(int x, int y, int z, int emitt)
{
	int n = getRawBrightness(x, y, z);
	if (n < emitt) n = emitt;
	return dimension->brightnessRamp[n];
}


float Level::getBrightness(int x, int y, int z)
{
	return dimension->brightnessRamp[getRawBrightness(x, y, z)];
}


bool Level::isDay()
{
	return this->skyDarken < 4;
}


HitResult *Level::clip(Vec3 *a, Vec3 *b)
{
	return clip(a, b, false, false);
}


HitResult *Level::clip(Vec3 *a, Vec3 *b, bool liquid)
{
	return clip(a, b, liquid, false);
}


HitResult *Level::clip(Vec3 *a, Vec3 *b, bool liquid, bool solidOnly)
{
	if (Double::isNaN(a->x) || Double::isNaN(a->y) || Double::isNaN(a->z)) return NULL;
	if (Double::isNaN(b->x) || Double::isNaN(b->y) || Double::isNaN(b->z)) return NULL;

	int xTile1 = Mth::floor(b->x);
	int yTile1 = Mth::floor(b->y);
	int zTile1 = Mth::floor(b->z);

	int xTile0 = Mth::floor(a->x);
	int yTile0 = Mth::floor(a->y);
	int zTile0 = Mth::floor(a->z);

	{
		int t = getTile(xTile0, yTile0, zTile0);
		int data = getData(xTile0, yTile0, zTile0);
		Tile *tile = Tile::tiles[t];
		if (solidOnly && tile != NULL && tile->getAABB(this, xTile0, yTile0, zTile0) == NULL)
		{
			// No collision

		}
		else if (t > 0 && tile->mayPick(data, liquid))
		{
			HitResult *r = tile->clip(this, xTile0, yTile0, zTile0, a, b);
			if (r != NULL) return r;
		}
	}

	int maxIterations = 200;
	while (maxIterations-- >= 0)
	{
		if (Double::isNaN(a->x) || Double::isNaN(a->y) || Double::isNaN(a->z)) return NULL;
		if (xTile0 == xTile1 && yTile0 == yTile1 && zTile0 == zTile1) return NULL;

		bool xClipped = true;
		bool yClipped = true;
		bool zClipped = true;

		double xClip = 999;
		double yClip = 999;
		double zClip = 999;

		if (xTile1 > xTile0) xClip = xTile0 + 1.000;
		else if (xTile1 < xTile0) xClip = xTile0 + 0.000;
		else xClipped = false;

		if (yTile1 > yTile0) yClip = yTile0 + 1.000;
		else if (yTile1 < yTile0) yClip = yTile0 + 0.000;
		else yClipped = false;

		if (zTile1 > zTile0) zClip = zTile0 + 1.000;
		else if (zTile1 < zTile0) zClip = zTile0 + 0.000;
		else zClipped = false;

		double xDist = 999;
		double yDist = 999;
		double zDist = 999;

		double xd = b->x - a->x;
		double yd = b->y - a->y;
		double zd = b->z - a->z;

		if (xClipped) xDist = (xClip - a->x) / xd;
		if (yClipped) yDist = (yClip - a->y) / yd;
		if (zClipped) zDist = (zClip - a->z) / zd;

		int face = 0;
		if (xDist < yDist && xDist < zDist)
		{
			if (xTile1 > xTile0) face = 4;
			else face = 5;

			a->x = xClip;
			a->y += yd * xDist;
			a->z += zd * xDist;
		}
		else if (yDist < zDist)
		{
			if (yTile1 > yTile0) face = 0;
			else face = 1;

			a->x += xd * yDist;
			a->y = yClip;
			a->z += zd * yDist;
		}
		else
		{
			if (zTile1 > zTile0) face = 2;
			else face = 3;

			a->x += xd * zDist;
			a->y += yd * zDist;
			a->z = zClip;
		}

		Vec3 *tPos = Vec3::newTemp(a->x, a->y, a->z);
		xTile0 = (int) (tPos->x = floor(a->x));
		if (face == 5)
		{
			xTile0--;
			tPos->x++;
		}
		yTile0 = (int) (tPos->y = floor(a->y));
		if (face == 1)
		{
			yTile0--;
			tPos->y++;
		}
		zTile0 = (int) (tPos->z = floor(a->z));
		if (face == 3)
		{
			zTile0--;
			tPos->z++;
		}

		int t = getTile(xTile0, yTile0, zTile0);
		int data = getData(xTile0, yTile0, zTile0);
		Tile *tile = Tile::tiles[t];
		if (solidOnly && tile != NULL && tile->getAABB(this, xTile0, yTile0, zTile0) == NULL)
		{
			// No collision

		}
		else if (t > 0 && tile->mayPick(data, liquid))
		{
			HitResult *r = tile->clip(this, xTile0, yTile0, zTile0, a, b);
			if (r != NULL) return r;
		}
	}
	return NULL;
}


void Level::playSound(shared_ptr<Entity> entity, int iSound, float volume, float pitch)
{
	if(entity == NULL) return;
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
	{
		// 4J-PB - if the entity is a local player, don't play the sound
		if(entity->GetType() == eTYPE_SERVERPLAYER)
		{
			//app.DebugPrintf("ENTITY is serverplayer\n");
	
			(*it)->playSound(entity,iSound, entity->x, entity->y - entity->heightOffset, entity->z, volume, pitch);
		}
		else
		{
			(*it)->playSound(iSound, entity->x, entity->y - entity->heightOffset, entity->z, volume, pitch);
		}
	}
}


//void Level::playSound(double x, double y, double z, const wstring& name, float volume, float pitch)
void Level::playSound(double x, double y, double z, int iSound, float volume, float pitch, float fClipSoundDist)
{
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
	{
		(*it)->playSound(iSound, x, y, z, volume, pitch, fClipSoundDist);
	}
}

void Level::playLocalSound(double x, double y, double z, int iSound, float volume, float pitch, float fClipSoundDist)
{
}

void Level::playStreamingMusic(const wstring& name, int x, int y, int z)
{
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
	{
		(*it)->playStreamingMusic(name, x, y, z);
	}
}


void Level::playMusic(double x, double y, double z, const wstring& string, float volume)
{
}

// 4J removed - 
/*
void Level::addParticle(const wstring& id, double x, double y, double z, double xd, double yd, double zd)
{
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
		(*it)->addParticle(id, x, y, z, xd, yd, zd);
}
*/

// 4J-PB added
void Level::addParticle(ePARTICLE_TYPE id, double x, double y, double z, double xd, double yd, double zd)
{
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
		(*it)->addParticle(id, x, y, z, xd, yd, zd);
}

bool Level::addGlobalEntity(shared_ptr<Entity> e)
{
	globalEntities.push_back(e);
	return true;
}

#pragma optimize( "", off )

bool Level::addEntity(shared_ptr<Entity> e)
{
	int xc = Mth::floor(e->x / 16);
	int zc = Mth::floor(e->z / 16);

	if(e == NULL)
	{
		return false;
	}

	bool forced = false;
	if (dynamic_pointer_cast<Player>( e ) != NULL)
	{
		forced = true;
	}

	if (forced || hasChunk(xc, zc))
	{
		if (dynamic_pointer_cast<Player>( e ) != NULL)
		{
			shared_ptr<Player> player = dynamic_pointer_cast<Player>(e);

			// 4J Stu - Added so we don't continually add the player to the players list while they are dead
			if( find( players.begin(), players.end(), e ) == players.end() )
			{
				players.push_back(player);
			}

			updateSleepingPlayerList();
		}
		MemSect(42);
		getChunk(xc, zc)->addEntity(e);
		MemSect(0);
		EnterCriticalSection(&m_entitiesCS);
		MemSect(43);
		entities.push_back(e);
		MemSect(0);
		LeaveCriticalSection(&m_entitiesCS);
		MemSect(44);
		entityAdded(e);
		MemSect(0);
		return true;
	}
	return false;
}

#pragma optimize( "", on )

void Level::entityAdded(shared_ptr<Entity> e)
{
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
	{
		(*it)->entityAdded(e);
	}
}


void Level::entityRemoved(shared_ptr<Entity> e)
{
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
	{
		(*it)->entityRemoved(e);
	}
}

// 4J added
void Level::playerRemoved(shared_ptr<Entity> e)
{
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
	{
		(*it)->playerRemoved(e);
	}
}

void Level::removeEntity(shared_ptr<Entity> e)
{
	if (e->rider.lock() != NULL)
	{
		e->rider.lock()->ride(nullptr);
	}
	if (e->riding != NULL)
	{
		e->ride(nullptr);
	}
	e->remove();
	if (dynamic_pointer_cast<Player>( e ) != NULL)
	{
		vector<shared_ptr<Player> >::iterator it = players.begin();
		vector<shared_ptr<Player> >::iterator itEnd = players.end();
		while( it != itEnd && *it != dynamic_pointer_cast<Player>(e) )
			it++;

		if( it != itEnd )
		{
			players.erase( it );
		}

		updateSleepingPlayerList();
		playerRemoved(e);	// 4J added - this will let the entity tracker know that we have actually removed the player from the level's player list
	}
}


void Level::removeEntityImmediately(shared_ptr<Entity> e)
{
	e->remove();

	if (dynamic_pointer_cast<Player>( e ) != NULL)
	{
		vector<shared_ptr<Player> >::iterator it = players.begin();
		vector<shared_ptr<Player> >::iterator itEnd = players.end();
		while( it != itEnd && *it != dynamic_pointer_cast<Player>(e) )
			it++;

		if( it != itEnd )
		{
			players.erase( it );
		}

		updateSleepingPlayerList();
		playerRemoved(e);	// 4J added - this will let the entity tracker know that we have actually removed the player from the level's player list
	}

	int xc = e->xChunk;
	int zc = e->zChunk;
	if (e->inChunk && hasChunk(xc, zc))
	{
		getChunk(xc, zc)->removeEntity(e);
	}

	EnterCriticalSection(&m_entitiesCS);
	vector<shared_ptr<Entity> >::iterator it = entities.begin();
	vector<shared_ptr<Entity> >::iterator endIt = entities.end();
	while( it != endIt && *it != e)
		it++;

	if( it != endIt )
	{
		entities.erase( it );
	}
	LeaveCriticalSection(&m_entitiesCS);
	entityRemoved(e);
}


void Level::addListener(LevelListener *listener)
{
	listeners.push_back(listener);
}


void Level::removeListener(LevelListener *listener)
{
	vector<LevelListener *>::iterator it = listeners.begin();
	vector<LevelListener *>::iterator itEnd = listeners.end();
	while( it != itEnd && *it != listener )
		it++;

	if( it != itEnd )
		listeners.erase( it );
}


// 4J - added noEntities and blockAtEdge parameter
AABBList *Level::getCubes(shared_ptr<Entity> source, AABB *box, bool noEntities, bool blockAtEdge)
{
	boxes.clear();
	int x0 = Mth::floor(box->x0);
	int x1 = Mth::floor(box->x1 + 1);
	int y0 = Mth::floor(box->y0);
	int y1 = Mth::floor(box->y1 + 1);
	int z0 = Mth::floor(box->z0);
	int z1 = Mth::floor(box->z1 + 1);

	int maxxz = ( dimension->getXZSize() * 16 ) / 2;
	int minxz = -maxxz;
	for (int x = x0; x < x1; x++)
		for (int z = z0; z < z1; z++)
		{
			// 4J - If we are outside the map, return solid AABBs (rock is a bit of an arbitrary choice here, just need a correct AABB)
			if( blockAtEdge && ( ( x < minxz ) || ( x >= maxxz ) || ( z < minxz ) || ( z >= maxxz ) ) )
			{
				for (int y = y0 - 1; y < y1; y++)
				{
					Tile::rock->addAABBs(this, x, y, z, box, &boxes, source);
				}
			}
			else
			{
				if (hasChunkAt(x, 64, z))
				{
					for (int y = y0 - 1; y < y1; y++)
					{
						Tile *tile = Tile::tiles[getTile(x, y, z)];
						if (tile != NULL)
						{
							tile->addAABBs(this, x, y, z, box, &boxes, source);
						}
					}
				}
			}
		}
	// 4J - also stop player falling out of the bottom of the map if blockAtEdge is true. Again, rock is an arbitrary choice here
	// 4J Stu - Don't stop entities falling into the void while in The End (it has no bedrock)
	if( blockAtEdge && ( ( y0 - 1 ) < 0 ) && dimension->id != 1 )
	{
		for (int y = y0 - 1; y < 0; y++)
		{
			for (int x = x0; x < x1; x++)
				for (int z = z0; z < z1; z++)
				{
					Tile::rock->addAABBs(this, x, y, z, box, &boxes, source );
				}
		}
	}
	// 4J - final bounds check - limit vertical movement so we can't move above maxMovementHeight
	if( blockAtEdge && ( y1 > maxMovementHeight ) )
	{
		for (int y = maxMovementHeight; y < y1; y++)
		{
			for (int x = x0; x < x1; x++)
				for (int z = z0; z < z1; z++)
				{
					Tile::rock->addAABBs(this, x, y, z, box, &boxes, source );
				}
		}
	}
	// 4J - now add in collision for any blocks which have actually been removed, but haven't had their render data updated to reflect this yet. This is to stop the player
	// being able to move the view position inside a tile which is (visually) still there, and see out of the world. This is particularly a problem when moving upwards in
	// creative mode as the player can get very close to the edge of tiles whilst looking upwards and can therefore very quickly move inside one.
	Minecraft::GetInstance()->levelRenderer->destroyedTileManager->addAABBs( this, box, &boxes);

	// 4J - added
	if( noEntities ) return &boxes;

	double r = 0.25;
	vector<shared_ptr<Entity> > *ee = getEntities(source, box->grow(r, r, r));
	vector<shared_ptr<Entity> >::iterator itEnd = ee->end();
	for (AUTO_VAR(it, ee->begin()); it != itEnd; it++)
	{
		AABB *collideBox = (*it)->getCollideBox();
		if (collideBox != NULL && collideBox->intersects(box))
		{
			boxes.push_back(collideBox);
		}

		collideBox = source->getCollideAgainstBox(*it);
		if (collideBox != NULL && collideBox->intersects(box))
		{
			boxes.push_back(collideBox);
		}
	}

	return &boxes;
}

// 4J Stu - Brought forward from 12w36 to fix #46282 - TU5: Gameplay: Exiting the minecart in a tight corridor damages the player
AABBList *Level::getTileCubes(AABB *box, bool blockAtEdge)
{
	return getCubes(nullptr, box, true, blockAtEdge);
	//boxes.clear();
	//int x0 = Mth::floor(box->x0);
	//int x1 = Mth::floor(box->x1 + 1);
	//int y0 = Mth::floor(box->y0);
	//int y1 = Mth::floor(box->y1 + 1);
	//int z0 = Mth::floor(box->z0);
	//int z1 = Mth::floor(box->z1 + 1);

	//for (int x = x0; x < x1; x++)
	//{
	//	for (int z = z0; z < z1; z++)
	//	{
	//		if (hasChunkAt(x, 64, z))
	//		{
	//			for (int y = y0 - 1; y < y1; y++)
	//			{
	//				Tile *tile = Tile::tiles[getTile(x, y, z)];

	//				if (tile != NULL)
	//				{
	//					tile->addAABBs(this, x, y, z, box, &boxes);
	//				}
	//			}
	//		}
	//	}
	//}

	//return boxes;
}

//4J - change brought forward from 1.8.2
int Level::getOldSkyDarken(float a)
{
	float td = getTimeOfDay(a);

	float br = 1 - (Mth::cos(td * PI * 2) * 2 + 0.5f);
	if (br < 0.0f) br = 0.0f;
	if (br > 1.0f) br = 1.0f;

	br = 1 - br;

	br *= 1 - (getRainLevel(a) * 5 / 16.0f);
	br *= 1 - (getThunderLevel(a) * 5 / 16.0f);
	br = 1 - br;
	return ((int) (br * 11));
}

//4J - change brought forward from 1.8.2
float Level::getSkyDarken(float a)
{
    float td = getTimeOfDay(a);

    float br = 1 - (Mth::cos(td * PI * 2) * 2 + 0.2f);
    if (br < 0.0f) br = 0.0f;
    if (br > 1.0f) br = 1.0f;

    br = 1.0f - br;

    br *= 1.0f - (getRainLevel(a) * 5.0f / 16.0f);
    br *= 1.0f - (getThunderLevel(a) * 5.0f / 16.0f);
    // return ((int) (br * 13));

    return br * 0.8f + 0.2f;
}



Vec3 *Level::getSkyColor(shared_ptr<Entity> source, float a)
{
	float td = getTimeOfDay(a);

	float br = Mth::cos(td * PI * 2) * 2 + 0.5f;
	if (br < 0.0f) br = 0.0f;
	if (br > 1.0f) br = 1.0f;

	int xx = Mth::floor(source->x);
	int zz = Mth::floor(source->z);
	Biome *biome = getBiome(xx, zz);
	float temp = biome->getTemperature();
	int skyColor = biome->getSkyColor(temp);

	float r = ((skyColor >> 16) & 0xff) / 255.0f;
	float g = ((skyColor >> 8) & 0xff) / 255.0f;
	float b = ((skyColor) & 0xff) / 255.0f;
	r *= br;
	g *= br;
	b *= br;

	float rainLevel = getRainLevel(a);
	if (rainLevel > 0)
	{
		float mid = (r * 0.30f + g * 0.59f + b * 0.11f) * 0.6f;

		float ba = 1 - rainLevel * 0.75f;
		r = r * ba + mid * (1 - ba);
		g = g * ba + mid * (1 - ba);
		b = b * ba + mid * (1 - ba);
	}
	float thunderLevel = getThunderLevel(a);
	if (thunderLevel > 0)
	{
		float mid = (r * 0.30f + g * 0.59f + b * 0.11f) * 0.2f;

		float ba = 1 - thunderLevel * 0.75f;
		r = r * ba + mid * (1 - ba);
		g = g * ba + mid * (1 - ba);
		b = b * ba + mid * (1 - ba);
	}

	if (lightningBoltTime > 0)
	{
		float f = (lightningBoltTime - a);
		if (f > 1) f = 1;
		f = f * 0.45f;
		r = r * (1 - f) + 0.8f * f;
		g = g * (1 - f) + 0.8f * f;
		b = b * (1 - f) + 1 * f;
	}

	return Vec3::newTemp(r, g, b);
}


float Level::getTimeOfDay(float a)
{
	/*
	* 4J-PB removed line below - notch committed 1.6.6 with the incorrect
	* getTimeOfDay and changed it before releasing (without
	* re-committing)... that should be the only difference // jeb
	*/
	/* if (this != NULL) return 0.5f; */

	// 4J Added if so we can override timeOfDay without changing the time that affects ticking of things
	if( m_timeOfDayOverride >= 0 )
	{
		return dimension->getTimeOfDay(m_timeOfDayOverride, a);
	}
	else
	{
		return dimension->getTimeOfDay(levelData->getTime(), a);;
	}
}

int Level::getMoonPhase(float a)
{
	return dimension->getMoonPhase(levelData->getTime(), a);
}

float Level::getSunAngle(float a)
{
	float td = getTimeOfDay(a);
	return td * PI * 2;
}


Vec3 *Level::getCloudColor(float a)
{
	float td = getTimeOfDay(a);

	float br = Mth::cos(td * PI * 2) * 2.0f + 0.5f;
	if (br < 0.0f) br = 0.0f;
	if (br > 1.0f) br = 1.0f;

	int baseCloudColour = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_In_Cloud_Base_Colour );

	float r = ((baseCloudColour >> 16) & 0xff) / 255.0f;
	float g = ((baseCloudColour >> 8) & 0xff) / 255.0f;
	float b = ((baseCloudColour) & 0xff) / 255.0f;

	float rainLevel = getRainLevel(a);
	if (rainLevel > 0)
	{
		float mid = (r * 0.30f + g * 0.59f + b * 0.11f) * 0.6f;

		float ba = 1 - rainLevel * 0.95f;
		r = r * ba + mid * (1 - ba);
		g = g * ba + mid * (1 - ba);
		b = b * ba + mid * (1 - ba);
	}

	r *= br * 0.90f + 0.10f;
	g *= br * 0.90f + 0.10f;
	b *= br * 0.85f + 0.15f;

	float thunderLevel = getThunderLevel(a);
	if (thunderLevel > 0)
	{
		float mid = (r * 0.30f + g * 0.59f + b * 0.11f) * 0.2f;

		float ba = 1 - thunderLevel * 0.95f;
		r = r * ba + mid * (1 - ba);
		g = g * ba + mid * (1 - ba);
		b = b * ba + mid * (1 - ba);
	}

	return Vec3::newTemp(r, g, b);
}


Vec3 *Level::getFogColor(float a)
{
	float td = getTimeOfDay(a);
	return dimension->getFogColor(td, a);
}


int Level::getTopRainBlock(int x, int z)
{
	// 4J - optimisation brought forward from 1.8.2 - used to do full calculation here but result is now cached in LevelChunk
	return getChunkAt(x, z)->getTopRainBlock(x & 15, z & 15);
}

// 4J added
bool Level::biomeHasRain(int x, int z)
{
	return getChunkAt(x, z)->biomeHasRain(x & 15, z & 15);
}

// 4J added
bool Level::biomeHasSnow(int x, int z)
{
	return getChunkAt(x, z)->biomeHasSnow(x & 15, z & 15);
}

int Level::getTopSolidBlock(int x, int z)
{
	LevelChunk *levelChunk = getChunkAt(x, z);

	int y = levelChunk->getHighestSectionPosition() + 15;

	x &= 15;
	z &= 15;

	while (y > 0)
	{
		int t = levelChunk->getTile(x, y, z);
		if (t == 0 || !(Tile::tiles[t]->material->blocksMotion()) || Tile::tiles[t]->material == Material::leaves)
		{
			y--;
		}
		else
		{
			return y + 1;
		}
	}
	return -1;
}


int Level::getLightDepth(int x, int z)
{
	return getChunkAt(x, z)->getHeightmap(x & 15, z & 15);
}


float Level::getStarBrightness(float a)
{
	float td = getTimeOfDay(a);

	float br = 1 - (Mth::cos(td * PI * 2) * 2 + 0.25f);
	if (br < 0.0f) br = 0.0f;
	if (br > 1.0f) br = 1.0f;

	return br * br * 0.5f;
}


void Level::addToTickNextTick(int x, int y, int z, int tileId, int tickDelay)
{
}

void Level::forceAddTileTick(int x, int y, int z, int tileId, int tickDelay)
{
}

void Level::tickEntities()
{
	//for (int i = 0; i < globalEntities.size(); i++)
	vector<shared_ptr<Entity> >::iterator itGE = globalEntities.begin();
	while( itGE != globalEntities.end() )
	{
		shared_ptr<Entity> e = *itGE;//globalEntities.at(i);
		e->tick();
		if (e->removed)
		{
			//globalEntities.remove(i--);
			itGE = globalEntities.erase( itGE );
		}
		else
		{
			itGE++;
		}
	}

	//entities.removeAll(entitiesToRemove);
	EnterCriticalSection(&m_entitiesCS);

	for( AUTO_VAR(it, entities.begin()); it != entities.end(); )
	{
		bool found = false;
		for( AUTO_VAR(it2, entitiesToRemove.begin()); it2 != entitiesToRemove.end(); it2++ )
		{
			if( (*it) == (*it2) )
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

	AUTO_VAR(itETREnd, entitiesToRemove.end());
	for (AUTO_VAR(it, entitiesToRemove.begin()); it != itETREnd; it++)
	{
		shared_ptr<Entity> e = *it;//entitiesToRemove.at(j);
		int xc = e->xChunk;
		int zc = e->zChunk;
		if (e->inChunk && hasChunk(xc, zc))
		{
			getChunk(xc, zc)->removeEntity(e);
		}
	}

	itETREnd = entitiesToRemove.end();
	for (AUTO_VAR(it, entitiesToRemove.begin()); it != itETREnd; it++)
	{
		entityRemoved(*it);
	}
	// 
	entitiesToRemove.clear();

	//for (int i = 0; i < entities.size(); i++)

	/* 4J Jev, using an iterator causes problems here as
	 * the vector is modified from inside this loop.
	 */
	EnterCriticalSection(&m_entitiesCS);

	for (unsigned int i = 0; i < entities.size(); )
	{
		shared_ptr<Entity> e = entities.at(i);
		
		if (e->riding != NULL)
		{
			if (e->riding->removed || e->riding->rider.lock() != e)
			{
				e->riding->rider = weak_ptr<Entity>();
				e->riding = nullptr;
			}
			else
			{
				i++;
				continue;
			}
		}

		if (!e->removed)
		{
#ifndef _FINAL_BUILD
			if(!( app.DebugSettingsOn() && app.GetMobsDontTickEnabled() && (dynamic_pointer_cast<Mob>(e) != NULL) &&  (dynamic_pointer_cast<Player>(e) == NULL)))
#endif			
			{
				tick(e);
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
			//itE = entities.erase( itE );

			// 4J Find the entity again before deleting, as things might have moved in the entity array eg
			// from the explosion created by tnt
			AUTO_VAR(it, find(entities.begin(), entities.end(), e));
			if( it != entities.end() )
			{
				entities.erase(it);
			}

			entityRemoved(e);
		}
		else
		{
			i++;
		}
	}
	LeaveCriticalSection(&m_entitiesCS);

	EnterCriticalSection(&m_tileEntityListCS);

	updatingTileEntities = true;
	for (AUTO_VAR(it, tileEntityList.begin()); it != tileEntityList.end();)
	{
		shared_ptr<TileEntity> te = *it;//tilevector<shared_ptr<Entity> >.at(i);
		if( !te->isRemoved() && te->hasLevel() )
		{
			if (hasChunkAt(te->x, te->y, te->z))
			{
#ifdef _LARGE_WORLDS
				LevelChunk *lc = getChunk(te->x >> 4, te->z >> 4);
				if(!isClientSide || !lc->isUnloaded())
#endif
				{
					te->tick();
				}
			}
		}

		if( te->isRemoved() )
		{
			it = tileEntityList.erase(it);
			if (hasChunk(te->x >> 4, te->z >> 4))
			{
				LevelChunk *lc = getChunk(te->x >> 4, te->z >> 4);
				if (lc != NULL) lc->removeTileEntity(te->x & 15, te->y, te->z & 15);
			}
		}
		else
		{
			it++;
		}
	}
	updatingTileEntities = false;

// 4J-PB - Stuart  - check this is correct here

	if (!tileEntitiesToUnload.empty())
	{	
		//tileEntityList.removeAll(tileEntitiesToUnload);

		for( AUTO_VAR(it, tileEntityList.begin()); it != tileEntityList.end(); )
		{
			bool found = false;
			for( AUTO_VAR(it2, tileEntitiesToUnload.begin()); it2 != tileEntitiesToUnload.end(); it2++ )
			{
				if( (*it) == (*it2) )
				{
					found = true;
					break;
				}
			}
			if( found )
			{
				if(isClientSide)
				{
					__debugbreak();
				}
				it = tileEntityList.erase(it);
			}
			else
			{
				it++;
			}
		}
		tileEntitiesToUnload.clear();
	}

	if( !pendingTileEntities.empty() )
	{
		for( AUTO_VAR(it, pendingTileEntities.begin()); it != pendingTileEntities.end(); it++ )
		{
			shared_ptr<TileEntity> e = *it;
			if( !e->isRemoved() )
			{
				if( find(tileEntityList.begin(),tileEntityList.end(),e) == tileEntityList.end() )
				{
					tileEntityList.push_back(e);
				}
				if (hasChunk(e->x >> 4, e->z >> 4))
				{
					LevelChunk *lc = getChunk(e->x >> 4, e->z >> 4);
					if (lc != NULL) lc->setTileEntity(e->x & 15, e->y, e->z & 15, e);
				}

                sendTileUpdated(e->x, e->y, e->z);
			}
		}
		pendingTileEntities.clear();
	}
	LeaveCriticalSection(&m_tileEntityListCS);
}

void Level::addAllPendingTileEntities(vector< shared_ptr<TileEntity> >& entities)
{
	EnterCriticalSection(&m_tileEntityListCS);
	if( updatingTileEntities )
	{
		for( AUTO_VAR(it, entities.begin()); it != entities.end(); it++ )
		{
			pendingTileEntities.push_back(*it);
		}
	}
	else
	{
		for( AUTO_VAR(it, entities.begin()); it != entities.end(); it++ )
		{
			tileEntityList.push_back(*it);
		}
	}
	LeaveCriticalSection(&m_tileEntityListCS);
}

void Level::tick(shared_ptr<Entity> e)
{
	tick(e, true);
}


void Level::tick(shared_ptr<Entity> e, bool actual)
{
	int xc = Mth::floor(e->x);
	int zc = Mth::floor(e->z);
	int r = 32;
#ifdef __PSVITA__
	// AP - make sure the dragon ticks all the time, even when there aren't any chunks.
	if (actual && e->GetType() != eTYPE_ENDERDRAGON && !hasChunksAt(xc - r, 0, zc - r, xc + r, 0, zc + r))
#else
	if (actual && !hasChunksAt(xc - r, 0, zc - r, xc + r, 0, zc + r))
#endif
	{
		return;
	}

	e->xOld = e->x;
	e->yOld = e->y;
	e->zOld = e->z;
	e->yRotO = e->yRot;
	e->xRotO = e->xRot;

#ifdef __PSVITA__
	// AP - make sure the dragon ticks all the time, even when there aren't any chunks.
	if (actual && (e->GetType() == eTYPE_ENDERDRAGON || e->inChunk) )
#else
	if (actual && e->inChunk )
#endif
	{
		if (e->riding != NULL)
		{
			e->rideTick();
		}
		else
		{
			e->tick();
		}
	}

	// SANTITY!!
	if (Double::isNaN(e->x) || Double::isInfinite(e->x)) e->x = e->xOld;
	if (Double::isNaN(e->y) || Double::isInfinite(e->y)) e->y = e->yOld;
	if (Double::isNaN(e->z) || Double::isInfinite(e->z)) e->z = e->zOld;
	if (Double::isNaN(e->xRot) || Double::isInfinite(e->xRot)) e->xRot = e->xRotO;
	if (Double::isNaN(e->yRot) || Double::isInfinite(e->yRot)) e->yRot = e->yRotO;

	int xcn = Mth::floor(e->x / 16);
	int ycn = Mth::floor(e->y / 16);
	int zcn = Mth::floor(e->z / 16);



	if (!e->inChunk || (e->xChunk != xcn || e->yChunk != ycn || e->zChunk != zcn))
	{
		if (e->inChunk && hasChunk(e->xChunk, e->zChunk))
		{
			getChunk(e->xChunk, e->zChunk)->removeEntity(e, e->yChunk);
		}

		if (hasChunk(xcn, zcn))
		{

			e->inChunk = true;
			MemSect(39);
			getChunk(xcn, zcn)->addEntity(e);
			MemSect(0);
		}
		else
		{
			e->inChunk = false;
			// e.remove();
		}
	}

	if (actual && e->inChunk)
	{
		if (e->rider.lock() != NULL)
		{
			if (e->rider.lock()->removed || e->rider.lock()->riding != e)
			{
				e->rider.lock()->riding = nullptr;
				e->rider = weak_ptr<Entity>();
			}
			else
			{
				tick(e->rider.lock());
			}
		}
	}
}


bool Level::isUnobstructed(AABB *aabb)
{
	return isUnobstructed(aabb, nullptr);
}

bool Level::isUnobstructed(AABB *aabb, shared_ptr<Entity> ignore)
{
	vector<shared_ptr<Entity> > *ents = getEntities(nullptr, aabb);
	AUTO_VAR(itEnd, ents->end());
	for (AUTO_VAR(it, ents->begin()); it != itEnd; it++)
	{
		shared_ptr<Entity> e = *it;
		if (!e->removed && e->blocksBuilding && e != ignore) return false;
	}
	return true;
}


bool Level::containsAnyBlocks(AABB *box)
{
	int x0 = Mth::floor(box->x0);
	int x1 = Mth::floor(box->x1 + 1);
	int y0 = Mth::floor(box->y0);
	int y1 = Mth::floor(box->y1 + 1);
	int z0 = Mth::floor(box->z0);
	int z1 = Mth::floor(box->z1 + 1);

	if (box->x0 < 0) x0--;
	if (box->y0 < 0) y0--;
	if (box->z0 < 0) z0--;

	for (int x = x0; x < x1; x++)
		for (int y = y0; y < y1; y++)
			for (int z = z0; z < z1; z++)
			{
				Tile *tile = Tile::tiles[getTile(x, y, z)];
				if (tile != NULL)
				{
					return true;
				}
			}
			return false;
}


bool Level::containsAnyLiquid(AABB *box)
{
	int x0 = Mth::floor(box->x0);
	int x1 = Mth::floor(box->x1 + 1);
	int y0 = Mth::floor(box->y0);
	int y1 = Mth::floor(box->y1 + 1);
	int z0 = Mth::floor(box->z0);
	int z1 = Mth::floor(box->z1 + 1);

	if (box->x0 < 0) x0--;
	if (box->y0 < 0) y0--;
	if (box->z0 < 0) z0--;

	for (int x = x0; x < x1; x++)
		for (int y = y0; y < y1; y++)
			for (int z = z0; z < z1; z++)
			{
				Tile *tile = Tile::tiles[getTile(x, y, z)];
				if (tile != NULL && tile->material->isLiquid())
				{
					return true;
				}
			}
			return false;
}

// 4J - added this to be used during mob spawning, and it returns true if there's any liquid in the bounding box, or might be because
// we don't have a loaded chunk that we'd need to determine whether it really did. The overall aim is to not load or create any chunk
// we haven't already got, and be cautious about placing the mob's.
bool Level::containsAnyLiquid_NoLoad(AABB *box)
{
	int x0 = Mth::floor(box->x0);
	int x1 = Mth::floor(box->x1 + 1);
	int y0 = Mth::floor(box->y0);
	int y1 = Mth::floor(box->y1 + 1);
	int z0 = Mth::floor(box->z0);
	int z1 = Mth::floor(box->z1 + 1);

	if (box->x0 < 0) x0--;
	if (box->y0 < 0) y0--;
	if (box->z0 < 0) z0--;

	for (int x = x0; x < x1; x++)
		for (int y = y0; y < y1; y++)
			for (int z = z0; z < z1; z++)
			{
				if( !hasChunkAt(x,y,z) ) return true;				// If we don't have it, it might be liquid...
				Tile *tile = Tile::tiles[getTile(x, y, z)];
				if (tile != NULL && tile->material->isLiquid())
				{
					return true;
				}
			}
			return false;
}


bool Level::containsFireTile(AABB *box)
{
	int x0 = Mth::floor(box->x0);
	int x1 = Mth::floor(box->x1 + 1);
	int y0 = Mth::floor(box->y0);
	int y1 = Mth::floor(box->y1 + 1);
	int z0 = Mth::floor(box->z0);
	int z1 = Mth::floor(box->z1 + 1);

	if (hasChunksAt(x0, y0, z0, x1, y1, z1))
	{
		for (int x = x0; x < x1; x++)
			for (int y = y0; y < y1; y++)
				for (int z = z0; z < z1; z++)
				{
					int t = getTile(x, y, z);

					if (t == Tile::fire_Id || t == Tile::lava_Id || t == Tile::calmLava_Id) return true;
				}
	}
	return false;
}


bool Level::checkAndHandleWater(AABB *box, Material *material, shared_ptr<Entity> e)
{
	int x0 = Mth::floor(box->x0);
	int x1 = Mth::floor(box->x1 + 1);

	int y0 = Mth::floor(box->y0);
	int y1 = Mth::floor(box->y1 + 1);

	int z0 = Mth::floor(box->z0);
	int z1 = Mth::floor(box->z1 + 1);

	if (!hasChunksAt(x0, y0, z0, x1, y1, z1))
	{
		return false;
	}

	bool ok = false;
	Vec3 *current = Vec3::newTemp(0, 0, 0);
	for (int x = x0; x < x1; x++)
		for (int y = y0; y < y1; y++)
			for (int z = z0; z < z1; z++)
			{
				Tile *tile = Tile::tiles[getTile(x, y, z)];
				if (tile != NULL && tile->material == material)
				{
					double yt0 = y + 1 - LiquidTile::getHeight(getData(x, y, z));
					if (y1 >= yt0)
					{
						ok = true;
						tile->handleEntityInside(this, x, y, z, e, current);
					}
				}
			}
	if (current->length() > 0)
	{
		current = current->normalize();
		double pow = 0.014;
		e->xd += current->x * pow;
		e->yd += current->y * pow;
		e->zd += current->z * pow;
	}
	return ok;
}


bool Level::containsMaterial(AABB *box, Material *material)
{
	int x0 = Mth::floor(box->x0);
	int x1 = Mth::floor(box->x1 + 1);
	int y0 = Mth::floor(box->y0);
	int y1 = Mth::floor(box->y1 + 1);
	int z0 = Mth::floor(box->z0);
	int z1 = Mth::floor(box->z1 + 1);

	for (int x = x0; x < x1; x++)
		for (int y = y0; y < y1; y++)
			for (int z = z0; z < z1; z++)
			{
				Tile *tile = Tile::tiles[getTile(x, y, z)];
				if (tile != NULL && tile->material == material)
				{
					return true;
				}
			}
	return false;
}


bool Level::containsLiquid(AABB *box, Material *material)
{
	int x0 = Mth::floor(box->x0);
	int x1 = Mth::floor(box->x1 + 1);
	int y0 = Mth::floor(box->y0);
	int y1 = Mth::floor(box->y1 + 1);
	int z0 = Mth::floor(box->z0);
	int z1 = Mth::floor(box->z1 + 1);

	for (int x = x0; x < x1; x++)
		for (int y = y0; y < y1; y++)
			for (int z = z0; z < z1; z++)
			{
				Tile *tile = Tile::tiles[getTile(x, y, z)];
				if (tile != NULL && tile->material == material)
				{
					int data = getData(x, y, z);
					double yh1 = y + 1;
					if (data < 8)
					{
						yh1 = y + 1 - data / 8.0;
					}
					if (yh1 >= box->y0)
					{
						return true;
					}
				}
			}
	return false;
}


shared_ptr<Explosion> Level::explode(shared_ptr<Entity> source, double x, double y, double z, float r, bool destroyBlocks)
{
	return explode(source, x, y, z, r, false, destroyBlocks);
}


shared_ptr<Explosion> Level::explode(shared_ptr<Entity> source, double x, double y, double z, float r, bool fire, bool destroyBlocks)
{
	shared_ptr<Explosion> explosion = shared_ptr<Explosion>( new Explosion(this, source, x, y, z, r) );
	explosion->fire = fire;	
	explosion->destroyBlocks = destroyBlocks;
	explosion->explode();
	explosion->finalizeExplosion(true);
	return explosion;
}


float Level::getSeenPercent(Vec3 *center, AABB *bb)
{
	double xs = 1.0 / ((bb->x1 - bb->x0) * 2 + 1);
	double ys = 1.0 / ((bb->y1 - bb->y0) * 2 + 1);
	double zs = 1.0 / ((bb->z1 - bb->z0) * 2 + 1);
	int hits = 0;
	int count = 0;
	for (double xx = 0; xx <= 1; xx += xs) // 4J Stu - xx, yy and zz were floats, made them doubles to remove warnings
		for (double yy = 0; yy <= 1; yy += ys)
			for (double zz = 0; zz <= 1; zz += zs)
			{
				double x = bb->x0 + (bb->x1 - bb->x0) * xx;
				double y = bb->y0 + (bb->y1 - bb->y0) * yy;
				double z = bb->z0 + (bb->z1 - bb->z0) * zz;
				HitResult *res = clip(Vec3::newTemp(x, y, z), center);
				if ( res == NULL) hits++;
				delete res;
				count++;
			}

	return hits / (float) count;
}


bool Level::extinguishFire(shared_ptr<Player> player, int x, int y, int z, int face)
{
	if (face == 0) y--;
	if (face == 1) y++;
	if (face == 2) z--;
	if (face == 3) z++;
	if (face == 4) x--;
	if (face == 5) x++;

	if (getTile(x, y, z) == Tile::fire_Id)
	{
		levelEvent(player, LevelEvent::SOUND_FIZZ, x, y, z, 0);
		setTile(x, y, z, 0);
		return true;
	}
	return false;
}

/*
shared_ptr<Entity> Level::findSubclassOf(Entity::Class *entityClass)
{
	return shared_ptr<Entity>();
}
*/


wstring Level::gatherStats()
{
	wchar_t buf[64];
	EnterCriticalSection(&m_entitiesCS);
	swprintf(buf,64,L"All:%d",this->entities.size());
	LeaveCriticalSection(&m_entitiesCS);
	return wstring(buf);
}


wstring Level::gatherChunkSourceStats()
{
	return chunkSource->gatherStats();
}


shared_ptr<TileEntity> Level::getTileEntity(int x, int y, int z)
{
	if (y >= Level::maxBuildHeight)
	{
		return nullptr;
	}
	LevelChunk *lc = getChunk(x >> 4, z >> 4);
	if (lc != NULL) return lc->getTileEntity(x & 15, y, z & 15);

    if (lc != NULL)
	{
        shared_ptr<TileEntity> tileEntity = lc->getTileEntity(x & 15, y, z & 15);

        if (tileEntity == NULL)
		{
			EnterCriticalSection(&m_tileEntityListCS);
            for( AUTO_VAR(it, pendingTileEntities.begin()); it != pendingTileEntities.end(); it++ )
			{
				shared_ptr<TileEntity> e = *it;

                if (!e->isRemoved() && e->x == x && e->y == y && e->z == z)
				{
                    tileEntity = e;
                    break;
                }
            }
			LeaveCriticalSection(&m_tileEntityListCS);
        }
        return tileEntity;
    }

	return nullptr;
}


void Level::setTileEntity(int x, int y, int z, shared_ptr<TileEntity> tileEntity)
{
    if (tileEntity != NULL && !tileEntity->isRemoved())
	{
		EnterCriticalSection(&m_tileEntityListCS);
        if (updatingTileEntities)
		{
            tileEntity->x = x;
            tileEntity->y = y;
            tileEntity->z = z;
            pendingTileEntities.push_back(tileEntity);
        }
		else
		{
            tileEntityList.push_back(tileEntity);

			LevelChunk *lc = getChunk(x >> 4, z >> 4);
			if (lc != NULL) lc->setTileEntity(x & 15, y, z & 15, tileEntity);
		}
		LeaveCriticalSection(&m_tileEntityListCS);
	}


}


void Level::removeTileEntity(int x, int y, int z)
{
	EnterCriticalSection(&m_tileEntityListCS);
    shared_ptr<TileEntity> te = getTileEntity(x, y, z);
    if (te != NULL && updatingTileEntities)
	{
        te->setRemoved();
		AUTO_VAR(it, find(pendingTileEntities.begin(), pendingTileEntities.end(), te ));
		if( it != pendingTileEntities.end() )
		{
			pendingTileEntities.erase(it);
		}
    }
	else
	{
        if (te != NULL)
		{
			AUTO_VAR(it, find(pendingTileEntities.begin(), pendingTileEntities.end(), te ));
			if( it != pendingTileEntities.end() )
			{
				pendingTileEntities.erase(it);
			}
			AUTO_VAR(it2, find(tileEntityList.begin(), tileEntityList.end(), te));
			if( it2 != tileEntityList.end() )
			{
				tileEntityList.erase(it2);
			}
        }
		LevelChunk *lc = getChunk(x >> 4, z >> 4);
		if (lc != NULL) lc->removeTileEntity(x & 15, y, z & 15);
	}
	LeaveCriticalSection(&m_tileEntityListCS);
}

void Level::markForRemoval(shared_ptr<TileEntity> entity)
{
	tileEntitiesToUnload.push_back(entity);
}

bool Level::isSolidRenderTile(int x, int y, int z)
{
	Tile *tile = Tile::tiles[getTile(x, y, z)];
	if (tile == NULL) return false;
	
	// 4J - addition here to make rendering big blocks of leaves more efficient. Normally leaves never consider themselves as solid, so
	// blocks of leaves will have all sides of each block completely visible. Changing to consider as solid if this block is surrounded by
	// other leaves (or solid things). This is paired with another change in Tile::getTexture which makes such solid tiles actually visibly solid (these
	// textures exist already for non-fancy graphics). Note: this tile-specific code is here rather than making some new virtual method in the tiles,
	// for the sake of efficiency - I don't imagine we'll be doing much more of this sort of thing

	if( tile->id == Tile::leaves_Id )
	{
		int axo[6] = { 1,-1, 0, 0, 0, 0};
		int ayo[6] = { 0, 0, 1,-1, 0, 0};
		int azo[6] = { 0, 0, 0, 0, 1,-1};
		for( int i = 0; i < 6; i++ )
		{
			int t = getTile(x + axo[i], y + ayo[i] , z + azo[i]);
			if( ( t != Tile::leaves_Id ) && ( ( Tile::tiles[t] == NULL ) || !Tile::tiles[t]->isSolidRender() ) )
			{
				return false;
			}
		}

		return true;
	}
	return tile->isSolidRender(!isClientSide);
}


bool Level::isSolidBlockingTile(int x, int y, int z)
{
	return Tile::isSolidBlockingTile(getTile(x, y, z));
}

/**
    * This method does the same as isSolidBlockingTile, except it will not
    * check the tile if the coordinates is in an unloaded or empty chunk. This
    * is to help vs the problem of "popping" torches in SMP.
	*/

bool Level::isSolidBlockingTileInLoadedChunk(int x, int y, int z, bool valueIfNotLoaded)
{
    if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z >= MAX_LEVEL_SIZE)
	{
        return valueIfNotLoaded;
    }
    LevelChunk *chunk = chunkSource->getChunk(x >> 4, z >> 4);
    if (chunk == NULL || chunk->isEmpty())
	{
        return valueIfNotLoaded;
    }

    Tile *tile = Tile::tiles[getTile(x, y, z)];
    if (tile == NULL) return false;
    return tile->material->isSolidBlocking() && tile->isCubeShaped();
}

// 4J - brought forward from 1.3.2
bool Level::isTopSolidBlocking(int x, int y, int z)
{
    // Temporary workaround until tahgs per-face solidity is finished
    Tile *tile = Tile::tiles[getTile(x, y, z)];
    if (tile == NULL) return false;

    if (tile->material->isSolidBlocking() && tile->isCubeShaped()) return true;
	if (dynamic_cast<StairTile *>(tile) != NULL) 
	{
		return (getData(x, y, z) & StairTile::UPSIDEDOWN_BIT) == StairTile::UPSIDEDOWN_BIT;
	}
    if (dynamic_cast<HalfSlabTile *>(tile) != NULL)
	{
		return (getData(x, y, z) & HalfSlabTile::TOP_SLOT_BIT) == HalfSlabTile::TOP_SLOT_BIT;
	}
	if (dynamic_cast<TopSnowTile *>(tile) != NULL) return (getData(x, y, z) & TopSnowTile::HEIGHT_MASK) == TopSnowTile::MAX_HEIGHT + 1;
    return false;
}

void Level::updateSkyBrightness()
{
	int newDark = this->getOldSkyDarken(1);
	if (newDark != skyDarken)
	{
		skyDarken = newDark;
	}
}

void Level::setSpawnSettings(bool spawnEnemies, bool spawnFriendlies)
{
	this->spawnEnemies = spawnEnemies;
	this->spawnFriendlies = spawnFriendlies;
}

void Level::tick()
{
	PIXBeginNamedEvent(0,"Weather tick");
	tickWeather();
	PIXEndNamedEvent();
}

void Level::prepareWeather()
{
	if (levelData->isRaining())
	{
		this->rainLevel = 1;
		if (levelData->isThundering())
		{
			this->thunderLevel = 1;
		}
	}
}


void Level::tickWeather()
{
	if (dimension->hasCeiling) return;

#ifndef _FINAL_BUILD
	// debug setting added to disable weather
	if(app.DebugSettingsOn())
	{
		if(app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_DisableWeather))
		{
			levelData->setThundering(false);
			levelData->setThunderTime(random->nextInt(TICKS_PER_DAY * 7) + TICKS_PER_DAY / 2);
			levelData->setRaining(false);
			levelData->setRainTime(random->nextInt(TICKS_PER_DAY * 7) + TICKS_PER_DAY / 2);
		}
	}
#endif

	if (lightningTime > 0)
	{
		lightningTime--;
	}

	int thunderTime = levelData->getThunderTime();
	if (thunderTime <= 0)
	{
		if (levelData->isThundering())
		{
			levelData->setThunderTime(random->nextInt(20 * 60 * 10) + 20 * 60 * 3);
		}
		else
		{
			levelData->setThunderTime(random->nextInt(TICKS_PER_DAY * 7) + TICKS_PER_DAY / 2);
		}
	}
	else
	{
		thunderTime--;
		levelData->setThunderTime(thunderTime);
		if (thunderTime <= 0)
		{
			levelData->setThundering(!levelData->isThundering());
		}
	}

	int rainTime = levelData->getRainTime();
	if (rainTime <= 0)
	{
		if (levelData->isRaining())
		{
			levelData->setRainTime(random->nextInt(TICKS_PER_DAY / 2) + TICKS_PER_DAY / 2);
		}
		else
		{
			levelData->setRainTime(random->nextInt(TICKS_PER_DAY * 7) + TICKS_PER_DAY / 2);
		}
	}
	else
	{
		rainTime--;
		levelData->setRainTime(rainTime);
		if (rainTime <= 0)
		{
			levelData->setRaining(!levelData->isRaining());
		}
/*		if( !levelData->isRaining() )
		{
			levelData->setRaining(true);
		}*/
	}

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

void Level::toggleDownfall()
{
    // this will trick the tickWeather method to toggle rain next tick
    levelData->setRainTime(1);
}

void Level::buildAndPrepareChunksToPoll()
{
#if 0	
	AUTO_VAR(itEnd, players.end());
	for (AUTO_VAR(it, players.begin()); it != itEnd; it++)
	{
		shared_ptr<Player> player = *it;
		int xx = Mth::floor(player->x / 16);
		int zz = Mth::floor(player->z / 16);

		int r = CHUNK_POLL_RANGE;
		for (int x = -r; x <= r; x++)
			for (int z = -r; z <= r; z++)
			{
				chunksToPoll.insert(ChunkPos(x + xx, z + zz));
			}
	}
#else
	// 4J - rewritten to add chunks interleaved by player, and to add them from the centre outwards. We're going to be
	// potentially adding less creatures than the original so that our count stays consistent with number of players added, so
	// we want to make sure as best we can that the ones we do add are near the active players
	int playerCount = (int)players.size();
	int *xx = new int[playerCount];
	int *zz = new int[playerCount];
	for (int i = 0; i < playerCount; i++)
	{
		shared_ptr<Player> player = players[i];
		xx[i] = Mth::floor(player->x / 16);
		zz[i] = Mth::floor(player->z / 16);
		chunksToPoll.insert(ChunkPos(xx[i], zz[i] ));
	}

	for( int r = 1; r <= 9; r++ )
	{
		for( int l = 0; l < ( r * 2 ) ; l++ )
		{
			for( int i = 0; i < playerCount; i++ )
			{
				chunksToPoll.insert(ChunkPos( ( xx[i] - r ) + l , ( zz[i] - r )		) );
				chunksToPoll.insert(ChunkPos( ( xx[i] + r )		, ( zz[i] - r ) + l ) );
				chunksToPoll.insert(ChunkPos( ( xx[i] + r ) - l , ( zz[i] + r )		) );
				chunksToPoll.insert(ChunkPos( ( xx[i] - r )		, ( zz[i] + r ) - l ) );
			}
		}
	}
	delete [] xx;
	delete [] zz;
#endif
	
	if (delayUntilNextMoodSound > 0) delayUntilNextMoodSound--;

	// 4J Stu - Added 1.2.3, but not sure if we want to do it
	//util.Timer.push("playerCheckLight");
	//// randomly check areas around the players
	//if (!players.isEmpty()) {
	//	int select = random.nextInt(players.size());
	//	Player player = players.get(select);
	//	int px = Mth.floor(player.x) + random.nextInt(11) - 5;
	//	int py = Mth.floor(player.y) + random.nextInt(11) - 5;
	//	int pz = Mth.floor(player.z) + random.nextInt(11) - 5;
	//	checkLight(px, py, pz);
	//}
	//util.Timer.pop();
}

void Level::tickClientSideTiles(int xo, int zo, LevelChunk *lc)
{
	//lc->tick();	// 4J - brought this lighting update forward from 1.8.2

	if (delayUntilNextMoodSound == 0)
	{
		randValue = randValue * 3 + addend;
		int val = (randValue >> 2);
		int x = (val & 15);
		int z = ((val >> 8) & 15);
		int y = ((val >> 16) & genDepthMinusOne);

		int id = lc->getTile(x, y, z);
		x += xo;
		z += zo;
		if (id == 0 && this->getDaytimeRawBrightness(x, y, z) <= random->nextInt(8) && getBrightness(LightLayer::Sky, x, y, z) <= 0)
		{
			shared_ptr<Player> player = getNearestPlayer(x + 0.5, y + 0.5, z + 0.5, 8);
			if (player != NULL && player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) > 2 * 2)
			{
				// 4J-PB - Fixed issue with cave audio event having 2 sounds at 192k
#ifdef _XBOX
				this->playSound(x + 0.5, y + 0.5, z + 0.5,eSoundType_AMBIENT_CAVE_CAVE2, 0.7f, 0.8f + random->nextFloat() * 0.2f);
#else
				this->playSound(x + 0.5, y + 0.5, z + 0.5,eSoundType_AMBIENT_CAVE_CAVE, 0.7f, 0.8f + random->nextFloat() * 0.2f);
#endif
				delayUntilNextMoodSound = random->nextInt(20 * 60 * 10) + 20 * 60 * 5;
			}
		}
	}

	// 4J Stu - Added 1.2.3, but do we need it?
	//lc->checkNextLight();
}

void Level::tickTiles()
{
	buildAndPrepareChunksToPoll();
}

bool Level::shouldFreezeIgnoreNeighbors(int x, int y, int z)
{
    return shouldFreeze(x, y, z, false);
}

bool Level::shouldFreeze(int x, int y, int z)
{
    return shouldFreeze(x, y, z, true);
}

bool Level::shouldFreeze(int x, int y, int z, bool checkNeighbors)
{
    Biome *biome = getBiome(x, z);
    float temp = biome->getTemperature();
    if (temp > 0.15f) return false;

    if (y >= 0 && y < maxBuildHeight && getBrightness(LightLayer::Block, x, y, z) < 10)
	{
        int current = getTile(x, y, z);
        if ((current == Tile::calmWater_Id || current == Tile::water_Id) && getData(x, y, z) == 0)
		{
            if (!checkNeighbors) return true;

            bool surroundedByWater = true;
            if (surroundedByWater && getMaterial(x - 1, y, z) != Material::water) surroundedByWater = false;
            if (surroundedByWater && getMaterial(x + 1, y, z) != Material::water) surroundedByWater = false;
            if (surroundedByWater && getMaterial(x, y, z - 1) != Material::water) surroundedByWater = false;
            if (surroundedByWater && getMaterial(x, y, z + 1) != Material::water) surroundedByWater = false;
            if (!surroundedByWater) return true;
        }
    }
    return false;
}

bool Level::shouldSnow(int x, int y, int z)
{
    Biome *biome = getBiome(x, z);
    float temp = biome->getTemperature();
    if (temp > 0.15f) return false;


    if (y >= 0 && y < maxBuildHeight && getBrightness(LightLayer::Block, x, y, z) < 10)
	{
        int below = getTile(x, y - 1, z);
        int current = getTile(x, y, z);
        if (current == 0)
		{
            if (Tile::topSnow->mayPlace(this, x, y, z) && (below != 0 && below != Tile::ice_Id && Tile::tiles[below]->material->blocksMotion()))
			{
                return true;
            }
        }
    }

    return false;
}


void Level::checkLight(int x, int y, int z, bool force, bool rootOnlyEmissive)		// 4J added force, rootOnlyEmissive parameters
{
    if (!dimension->hasCeiling) checkLight(LightLayer::Sky, x, y, z, force, false);
    checkLight(LightLayer::Block, x, y, z, force, rootOnlyEmissive);
}
int Level::getExpectedSkyColor(lightCache_t *cache, int oc, int x, int y , int z, int ct, int block)
{
    int expected = 0;

	if( block == 255 ) return 0;		// 4J added as optimisation

    if (canSeeSky(x, y, z))
	{
        expected = 15;
    }
	else
	{
        if (block == 0) block = 1;

		// 4J - changed this to attempt to get all 6 brightnesses of neighbours in a single call, as an optimisation
		int b[6];
		b[0] = getBrightnessCached(cache, LightLayer::Sky, x - 1, y, z);
		b[1] = getBrightnessCached(cache, LightLayer::Sky, x + 1, y, z);
		b[2] = getBrightnessCached(cache, LightLayer::Sky, x, y - 1, z);
		b[3] = getBrightnessCached(cache, LightLayer::Sky, x, y + 1, z);
		b[4] = getBrightnessCached(cache, LightLayer::Sky, x, y, z - 1);
		b[5] = getBrightnessCached(cache, LightLayer::Sky, x, y, z + 1);
		for( int i = 0; i < 6; i++ )
		{
			if( ( b[i] - block ) > expected ) expected = b[i] - block;
		}
    }

    return expected;
}

int Level::getExpectedBlockColor(lightCache_t *cache, int oc, int x, int y, int z, int ct, int block, bool propagatedOnly)
{
    int expected = propagatedOnly ? 0 : getEmissionCached(cache, ct, x, y, z);

	if( block >= 15 ) return expected;	// 4J added as optimisation

	// 4J - changed this to attempt to get all 6 brightnesses of neighbours in a single call, as an optimisation
	int b[6];
	b[0] = getBrightnessCached(cache, LightLayer::Block, x - 1, y, z);
	b[1] = getBrightnessCached(cache, LightLayer::Block, x + 1, y, z);
	b[2] = getBrightnessCached(cache, LightLayer::Block, x, y - 1, z);
	b[3] = getBrightnessCached(cache, LightLayer::Block, x, y + 1, z);
	b[4] = getBrightnessCached(cache, LightLayer::Block, x, y, z - 1);
	b[5] = getBrightnessCached(cache, LightLayer::Block, x, y, z + 1);
	for( int i = 0; i < 6; i++ )
	{
		if( ( b[i] - block ) > expected ) expected = b[i] - block;
	}

    return expected;
}

inline int GetIndex(int x, int y, int z)
{
	return ( ( x & 15 ) << 8 ) | ( ( y & 15 ) << 4 ) | ( z & 15 );
}

// 4J - Made changes here so that lighting goes through a cache, if enabled for this thread
void Level::checkLight(LightLayer::variety layer, int xc, int yc, int zc, bool force, bool rootOnlyEmissive)
{
	lightCache_t *cache = (lightCache_t *)TlsGetValue(tlsIdxLightCache);
	__uint64 cacheUse = 0;

	if( force )
	{
		// 4J - special mode added so we can do lava lighting updates without having all neighbouring chunks loaded in
		if (!hasChunksAt(xc, yc, zc, 0)) return;
	}
	else
	{
		// 4J - this is normal java behaviour
	    if (!hasChunksAt(xc, yc, zc, 17)) return;
	}

#if 0
	/////////////////////////////////////////////////////////////////////////////////////////////
	 // Get the frequency of the timer
	LARGE_INTEGER qwTicksPerSec, qwTime, qwNewTime, qwDeltaTime1, qwDeltaTime2;
	float fElapsedTime1 = 0.0f;
	float fElapsedTime2 = 0.0f;
	QueryPerformanceFrequency( &qwTicksPerSec );
	float fSecsPerTick = 1.0f / (float)qwTicksPerSec.QuadPart;

	QueryPerformanceCounter( &qwTime );
	/////////////////////////////////////////////////////////////////////////////////////////////
#endif

	EnterCriticalSection(&m_checkLightCS);

#ifdef __PSVITA__
	// AP - only clear the one array element required to check if something has changed 
	cachewritten = false;
	if( cache != NULL )
	{
		int idx;
		if( !(yc & 0xffffff00) )
		{
			idx = GetIndex(xc, yc, zc);
			cache[idx] = 0;
			idx = GetIndex(xc - 1, yc, zc);
			cache[idx] = 0;
			idx = GetIndex(xc + 1, yc, zc);
			cache[idx] = 0;
			idx = GetIndex(xc, yc, zc - 1);
			cache[idx] = 0;
			idx = GetIndex(xc, yc, zc + 1);
			cache[idx] = 0;
		}
		if( !((yc-1) & 0xffffff00) )
		{
			idx = GetIndex(xc, yc - 1, zc);
			cache[idx] = 0;
		}
		if( !((yc+1) & 0xffffff00) )
		{
			idx = GetIndex(xc, yc + 1, zc);
			cache[idx] = 0;
		}
	}
#else
	initCache(cache);
#endif

	// If we're in cached mode, then use memory allocated after the cached data itself for the toCheck array, in an attempt to make both that & the other cached data sit on the CPU L2 cache better.
	
	int *toCheck;
	if( cache == NULL )
	{
		toCheck = toCheckLevel;
	}
	else
	{
		toCheck = (int *)(cache + (16*16*16));
	}

    int tcp = 0;
    int tcc = 0;
	//int darktcc = 0;


	// 4J - added
	int minXZ = - (dimension->getXZSize() * 16 ) / 2;
	int maxXZ = (dimension->getXZSize() * 16 ) / 2 - 1;
	if( ( xc > maxXZ ) || ( xc < minXZ ) || ( zc > maxXZ ) || ( zc < minXZ ) )
	{
		LeaveCriticalSection(&m_checkLightCS);
		return;
	}

	// Lock 128K of cache (containing all the lighting cache + first 112K of toCheck array) on L2 to try and stop any cached data getting knocked out of L2 by other non-cached reads (or vice-versa)
//	if( cache ) XLockL2(XLOCKL2_INDEX_TITLE, cache, 128 * 1024, XLOCKL2_LOCK_SIZE_1_WAY, 0 );
	
    {
        int cc = getBrightnessCached(cache, layer, xc, yc, zc);
        int ex = 0;
        {
			int ct = 0;
			int block = getBlockingCached(cache, layer, &ct, xc, yc, zc);
            if (block == 0) block = 1;

            int expected = 0;
            if (layer == LightLayer::Sky)
			{
                expected = getExpectedSkyColor(cache, cc, xc, yc, zc, ct, block);
            }
			else
			{
                expected = getExpectedBlockColor(cache, cc, xc, yc, zc, ct, block, false);
            }

            ex = expected;

        }

#ifdef __PSVITA__
		// AP - we only need to memset the entire array if we discover something has changed
		if( ex != cc && cache )
		{
			lightCache_t old[7];
			if( !(yc & 0xffffff00) )
			{
				old[0] = cache[GetIndex(xc, yc, zc)];
				old[1] = cache[GetIndex(xc - 1, yc, zc)];
				old[2] = cache[GetIndex(xc + 1, yc, zc)];
				old[5] = cache[GetIndex(xc, yc, zc - 1)];
				old[6] = cache[GetIndex(xc, yc, zc + 1)];
			}
			if( !((yc-1) & 0xffffff00) )
			{
				old[3] = cache[GetIndex(xc, yc - 1, zc)];
			}
			if( !((yc+1) & 0xffffff00) )
			{
				old[4] = cache[GetIndex(xc, yc + 1, zc)];
			}

			XMemSet128(cache,0,16*16*16*sizeof(lightCache_t));

			if( !(yc & 0xffffff00) )
			{
				cache[GetIndex(xc, yc, zc)] = old[0];
				cache[GetIndex(xc - 1, yc, zc)] = old[1];
				cache[GetIndex(xc + 1, yc, zc)] = old[2];
				cache[GetIndex(xc, yc, zc - 1)] = old[5];
				cache[GetIndex(xc, yc, zc + 1)] = old[6];
			}
			if( !((yc-1) & 0xffffff00) )
			{
				cache[GetIndex(xc, yc - 1, zc)] = old[3];
			}
			if( !((yc+1) & 0xffffff00) )
			{
				cache[GetIndex(xc, yc + 1, zc)] = old[4];
			}
		}
#endif

        if (ex > cc)
		{
            toCheck[tcc++] = ((32)) + ((32) << 6) + ((32) << 12);
        }
		else if (ex < cc)
		{
			// 4J - added tcn. This is the code that is run when checkLight has been called for a light source that has got darker / turned off.
			// In the original version, after zeroing tiles brightnesses that are deemed to come from this light source, all the zeroed tiles are then passed to the next
			// stage of the function to potentially have their brightnesses put back up again. We shouldn't need to consider All these tiles as starting points for this process, now just
			// considering the edge tiles (defined as a tile where we have a neighbour that is brightner than can be explained by the original light source we are turning off)
			int tcn = 0;
            if (layer == LightLayer::Block || true)
			{
                toCheck[tcc++] = ((32)) + ((32) << 6) + ((32) << 12) + (cc << 18);
                while (tcp < tcc)
				{
                    int p = toCheck[tcp++];
                    int x = ((p) & 63) - 32 + xc;
                    int y = ((p >> 6) & 63) - 32 + yc;
                    int z = ((p >> 12) & 63) - 32 + zc;
                    int cexp = ((p >> 18) & 15);
                    int o = getBrightnessCached(cache, layer, x, y, z);
                    if (o == cexp)
					{
                        setBrightnessCached(cache, &cacheUse, layer, x, y, z, 0);
                        // cexp--;		// 4J - removed, change from 1.2.3
                        if (cexp > 0)
						{
                            int xd = x - xc;
                            int yd = y - yc;
                            int zd = z - zc;
                            if (xd < 0) xd = -xd;
                            if (yd < 0) yd = -yd;
                            if (zd < 0) zd = -zd;
                            if (xd + yd + zd < 17)
							{
								bool edge = false;
                                for (int j = 0; j < 6; j++)
								{
                                    int flip = j % 2 * 2 - 1;

                                    int xx = x + ((j / 2) % 3 / 2) * flip;
                                    int yy = y + ((j / 2 + 1) % 3 / 2) * flip;
                                    int zz = z + ((j / 2 + 2) % 3 / 2) * flip;

									// 4J - added - don't let this lighting creep out of the normal fixed world and into the infinite water chunks beyond
									if( ( xx > maxXZ ) || ( xx < minXZ ) || ( zz > maxXZ ) || ( zz < minXZ ) ) continue;
									if( ( yy < 0 ) || ( yy >= maxBuildHeight ) ) continue;

                                    o = getBrightnessCached(cache, layer, xx, yy, zz);
									// 4J - some changes here brought forward from 1.2.3
									int block = getBlockingCached(cache, layer, NULL, xx, yy, zz);
                                    if (block == 0) block = 1;
                                    if ((o == cexp - block) && (tcc < (32 * 32 * 32))) // 4J - 32 * 32 * 32 was toCheck.length
									{
                                        toCheck[tcc++] = (((xx - xc) + 32)) + (((yy - yc) + 32) << 6) + (((zz - zc) + 32) << 12) + ((cexp - block) << 18);
									}
									else
									{
										// 4J - added - keep track of which tiles form the edge of the region we are zeroing
										if( o > ( cexp - block ) )
										{
											edge = true;
										}
									}
                                }
								// 4J - added - keep track of which tiles form the edge of the region we are zeroing - can store over the original elements in the array because tcn must be <= tcp
								if( edge == true )
								{
									toCheck[tcn++] = p;
								}
                            }
                        }

                    }
                }
            }
			tcp = 0;
//			darktcc = tcc;	///////////////////////////////////////////////////
			tcc = tcn;	// 4J added - we've moved all the edge tiles to the start of the array, so only need to process these now. The original processes all tcc tiles again in the next section
        }
    }

    while (tcp < tcc)
	{
        int p = toCheck[tcp++];
        int x = ((p) & 63) - 32 + xc;
        int y = ((p >> 6) & 63) - 32 + yc;
        int z = ((p >> 12) & 63) - 32 + zc;

		// If force is set, then this is being used to in a special mode to try and light lava tiles as chunks are being loaded in. In this case, we
		// don't want a lighting update to drag in any neighbouring chunks that aren't loaded yet.
		if( force )
		{
			if( !hasChunkAt(x,y,z) )
			{
				continue;
			}
		}

        int c = getBrightnessCached(cache, layer, x, y, z);
		int ct = 0;
		int block = getBlockingCached(cache, layer, &ct, x, y, z);
        if (block == 0) block = 1;

        int expected = 0;
        if (layer == LightLayer::Sky)
		{
            expected = getExpectedSkyColor(cache, c, x, y, z, ct, block);
        }
		else
		{
			// If rootOnlyEmissive flag is set, then only consider the starting tile to be possibly emissive.
			bool propagatedOnly = false;
			if( rootOnlyEmissive )
			{
				propagatedOnly = ( x != xc ) || ( y != yc ) || ( z != zc );
			}
            expected = getExpectedBlockColor(cache, c, x, y, z, ct, block, propagatedOnly);
        }

        if (expected != c)
		{
            setBrightnessCached(cache, &cacheUse, layer, x, y, z, expected);

            if (expected > c)
			{
                int xd = x - xc;
                int yd = y - yc;
                int zd = z - zc;
                if (xd < 0) xd = -xd;
                if (yd < 0) yd = -yd;
                if (zd < 0) zd = -zd;
                if (xd + yd + zd < 17 && tcc < (32 * 32 * 32) - 6)		// 4J - 32 * 32 * 32 was toCheck.length
				{
					// 4J - added extra checks here to stop lighting updates moving out of the actual fixed world and into the infinite water chunks
					if( ( x - 1 ) >= minXZ ) { if (getBrightnessCached(cache, layer, x - 1, y, z) < expected) toCheck[tcc++] = (((x - 1 - xc) + 32)) + (((y - yc) + 32) << 6) + (((z - zc) + 32) << 12); }
					if( ( x + 1 ) <= maxXZ ) { if (getBrightnessCached(cache, layer, x + 1, y, z) < expected) toCheck[tcc++] = (((x + 1 - xc) + 32)) + (((y - yc) + 32) << 6) + (((z - zc) + 32) << 12); }
					if( ( y - 1 ) >= 0 )     { if (getBrightnessCached(cache, layer, x, y - 1, z) < expected) toCheck[tcc++] = (((x - xc) + 32)) + (((y - 1 - yc) + 32) << 6) + (((z - zc) + 32) << 12); }
					if( ( y + 1 ) < maxBuildHeight ) { if (getBrightnessCached(cache, layer, x, y + 1, z) < expected) toCheck[tcc++] = (((x - xc) + 32)) + (((y + 1 - yc) + 32) << 6) + (((z - zc) + 32) << 12); }
					if( ( z - 1 ) >= minXZ ) { if (getBrightnessCached(cache, layer, x, y, z - 1) < expected) toCheck[tcc++] = (((x - xc) + 32)) + (((y - yc) + 32) << 6) + (((z - 1 - zc) + 32) << 12); }
					if( ( z + 1 ) <= maxXZ ) { if (getBrightnessCached(cache, layer, x, y, z + 1) < expected) toCheck[tcc++] = (((x - xc) + 32)) + (((y - yc) + 32) << 6) + (((z + 1 - zc) + 32) << 12); }
                }
            }
        }
    }
//	if( cache ) XUnlockL2(XLOCKL2_INDEX_TITLE);
#if 0
	QueryPerformanceCounter( &qwNewTime );
	qwDeltaTime1.QuadPart = qwNewTime.QuadPart - qwTime.QuadPart;
	qwTime = qwNewTime;
#endif

	flushCache(cache, cacheUse, layer);
#if 0
	/////////////////////////////////////////////////////////////////
	if( cache )
	{
		QueryPerformanceCounter( &qwNewTime );
		qwDeltaTime2.QuadPart = qwNewTime.QuadPart - qwTime.QuadPart;
		fElapsedTime1 = fSecsPerTick * ((FLOAT)(qwDeltaTime1.QuadPart));
		fElapsedTime2 = fSecsPerTick * ((FLOAT)(qwDeltaTime2.QuadPart));
		if( ( darktcc > 0 ) | ( tcc > 0 ) )
		{
			printf("%d %d %d %f + %f = %f\n", darktcc, tcc, darktcc + tcc, fElapsedTime1 * 1000.0f, fElapsedTime2 * 1000.0f, ( fElapsedTime1 + fElapsedTime2 ) * 1000.0f);
		}
	}
	/////////////////////////////////////////////////////////////////
#endif
	LeaveCriticalSection(&m_checkLightCS);
	
}


bool Level::tickPendingTicks(bool force)
{
	return false;
}

vector<TickNextTickData> *Level::fetchTicksInChunk(LevelChunk *chunk, bool remove)
{
	return NULL;
}


vector<shared_ptr<Entity> > *Level::getEntities(shared_ptr<Entity> except, AABB *bb)
{
	MemSect(40);
	es.clear();
	int xc0 = Mth::floor((bb->x0 - 2) / 16);
	int xc1 = Mth::floor((bb->x1 + 2) / 16);
	int zc0 = Mth::floor((bb->z0 - 2) / 16);
	int zc1 = Mth::floor((bb->z1 + 2) / 16);

#ifdef __PSVITA__
#ifdef _ENTITIES_RW_SECTION
	// AP - RW critical sections are expensive so enter it here so we only have to call it once instead of X times
	EnterCriticalRWSection(&LevelChunk::m_csEntities, false);
#else
	EnterCriticalSection(&LevelChunk::m_csEntities);
#endif
#endif

	for (int xc = xc0; xc <= xc1; xc++)
		for (int zc = zc0; zc <= zc1; zc++)
		{
			if (hasChunk(xc, zc))
			{
				getChunk(xc, zc)->getEntities(except, bb, es);
			}
		}
	MemSect(0);

#ifdef __PSVITA__
#ifdef _ENTITIES_RW_SECTION
	LeaveCriticalRWSection(&LevelChunk::m_csEntities, false);
#else
	LeaveCriticalSection(&LevelChunk::m_csEntities);
#endif
#endif

	return &es;
}


vector<shared_ptr<Entity> > *Level::getEntitiesOfClass(const type_info& baseClass, AABB *bb)
{
	int xc0 = Mth::floor((bb->x0 - 2) / 16);
	int xc1 = Mth::floor((bb->x1 + 2) / 16);
	int zc0 = Mth::floor((bb->z0 - 2) / 16);
	int zc1 = Mth::floor((bb->z1 + 2) / 16);
	vector<shared_ptr<Entity> > *es = new vector<shared_ptr<Entity> >();

#ifdef __PSVITA__
#ifdef _ENTITIES_RW_SECTION
	// AP - RW critical sections are expensive so enter it here so we only have to call it once instead of X times
	EnterCriticalRWSection(&LevelChunk::m_csEntities, false);
#else
	EnterCriticalSection(&LevelChunk::m_csEntities);
#endif
#endif

	for (int xc = xc0; xc <= xc1; xc++)
		for (int zc = zc0; zc <= zc1; zc++)
		{
			if (hasChunk(xc, zc))
			{
				getChunk(xc, zc)->getEntitiesOfClass(baseClass, bb, *es);
			}
		}

#ifdef __PSVITA__
#ifdef _ENTITIES_RW_SECTION
	LeaveCriticalRWSection(&LevelChunk::m_csEntities, false);
#else
	LeaveCriticalSection(&LevelChunk::m_csEntities);
#endif
#endif

	return es;
}

shared_ptr<Entity> Level::getClosestEntityOfClass(const type_info& baseClass, AABB *bb, shared_ptr<Entity> source)
{
	vector<shared_ptr<Entity> > *entities = getEntitiesOfClass(baseClass, bb);
	shared_ptr<Entity> closest = nullptr;
	double closestDistSqr = Double::MAX_VALUE;
	//for (Entity entity : entities)
	for(AUTO_VAR(it, entities->begin()); it != entities->end(); ++it)
	{
		shared_ptr<Entity> entity = *it;
		if (entity == source) continue;
		double distSqr = source->distanceToSqr(entity);
		if (distSqr > closestDistSqr) continue;
		closest = entity;
		closestDistSqr = distSqr;
	}
	delete entities;
	return closest;
}

vector<shared_ptr<Entity> > Level::getAllEntities()
{
	EnterCriticalSection(&m_entitiesCS);
	vector<shared_ptr<Entity> > retVec = entities;
	LeaveCriticalSection(&m_entitiesCS);
	return retVec;
}


void Level::tileEntityChanged(int x, int y, int z, shared_ptr<TileEntity> te)
{
	if (this->hasChunkAt(x, y, z))
	{
		getChunkAt(x, z)->markUnsaved();
	}
}

#if 0
unsigned int Level::countInstanceOf(BaseObject::Class *clas)
{
	unsigned int count = 0;
	EnterCriticalSection(&m_entitiesCS);
	AUTO_VAR(itEnd, entities.end());
	for (AUTO_VAR(it, entities.begin()); it != itEnd; it++)
	{
		shared_ptr<Entity> e = *it;//entities.at(i);
		if (clas->isAssignableFrom(e->getClass())) count++;
	}
	LeaveCriticalSection(&m_entitiesCS);

	return count;
}
#endif

// 4J - added - more limited (but faster) version of above, used to count water animals, animals, monsters for the mob spawner
// singleType flag should be true if we are just trying to match eINSTANCEOF exactly, and false if it is a eINSTANCEOF from a group (eTYPE_WATERANIMAL, eTYPE_ANIMAL, eTYPE_MONSTER)
unsigned int Level::countInstanceOf(eINSTANCEOF clas, bool singleType, unsigned int *protectedCount/* = NULL*/, unsigned int *couldWanderCount/* = NULL*/)
{
	unsigned int count = 0;
	if( protectedCount ) *protectedCount = 0;
	if( couldWanderCount ) *couldWanderCount = 0;
	EnterCriticalSection(&m_entitiesCS);
	AUTO_VAR(itEnd, entities.end());
	for (AUTO_VAR(it, entities.begin()); it != itEnd; it++)
	{
		shared_ptr<Entity> e = *it;//entities.at(i);
		if( singleType )
		{
			if (e->GetType() == clas)
			{
				if ( protectedCount && e->isDespawnProtected() )
				{
					(*protectedCount)++;
				}

				if ( couldWanderCount && e->couldWander() )
				{
					(*couldWanderCount)++;
				}

				count++;
			}
		}
		else
		{
			if (e->GetType() & clas) count++;
		}
	}
	LeaveCriticalSection(&m_entitiesCS);

	return count;
}

unsigned int Level::countInstanceOfInRange(eINSTANCEOF clas, bool singleType, int range, int x, int y, int z)
{
	unsigned int count = 0;
	EnterCriticalSection(&m_entitiesCS);
	AUTO_VAR(itEnd, entities.end());
	for (AUTO_VAR(it, entities.begin()); it != itEnd; it++)
	{
		shared_ptr<Entity> e = *it;//entities.at(i);
	
		float sd = e->distanceTo(x,y,z);
		if (sd * sd > range * range)
		{
			continue;
		}

		if( singleType )
		{
			if (e->GetType() == clas)
			{
				count++;
			}
		}
		else
		{
			if (e->GetType() & clas)
			{
				count++;
			}
		}
	}
	LeaveCriticalSection(&m_entitiesCS);

	return count;
}

void Level::addEntities(vector<shared_ptr<Entity> > *list)
{
	//entities.addAll(list);
	EnterCriticalSection(&m_entitiesCS);
	entities.insert(entities.end(), list->begin(), list->end());
	AUTO_VAR(itEnd, list->end());
	bool deleteDragons = false;
	for (AUTO_VAR(it, list->begin()); it != itEnd; it++)
	{
		entityAdded(*it);

		// 4J Stu - Special change to remove duplicate enderdragons that a previous bug might have produced
		if( (*it)->GetType() == eTYPE_ENDERDRAGON)
		{
			deleteDragons = true;
		}
	}

	if(deleteDragons)
	{
		deleteDragons = false;
		for(AUTO_VAR(it, entities.begin()); it != entities.end(); ++it)
		{
			// 4J Stu - Special change to remove duplicate enderdragons that a previous bug might have produced
			if( (*it)->GetType() == eTYPE_ENDERDRAGON)
			{
				if(deleteDragons)
				{
					(*it)->remove();
				}
				else
				{
					deleteDragons = true;
				}
			}
		}
	}
	LeaveCriticalSection(&m_entitiesCS);
}


void Level::removeEntities(vector<shared_ptr<Entity> > *list)
{
	//entitiesToRemove.addAll(list);
	entitiesToRemove.insert(entitiesToRemove.end(), list->begin(), list->end());
}

bool Level::mayPlace(int tileId, int x, int y, int z, bool ignoreEntities, int face, shared_ptr<Entity> ignoreEntity)
{
	int targetType = getTile(x, y, z);
	Tile *targetTile = Tile::tiles[targetType];

	Tile *tile = Tile::tiles[tileId];

	AABB *aabb = tile->getAABB(this, x, y, z);
	if (ignoreEntities) aabb = NULL;
	if (aabb != NULL && !isUnobstructed(aabb, ignoreEntity)) return false;
	if (targetTile != NULL &&
		(targetTile == Tile::water || targetTile == Tile::calmWater || targetTile == Tile::lava ||
		targetTile == Tile::calmLava || targetTile == Tile::fire || targetTile->material->isReplaceable())) targetTile = NULL;
	if (targetTile != NULL && targetTile->material == Material::decoration && tile == Tile::anvil) return true;
	if (tileId > 0 && targetTile == NULL)
	{
		if (tile->mayPlace(this, x, y, z, face))
		{
			return true;
		}
	}
	return false;
}


int Level::getSeaLevel()
{
	return seaLevel;
}


Path *Level::findPath(shared_ptr<Entity> from, shared_ptr<Entity> to, float maxDist, bool canPassDoors, bool canOpenDoors, bool avoidWater, bool canFloat)
{
	int x = Mth::floor(from->x);
	int y = Mth::floor(from->y + 1);
	int z = Mth::floor(from->z);

	int r = (int) (maxDist + 16);
	int x1 = x - r;
	int y1 = y - r;
	int z1 = z - r;
	int x2 = x + r;
	int y2 = y + r;
	int z2 = z + r;
	Region region = Region(this, x1, y1, z1, x2, y2, z2);
	Path *path = (PathFinder(&region, canPassDoors, canOpenDoors, avoidWater, canFloat)).findPath(from.get(), to.get(), maxDist);
	return path;
}


Path *Level::findPath(shared_ptr<Entity> from, int xBest, int yBest, int zBest, float maxDist, bool canPassDoors, bool canOpenDoors, bool avoidWater, bool canFloat)
{
	int x = Mth::floor(from->x);
	int y = Mth::floor(from->y);
	int z = Mth::floor(from->z);

	int r = (int) (maxDist + 8);
	int x1 = x - r;
	int y1 = y - r;
	int z1 = z - r;
	int x2 = x + r;
	int y2 = y + r;
	int z2 = z + r;
	Region region = Region(this, x1, y1, z1, x2, y2, z2);
	Path *path = (PathFinder(&region, canPassDoors, canOpenDoors, avoidWater, canFloat)).findPath(from.get(), xBest, yBest, zBest, maxDist);
	return path;
}


bool Level::getDirectSignal(int x, int y, int z, int dir)
{
	int t = getTile(x, y, z);
	if (t == 0) return false;
	return Tile::tiles[t]->getDirectSignal(this, x, y, z, dir);
}


bool Level::hasDirectSignal(int x, int y, int z)
{
	if (getDirectSignal(x, y - 1, z, 0)) return true;
	if (getDirectSignal(x, y + 1, z, 1)) return true;
	if (getDirectSignal(x, y, z - 1, 2)) return true;
	if (getDirectSignal(x, y, z + 1, 3)) return true;
	if (getDirectSignal(x - 1, y, z, 4)) return true;
	if (getDirectSignal(x + 1, y, z, 5)) return true;
	return false;
}


bool Level::getSignal(int x, int y, int z, int dir)
{
	if (isSolidBlockingTile(x, y, z))
	{
		return hasDirectSignal(x, y, z);
	}
	int t = getTile(x, y, z);
	if (t == 0) return false;
	return Tile::tiles[t]->getSignal(this, x, y, z, dir);
}


bool Level::hasNeighborSignal(int x, int y, int z)
{
	if (getSignal(x, y - 1, z, 0)) return true;
	if (getSignal(x, y + 1, z, 1)) return true;
	if (getSignal(x, y, z - 1, 2)) return true;
	if (getSignal(x, y, z + 1, 3)) return true;
	if (getSignal(x - 1, y, z, 4)) return true;
	if (getSignal(x + 1, y, z, 5)) return true;
	return false;
}

// 4J Stu - Added maxYDist param
shared_ptr<Player> Level::getNearestPlayer(shared_ptr<Entity> source, double maxDist, double maxYDist /*= -1*/)
{
	return getNearestPlayer(source->x, source->y, source->z, maxDist, maxYDist);
}

// 4J Stu - Added maxYDist param
shared_ptr<Player> Level::getNearestPlayer(double x, double y, double z, double maxDist, double maxYDist /*= -1*/)
{
	MemSect(21);
	double best = -1;
	shared_ptr<Player> result = nullptr;
	AUTO_VAR(itEnd, players.end());
	for (AUTO_VAR(it, players.begin()); it != itEnd; it++)
	{
		shared_ptr<Player> p = *it;//players.at(i);
		double dist = p->distanceToSqr(x, y, z);

		// Allow specifying shorter distances in the vertical
		if(maxYDist > 0 && abs(p->y - y) > maxYDist) continue;

		// 4J Stu - Added check that this player is still alive
		if ((maxDist < 0 || dist < maxDist * maxDist) && (best == -1 || dist < best) && p->isAlive() )
		{
			best = dist;
			result = p;
		}
	}
	MemSect(0);
	return result;
}

shared_ptr<Player> Level::getNearestPlayer(double x, double z, double maxDist)
{
	double best = -1;
	shared_ptr<Player> result = nullptr;
	AUTO_VAR(itEnd, players.end());
	for (AUTO_VAR(it, players.begin()); it != itEnd; it++)
	{
		shared_ptr<Player> p = *it;
		double dist = p->distanceToSqr(x, p->y, z);
		if ((maxDist < 0 || dist < maxDist * maxDist) && (best == -1 || dist < best))
		{
			best = dist;
			result = p;
		}
	}
	return result;
}

shared_ptr<Player> Level::getNearestAttackablePlayer(shared_ptr<Entity> source, double maxDist)
{
	return getNearestAttackablePlayer(source->x, source->y, source->z, maxDist);
}

shared_ptr<Player> Level::getNearestAttackablePlayer(double x, double y, double z, double maxDist)
{
    double best = -1;
	
    shared_ptr<Player> result = nullptr;
	AUTO_VAR(itEnd, players.end());
	for (AUTO_VAR(it, players.begin()); it != itEnd; it++)
	{
		shared_ptr<Player> p = *it;

		if (p->abilities.invulnerable)
		{
			continue;
		}

		double dist = p->distanceToSqr(x, y, z);
		double visibleDist = maxDist;

		// decrease the max attackable distance if the target player
		// is sneaking or invisible
        if (p->isSneaking())
		{
            visibleDist *= .8f;
        }
		if (p->isInvisible())
		{
            float coverPercentage = p->getArmorCoverPercentage();
            if (coverPercentage < .1f)
			{
                coverPercentage = .1f;
            }
            visibleDist *= (.7f * coverPercentage);
        }
        
		// 4J Stu - Added check that this player is still alive and privilege check
        if ((visibleDist < 0 || dist < visibleDist * visibleDist) && (best == -1 || dist < best) && p->isAlive() && !p->hasInvisiblePrivilege())
		{
            best = dist;
            result = p;
        }
    }
    return result;
}

shared_ptr<Player> Level::getPlayerByName(const wstring& name)
{
	AUTO_VAR(itEnd, players.end());
	for (AUTO_VAR(it, players.begin()); it != itEnd; it++)
	{
		if (name.compare( (*it)->name) == 0)
		{
			return *it; //players.at(i);
		}
	}
	return shared_ptr<Player>();
}

shared_ptr<Player> Level::getPlayerByUUID(const wstring& name)
{
	AUTO_VAR(itEnd, players.end());
	for (AUTO_VAR(it, players.begin()); it != itEnd; it++)
	{
		if (name.compare( (*it)->getUUID() ) == 0)
		{
			return *it; //players.at(i);
		}
	}
	return shared_ptr<Player>();
}

// 4J Stu - Removed in 1.2.3 ?
byteArray Level::getBlocksAndData(int x, int y, int z, int xs, int ys, int zs, bool includeLighting/* = true*/)
{
	byteArray result( xs * ys * zs * 5 / 2 );
	int xc0 = x >> 4;
	int zc0 = z >> 4;
	int xc1 = (x + xs - 1) >> 4;
	int zc1 = (z + zs - 1) >> 4;

	int p = 0;

	int y0 = y;
	int y1 = y + ys;
	if (y0 < 0) y0 = 0;
	if (y1 > Level::maxBuildHeight) y1 = Level::maxBuildHeight;
	for (int xc = xc0; xc <= xc1; xc++)
	{
		int x0 = x - xc * 16;
		int x1 = x + xs - xc * 16;
		if (x0 < 0) x0 = 0;
		if (x1 > 16) x1 = 16;
		for (int zc = zc0; zc <= zc1; zc++)
		{
			int z0 = z - zc * 16;
			int z1 = z + zs - zc * 16;
			if (z0 < 0) z0 = 0;
			if (z1 > 16) z1 = 16;
			p = getChunk(xc, zc)->getBlocksAndData(&result, x0, y0, z0, x1, y1, z1, p, includeLighting);
		}
	}

	return result;
}

// 4J Stu - Removed in 1.2.3 ?
void Level::setBlocksAndData(int x, int y, int z, int xs, int ys, int zs, byteArray data, bool includeLighting/* = true*/)
{
	int xc0 = x >> 4;
	int zc0 = z >> 4;
	int xc1 = (x + xs - 1) >> 4;
	int zc1 = (z + zs - 1) >> 4;

	int p = 0;

	int y0 = y;
	int y1 = y + ys;
	if (y0 < 0) y0 = 0;
	if (y1 > Level::maxBuildHeight) y1 = Level::maxBuildHeight;
	for (int xc = xc0; xc <= xc1; xc++)
	{
		int x0 = x - xc * 16;
		int x1 = x + xs - xc * 16;
		if (x0 < 0) x0 = 0;
		if (x1 > 16) x1 = 16;
		for (int zc = zc0; zc <= zc1; zc++)
		{
			int z0 = z - zc * 16;
			int z1 = z + zs - zc * 16;
			if (z0 < 0) z0 = 0;
			if (z1 > 16) z1 = 16;
			LevelChunk *lc = getChunk(xc, zc);
			// 4J Stu - Unshare before we make any changes incase the server is already another step ahead of us
			// Fix for #7904 - Gameplay: Players can dupe torches by throwing them repeatedly into water.
			// This is quite expensive so only actually do it if we are hosting, online, and the update will actually
			// change something
			bool forceUnshare = false;
			if( g_NetworkManager.IsHost() && isClientSide )
			{
				forceUnshare = lc->testSetBlocksAndData(data, x0, y0, z0, x1, y1, z1, p);
			}
			if( forceUnshare )
			{
				int size = (x1 - x0 ) * ( y1 - y0 ) * ( z1 - z0 );
				PIXBeginNamedEvent(0,"Chunk data unsharing %d\n", size);
				lc->stopSharingTilesAndData();
				PIXEndNamedEvent();
			}
			if(p < data.length) p = lc->setBlocksAndData(data, x0, y0, z0, x1, y1, z1, p, includeLighting);
			setTilesDirty(xc * 16 + x0, y0, zc * 16 + z0, xc * 16 + x1, y1, zc * 16 + z1);

			PIXBeginNamedEvent(0,"Chunk data sharing\n");
			if( g_NetworkManager.IsHost() && isClientSide )
			{
				lc->startSharingTilesAndData();
			}
			PIXEndNamedEvent();
		}
	}
}


void Level::disconnect(bool sendDisconnect /*= true*/)
{
}


void Level::checkSession()
{
	levelStorage->checkSession();
}


void Level::setTime(__int64 time)
{
	// 4J : WESTY : Added to track game time played by players for other awards.
	if (time != 0) // Ignore setting time to 0, done at level start and during tutorial.
	{
		// Determine step in time and ensure it is reasonable ( we only have an int to store the player stat).
		__int64 timeDiff = time - levelData->getTime();
	
		// debug setting added to keep it at day time
#ifndef _FINAL_BUILD		
		if(app.DebugSettingsOn())
		{
			if(app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_FreezeTime))
			{
				timeDiff=0;
				time=levelData->getTime();
			}
		}
#endif

		if (timeDiff < 0)
		{
			timeDiff = 0;
		}
		else if (timeDiff > 100)
		{
			// Time differences of more than ~5 seconds are generally not real time passing so ignore (moving dimensions does this)
			app.DebugPrintf("Level::setTime: Massive time difference, ignoring for time passed stat (%lli)\n", timeDiff);
			timeDiff = 0;
		}

		// Apply stat to each player.
		if ( timeDiff > 0 && levelData->getTime() != -1 )
		{
			AUTO_VAR(itEnd, players.end());
			for (vector<shared_ptr<Player> >::iterator it = players.begin(); it != itEnd; it++)
			{
				(*it)->awardStat( GenericStats::timePlayed(), GenericStats::param_time(timeDiff) );
			}
		}
	}

	this->levelData->setTime(time);
}

void Level::setOverrideTimeOfDay(__int64 time)
{
	m_timeOfDayOverride = time;
}

__int64 Level::getSeed()
{
	return levelData->getSeed();
}


__int64 Level::getTime()
{
	return levelData->getTime();
}


Pos *Level::getSharedSpawnPos()
{
	return new Pos(levelData->getXSpawn(), levelData->getYSpawn(), levelData->getZSpawn());
}

void Level::setSpawnPos(int x, int y, int z)
{
	levelData->setSpawn(x, y, z);
}

void Level::setSpawnPos(Pos *spawnPos)
{
	setSpawnPos(spawnPos->x, spawnPos->y, spawnPos->z);
}


void Level::ensureAdded(shared_ptr<Entity> entity)
{
	int xc = Mth::floor(entity->x / 16);
	int zc = Mth::floor(entity->z / 16);
	int r = 2;
	for (int x = xc - r; x <= xc + r; x++)
	{
		for (int z = zc - r; z <= zc + r; z++)
		{
			this->getChunk(x, z);
		}
	}

	//if (!entities.contains(entity))
	EnterCriticalSection(&m_entitiesCS);
	if( find(entities.begin(), entities.end(), entity) == entities.end() )
	{
		entities.push_back(entity);
	}
	LeaveCriticalSection(&m_entitiesCS);
}


bool Level::mayInteract(shared_ptr<Player> player, int xt, int yt, int zt, int content)
{
	return true;
}


void Level::broadcastEntityEvent(shared_ptr<Entity> e, byte event)
{
}

ChunkSource *Level::getChunkSource()
{
	return chunkSource;
}


void Level::tileEvent(int x, int y, int z, int tile, int b0, int b1)
{
	if (tile > 0) Tile::tiles[tile]->triggerEvent(this, x, y, z, b0, b1);
}


LevelStorage *Level::getLevelStorage()
{
	return levelStorage.get();
}


LevelData *Level::getLevelData()
{
	return levelData;
}


void Level::updateSleepingPlayerList()
{
}

float Level::getThunderLevel(float a)
{
	return (oThunderLevel + (thunderLevel - oThunderLevel) * a) * getRainLevel(a);
}


float Level::getRainLevel(float a)
{
	return oRainLevel + (rainLevel - oRainLevel) * a;
}


void Level::setRainLevel(float rainLevel)
{
	this->oRainLevel = rainLevel;
	this->rainLevel = rainLevel;
}


bool Level::isThundering()
{
	return getThunderLevel(1) > 0.9;
}


bool Level::isRaining()
{
	return getRainLevel(1) > 0.2;
}


bool Level::isRainingAt(int x, int y, int z)
{
	if (!isRaining()) return false;
	if (!canSeeSky(x, y, z)) return false;
	if (getTopRainBlock(x, z) > y) return false;

// 4J - changed to use new method of getting biomedata that caches results of rain & snow
	if (biomeHasSnow(x, z)) return false;
	return biomeHasRain(x, z);
}

bool Level::isHumidAt(int x, int y, int z)
{
	Biome *biome = getBiome(x, z);
	return biome->isHumid();
}


void Level::setSavedData(const wstring& id, shared_ptr<SavedData> data)
{
	savedDataStorage->set(id, data);
}


shared_ptr<SavedData> Level::getSavedData(const type_info& clazz, const wstring& id)
{
	return savedDataStorage->get(clazz, id);
}


int Level::getFreeAuxValueFor(const wstring& id)
{
	return savedDataStorage->getFreeAuxValueFor(id);
}

// 4J Added
int Level::getAuxValueForMap(PlayerUID xuid, int dimension, int centreXC, int centreZC, int scale)
{
	return savedDataStorage->getAuxValueForMap(xuid, dimension, centreXC, centreZC, scale);
}

// void Level::globalLevelEvent(int type, int sourceX, int sourceY, int sourceZ, int data) 
// {
// 	auto itEnd = listeners.end();
// 	for (auto it = listeners.begin(); it != itEnd; it++)
// 	{
// 		(*it)->globalLevelEvent(type, sourceX, sourceY, sourceZ, data);
// 	}
// }

void Level::levelEvent(int type, int x, int y, int z, int data)
{
	levelEvent(nullptr, type, x, y, z, data);
}


void Level::levelEvent(shared_ptr<Player> source, int type, int x, int y, int z, int data)
{
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
	{
		(*it)->levelEvent(source, type, x, y, z, data);
	}
}

int Level::getMaxBuildHeight()
{
	return maxBuildHeight;
}

int Level::getHeight()
{
	return dimension->hasCeiling ? genDepth : maxBuildHeight;
}

Random *Level::getRandomFor(int x, int z, int blend)
{
	__int64 seed = (x * 341873128712l + z * 132897987541l) + getLevelData()->getSeed() + blend;
	random->setSeed(seed);
	return random;
}

bool Level::updateLights()
{
	return false;
}

TilePos *Level::findNearestMapFeature(const wstring& featureName, int x, int y, int z)
{
    return getChunkSource()->findNearestMapFeature(this, featureName, x, y, z);
}

bool Level::isAllEmpty()
{
	return false;
}

double Level::getHorizonHeight() 
{
	if (levelData->getGenerator() == LevelType::lvl_flat) 
	{
		return 0.0;
	}
	return 63.0;
}

void Level::destroyTileProgress(int id, int x, int y, int z, int progress)
{
	AUTO_VAR(itEnd, listeners.end());
	for (AUTO_VAR(it, listeners.begin()); it != itEnd; it++)
	{
		(*it)->destroyTileProgress(id, x, y, z, progress);
	}
}

bool Level::useNewSeaLevel()
{
	return levelData->useNewSeaLevel();
}

bool Level::getHasBeenInCreative()
{
	return levelData->getHasBeenInCreative();
}

bool Level::isGenerateMapFeatures()
{
	return levelData->isGenerateMapFeatures();
}

int Level::getSaveVersion()
{
	return getLevelStorage()->getSaveFile()->getSaveVersion();
}

int Level::getOriginalSaveVersion()
{
	return getLevelStorage()->getSaveFile()->getOriginalSaveVersion();
}

// 4J - determine if a chunk has been done the post-post-processing stage. This happens when *its* neighbours have each been post-processed, and does some final lighting that can
// only really be done when the post-processing has placed all possible tiles into this chunk.
bool Level::isChunkPostPostProcessed(int x, int z)
{
	if( !hasChunk(x, z) ) return false;		// This will occur for non-loaded chunks, not for edge chunks

	LevelChunk *lc = getChunk(x, z);
	if( lc->isEmpty() ) return true;				// Since we've already eliminated non-loaded chunks, this should only occur for edge chunks. Consider those as fully processed

	return (( lc->terrainPopulated & LevelChunk::sTerrainPostPostProcessed ) == LevelChunk::sTerrainPostPostProcessed);
}

// 4J added - returns true if a chunk is fully, fully finalised - in that it can be sent to another machine. This is the case when all 8 neighbours of this chunk
// have not only been post-processed, but also had the post-post-processing done that they themselves can only do once Their 8 neighbours have been post-processed.
bool Level::isChunkFinalised(int x, int z)
{
	for( int xo = -1; xo <= 1; xo++ )
		for( int zo = -1; zo <= 1; zo++ )
		{
			if( !isChunkPostPostProcessed(x + xo, z + zo) ) return false;
		}

	return true;
}

int Level::getUnsavedChunkCount()
{
	return m_unsavedChunkCount;
}

void Level::incrementUnsavedChunkCount()
{
	++m_unsavedChunkCount;
}

void Level::decrementUnsavedChunkCount()
{
	--m_unsavedChunkCount;
}

bool Level::canCreateMore(eINSTANCEOF type, ESPAWN_TYPE spawnType)
{
	int count = 0;
	int max = 0;
	if(spawnType == eSpawnType_Egg)
	{
		switch(type)
		{
		case eTYPE_VILLAGER:
			count = countInstanceOf( eTYPE_VILLAGER, true);
			max = MobCategory::MAX_XBOX_VILLAGERS_WITH_SPAWN_EGG;
			break;
		case eTYPE_CHICKEN:
			count = countInstanceOf( eTYPE_CHICKEN, true);
			max = MobCategory::MAX_XBOX_CHICKENS_WITH_SPAWN_EGG;
			break;
		case eTYPE_WOLF:
			count = countInstanceOf( eTYPE_WOLF, true);
			max = MobCategory::MAX_XBOX_WOLVES_WITH_SPAWN_EGG;
			break;
		case eTYPE_MUSHROOMCOW:
			count = countInstanceOf( eTYPE_MUSHROOMCOW, true);
			max = MobCategory::MAX_XBOX_MUSHROOMCOWS_WITH_SPAWN_EGG;
			break;
		case eTYPE_SQUID:
			count = countInstanceOf( eTYPE_SQUID, true);
			max = MobCategory::MAX_XBOX_SQUIDS_WITH_SPAWN_EGG;
			break;
		case eTYPE_SNOWMAN:
			count = countInstanceOf( eTYPE_SNOWMAN, true);
			max = MobCategory::MAX_XBOX_SNOWMEN;
			break;
		case eTYPE_VILLAGERGOLEM:
			count = countInstanceOf( eTYPE_VILLAGERGOLEM, true);
			max = MobCategory::MAX_XBOX_IRONGOLEM;
			break;
		default:
			if((type & eTYPE_ANIMALS_SPAWN_LIMIT_CHECK) == eTYPE_ANIMALS_SPAWN_LIMIT_CHECK)
			{
				count = countInstanceOf( eTYPE_ANIMALS_SPAWN_LIMIT_CHECK, false);
				max = MobCategory::MAX_XBOX_ANIMALS_WITH_SPAWN_EGG;
			}
			else if( (type & eTYPE_MONSTER) == eTYPE_MONSTER)
			{
				count = countInstanceOf( eTYPE_MONSTER, false);
				max = MobCategory::MAX_XBOX_MONSTERS_WITH_SPAWN_EGG;
			}
		};
	}
	else if(spawnType == eSpawnType_Breed)
	{
		switch(type)
		{
		case eTYPE_VILLAGER:
			count = countInstanceOf( eTYPE_VILLAGER, true);
			max = MobCategory::MAX_VILLAGERS_WITH_BREEDING;
			break;
		case eTYPE_CHICKEN:
			count = countInstanceOf( eTYPE_CHICKEN, true);
			max = MobCategory::MAX_XBOX_CHICKENS_WITH_BREEDING;
			break;
		case eTYPE_WOLF:
			count = countInstanceOf( eTYPE_WOLF, true);
			max = MobCategory::MAX_XBOX_WOLVES_WITH_BREEDING;
			break;
		case eTYPE_MUSHROOMCOW:
			count = countInstanceOf( eTYPE_MUSHROOMCOW, true);
			max = MobCategory::MAX_XBOX_MUSHROOMCOWS_WITH_BREEDING;
			break;
		default:
			if((type & eTYPE_ANIMALS_SPAWN_LIMIT_CHECK) == eTYPE_ANIMALS_SPAWN_LIMIT_CHECK)
			{
				count = countInstanceOf( eTYPE_ANIMALS_SPAWN_LIMIT_CHECK, false);
				max = MobCategory::MAX_XBOX_ANIMALS_WITH_BREEDING;
			}
			else if( (type & eTYPE_MONSTER) == eTYPE_MONSTER)
			{

			}
			break;
		}
	}
	return count < max;
}