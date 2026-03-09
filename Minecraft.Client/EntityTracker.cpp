#include "stdafx.h"
#include "EntityTracker.h"
#include "MinecraftServer.h"
#include "PlayerList.h"
#include "TrackedEntity.h"
#include "ServerPlayer.h"
#include "ServerLevel.h"
#include "..\Minecraft.World\Mth.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\Minecraft.World\net.minecraft.world.entity.monster.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "..\Minecraft.World\net.minecraft.world.entity.global.h"
#include "..\Minecraft.World\net.minecraft.world.entity.projectile.h"
#include "..\Minecraft.World\net.minecraft.world.entity.boss.enderdragon.h"
#include "..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\Minecraft.World\net.minecraft.network.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "..\Minecraft.World\BasicTypeContainers.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "PlayerConnection.h"

EntityTracker::EntityTracker(ServerLevel *level)
{
    this->level = level;
    maxRange = level->getServer()->getPlayers()->getMaxRange();
}

void EntityTracker::addEntity(shared_ptr<Entity> e)
{
    if (e->instanceof(eTYPE_SERVERPLAYER))
	{
        addEntity(e, 32 * 16, 2);
        shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(e);
		for ( auto& it : entities )
		{
			if( it && it->e != player )
			{
				it->updatePlayer(this, player);
			}
		}
    }
	else if (e->instanceof(eTYPE_FISHINGHOOK)) addEntity(e, 16 * 4, 5, true);
	else if (e->instanceof(eTYPE_SMALL_FIREBALL)) addEntity(e, 16 * 4, 10, false);
	else if (e->instanceof(eTYPE_DRAGON_FIREBALL)) addEntity(e, 16 * 4, 10, false); // 4J Added TU9
    else if (e->instanceof(eTYPE_ARROW)) addEntity(e, 16 * 4, 20, false);
    else if (e->instanceof(eTYPE_FIREBALL)) addEntity(e, 16 * 4, 10, false);
    else if (e->instanceof(eTYPE_SNOWBALL)) addEntity(e, 16 * 4, 10, true);
	else if (e->instanceof(eTYPE_THROWNENDERPEARL)) addEntity(e, 16 * 4, 10, true);
	else if (e->instanceof(eTYPE_EYEOFENDERSIGNAL)) addEntity(e, 16 * 4, 4, true);
    else if (e->instanceof(eTYPE_THROWNEGG)) addEntity(e, 16 * 4, 10, true);
	else if (e->instanceof(eTYPE_THROWNPOTION)) addEntity(e, 16 * 4, 10, true);
	else if (e->instanceof(eTYPE_THROWNEXPBOTTLE)) addEntity(e, 16 * 4, 10, true);
	else if (e->instanceof(eTYPE_FIREWORKS_ROCKET)) addEntity(e, 16 * 4, 10, true);
    else if (e->instanceof(eTYPE_ITEMENTITY)) addEntity(e, 16 * 4, 20, true);
    else if (e->instanceof(eTYPE_MINECART)) addEntity(e, 16 * 5, 1, true);
    else if (e->instanceof(eTYPE_BOAT)) addEntity(e, 16 * 5, 1, true);
    else if (e->instanceof(eTYPE_SQUID)) addEntity(e, 16 * 4, 1, true);
	else if (e->instanceof(eTYPE_WITHERBOSS)) addEntity(e, 16 * 5, 1, false);
	else if (e->instanceof(eTYPE_BAT)) addEntity(e, 16 * 5, 1, false);
    else if (dynamic_pointer_cast<Creature>(e)!=nullptr) addEntity(e, 16 * 5, 1, true);
	else if (e->instanceof(eTYPE_ENDERDRAGON)) addEntity(e, 16 * 10, 1, true);
    else if (e->instanceof(eTYPE_PRIMEDTNT)) addEntity(e, 16 * 10, 10, true);
    else if (e->instanceof(eTYPE_FALLINGTILE)) addEntity(e, 16 * 10, 20, true);
    else if (e->instanceof(eTYPE_HANGING_ENTITY)) addEntity(e, 16 * 10, INT_MAX, false);
	else if (e->instanceof(eTYPE_EXPERIENCEORB)) addEntity(e, 16 * 10, 20, true);
	else if (e->instanceof(eTYPE_ENDER_CRYSTAL)) addEntity(e, 16 * 16, INT_MAX, false);
	else if (e->instanceof(eTYPE_ITEM_FRAME)) addEntity(e, 16 * 10, INT_MAX, false);
}

void EntityTracker::addEntity(shared_ptr<Entity> e, int range, int updateInterval)
{
	addEntity(e, range, updateInterval, false);
}

void EntityTracker::addEntity(shared_ptr<Entity> e, int range, int updateInterval, bool trackDeltas)
{
    if (range > maxRange) range = maxRange;
    if (entityMap.find(e->entityId) != entityMap.end())
	{
		assert(false);	// Entity already tracked
	}
	if( e->entityId >= 2048 )
	{
		__debugbreak();
	}
    shared_ptr<TrackedEntity> te = std::make_shared<TrackedEntity>(e, range, updateInterval, trackDeltas);
	entities.insert(te);
    entityMap[e->entityId] = te;
    te->updatePlayers(this, &level->players);
}

