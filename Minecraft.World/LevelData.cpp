#include "stdafx.h"
#include "System.h"
#include "net.minecraft.world.entity.player.h"
#include "com.mojang.nbt.h"
#include "LevelData.h"
#include "LevelType.h"
#include "LevelSettings.h"

LevelData::LevelData()
{
}

LevelData::LevelData(CompoundTag *tag)
{
    seed = tag->getLong(L"RandomSeed");
	m_pGenerator = LevelType::lvl_normal;
	if (tag->contains(L"generatorName")) 
	{
		wstring generatorName = tag->getString(L"generatorName");
		m_pGenerator = LevelType::getLevelType(generatorName);
		if (m_pGenerator == NULL) 
		{
			m_pGenerator = LevelType::lvl_normal;
		} 
		else if (m_pGenerator->hasReplacement()) 
		{
			int generatorVersion = 0;
			if (tag->contains(L"generatorVersion")) 
			{
				generatorVersion = tag->getInt(L"generatorVersion");
			}
			m_pGenerator = m_pGenerator->getReplacementForVersion(generatorVersion);
		}
	}

	gameType = GameType::byId(tag->getInt(L"GameType"));
    if (tag->contains(L"MapFeatures"))
	{
         generateMapFeatures = tag->getBoolean(L"MapFeatures");
    }
	else
	{
		generateMapFeatures = true;
    }
	spawnBonusChest = tag->getBoolean(L"spawnBonusChest");

    xSpawn = tag->getInt(L"SpawnX");
    ySpawn = tag->getInt(L"SpawnY");
    zSpawn = tag->getInt(L"SpawnZ");
    time = tag->getLong(L"Time");
    lastPlayed = tag->getLong(L"LastPlayed");
    sizeOnDisk = tag->getLong(L"SizeOnDisk");
    levelName = tag->getString(L"LevelName");
    version = tag->getInt(L"version");
    rainTime = tag->getInt(L"rainTime");
    raining = tag->getBoolean(L"raining");
    thunderTime = tag->getInt(L"thunderTime");
    thundering = tag->getBoolean(L"thundering");
	hardcore = tag->getBoolean(L"hardcore");

	if (tag->contains(L"initialized"))
	{
		initialized = tag->getBoolean(L"initialized");
	}
	else
	{
		initialized = true;
	}

	if (tag->contains(L"allowCommands"))
	{
		allowCommands = tag->getBoolean(L"allowCommands");
	}
	else
	{
		allowCommands = gameType == GameType::CREATIVE;
	}

	newSeaLevel = tag->getBoolean(L"newSeaLevel"); // 4J added - only use new sea level for newly created maps. This read defaults to false. (sea level changes in 1.8.2)
	hasBeenInCreative = tag->getBoolean(L"hasBeenInCreative"); // 4J added so we can not award achievements to levels modified in creative

	// 4J added - for stronghold position
	bStronghold = tag->getBoolean(L"hasStronghold"); 

	if(bStronghold==false)
	{
		// we need to generate the position
		xStronghold=yStronghold=zStronghold=0;
	}
	else
	{
		xStronghold = tag->getInt(L"StrongholdX");
		yStronghold = tag->getInt(L"StrongholdY");
		zStronghold = tag->getInt(L"StrongholdZ");
	}

	// 4J added - for stronghold end portal position
	bStrongholdEndPortal = tag->getBoolean(L"hasStrongholdEndPortal"); 

	if(bStrongholdEndPortal==false)
	{
		// we need to generate the position
		xStrongholdEndPortal=zStrongholdEndPortal=0;
	}
	else
	{
		xStrongholdEndPortal = tag->getInt(L"StrongholdEndPortalX");
		zStrongholdEndPortal = tag->getInt(L"StrongholdEndPortalZ");
	}

	// 4J Added
	m_xzSize = tag->getInt(L"XZSize");
	m_hellScale = tag->getInt(L"HellScale");
	
	m_xzSize = min(m_xzSize,LEVEL_MAX_WIDTH);
	m_xzSize = max(m_xzSize,LEVEL_MIN_WIDTH);

	m_hellScale = min(m_hellScale,HELL_LEVEL_MAX_SCALE);
	m_hellScale = max(m_hellScale,HELL_LEVEL_MIN_SCALE);

	int hellXZSize = m_xzSize / m_hellScale;
	while(hellXZSize > HELL_LEVEL_MAX_WIDTH && m_hellScale < HELL_LEVEL_MAX_SCALE)
	{
		++m_hellScale;
		hellXZSize = m_xzSize / m_hellScale;
	}

	/* 4J - we don't store this anymore
    if (tag->contains(L"Player")) 
	{
        loadedPlayerTag = tag->getCompound(L"Player");
        dimension = loadedPlayerTag->getInt(L"Dimension");
    }
	else
	{		
		this->loadedPlayerTag = NULL;
	}
	*/
	dimension = 0;
}

