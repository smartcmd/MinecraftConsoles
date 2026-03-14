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

wstring ColoredTile::getColoredIconName(int id)
{
	return (id != DyePowderItem::SILVER ? DyePowderItem::COLOR_TEXTURES[id] : L"light_gray") + L"_" + getIconName();
}

void ColoredTile::registerIcons(IconRegister *iconRegister)
{
	for (int i = 0; i < ICON_COUNT; i++)
	{
		int val = getItemAuxValueForTileData(i);
		auto colorName = (val != DyePowderItem::SILVER ? DyePowderItem::COLOR_TEXTURES[val] : L"light_gray");
		icons[i] = iconRegister->registerIcon(colorName + L"_" + getIconName());
	}
}