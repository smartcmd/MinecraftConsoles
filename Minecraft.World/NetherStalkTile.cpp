#include "stdafx.h"
#include "NetherStalkTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.h"

const wstring NetherStalkTile::TEXTURE_NAMES[] = { L"netherStalk_0", L"netherStalk_1", L"netherStalk_2" };

NetherStalkTile::NetherStalkTile(int id) : Bush(id)
{
	setTicking(true);
	updateDefaultShape();

	icons = NULL;
}

// 4J Added override
void NetherStalkTile::updateDefaultShape()
{
	float ss = 0.5f;
	this->setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, 0.25f, 0.5f + ss);
}

bool NetherStalkTile::mayPlaceOn(int tile)
{
	return tile == Tile::hellSand_Id;
}

// Brought forward to fix #60073 - TU7: Content: Gameplay: Nether Warts cannot be placed next to each other in the Nether
bool NetherStalkTile::canSurvive(Level *level, int x, int y, int z)
{
	return mayPlaceOn(level->getTile(x, y - 1, z));
}

void NetherStalkTile::tick(Level *level, int x, int y, int z, Random *random)
{
	int age = level->getData(x, y, z);
	if (age < MAX_AGE)
	{
		//Biome *biome = biomeSource->getBiome(x, z);
		//if (dynamic_cast<HellBiome *>(biome) != NULL)
		//{
		if (random->nextInt(10) == 0)
		{
			age++;
			level->setData(x, y, z, age);
		}
		//}
	}

	Bush::tick(level, x, y, z, random);
}

void NetherStalkTile::growCropsToMax(Level *level, int x, int y, int z)
{
	level->setData(x, y, z, MAX_AGE);
}

Icon *NetherStalkTile::getTexture(int face, int data)
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

int NetherStalkTile::getRenderShape()
{
	return Tile::SHAPE_ROWS;
}

void NetherStalkTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus)
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
		popResource(level, x, y, z, shared_ptr<ItemInstance>(new ItemInstance(Item::netherStalkSeeds)));
	}
}

int NetherStalkTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return 0;
}

int NetherStalkTile::getResourceCount(Random *random)
{
	return 0;
}

int NetherStalkTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::netherStalkSeeds_Id;
}

void NetherStalkTile::registerIcons(IconRegister *iconRegister)
{
	icons = new Icon*[NETHER_STALK_TEXTURE_COUNT];

	for (int i = 0; i < NETHER_STALK_TEXTURE_COUNT; i++)
	{
		icons[i] = iconRegister->registerIcon(TEXTURE_NAMES[i]);
	}
}
