#include "stdafx.h"
#include "MultiPlayerGameMode.h"
#include "CreativeMode.h"
#include "MultiPlayerLocalPlayer.h"
#include "MultiPlayerLevel.h"
#include "Minecraft.h"
#include "ClientConnection.h"
#include "LevelRenderer.h"
#include "Common\Network\GameNetworkManager.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\Minecraft.World\net.minecraft.h"

MultiPlayerGameMode::MultiPlayerGameMode(Minecraft *minecraft, ClientConnection *connection)
{
	// 4J - added initialisers
    xDestroyBlock = -1;
    yDestroyBlock = -1;
    zDestroyBlock = -1;
    destroyingItem = nullptr;
    destroyProgress = 0;
    destroyTicks = 0;
    destroyDelay = 0;
    isDestroying = false;
	carriedItem = 0;
	localPlayerMode = GameType::SURVIVAL;
	this->minecraft = minecraft;
	this->connection = connection;
}

void MultiPlayerGameMode::creativeDestroyBlock(Minecraft *minecraft, MultiPlayerGameMode *gameMode, int x, int y, int z, int face)
{
	if (!minecraft->level->extinguishFire(minecraft->player, x, y, z, face))
	{
		gameMode->destroyBlock(x, y, z, face);
	}
}

void MultiPlayerGameMode::adjustPlayer(shared_ptr<Player> player)
{
	localPlayerMode->updatePlayerAbilities(&player->abilities);
}

bool MultiPlayerGameMode::isCutScene()
{
	return false;
}

void MultiPlayerGameMode::setLocalMode(GameType *mode)
{
	localPlayerMode = mode;
	localPlayerMode->updatePlayerAbilities(&minecraft->player->abilities);
}

void MultiPlayerGameMode::initPlayer(shared_ptr<Player> player)
{
	player->yRot = -180;
}

bool MultiPlayerGameMode::canHurtPlayer()
{
	return localPlayerMode->isSurvival();
}

bool MultiPlayerGameMode::destroyBlock(int x, int y, int z, int face)
{
	if (localPlayerMode->isAdventureRestricted()) {
        if (!minecraft->player->mayDestroyBlockAt(x, y, z)) {
            return false;
        }
    }

	if (localPlayerMode->isCreative())
	{
        if (minecraft->player->getCarriedItem() != NULL && dynamic_cast<WeaponItem *>(minecraft->player->getCarriedItem()->getItem()) != NULL)
		{
            return false;
        }
    }

	Level *level = minecraft->level;
	Tile *oldTile = Tile::tiles[level->getTile(x, y, z)];

	if (oldTile == NULL) return false;

#ifdef _WINDOWS64
	if (g_NetworkManager.IsHost())
	{
		level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, x, y, z, oldTile->id + (level->getData(x, y, z) << Tile::TILE_NUM_SHIFT));
		return true;
	}
#endif

	level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, x, y, z, oldTile->id + (level->getData(x, y, z) << Tile::TILE_NUM_SHIFT));

	int data = level->getData(x, y, z);
	bool changed = level->removeTile(x, y, z);
	if (changed)
	{
		oldTile->destroy(level, x, y, z, data);
	}
	yDestroyBlock = -1;

	if (!localPlayerMode->isCreative())
	{
		shared_ptr<ItemInstance> item = minecraft->player->getSelectedItem();
		if (item != NULL)
		{
			item->mineBlock(level, oldTile->id, x, y, z, minecraft->player);
			if (item->count == 0)
			{
				minecraft->player->removeSelectedItem();
			}
		}
	}

    return changed;
}

