#include "stdafx.h"
#include "LevelSettings.h"
#include "net.minecraft.world.level.storage.h"
#include "LevelType.h"

GameType *GameType::NOT_SET = NULL;
GameType *GameType::SURVIVAL= NULL;
GameType *GameType::CREATIVE = NULL;
GameType *GameType::ADVENTURE = NULL;

void GameType::staticCtor()
{
	NOT_SET = new GameType(-1, L"");
	SURVIVAL = new GameType(0, L"survival");
	CREATIVE = new GameType(1, L"creative");
	ADVENTURE = new GameType(2, L"adventure");
}

GameType::GameType(int id, const wstring &name)
{
	this->id = id;
	this->name = name;
}

int GameType::getId()
{
	return id;
}

wstring GameType::getName()
{
	return name;
}

void GameType::updatePlayerAbilities(Abilities *abilities)
{
	if (this == CREATIVE)
	{
		abilities->mayfly = true;
		abilities->instabuild = true;
		abilities->invulnerable = true;
	}
	else
	{
		abilities->mayfly = false;
		abilities->instabuild = false;
		abilities->invulnerable = false;
		abilities->flying = false;
	}
	abilities->mayBuild = !isAdventureRestricted();
}

bool GameType::isAdventureRestricted()
{
	return this == ADVENTURE;
}

bool GameType::isCreative()
{
	return this == CREATIVE;
}

bool GameType::isSurvival()
{
	return this == SURVIVAL || this == ADVENTURE;
}

GameType *GameType::byId(int id)
{
	if(id == NOT_SET->id) return NOT_SET;
	else if(id == SURVIVAL->id) return SURVIVAL;
	else if(id == CREATIVE->id) return CREATIVE;
	else if(id == ADVENTURE->id) return ADVENTURE;

	return SURVIVAL;
}

GameType *GameType::byName(const wstring &name)
{
	if(name.compare(NOT_SET->name) == 0) return NOT_SET;
	else if(name.compare(SURVIVAL->name) == 0) return SURVIVAL;
	else if(name.compare(CREATIVE->name) == 0) return CREATIVE;
	else if(name.compare(ADVENTURE->name) == 0) return ADVENTURE;

	return SURVIVAL;
}

void LevelSettings::_init(int64_t seed, GameType *gameType, bool generateMapFeatures, bool hardcore, bool newSeaLevel, LevelType *levelType, int xzSize, int hellScale)
{
	this->seed = seed;
	this->gameType = gameType;
	this->hardcore = hardcore;
	this->generateMapFeatures = generateMapFeatures;
	this->newSeaLevel = newSeaLevel;
	this->levelType = levelType;
	this->allowCommands = false;
	this->startingBonusItems = false;
	levelTypeOptions = L"";
	m_xzSize = xzSize;
	m_hellScale = hellScale;
}

LevelSettings::LevelSettings(int64_t seed, GameType *gameType, bool generateMapFeatures, bool hardcore, bool newSeaLevel, LevelType *levelType, int xzSize, int hellScale) :
	seed(seed),
	gameType(gameType),
	hardcore(hardcore),
	generateMapFeatures(generateMapFeatures),
	newSeaLevel(newSeaLevel),
	levelType(levelType),
	startingBonusItems(false)
{
	_init(seed, gameType, generateMapFeatures, hardcore, newSeaLevel, levelType, xzSize, hellScale);
}

LevelSettings::LevelSettings(LevelData *levelData)
{
	_init(levelData->getSeed(), levelData->getGameType(), levelData->isGenerateMapFeatures(), levelData->isHardcore(), levelData->useNewSeaLevel(), levelData->getGenerator(), levelData->getXZSize(), levelData->getHellScale() );
}

LevelSettings *LevelSettings::enableStartingBonusItems()
{
	startingBonusItems = true;
	return this;
}

LevelSettings *LevelSettings::enableSinglePlayerCommands()
{
	allowCommands = true;
	return this;
}

LevelSettings *LevelSettings::setLevelTypeOptions(const wstring &options)
{
	levelTypeOptions = options;
	return this;
}

bool LevelSettings::hasStartingBonusItems()
{
	return startingBonusItems;
}

int64_t LevelSettings::getSeed()
{
	return seed;
}

GameType *LevelSettings::getGameType()
{
	return gameType;
}

bool LevelSettings::isHardcore()
{
	return hardcore;
}

LevelType *LevelSettings::getLevelType()
{
	return levelType;
}

bool LevelSettings::getAllowCommands()
{
	return allowCommands;
}

bool LevelSettings::isGenerateMapFeatures()
{
	return generateMapFeatures;
}

GameType *LevelSettings::validateGameType(int gameType)
{
	return GameType::byId(gameType);
}

bool LevelSettings::useNewSeaLevel()
{
	return newSeaLevel;
}

// 4J Added
int LevelSettings::getXZSize()
{
	return m_xzSize;
}

int LevelSettings::getHellScale()
{
	return m_hellScale;
}

wstring LevelSettings::getLevelTypeOptions()
{
	return levelTypeOptions;
}