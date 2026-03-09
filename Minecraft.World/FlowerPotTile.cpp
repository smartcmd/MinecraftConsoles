#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "FlowerPotTile.h"

FlowerPotTile::FlowerPotTile(int id) : Tile(id, Material::decoration, isSolidRender() )
{
	updateDefaultShape();
	sendTileData();
}

void FlowerPotTile::updateDefaultShape()
{
	float size = 6.0f / 16.0f;
	float half = size / 2;
	setShape(0.5f - half, 0, 0.5f - half, 0.5f + half, size, 0.5f + half);
}

bool FlowerPotTile::isSolidRender(bool isServerLevel)
{
	return false;
}

int FlowerPotTile::getRenderShape()
{
	return SHAPE_FLOWER_POT;
}

bool FlowerPotTile::isCubeShaped()
{
	return false;
}

bool FlowerPotTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
	shared_ptr<ItemInstance> item = player->inventory->getSelected();
	if (item == nullptr) return false;
	if (level->getData(x, y, z) != 0) return false;
	int type = getTypeFromItem(item);

	if (type > 0)
	{
		level->setData(x, y, z, type, Tile::UPDATE_CLIENTS);

		if (!player->abilities.instabuild)
		{
			if (--item->count <= 0)
			{
				player->inventory->setItem(player->inventory->selected, nullptr);
			}
		}

		return true;
	}

	return false;
}

int FlowerPotTile::cloneTileId(Level *level, int x, int y, int z)
{
	shared_ptr<ItemInstance> item = getItemFromType(level->getData(x, y, z));

	if (item == nullptr)
	{
		return Item::flowerPot_Id;
	}
	else
	{
		return item->id;
	}
}

int FlowerPotTile::cloneTileData(Level *level, int x, int y, int z)
{
	shared_ptr<ItemInstance> item = getItemFromType(level->getData(x, y, z));

	if (item == nullptr)
	{
		return Item::flowerPot_Id;
	}
	else
	{
		return item->getAuxValue();
	}
}

bool FlowerPotTile::useOwnCloneData()
{
	return true;
}

bool FlowerPotTile::mayPlace(Level *level, int x, int y, int z)
{
	return Tile::mayPlace(level, x, y, z) && level->isTopSolidBlocking(x, y - 1, z);
}

void FlowerPotTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!level->isTopSolidBlocking(x, y - 1, z))
	{
		spawnResources(level, x, y, z, level->getData(x, y, z), 0);

		level->removeTile(x, y, z);
	}
}

void FlowerPotTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	Tile::spawnResources(level, x, y, z, data, odds, playerBonusLevel);

	if (data > 0)
	{
		shared_ptr<ItemInstance> item = getItemFromType(data);
		if (item != nullptr) popResource(level, x, y, z, item);
	}
}

int FlowerPotTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::flowerPot_Id;
}

shared_ptr<ItemInstance> FlowerPotTile::getItemFromType(int type)
{
	switch (type)
	{
	case TYPE_FLOWER_RED:
		return std::make_shared<ItemInstance>(Tile::rose);
	case TYPE_FLOWER_YELLOW:
		return std::make_shared<ItemInstance>(Tile::flower);
	case TYPE_CACTUS:
		return std::make_shared<ItemInstance>(Tile::cactus);
	case TYPE_MUSHROOM_BROWN:
		return std::make_shared<ItemInstance>(Tile::mushroom_brown);
	case TYPE_MUSHROOM_RED:
		return std::make_shared<ItemInstance>(Tile::mushroom_red);
	case TYPE_DEAD_BUSH:
		return std::make_shared<ItemInstance>(Tile::deadBush);
	case TYPE_SAPLING_DEFAULT:
		return std::make_shared<ItemInstance>(Tile::sapling, 1, Sapling::TYPE_DEFAULT);
	case TYPE_SAPLING_BIRCH:
		return std::make_shared<ItemInstance>(Tile::sapling, 1, Sapling::TYPE_BIRCH);
	case TYPE_SAPLING_EVERGREEN:
		return std::make_shared<ItemInstance>(Tile::sapling, 1, Sapling::TYPE_EVERGREEN);
	case TYPE_SAPLING_JUNGLE:
		return std::make_shared<ItemInstance>(Tile::sapling, 1, Sapling::TYPE_JUNGLE);
	case TYPE_FERN:
		return std::make_shared<ItemInstance>(Tile::tallgrass, 1, TallGrass::FERN);
	}

	return nullptr;
}

int FlowerPotTile::getTypeFromItem(shared_ptr<ItemInstance> item)
{
	int id = item->getItem()->id;

	if (id == Tile::rose_Id) return TYPE_FLOWER_RED;
	if (id == Tile::flower_Id) return TYPE_FLOWER_YELLOW;
	if (id == Tile::cactus_Id) return TYPE_CACTUS;
	if (id == Tile::mushroom_brown_Id) return TYPE_MUSHROOM_BROWN;
	if (id == Tile::mushroom_red_Id) return TYPE_MUSHROOM_RED;
	if (id == Tile::deadBush_Id) return TYPE_DEAD_BUSH;

	if (id == Tile::sapling_Id)
	{
		switch (item->getAuxValue())
		{
		case Sapling::TYPE_DEFAULT:
			return TYPE_SAPLING_DEFAULT;
		case Sapling::TYPE_BIRCH:
			return TYPE_SAPLING_BIRCH;
		case Sapling::TYPE_EVERGREEN:
			return TYPE_SAPLING_EVERGREEN;
		case Sapling::TYPE_JUNGLE:
			return TYPE_SAPLING_JUNGLE;
		}
	}

	if (id == Tile::tallgrass_Id)
	{
		switch (item->getAuxValue())
		{
		case TallGrass::FERN:
			return TYPE_FERN;
		}
	}

	return 0;
}