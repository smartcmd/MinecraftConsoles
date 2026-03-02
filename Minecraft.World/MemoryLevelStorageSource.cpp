#include "stdafx.h"
#include "net.minecraft.world.level.chunk.storage.h"
#include "MemoryLevelStorage.h"
#include "LevelSummary.h"
#include "MemoryLevelStorageSource.h"

MemoryLevelStorageSource::MemoryLevelStorageSource()
{
}

wstring MemoryLevelStorageSource::getName()
{
	return L"Memory Storage";
}

shared_ptr<LevelStorage> MemoryLevelStorageSource::selectLevel(const wstring&  levelId, bool createPlayerDir) 
{
	return shared_ptr<LevelStorage> () new MemoryLevelStorage());
}

vector<LevelSummary *> *MemoryLevelStorageSource::getLevelList() 
{
	return new vector<LevelSummary *>;
}

void MemoryLevelStorageSource::clearAll()
{
}

LevelData *MemoryLevelStorageSource::getDataTagFor(const wstring&  levelId)
{
	return NULL;
}

bool MemoryLevelStorageSource::isNewLevelIdAcceptable(const wstring&  levelId) 
{
	return true;
}

void MemoryLevelStorageSource::deleteLevel(const wstring&  levelId) 
{
}

void MemoryLevelStorageSource::renameLevel(const wstring&  levelId, const wstring&  newLevelName)
{
}

bool MemoryLevelStorageSource::isConvertible(const wstring&  levelId)
{
	return false;
}

bool MemoryLevelStorageSource::requiresConversion(const wstring&  levelId)
{
	return false;
}

bool MemoryLevelStorageSource::convertLevel(const wstring& levelId, ProgressListener *progress)
{
	return false;
}