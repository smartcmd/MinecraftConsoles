#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.h"
#include "SharedConstants.h"
#include "BreakDoorGoal.h"

BreakDoorGoal::BreakDoorGoal(Mob *mob) : DoorInteractGoal(mob)
{
	breakTime = 0;
	lastBreakProgress = -1;
}

bool BreakDoorGoal::canUse()
{
	if (!DoorInteractGoal::canUse()) return false;
	if (!mob->level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING)) return false;
	return !doorTile->isOpen(mob->level, doorX, doorY, doorZ);
}

void BreakDoorGoal::start()
{
	DoorInteractGoal::start();
	breakTime = 0;
}

bool BreakDoorGoal::canContinueToUse()
{
	double d = mob->distanceToSqr(doorX, doorY, doorZ);
	return breakTime <= DOOR_BREAK_TIME && !doorTile->isOpen(mob->level, doorX, doorY, doorZ) && d < 2 * 2;
}

void BreakDoorGoal::stop()
{
	DoorInteractGoal::stop();
	mob->level->destroyTileProgress(mob->entityId, doorX, doorY, doorZ, -1);
}

void BreakDoorGoal::tick()
{
	DoorInteractGoal::tick();
	if (mob->getRandom()->nextInt(20) == 0)
	{
		mob->level->levelEvent(LevelEvent::SOUND_ZOMBIE_WOODEN_DOOR, doorX, doorY, doorZ, 0);
	}

	breakTime++;

	int progress = static_cast<int>(breakTime / (float)DOOR_BREAK_TIME * 10);
	if (progress != lastBreakProgress)
	{
		mob->level->destroyTileProgress(mob->entityId, doorX, doorY, doorZ, progress);
		lastBreakProgress = progress;
	}

	if (breakTime == DOOR_BREAK_TIME)
	{
		if (mob->level->difficulty == Difficulty::HARD)
		{
			mob->level->removeTile(doorX, doorY, doorZ);
			mob->level->levelEvent(LevelEvent::SOUND_ZOMBIE_DOOR_CRASH, doorX, doorY, doorZ, 0);
			mob->level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, doorX, doorY, doorZ, doorTile->id);
		}
	}
}