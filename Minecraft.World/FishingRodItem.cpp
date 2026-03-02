#include "stdafx.h"
#include "net.minecraft.network.packet.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.chunk.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.saveddata.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.h"
#include "ItemInstance.h"
#include "FishingRodItem.h"
#include "SoundTypes.h"

const wstring FishingRodItem::TEXTURE_EMPTY = L"fishingRod_empty";

FishingRodItem::FishingRodItem(int id) : Item(id)
{
	setMaxDamage(64);
	setMaxStackSize(1);
	emptyIcon = NULL;
}

bool FishingRodItem::isHandEquipped() 
{
	return true;
}

bool FishingRodItem::isMirroredArt() 
{
	return true;
}

shared_ptr<ItemInstance> FishingRodItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player) 
{
	if (player->fishing != NULL) 
	{
		int dmg = player->fishing->retrieve();
		instance->hurt(dmg, player);
		player->swing();
	} 
	else 
	{
		level->playSound(player, eSoundType_RANDOM_BOW, 0.5f, 0.4f / (random->nextFloat() * 0.4f + 0.8f));
		if (!level->isClientSide) 
		{
			// 4J Stu - Move the player->fishing out of the ctor as we cannot reference 'this'
			shared_ptr<FishingHook> hook = shared_ptr<FishingHook>( new FishingHook(level, player) );
			player->fishing = hook;
			level->addEntity( shared_ptr<FishingHook>( hook ) );
		}
		player->swing();
	}
	return instance;
}

void FishingRodItem::registerIcons(IconRegister *iconRegister)
{
	Item::registerIcons(iconRegister);
	emptyIcon = iconRegister->registerIcon(TEXTURE_EMPTY);
}

Icon *FishingRodItem::getEmptyIcon()
{
	return emptyIcon;
}
