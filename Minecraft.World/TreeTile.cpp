#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.piston.h"
#include "net.minecraft.h"
#include "net.minecraft.world.h"
#include "LeafTile.h"

#include "TreeTile.h"

const unsigned int TreeTile::TREE_NAMES[TREE_NAMES_LENGTH] = {	IDS_TILE_LOG_OAK,
													IDS_TILE_LOG_SPRUCE,
													IDS_TILE_LOG_BIRCH,
													IDS_TILE_LOG_JUNGLE
												};

const wstring TreeTile::TREE_TEXTURES[] = {L"tree_side", L"tree_spruce", L"tree_birch", L"tree_jungle"};

TreeTile::TreeTile(int id) : Tile(id, Material::wood)
{
	icons = NULL;
	iconTop = NULL;
}

int TreeTile::getRenderShape()
{
	return Tile::SHAPE_TREE;
}

int TreeTile::getResourceCount(Random *random)
{
	return 1;
}

int TreeTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::treeTrunk_Id;
}

void TreeTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	int r = LeafTile::REQUIRED_WOOD_RANGE;
	int r2 = r + 1;

	if (level->hasChunksAt(x - r2, y - r2, z - r2, x + r2, y + r2, z + r2))
	{
		for (int xo = -r; xo <= r; xo++)
			for (int yo = -r; yo <= r; yo++)
				for (int zo = -r; zo <= r; zo++)
				{
					int t = level->getTile(x + xo, y + yo, z + zo);
					if (t == Tile::leaves_Id)
					{
						int currentData = level->getData(x + xo, y + yo, z + zo);
						if ((currentData & LeafTile::UPDATE_LEAF_BIT) == 0)
						{
							level->setDataNoUpdate(x + xo, y + yo, z + zo, currentData | LeafTile::UPDATE_LEAF_BIT);
						}
					}
				}
	}
}

void TreeTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by)
{
	int type = level->getData(x, y, z) & MASK_TYPE;
	int dir = PistonBaseTile::getNewFacing(level, x, y, z, dynamic_pointer_cast<Player>(by));
	int facing = 0;

	switch (dir)
	{
	case Facing::NORTH:
	case Facing::SOUTH:
		facing = FACING_Z;
		break;
	case Facing::EAST:
	case Facing::WEST:
		facing = FACING_X;
		break;
	case Facing::UP:
	case Facing::DOWN:
		facing = FACING_Y;
		break;
	}

	level->setData(x, y, z, type | facing);
}

Icon *TreeTile::getTexture(int face, int data)
{
	int dir = data & MASK_FACING;
	int type = data & MASK_TYPE;

	if (dir == FACING_Y && (face == Facing::UP || face == Facing::DOWN))
	{
		return iconTop;
	} 
	else if (dir == FACING_X && (face == Facing::EAST || face == Facing::WEST))
	{
		return iconTop;
	}
	else if (dir == FACING_Z && (face == Facing::NORTH || face == Facing::SOUTH))
	{
		return iconTop;
	}

	return icons[type];
}

unsigned int TreeTile::getDescriptionId(int iData /*= -1*/)
{
	int type = iData & MASK_TYPE;
	if(type < 0 ) type = 0;
	return TreeTile::TREE_NAMES[type];
}

int TreeTile::getSpawnResourcesAuxValue(int data)
{
	return data & MASK_TYPE;
}

int TreeTile::getWoodType(int data)
{
	return data & MASK_TYPE;
}

shared_ptr<ItemInstance> TreeTile::getSilkTouchItemInstance(int data)
{
	// fix to avoid getting silktouched sideways logs
	return shared_ptr<ItemInstance>(new ItemInstance(id, 1, getWoodType(data)));
}

void TreeTile::registerIcons(IconRegister *iconRegister)
{
	iconTop = iconRegister->registerIcon(L"tree_top");
	icons = new Icon*[TREE_NAMES_LENGTH];

	for (int i = 0; i < TREE_NAMES_LENGTH; i++)
	{
		icons[i] = iconRegister->registerIcon(TREE_TEXTURES[i]);
	}
}