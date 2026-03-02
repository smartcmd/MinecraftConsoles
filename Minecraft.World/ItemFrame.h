#pragma once
using namespace std;
#include "Entity.h"
#include "HangingEntity.h"

class Level;

class ItemFrame : public HangingEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_ITEM_FRAME; };
	static Entity *create(Level *level) { return new ItemFrame(level); }
private:
	static const int DATA_ITEM = 2;
	static const int DATA_ROTATION = 3;

private:

	void _init();

public:
	ItemFrame(Level *level);
	ItemFrame(Level *level, int xTile, int yTile, int zTile, int dir);

protected:
	virtual void defineSynchedData();
	virtual int getWidth() {return 9;}
	virtual int getHeight() {return 9;}
	virtual void dropItem();

public:

	shared_ptr<ItemInstance> getItem();
	void setItem(shared_ptr<ItemInstance> item);
	int getRotation();
	void setRotation(int rotation);

	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual bool interact(shared_ptr<Player> player);
};