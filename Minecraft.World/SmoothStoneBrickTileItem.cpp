#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "SmoothStoneBrickTileItem.h"

SmoothStoneBrickTileItem::SmoothStoneBrickTileItem(int id, Tile *parentTile) : TileItem(id)
{
	this->parentTile = parentTile;

	setMaxDamage(0);
	setStackedByData(true);
}

Icon *SmoothStoneBrickTileItem::getIcon(int itemAuxValue)
{
	return parentTile->getTexture(2, itemAuxValue);
}

int SmoothStoneBrickTileItem::getLevelDataForAuxValue(int auxValue)
{
	return auxValue;
}

unsigned int SmoothStoneBrickTileItem::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	int auxValue = instance->getAuxValue();
	if (auxValue < 0 || auxValue >= SmoothStoneBrickTile::SMOOTH_STONE_BRICK_NAMES_LENGTH)
	{
		auxValue = 0;
	}
	return SmoothStoneBrickTile::SMOOTH_STONE_BRICK_NAMES[auxValue];
}