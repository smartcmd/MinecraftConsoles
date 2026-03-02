#include "stdafx.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.level.h"
#include "StoneMonsterTile.h"

const unsigned int StoneMonsterTile::STONE_MONSTER_NAMES[STONE_MONSTER_NAMES_LENGTH] = {	IDS_TILE_STONE_SILVERFISH,
													IDS_TILE_STONE_SILVERFISH_COBBLESTONE,
													IDS_TILE_STONE_SILVERFISH_STONE_BRICK,
												};

StoneMonsterTile::StoneMonsterTile(int id) : Tile(id, Material::clay)
{
    setDestroyTime(0);
}

Icon *StoneMonsterTile::getTexture(int face, int data)
{
    if (data == HOST_COBBLE)
	{
        return Tile::stoneBrick->getTexture(face);
    }
    if (data == HOST_STONEBRICK)
	{
        return Tile::stoneBrickSmooth->getTexture(face);
    }
    return Tile::rock->getTexture(face);
}

void StoneMonsterTile::registerIcons(IconRegister *iconRegister)
{
	// None
}

void StoneMonsterTile::destroy(Level *level, int x, int y, int z, int data)
{
    if (!level->isClientSide)
	{
		// 4J - limit total amount of monsters. The normal map spawning limits these to 50, and mobspawning tiles limit to 60, so give ourselves a bit of headroom here to also be able to make silverfish
		if(level->countInstanceOf( eTYPE_MONSTER, false) < 70 )
		{
			// Also limit the amount of silverfish specifically
			if(level->countInstanceOf( eTYPE_SILVERFISH, true) < 15 )
			{
				shared_ptr<Silverfish> silverfish = shared_ptr<Silverfish>(new Silverfish(level));
				silverfish->moveTo(x + .5, y, z + .5, 0, 0);
				level->addEntity(silverfish);

				silverfish->spawnAnim();
			}
		}
    }
    Tile::destroy(level, x, y, z, data);
}

int StoneMonsterTile::getResourceCount(Random *random)
{
	return 0;
}

bool StoneMonsterTile::isCompatibleHostBlock(int block)
{
	return block == Tile::rock_Id || block == Tile::stoneBrick_Id || block == Tile::stoneBrickSmooth_Id;
}

int StoneMonsterTile::getDataForHostBlock(int block)
{
    if (block == Tile::stoneBrick_Id)
	{
        return HOST_COBBLE;
    }
    if (block == Tile::stoneBrickSmooth_Id)
	{
        return HOST_STONEBRICK;
    }
    return HOST_ROCK;
}

Tile *StoneMonsterTile::getHostBlockForData(int data)
{
	switch (data)
	{
	case HOST_COBBLE:
		return Tile::stoneBrick;
	case HOST_STONEBRICK:
		return Tile::stoneBrickSmooth;
	default:
		return Tile::rock;
	}
}

shared_ptr<ItemInstance> StoneMonsterTile::getSilkTouchItemInstance(int data)
{
    Tile *tile = Tile::rock;
    if (data == HOST_COBBLE)
	{
        tile = Tile::stoneBrick;
    }
    if (data == HOST_STONEBRICK)
	{
        tile = Tile::stoneBrickSmooth;
    }
    return shared_ptr<ItemInstance>(new ItemInstance(tile));
}

int StoneMonsterTile::cloneTileData(Level *level, int x, int y, int z)
{
	return level->getData(x, y, z);
}

unsigned int StoneMonsterTile::getDescriptionId(int iData /*= -1*/)
{
	if(iData < 0 ) iData = 0;
	return StoneMonsterTile::STONE_MONSTER_NAMES[iData];
}