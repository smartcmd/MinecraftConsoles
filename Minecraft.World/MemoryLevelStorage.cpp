#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.chunk.storage.h"
#include "net.minecraft.world.level.dimension.h"
#include "com.mojang.nbt.h"
#include "MemoryLevelStorage.h"

#include "ConsoleSaveFileIO.h"

MemoryLevelStorage::MemoryLevelStorage()
{
}

LevelData *MemoryLevelStorage::prepareLevel()
{
	return NULL;
}

void MemoryLevelStorage::checkSession() 
{
}

ChunkStorage *MemoryLevelStorage::createChunkStorage(Dimension *dimension)
{
	return new MemoryChunkStorage();
}

void MemoryLevelStorage::saveLevelData(LevelData *levelData, vector<shared_ptr<Player> > *players) 
{
}

void MemoryLevelStorage::saveLevelData(LevelData *levelData)
{
}

PlayerIO *MemoryLevelStorage::getPlayerIO()
{
	return this;
}

void MemoryLevelStorage::closeAll()
{
}

void MemoryLevelStorage::save(shared_ptr<Player> player) 
{
}

bool MemoryLevelStorage::load(shared_ptr<Player> player)
{
	return false;
}

CompoundTag *MemoryLevelStorage::loadPlayerDataTag(const wstring& playerName) 
{
	return NULL;
}

ConsoleSavePath MemoryLevelStorage::getDataFile(const wstring& id) 
{
	return ConsoleSaveFile(wstring(L""));
}