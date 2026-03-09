#include "stdafx.h"
#include "ServerPlayerGameMode.h"
#include "ServerLevel.h"
#include "ServerPlayer.h"
#include "PlayerConnection.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "MultiPlayerLevel.h"
#include "LevelRenderer.h"

ServerPlayerGameMode::ServerPlayerGameMode(Level *level)
{
	// 4J - added initialisers
	isDestroyingBlock = false;
	destroyProgressStart = 0;
	xDestroyBlock = yDestroyBlock = zDestroyBlock = 0;
	gameTicks = 0;
	hasDelayedDestroy = false;
	delayedDestroyX = delayedDestroyY = delayedDestroyZ = 0;
	delayedTickStart = 0;
	lastSentState = -1;
	gameModeForPlayer = GameType::NOT_SET;

	this->level = level;

	// 4J Added
	m_gameRules = nullptr;
}

ServerPlayerGameMode::~ServerPlayerGameMode()
{
	if(m_gameRules!=nullptr) delete m_gameRules;
}

void ServerPlayerGameMode::setGameModeForPlayer(GameType *gameModeForPlayer)
{
	this->gameModeForPlayer = gameModeForPlayer;

	gameModeForPlayer->updatePlayerAbilities(&(player->abilities));
	player->onUpdateAbilities();

}

GameType *ServerPlayerGameMode::getGameModeForPlayer()
{
	return gameModeForPlayer;
}

bool ServerPlayerGameMode::isSurvival()
{
	return gameModeForPlayer->isSurvival();
}

bool ServerPlayerGameMode::isCreative()
{
	return gameModeForPlayer->isCreative();
}

void ServerPlayerGameMode::updateGameMode(GameType *gameType)
{
	if (gameModeForPlayer == GameType::NOT_SET)
	{
		gameModeForPlayer = gameType;
	}
	setGameModeForPlayer(gameModeForPlayer);
}

void ServerPlayerGameMode::tick()
{
	gameTicks++;

	if (hasDelayedDestroy)
	{
		int ticksSpentDestroying = gameTicks - delayedTickStart;
		int t = level->getTile(delayedDestroyX, delayedDestroyY, delayedDestroyZ);
		if (t == 0)
		{
			hasDelayedDestroy = false;
		}
		else
		{
			Tile *tile = Tile::tiles[t];
			float destroyProgress = tile->getDestroyProgress(player, player->level, delayedDestroyX, delayedDestroyY, delayedDestroyZ) * (ticksSpentDestroying + 1);
			int state = static_cast<int>(destroyProgress * 10);

			if (state != lastSentState)
			{
				level->destroyTileProgress(player->entityId, delayedDestroyX, delayedDestroyY, delayedDestroyZ, state);
				lastSentState = state;
			}
			if (destroyProgress >= 1)
			{
				hasDelayedDestroy = false;
				destroyBlock(delayedDestroyX, delayedDestroyY, delayedDestroyZ);
			}
		}
	}
	else if (isDestroyingBlock)
	{
		int t = level->getTile(xDestroyBlock, yDestroyBlock, zDestroyBlock);
		Tile *tile = Tile::tiles[t];

		if (tile == nullptr)
		{
			level->destroyTileProgress(player->entityId, xDestroyBlock, yDestroyBlock, zDestroyBlock, -1);
			lastSentState = -1;
			isDestroyingBlock = false;
		}
		else
		{
			int ticksSpentDestroying = gameTicks - destroyProgressStart;
			float destroyProgress = tile->getDestroyProgress(player, player->level, xDestroyBlock, yDestroyBlock, zDestroyBlock) * (ticksSpentDestroying + 1);
			int state = static_cast<int>(destroyProgress * 10);

			if (state != lastSentState)
			{
				level->destroyTileProgress(player->entityId, xDestroyBlock, yDestroyBlock, zDestroyBlock, state);
				lastSentState = state;
			}
		}
	}
}

