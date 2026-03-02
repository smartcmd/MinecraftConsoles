#pragma once
using namespace std;

#include "Monster.h"
#include "MobGroupData.h"

class Spider : public Monster
{
public:
	eINSTANCEOF GetType() { return eTYPE_SPIDER; }
	static Entity *create(Level *level) { return new Spider(level); }

private:
	static const int DATA_FLAGS_ID = 16;

public:
	Spider(Level *level);

protected:
	virtual void defineSynchedData();

public:
	virtual void tick();

protected:
	virtual void registerAttributes();
	virtual shared_ptr<Entity> findAttackTarget();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual void playStepSound(int xt, int yt, int zt, int t);
	virtual void checkHurtTarget(shared_ptr<Entity> target, float d);
	virtual int getDeathLoot();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual bool onLadder();

	virtual void makeStuckInWeb();
	virtual MobType getMobType();
	virtual bool canBeAffected(MobEffectInstance *newEffect);
	virtual bool isClimbing();
	virtual void setClimbing(bool value);
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0); // 4J Added extraData param

private:
	static const float SPIDER_SPECIAL_EFFECT_CHANCE;

public:
	class SpiderEffectsGroupData : public MobGroupData
	{
	public:
		int effectId;

		SpiderEffectsGroupData();
		void setRandomEffect(Random *random);
	};
};
