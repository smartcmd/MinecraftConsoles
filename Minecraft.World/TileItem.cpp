using namespace std;

#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.h"
#include "net.minecraft.stats.h"
#include "TileItem.h"
#include "Facing.h"

// 4J-PB - for the debug option of not removing items
#include <xuiresource.h>
#include <xuiapp.h>


TileItem::TileItem(int id) : Item(id) 
{
	this->tileId = id + 256;
	itemIcon = NULL;
}

int TileItem::getTileId() 
{
	return tileId;
}

int TileItem::getIconType()
{
	if (!Tile::tiles[tileId]->getTileItemIconName().empty())
	{
		return Icon::TYPE_ITEM;
	}
	return Icon::TYPE_TERRAIN;
}

Icon *TileItem::getIcon(int auxValue)
{
	if (itemIcon != NULL)
	{
		return itemIcon;
	}
	return Tile::tiles[tileId]->getTexture(Facing::UP, auxValue);
}

bool TileItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly) 
{
	// 4J-PB - Adding a test only version to allow tooltips to be displayed
	int currentTile = level->getTile(x, y, z);
	if (currentTile == Tile::topSnow_Id && (level->getData(x, y, z) & TopSnowTile::HEIGHT_MASK) < 1) 
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

	if (instance->count == 0) return false;
	if (!player->mayUseItemAt(x, y, z, face, instance)) return false;


	if (y == Level::maxBuildHeight - 1 && Tile::tiles[tileId]->material->isSolid()) return false;

	int undertile = level->getTile(x,y-1,z); // For 'BodyGuard' achievement.

	if (level->mayPlace(tileId, x, y, z, false, face, player, instance)) 
	{
		if(!bTestUseOnOnly)
		{
			Tile *tile = Tile::tiles[tileId];
			// 4J - Adding this from 1.6
			int itemValue = getLevelDataForAuxValue(instance->getAuxValue());
			int dataValue = Tile::tiles[tileId]->getPlacedOnFaceDataValue(level, x, y, z, face, clickX, clickY, clickZ, itemValue);
			if (level->setTileAndData(x, y, z, tileId, dataValue, Tile::UPDATE_ALL)) 
			{
				// 4J-JEV: Snow/Iron Golems do not have owners apparently.
				int newTileId = level->getTile(x,y,z);
				if ( (tileId == Tile::pumpkin_Id || tileId == Tile::litPumpkin_Id) && newTileId == 0 )
				{
					eINSTANCEOF golemType;
					switch (undertile)
					{
					case Tile::ironBlock_Id:	golemType = eTYPE_VILLAGERGOLEM; break;
					case Tile::snow_Id:			golemType = eTYPE_SNOWMAN; break;
					default:					golemType = eTYPE_NOTSET; break;
					}

					if (golemType != eTYPE_NOTSET)
					{
						player->awardStat(GenericStats::craftedEntity(golemType),GenericStats::param_craftedEntity(golemType));
					}
				}

				// 4J-JEV: Hook for durango 'BlockPlaced' event.
				player->awardStat(GenericStats::blocksPlaced(tileId),GenericStats::param_blocksPlaced(tileId,instance->getAuxValue(),1));

				// 4J - Original comment
				// ok this may look stupid, but neighbor updates can cause the
				// placed block to become something else before these methods
				// are called
				if (level->getTile(x, y, z) == tileId)
				{
					Tile::tiles[tileId]->setPlacedBy(level, x, y, z, player, instance);
					Tile::tiles[tileId]->finalizePlacement(level, x, y, z, dataValue);
				}

				// 4J-PB - Java 1.4 change - getStepSound replaced with getPlaceSound
				//level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, tile->soundType->getStepSound(), (tile->soundType->getVolume() + 1) / 2, tile->soundType->getPitch() * 0.8f);
#ifdef _DEBUG
				int iPlaceSound=tile->soundType->getPlaceSound();
				int iStepSound=tile->soundType->getStepSound();

				// 				char szPlaceSoundName[256];
				// 				char szStepSoundName[256];
				// 				Minecraft *pMinecraft = Minecraft::GetInstance();
				// 
				// 				if(iPlaceSound==-1)
				// 				{
				// 					strcpy(szPlaceSoundName,"NULL");
				// 				}
				// 				else
				// 				{
				// 					pMinecraft->soundEngine->GetSoundName(szPlaceSoundName,iPlaceSound);
				// 				}
				// 				if(iStepSound==-1)
				// 				{
				// 					strcpy(szStepSoundName,"NULL");
				// 				}
				// 				else
				// 				{
				// 					pMinecraft->soundEngine->GetSoundName(szStepSoundName,iStepSound);
				// 				}

				//app.DebugPrintf("Place Sound - %s, Step Sound - %s\n",szPlaceSoundName,szStepSoundName);
				app.DebugPrintf("Place Sound - %d, Step Sound - %d\n",iPlaceSound,iStepSound);
#endif
				level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, tile->soundType->getPlaceSound(), (tile->soundType->getVolume() + 1) / 2, tile->soundType->getPitch() * 0.8f);
#ifndef _FINAL_BUILD
				// 4J-PB - If we have the debug option on, don't reduce the number of this item
				if(!(app.DebugSettingsOn() && app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_CraftAnything)))
#endif
				{
					instance->count--;
				}
			}
		}
		return true;
	}
	return false;
}


bool TileItem::mayPlace(Level *level, int x, int y, int z, int face, shared_ptr<Player> player, shared_ptr<ItemInstance> item) 
{
	int currentTile = level->getTile(x, y, z);
	if (currentTile == Tile::topSnow_Id) 
	{
		face = Facing::UP;
	} 
	else if (currentTile != Tile::vine_Id && currentTile != Tile::tallgrass_Id && currentTile != Tile::deadBush_Id) 
	{
		if (face == 0) y--;
		if (face == 1) y++;
		if (face == 2) z--;
		if (face == 3) z++;
		if (face == 4) x--;
		if (face == 5) x++;
	}

	return level->mayPlace(getTileId(), x, y, z, false, face, nullptr, item);
}

// 4J Added to colourise some tile types in the hint popups
int TileItem::getColor(int itemAuxValue, int spriteLayer)
{
	return Tile::tiles[tileId]->getColor();
}

unsigned int TileItem::getDescriptionId(shared_ptr<ItemInstance> instance) 
{
	return Tile::tiles[tileId]->getDescriptionId();
}


unsigned int TileItem::getDescriptionId(int iData /*= -1*/) 
{
	return Tile::tiles[tileId]->getDescriptionId(iData);
}


unsigned int TileItem::getUseDescriptionId(shared_ptr<ItemInstance> instance) 
{
	return Tile::tiles[tileId]->getUseDescriptionId();
}


unsigned int TileItem::getUseDescriptionId() 
{
	return Tile::tiles[tileId]->getUseDescriptionId();
}

void TileItem::registerIcons(IconRegister *iconRegister)
{
	wstring iconName = Tile::tiles[tileId]->getTileItemIconName();
	if (!iconName.empty())
	{
		itemIcon = iconRegister->registerIcon(iconName);
	}
}
