#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.stats.h"
#include "Material.h"
#include "ItemInstance.h"
#include "FlintAndSteelItem.h"
#include "SoundTypes.h"

FlintAndSteelItem::FlintAndSteelItem(int id) : Item( id )
{
	maxStackSize = 1;
	setMaxDamage(64);
}

bool FlintAndSteelItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly) 
{
	// 4J-PB - Adding a test only version to allow tooltips to be displayed
	if (face == 0) y--;
	if (face == 1) y++;
	if (face == 2) z--;
	if (face == 3) z++;
	if (face == 4) x--;
	if (face == 5) x++;
	
	if (!player->mayBuild(x, y, z)) return false;

	int targetType = level->getTile(x, y, z);

	if(!bTestUseOnOnly)
	{	
		if (targetType == 0) 
		{
			if( level->getTile(x, y-1, z) == Tile::obsidian_Id )
			{
				if( Tile::portalTile->trySpawnPortal(level, x, y, z, false) )
				{
					player->awardStat(
						GenericStats::portalsCreated(),
						GenericStats::param_noArgs()
						);

					// 4J : WESTY : Added for achievement.
					player->awardStat(GenericStats::InToTheNether(),GenericStats::param_InToTheNether());
				}
			}

			level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_FIRE_IGNITE, 1, random->nextFloat() * 0.4f + 0.8f);
			level->setTile(x, y, z, Tile::fire_Id);
		}

		instance->hurt(1, player);
	}
	else
	{
		if(targetType == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	// 4J-PB - this function shouldn't really return true all the time, but I've added a special case for my test use for the tooltips display
	// and will leave it as is for the game use

	return true;
}
