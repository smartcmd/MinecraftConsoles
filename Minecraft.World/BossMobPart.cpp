#include "stdafx.h"
#include "BossMob.h"
#include "BossMobPart.h"



BossMobPart::BossMobPart(BossMob *bossMob, const wstring &id, float w, float h) : Entity(bossMob->level), bossMob( bossMob ), id( id )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	setSize(w, h);
}


void BossMobPart::defineSynchedData()
{
}

void BossMobPart::readAdditionalSaveData(CompoundTag *tag)
{
}

void BossMobPart::addAdditonalSaveData(CompoundTag *tag)
{
}


bool BossMobPart::isPickable()
{
	return true;
}

bool BossMobPart::hurt(DamageSource *source, int damage)
{
	return bossMob->hurt( dynamic_pointer_cast<BossMobPart>( shared_from_this() ), source, damage);
}

bool BossMobPart::is(shared_ptr<Entity> other)
{
	return shared_from_this() == other || bossMob == other.get();
}