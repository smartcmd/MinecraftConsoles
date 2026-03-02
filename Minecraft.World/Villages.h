#pragma once

#include "SavedData.h"

class Villages : public SavedData
{
public:
	static const wstring VILLAGE_FILE_ID;

	static const int MaxDoorDist = 32;

private:
	Level *level;
	deque<Pos *> queries;
	vector<shared_ptr<DoorInfo> > unclustered;
	vector<shared_ptr<Village> > villages;
	int _tick;

public:
	Villages(const wstring &id);
	Villages(Level *level);
	~Villages();

	void setLevel(Level *level);
	void queryUpdateAround(int x, int y, int z);
	void tick();

private:
	void removeVillages();

public:
	vector<shared_ptr<Village> > *getVillages();
	shared_ptr<Village> getClosestVillage(int x, int y, int z, int maxDist);

private:
	void processNextQuery();
	void cluster();
	void addDoorInfos(Pos *pos);
	shared_ptr<DoorInfo>getDoorInfo(int x, int y, int z);
	void createDoorInfo(int x, int y, int z);
	bool hasQuery(int x, int y, int z);
	bool isDoor(int x, int y, int z);

public:
	void load(CompoundTag *tag);
	void save(CompoundTag *tag);
};