LevelData::LevelData(LevelSettings *levelSettings, const wstring& levelName) 
{
    this->seed = levelSettings->getSeed();
    this->gameType = levelSettings->getGameType();
    this->generateMapFeatures = levelSettings->isGenerateMapFeatures();
	this->spawnBonusChest = levelSettings->hasStartingBonusItems();
    this->levelName = levelName;
	this->m_pGenerator = levelSettings->getLevelType();
	this->hardcore = levelSettings->isHardcore();

	// 4J Stu - Default initers	
    this->xSpawn = 0;
    this->ySpawn = 0;
    this->zSpawn = 0;
    this->time = -1;		// 4J-JEV: Edited: To know when this is uninitialized.
    this->lastPlayed = 0;
    this->sizeOnDisk = 0;
//    this->loadedPlayerTag = NULL;	// 4J - we don't store this anymore
    this->dimension = 0;
    this->version = 0;
    this->rainTime = 0;
    this->raining = false;
    this->thunderTime = 0;
    this->thundering = false;
	this->allowCommands = levelSettings->getAllowCommands();
	this->initialized = false;
	this->newSeaLevel = levelSettings->useNewSeaLevel();	// 4J added - only use new sea level for newly created maps (sea level changes in 1.8.2)
	this->hasBeenInCreative = levelSettings->getGameType() == GameType::CREATIVE; // 4J added

	// 4J-PB for the stronghold position
	this->bStronghold=false;
	this->xStronghold = 0;
	this->yStronghold = 0;
	this->zStronghold = 0;

	this->xStrongholdEndPortal = 0;
	this->zStrongholdEndPortal = 0;
	this->bStrongholdEndPortal = false;
	m_xzSize = levelSettings->getXZSize();
	m_hellScale = levelSettings->getHellScale();
	
	m_xzSize = min(m_xzSize,LEVEL_MAX_WIDTH);
	m_xzSize = max(m_xzSize,LEVEL_MIN_WIDTH);

	m_hellScale = min(m_hellScale,HELL_LEVEL_MAX_SCALE);
	m_hellScale = max(m_hellScale,HELL_LEVEL_MIN_SCALE);

	int hellXZSize = m_xzSize / m_hellScale;
	while(hellXZSize > HELL_LEVEL_MAX_WIDTH && m_hellScale < HELL_LEVEL_MAX_SCALE)
	{
		++m_hellScale;
		hellXZSize = m_xzSize / m_hellScale;
}
}

LevelData::LevelData(LevelData *copy)
{
    this->seed = copy->seed;
	this->m_pGenerator = copy->m_pGenerator;
    this->gameType = copy->gameType;
    this->generateMapFeatures = copy->generateMapFeatures;
	this->spawnBonusChest = copy->spawnBonusChest;
    this->xSpawn = copy->xSpawn;
    this->ySpawn = copy->ySpawn;
    this->zSpawn = copy->zSpawn;
    this->time = copy->time;
    this->lastPlayed = copy->lastPlayed;
    this->sizeOnDisk = copy->sizeOnDisk;
//    this->loadedPlayerTag = copy->loadedPlayerTag;		// 4J - we don't store this anymore
    this->dimension = copy->dimension;
    this->levelName = copy->levelName;
    this->version = copy->version;
    this->rainTime = copy->rainTime;
    this->raining = copy->raining;
    this->thunderTime = copy->thunderTime;
    this->thundering = copy->thundering;
	this->hardcore = copy->hardcore;
	this->allowCommands = copy->allowCommands;
	this->initialized = copy->initialized;
	this->newSeaLevel = copy->newSeaLevel;
	this->hasBeenInCreative = copy->hasBeenInCreative;

	// 4J-PB for the stronghold position
	this->bStronghold=copy->bStronghold;
	this->xStronghold = copy->xStronghold;
	this->yStronghold = copy->yStronghold;
	this->zStronghold = copy->zStronghold;

	this->xStrongholdEndPortal = copy->xStrongholdEndPortal;
	this->zStrongholdEndPortal = copy->zStrongholdEndPortal;
	this->bStrongholdEndPortal = copy->bStrongholdEndPortal;
	m_xzSize = copy->m_xzSize;
	m_hellScale = copy->m_hellScale;
}

