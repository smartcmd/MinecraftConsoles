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
#include "PlayerConnection.h"

EntityTracker::EntityTracker(ServerLevel *level)
{
    this->level = level;
    maxRange = level->getServer()->getPlayers()->getMaxRange();
}

void EntityTracker::addEntity(shared_ptr<Entity> e)
{
    if (e->GetType() == eTYPE_SERVERPLAYER)
	{
        addEntity(e, 32 * 16, 2);
        shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(e);
		for( AUTO_VAR(it, entities.begin()); it != entities.end(); it++ )
		{
			if( (*it)->e != player )
			{
				(*it)->updatePlayer(this, player);
			}
		}
    }
	else if (e->GetType() == eTYPE_FISHINGHOOK) addEntity(e, 16 * 4, 5, true);
	else if (e->GetType() == eTYPE_SMALL_FIREBALL) addEntity(e, 16 * 4, 10, false);
	else if (e->GetType() == eTYPE_DRAGON_FIREBALL) addEntity(e, 16 * 4, 10, false); // 4J Added TU9
    else if (e->GetType() == eTYPE_ARROW) addEntity(e, 16 * 4, 20, false);
    else if (e->GetType() == eTYPE_FIREBALL) addEntity(e, 16 * 4, 10, false);
    else if (e->GetType() == eTYPE_SNOWBALL) addEntity(e, 16 * 4, 10, true);
	else if (e->GetType() == eTYPE_THROWNENDERPEARL) addEntity(e, 16 * 4, 10, true);
	else if (e->GetType() == eTYPE_EYEOFENDERSIGNAL ) addEntity(e, 16 * 4, 4, true);
    else if (e->GetType() == eTYPE_THROWNEGG) addEntity(e, 16 * 4, 10, true);
	else if (e->GetType() == eTYPE_THROWNPOTION ) addEntity(e, 16 * 4, 10, true);
	else if (e->GetType() == eTYPE_THROWNEXPBOTTLE) addEntity(e, 16 * 4, 10, true);
    else if (e->GetType() == eTYPE_ITEMENTITY) addEntity(e, 16 * 4, 20, true);
    else if (e->GetType() == eTYPE_MINECART) addEntity(e, 16 * 5, 3, true);
    else if (e->GetType() == eTYPE_BOAT) addEntity(e, 16 * 5, 3, true);
    else if (e->GetType() == eTYPE_SQUID) addEntity(e, 16 * 4, 3, true);
    else if (dynamic_pointer_cast<Creature>(e)!=NULL) addEntity(e, 16 * 5, 3, true);
	else if (e->GetType() == eTYPE_ENDERDRAGON ) addEntity(e, 16 * 10, 3, true);
    else if (e->GetType() == eTYPE_PRIMEDTNT) addEntity(e, 16 * 10, 10, true);
    else if (e->GetType() == eTYPE_FALLINGTILE) addEntity(e, 16 * 10, 20, true);
    else if (e->GetType() == eTYPE_PAINTING) addEntity(e, 16 * 10, INT_MAX, false);
	else if (e->GetType() == eTYPE_EXPERIENCEORB) addEntity(e, 16 * 10, 20, true);
	else if (e->GetType() == eTYPE_ENDER_CRYSTAL) addEntity(e, 16 * 16, INT_MAX, false);
	else if (e->GetType() == eTYPE_ITEM_FRAME) addEntity(e, 16 * 10, INT_MAX, false);
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
    shared_ptr<TrackedEntity> te = shared_ptr<TrackedEntity>( new TrackedEntity(e, range, updateInterval, trackDeltas) );
	entities.insert(te);
    entityMap[e->entityId] = te;
    te->updatePlayers(this, &level->players);
}

// 4J - have split removeEntity into two bits - it used to do the equivalent of EntityTracker::removePlayer followed by EntityTracker::removeEntity.
// This is to allow us to now choose to remove the player as a "seenBy" only when the player has actually been removed from the level's own player array
void EntityTracker::removeEntity(shared_ptr<Entity> e)
{
	AUTO_VAR(it, entityMap.find(e->entityId));
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
		for( AUTO_VAR(it, entities.begin()); it != entities.end(); it++ )
		{
			(*it)->removePlayer(player);
		}
    }
}

void EntityTracker::tick()
{
    vector<shared_ptr<ServerPlayer> > movedPlayers;
	for( AUTO_VAR(it, entities.begin()); it != entities.end(); it++ )
	{
		shared_ptr<TrackedEntity> te = *it;
        te->tick(this, &level->players);
        if (te->moved && te->e->GetType() == eTYPE_SERVERPLAYER)
		{
            movedPlayers.push_back(dynamic_pointer_cast<ServerPlayer>(te->e));
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

		if( ep->connection == NULL ) continue;
		INetworkPlayer *thisPlayer = ep->connection->getNetworkPlayer();
		if( thisPlayer == NULL ) continue;

		bool addPlayer = false;
		for (unsigned int j = 0; j < movedPlayers.size(); j++)
		{
			shared_ptr<ServerPlayer> sp = movedPlayers[j];

			if( sp == ep ) break;

			if(sp->connection == NULL) continue;
			INetworkPlayer *otherPlayer = sp->connection->getNetworkPlayer();
			if( otherPlayer != NULL && thisPlayer->IsSameSystem(otherPlayer) )
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
		if(player->connection == NULL) continue;
		for( AUTO_VAR(it, entities.begin()); it != entities.end(); it++ )
		{
			shared_ptr<TrackedEntity> te = *it;
            if (te->e != player)
			{
                te->updatePlayer(this, player);
            }
		}
    }

	// 4J Stu - We want to do this for dead players as they don't tick normally
	for(AUTO_VAR(it, level->players.begin()); it != level->players.end(); ++it)
	{
        shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(*it);
		if(!player->isAlive())
		{
			player->flushEntitiesToRemove();
		}
	}
}

void EntityTracker::broadcast(shared_ptr<Entity> e, shared_ptr<Packet> packet)
{
	AUTO_VAR(it, entityMap.find( e->entityId ));
	if( it != entityMap.end() )
	{
		shared_ptr<TrackedEntity> te = it->second;
		te->broadcast(packet);
	}
}

void EntityTracker::broadcastAndSend(shared_ptr<Entity> e, shared_ptr<Packet> packet)
{
	AUTO_VAR(it, entityMap.find( e->entityId ));
	if( it != entityMap.end() )
	{
		shared_ptr<TrackedEntity> te = it->second;
		te->broadcastAndSend(packet);
	}
}

void EntityTracker::clear(shared_ptr<ServerPlayer> serverPlayer)
{
	for( AUTO_VAR(it, entities.begin()); it != entities.end(); it++ )
	{
		shared_ptr<TrackedEntity> te = *it;
		te->clear(serverPlayer);
	}
}

// AP added for Vita so the range can be increased once the level starts
void EntityTracker::updateMaxRange()
{
    maxRange = level->getServer()->getPlayers()->getMaxRange();
}


shared_ptr<TrackedEntity> EntityTracker::getTracker(shared_ptr<Entity> e)
{
	AUTO_VAR(it, entityMap.find(e->entityId));
	if( it != entityMap.end() )
	{
		return it->second;
	}
	return nullptr;
}