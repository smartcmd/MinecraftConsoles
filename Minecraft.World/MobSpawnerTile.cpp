#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "MobSpawnerTile.h"

MobSpawnerTile::MobSpawnerTile(int id) : BaseEntityTile(id, Material::stone, isSolidRender() )
{
}

shared_ptr<TileEntity> MobSpawnerTile::newTileEntity(Level *level)
{
	return std::make_shared<MobSpawnerTileEntity>();
}

int MobSpawnerTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return 0;
}

int MobSpawnerTile::getResourceCount(Random *random)
{
	return 0;
}

bool MobSpawnerTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool MobSpawnerTile::blocksLight()
{
	return false;
}

void MobSpawnerTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	Tile::spawnResources(level, x, y, z, data, odds, playerBonusLevel);

	// also spawn experience if the block is broken
	{
		int magicCount = 15 + level->random->nextInt(15) + level->random->nextInt(15);
		popExperience(level, x, y, z, magicCount);
	}
}

int MobSpawnerTile::cloneTileId(Level *level, int x, int y, int z)
{
	return 0;
}