CompoundTag *LevelData::createTag()
{
    CompoundTag *tag = new CompoundTag();

    setTagData(tag);

    return tag;
}

CompoundTag *LevelData::createTag(vector<shared_ptr<Player> > *players) 
{
	// 4J - removed all code for storing tags for players
	return createTag();
}

void LevelData::setTagData(CompoundTag *tag) 
{
    tag->putLong(L"RandomSeed", seed);
	tag->putString(L"generatorName", m_pGenerator->getGeneratorName());
	tag->putInt(L"generatorVersion", m_pGenerator->getVersion());
    tag->putInt(L"GameType", gameType->getId());
    tag->putBoolean(L"MapFeatures", generateMapFeatures);
	tag->putBoolean(L"spawnBonusChest",spawnBonusChest);
    tag->putInt(L"SpawnX", xSpawn);
    tag->putInt(L"SpawnY", ySpawn);
    tag->putInt(L"SpawnZ", zSpawn);
    tag->putLong(L"Time", time);
    tag->putLong(L"SizeOnDisk", sizeOnDisk);
    tag->putLong(L"LastPlayed", System::currentTimeMillis());
    tag->putString(L"LevelName", levelName);
    tag->putInt(L"version", version);
    tag->putInt(L"rainTime", rainTime);
    tag->putBoolean(L"raining", raining);
    tag->putInt(L"thunderTime", thunderTime);
    tag->putBoolean(L"thundering", thundering);
	tag->putBoolean(L"hardcore", hardcore);
	tag->putBoolean(L"allowCommands", allowCommands);
	tag->putBoolean(L"initialized", initialized);
	tag->putBoolean(L"newSeaLevel", newSeaLevel);
	tag->putBoolean(L"hasBeenInCreative", hasBeenInCreative);
	// store the stronghold position
	tag->putBoolean(L"hasStronghold", bStronghold);
	tag->putInt(L"StrongholdX", xStronghold);
	tag->putInt(L"StrongholdY", yStronghold);
	tag->putInt(L"StrongholdZ", zStronghold);
	// store the stronghold end portal position
	tag->putBoolean(L"hasStrongholdEndPortal", bStrongholdEndPortal);
	tag->putInt(L"StrongholdEndPortalX", xStrongholdEndPortal);
	tag->putInt(L"StrongholdEndPortalZ", zStrongholdEndPortal);
	tag->putInt(L"XZSize", m_xzSize);
	tag->putInt(L"HellScale", m_hellScale);
}

__int64 LevelData::getSeed() 
{
    return seed;
}

int LevelData::getXSpawn() 
{
    return xSpawn;
}

int LevelData::getYSpawn() 
{
    return ySpawn;
}

int LevelData::getZSpawn()
{
    return zSpawn;
}

int LevelData::getXStronghold() 
{
	return xStronghold;
}


int LevelData::getZStronghold()
{
	return zStronghold;
}

int LevelData::getXStrongholdEndPortal() 
{
	return xStrongholdEndPortal;
}


int LevelData::getZStrongholdEndPortal()
{
	return zStrongholdEndPortal;
}

__int64 LevelData::getTime()
{
    return time;
}

__int64 LevelData::getSizeOnDisk()
{
    return sizeOnDisk;
}

CompoundTag *LevelData::getLoadedPlayerTag()
{
    return NULL;	// 4J - we don't store this anymore
}

// 4J Removed TU9 as it's never accurate due to the dimension never being set
//int LevelData::getDimension()
//{
//    return dimension;
//}

void LevelData::setSeed(__int64 seed)
{
    this->seed = seed;
}

void LevelData::setXSpawn(int xSpawn)
{
    this->xSpawn = xSpawn;
}

void LevelData::setYSpawn(int ySpawn) 
{
    this->ySpawn = ySpawn;
}

