#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "LiquidTileStatic.h"
#include "FireTile.h"

LiquidTileStatic::LiquidTileStatic(int id, Material *material) : LiquidTile(id, material)
{
	setTicking(false);
	if (material == Material::lava) setTicking(true);
}

bool LiquidTileStatic::isPathfindable(LevelSource *level, int x, int y, int z)
{
	return material != Material::lava;
}

void LiquidTileStatic::neighborChanged(Level *level, int x, int y, int z, int type)
{
	LiquidTile::neighborChanged(level, x, y, z, type);
	if (level->getTile(x, y, z) == id)
	{
		setDynamic(level, x, y, z);
	}
}

void LiquidTileStatic::setDynamic(Level *level, int x, int y, int z)
{
	int d = level->getData(x, y, z);
	level->setTileAndData(x, y, z, id - 1, d, Tile::UPDATE_CLIENTS);
	level->addToTickNextTick(x, y, z, id - 1, getTickDelay(level));
}

void LiquidTileStatic::tick(Level *level, int x, int y, int z, Random *random)
{
	if (material == Material::lava)
	{
		int h = random->nextInt(3);
		for (int i = 0; i < h; i++)
		{
			x += random->nextInt(3) - 1;
			y++;
			z += random->nextInt(3) - 1;
			int t = level->getTile(x, y, z);
			if (t == 0)
			{
				if (isFlammable(level, x - 1, y, z) ||
					isFlammable(level, x + 1, y, z) ||
					isFlammable(level, x, y, z - 1) ||
					isFlammable(level, x, y, z + 1) ||
					isFlammable(level, x, y - 1, z) ||
					isFlammable(level, x, y + 1, z))
				{
					level->setTileAndUpdate(x, y, z, Tile::fire_Id);
					return;
				}
			}
			else if (Tile::tiles[t]->material->blocksMotion())
			{
				return;
			}

		}
		if (h == 0)
		{
			int ox = x;
			int oz = z;
			for (int i = 0; i<  3; i++)
			{
				x = ox + random->nextInt(3) - 1;
				z = oz + random->nextInt(3) - 1;
				if (level->isEmptyTile(x, y + 1, z) && isFlammable(level, x, y, z))
				{
					level->setTileAndUpdate(x, y + 1, z, Tile::fire_Id);
				}
			}
		}
	}
}

bool LiquidTileStatic::isFlammable(Level *level, int x, int y, int z)
{
	return level->getMaterial(x, y, z)->isFlammable();
}
