#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.h"
#include "SandStoneTile.h"

const wstring SandStoneTile::TEXTURE_TOP = L"sandstone_top";
const wstring SandStoneTile::TEXTURE_BOTTOM = L"sandstone_bottom";
const wstring SandStoneTile::TEXTURE_NAMES[] = {L"sandstone_side", L"sandstone_carved", L"sandstone_smooth"};

int SandStoneTile::SANDSTONE_NAMES[SANDSTONE_BLOCK_NAMES] = {
	IDS_TILE_SANDSTONE, IDS_TILE_SANDSTONE_CHISELED, IDS_TILE_SANDSTONE_SMOOTH
};

SandStoneTile::SandStoneTile(int id) : Tile(id, Material::stone)
{
	icons = nullptr;
	iconTop = nullptr;
	iconBottom = nullptr;
}

Icon *SandStoneTile::getTexture(int face, int data)
{
	if (face == Facing::UP || (face == Facing::DOWN && (data == TYPE_HEIROGLYPHS || data == TYPE_SMOOTHSIDE)))
	{
		return iconTop;
	}
	if (face == Facing::DOWN)
	{
		return iconBottom;
	}
	if (data < 0 || data >= SANDSTONE_TILE_TEXTURE_COUNT) data = 0;
	return icons[data];
}

int SandStoneTile::getSpawnResourcesAuxValue(int data)
{
	return data;
}

void SandStoneTile::registerIcons(IconRegister *iconRegister)
{
	icons = new Icon*[SANDSTONE_TILE_TEXTURE_COUNT];

	for (int i = 0; i < SANDSTONE_TILE_TEXTURE_COUNT; i++)
	{
		icons[i] = iconRegister->registerIcon(TEXTURE_NAMES[i]);
	}

	iconTop = iconRegister->registerIcon(TEXTURE_TOP);
	iconBottom = iconRegister->registerIcon(TEXTURE_BOTTOM);
}