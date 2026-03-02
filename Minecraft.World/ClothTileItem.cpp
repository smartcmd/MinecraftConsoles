#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "ItemInstance.h"
#include "DyePowderItem.h"
#include "ClothTileItem.h"

const unsigned int ClothTileItem::COLOR_DESCS[] = 
{ 
	IDS_TILE_CLOTH_BLACK,
	IDS_TILE_CLOTH_RED,
	IDS_TILE_CLOTH_GREEN,
	IDS_TILE_CLOTH_BROWN,
	IDS_TILE_CLOTH_BLUE,
	IDS_TILE_CLOTH_PURPLE,
	IDS_TILE_CLOTH_CYAN,
	IDS_TILE_CLOTH_SILVER,
	IDS_TILE_CLOTH_GRAY,
	IDS_TILE_CLOTH_PINK,
	IDS_TILE_CLOTH_LIME,
	IDS_TILE_CLOTH_YELLOW,
	IDS_TILE_CLOTH_LIGHT_BLUE,
	IDS_TILE_CLOTH_MAGENTA,
	IDS_TILE_CLOTH_ORANGE,
	IDS_TILE_CLOTH_WHITE
};

const unsigned int ClothTileItem::CARPET_COLOR_DESCS[] = 
{ 
	IDS_TILE_CARPET_BLACK,
	IDS_TILE_CARPET_RED,
	IDS_TILE_CARPET_GREEN,
	IDS_TILE_CARPET_BROWN,
	IDS_TILE_CARPET_BLUE,
	IDS_TILE_CARPET_PURPLE,
	IDS_TILE_CARPET_CYAN,
	IDS_TILE_CARPET_SILVER,
	IDS_TILE_CARPET_GRAY,
	IDS_TILE_CARPET_PINK,
	IDS_TILE_CARPET_LIME,
	IDS_TILE_CARPET_YELLOW,
	IDS_TILE_CARPET_LIGHT_BLUE,
	IDS_TILE_CARPET_MAGENTA,
	IDS_TILE_CARPET_ORANGE,
	IDS_TILE_CARPET_WHITE
};

ClothTileItem::ClothTileItem(int id) : TileItem(id)
{
	setMaxDamage(0);
	setStackedByData(true);
}

Icon *ClothTileItem::getIcon(int itemAuxValue) 
{
	return Tile::cloth->getTexture(2, ClothTile::getTileDataForItemAuxValue(itemAuxValue));

}

int ClothTileItem::getLevelDataForAuxValue(int auxValue) 
{
	return auxValue;
}

unsigned int ClothTileItem::getDescriptionId(shared_ptr<ItemInstance> instance) 
{
	if(getTileId() == Tile::woolCarpet_Id) return CARPET_COLOR_DESCS[ClothTile::getTileDataForItemAuxValue(instance->getAuxValue())];
	else return COLOR_DESCS[ClothTile::getTileDataForItemAuxValue(instance->getAuxValue())];
}
