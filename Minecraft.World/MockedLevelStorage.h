#pragma once
using namespace std;

#include "LevelStorage.h"

#include "ConsoleSavePath.h"

class MockedLevelStorage : public LevelStorage 
{
public:
	virtual LevelData *prepareLevel();
	virtual void checkSession();
	virtual ChunkStorage *createChunkStorage(Dimension *dimension);
	virtual void saveLevelData(LevelData *levelData, vector<shared_ptr<Player> > *players);
	virtual void saveLevelData(LevelData *levelData);
	virtual PlayerIO *getPlayerIO();
	virtual void closeAll();
	virtual ConsoleSavePath getDataFile(const wstring& id);
	virtual wstring getLevelId();
public:
	virtual ConsoleSaveFile *getSaveFile() { return NULL; }
};