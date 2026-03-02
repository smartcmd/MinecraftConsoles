#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.h"
#include "VinesFeature.h"


VinesFeature::VinesFeature()
{
}

bool VinesFeature::place(Level *level, Random *random, int x, int y, int z)
{
	int ox = x;
	int oz = z;

	while (y < 128)
	{
		if (level->isEmptyTile(x, y, z))
		{
			for (int face = Facing::NORTH; face <= Facing::EAST; face++)
			{
				if (Tile::vine->mayPlace(level, x, y, z, face))
				{
					level->setTileAndData(x, y, z, Tile::vine_Id, 1 << Direction::FACING_DIRECTION[Facing::OPPOSITE_FACING[face]], Tile::UPDATE_CLIENTS);
					break;
				}
			}
		}
		else
		{
			x = ox + random->nextInt(4) - random->nextInt(4);
			z = oz + random->nextInt(4) - random->nextInt(4);
		}
		y++;
	}

	return true;
}