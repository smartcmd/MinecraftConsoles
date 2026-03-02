#pragma once

#include "HangingEntity.h"

class LeashFenceKnotEntity : public HangingEntity
{
	
public:
	eINSTANCEOF GetType() { return eTYPE_LEASHFENCEKNOT; };
	static Entity *create(Level *level) { return new LeashFenceKnotEntity(level); }

private:

	void _init();

public:
	LeashFenceKnotEntity(Level *level);
	LeashFenceKnotEntity(Level *level, int xTile, int yTile, int zTile);

protected:
	void defineSynchedData();

public:
	void setDir(int dir);
	int getWidth();
	int getHeight();
	bool shouldRenderAtSqrDistance(double distance);
	void dropItem(shared_ptr<Entity> causedBy);
	bool save(CompoundTag *entityTag);
	void addAdditonalSaveData(CompoundTag *tag);
	void readAdditionalSaveData(CompoundTag *tag);
	bool interact(shared_ptr<Player> player);
	virtual bool survives();
	static shared_ptr<LeashFenceKnotEntity> createAndAddKnot(Level *level, int x, int y, int z);
	static shared_ptr<LeashFenceKnotEntity> findKnotAt(Level *level, int x, int y, int z);
};