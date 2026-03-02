#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.h"
#include "SeedFoodItem.h"

SeedFoodItem::SeedFoodItem(int id, int nutrition, float saturationMod, int resultId, int targetLand) : FoodItem(id, nutrition, saturationMod, false)
{
	this->resultId = resultId;
	this->targetLand = targetLand;

}

bool SeedFoodItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	if (face != Facing::UP) return false;

	if (!player->mayUseItemAt(x, y, z, face, instance) || !player->mayUseItemAt(x, y + 1, z, face, instance)) return false;
	int targetType = level->getTile(x, y, z);

	if (targetType == targetLand && level->isEmptyTile(x, y + 1, z))
	{
		if(!bTestUseOnOnly)
		{
			level->setTileAndUpdate(x, y + 1, z, resultId);
			instance->count--;
		}
		return true;
	}
	return false;
}