void MultiPlayerGameMode::startDestroyBlock(int x, int y, int z, int face)
{	
	if(!minecraft->player->isAllowedToMine()) return;

	if (localPlayerMode->isAdventureRestricted())
	{
        if (!minecraft->player->mayDestroyBlockAt(x, y, z))
		{
            return;
        }
    }

	if (localPlayerMode->isCreative())
	{
		connection->send(shared_ptr<PlayerActionPacket>( new PlayerActionPacket(PlayerActionPacket::START_DESTROY_BLOCK, x, y, z, face) ));
		creativeDestroyBlock(minecraft, this, x, y, z, face);
		destroyDelay = 5;
	}
	else if (!isDestroying || !sameDestroyTarget(x, y, z))
	{
		if (isDestroying)
		{
            connection->send(shared_ptr<PlayerActionPacket>(new PlayerActionPacket(PlayerActionPacket::ABORT_DESTROY_BLOCK, xDestroyBlock, yDestroyBlock, zDestroyBlock, face)));
        }
        connection->send( shared_ptr<PlayerActionPacket>( new PlayerActionPacket(PlayerActionPacket::START_DESTROY_BLOCK, x, y, z, face) ) );
        int t = minecraft->level->getTile(x, y, z);
        if (t > 0 && destroyProgress == 0) Tile::tiles[t]->attack(minecraft->level, x, y, z, minecraft->player);
        if (t > 0 &&
			(Tile::tiles[t]->getDestroyProgress(minecraft->player, minecraft->player->level, x, y, z) >= 1
			// ||(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_InstantDestroy))
			)
			)
		{
            destroyBlock(x, y, z, face);
        }
		else
		{
            isDestroying = true;
            xDestroyBlock = x;
            yDestroyBlock = y;
            zDestroyBlock = z;
			destroyingItem = minecraft->player->getCarriedItem();
            destroyProgress = 0;        
            destroyTicks = 0;
			minecraft->level->destroyTileProgress(minecraft->player->entityId, xDestroyBlock, yDestroyBlock, zDestroyBlock, (int)(destroyProgress * 10) - 1);
        }
    }

}

void MultiPlayerGameMode::stopDestroyBlock()
{
	if (isDestroying)
	{
		connection->send(shared_ptr<PlayerActionPacket>(new PlayerActionPacket(PlayerActionPacket::ABORT_DESTROY_BLOCK, xDestroyBlock, yDestroyBlock, zDestroyBlock, -1)));
	}

	isDestroying = false;
	destroyProgress = 0;
	minecraft->level->destroyTileProgress(minecraft->player->entityId, xDestroyBlock, yDestroyBlock, zDestroyBlock, -1);
}

void MultiPlayerGameMode::continueDestroyBlock(int x, int y, int z, int face)
{
	if(!minecraft->player->isAllowedToMine()) return;
    ensureHasSentCarriedItem();
//        connection.send(new PlayerActionPacket(PlayerActionPacket.CONTINUE_DESTROY_BLOCK, x, y, z, face));

    if (destroyDelay > 0)
	{
        destroyDelay--;
        return;
    }

	if (localPlayerMode->isCreative())
	{
		destroyDelay = 5;
		connection->send(shared_ptr<PlayerActionPacket>( new PlayerActionPacket(PlayerActionPacket::START_DESTROY_BLOCK, x, y, z, face) ) );
		creativeDestroyBlock(minecraft, this, x, y, z, face);
		return;
	}

    if (sameDestroyTarget(x, y, z))
	{
        int t = minecraft->level->getTile(x, y, z);
        if (t == 0)
		{
            isDestroying = false;
            return;
        }
        Tile *tile = Tile::tiles[t];

        destroyProgress += tile->getDestroyProgress(minecraft->player, minecraft->player->level, x, y, z);

        if (destroyTicks % 4 == 0)
		{
            if (tile != NULL)
			{
				int iStepSound=tile->soundType->getStepSound();

				minecraft->soundEngine->play(iStepSound, x + 0.5f, y + 0.5f, z + 0.5f, (tile->soundType->getVolume() + 1) / 8, tile->soundType->getPitch() * 0.5f);
            }
        }

        destroyTicks++;

        if (destroyProgress >= 1)
		{
            isDestroying = false;
            connection->send( shared_ptr<PlayerActionPacket>( new PlayerActionPacket(PlayerActionPacket::STOP_DESTROY_BLOCK, x, y, z, face) ) );
			destroyBlock(x, y, z, face);
            destroyProgress = 0;
            destroyTicks = 0;
            destroyDelay = 5;
        }

		minecraft->level->destroyTileProgress(minecraft->player->entityId, xDestroyBlock, yDestroyBlock, zDestroyBlock, (int)(destroyProgress * 10) - 1);
    }
	else
	{
        startDestroyBlock(x, y, z, face);
    }

}

