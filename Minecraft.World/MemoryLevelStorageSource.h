#pragma once
using namespace std;

#include "LevelStorageSource.h"

class MemoryLevelStorageSource : public LevelStorageSource 
{
public:
    MemoryLevelStorageSource();
    wstring getName();
    shared_ptr<LevelStorage> selectLevel(const wstring& levelId, bool createPlayerDir);
    vector<LevelSummary *> *getLevelList();
    void clearAll();
    LevelData *getDataTagFor(const wstring& levelId);
    bool isNewLevelIdAcceptable(const wstring& levelId);
    void deleteLevel(const wstring& levelId);
    void renameLevel(const wstring& levelId, const wstring& newLevelName);
    bool isConvertible(const wstring& levelId);
    bool requiresConversion(const wstring& levelId);
    bool convertLevel(const wstring& levelId, ProgressListener *progress);
};