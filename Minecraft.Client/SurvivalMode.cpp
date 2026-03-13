#include "stdafx.h"
#include "SurvivalMode.h"
#include "DemoMode.h"
#include "LevelRenderer.h"
#include "LocalPlayer.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/net.minecraft.world.entity.player.h"
#include "../Minecraft.World/net.minecraft.world.item.h"
#include "ClientConstants.h"

SurvivalMode::SurvivalMode(Minecraft *minecraft) : GameMode(minecraft)
{
	// 4J - added initialisers
	xDestroyBlock = -1;
    yDestroyBlock = -1;
    zDestroyBlock = -1;
    destroyProgress = 0;
    oDestroyProgress = 0;
    destroyTicks = 0;
    destroyDelay = 0;

   if (ClientConstants::IS_DEMO_VERSION)
	{
		if( dynamic_cast<DemoMode *>(this) == NULL )
		{
			assert(false);
//            throw new IllegalStateException("Invalid game mode");		// 4J - removed
        }
    }
}

// 4J Stu - Added this ctor so we can exit the tutorial and replace it with a standard
// survival mode
SurvivalMode::SurvivalMode(SurvivalMode *copy) : GameMode( copy->minecraft )
{
	xDestroyBlock = copy->xDestroyBlock;
	yDestroyBlock = copy->yDestroyBlock;
	zDestroyBlock = copy->zDestroyBlock;
	destroyProgress = copy->destroyProgress;
	oDestroyProgress = copy->oDestroyProgress;
	destroyTicks = copy->destroyTicks;
    destroyDelay = copy->destroyDelay;
}

void SurvivalMode::initPlayer(shared_ptr<Player> player)
{
	player->yRot = -180;
}

void SurvivalMode::init()
{
}

bool SurvivalMode::canHurtPlayer()
{
	return true;
}

bool SurvivalMode::destroyBlock(int x, int y, int z, int face)
{
    int t = minecraft->level->getTile(x, y, z);
    int data = minecraft->level->getData(x, y, z);
    bool changed = GameMode::destroyBlock(x, y, z, face);

    shared_ptr<ItemInstance> item = minecraft->player->getSelectedItem();
    bool couldDestroy = minecraft->player->canDestroy(Tile::tiles[t]);
    if (item != NULL)
	{
        item->mineBlock(t, x, y, z, minecraft->player);
        if (item->count == 0)
		{
            minecraft->player->removeSelectedItem();
        }
    }
    if (changed && couldDestroy) 
	{
		Tile::tiles[t]->playerDestroy(minecraft->level, minecraft->player, x, y, z, data);
	}
    return changed;

}

void SurvivalMode::startDestroyBlock(int x, int y, int z, int face)
{
	if (!minecraft->player->mayBuild(x, y, z)) return;
    minecraft->level->extinguishFire(minecraft->player, x, y, z, face);
    int t = minecraft->level->getTile(x, y, z);
    if (t > 0 && destroyProgress == 0) Tile::tiles[t]->attack(minecraft->level, x, y, z, minecraft->player);
    if (t > 0 && Tile::tiles[t]->getDestroyProgress(minecraft->player) >= 1)
	{
        destroyBlock(x, y, z, face);
    }
}

void SurvivalMode::stopDestroyBlock()
{
    destroyProgress = 0;
    destroyDelay = 0;
}

void SurvivalMode::continueDestroyBlock(int x, int y, int z, int face)
{
    if (destroyDelay > 0)
	{
        destroyDelay--;
        return;
    }
    if (x == xDestroyBlock && y == yDestroyBlock && z == zDestroyBlock)
	{
        int t = minecraft->level->getTile(x, y, z);
		if (!minecraft->player->mayBuild(x, y, z)) return;
        if (t == 0) return;
        Tile *tile = Tile::tiles[t];

        destroyProgress += tile->getDestroyProgress(minecraft->player);

        if (destroyTicks % 4 == 0)
		{
            if (tile != NULL)
			{
                minecraft->soundEngine->play(tile->soundType->getStepSound(), x + 0.5f, y + 0.5f, z + 0.5f, (tile->soundType->getVolume() + 1) / 8, tile->soundType->getPitch() * 0.5f);
            }
        }

        destroyTicks++;

        if (destroyProgress >= 1)
		{
            destroyBlock(x, y, z, face);
            destroyProgress = 0;
            oDestroyProgress = 0;
            destroyTicks = 0;
            destroyDelay = 5;
        }
    }
	else
	{
        destroyProgress = 0;
        oDestroyProgress = 0;
        destroyTicks = 0;
        xDestroyBlock = x;
        yDestroyBlock = y;
        zDestroyBlock = z;
    }

}

void SurvivalMode::render(float a)
{
    if (destroyProgress <= 0)
	{
        minecraft->gui->progress = 0;
        minecraft->levelRenderer->destroyProgress = 0;
    }
	else
	{
        float dp = oDestroyProgress + (destroyProgress - oDestroyProgress) * a;
        minecraft->gui->progress = dp;
        minecraft->levelRenderer->destroyProgress = dp;
    }
}

float SurvivalMode::getPickRange()
{
	return 4.0f;
}

void SurvivalMode::initLevel(Level *level)
{
	GameMode::initLevel(level);
}

shared_ptr<Player> SurvivalMode::createPlayer(Level *level)
{
	shared_ptr<Player> player = GameMode::createPlayer(level);
	//        player.inventory.add(new ItemInstance(Item.pickAxe_diamond));
	//        player.inventory.add(new ItemInstance(Item.hatchet_diamond));
	//        player.inventory.add(new ItemInstance(Tile.torch, 64));
	//        player.inventory.add(new ItemInstance(Item.porkChop_cooked, 4));
	//        player.inventory.add(new ItemInstance(Item.bow, 1));
	//        player.inventory.add(new ItemInstance(Item.arrow, 64));
	return player;
}

void SurvivalMode::tick()
{
    oDestroyProgress = destroyProgress;
    //minecraft->soundEngine->playMusicTick();
}

bool SurvivalMode::useItemOn(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, int face, bool bTestUseOnOnly, bool *pbUsedItem)
{
	int t = level->getTile(x, y, z);
	if (t > 0)
	{
		if (Tile::tiles[t]->use(level, x, y, z, player)) return true;
	}
	if (item == NULL) return false;
	return item->useOn(player, level, x, y, z, face);
}

bool SurvivalMode::hasExperience()
{
	return true;
}