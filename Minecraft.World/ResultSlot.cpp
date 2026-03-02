#include "stdafx.h"
#include "Container.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "ResultSlot.h"

ResultSlot::ResultSlot(Player *player, shared_ptr<Container> craftSlots, shared_ptr<Container> container, int id, int x, int y) : Slot( container, id, x, y )
{
	this->player = player;
	this->craftSlots = craftSlots;
	removeCount = 0;
}

bool ResultSlot::mayPlace(shared_ptr<ItemInstance> item)
{
	return false;
}

shared_ptr<ItemInstance> ResultSlot::remove(int c)
{
	if (hasItem())
	{
		removeCount += min(c, getItem()->count);
	}
	return Slot::remove(c);
}

void ResultSlot::onQuickCraft(shared_ptr<ItemInstance> picked, int count)
{
	removeCount += count;
	checkTakeAchievements(picked);
}

void ResultSlot::checkTakeAchievements(shared_ptr<ItemInstance> carried)
{
	carried->onCraftedBy(player->level, dynamic_pointer_cast<Player>( player->shared_from_this() ), removeCount);
	removeCount = 0;

	if (carried->id == Tile::workBench_Id)				player->awardStat(GenericStats::buildWorkbench(),		GenericStats::param_buildWorkbench());
	else if (carried->id == Item::pickAxe_wood_Id)		player->awardStat(GenericStats::buildPickaxe(),			GenericStats::param_buildPickaxe());
	else if (carried->id == Tile::furnace_Id)			player->awardStat(GenericStats::buildFurnace(),			GenericStats::param_buildFurnace());
	else if (carried->id == Item::hoe_wood_Id)			player->awardStat(GenericStats::buildHoe(),				GenericStats::param_buildHoe());
	else if (carried->id == Item::bread_Id)				player->awardStat(GenericStats::makeBread(),			GenericStats::param_makeBread());
	else if (carried->id == Item::cake_Id)				player->awardStat(GenericStats::bakeCake(),				GenericStats::param_bakeCake());
	else if (carried->id == Item::pickAxe_stone_Id)		player->awardStat(GenericStats::buildBetterPickaxe(),	GenericStats::param_buildBetterPickaxe());
	else if (carried->id == Item::sword_wood_Id)		player->awardStat(GenericStats::buildSword(),			GenericStats::param_buildSword());
	//else if (carried->id == Tile::enchantTable_Id)	player->awardStat(GenericStats::enchantments(),			GenericStats::param_achievement(eAward_));
	else if (carried->id == Tile::bookshelf_Id)			player->awardStat(GenericStats::bookcase(),				GenericStats::param_bookcase());

	// 4J : WESTY : Added new acheivements. 
	else if (carried->id == Tile::dispenser_Id) player->awardStat(GenericStats::dispenseWithThis(), GenericStats::param_dispenseWithThis());
}

void ResultSlot::onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried)
{
	checkTakeAchievements(carried);

	for (unsigned int i = 0; i < craftSlots->getContainerSize(); i++)
	{
		shared_ptr<ItemInstance> item = craftSlots->getItem(i);
		if (item != NULL)
		{
			craftSlots->removeItem(i, 1);

			if (item->getItem()->hasCraftingRemainingItem())
			{
				shared_ptr<ItemInstance> craftResult = shared_ptr<ItemInstance>(new ItemInstance(item->getItem()->getCraftingRemainingItem()));

				/*
				* Try to place this in the player's inventory (See we.java for new method)
				*/
				if (item->getItem()->shouldMoveCraftingResultToInventory(item) && player->inventory->add(craftResult))
				{
					continue;
				}

				// If this slot is now empty, place it there (current behavior)
				if (craftSlots->getItem(i) == NULL)
				{
					craftSlots->setItem(i, craftResult);
				}
				else
				{
					// Finally, if nothing else, just drop the item
					player->drop(craftResult);
				}
			}

		}
	}
}

bool ResultSlot::mayCombine(shared_ptr<ItemInstance> second)
{
	return false;
}
