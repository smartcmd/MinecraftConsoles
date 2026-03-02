#pragma once

#include "Entity.h"

class FireworksRocketEntity : public Entity
{
public:
	eINSTANCEOF GetType() { return 	eTYPE_FIREWORKS_ROCKET; }
	static Entity *create(Level *level) { return new FireworksRocketEntity(level); }

private:
	static const int DATA_ID_FIREWORKS_ITEM = 8;

	int life;
	int lifetime;

	// constructor needed for level loader
public:
	FireworksRocketEntity(Level *level);

protected:
	virtual void defineSynchedData();

public:
	virtual bool shouldRenderAtSqrDistance(double distance);

	FireworksRocketEntity(Level *level, double x, double y, double z, shared_ptr<ItemInstance> sourceItem);

	virtual void lerpMotion(double xd, double yd, double zd);
	virtual void tick();
	virtual void handleEntityEvent(byte eventId);
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual float getShadowHeightOffs();
	virtual float getBrightness(float a);
	virtual int getLightColor(float a);
	virtual bool isAttackable();
};