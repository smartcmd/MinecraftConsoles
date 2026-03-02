#include "stdafx.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.h"
#include "StoneSlabTileItem.h"

StoneSlabTileItem::StoneSlabTileItem(int id, HalfSlabTile *halfTile, HalfSlabTile *fullTile, bool full) : TileItem(id)
{
	this->halfTile = halfTile;
	this->fullTile = fullTile;

	isFull = full;
	setMaxDamage(0);
	setStackedByData(true);
}

Icon *StoneSlabTileItem::getIcon(int itemAuxValue) 
{
	return Tile::tiles[id]->getTexture(2, itemAuxValue);
}

int StoneSlabTileItem::getLevelDataForAuxValue(int auxValue) 
{
	return auxValue;
}

unsigned int StoneSlabTileItem::getDescriptionId(shared_ptr<ItemInstance> instance) 
{
	return halfTile->getAuxName(instance->getAuxValue());
}

bool StoneSlabTileItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	if (isFull) 
	{
		return TileItem::useOn(instance, player, level, x, y, z, face, clickX, clickY, clickZ, bTestUseOnOnly);
	}

	if (instance->count == 0) return false;
	if (!player->mayUseItemAt(x, y, z, face, instance)) return false;

	int currentTile = level->getTile(x, y, z);
	int currentData = level->getData(x, y, z);
	int slabType = currentData & HalfSlabTile::TYPE_MASK;
	bool isUpper = (currentData & HalfSlabTile::TOP_SLOT_BIT) != 0;

	if (((face == Facing::UP && !isUpper) || (face == Facing::DOWN && isUpper)) && currentTile == halfTile->id && slabType == instance->getAuxValue()) 
	{
		if(bTestUseOnOnly)
		{
			return true;
		}

		if (level->isUnobstructed(fullTile->getAABB(level, x, y, z)) && level->setTileAndData(x, y, z, fullTile->id, slabType, Tile::UPDATE_ALL)) 
		{
			level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, fullTile->soundType->getPlaceSound(), (fullTile->soundType->getVolume() + 1) / 2, fullTile->soundType->getPitch() * 0.8f);
			instance->count--;
		}
		return true;
	} 
	else if (tryConvertTargetTile(instance, player, level, x, y, z, face, bTestUseOnOnly)) 
	{
		return true;
	} 
	else 
	{
		return TileItem::useOn(instance, player, level, x, y, z, face, clickX, clickY, clickZ, bTestUseOnOnly);
	}
}


bool StoneSlabTileItem::mayPlace(Level *level, int x, int y, int z, int face,shared_ptr<Player> player, shared_ptr<ItemInstance> item) 
{
	int ox = x, oy = y, oz = z;

	int currentTile = level->getTile(x, y, z);
	int currentData = level->getData(x, y, z);
	int slabType = currentData & HalfSlabTile::TYPE_MASK;
	boolean isUpper = (currentData & HalfSlabTile::TOP_SLOT_BIT) != 0;

	if (((face == Facing::UP && !isUpper) || (face == Facing::DOWN && isUpper)) && currentTile == halfTile->id && slabType == item->getAuxValue()) 
	{
		return true;
	}

	if (face == 0) y--;
	if (face == 1) y++;
	if (face == 2) z--;
	if (face == 3) z++;
	if (face == 4) x--;
	if (face == 5) x++;

	currentTile = level->getTile(x, y, z);
	currentData = level->getData(x, y, z);
	slabType = currentData & HalfSlabTile::TYPE_MASK;
	isUpper = (currentData & HalfSlabTile::TOP_SLOT_BIT) != 0;

	if (currentTile == halfTile->id && slabType == item->getAuxValue()) 
	{
		return true;
	}

	return TileItem::mayPlace(level, ox, oy, oz, face, player, item);
}

bool StoneSlabTileItem::tryConvertTargetTile(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, bool bTestUseOnOnly) 
{
	if (face == 0) y--;
	if (face == 1) y++;
	if (face == 2) z--;
	if (face == 3) z++;
	if (face == 4) x--;
	if (face == 5) x++;

	int currentTile = level->getTile(x, y, z);
	int currentData = level->getData(x, y, z);
	int slabType = currentData & HalfSlabTile::TYPE_MASK;

	if (currentTile == halfTile->id && slabType == instance->getAuxValue()) 
	{
		if(bTestUseOnOnly)
		{
			return true;
		}			
		if (level->isUnobstructed(fullTile->getAABB(level, x, y, z)) && level->setTileAndData(x, y, z, fullTile->id, slabType, Tile::UPDATE_ALL)) 
		{
			level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, fullTile->soundType->getPlaceSound(), (fullTile->soundType->getVolume() + 1) / 2, fullTile->soundType->getPitch() * 0.8f);
			instance->count--;	
		}
		return true;
	}

	return false;
}