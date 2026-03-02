#pragma once
#include "Entity.h"

class Level;

class FallingTile : public Entity
{
public:
	eINSTANCEOF GetType()			{ return eTYPE_FALLINGTILE;}
	static Entity *create(Level *level) { return new FallingTile(level); }

	static const int serialVersionUID = 0;
	int tile;
	int data;
	int time;
	bool dropItem;

private:
	bool cancelDrop;
	bool hurtEntities;
	int fallDamageMax ;
	float fallDamageAmount;

	// 4J - added for common ctor code
	void _init();

public:
	CompoundTag *tileData;


	FallingTile(Level *level);
	FallingTile(Level *level, double x, double y, double z, int tile, int data = 0);
	~FallingTile();

protected:
	virtual bool makeStepSound();
	virtual void defineSynchedData();

public:
	virtual bool isPickable();
	virtual void tick();

protected:
	void causeFallDamage(float distance);
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

public:
	virtual float getShadowHeightOffs();
	Level *getLevel();
	void setHurtsEntities(bool value);
	bool displayFireAnimation();
};
