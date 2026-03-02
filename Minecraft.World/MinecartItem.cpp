#include "stdafx.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "ItemInstance.h"
#include "MinecartItem.h"

MinecartItem::MinecartItem(int id, int type) : Item(id)
{
	this->maxStackSize = 1;
	this->type = type;
}

bool MinecartItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	// 4J-PB - Adding a test only version to allow tooltips to be displayed
	int targetType = level->getTile(x, y, z);

	if (RailTile::isRail(targetType)) 
	{
		if(!bTestUseOnOnly)
		{
			if (!level->isClientSide) 
			{
				level->addEntity(shared_ptr<Minecart>( new Minecart(level, x + 0.5f, y + 0.5f, z + 0.5f, type) ) );
			}
			instance->count--;
		}
		return true;
	}
	return false;
}