#include "stdafx.h"
#include "WoodTile.h"
#include "TreeTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.h"

const unsigned int WoodTile::WOOD_NAMES[WOOD_NAMES_LENGTH] = {	IDS_TILE_OAKWOOD_PLANKS,
	IDS_TILE_SPRUCEWOOD_PLANKS,
	IDS_TILE_BIRCHWOOD_PLANKS,
	IDS_TILE_JUNGLE_PLANKS,
};

const wstring WoodTile::TEXTURE_NAMES[] = {L"oak", L"spruce", L"birch", L"jungle"};

// 	public static final String[] WOOD_NAMES = {
// 		"oak", "spruce", "birch", "jungle"
// 	};

WoodTile::WoodTile(int id) : Tile(id, Material::wood)
{
	icons = NULL;
}

unsigned int WoodTile::getDescriptionId(int iData)
{
	if(iData < 0 || iData >= WOOD_NAMES_LENGTH) iData = 0;

	return WOOD_NAMES[iData];
}

Icon *WoodTile::getTexture(int face, int data) 
{
	if (data < 0 || data >= WOOD_NAMES_LENGTH)
	{
		data = 0;
	}
	return icons[data];
}

int WoodTile::getSpawnResourcesAuxValue(int data) 
{
		return data;
}

void WoodTile::registerIcons(IconRegister *iconRegister)
{
	icons = new Icon*[WOOD_NAMES_LENGTH];

	for (int i = 0; i < WOOD_NAMES_LENGTH; i++)
	{
		icons[i] = iconRegister->registerIcon(getIconName() + L"_" + TEXTURE_NAMES[i]);
	}
}