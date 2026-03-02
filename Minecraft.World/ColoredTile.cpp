#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.item.h"
#include "ColoredTile.h"

ColoredTile::ColoredTile(int id, Material *material) : Tile(id, material)
{
}

Icon *ColoredTile::getTexture(int face, int data)
{
	return icons[data % ICON_COUNT];
}

int ColoredTile::getSpawnResourcesAuxValue(int data)
{
	return data;
}

int ColoredTile::getTileDataForItemAuxValue(int auxValue)
{
	return (~auxValue & 0xf);
}

int ColoredTile::getItemAuxValueForTileData(int data)
{
	return (~data & 0xf);
}

void ColoredTile::registerIcons(IconRegister *iconRegister)
{
	for (int i = 0; i < ICON_COUNT; i++)
	{
		icons[i] = iconRegister->registerIcon(getIconName() + L"_" + DyePowderItem::COLOR_TEXTURES[getItemAuxValueForTileData(i)]);
	}
}