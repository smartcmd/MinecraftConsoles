#pragma once

#include "TileItem.h"

class ItemInstance;

class ColoredTileItem : public TileItem
{
private:
	// Was const, but removing that so we don't have to make all the functions const as well!
	Tile *colorTile;
	intArray descriptionPostfixes;

public:
	using TileItem::getColor;
	ColoredTileItem(int id, bool stackedByData);
	~ColoredTileItem();

	virtual int getColor(shared_ptr<ItemInstance> item, int spriteLayer);
	virtual Icon *getIcon(int auxValue);
	virtual int getLevelDataForAuxValue(int auxValue);

	ColoredTileItem *setDescriptionPostfixes(intArray descriptionPostfixes);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
};
