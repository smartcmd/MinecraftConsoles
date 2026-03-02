#include "stdafx.h"
#include "GameMode.h"
#include "LocalPlayer.h"
#include "LevelRenderer.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.h"

GameMode::GameMode(Minecraft *minecraft)
{
	instaBuild = false;	// 4J - added
	this->minecraft = minecraft;
}

void GameMode::initLevel(Level *level)
{
}

bool GameMode::destroyBlock(int x, int y, int z, int face)
{
    Level *level = minecraft->level;
    Tile *oldTile = Tile::tiles[level->getTile(x, y, z)];
	if (oldTile == NULL) return false;

	//  4J - Let the rendering side of thing know we are about to destroy the tile, so we can synchronise collision with async render data upates.
	minecraft->levelRenderer->destroyedTileManager->destroyingTileAt(level, x, y, z);
    level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, x, y, z, oldTile->id + (level->getData(x, y, z) << Tile::TILE_NUM_SHIFT));
    int data = level->getData(x, y, z);
	// 4J - before we remove the tile, recalc the heightmap - setTile depends on this being valid to be able to do
	// a quick update of skylighting when the block is removed, and there are cases with falling tiles where this can get out of sync
	level->getChunkAt(x,z)->recalcHeightmapOnly();
    bool changed = level->setTile(x, y, z, 0);

    if (oldTile != NULL && changed)
	{
        oldTile->destroy(level, x, y, z, data);
    }
    return changed;
}

void GameMode::render(float a)
{
}

bool GameMode::useItem(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, bool bTestUseOnly)
{
}

void GameMode::initPlayer(shared_ptr<Player> player)
{
}

void GameMode::tick()
{
}

void GameMode::adjustPlayer(shared_ptr<Player> player)
{
}

//bool GameMode::useItemOn(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, int face, bool bTestUseOnOnly)
//{
//	// 4J-PB - Adding a test only version to allow tooltips to be displayed
//	int t = level->getTile(x, y, z);
//	if (t > 0)
//	{
//		if(bTestUseOnOnly)
//		{
//			switch(t)
//			{
//			case Tile::recordPlayer_Id: 
//			case Tile::bed_Id: // special case for a bed
//				if (Tile::tiles[t]->TestUse(level, x, y, z, player )) 
//				{
//					return true;
//				}
//				else
//				{
//					// bed is too far away, or something
//					return false;
//				}
//			break;
//			default:
//				if (Tile::tiles[t]->TestUse()) return true;
//				break;
//			}
//		}
//		else 
//		{
//			if (Tile::tiles[t]->use(level, x, y, z, player )) return true;
//		}
//	}
//	
//    if (item == NULL) return false;
//    return item->useOn(player, level, x, y, z, face, bTestUseOnOnly);
//}


shared_ptr<Player> GameMode::createPlayer(Level *level)
{
	return shared_ptr<Player>( new LocalPlayer(minecraft, level, minecraft->user, level->dimension->id) );
}

bool GameMode::interact(shared_ptr<Player> player, shared_ptr<Entity> entity)
{
	return player->interact(entity);
}

void GameMode::attack(shared_ptr<Player> player, shared_ptr<Entity> entity)
{
	player->attack(entity);
}

shared_ptr<ItemInstance> GameMode::handleInventoryMouseClick(int containerId, int slotNum, int buttonNum, bool quickKeyHeld, shared_ptr<Player> player)
{
	return nullptr;
}

void GameMode::handleCloseInventory(int containerId, shared_ptr<Player> player)
{
    player->containerMenu->removed(player);
	delete player->containerMenu;
    player->containerMenu = player->inventoryMenu;
}

void GameMode::handleInventoryButtonClick(int containerId, int buttonId)
{

}

bool GameMode::isCutScene()
{
	return false;
}

void GameMode::releaseUsingItem(shared_ptr<Player> player)
{
	player->releaseUsingItem();
}

bool GameMode::hasExperience()
{
	return false;
}

bool GameMode::hasMissTime()
{
	return true;
}

bool GameMode::hasInfiniteItems()
{
	return false;
}

bool GameMode::hasFarPickRange()
{
	return false;
}

void GameMode::handleCreativeModeItemAdd(shared_ptr<ItemInstance> clicked, int i)
{
}

void GameMode::handleCreativeModeItemDrop(shared_ptr<ItemInstance> clicked)
{
}

bool GameMode::handleCraftItem(int recipe, shared_ptr<Player> player)
{
	return true;
}

// 4J-PB
void GameMode::handleDebugOptions(unsigned int uiVal, shared_ptr<Player> player)
{
	player->SetDebugOptions(uiVal);
}
