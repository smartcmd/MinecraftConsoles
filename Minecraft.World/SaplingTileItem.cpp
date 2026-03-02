#include "stdafx.h"
#include "TileItem.h"
#include "net.minecraft.world.level.tile.h"
#include "SaplingTileItem.h"

SaplingTileItem::SaplingTileItem(int id) : TileItem(id)
{
	setMaxDamage(0);
	setStackedByData(true);
}

int SaplingTileItem::getLevelDataForAuxValue(int auxValue) 
{
	return auxValue;
}

Icon *SaplingTileItem::getIcon(int itemAuxValue) 
{
	return Tile::sapling->getTexture(0, itemAuxValue);
}

// 4J brought forward to have unique names for different sapling types
unsigned int SaplingTileItem::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	int auxValue = instance->getAuxValue();
	if (auxValue < 0 || auxValue >= Sapling::SAPLING_NAMES_SIZE)
	{
		auxValue = 0;
	}
	return Sapling::SAPLING_NAMES[auxValue];
}