#include "stdafx.h"
#include "ServerLevelListener.h"

#include "EntityTracker.h"
#include "MinecraftServer.h"
#include "ServerLevel.h"
#include "ServerPlayer.h"
#include "PlayerList.h"
#include "PlayerChunkMap.h"
#include "PlayerConnection.h"
#include "../Minecraft.World/net.minecraft.world.level.dimension.h"
#include "../Minecraft.World/net.minecraft.network.packet.h"
#include "../Minecraft.World/LevelData.h"


ServerLevelListener::ServerLevelListener(MinecraftServer *server, ServerLevel *level)
{
	this->server = server;
	this->level = level;
}

// 4J removed -
/*
void ServerLevelListener::addParticle(const wstring& name, double x, double y, double z, double xa, double ya, double za)
{
}
*/

void ServerLevelListener::addParticle(ePARTICLE_TYPE name, double x, double y, double z, double xa, double ya, double za)
{
}

void ServerLevelListener::allChanged()
{
}

void ServerLevelListener::entityAdded(shared_ptr<Entity> entity)
{
	MemSect(10);
	level->getTracker()->addEntity(entity);
	MemSect(0);
}

void ServerLevelListener::entityRemoved(shared_ptr<Entity> entity)
{
	level->getTracker()->removeEntity(entity);
}

// 4J added
void ServerLevelListener::playerRemoved(shared_ptr<Entity> entity)
{
	dynamic_pointer_cast<ServerPlayer>(entity)->getLevel()->getTracker()->removePlayer(entity);
}

void ServerLevelListener::playSound(int iSound, double x, double y, double z, float volume, float pitch, float fClipSoundDist)
{
	if(iSound < 0)
	{
		app.DebugPrintf("ServerLevelListener received request for sound less than 0, so ignoring\n");
	}
	else
	{
		// 4J-PB - I don't want to broadcast player sounds to my local machine, since we're already playing these in the LevelRenderer::playSound.
		// The PC version does seem to do this and the result is I can stop walking , and then I'll hear my footstep sound with a delay
		server->getPlayers()->broadcast(x, y, z, volume > 1 ? 16 * volume : 16, level->dimension->id, std::make_shared<LevelSoundPacket>(iSound, x, y, z, volume, pitch));
	}
}

void ServerLevelListener::playSoundExceptPlayer(shared_ptr<Player> player, int iSound, double x, double y, double z, float volume, float pitch, float fSoundClipDist)
{
	if(iSound < 0)
	{
		app.DebugPrintf("ServerLevelListener received request for sound less than 0, so ignoring\n");
	}
	else
	{
		// 4J-PB - I don't want to broadcast player sounds to my local machine, since we're already playing these in the LevelRenderer::playSound.
		// The PC version does seem to do this and the result is I can stop walking , and then I'll hear my footstep sound with a delay
		server->getPlayers()->broadcast(player,x, y, z, volume > 1 ? 16 * volume : 16, level->dimension->id, std::make_shared<LevelSoundPacket>(iSound, x, y, z, volume, pitch));
	}
}

void ServerLevelListener::setTilesDirty(int x0, int y0, int z0, int x1, int y1, int z1, Level *level)
{
}

void ServerLevelListener::skyColorChanged()
{
}

void ServerLevelListener::tileChanged(int x, int y, int z)
{
	level->getChunkMap()->tileChanged(x, y, z);
}

void ServerLevelListener::tileLightChanged(int x, int y, int z)
{
}

void ServerLevelListener::playStreamingMusic(const wstring& name, int x, int y, int z)
{
}

void ServerLevelListener::levelEvent(shared_ptr<Player> source, int type, int x, int y, int z, int data)
{
	server->getPlayers()->broadcast(source, x, y, z, 64, level->dimension->id, std::make_shared<LevelEventPacket>( type, x, y, z, data, false ) );
}

void ServerLevelListener::globalLevelEvent(int type, int sourceX, int sourceY, int sourceZ, int data)
{
	server->getPlayers()->broadcastAll( std::make_shared<LevelEventPacket>( type, sourceX, sourceY, sourceZ, data, true) );
}

void ServerLevelListener::destroyTileProgress(int id, int x, int y, int z, int progress)
{
	//for (ServerPlayer p : server->getPlayers()->players)
	for(auto& p : server->getPlayers()->players)
	{
		if (p == nullptr || p->level != level || p->entityId == id) continue;
		double xd = (double) x - p->x;
		double yd = (double) y - p->y;
		double zd = (double) z - p->z;

		if (xd * xd + yd * yd + zd * zd < 32 * 32)
		{
			p->connection->send(std::make_shared<TileDestructionPacket>(id, x, y, z, progress));
		}
	}
}