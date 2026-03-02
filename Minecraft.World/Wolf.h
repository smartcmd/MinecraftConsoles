#pragma once

using namespace std;

#include "TamableAnimal.h"
class DamageSource;

class Wolf : public TamableAnimal
{
public:
	eINSTANCEOF GetType() { return eTYPE_WOLF; }
	static Entity *create(Level *level) { return new Wolf(level); }
private:
	// synch health in a separate field to show tame wolves' health
    static const int DATA_HEALTH_ID = 18;
	static const int DATA_INTERESTED_ID = 19;
	static const int DATA_COLLAR_COLOR = 20;
    static const int START_HEALTH = 8;
    static const int MAX_HEALTH = 20;
    static const int TAME_HEALTH = 20;

	float interestedAngle, interestedAngleO;
	bool m_isWet, isShaking;
	float shakeAnim, shakeAnimO;

public:
	Wolf(Level *level);
	virtual bool useNewAi();
	virtual void setTarget(shared_ptr<Mob> target);

protected:
	virtual void serverAiMobStep();

public:
	virtual int getMaxHealth();

protected:
	virtual void defineSynchedData();
	virtual bool makeStepSound();

public:
	virtual int getTexture();		// 4J - changed from wstring to ing
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual bool removeWhenFarAway();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual float getSoundVolume();
	virtual int getDeathLoot();

public:
	virtual void aiStep();
	virtual void tick();
	bool isWet();
	float getWetShade(float a);
	float getBodyRollAngle(float a, float offset);
	float getHeadRollAngle(float a);
	float getHeadHeight();
	int getMaxHeadXRot();
	virtual bool hurt(DamageSource *source, int dmg);
	virtual bool doHurtTarget(shared_ptr<Entity> target);
	virtual bool interact(shared_ptr<Player> player);
	virtual void handleEntityEvent(byte id);
	float getTailAngle();
	virtual bool isFood(shared_ptr<ItemInstance> item);
	virtual int getMaxSpawnClusterSize();
	bool isAngry();
	void setAngry(bool value);
	int getCollarColor();
	void setCollarColor(int color);
	void tame(const wstring &wsOwnerUUID, bool bDisplayTamingParticles, bool bSetSitting);

	// For tooltips
	int GetSynchedHealth();

protected:
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target);

public:
	virtual void setIsInterested(bool isInterested);
	virtual bool canMate(shared_ptr<Animal> animal);
	bool isInterested();
};
