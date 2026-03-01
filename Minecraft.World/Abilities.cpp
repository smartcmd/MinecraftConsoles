#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "Abilities.h"

Abilities::Abilities()
{
	invulnerable = false;
	flying = false;
	mayfly = false;
	instabuild = false;
	mayBuild = true;
	flyingSpeed = 0.05f;
	walkingSpeed = 0.1f;

#ifdef _DEBUG_MENUS_ENABLED
	debugflying = false;
#endif
}

void Abilities::addSaveData(CompoundTag *parentTag)
{
	CompoundTag *tag = new CompoundTag();

	tag->putBoolean(L"invulnerable", invulnerable);
	tag->putBoolean(L"flying", flying);
	tag->putBoolean(L"mayfly", mayfly);
	tag->putBoolean(L"instabuild", instabuild);
	tag->putBoolean(L"mayBuild", mayBuild);
	tag->putFloat(L"flySpeed", flyingSpeed);
	tag->putFloat(L"walkSpeed", walkingSpeed);

	parentTag->put(L"abilities", tag);

}

void Abilities::loadSaveData(CompoundTag *parentTag)
{
	if (parentTag->contains(L"abilities"))
	{
		CompoundTag *tag = parentTag->getCompound(L"abilities");

		invulnerable = tag->getBoolean(L"invulnerable");
		flying = tag->getBoolean(L"flying");
		mayfly = tag->getBoolean(L"mayfly");
		instabuild = tag->getBoolean(L"instabuild");

		if (tag->contains(L"flySpeed"))
		{
			flyingSpeed = tag->getFloat(L"flySpeed");
			walkingSpeed = tag->getFloat(L"walkSpeed");
		}
		if (tag->contains(L"mayBuild"))
		{
			mayBuild = tag->getBoolean(L"mayBuild");
		}
	}
}

float Abilities::getFlyingSpeed()
{
	return flyingSpeed;
}

void Abilities::setFlyingSpeed(float value)
{
	flyingSpeed = value;
}

float Abilities::getWalkingSpeed()
{
	return walkingSpeed;
}

void Abilities::setWalkingSpeed(float value)
{
	walkingSpeed = value;
}