#pragma once
using namespace std;

#include "TileEntity.h"
#include "BaseMobSpawner.h"

class Packet;
class Entity;

class MobSpawnerTileEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_MOBSPAWNERTILEENTITY; }
	static TileEntity *create() { return new MobSpawnerTileEntity(); }

private:
	class TileEntityMobSpawner : public BaseMobSpawner
	{
	private:
		MobSpawnerTileEntity *m_parent;

	public:
		TileEntityMobSpawner(MobSpawnerTileEntity *parent);

		void broadcastEvent(int id);
		Level *getLevel();
		int getX();
		int getY();
		int getZ();
		void setNextSpawnData(BaseMobSpawner::SpawnData *nextSpawnData);
	};

	BaseMobSpawner *spawner;

public:
	MobSpawnerTileEntity();
	~MobSpawnerTileEntity();

	virtual void load(CompoundTag *tag);
	virtual void save(CompoundTag *tag);
	virtual void tick();
	virtual shared_ptr<Packet> getUpdatePacket();
	virtual bool triggerEvent(int b0, int b1);
	virtual BaseMobSpawner *getSpawner();

	// 4J Added
	virtual shared_ptr<TileEntity> clone();
	void setEntityId(const wstring &id);
};