void ServerPlayerGameMode::startDestroyBlock(int x, int y, int z, int face)
{
	if(!player->isAllowedToMine()) return;

	if (gameModeForPlayer->isAdventureRestricted())
	{
		if (!player->mayDestroyBlockAt(x, y, z))
		{
			return;
		}
	}

	if (isCreative())
	{
		if(!level->extinguishFire(nullptr, x, y, z, face))
		{
			destroyBlock(x, y, z);
		}
		return;
	}
	level->extinguishFire(player, x, y, z, face);
	destroyProgressStart = gameTicks;
	float progress = 1.0f;
	int t = level->getTile(x, y, z);
	if (t > 0)
	{
		Tile::tiles[t]->attack(level, x, y, z, player);
		progress = Tile::tiles[t]->getDestroyProgress(player, player->level, x, y, z);
	}

	if (t > 0 && (progress >= 1 ) ) //|| (app.DebugSettingsOn() && (player->GetDebugOptions()&(1L<<eDebugSetting_InstantDestroy) ) )))
	{
		destroyBlock(x, y, z);
	}
	else
	{
		isDestroyingBlock = true;
		xDestroyBlock = x;
		yDestroyBlock = y;
		zDestroyBlock = z;
		int state = static_cast<int>(progress * 10);
		level->destroyTileProgress(player->entityId, x, y, z, state);
		lastSentState = state;
	}
}

void ServerPlayerGameMode::stopDestroyBlock(int x, int y, int z)
{
	if (x == xDestroyBlock && y == yDestroyBlock && z == zDestroyBlock)
	{
		//         int ticksSpentDestroying = gameTicks - destroyProgressStart;

		int t = level->getTile(x, y, z);
		if (t != 0)
		{
			Tile *tile = Tile::tiles[t];

			// MGH -	removed checking for the destroy progress here, it has already been checked on the client before it sent the packet.
			//			fixes issues with this failing to destroy because of packets bunching up
			//             float destroyProgress = tile->getDestroyProgress(player, player->level, x, y, z) * (ticksSpentDestroying + 1);
			//             if (destroyProgress >= .7f || bIgnoreDestroyProgress)
			{
				isDestroyingBlock = false;
				level->destroyTileProgress(player->entityId, x, y, z, -1);
				destroyBlock(x, y, z);
			}
			// 			else if (!hasDelayedDestroy)
			// 			{
			// 				isDestroyingBlock = false;
			//                 hasDelayedDestroy = true;
			//                 delayedDestroyX = x;
			//                 delayedDestroyY = y;
			//                 delayedDestroyZ = z;
			//                 delayedTickStart = destroyProgressStart;
			//             }
		}
	}
}

void ServerPlayerGameMode::abortDestroyBlock(int x, int y, int z)
{
	isDestroyingBlock = false;
	level->destroyTileProgress(player->entityId, xDestroyBlock, yDestroyBlock, zDestroyBlock, -1);
}

bool ServerPlayerGameMode::superDestroyBlock(int x, int y, int z)
{
	Tile *oldTile = Tile::tiles[level->getTile(x, y, z)];
	int data = level->getData(x, y, z);

	if (oldTile != nullptr)
	{
		oldTile->playerWillDestroy(level, x, y, z, data, player);
	}

	bool changed = level->removeTile(x, y, z);
	if (oldTile != nullptr && changed)
	{
		oldTile->destroy(level, x, y, z, data);
	}
	return changed;
}

