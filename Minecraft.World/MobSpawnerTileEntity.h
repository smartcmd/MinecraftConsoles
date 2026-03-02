#pragma once
using namespace std;

#include "TileEntity.h"

class Packet;
class Entity;

class MobSpawnerTileEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_MOBSPAWNERTILEENTITY; }
	static TileEntity *create() { return new MobSpawnerTileEntity(); }

using TileEntity::setChanged;

private:
	static const int MAX_DIST;

public:
	int spawnDelay;

private:
	wstring entityId;
	CompoundTag *spawnData;

	bool m_bEntityIdUpdated; // 4J Added

public:
	double spin, oSpin;

private:
	int minSpawnDelay;
	int maxSpawnDelay;
	int spawnCount;
	shared_ptr<Entity> displayEntity;
	
public:
	MobSpawnerTileEntity();

	wstring getEntityId();
	void setEntityId(const wstring& entityId);
	bool isNearPlayer();
	virtual void tick();
	void fillExtraData(shared_ptr<Entity> entity);

private:
	void delay();

public:
	virtual void load(CompoundTag *tag);
	virtual void save(CompoundTag *tag);

	shared_ptr<Entity> getDisplayEntity();
	virtual shared_ptr<Packet> getUpdatePacket();

	// 4J Added
	virtual shared_ptr<TileEntity> clone();
};
