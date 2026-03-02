using namespace std;

#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.global.h"
#include "net.minecraft.world.entity.projectile.h"
#include "EggItem.h"
#include "SoundTypes.h"


EggItem::EggItem(int id) : Item( id )
{
        this->maxStackSize = 16;
}

shared_ptr<ItemInstance> EggItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player) 
{
	if (!player->abilities.instabuild)
	{
		instance->count--;
	}
    level->playSound( dynamic_pointer_cast<Entity>(player), eSoundType_RANDOM_BOW, 0.5f, 0.4f / (random->nextFloat() * 0.4f + 0.8f));
    if (!level->isClientSide) level->addEntity( shared_ptr<ThrownEgg>( new ThrownEgg(level, dynamic_pointer_cast<Mob>( player )) ));
    return instance;
}