bool ServerPlayerGameMode::destroyBlock(int x, int y, int z)
{
	if (gameModeForPlayer->isAdventureRestricted())
	{
		if (!player->mayDestroyBlockAt(x, y, z))
		{
			return false;
		}
	}

	if (gameModeForPlayer->isCreative())
	{
		if (player->getCarriedItem() != nullptr && dynamic_cast<WeaponItem *>(player->getCarriedItem()->getItem()) != nullptr)
		{
			return false;
		}
	}

	int t = level->getTile(x, y, z);
	int data = level->getData(x, y, z);

	level->levelEvent(player, LevelEvent::PARTICLES_DESTROY_BLOCK, x, y, z, t + (level->getData(x, y, z) << Tile::TILE_NUM_SHIFT));

	// 4J - In creative mode, the point where we need to tell the renderer that we are about to destroy a tile via destroyingTileAt is quite complicated.
	// If the player being told is remote, then we always want the client to do it as it does the final update. If the player being told is local,
	// then we need to update the renderer Here if we are sharing data between host & client as this is the final point where the original data is still intact.
	// If the player being told is local, and we aren't sharing data between host & client, then we can just treat it as if it is a remote player and
	// it can update the renderer.
	bool clientToUpdateRenderer = false;
	if( isCreative() )
	{
		clientToUpdateRenderer = true;
		if( dynamic_pointer_cast<ServerPlayer>(player)->connection->isLocal() )
		{
			// Establish whether we are sharing this chunk between client & server
			MultiPlayerLevel *clientLevel = Minecraft::GetInstance()->getLevel(level->dimension->id);
			if( clientLevel )
			{
				LevelChunk *lc = clientLevel->getChunkAt( x, z );
#ifdef SHARING_ENABLED
				if( lc->sharingTilesAndData )
				{
					// We are sharing - this is the last point we can tell the renderer
					Minecraft::GetInstance()->levelRenderer->destroyedTileManager->destroyingTileAt( clientLevel, x, y, z );

					// Don't need to ask the client to do this too
					clientToUpdateRenderer = false;
				}
#endif
			}
		}
	}

	bool changed = superDestroyBlock(x, y, z);

	if (isCreative())
	{
		shared_ptr<TileUpdatePacket> tup = std::make_shared<TileUpdatePacket>(x, y, z, level);
		// 4J - a bit of a hack here, but if we want to tell the client that it needs to inform the renderer of a block being destroyed, then send a block 255 instead of a 0. This is handled in ClientConnection::handleTileUpdate
		if( tup->block == 0 )
		{
			if( clientToUpdateRenderer ) tup->block = 255;
		}
		player->connection->send( tup );
	}
	else 
	{
		shared_ptr<ItemInstance> item = player->getSelectedItem();
		bool canDestroy = player->canDestroy(Tile::tiles[t]);
		if (item != nullptr)
		{
			item->mineBlock(level, t, x, y, z, player);
			if (item->count == 0)
			{
				player->removeSelectedItem();
			}
		}
		if (changed && canDestroy)
		{
			Tile::tiles[t]->playerDestroy(level, player, x, y, z, data);
		}
	}
	return changed;

}

bool ServerPlayerGameMode::useItem(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, bool bTestUseOnly)
{
	if(!player->isAllowedToUse(item)) return false;

	int oldCount = item->count;
	int oldAux = item->getAuxValue();
	shared_ptr<ItemInstance> itemInstance = item->use(level, player);
	if (itemInstance != item || (itemInstance != nullptr && (itemInstance->count != oldCount || itemInstance->getUseDuration() > 0 || itemInstance->getAuxValue() != oldAux)))
	{
		player->inventory->items[player->inventory->selected] = itemInstance;
		if (isCreative())
		{
			itemInstance->count = oldCount;
			if (itemInstance->isDamageableItem()) itemInstance->setAuxValue(oldAux);
		}
		if (itemInstance->count == 0)
		{
			player->inventory->items[player->inventory->selected] = nullptr;
		}
		if (!player->isUsingItem())
		{
			dynamic_pointer_cast<ServerPlayer>(player)->refreshContainer(player->inventoryMenu);
		}
		return true;
	}
	return false;

}

bool ServerPlayerGameMode::useItemOn(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly, bool *pbUsedItem)
{
	// 4J-PB - Adding a test only version to allow tooltips to be displayed
	int t = level->getTile(x, y, z);
	if (!player->isSneaking() || player->getCarriedItem() == nullptr)
	{
		if (t > 0 && player->isAllowedToUse(Tile::tiles[t]))
		{
			if(bTestUseOnOnly)
			{
				if (Tile::tiles[t]->TestUse()) return true;
			}
			else 
			{
				if (Tile::tiles[t]->use(level, x, y, z, player, face, clickX, clickY, clickZ))
				{
					if(m_gameRules != nullptr) m_gameRules->onUseTile(t,x,y,z);
					return true;
				}
			}
		}
	}

	if (item == nullptr || !player->isAllowedToUse(item)) return false;
	if (isCreative())
	{
		int aux = item->getAuxValue();
		int count = item->count;
		bool success = item->useOn(player, level, x, y, z, face, clickX, clickY, clickZ);
		item->setAuxValue(aux);
		item->count = count;
		return success;
	}
	else
	{
		return item->useOn(player, level, x, y, z, face, clickX, clickY, clickZ, bTestUseOnOnly);
	}
}

void ServerPlayerGameMode::setLevel(ServerLevel *newLevel)
{
	level = newLevel;
}

// 4J Added
void ServerPlayerGameMode::setGameRules(GameRulesInstance *rules)
{
	if(m_gameRules != nullptr) delete m_gameRules;
	m_gameRules = rules;
}