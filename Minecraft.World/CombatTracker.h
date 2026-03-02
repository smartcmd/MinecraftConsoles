#pragma once

#include "SharedConstants.h"

class CombatEntry;
class LivingEntity;
class ChatPacket;

class CombatTracker
{
public:
	static const int RESET_DAMAGE_STATUS_TIME = SharedConstants::TICKS_PER_SECOND * 5;
	static const int RESET_COMBAT_STATUS_TIME = SharedConstants::TICKS_PER_SECOND * 15;

	// 4J: This enum replaces 
	enum eLOCATION
	{
		eLocation_GENERIC = 0, 
		eLocation_LADDER,
		eLocation_VINES,
		eLocation_WATER,

		eLocation_COUNT,
	};

private:
	vector<CombatEntry *> entries;
	LivingEntity *mob; //Owner
	int lastDamageTime;
	bool inCombat;
	bool takingDamage;
	eLOCATION nextLocation; // 4J: Location is now an enum, not a string

public:
	CombatTracker(LivingEntity *mob);
	~CombatTracker();

	void prepareForDamage();
	void recordDamage(DamageSource *source, float health, float damage);
	shared_ptr<ChatPacket> getDeathMessagePacket(); // 4J: Changed this to return a chat packet
	shared_ptr<LivingEntity> getKiller();

private:
	CombatEntry *getMostSignificantFall();
	eLOCATION getFallLocation(CombatEntry *entry);

public:
	bool isTakingDamage();
	bool isInCombat();

private:
	void resetPreparedStatus();
	void recheckStatus();
};