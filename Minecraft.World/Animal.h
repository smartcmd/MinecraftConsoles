#pragma once
#include "AgableMob.h"
#include "Creature.h"

class Level;
class CompoundTag;
class DamageSource;

class Animal : public AgableMob, public Creature
{
private:
	static const int DATA_IN_LOVE = 13;	// 4J added

//	int inLove;							// 4J removed - now synched data
	int loveTime;
	weak_ptr<Player> loveCause;

public:
	Animal(Level *level);

protected:
	virtual void defineSynchedData();
	virtual void serverAiMobStep();

public:
	virtual void aiStep();

protected:
	virtual void checkHurtTarget(shared_ptr<Entity> target, float d);

private:
	virtual void breedWith(shared_ptr<Animal> target);

public:
	virtual float getWalkTargetValue(int x, int y, int z);

public:
	virtual bool hurt(DamageSource *source, int dmg);
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual shared_ptr<Entity> findAttackTarget();

public:
	virtual bool canSpawn();
	virtual int getAmbientSoundInterval();

protected:
	virtual bool removeWhenFarAway();
	virtual int getExperienceReward(shared_ptr<Player> killedBy);

public:
	virtual bool isFood(shared_ptr<ItemInstance> itemInstance);
	virtual bool interact(shared_ptr<Player> player);

protected:
	int getInLoveValue();					// 4J added

public:
	void setInLoveValue(int value); // 4J added
	void setInLove(shared_ptr<Player> player);			// 4J added, then modified to match latest Java for XboxOne achievements
	shared_ptr<Player> getLoveCause();
	bool isInLove();
    void resetLove();
    virtual bool canMate(shared_ptr<Animal> partner);

	// 4J added for determining whether animals are enclosed or not
private:
	bool m_isDespawnProtected;

	static const int MAX_WANDER_DISTANCE = 20;		// Maximum distance that the entity can wander before being considered as not enclosed & therefore not protected (in tiles)
	short m_minWanderX, m_maxWanderX;				// Bounding x values that this has moved since last being set to protected ( in tiles )
	short m_minWanderZ, m_maxWanderZ;				// Bounding z values that this has moved since last being set to protected ( in tiles )
	void updateDespawnProtectedState();
public:
	virtual bool isDespawnProtected();
	virtual void setDespawnProtected();

};
