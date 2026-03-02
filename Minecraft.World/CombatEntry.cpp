#include "stdafx.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.entity.h"
#include "BasicTypeContainers.h"
#include "CombatEntry.h"

CombatEntry::CombatEntry(DamageSource *source, int time, float health, float damage, CombatTracker::eLOCATION location, float fallDistance)
{
	this->source = NULL;
	if(source != NULL)
	{
		// 4J: this might actually be a derived damage source so use copy func
		this->source = source->copy();
	}
	this->time = time;
	this->damage = damage;
	this->health = health;
	this->location = location;
	this->fallDistance = fallDistance;
}

CombatEntry::~CombatEntry()
{
	delete source;
}

DamageSource *CombatEntry::getSource()
{
	return source;
}

int CombatEntry::getTime()
{
	return time;
}

float CombatEntry::getDamage()
{
	return damage;
}

float CombatEntry::getHealthBeforeDamage()
{
	return health;
}

float CombatEntry::getHealthAfterDamage()
{
	return health - damage;
}

bool CombatEntry::isCombatRelated()
{
	return source->getEntity() && source->getEntity()->instanceof(eTYPE_LIVINGENTITY);
}

CombatTracker::eLOCATION CombatEntry::getLocation()
{
	return location;
}

wstring CombatEntry::getAttackerName()
{
	return getSource()->getEntity() == NULL ? L"" : getSource()->getEntity()->getNetworkName();
}

float CombatEntry::getFallDistance()
{
	if (source == DamageSource::outOfWorld) return Float::MAX_VALUE;
	return fallDistance;
}