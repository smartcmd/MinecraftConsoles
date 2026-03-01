#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.tile.h"
#include "OpenDoorGoal.h"

OpenDoorGoal::OpenDoorGoal(Mob *mob, bool closeDoorAfter) : DoorInteractGoal(mob)
{
	this->mob = mob;
	closeDoor = closeDoorAfter;
}

bool OpenDoorGoal::canContinueToUse()
{
	return closeDoor && forgetTime > 0 && DoorInteractGoal::canContinueToUse();
}

void OpenDoorGoal::start()
{
	forgetTime = 20;
	doorTile->setOpen(mob->level, doorX, doorY, doorZ, true);
}

void OpenDoorGoal::stop()
{
	if (closeDoor)
	{
		doorTile->setOpen(mob->level, doorX, doorY, doorZ, false);
	}
}

void OpenDoorGoal::tick()
{
	--forgetTime;
	DoorInteractGoal::tick();
}