// 4J - have split removeEntity into two bits - it used to do the equivalent of EntityTracker::removePlayer followed by EntityTracker::removeEntity.
// This is to allow us to now choose to remove the player as a "seenBy" only when the player has actually been removed from the level's own player array
void EntityTracker::removeEntity(shared_ptr<Entity> e)
{
	auto it = entityMap.find(e->entityId);
	if( it != entityMap.end() )
	{
		shared_ptr<TrackedEntity> te = it->second;
		entityMap.erase(it);
		entities.erase(te);
		te->broadcastRemoved();
	}
}

void EntityTracker::removePlayer(shared_ptr<Entity> e)
{
    if (e->GetType() == eTYPE_SERVERPLAYER)
	{
        shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(e);
		for( auto& it : entities )
		{
			if ( it )
				it->removePlayer(player);
		}

		// 4J: Flush now to ensure remove packets are sent before player respawns and add entity packets are sent
		player->flushEntitiesToRemove();
    }
}

void EntityTracker::tick()
{
    vector<shared_ptr<ServerPlayer> > movedPlayers;
	for( auto& te : entities )
	{
		if ( te )
		{
			te->tick(this, &level->players);
			if (te->moved && te->e->GetType() == eTYPE_SERVERPLAYER)
			{
				movedPlayers.push_back(dynamic_pointer_cast<ServerPlayer>(te->e));
			}
		}
	}

	// 4J Stu - If one player on a system is updated, then make sure they all are as they all have their
	// range extended to include entities visible by any other player on the system
	// Fix for #11194 - Gameplay: Host player and their split-screen avatars can become invisible and invulnerable to client.
	MinecraftServer *server = MinecraftServer::getInstance();
	for( unsigned int i = 0; i < server->getPlayers()->players.size(); i++ )
	{
		shared_ptr<ServerPlayer> ep = server->getPlayers()->players[i];			
		if( ep->dimension != level->dimension->id ) continue;

		if( ep->connection == nullptr ) continue;
		INetworkPlayer *thisPlayer = ep->connection->getNetworkPlayer();
		if( thisPlayer == nullptr ) continue;

		bool addPlayer = false;
		for (unsigned int j = 0; j < movedPlayers.size(); j++)
		{
			shared_ptr<ServerPlayer> sp = movedPlayers[j];

			if( sp == ep ) break;

			if(sp->connection == nullptr) continue;
			INetworkPlayer *otherPlayer = sp->connection->getNetworkPlayer();
			if( otherPlayer != nullptr && thisPlayer->IsSameSystem(otherPlayer) )
			{
				addPlayer = true;
				break;
			}
		}		
		if( addPlayer ) movedPlayers.push_back( ep );
	}

    for (unsigned int i = 0; i < movedPlayers.size(); i++)
	{
        shared_ptr<ServerPlayer> player = movedPlayers[i];
		if(player->connection == nullptr) continue;
		for( auto& te : entities )
		{
            if ( te && te->e != player)
			{
                te->updatePlayer(this, player);
            }
		}
    }

	// 4J Stu - We want to do this for dead players as they don't tick normally
	for (auto& it : level->players )
	{
        shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(it);
		if( player && !player->isAlive())
		{
			player->flushEntitiesToRemove();
		}
	}
}

void EntityTracker::broadcast(shared_ptr<Entity> e, shared_ptr<Packet> packet)
{
	auto it = entityMap.find(e->entityId);
	if( it != entityMap.end() )
	{
		shared_ptr<TrackedEntity> te = it->second;
		te->broadcast(packet);
	}
}

void EntityTracker::broadcastAndSend(shared_ptr<Entity> e, shared_ptr<Packet> packet)
{
	auto it = entityMap.find(e->entityId);
	if( it != entityMap.end() )
	{
		shared_ptr<TrackedEntity> te = it->second;
		te->broadcastAndSend(packet);
	}
}

void EntityTracker::clear(shared_ptr<ServerPlayer> serverPlayer)
{
	for ( auto& te : entities )
	{
		if ( te )
			te->clear(serverPlayer);
	}
}

void EntityTracker::playerLoadedChunk(shared_ptr<ServerPlayer> player, LevelChunk *chunk)
{
	for ( auto& te : entities )
	{
		if (te->e != player && te->e->xChunk == chunk->x && te->e->zChunk == chunk->z)
		{
			te->updatePlayer(this, player);
		}
	}
}

// AP added for Vita so the range can be increased once the level starts
void EntityTracker::updateMaxRange()
{
    maxRange = level->getServer()->getPlayers()->getMaxRange();
}


shared_ptr<TrackedEntity> EntityTracker::getTracker(shared_ptr<Entity> e)
{
	auto it = entityMap.find(e->entityId);
	if( it != entityMap.end() )
	{
		return it->second;
	}
	return nullptr;
}
