using namespace std;

#include "stdafx.h"
#include "Item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "ItemInstance.h"
#include "SeedItem.h"

SeedItem::SeedItem(int id, int resultId, int targetLand) : Item(id)
{
    this->resultId = resultId;
	this->targetLand = targetLand;
}

bool SeedItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	// 4J-PB - Adding a test only version to allow tooltips to be displayed
    if (face != 1) return false;

	if (!player->mayBuild(x, y, z) || !player->mayBuild(x, y + 1, z)) return false;

    int targetType = level->getTile(x, y, z);

    if (targetType == targetLand && level->isEmptyTile(x, y + 1, z)) 
	{
		if(!bTestUseOnOnly)
		{
			level->setTile(x, y + 1, z, resultId);
			instance->count--;
		}
        return true;
    }
    return false;
}