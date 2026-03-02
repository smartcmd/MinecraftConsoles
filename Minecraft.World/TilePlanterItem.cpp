#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.h"
#include "TilePlanterItem.h"
#include "GenericStats.h"
// 4J-PB - for the debug option of not removing items
#include <xuiresource.h>
#include <xuiapp.h>

TilePlanterItem::TilePlanterItem(int id, Tile *tile) : Item(id)
{
    this->tileId = tile->id;
}

bool TilePlanterItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	// 4J-PB - Adding a test only version to allow tooltips to be displayed
	int currentTile = level->getTile(x, y, z);
    if (currentTile == Tile::topSnow_Id) 
	{
        face = Facing::UP;
    } 
	else if (currentTile == Tile::vine_Id || currentTile == Tile::tallgrass_Id || currentTile == Tile::deadBush_Id)
	{
	}
	else
	{
        if (face == 0) y--;
        if (face == 1) y++;
        if (face == 2) z--;
        if (face == 3) z++;
        if (face == 4) x--;
        if (face == 5) x++;
    }

	if (!player->mayBuild(x, y, z)) return false;
    if (instance->count == 0) return false;

	if (level->mayPlace(tileId, x, y, z, false, face, nullptr)) 
	{
		if(!bTestUseOnOnly)
		{
			Tile *tile = Tile::tiles[tileId];
			int dataValue = tile->getPlacedOnFaceDataValue(level, x, y, z, face, clickX, clickY, clickZ, 0);
			if (level->setTileAndData(x, y, z, tileId, dataValue))
			{
				// 4J-JEV: Hook for durango 'BlockPlaced' event.
				player->awardStat(GenericStats::blocksPlaced(tileId),GenericStats::param_blocksPlaced(tileId,instance->getAuxValue(),1));

				// 4J Original comment
				// ok this may look stupid, but neighbor updates can cause the
				// placed block to become something else before these methods
				// are called
				if (level->getTile(x, y, z) == tileId)
				{
					Tile::tiles[tileId]->setPlacedBy(level, x, y, z, player);
					Tile::tiles[tileId]->finalizePlacement(level, x, y, z, dataValue);
				}
				level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, tile->soundType->getStepSound(), (tile->soundType->getVolume() + 1) / 2, tile->soundType->getPitch() * 0.8f);
				// 4J-PB - If we have the debug option on, don't reduce the number of this item
	#ifndef _FINAL_BUILD
				if(!(app.DebugSettingsOn() && app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_CraftAnything)))
	#endif				
				{
					instance->count--;
				}

			}
		}		
	}
	else
	{
		// Can't place, so return false
		if(bTestUseOnOnly) return false;
	}
    return true;
}
