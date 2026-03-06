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

		if (tag->contains(L"generatorOptions")) generatorOptions = tag->getString(L"generatorOptions");
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
	gameTime = tag->getLong(L"Time");
	if (tag->contains(L"DayTime"))
	{
		dayTime = tag->getLong(L"DayTime");
	}
	else
	{
		dayTime = gameTime;
	}
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

	// 4J: Game rules are now stored with app game host options
	/*if (tag->contains(L"GameRules"))
	{
		gameRules.loadFromTag(tag->getCompound(L"GameRules"));
	}*/

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

#ifdef _LARGE_WORLDS
	m_classicEdgeMoat = tag->getInt(L"ClassicMoat");
	m_smallEdgeMoat = tag->getInt(L"SmallMoat");
	m_mediumEdgeMoat = tag->getInt(L"MediumMoat");

	int newWorldSize = app.GetGameNewWorldSize();
	int newHellScale = app.GetGameNewHellScale();
	m_hellScaleOld = m_hellScale;
	m_xzSizeOld = m_xzSize;
	if(newWorldSize > m_xzSize)
	{
		bool bUseMoat = app.GetGameNewWorldSizeUseMoat();
		switch (m_xzSize)
		{
		case LEVEL_WIDTH_CLASSIC:	m_classicEdgeMoat = bUseMoat;		break;
		case LEVEL_WIDTH_SMALL:		m_smallEdgeMoat = bUseMoat;			break;
		case LEVEL_WIDTH_MEDIUM:	m_mediumEdgeMoat = bUseMoat;		break;
		default: assert(0); 	break;
		}
		assert(newWorldSize > m_xzSize);
		m_xzSize = newWorldSize;
		m_hellScale = newHellScale;
	}
#endif


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

#ifdef _LARGE_WORLDS
	// set the host option, in case it wasn't setup already
	EGameHostOptionWorldSize hostOptionworldSize = e_worldSize_Unknown;
	switch(m_xzSize)
	{
	case LEVEL_WIDTH_CLASSIC:	hostOptionworldSize = e_worldSize_Classic;		break;
	case LEVEL_WIDTH_SMALL:		hostOptionworldSize = e_worldSize_Small;		break;
	case LEVEL_WIDTH_MEDIUM:	hostOptionworldSize = e_worldSize_Medium;		break;
	case LEVEL_WIDTH_LARGE:		hostOptionworldSize = e_worldSize_Large;		break;
	default: assert(0);		break;
	}
	app.SetGameHostOption(eGameHostOption_WorldSize, hostOptionworldSize );
#endif

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
	seed = levelSettings->getSeed();
	gameType = levelSettings->getGameType();
	generateMapFeatures = levelSettings->isGenerateMapFeatures();
	spawnBonusChest = levelSettings->hasStartingBonusItems();
	this->levelName = levelName;
	m_pGenerator = levelSettings->getLevelType();
	hardcore = levelSettings->isHardcore();
	generatorOptions = levelSettings->getLevelTypeOptions();
	allowCommands = levelSettings->getAllowCommands();

	// 4J Stu - Default initers
	xSpawn = 0;
	ySpawn = 0;
	zSpawn = 0;
	dayTime = -1;		// 4J-JEV: Edited: To know when this is uninitialized.
	gameTime = -1;
	lastPlayed = 0;
	sizeOnDisk = 0;
	//    this->loadedPlayerTag = NULL;	// 4J - we don't store this anymore
	dimension = 0;
	version = 0;
	rainTime = 0;
	raining = false;
	thunderTime = 0;
	thundering = false;
	initialized = false;
	newSeaLevel = levelSettings->useNewSeaLevel();	// 4J added - only use new sea level for newly created maps (sea level changes in 1.8.2)
	hasBeenInCreative = levelSettings->getGameType() == GameType::CREATIVE; // 4J added

	// 4J-PB for the stronghold position
	bStronghold=false;
	xStronghold = 0;
	yStronghold = 0;
	zStronghold = 0;

	xStrongholdEndPortal = 0;
	zStrongholdEndPortal = 0;
	bStrongholdEndPortal = false;
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
#ifdef _LARGE_WORLDS
	m_hellScaleOld = m_hellScale;
	m_xzSizeOld = m_xzSize;
	m_classicEdgeMoat = false;
	m_smallEdgeMoat = false;
	m_mediumEdgeMoat = false;
#endif

}

