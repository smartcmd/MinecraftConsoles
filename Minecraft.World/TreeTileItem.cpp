#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "TreeTileItem.h"

TreeTileItem::TreeTileItem(int id, Tile *parentTile) : TileItem(id)
{
	this->parentTile = parentTile;

	setMaxDamage(0);
	setStackedByData(true);
}

Icon *TreeTileItem::getIcon(int itemAuxValue)
{
	return parentTile->getTexture(2, itemAuxValue);
}

int TreeTileItem::getLevelDataForAuxValue(int auxValue) 
{
	return auxValue;
}

unsigned int TreeTileItem::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	int auxValue = instance->getAuxValue();
	if (auxValue < 0 || auxValue >= TreeTile::TREE_NAMES_LENGTH)
	{
		auxValue = 0;
	}
	return TreeTile::TREE_NAMES[auxValue];
}