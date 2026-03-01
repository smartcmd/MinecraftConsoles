#include "stdafx.h"
#include "MelonTile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.h"
#include "Facing.h"

MelonTile::MelonTile(int id) : Tile(id, Material::vegetable)
{
	iconTop = NULL;
}

Icon *MelonTile::getTexture(int face, int data)
{
	if (face == Facing::UP || face == Facing::DOWN) return iconTop;
	return icon;
}

int MelonTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::melon->id;
}

int MelonTile::getResourceCount(Random *random)
{
	return 3 + random->nextInt(5);
}

int MelonTile::getResourceCountForLootBonus(int bonusLevel, Random *random)
{
	int total = getResourceCount(random) + random->nextInt(1 + bonusLevel);
	if (total > 9)
	{
		total = 9;
	}
	return total;
}

void MelonTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(getIconName() + L"_side");
	iconTop = iconRegister->registerIcon(getIconName() + L"_top");
}