#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.pathfinder.h"
#include "net.minecraft.world.level.tile.h"
#include "DoorInteractGoal.h"

DoorInteractGoal::DoorInteractGoal(Mob *mob)
{
	doorX = doorY = doorZ = 0;
	doorTile = nullptr;
	passed = false;
	doorOpenDirX = doorOpenDirZ = 0.0f;

	this->mob = mob;
}

bool DoorInteractGoal::canUse()
{
	if (!mob->horizontalCollision) return false;
	PathNavigation *pathNav = mob->getNavigation();
	Path *path = pathNav->getPath();
	if (path == nullptr || path->isDone() || !pathNav->canOpenDoors()) return false;

	for (int i = 0; i < min(path->getIndex() + 2, path->getSize()); ++i)
	{
		Node *n = path->get(i);
		doorX = n->x;
		doorY = n->y + 1;
		doorZ = n->z;
		if (mob->distanceToSqr(doorX, mob->y, doorZ) > 1.5 * 1.5) continue;
		doorTile = getDoorTile(doorX, doorY, doorZ);
		if (doorTile == nullptr) continue;
		return true;
	}

	doorX = Mth::floor(mob->x);
	doorY = Mth::floor(mob->y + 1);
	doorZ = Mth::floor(mob->z);
	doorTile = getDoorTile(doorX, doorY, doorZ);
	return doorTile != nullptr;
}

bool DoorInteractGoal::canContinueToUse()
{
	return !passed;
}

void DoorInteractGoal::start()
{
	passed = false;
	doorOpenDirX = static_cast<float>(doorX + 0.5f - mob->x);
	doorOpenDirZ = static_cast<float>(doorZ + 0.5f - mob->z);
}

void DoorInteractGoal::tick()
{
	float newDoorDirX = static_cast<float>(doorX + 0.5f - mob->x);
	float newDoorDirZ = static_cast<float>(doorZ + 0.5f - mob->z);
	float dot = doorOpenDirX * newDoorDirX + doorOpenDirZ * newDoorDirZ;
	if (dot < 0)
	{
		passed = true;
	}
}

DoorTile *DoorInteractGoal::getDoorTile(int x, int y, int z)
{
	int tileId = mob->level->getTile(x, y, z);
	if (tileId != Tile::door_wood_Id) return nullptr;
	return static_cast<DoorTile *>(Tile::tiles[tileId]);
}