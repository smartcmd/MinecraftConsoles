#pragma once

#include "WeighedRandom.h"

class BaseMobSpawner
{
public:
	class SpawnData : public WeighedRandomItem
	{
	public:
		CompoundTag *tag;
		wstring type;

		SpawnData(CompoundTag *base);
		SpawnData(CompoundTag *tag, wstring type);
		~SpawnData();

		virtual CompoundTag *save();
	};

private:
	static const int EVENT_SPAWN = 1;

public:
	int spawnDelay;

private:
	wstring entityId;
	vector<SpawnData *> *spawnPotentials;
	SpawnData *nextSpawnData;

public:
	double spin, oSpin;

private:
	int minSpawnDelay;
	int maxSpawnDelay;
	int spawnCount;
	shared_ptr<Entity> displayEntity;
	int maxNearbyEntities;
	int requiredPlayerRange;
	int spawnRange;

public:
	BaseMobSpawner();
	~BaseMobSpawner();

	virtual wstring getEntityId();
	virtual void setEntityId(const wstring &entityId);
	virtual bool isNearPlayer();
	virtual void tick();
	virtual shared_ptr<Entity> loadDataAndAddEntity(shared_ptr<Entity> entity);

private:
	virtual void delay();

public:
	virtual void load(CompoundTag *tag);
	virtual void save(CompoundTag *tag);
	virtual shared_ptr<Entity> getDisplayEntity();
	virtual bool onEventTriggered(int id);
	virtual SpawnData *getNextSpawnData();
	virtual void setNextSpawnData(SpawnData *nextSpawnData);

	virtual void broadcastEvent(int id) = 0;
	virtual Level *getLevel() = 0;
	virtual int getX() = 0;
	virtual int getY() = 0;
	virtual int getZ() = 0;
};