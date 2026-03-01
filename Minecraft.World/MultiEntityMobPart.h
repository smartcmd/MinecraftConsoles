#pragma once
using namespace std;
#include "Entity.h"

class Level;
class MultiEntityMob;

class MultiEntityMobPart : public Entity
{
public:
	eINSTANCEOF GetType() { return eTYPE_MULTIENTITY_MOB_PART; };
public:
	weak_ptr<MultiEntityMob> parentMob;
	const wstring id;

	MultiEntityMobPart(shared_ptr<MultiEntityMob> parentMob, const wstring &id, float w, float h);

protected:
	virtual void defineSynchedData();
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void addAdditonalSaveData(CompoundTag *tag);

public:
	virtual bool isPickable();
	virtual bool hurt(DamageSource *source, float damage);
	virtual bool is(shared_ptr<Entity> other);
};