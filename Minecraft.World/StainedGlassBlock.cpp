#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.h"
#include "StainedGlassBlock.h"

Icon *StainedGlassBlock::ICONS[StainedGlassBlock::ICONS_LENGTH];

StainedGlassBlock::StainedGlassBlock(int id, Material *material) : HalfTransparentTile(id, L"glass", material, false)
{
}

Icon *StainedGlassBlock::getTexture(int face, int data)
{
	return ICONS[data % ICONS_LENGTH];
}

int StainedGlassBlock::getSpawnResourcesAuxValue(int data)
{
	return data;
}

int StainedGlassBlock::getItemAuxValueForBlockData(int data)
{
	return (~data & 0xf);
}


int StainedGlassBlock::getRenderLayer()
{
	return 1;
}

void StainedGlassBlock::registerIcons(IconRegister *iconRegister)
{
	for (int i = 0; i < ICONS_LENGTH; i++)
	{
		int val = getItemAuxValueForBlockData(i);
		auto colorName = (val != DyePowderItem::SILVER ? DyePowderItem::COLOR_TEXTURES[val] : L"light_gray");
		ICONS[i] = iconRegister->registerIcon(colorName + L"_" + getIconName());
	}
}

int StainedGlassBlock::getResourceCount(Random *random) 
{
	return 0;
}

bool StainedGlassBlock::isSilkTouchable()
{
	return true;
}

bool StainedGlassBlock::isCubeShaped()
{
	return false;
}
