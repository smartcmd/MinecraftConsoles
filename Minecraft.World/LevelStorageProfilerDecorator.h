#pragma once
using namespace std;

#include "LevelStorage.h"

class ConsoleSaveFile;

class LevelStorageProfilerDecorator : public LevelStorage
{
private:
	 /* final */ LevelStorage *capsulated;

public:
	virtual ConsoleSaveFile *getSaveFile() { return capsulated->getSaveFile(); }

public:
	LevelStorageProfilerDecorator(LevelStorage *capsulated);
    LevelData *prepareLevel();
    void checkSession();
    ChunkStorage *createChunkStorage(Dimension *dimension);
    void saveLevelData(LevelData *levelData, vector<shared_ptr<Player> > *players);
    void saveLevelData(LevelData *levelData);
    PlayerIO *getPlayerIO();
    void closeAll();
    ConsoleSavePath getDataFile(const wstring& id);
	wstring getLevelId();
};