float MultiPlayerGameMode::getPickRange()
{
	if (localPlayerMode->isCreative())
	{
		return 5.0f;
	}
	return 4.5f;
}

void MultiPlayerGameMode::tick()
{
    ensureHasSentCarriedItem();
    //minecraft->soundEngine->playMusicTick();
}

bool MultiPlayerGameMode::sameDestroyTarget(int x, int y, int z)
{
    shared_ptr<ItemInstance> selected = minecraft->player->getCarriedItem();
    bool sameItems = destroyingItem == NULL && selected == NULL;
    if (destroyingItem != NULL && selected != NULL)
	{
        sameItems = 
			selected->id == destroyingItem->id &&
			ItemInstance::tagMatches(selected, destroyingItem) &&
			(selected->isDamageableItem() || selected->getAuxValue() == destroyingItem->getAuxValue());
    }
    return x == xDestroyBlock && y == yDestroyBlock && z == zDestroyBlock && sameItems;
}

void MultiPlayerGameMode::ensureHasSentCarriedItem()
{
    int newItem = minecraft->player->inventory->selected;
    if (newItem != carriedItem)
	{
        carriedItem = newItem;
        connection->send( shared_ptr<SetCarriedItemPacket>( new SetCarriedItemPacket(carriedItem) ) );
    }
}

