#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "LockedChestTile.h"

LockedChestTile::LockedChestTile(int id) : Tile(id, Material::wood)
{
}

bool LockedChestTile::mayPlace(Level *level, int x, int y, int z)
{
	return true;
}

void LockedChestTile::tick(Level *level, int x, int y, int z, Random *random)
{
	level->removeTile(x, y, z);
}

void LockedChestTile::registerIcons(IconRegister *iconRegister)
{
	// None
}