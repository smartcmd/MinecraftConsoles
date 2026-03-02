#include "stdafx.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "SynchedEntityData.h"
#include "ParticleTypes.h"
#include "TamableAnimal.h"

TamableAnimal::TamableAnimal(Level *level) : Animal(level)
{
	sitGoal = new SitGoal(this);
}

TamableAnimal::~TamableAnimal()
{
	if(sitGoal != NULL) delete sitGoal;
}

void TamableAnimal::defineSynchedData()
{
	Animal::defineSynchedData();
	entityData->define(DATA_FLAGS_ID, (byte) 0);
	entityData->define(DATA_OWNERUUID_ID, L"");
}

void TamableAnimal::addAdditonalSaveData(CompoundTag *tag)
{
	Animal::addAdditonalSaveData(tag);
#ifdef _XBOX_ONE
	// 4J Stu Added from later Java version to remove owners from save transfer saves. We will probably want this on other platforms in the future
	if (getOwnerUUID().empty())
	{
		tag->putString(L"OwnerUUID", L"");
	}
	else
	{
		tag->putString(L"OwnerUUID", getOwnerUUID());
	}
#else
	if (getOwnerUUID().empty())
	{
		tag->putString(L"Owner", L"");
	}
	else
	{
		tag->putString(L"Owner", getOwnerUUID());
	}
#endif
	tag->putBoolean(L"Sitting", isSitting());
}

void TamableAnimal::readAdditionalSaveData(CompoundTag *tag)
{
	Animal::readAdditionalSaveData(tag);
#ifdef _XBOX_ONE
	// 4J Stu Added from later Java version to remove owners from save transfer saves. We will probably want this on other platforms in the future
	wstring owner = L"";
	if(tag->contains(L"OwnerUUID") )
	{
		owner = tag->getString(L"OwnerUUID");
	}
#else
	wstring owner = tag->getString(L"Owner");
#endif
	if (owner.length() > 0)
	{
		setOwnerUUID(owner);
		setTame(true);
	}
	sitGoal->wantToSit(tag->getBoolean(L"Sitting"));
}

void TamableAnimal::spawnTamingParticles(bool success)
{
	ePARTICLE_TYPE particle = eParticleType_heart;
	if (!success)
	{
		particle = eParticleType_smoke;
	}
	for (int i = 0; i < 7; i++)
	{
		double xa = random->nextGaussian() * 0.02;
		double ya = random->nextGaussian() * 0.02;
		double za = random->nextGaussian() * 0.02;
		level->addParticle(particle, x + random->nextFloat() * bbWidth * 2 - bbWidth, y + .5f + random->nextFloat() * bbHeight, z + random->nextFloat() * bbWidth * 2 - bbWidth, xa, ya, za);
	}
}

void TamableAnimal::handleEntityEvent(byte id)
{
	if (id == EntityEvent::TAMING_SUCCEEDED)
	{
		spawnTamingParticles(true);
	}
	else if (id == EntityEvent::TAMING_FAILED)
	{
		spawnTamingParticles(false);
	}
	else
	{
		Animal::handleEntityEvent(id);
	}
}

bool TamableAnimal::isTame()
{
	return (entityData->getByte(DATA_FLAGS_ID) & 0x04) != 0;
}

void TamableAnimal::setTame(bool value)
{
	byte current = entityData->getByte(DATA_FLAGS_ID);
	if (value)
	{
		entityData->set(DATA_FLAGS_ID, (byte) (current | 0x04));
	}
	else
	{
		entityData->set(DATA_FLAGS_ID, (byte) (current & ~0x04));
	}
}

bool TamableAnimal::isSitting()
{
	return (entityData->getByte(DATA_FLAGS_ID) & 0x01) != 0;
}

void TamableAnimal::setSitting(bool value)
{
	byte current = entityData->getByte(DATA_FLAGS_ID);
	if (value)
	{
		entityData->set(DATA_FLAGS_ID, (byte) (current | 0x01));
	}
	else
	{
		entityData->set(DATA_FLAGS_ID, (byte) (current & ~0x01));
	}
}

wstring TamableAnimal::getOwnerUUID()
{
	return entityData->getString(DATA_OWNERUUID_ID);
}

void TamableAnimal::setOwnerUUID(const wstring &name)
{
	entityData->set(DATA_OWNERUUID_ID, name);
}

shared_ptr<Mob> TamableAnimal::getOwner()
{
	return level->getPlayerByUUID(getOwnerUUID());
}

SitGoal *TamableAnimal::getSitGoal()
{
	return sitGoal;
}