bool MultiPlayerGameMode::useItemOn(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, int face, Vec3 *hit, bool bTestUseOnly, bool *pbUsedItem)
{
	if( pbUsedItem ) *pbUsedItem = false;	// Did we actually use the held item?

	// 4J-PB - Adding a test only version to allow tooltips to be displayed
	if(!bTestUseOnly)
	{
		ensureHasSentCarriedItem();
	}
	float clickX = (float) hit->x - x;
	float clickY = (float) hit->y - y;
	float clickZ = (float) hit->z - z;
	bool didSomething = false;

	if (!player->isSneaking() || player->getCarriedItem() == NULL)
	{
		int t = level->getTile(x, y, z);	
		if (t > 0 && player->isAllowedToUse(Tile::tiles[t]))
		{
			if(bTestUseOnly)
			{
				switch(t)
				{
				case Tile::jukebox_Id: 
				case Tile::bed_Id: // special case for a bed
					if (Tile::tiles[t]->TestUse(level, x, y, z, player )) 
					{
						return true;
					}
					else if (t==Tile::bed_Id) // 4J-JEV: You can still use items on record players (ie. set fire to them).
					{
						// bed is too far away, or something
						return false;
					}
				break;
				default:
					if (Tile::tiles[t]->TestUse()) return true;
					break;
				}
			}
			else 
			{
				if (Tile::tiles[t]->use(level, x, y, z, player, face, clickX, clickY, clickZ)) didSomething = true;
			}
		}
	}

	if (!didSomething && item != NULL && dynamic_cast<TileItem *>(item->getItem()))
	{
		TileItem *tile = dynamic_cast<TileItem *>(item->getItem());
		if (!tile->mayPlace(level, x, y, z, face, player, item)) return false;
	}

	// 4J Stu - In Java we send the use packet before the above check for item being NULL
	// so the following never gets executed but the packet still gets sent (for opening chests etc)
	if(item != NULL)
	{
		if(!didSomething && player->isAllowedToUse(item))
		{
			if (localPlayerMode->isCreative())
			{
				int aux = item->getAuxValue();
				int count = item->count;
				didSomething = item->useOn(player, level, x, y, z, face, clickX, clickY, clickZ, bTestUseOnly);
				item->setAuxValue(aux);
				item->count = count;
			}
			else
			{
				didSomething = item->useOn(player, level, x, y, z, face, clickX, clickY, clickZ, bTestUseOnly);
			}
			if( didSomething )
			{
				if( pbUsedItem ) *pbUsedItem = true;
			}
		}
	}
	else
	{
		int t = level->getTile(x, y, z);
		// 4J - Bit of a hack, however seems preferable to any larger changes which would have more chance of causing unwanted side effects. 
		// If we aren't going to be actually performing the use method locally, then call this method with its "soundOnly" parameter set to true.
		// This is an addition from the java version, and as its name suggests, doesn't actually perform the use locally but just makes any sounds that
		// are meant to be directly caused by this. If we don't do this, then the sounds never happen as the tile's use method is only called on the
		// server, and that won't allow any sounds that are directly made, or broadcast back level events to us that would make the sound, since we are
		// the source of the event.
		// ---------------------------------------------------------------------------------
		// Only call soundOnly version if we didn't already call the tile's use method above
		if( !didSomething && ( t > 0 ) && ( !bTestUseOnly ) && player->isAllowedToUse(Tile::tiles[t]) )
		{
			Tile::tiles[t]->use(level, x, y, z, player, face, clickX, clickY, clickZ, true);
		}
	}

	// 4J Stu - Do the action before we send the packet, so that our predicted count is sent in the packet and the server 
	// doesn't think it has to update us
	// Fix for #7904 - Gameplay: Players can dupe torches by throwing them repeatedly into water.
	if(!bTestUseOnly)
	{
		connection->send( shared_ptr<UseItemPacket>( new UseItemPacket(x, y, z, face, player->inventory->getSelected(), clickX, clickY, clickZ) ) );
	}
    return didSomething;
}

bool MultiPlayerGameMode::useItem(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, bool bTestUseOnly)
{
	if(!player->isAllowedToUse(item)) return false;

	// 4J-PB - Adding a test only version to allow tooltips to be displayed
	if(!bTestUseOnly)
	{
		ensureHasSentCarriedItem();
	}

	// 4J Stu - Do the action before we send the packet, so that our predicted count is sent in the packet and the server 
	// doesn't think it has to update us, or can update us if we are wrong
	// Fix for #13120 - Using a bucket of water or lava in the spawn area (centre of the map) causes the inventory to get out of sync
    bool result = false;
	
	// 4J-PB added for tooltips to test use only
	if(bTestUseOnly)
	{
		result = item->TestUse(item, level, player);
	}
	else
	{
		int oldCount = item->count;
		shared_ptr<ItemInstance> itemInstance = item->use(level, player);
		if ((itemInstance != NULL && itemInstance != item) || (itemInstance != NULL && itemInstance->count != oldCount))
		{
			player->inventory->items[player->inventory->selected] = itemInstance;
			if (itemInstance->count == 0)
			{
				player->inventory->items[player->inventory->selected] = nullptr;
			}
			result = true;
		}
	}
	
	if(!bTestUseOnly)
	{
		connection->send( shared_ptr<UseItemPacket>( new UseItemPacket(-1, -1, -1, 255, player->inventory->getSelected(), 0, 0, 0) ) );
	}
    return result;
}

shared_ptr<MultiplayerLocalPlayer> MultiPlayerGameMode::createPlayer(Level *level)
{
	return shared_ptr<MultiplayerLocalPlayer>( new MultiplayerLocalPlayer(minecraft, level, minecraft->user, connection) );
}

