#pragma once
using namespace std;
#include "Entity.h"

class Level;
class BossMob;

class BossMobPart : public Entity
{
public:
	eINSTANCEOF GetType() { return eTYPE_BOSS_MOB_PART; };
public:
	BossMob *bossMob;
	const wstring id;

	BossMobPart(BossMob *bossMob, const wstring &id, float w, float h);

protected:
	virtual void defineSynchedData();
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void addAdditonalSaveData(CompoundTag *tag);

public:
	virtual bool isPickable();
	virtual bool hurt(DamageSource *source, int damage);
	virtual bool is(shared_ptr<Entity> other);
};