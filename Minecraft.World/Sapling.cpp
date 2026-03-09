#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.levelgen.feature.h"
#include "net.minecraft.world.h"

#include "Sapling.h"

int Sapling::SAPLING_NAMES[SAPLING_NAMES_SIZE] = {	IDS_TILE_SAPLING_OAK,
	IDS_TILE_SAPLING_SPRUCE,
	IDS_TILE_SAPLING_BIRCH,
	IDS_TILE_SAPLING_JUNGLE
};

const wstring Sapling::TEXTURE_NAMES[] = {L"sapling", L"sapling_spruce", L"sapling_birch", L"sapling_jungle"};

Sapling::Sapling(int id) : Bush( id )
{
	this->updateDefaultShape();
	icons = nullptr;
}

// 4J Added override
void Sapling::updateDefaultShape()
{
	float ss = 0.4f;
	this->setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, ss * 2, 0.5f + ss);
}

void Sapling::tick(Level *level, int x, int y, int z, Random *random)
{
	if (level->isClientSide) return;

	Bush::tick(level, x, y, z, random);

	if (level->getRawBrightness(x, y + 1, z) >= Level::MAX_BRIGHTNESS - 6)
	{
		if (random->nextInt(7) == 0)
		{
			advanceTree(level, x, y, z, random);
		}
	}
}

Icon *Sapling::getTexture(int face, int data)
{
	data = data & TYPE_MASK;
	return icons[data];
}

void Sapling::advanceTree(Level *level, int x, int y, int z, Random *random)
{
	int data = level->getData(x, y, z);
	if ((data & AGE_BIT) == 0)
	{
		level->setData(x, y, z, data | AGE_BIT, Tile::UPDATE_NONE);
	}
	else
	{
		growTree(level, x, y, z, random);
	}
}

void Sapling::growTree(Level *level, int x, int y, int z, Random *random)
{
	int data = level->getData(x, y, z) & TYPE_MASK;

	Feature *f = nullptr;

	int ox = 0, oz = 0;
	bool multiblock = false;

	if (data == TYPE_EVERGREEN)
	{
		f = new SpruceFeature(true);
	}
	else if (data == TYPE_BIRCH)
	{
		f = new BirchFeature(true);
	}
	else if (data == TYPE_JUNGLE)
	{
		// check for mega tree
		for (ox = 0; ox >= -1; ox--)
		{
			for (oz = 0; oz >= -1; oz--)
			{
				if (isSapling(level, x + ox, y, z + oz, TYPE_JUNGLE) && isSapling(level, x + ox + 1, y, z + oz, TYPE_JUNGLE) && isSapling(level, x + ox, y, z + oz + 1, TYPE_JUNGLE)
					&& isSapling(level, x + ox + 1, y, z + oz + 1, TYPE_JUNGLE))
				{
					f = new MegaTreeFeature(true, 10 + random->nextInt(20), TreeTile::JUNGLE_TRUNK, LeafTile::JUNGLE_LEAF);
					multiblock = true;
					break;
				}
			}
			if (f != nullptr)
			{
				break;
			}
		}
		if (f == nullptr)
		{
			ox = oz = 0;
			f = new TreeFeature(true, 4 + random->nextInt(7), TreeTile::JUNGLE_TRUNK, LeafTile::JUNGLE_LEAF, false);
		}
	}
	else
	{
		f = new TreeFeature(true);
		if (random->nextInt(10) == 0)
		{
			delete f;
			f = new BasicTree(true);
		}
	}
	if (multiblock)
	{
		level->setTileAndData(x + ox, y, z + oz, 0, 0, Tile::UPDATE_NONE);
		level->setTileAndData(x + ox + 1, y, z + oz, 0, 0, Tile::UPDATE_NONE);
		level->setTileAndData(x + ox, y, z + oz + 1, 0, 0, Tile::UPDATE_NONE);
		level->setTileAndData(x + ox + 1, y, z + oz + 1, 0, 0, Tile::UPDATE_NONE);
	}
	else
	{
		level->setTileAndData(x, y, z, 0, 0, Tile::UPDATE_NONE);
	}
	if (!f->place(level, random, x + ox, y, z + oz))
	{
		if (multiblock)
		{
			level->setTileAndData(x + ox, y, z + oz, id, data, Tile::UPDATE_NONE);
			level->setTileAndData(x + ox + 1, y, z + oz, id, data, Tile::UPDATE_NONE);
			level->setTileAndData(x + ox, y, z + oz + 1, id, data, Tile::UPDATE_NONE);
			level->setTileAndData(x + ox + 1, y, z + oz + 1, id, data, Tile::UPDATE_NONE);
		}
		else
		{
			level->setTileAndData(x, y, z, id, data, Tile::UPDATE_NONE);
		}
	}
	if( f != nullptr )
		delete f;
}

unsigned int Sapling::getDescriptionId(int iData /*= -1*/)
{
	if(iData < 0 ) iData = 0;
	return Sapling::SAPLING_NAMES[iData];
}

int Sapling::getSpawnResourcesAuxValue(int data)
{
	return data & TYPE_MASK;
}

bool Sapling::isSapling(Level *level, int x, int y, int z, int type)
{
	return (level->getTile(x, y, z) == id) && ((level->getData(x, y, z) & TYPE_MASK) == type);
}

void Sapling::registerIcons(IconRegister *iconRegister)
{
	icons = new Icon*[SAPLING_NAMES_SIZE];

	for (int i = 0; i < SAPLING_NAMES_SIZE; i++)
	{
		icons[i] = iconRegister->registerIcon(TEXTURE_NAMES[i]);
	}
}
