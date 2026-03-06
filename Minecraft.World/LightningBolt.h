#pragma once
using namespace std;
#include "GlobalEntity.h"
#include "net.minecraft.world.level.tile.h"


//class LightningBolt : public GlobalEntity
class LightningBolt : public GlobalEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_LIGHTNINGBOLT; }

private:
	static const int START_LIFE = 2;

	int life;

public:
	int64_t seed;

private:
	int flashes;

public:
	LightningBolt(Level *level, double x, double y, double z);
	virtual void tick();

protected:
	virtual void defineSynchedData();
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void addAdditonalSaveData(CompoundTag *tag);

public:
	bool shouldAlwaysRender();
	virtual bool shouldRender(Vec3 *c);
};