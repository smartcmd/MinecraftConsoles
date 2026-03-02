#include "stdafx.h"
#include "DerivedServerLevel.h"
#include "..\Minecraft.World\SavedDataStorage.h"
#include "..\Minecraft.World\DerivedLevelData.h"

DerivedServerLevel::DerivedServerLevel(MinecraftServer *server, shared_ptr<LevelStorage> levelStorage, const wstring& levelName, int dimension, LevelSettings *levelSettings, ServerLevel *wrapped)
	: ServerLevel(server, levelStorage, levelName, dimension, levelSettings)
{
	// 4J-PB - we're going to override the savedDataStorage, so we need to delete the current one
	if(this->savedDataStorage)
	{
		delete this->savedDataStorage;
		this->savedDataStorage=NULL;
	}
	this->savedDataStorage = wrapped->savedDataStorage;
	levelData = new DerivedLevelData(wrapped->getLevelData());
}

DerivedServerLevel::~DerivedServerLevel()
{
	// we didn't allocate savedDataStorage here, so we don't want the level destructor to delete it
	this->savedDataStorage=NULL;
}

void DerivedServerLevel::saveLevelData()
{
	// Do nothing?
	// Do nothing!
}