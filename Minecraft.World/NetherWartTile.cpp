#include "stdafx.h"
#include "NetherWartTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.h"

NetherWartTile::NetherWartTile(int id) : Bush(id)
{
	setTicking(true);
	updateDefaultShape();
}

// 4J Added override
void NetherWartTile::updateDefaultShape()
{
	float ss = 0.5f;
	setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, 0.25f, 0.5f + ss);
}

bool NetherWartTile::mayPlaceOn(int tile)
{
	return tile == Tile::soulsand_Id;
}

// Brought forward to fix #60073 - TU7: Content: Gameplay: Nether Warts cannot be placed next to each other in the Nether
bool NetherWartTile::canSurvive(Level *level, int x, int y, int z)
{
	return mayPlaceOn(level->getTile(x, y - 1, z));
}

void NetherWartTile::tick(Level *level, int x, int y, int z, Random *random)
{
	int age = level->getData(x, y, z);
	if (age < MAX_AGE)
	{
		if (random->nextInt(10) == 0)
		{
			age++;
			level->setData(x, y, z, age, Tile::UPDATE_CLIENTS);
		}
	}

	Bush::tick(level, x, y, z, random);
}

void NetherWartTile::growCropsToMax(Level *level, int x, int y, int z)
{
	level->setData(x, y, z, MAX_AGE, Tile::UPDATE_CLIENTS);
}

Icon *NetherWartTile::getTexture(int face, int data)
{
	if (data >= MAX_AGE)
	{
		return icons[2];
	}
	if (data > 0)
	{
		return icons[1];
	}
	return icons[0];
}

int NetherWartTile::getRenderShape()
{
	return Tile::SHAPE_ROWS;
}

void NetherWartTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus)
{
	if (level->isClientSide)
	{
		return;
	}
	int count = 1;
	if (data >= MAX_AGE)
	{
		count = 2 + level->random->nextInt(3);
		if (playerBonus > 0)
		{
			count += level->random->nextInt(playerBonus + 1);
		}
	}
	for (int i = 0; i < count; i++)
	{
		popResource(level, x, y, z, shared_ptr<ItemInstance>(new ItemInstance(Item::netherwart_seeds)));
	}
}

int NetherWartTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return 0;
}

int NetherWartTile::getResourceCount(Random *random)
{
	return 0;
}

int NetherWartTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::netherwart_seeds_Id;
}

void NetherWartTile::registerIcons(IconRegister *iconRegister)
{
	for (int i = 0; i < NETHER_STALK_TEXTURE_COUNT; i++)
	{
		icons[i] = iconRegister->registerIcon(getIconName() + L"_stage_" + std::to_wstring(i) );
	}
}
