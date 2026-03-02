#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "LeafTileItem.h"
#include "FoliageColor.h"

LeafTileItem::LeafTileItem(int id) : TileItem(id)
{
	setMaxDamage(0);
	setStackedByData(true);
}

int LeafTileItem::getLevelDataForAuxValue(int auxValue)
{
	return auxValue | LeafTile::PERSISTENT_LEAF_BIT;

}

Icon *LeafTileItem::getIcon(int itemAuxValue)
{
	return Tile::leaves->getTexture(0, itemAuxValue);	
}


int LeafTileItem::getColor(shared_ptr<ItemInstance> item, int spriteLayer)
{
	int data = item->getAuxValue();
    if ((data & LeafTile::EVERGREEN_LEAF) == LeafTile::EVERGREEN_LEAF)
	{
        return FoliageColor::getEvergreenColor();
    }
    if ((data & LeafTile::BIRCH_LEAF) == LeafTile::BIRCH_LEAF)
	{
        return FoliageColor::getBirchColor();
    }
    return FoliageColor::getDefaultColor();
}

unsigned int LeafTileItem::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	int auxValue = instance->getAuxValue();
	if (auxValue < 0 || auxValue >= LeafTile::LEAF_NAMES_LENGTH)
	{
		auxValue = 0;
	}
	return LeafTile::LEAF_NAMES[auxValue];
}