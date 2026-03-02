#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.player.h"
#include "MilkBucketItem.h"

MilkBucketItem::MilkBucketItem(int id) : Item( id )
{
	setMaxStackSize(1);
}

shared_ptr<ItemInstance> MilkBucketItem::useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	if (!player->abilities.instabuild) instance->count--;

	if (!level->isClientSide)
	{
		player->removeAllEffects();
	}

	if (instance->count <= 0)
	{
		return shared_ptr<ItemInstance>( new ItemInstance(Item::bucket_empty) );
	}
	return instance;
}

int MilkBucketItem::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return DRINK_DURATION;
}

UseAnim MilkBucketItem::getUseAnimation(shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_drink;
}

shared_ptr<ItemInstance> MilkBucketItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	player->startUsingItem(instance, getUseDuration(instance));
	return instance;
}