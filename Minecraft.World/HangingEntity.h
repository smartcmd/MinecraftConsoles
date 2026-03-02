
#pragma once

#include "Entity.h"

class HangingEntity : public Entity
{
public:
	eINSTANCEOF GetType() { return eTYPE_HANGING_ENTITY; }
private:

	void _init(Level *level);
	float offs(int w);

	int checkInterval;
	//eINSTANCEOF eType;

protected: 
	virtual void defineSynchedData() {};

public:
	int dir;
	int xTile, yTile, zTile;

	HangingEntity(Level *level);
	HangingEntity(Level *level, int xTile, int yTile, int zTile, int dir);
	void setDir(int dir);
	bool survives();

	virtual void tick();
	virtual bool isPickable();
	virtual bool skipAttackInteraction(shared_ptr<Entity> source);
	virtual bool hurt(DamageSource *source, int damage);
	virtual void move(double xa, double ya, double za, bool noEntityCubes=false);	// 4J - added noEntityCubes parameter
	virtual void push(double xa, double ya, double za);
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

	virtual int getWidth()=0;
	virtual int getHeight()=0;
	virtual void dropItem()=0;
};
