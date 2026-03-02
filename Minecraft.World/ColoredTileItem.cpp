#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "ColoredTileItem.h"

ColoredTileItem::ColoredTileItem(int id, bool stackedByData) : TileItem(id)
{
	this->colorTile = Tile::tiles[getTileId()];

	if (stackedByData)
	{
		setMaxDamage(0);
		setStackedByData(true);
	}
}

ColoredTileItem::~ColoredTileItem()
{
	if(descriptionPostfixes.data != NULL) delete [] descriptionPostfixes.data;
}

int ColoredTileItem::getColor(shared_ptr<ItemInstance> item, int spriteLayer)
{
	return colorTile->getColor(item->getAuxValue());
}

Icon *ColoredTileItem::getIcon(int auxValue)
{
	return colorTile->getTexture(0, auxValue);
}

int ColoredTileItem::getLevelDataForAuxValue(int auxValue)
{
	return auxValue;
}

ColoredTileItem *ColoredTileItem::setDescriptionPostfixes(intArray descriptionPostfixes)
{
	if(this->descriptionPostfixes.data != NULL) delete this->descriptionPostfixes.data;
	this->descriptionPostfixes = intArray(descriptionPostfixes.length);
	for(unsigned int i = 0; i < descriptionPostfixes.length; ++i )
	{
		this->descriptionPostfixes[i] = descriptionPostfixes[i];
	}

	return this;
}

unsigned int ColoredTileItem::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	if (descriptionPostfixes.data == NULL)
	{
		return TileItem::getDescriptionId(instance);
	}
	int id = instance->getAuxValue();
	if (id >= 0 && id < descriptionPostfixes.length)
	{
		return descriptionPostfixes[id]; //TileItem::getDescriptionId(instance) + "." + descriptionPostfixes[id];
	}
	return TileItem::getDescriptionId(instance);
}