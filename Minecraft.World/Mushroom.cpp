#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.levelgen.feature.h"
#include "net.minecraft.world.h"
#include "Mushroom.h"

Mushroom::Mushroom(int id) : Bush(id)
{
	this->updateDefaultShape();
	this->setTicking(true);
}

// 4J Added override
void Mushroom::updateDefaultShape()
{
	float ss = 0.2f;
	this->setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, ss * 2, 0.5f + ss);
}

void Mushroom::tick(Level *level, int x, int y, int z, Random *random)
{
	if (random->nextInt(25) == 0)
	{
		int r = 4;
		int max = 5;
		for (int xx = x - r; xx <= x + r; xx++)
			for (int zz = z - r; zz <= z + r; zz++)
				for (int yy = y - 1; yy <= y + 1; yy++)
				{
					if (level->getTile(xx, yy, zz) == id && --max <= 0) return;
				}

				int x2 = x + random->nextInt(3) - 1;
				int y2 = y + random->nextInt(2) - random->nextInt(2);
				int z2 = z + random->nextInt(3) - 1;
				for (int i = 0; i < 4; i++)
				{
					if (level->isEmptyTile(x2, y2, z2) && canSurvive(level, x2, y2, z2))
					{
						x = x2;
						y = y2;
						z = z2;
					}
					x2 = x + random->nextInt(3) - 1;
					y2 = y + random->nextInt(2) - random->nextInt(2);
					z2 = z + random->nextInt(3) - 1;
				}

				if (level->isEmptyTile(x2, y2, z2) && canSurvive(level, x2, y2, z2))
				{
					level->setTileAndData(x2, y2, z2, id, 0, UPDATE_CLIENTS);
				}
	}
}

bool Mushroom::mayPlace(Level *level, int x, int y, int z)
{
	return Bush::mayPlace(level, x, y, z) && canSurvive(level, x, y, z);
}

bool Mushroom::mayPlaceOn(int tile)
{
	return Tile::solid[tile];
}

bool Mushroom::canSurvive(Level *level, int x, int y, int z)
{
	if (y < 0 || y >= Level::maxBuildHeight) return false;

	int below = level->getTile(x, y - 1, z);

	return below == Tile::mycel_Id || (level->getDaytimeRawBrightness(x, y, z) < 13 && mayPlaceOn(below));
}

bool Mushroom::growTree(Level *level, int x, int y, int z, Random *random)
{
	int data = level->getData(x, y, z);

	level->removeTile(x, y, z);
	Feature *f = nullptr;

	if (id == Tile::mushroom_brown_Id)
	{
		f = new HugeMushroomFeature(0);
	}
	else if (id == Tile::mushroom_red_Id)
	{
		f = new HugeMushroomFeature(1);
	}

	if (f == nullptr || !f->place(level, random, x, y, z))
	{
		level->setTileAndData(x, y, z, id, data, Tile::UPDATE_ALL);
		if( f != nullptr )
			delete f;
		return false;
	}
	if( f != nullptr )
		delete f;
	return true;
}