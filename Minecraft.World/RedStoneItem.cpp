#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "ItemInstance.h"
#include "GenericStats.h"
#include "RedstoneItem.h"

RedStoneItem::RedStoneItem(int id) : Item(id)
{
}

bool RedStoneItem::useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly) 
{
	// 4J-PB - Adding a test only version to allow tooltips to be displayed
	if (level->getTile(x, y, z) != Tile::topSnow_Id)
	{
		if (face == 0) y--;
		if (face == 1) y++;
		if (face == 2) z--;
		if (face == 3) z++;
		if (face == 4) x--;
		if (face == 5) x++;
		if (!level->isEmptyTile(x, y, z)) return false;
	}
	if (!player->mayUseItemAt(x, y, z, face, itemInstance)) return false;
	if (Tile::redStoneDust->mayPlace(level, x, y, z))
	{
		if(!bTestUseOnOnly)
		{	
			// 4J-JEV: Hook for durango 'BlockPlaced' event.
			player->awardStat(GenericStats::blocksPlaced(Tile::redStoneDust_Id), GenericStats::param_blocksPlaced(Tile::redStoneDust_Id,itemInstance->getAuxValue(),1));

			itemInstance->count--;
			level->setTileAndUpdate(x, y, z, Tile::redStoneDust_Id);
		}
	}

	return true;
}
