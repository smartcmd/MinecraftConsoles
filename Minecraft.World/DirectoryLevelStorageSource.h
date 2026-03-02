#pragma once
using namespace std;

#include "LevelStorageSource.h"
#include "File.h"

class ProgressListener;
class LevelData;
class ConsoleSaveFile;

class DirectoryLevelStorageSource : public LevelStorageSource 
{
protected:
	const File baseDir;

public:
	DirectoryLevelStorageSource(const File dir);
	virtual wstring getName();
    virtual vector<LevelSummary *> *getLevelList();
    virtual void clearAll();
    virtual LevelData *getDataTagFor(ConsoleSaveFile *saveFile, const wstring& levelId);
    virtual void renameLevel(const wstring& levelId, const wstring& newLevelName);
    virtual bool isNewLevelIdAcceptable(const wstring& levelId);
    virtual void deleteLevel(const wstring& levelId);

protected:
	static void deleteRecursive(vector<File *> *files);

public:
	virtual shared_ptr<LevelStorage> selectLevel(ConsoleSaveFile *saveFile, const wstring& levelId, bool createPlayerDir);
    virtual bool isConvertible(ConsoleSaveFile *saveFile, const wstring& levelId);
    virtual bool requiresConversion(ConsoleSaveFile *saveFile, const wstring& levelId);
    virtual bool convertLevel(ConsoleSaveFile *saveFile, const wstring& levelId, ProgressListener *progress);
};