void LevelData::setZSpawn(int zSpawn)
{
    this->zSpawn = zSpawn;
}

void LevelData::setHasStronghold()
{
	this->bStronghold = true;
}

bool LevelData::getHasStronghold()
{
	return this->bStronghold;
}


void LevelData::setXStronghold(int xStronghold)
{
	this->xStronghold = xStronghold;
}

void LevelData::setZStronghold(int zStronghold)
{
	this->zStronghold = zStronghold;
}

void LevelData::setHasStrongholdEndPortal()
{
	this->bStrongholdEndPortal = true;
}

bool LevelData::getHasStrongholdEndPortal()
{
	return this->bStrongholdEndPortal;
}

void LevelData::setXStrongholdEndPortal(int xStrongholdEndPortal)
{
	this->xStrongholdEndPortal = xStrongholdEndPortal;
}

void LevelData::setZStrongholdEndPortal(int zStrongholdEndPortal)
{
	this->zStrongholdEndPortal = zStrongholdEndPortal;
}

void LevelData::setTime(__int64 time)
{
    this->time = time;
}

void LevelData::setSizeOnDisk(__int64 sizeOnDisk)
{
    this->sizeOnDisk = sizeOnDisk;
}

void LevelData::setLoadedPlayerTag(CompoundTag *loadedPlayerTag)
{
	// 4J - we don't store this anymore
//    this->loadedPlayerTag = loadedPlayerTag;
}

// 4J Remove TU9 as it's never used
//void LevelData::setDimension(int dimension) 
//{
//    this->dimension = dimension;
//}

void LevelData::setSpawn(int xSpawn, int ySpawn, int zSpawn)
{
    this->xSpawn = xSpawn;
    this->ySpawn = ySpawn;
    this->zSpawn = zSpawn;
}

wstring LevelData::getLevelName() 
{
    return levelName;
}

void LevelData::setLevelName(const wstring& levelName)
{
    this->levelName = levelName;
}

int LevelData::getVersion() 
{
    return version;
}

void LevelData::setVersion(int version)
{
    this->version = version;
}

__int64 LevelData::getLastPlayed()
{
    return lastPlayed;
}

bool LevelData::isThundering()
{
    return thundering;
}

void LevelData::setThundering(bool thundering)
{
    this->thundering = thundering;
}

int LevelData::getThunderTime()
{
    return thunderTime;
}

void LevelData::setThunderTime(int thunderTime)
{
    this->thunderTime = thunderTime;
}

bool LevelData::isRaining() 
{
    return raining;
}

void LevelData::setRaining(bool raining)
{
    this->raining = raining;
}

int LevelData::getRainTime()
{
    return rainTime;
}

void LevelData::setRainTime(int rainTime)
{
    this->rainTime = rainTime;
}

GameType *LevelData::getGameType()
{
	return gameType;
}

bool LevelData::isGenerateMapFeatures()
{
	return generateMapFeatures;
}

bool LevelData::getSpawnBonusChest()
{
	return spawnBonusChest;
}

void LevelData::setGameType(GameType *gameType)
{
	this->gameType = gameType;

	// 4J Added
	hasBeenInCreative = hasBeenInCreative || (gameType == GameType::CREATIVE) || app.GetGameHostOption(eGameHostOption_CheatsEnabled) > 0;
}

bool LevelData::useNewSeaLevel()
{
	return newSeaLevel;
}

bool LevelData::getHasBeenInCreative()
{
	return hasBeenInCreative;
}

void LevelData::setHasBeenInCreative(bool value)
{
	hasBeenInCreative = value;
}

LevelType *LevelData::getGenerator() 
{
	return m_pGenerator;
}

void LevelData::setGenerator(LevelType *generator) 
{
	m_pGenerator = generator;
}

bool LevelData::isHardcore()
{
	return hardcore;
}

bool LevelData::getAllowCommands()
{
	return allowCommands;
}

void LevelData::setAllowCommands(bool allowCommands)
{
	this->allowCommands = allowCommands;
}

bool LevelData::isInitialized()
{
	return initialized;
}

void LevelData::setInitialized(bool initialized)
{
	this->initialized = initialized;
}

int LevelData::getXZSize()
{
	return m_xzSize;
}

int LevelData::getHellScale()
{
	return m_hellScale;
}
