#pragma once

#include "Minecart.h"
#include "BaseMobSpawner.h"

class MinecartSpawner : public Minecart
{
public:
	eINSTANCEOF GetType() { return eTYPE_MINECART_SPAWNER; };
	static Entity *create(Level *level) { return new MinecartSpawner(level); }

private:
	BaseMobSpawner *spawner;

	class MinecartMobSpawner : public BaseMobSpawner
	{
	private:
		MinecartSpawner *m_parent;

	public:
		MinecartMobSpawner(MinecartSpawner *parent);
		void broadcastEvent(int id);
		Level *getLevel();
		int getX();
		int getY();
		int getZ();
	};

public:
	MinecartSpawner(Level *level);
	MinecartSpawner(Level *level, double x, double y, double z);
	virtual ~MinecartSpawner();

	virtual int getType();
	virtual Tile *getDefaultDisplayTile();

protected:
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void addAdditonalSaveData(CompoundTag *tag);

public:
	virtual void handleEntityEvent(byte eventId);
	virtual void tick();
	virtual BaseMobSpawner *getSpawner();
};