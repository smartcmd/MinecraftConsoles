#include "stdafx.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.crafting.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "JavaMath.h"
#include "FurnaceResultSlot.h"


FurnaceResultSlot::FurnaceResultSlot(shared_ptr<Player> player, shared_ptr<Container> container, int slot, int x, int y) : Slot( container, slot, x, y )
{
	this->player = player;
	removeCount = 0;
}

bool FurnaceResultSlot::mayPlace(shared_ptr<ItemInstance> item)
{
	return false;
}

shared_ptr<ItemInstance> FurnaceResultSlot::remove(int c)
{
	if (hasItem())
	{
		removeCount += min(c, getItem()->count);
	}
	return Slot::remove(c);
}

void FurnaceResultSlot::onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried)
{
	checkTakeAchievements(carried);
	Slot::onTake(player, carried);
}

void FurnaceResultSlot::onQuickCraft(shared_ptr<ItemInstance> picked, int count)
{
	removeCount += count;
	checkTakeAchievements(picked);
}

bool FurnaceResultSlot::mayCombine(shared_ptr<ItemInstance> second)
{
	return false;
}

void FurnaceResultSlot::checkTakeAchievements(shared_ptr<ItemInstance> carried)
{
	carried->onCraftedBy(player->level, player, removeCount);
	// spawn xp right on top of the player
	if (!player->level->isClientSide)
	{
		int amount = removeCount;
		float value = FurnaceRecipes::getInstance()->getRecipeValue(carried->id);

		if (value == 0)
		{
			amount = 0;
		}
		else if (value < 1)
		{
			int baseValue = floor(static_cast<float>(amount) * value);
			if (baseValue < ceil(static_cast<float>(amount) * value) && static_cast<float>(Math::random()) < ((static_cast<float>(amount) * value) - baseValue))
			{
				baseValue++;
			}
			amount = baseValue;
		}

		while (amount > 0)
		{
			int newCount = ExperienceOrb::getExperienceValue(amount);
			amount -= newCount;
			player->level->addEntity(std::make_shared<ExperienceOrb>(player->level, player->x, player->y + .5, player->z + .5, newCount));
		}
	}

#ifdef _DURANGO
	if (!player->level->isClientSide && removeCount > 0)
	{
		player->awardStat(
			GenericStats::itemsSmelted(carried->id),
			GenericStats::param_itemsSmelted(carried->id, carried->getAuxValue(), removeCount)
			);
	}
#else
	if (carried->id == Item::ironIngot_Id) player->awardStat(GenericStats::acquireIron(), GenericStats::param_acquireIron());
	if (carried->id == Item::fish_cooked_Id) player->awardStat(GenericStats::cookFish(), GenericStats::param_cookFish());
#endif

	removeCount = 0;
}
