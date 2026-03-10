#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "SoundTypes.h"
#include "ExperienceItem.h"

ExperienceItem::ExperienceItem(int id) : Item(id)
{
}

bool ExperienceItem::isFoil(shared_ptr<ItemInstance> itemInstance)
{
	return true;
}

bool ExperienceItem::TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	return true;
}

shared_ptr<ItemInstance> ExperienceItem::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	if (!player->abilities.instabuild)
	{
		itemInstance->count--;
	}
	level->playEntitySound(player, eSoundType_RANDOM_BOW, 0.5f, 0.4f / (random->nextFloat() * 0.4f + 0.8f));
	if (!level->isClientSide) level->addEntity(std::make_shared<ThrownExpBottle>(level, player));
	return itemInstance;
}