#pragma once
#include "CombatTracker.h"

class DamageSource;

class CombatEntry
{
private:
	DamageSource *source;
	int time;
	float damage;
	float health;
	CombatTracker::eLOCATION location; // 4J: Location is now an enum, not a string
	float fallDistance;

public:
	CombatEntry(DamageSource *source, int time, float health, float damage, CombatTracker::eLOCATION nextLocation, float fallDistance);
	~CombatEntry();

	DamageSource *getSource();
	int getTime();
	float getDamage();
	float getHealthBeforeDamage();
	float getHealthAfterDamage();
	bool isCombatRelated();
	CombatTracker::eLOCATION getLocation();
	wstring getAttackerName();
	float getFallDistance();
};