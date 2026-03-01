#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.network.packet.h"
#include "MinecartChest.h"

MinecartChest::MinecartChest(Level *level) : MinecartContainer(level)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
}

MinecartChest::MinecartChest(Level *level, double x, double y, double z) : MinecartContainer(level, x, y, z)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
}

// 4J Added
int MinecartChest::getContainerType()
{
	return ContainerOpenPacket::MINECART_CHEST;
}

void MinecartChest::destroy(DamageSource *source)
{
	MinecartContainer::destroy(source);

	spawnAtLocation(Tile::chest_Id, 1, 0);
}

unsigned int MinecartChest::getContainerSize()
{
	return 9 * 3;
}

int MinecartChest::getType()
{
	return TYPE_CHEST;
}

Tile *MinecartChest::getDefaultDisplayTile()
{
	return Tile::chest;
}

int MinecartChest::getDefaultDisplayOffset()
{
	return 8;
}