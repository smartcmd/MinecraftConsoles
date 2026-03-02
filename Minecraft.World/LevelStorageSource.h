#pragma once
using namespace std;

#include "stdafx.h"

class LevelSummary;
class ProgressListener;
class LevelData;
class LevelStorage;
class ConsoleSaveFile;

class LevelStorageSource 
{
public:
	virtual wstring getName() = 0;
    virtual shared_ptr<LevelStorage> selectLevel(ConsoleSaveFile *saveFile, const wstring& levelId, bool createPlayerDir) = 0;
    virtual vector<LevelSummary *> *getLevelList() = 0;
    virtual void clearAll() = 0;
    virtual LevelData *getDataTagFor(ConsoleSaveFile *saveFile, const wstring& levelId) = 0;

    /**
     * Tests if a levelId can be used to store a level. For example, a levelId
     * can't be called COM1 on Windows systems, because that is a reserved file
     * handle.
     * <p>
     * Also, a new levelId may not overwrite an existing one.
     * 
     * @param levelId
     * @return
     */
    virtual bool isNewLevelIdAcceptable(const wstring& levelId) = 0;
    virtual void deleteLevel(const wstring& levelId) = 0;
    virtual void renameLevel(const wstring& levelId, const wstring& newLevelName) = 0;
    virtual bool isConvertible(ConsoleSaveFile *saveFile, const wstring& levelId) = 0;
    virtual bool requiresConversion(ConsoleSaveFile *saveFile, const wstring& levelId) = 0;
    virtual bool convertLevel(ConsoleSaveFile *saveFile, const wstring &levelId, ProgressListener *progress) = 0;
};