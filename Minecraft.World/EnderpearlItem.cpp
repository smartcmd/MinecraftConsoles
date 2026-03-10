#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "EnderpearlItem.h"
#include "SoundTypes.h"

EnderpearlItem::EnderpearlItem(int id) : Item(id)
{
	maxStackSize = 16;
}

bool EnderpearlItem::TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	return true;
}

shared_ptr<ItemInstance> EnderpearlItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	// 4J-PB - Not sure why this was disabled for creative mode, so commenting out
	//if (player->abilities.instabuild) return instance;
	if (player->riding != nullptr) return instance;
	if (!player->abilities.instabuild)
	{
		instance->count--;
	}

	level->playEntitySound(player, eSoundType_RANDOM_BOW, 0.5f, 0.4f / (random->nextFloat() * 0.4f + 0.8f));
	if (!level->isClientSide) 
	{
		level->addEntity(std::make_shared<ThrownEnderpearl>(level, player));
	}
	return instance;
}