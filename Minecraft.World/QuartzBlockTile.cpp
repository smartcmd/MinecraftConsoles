#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.h"
#include "QuartzBlockTile.h"

// 4J Added
#include "Level.h"
#include "PistonBaseTile.h"
#include "Player.h"

int QuartzBlockTile::BLOCK_NAMES[QUARTZ_BLOCK_NAMES] = {
	IDS_TILE_QUARTZ_BLOCK, IDS_TILE_QUARTZ_BLOCK_CHISELED, IDS_TILE_QUARTZ_BLOCK_LINES, IDS_TILE_QUARTZ_BLOCK_LINES, IDS_TILE_QUARTZ_BLOCK_LINES
};

const wstring QuartzBlockTile::TEXTURE_TOP = L"quartzblock_top";
const wstring QuartzBlockTile::TEXTURE_CHISELED_TOP = L"quartzblock_chiseled_top";
const wstring QuartzBlockTile::TEXTURE_LINES_TOP = L"quartzblock_lines_top";
const wstring QuartzBlockTile::TEXTURE_BOTTOM = L"quartzblock_bottom";
const wstring QuartzBlockTile::TEXTURE_NAMES[QUARTZ_BLOCK_TEXTURES] = {	L"quartzblock_side", L"quartzblock_chiseled", L"quartzblock_lines", L"", L""};

QuartzBlockTile::QuartzBlockTile(int id) : Tile(id, Material::stone)
{
}

Icon *QuartzBlockTile::getTexture(int face, int data)
{
	if (data == TYPE_LINES_Y || data == TYPE_LINES_X || data == TYPE_LINES_Z)
	{
		if (data == TYPE_LINES_Y && (face == Facing::UP || face == Facing::DOWN))
		{
			return iconLinesTop;
		}
		else if (data == TYPE_LINES_X && (face == Facing::EAST || face == Facing::WEST))
		{
			return iconLinesTop;
		}
		else if (data == TYPE_LINES_Z && (face == Facing::NORTH || face == Facing::SOUTH))
		{
			return iconLinesTop;
		}

		return icons[data];
	}

	if (face == Facing::UP || (face == Facing::DOWN && data == TYPE_CHISELED))
	{
		if (data == TYPE_CHISELED)
		{
			return iconChiseledTop;
		}
		return iconTop;
	}
	if (face == Facing::DOWN)
	{
		return iconBottom;
	}
	if (data < 0 || data >= QUARTZ_BLOCK_TEXTURES) data = 0;
	return icons[data];
}

int QuartzBlockTile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
	if (itemValue == TYPE_LINES_Y)
	{
		switch (face)
		{
		case Facing::NORTH:
		case Facing::SOUTH:
			itemValue = TYPE_LINES_Z;
			break;
		case Facing::EAST:
		case Facing::WEST:
			itemValue = TYPE_LINES_X;
			break;
		case Facing::UP:
		case Facing::DOWN:
			itemValue = TYPE_LINES_Y;
			break;
		}
	}

	return itemValue;
}

int QuartzBlockTile::getSpawnResourcesAuxValue(int data)
{
	if (data == TYPE_LINES_X || data == TYPE_LINES_Z) return TYPE_LINES_Y;

	return data;
}

shared_ptr<ItemInstance> QuartzBlockTile::getSilkTouchItemInstance(int data)
{
	if (data == TYPE_LINES_X || data == TYPE_LINES_Z) return shared_ptr<ItemInstance>(new ItemInstance(id, 1, TYPE_LINES_Y));
	return Tile::getSilkTouchItemInstance(data);
}

int QuartzBlockTile::getRenderShape()
{
	return Tile::SHAPE_QUARTZ;
}


void QuartzBlockTile::registerIcons(IconRegister *iconRegister)
{
	for (int i = 0; i < QUARTZ_BLOCK_TEXTURES; i++)
	{
		if (TEXTURE_NAMES[i].empty())
		{
			icons[i] = icons[i - 1];
		}
		else
		{
			icons[i] = iconRegister->registerIcon(TEXTURE_NAMES[i]);
		}
	}

	iconTop = iconRegister->registerIcon(TEXTURE_TOP);
	iconChiseledTop = iconRegister->registerIcon(TEXTURE_CHISELED_TOP);
	iconLinesTop = iconRegister->registerIcon(TEXTURE_LINES_TOP);
	iconBottom = iconRegister->registerIcon(TEXTURE_BOTTOM);
}