#pragma once
#include "ServerLevel.h"

class DerivedServerLevel : public ServerLevel
{
public:
	DerivedServerLevel(MinecraftServer *server, shared_ptr<LevelStorage>levelStorage, const wstring& levelName, int dimension, LevelSettings *levelSettings, ServerLevel *wrapped);
	~DerivedServerLevel();

protected:
	void saveLevelData();
};