LevelData::LevelData(LevelData *copy)
{
	seed = copy->seed;
	m_pGenerator = copy->m_pGenerator;
	generatorOptions = copy->generatorOptions;
	gameType = copy->gameType;
	generateMapFeatures = copy->generateMapFeatures;
	spawnBonusChest = copy->spawnBonusChest;
	xSpawn = copy->xSpawn;
	ySpawn = copy->ySpawn;
	zSpawn = copy->zSpawn;
	gameTime = copy->gameTime;
	dayTime = copy->dayTime;
	lastPlayed = copy->lastPlayed;
	sizeOnDisk = copy->sizeOnDisk;
	//    this->loadedPlayerTag = copy->loadedPlayerTag;		// 4J - we don't store this anymore
	dimension = copy->dimension;
	levelName = copy->levelName;
	version = copy->version;
	rainTime = copy->rainTime;
	raining = copy->raining;
	thunderTime = copy->thunderTime;
	thundering = copy->thundering;
	hardcore = copy->hardcore;
	allowCommands = copy->allowCommands;
	initialized = copy->initialized;
	newSeaLevel = copy->newSeaLevel;
	hasBeenInCreative = copy->hasBeenInCreative;
	gameRules = copy->gameRules;

	// 4J-PB for the stronghold position
	bStronghold=copy->bStronghold;
	xStronghold = copy->xStronghold;
	yStronghold = copy->yStronghold;
	zStronghold = copy->zStronghold;

	xStrongholdEndPortal = copy->xStrongholdEndPortal;
	zStrongholdEndPortal = copy->zStrongholdEndPortal;
	bStrongholdEndPortal = copy->bStrongholdEndPortal;
	m_xzSize = copy->m_xzSize;
	m_hellScale = copy->m_hellScale;
#ifdef _LARGE_WORLDS
	m_classicEdgeMoat = copy->m_classicEdgeMoat;
	m_smallEdgeMoat = copy->m_smallEdgeMoat;
	m_mediumEdgeMoat = copy->m_mediumEdgeMoat;
	m_xzSizeOld = copy->m_xzSizeOld;
	m_hellScaleOld = copy->m_hellScaleOld;
#endif
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
	tag->putString(L"generatorOptions", generatorOptions);
	tag->putInt(L"GameType", gameType->getId());
	tag->putBoolean(L"MapFeatures", generateMapFeatures);
	tag->putBoolean(L"spawnBonusChest",spawnBonusChest);
	tag->putInt(L"SpawnX", xSpawn);
	tag->putInt(L"SpawnY", ySpawn);
	tag->putInt(L"SpawnZ", zSpawn);
	tag->putLong(L"Time", gameTime);
	tag->putLong(L"DayTime", dayTime);
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
	// 4J: Game rules are now stored with app game host options
	//tag->putCompound(L"GameRules", gameRules.createTag());
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
#ifdef _LARGE_WORLDS
	tag->putInt(L"ClassicMoat", m_classicEdgeMoat);
	tag->putInt(L"SmallMoat", m_smallEdgeMoat);
	tag->putInt(L"MediumMoat", m_mediumEdgeMoat);
#endif

	tag->putInt(L"HellScale", m_hellScale);
}

int64_t LevelData::getSeed()
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

int64_t LevelData::getGameTime()
{
	return gameTime;
}

int64_t LevelData::getDayTime()
{
	return dayTime;
}

int64_t LevelData::getSizeOnDisk()
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

void LevelData::setSeed(int64_t seed)
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

void LevelData::setGameTime(int64_t time)
{
	gameTime = time;
}

void LevelData::setDayTime(int64_t time)
{
	dayTime = time;
}

void LevelData::setSizeOnDisk(int64_t sizeOnDisk)
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

int64_t LevelData::getLastPlayed()
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
	hasBeenInCreative =		hasBeenInCreative ||
							(gameType == GameType::CREATIVE) ||
							(app.GetGameHostOption(eGameHostOption_CheatsEnabled) > 0);
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

wstring LevelData::getGeneratorOptions()
{
	return generatorOptions;
}

void LevelData::setGeneratorOptions(const wstring &options)
{
	generatorOptions = options;
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

GameRules *LevelData::getGameRules()
{
	return &gameRules;
}

int LevelData::getXZSize()
{
	return m_xzSize;
}

#ifdef _LARGE_WORLDS
int LevelData::getXZSizeOld()
{
	return m_xzSizeOld;
}

void LevelData::getMoatFlags(bool* bClassicEdgeMoat, bool* bSmallEdgeMoat, bool* bMediumEdgeMoat)
{
	*bClassicEdgeMoat = m_classicEdgeMoat;
	*bSmallEdgeMoat = m_smallEdgeMoat;
	*bMediumEdgeMoat = m_mediumEdgeMoat;

}

int LevelData::getXZHellSizeOld()
{
	int hellXZSizeOld = ceil((float)m_xzSizeOld / m_hellScaleOld);

	while(hellXZSizeOld > HELL_LEVEL_MAX_WIDTH && m_hellScaleOld < HELL_LEVEL_MAX_SCALE)
	{
		assert(0); // should never get in here?
		++m_hellScaleOld;
		hellXZSizeOld = m_xzSize / m_hellScale;
	}

	return hellXZSizeOld;
}


#endif

int LevelData::getHellScale()
{
	return m_hellScale;
}
