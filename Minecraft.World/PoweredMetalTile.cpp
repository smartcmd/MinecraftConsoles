#include "stdafx.h"
#include "net.minecraft.world.level.redstone.h"
#include "PoweredMetalTile.h"

PoweredMetalTile::PoweredMetalTile(int id) : MetalTile(id)
{
}

bool PoweredMetalTile::isSignalSource()
{
	return true;
}

int PoweredMetalTile::getSignal(LevelSource *level, int x, int y, int z, int dir)
{
	return Redstone::SIGNAL_MAX;
}