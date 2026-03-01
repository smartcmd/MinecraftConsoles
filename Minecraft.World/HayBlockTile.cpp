#include "stdafx.h"
#include "net.minecraft.world.h"
#include "HayBlockTile.h"

HayBlockTile::HayBlockTile(int id) : RotatedPillarTile(id, Material::grass)
{
}

int HayBlockTile::getRenderShape()
{
	return SHAPE_TREE;
}

Icon *HayBlockTile::getTypeTexture(int type)
{
	return icon;
}

void HayBlockTile::registerIcons(IconRegister *iconRegister)
{
	iconTop = iconRegister->registerIcon(getIconName() + L"_top");
	icon = iconRegister->registerIcon(getIconName() + L"_side");
}