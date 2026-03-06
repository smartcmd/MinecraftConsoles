#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.h"
#include "CropTile.h"

CropTile::CropTile(int id) : Bush(id)
{
	setTicking(true);
	updateDefaultShape();
	icons = NULL;

	setDestroyTime(0.0f);
	setSoundType(SOUND_GRASS);
	setNotCollectStatistics();
	sendTileData();
}

// 4J Added override
void CropTile::updateDefaultShape()
{
	float ss = 0.5f;
	this->setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, 0.25f, 0.5f + ss);
}

bool CropTile::mayPlaceOn(int tile)
{
	return tile == Tile::farmland_Id;
}

void CropTile::tick(Level *level, int x, int y, int z, Random *random)
{
	Bush::tick(level, x, y, z, random);
	if (level->getRawBrightness(x, y + 1, z) >= Level::MAX_BRIGHTNESS - 6)
	{

		int age = level->getData(x, y, z);
		if (age < 7)
		{
			float growthSpeed = getGrowthSpeed(level, x, y, z);

			if (random->nextInt((int) (25 / growthSpeed) + 1) == 0)
			{
				age++;
				level->setData(x, y, z, age, Tile::UPDATE_CLIENTS);
			}
		}
	}
}

void CropTile::growCrops(Level *level, int x, int y, int z)
{
	int stage = level->getData(x, y, z) + Mth::nextInt(level->random, 2, 5);
	if (stage > 7) stage = 7;
	level->setData(x, y, z, stage, Tile::UPDATE_CLIENTS);
}

float CropTile::getGrowthSpeed(Level *level, int x, int y, int z)
{
	float speed = 1;

	int n = level->getTile(x, y, z - 1);
	int s = level->getTile(x, y, z + 1);
	int w = level->getTile(x - 1, y, z);
	int e = level->getTile(x + 1, y, z);

	int d0 = level->getTile(x - 1, y, z - 1);
	int d1 = level->getTile(x + 1, y, z - 1);
	int d2 = level->getTile(x + 1, y, z + 1);
	int d3 = level->getTile(x - 1, y, z + 1);

	bool horizontal = w == id || e == id;
	bool vertical = n == id || s == id;
	bool diagonal = d0 == id || d1 == id || d2 == id || d3 == id;

	for (int xx = x - 1; xx <= x + 1; xx++)
		for (int zz = z - 1; zz <= z + 1; zz++)
		{
			int t = level->getTile(xx, y - 1, zz);

			float tileSpeed = 0;
			if (t == Tile::farmland_Id)
			{
				tileSpeed = 1;
				if (level->getData(xx, y - 1, zz) > 0) tileSpeed = 3;
			}

			if (xx != x || zz != z) tileSpeed /= 4;

			speed += tileSpeed;
		}

		if (diagonal || (horizontal && vertical)) speed /= 2;

		return speed;

}

Icon *CropTile::getTexture(int face, int data)
{
	if (data < 0 || data > 7) data = 7;
	return icons[data];
}

int CropTile::getRenderShape()
{
	return Tile::SHAPE_ROWS;
}

int CropTile::getBaseSeedId()
{
	return Item::seeds_wheat_Id;
}

int CropTile::getBasePlantId()
{
	return Item::wheat_Id;
}

/**
* Using this method instead of destroy() to determine if seeds should be
* dropped
*/
void CropTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus)
{
	Bush::spawnResources(level, x, y, z, data, odds, 0);

	if (level->isClientSide)
	{
		return;
	}
	if (data >= 7)
	{
		int count = 3 + playerBonus;
		for (int i = 0; i < count; i++)
		{
			if (level->random->nextInt(5 * 3) > data) continue;
			popResource(level, x, y, z, shared_ptr<ItemInstance>(new ItemInstance(getBaseSeedId(), 1, 0)));
		}
	}
}

int CropTile::getResource(int data, Random *random, int playerBonusLevel)
{
	if (data == 7)
	{
		return getBasePlantId();
	}

	return getBaseSeedId();
}

int CropTile::getResourceCount(Random *random)
{
	return 1;
}

int CropTile::cloneTileId(Level *level, int x, int y, int z)
{
	return getBaseSeedId();
}

void CropTile::registerIcons(IconRegister *iconRegister)
{
	icons = new Icon*[8];

	for (int i = 0; i < 8; i++)
	{
		icons[i] = iconRegister->registerIcon(L"crops_" + std::to_wstring(i));
	}
}