void MultiPlayerGameMode::attack(shared_ptr<Player> player, shared_ptr<Entity> entity)
{
    ensureHasSentCarriedItem();
    connection->send( shared_ptr<InteractPacket>( new InteractPacket(player->entityId, entity->entityId, InteractPacket::ATTACK) ) );
    player->attack(entity);
}

bool MultiPlayerGameMode::interact(shared_ptr<Player> player, shared_ptr<Entity> entity)
{
    ensureHasSentCarriedItem();
    connection->send(shared_ptr<InteractPacket>( new InteractPacket(player->entityId, entity->entityId, InteractPacket::INTERACT) ) );
    return player->interact(entity);
}

shared_ptr<ItemInstance> MultiPlayerGameMode::handleInventoryMouseClick(int containerId, int slotNum, int buttonNum, bool quickKeyHeld, shared_ptr<Player> player)
{
    short changeUid = player->containerMenu->backup(player->inventory);

    shared_ptr<ItemInstance> clicked = player->containerMenu->clicked(slotNum, buttonNum, quickKeyHeld?AbstractContainerMenu::CLICK_QUICK_MOVE:AbstractContainerMenu::CLICK_PICKUP, player);
    connection->send( shared_ptr<ContainerClickPacket>( new ContainerClickPacket(containerId, slotNum, buttonNum, quickKeyHeld, clicked, changeUid) ) );

    return clicked;
}

void MultiPlayerGameMode::handleInventoryButtonClick(int containerId, int buttonId)
{
	connection->send(shared_ptr<ContainerButtonClickPacket>( new ContainerButtonClickPacket(containerId, buttonId) ));
}

void MultiPlayerGameMode::handleCreativeModeItemAdd(shared_ptr<ItemInstance> clicked, int slot)
{
	if (localPlayerMode->isCreative())
	{
		connection->send(shared_ptr<SetCreativeModeSlotPacket>( new SetCreativeModeSlotPacket(slot, clicked) ) );
	}
}

void MultiPlayerGameMode::handleCreativeModeItemDrop(shared_ptr<ItemInstance> clicked)
{
	if (localPlayerMode->isCreative() && clicked != NULL)
	{
		connection->send(shared_ptr<SetCreativeModeSlotPacket>( new SetCreativeModeSlotPacket(-1, clicked) ) );
	}
}

void MultiPlayerGameMode::releaseUsingItem(shared_ptr<Player> player)
{
	ensureHasSentCarriedItem();
	connection->send(shared_ptr<PlayerActionPacket>( new PlayerActionPacket(PlayerActionPacket::RELEASE_USE_ITEM, 0, 0, 0, 255) ) );
	player->releaseUsingItem();
}

bool MultiPlayerGameMode::hasExperience()
{
	return localPlayerMode->isSurvival();
}

bool MultiPlayerGameMode::hasMissTime()
{
	return !localPlayerMode->isCreative();
}

bool MultiPlayerGameMode::hasInfiniteItems()
{
	return localPlayerMode->isCreative();
}

bool MultiPlayerGameMode::hasFarPickRange()
{
	return localPlayerMode->isCreative();
}

// Returns true when the inventory is opened from the server-side. Currently
// only happens when the player is riding a horse.
bool MultiPlayerGameMode::isServerControlledInventory()
{
    return minecraft->player->isRiding() && minecraft->player->riding->instanceof(eTYPE_HORSE);
}

bool MultiPlayerGameMode::handleCraftItem(int recipe, shared_ptr<Player> player)
{
    short changeUid = player->containerMenu->backup(player->inventory);

	connection->send( shared_ptr<CraftItemPacket>( new CraftItemPacket(recipe, changeUid) ) );

    return true;
}

void MultiPlayerGameMode::handleDebugOptions(unsigned int uiVal, shared_ptr<Player> player)
{
	player->SetDebugOptions(uiVal);
	connection->send( shared_ptr<DebugOptionsPacket>( new DebugOptionsPacket(uiVal) ) );
}
