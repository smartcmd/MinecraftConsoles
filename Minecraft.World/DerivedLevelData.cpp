#include "stdafx.h"

#include "DerivedLevelData.h"

DerivedLevelData::DerivedLevelData(LevelData *wrapped)
{
	this->wrapped = wrapped;
}

void DerivedLevelData::setTagData(CompoundTag *tag)
{
	wrapped->setTagData(tag);
}

CompoundTag *DerivedLevelData::createTag()
{
	return wrapped->createTag();
}

CompoundTag *DerivedLevelData::createTag(vector<shared_ptr<Player> > *players)
{
	return wrapped->createTag(players);
}

int64_t DerivedLevelData::getSeed()
{
	return wrapped->getSeed();
}

int DerivedLevelData::getXSpawn()
{
	return wrapped->getXSpawn();
}


int DerivedLevelData::getYSpawn()
{
	return wrapped->getYSpawn();
}

int DerivedLevelData::getZSpawn()
{
	return wrapped->getZSpawn();
}

int64_t DerivedLevelData::getGameTime()
{
	return wrapped->getGameTime();
}

int64_t DerivedLevelData::getDayTime()
{
	return wrapped->getDayTime();
}

int64_t DerivedLevelData::getSizeOnDisk()
{
	return wrapped->getSizeOnDisk();
}

CompoundTag *DerivedLevelData::getLoadedPlayerTag()
{
	return wrapped->getLoadedPlayerTag();
}

wstring DerivedLevelData::getLevelName()
{
	return wrapped->getLevelName();
}

int DerivedLevelData::getVersion()
{
	return wrapped->getVersion();
}

int64_t DerivedLevelData::getLastPlayed()
{
	return wrapped->getLastPlayed();
}

bool DerivedLevelData::isThundering()
{
	return wrapped->isThundering();
}

int DerivedLevelData::getThunderTime()
{
	return wrapped->getThunderTime();
}

bool DerivedLevelData::isRaining()
{
	return wrapped->isRaining();
}

int DerivedLevelData::getRainTime()
{
	return wrapped->getRainTime();
}

GameType *DerivedLevelData::getGameType()
{
	return wrapped->getGameType();
}

void DerivedLevelData::setSeed(int64_t seed)
{
}

void DerivedLevelData::setXSpawn(int xSpawn)
{
}

void DerivedLevelData::setYSpawn(int ySpawn)
{
}

void DerivedLevelData::setZSpawn(int zSpawn)
{
}

void DerivedLevelData::setGameTime(int64_t time)
{
}

void DerivedLevelData::setDayTime(int64_t time)
{
}

void DerivedLevelData::setSizeOnDisk(int64_t sizeOnDisk)
{
}

void DerivedLevelData::setLoadedPlayerTag(CompoundTag *loadedPlayerTag)
{
}

void DerivedLevelData::setDimension(int dimension)
{
}

void DerivedLevelData::setSpawn(int xSpawn, int ySpawn, int zSpawn)
{
}

void DerivedLevelData::setLevelName(const wstring &levelName)
{
}

void DerivedLevelData::setVersion(int version)
{
}

void DerivedLevelData::setThundering(bool thundering)
{
}

void DerivedLevelData::setThunderTime(int thunderTime)
{
}

void DerivedLevelData::setRaining(bool raining)
{
}

void DerivedLevelData::setRainTime(int rainTime)
{
}

bool DerivedLevelData::isGenerateMapFeatures()
{
	return wrapped->isGenerateMapFeatures();
}

void DerivedLevelData::setGameType(GameType *gameType) {
}

bool DerivedLevelData::isHardcore()
{
	return wrapped->isHardcore();
}

LevelType *DerivedLevelData::getGenerator()
{
	return wrapped->getGenerator();
}

void DerivedLevelData::setGenerator(LevelType *generator)
{
}

bool DerivedLevelData::getAllowCommands()
{
	return wrapped->getAllowCommands();
}

void DerivedLevelData::setAllowCommands(bool allowCommands)
{
}

bool DerivedLevelData::isInitialized()
{
	return wrapped->isInitialized();
}

void DerivedLevelData::setInitialized(bool initialized)
{
}

GameRules *DerivedLevelData::getGameRules()
{
	return wrapped->getGameRules();
}

int DerivedLevelData::getXZSize()
{
	return wrapped->getXZSize();
}

int DerivedLevelData::getHellScale()
{
	return wrapped->getHellScale();
}
