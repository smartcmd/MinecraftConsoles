#include "stdafx.h"
#include "CreativeMode.h"
#include "User.h"
#include "LocalPlayer.h"
#include "..\Minecraft.World\\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"

CreativeMode::CreativeMode(Minecraft *minecraft) : GameMode(minecraft)
{
	destroyDelay = 0;
	instaBuild = true;
}

void CreativeMode::init()
{
	// initPlayer();
}

void CreativeMode::enableCreativeForPlayer(shared_ptr<Player> player)
{
	// please check ServerPlayerGameMode.java if you change these
	player->abilities.mayfly = true;
	player->abilities.instabuild = true;
	player->abilities.invulnerable = true;
}

void CreativeMode::disableCreativeForPlayer(shared_ptr<Player> player)
{
	player->abilities.mayfly = false;
	player->abilities.flying = false;
	player->abilities.instabuild = false;
	player->abilities.invulnerable = false;
}

void CreativeMode::adjustPlayer(shared_ptr<Player> player)
{
	enableCreativeForPlayer(player);

    for (int i = 0; i < 9; i++)
	{
        if (player->inventory->items[i] == NULL)
		{
            player->inventory->items[i] = shared_ptr<ItemInstance>( new ItemInstance(User::allowedTiles[i]) );
        }
		else
		{
			// 4J-PB - this line is commented out in 1.0.1
            //player->inventory->items[i]->count = 1;
        }
    }
}

void CreativeMode::creativeDestroyBlock(Minecraft *minecraft, GameMode *gameMode, int x, int y, int z, int face)
{
	if(!minecraft->level->extinguishFire(minecraft->player, x, y, z, face))
	{
		gameMode->destroyBlock(x, y, z, face);
	}
}

bool CreativeMode::useItemOn(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, int face, bool bTestUseOnOnly, bool *pbUsedItem)
{
	int t = level->getTile(x, y, z);
	if (t > 0)
	{
		if (Tile::tiles[t]->use(level, x, y, z, player)) return true;
	}
	if (item == NULL) return false;
	int aux = item->getAuxValue();
	int count = item->count;
	bool success = item->useOn(player, level, x, y, z, face);
	item->setAuxValue(aux);
	item->count = count;
	return success;
}

void CreativeMode::startDestroyBlock(int x, int y, int z, int face)
{
	creativeDestroyBlock(minecraft, this, x, y, z, face);
	destroyDelay = 5;
}

void CreativeMode::continueDestroyBlock(int x, int y, int z, int face)
{
	destroyDelay--;
	if (destroyDelay <= 0)
	{
		destroyDelay = 5;
		creativeDestroyBlock(minecraft, this, x, y, z, face);
	}
}

void CreativeMode::stopDestroyBlock()
{
}

bool CreativeMode::canHurtPlayer()
{
	return false;
}

void CreativeMode::initLevel(Level *level)
{
	GameMode::initLevel(level);
}

float CreativeMode::getPickRange()
{
	return 5.0f;
}

bool CreativeMode::hasMissTime()
{
	return false;
}

bool CreativeMode::hasInfiniteItems()
{
	return true;
}

bool CreativeMode::hasFarPickRange()
{
	return true;
}