#pragma once
using namespace std;

class ConsoleSaveFile;
#include "SavedData.h"

class SavedDataStorage 
{
private:
	LevelStorage *levelStorage;

	typedef unordered_map<wstring, shared_ptr<SavedData> > cacheMapType;
    cacheMapType cache;

    vector<shared_ptr<SavedData> > savedDatas;

	typedef unordered_map<wstring, short> uaiMapType;
    uaiMapType usedAuxIds;

public:
	SavedDataStorage(LevelStorage *);
	shared_ptr<SavedData> get(const type_info& clazz, const wstring& id);
    void set(const wstring& id, shared_ptr<SavedData> data);
    void save();

private:
	void save(shared_ptr<SavedData> data);
    void loadAuxValues();

public:
	int getFreeAuxValueFor(const wstring& id);

	// 4J Added
	int getAuxValueForMap(PlayerUID xuid, int dimension, int centreXC, int centreZC, int scale);
};
