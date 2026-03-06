#include "stdafx.h"
#include "net.minecraft.world.h"
#include "ClothTile.h"

ClothTile::ClothTile() :  Tile(35, Material::cloth)
{
	icons = NULL;
}

Icon *ClothTile::getTexture(int face, int data)
{
    return icons[data];
}

int ClothTile::getSpawnResourcesAuxValue(int data)
{
	return data;
}

int ClothTile::getTileDataForItemAuxValue(int auxValue)
{
	return (~auxValue & 0xf);
}

int ClothTile::getItemAuxValueForTileData(int data)
{
	return (~data & 0xf);
}

void ClothTile::registerIcons(IconRegister *iconRegister)
{
	icons = new Icon*[16];

	for (int i = 0; i < 16; i++)
	{
		icons[i] = iconRegister->registerIcon(L"cloth_" + std::to_wstring(i) );
	}
}