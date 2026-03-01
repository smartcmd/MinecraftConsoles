#pragma once

#include "Animal.h"
#include "OwnableEntity.h"

class SitGoal;

class TamableAnimal : public Animal, public OwnableEntity
{
protected:
	static const int DATA_FLAGS_ID = 16;
	static const int DATA_OWNERUUID_ID = 17;

	SitGoal *sitGoal;

public:
	TamableAnimal(Level *level);
	virtual ~TamableAnimal();

protected:
	virtual void defineSynchedData();

public:
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
protected:
	virtual void spawnTamingParticles(bool success);

public:
	virtual void handleEntityEvent(byte id);
	virtual bool isTame();
	virtual void setTame(bool value);
	virtual bool isSitting();
	virtual void setSitting(bool value);
	virtual wstring getOwnerUUID();
	virtual void setOwnerUUID(const wstring &name);
	virtual shared_ptr<Entity> getOwner();
	virtual SitGoal *getSitGoal();
	bool wantsToAttack(shared_ptr<LivingEntity> target, shared_ptr<LivingEntity> owner);
	Team *getTeam();
	bool isAlliedTo(shared_ptr<LivingEntity> other);
};