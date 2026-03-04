#include "stdafx.h"
#include "GlowstoneTile.h"
#include "net.minecraft.world.item.h"

Glowstonetile::Glowstonetile(int id, Material *material) : Tile(id, material)
{
}

int Glowstonetile::getResourceCountForLootBonus(int bonusLevel, Random *random)
{
	return Mth::clamp(getResourceCount(random) + random->nextInt(bonusLevel + 1), 1, 4);
}

int Glowstonetile::getResourceCount(Random *random)
{
	return 2 + random->nextInt(3);
}

int Glowstonetile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::yellowDust->id;
}