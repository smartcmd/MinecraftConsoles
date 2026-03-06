#include "stdafx.h"
#include "PlayerChunkMap.h"
#include "PlayerConnection.h"
#include "ServerLevel.h"
#include "ServerChunkCache.h"
#include "ServerPlayer.h"
#include "MinecraftServer.h"
#include "..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\ArrayWithLength.h"
#include "..\Minecraft.World\System.h"
#include "PlayerList.h"

PlayerChunkMap::PlayerChunk::PlayerChunk(int x, int z, PlayerChunkMap *pcm) : pos(x,z)
{
	// 4J - added initialisers
	changes = 0;
	changedTiles = shortArray(MAX_CHANGES_BEFORE_RESEND);
	xChangeMin = xChangeMax = 0;
	yChangeMin = yChangeMax = 0;
	zChangeMin = zChangeMax = 0;
	parent = pcm;	// 4J added
	ticksToNextRegionUpdate = 0;	// 4J added
	prioritised = false;	// 4J added
	firstInhabitedTime = 0;

    parent->getLevel()->cache->create(x, z);
}

PlayerChunkMap::PlayerChunk::~PlayerChunk()
{
	delete changedTiles.data;
}

// 4J added - construct an an array of flags that indicate which entities are still waiting to have network packets sent out to say that they have been removed
// If there aren't any entities to be flagged, this function does nothing. If there *are* entities to be added, uses the removedFound as an input to
// determine if the flag array has already been initialised at all - if it has been, then just adds flags to it; if it hasn't, then memsets the output
// flag array and adds to it for this ServerPlayer.
void PlayerChunkMap::flagEntitiesToBeRemoved(unsigned int *flags, bool *flagToBeRemoved)
{
	for(auto& serverPlayer : players)
	{
		if ( serverPlayer )
			serverPlayer->flagEntitiesToBeRemoved(flags, flagToBeRemoved);
	}
}

void PlayerChunkMap::PlayerChunk::add(shared_ptr<ServerPlayer> player, bool sendPacket /*= true*/)
{
	//app.DebugPrintf("--- Adding player to chunk x=%d\tz=%d\n",x, z);
    if (find(players.begin(),players.end(),player) != players.end())
	{
		// 4J-PB - At the start of the game, lots of chunks are added, and we can then move into an area that is outside the diameter of our starting area,
		// but is inside the area loaded at the start.
		app.DebugPrintf("--- Adding player to chunk x=%d\t z=%d, but they are already in there!\n",pos.x, pos.z);
		return;

		//assert(false);
// 4J - was			throw new IllegalStateException("Failed to add player. " + player + " already is in chunk " + x + ", " + z);
	}

    player->seenChunks.insert(pos);

	// 4J Added the sendPacket check. See PlayerChunkMap::add for the usage
	if( sendPacket ) player->connection->send( shared_ptr<ChunkVisibilityPacket>( new ChunkVisibilityPacket(pos.x, pos.z, true) ) );

	if (players.empty())
	{
		firstInhabitedTime = parent->level->getGameTime();
	}

	players.push_back(player);

	player->chunksToSend.push_back(pos);

#ifdef _LARGE_WORLDS
	parent->getLevel()->cache->dontDrop(pos.x, pos.z); // 4J Added;
#endif
}

void PlayerChunkMap::PlayerChunk::remove(shared_ptr<ServerPlayer> player)
{
	PlayerChunkMap::PlayerChunk *toDelete = NULL;

	//app.DebugPrintf("--- PlayerChunkMap::PlayerChunk::remove x=%d\tz=%d\n",x,z);
    auto it = find(players.begin(), players.end(), player);
    if ( it == players.end())
	{
		app.DebugPrintf("--- INFO - Removing player from chunk x=%d\t z=%d, but they are not in that chunk!\n",pos.x, pos.z);

		return;
	}

    players.erase(it);
    if (players.size() == 0)
	{
		{
			LevelChunk *chunk = parent->level->getChunk(pos.x, pos.z);
			updateInhabitedTime(chunk);
            auto it = find(parent->knownChunks.begin(), parent->knownChunks.end(), this);
            if(it != parent->knownChunks.end()) parent->knownChunks.erase(it);
		}
        __int64 id = (pos.x + 0x7fffffffLL) | ((pos.z + 0x7fffffffLL) << 32);
        auto it = parent->chunks.find(id);
        if( it != parent->chunks.end() )
		{
			toDelete = it->second;	// Don't delete until the end of the function, as this might be this instance
			parent->chunks.erase(it);
		}
        if (changes > 0)
		{
            auto it = find(parent->changedChunks.begin(), parent->changedChunks.end(), this);
            parent->changedChunks.erase(it);
        }
        parent->getLevel()->cache->drop(pos.x, pos.z);
    }

    player->chunksToSend.remove(pos);
	// 4J - I don't think there's any point sending these anymore, as we don't need to unload chunks with fixed sized maps
	// 4J - We do need to send these to unload entities in chunks when players are dead. If we do not and the entity is removed
	// while they are dead, that entity will remain in the clients world
    if (player->connection != NULL && player->seenChunks.find(pos) != player->seenChunks.end())
	{
		INetworkPlayer *thisNetPlayer = player->connection->getNetworkPlayer();
		bool noOtherPlayersFound = true;

		if( thisNetPlayer != nullptr )
		{
            for (auto& currPlayer : players )
            {
				if ( currPlayer )
				{
					INetworkPlayer *currNetPlayer = currPlayer->connection->getNetworkPlayer();
					if( currNetPlayer != NULL && currNetPlayer->IsSameSystem( thisNetPlayer ) && currPlayer->seenChunks.find(pos) != currPlayer->seenChunks.end() )
					{
						noOtherPlayersFound = false;
						break;
					}
				}
			}
			if(noOtherPlayersFound)
			{
				//wprintf(L"Sending ChunkVisiblity packet false for chunk (%d,%d) to player %ls\n", x, z, player->name.c_str() );
				player->connection->send( shared_ptr<ChunkVisibilityPacket>( new ChunkVisibilityPacket(pos.x, pos.z, false) ) );
			}
		}
		else
		{
			//app.DebugPrintf("PlayerChunkMap::PlayerChunk::remove - QNetPlayer is NULL\n");
		}
    }

	delete toDelete;
}

void PlayerChunkMap::PlayerChunk::updateInhabitedTime()
{
	updateInhabitedTime(parent->level->getChunk(pos.x, pos.z));
}

void PlayerChunkMap::PlayerChunk::updateInhabitedTime(LevelChunk *chunk)
{
	chunk->inhabitedTime += parent->level->getGameTime() - firstInhabitedTime;

	firstInhabitedTime = parent->level->getGameTime();
}

void PlayerChunkMap::PlayerChunk::tileChanged(int x, int y, int z)
{
    if (changes == 0)
	{
        parent->changedChunks.push_back(this);
        xChangeMin = xChangeMax = x;
        yChangeMin = yChangeMax = y;
        zChangeMin = zChangeMax = z;
    }
    if (xChangeMin > x) xChangeMin = x;
    if (xChangeMax < x) xChangeMax = x;

    if (yChangeMin > y) yChangeMin = y;
    if (yChangeMax < y) yChangeMax = y;

    if (zChangeMin > z) zChangeMin = z;
    if (zChangeMax < z) zChangeMax = z;

    if (changes < MAX_CHANGES_BEFORE_RESEND)
	{
        short id = (short) ((x << 12) | (z << 8) | (y));

        for (int i = 0; i < changes; i++)
		{
            if (changedTiles[i] == id) return;
        }

        changedTiles[changes++] = id;
    }
}

// 4J added - make sure that any tile updates for the chunk at this location get prioritised for sending
void PlayerChunkMap::PlayerChunk::prioritiseTileChanges()
{
	prioritised = true;
}

void PlayerChunkMap::PlayerChunk::broadcast(shared_ptr<Packet> packet)
{
	vector< shared_ptr<ServerPlayer> > sentTo;
    for (unsigned int i = 0; i < players.size(); i++)
	{
        shared_ptr<ServerPlayer> player = players[i];

		// 4J - don't send to a player we've already sent this data to that shares the same machine. TileUpdatePacket,
		// ChunkTilesUpdatePacket and SignUpdatePacket all used to limit themselves to sending once to each machine
		// by only sending to the primary player on each machine. This was causing trouble for split screen
		// as updates were only coming in for the region round this one player. Now these packets can be sent to any
		// player, but we try to restrict the network impact this has by not resending to the one machine
		bool dontSend = false;
		if( sentTo.size() )
		{
			INetworkPlayer *thisPlayer = player->connection->getNetworkPlayer();
			if( thisPlayer == NULL )
			{
				dontSend = true;
			}
			else
			{
				for(unsigned int j = 0; j < sentTo.size(); j++ )
				{
					shared_ptr<ServerPlayer> player2 = sentTo[j];
					INetworkPlayer *otherPlayer = player2->connection->getNetworkPlayer();
					if( otherPlayer != NULL && thisPlayer->IsSameSystem(otherPlayer) )
					{
						dontSend = true;
					}
				}
			}
		}
		if( dontSend )
		{
			continue;
		}

		// 4J Changed to get the flag index for the player before we send a packet. This flag is updated when we queue
		// for send the first BlockRegionUpdatePacket for this chunk to that player/players system. Therefore there is no need to
		// send tile updates or other updates until that has been sent
		int flagIndex = ServerPlayer::getFlagIndexForChunk(pos, parent->dimension);
        if (player->seenChunks.find(pos) != player->seenChunks.end() && (player->connection->isLocal() || g_NetworkManager.SystemFlagGet(player->connection->getNetworkPlayer(),flagIndex) ))
		{
            player->connection->send(packet);
			sentTo.push_back(player);
        }
    }
	// Now also check round all the players that are involved in this game. We also want to send the packet
	// to them if their system hasn't received it already, but they have received the first BlockRegionUpdatePacket for this
	// chunk

	// Make sure we are only doing this for BlockRegionUpdatePacket, ChunkTilesUpdatePacket and TileUpdatePacket.
	// We'll be potentially sending to players who aren't on the same level as this packet is intended for,
	// and only these 3 packets have so far been updated to be able to encode the level so they are robust
	// enough to cope with this
	if(!( ( packet->getId() == 51 ) || ( packet->getId() == 52 ) || ( packet->getId() == 53 ) ) )
	{
		return;
	}

	for( int i = 0; i < parent->level->getServer()->getPlayers()->players.size(); i++ )
	{
		shared_ptr<ServerPlayer> player = parent->level->getServer()->getPlayers()->players[i];
		// Don't worry about local players, they get all their updates through sharing level with the server anyway
		if ( player->connection == NULL ) continue;
		if( player->connection->isLocal() ) continue;

		// Don't worry about this player if they haven't had this chunk yet (this flag will be the
		// same for all players on the same system)
		int flagIndex = ServerPlayer::getFlagIndexForChunk(pos,parent->dimension);
		if(!g_NetworkManager.SystemFlagGet(player->connection->getNetworkPlayer(),flagIndex)) continue;

		// From here on the same rules as in the loop above - don't send it if we've already sent to the same system
		bool dontSend = false;
		if( sentTo.size() )
		{
			INetworkPlayer *thisPlayer = player->connection->getNetworkPlayer();
			if( thisPlayer == NULL )
			{
				dontSend = true;
			}
			else
			{
				for(unsigned int j = 0; j < sentTo.size(); j++ )
				{
					shared_ptr<ServerPlayer> player2 = sentTo[j];
					INetworkPlayer *otherPlayer = player2->connection->getNetworkPlayer();
					if( otherPlayer != NULL && thisPlayer->IsSameSystem(otherPlayer) )
					{
						dontSend = true;
					}
				}
			}
		}
		if( !dontSend )
		{
            player->connection->send(packet);
			sentTo.push_back(player);
		}
	}
}

bool PlayerChunkMap::PlayerChunk::broadcastChanges(bool allowRegionUpdate)
{
	bool didRegionUpdate = false;
    ServerLevel *level = parent->getLevel();
	if( ticksToNextRegionUpdate > 0 ) ticksToNextRegionUpdate--;
    if (changes == 0)
	{
		prioritised = false;
		return false;
	}
    if (changes == 1)
	{
        int x = pos.x * 16 + xChangeMin;
        int y = yChangeMin;
        int z = pos.z * 16 + zChangeMin;
        broadcast( shared_ptr<TileUpdatePacket>( new TileUpdatePacket(x, y, z, level) ) );
        if (level->isEntityTile(x, y, z))
		{
            broadcast(level->getTileEntity(x, y, z));
        }
    }
	else if (changes == MAX_CHANGES_BEFORE_RESEND)
	{
		// 4J added, to allow limiting of region update packets created
		if( !prioritised )
		{
			if( !allowRegionUpdate || ( ticksToNextRegionUpdate > 0 ) )
			{
				return false;
			}
		}

        yChangeMin = yChangeMin / 2 * 2;
        yChangeMax = (yChangeMax / 2 + 1) * 2;
        int xp = xChangeMin + pos.x * 16;
        int yp = yChangeMin;
        int zp = zChangeMin + pos.z * 16;
        int xs = xChangeMax - xChangeMin + 1;
        int ys = yChangeMax - yChangeMin + 2;
        int zs = zChangeMax - zChangeMin + 1;

		// Fix for buf #95007 : TCR #001 BAS Game Stability: TU12: Code: Compliance: More than 192 dropped items causes game to freeze or crash.
		// Block region update packets can only encode ys in a range of 1 - 256
		if( ys > 256 ) ys = 256;

        broadcast( shared_ptr<BlockRegionUpdatePacket>( new BlockRegionUpdatePacket(xp, yp, zp, xs, ys, zs, level) ) );
        vector<shared_ptr<TileEntity> > *tes = level->getTileEntitiesInRegion(xp, yp, zp, xp + xs, yp + ys, zp + zs);
        for (unsigned int i = 0; i < tes->size(); i++)
		{
            broadcast(tes->at(i));
        }
		delete tes;
		ticksToNextRegionUpdate = MIN_TICKS_BETWEEN_REGION_UPDATE;
		didRegionUpdate = true;
    }
	else
	{
		// 4J As we only get here if changes is less than MAX_CHANGES_BEFORE_RESEND (10) we only need to send a byte value in the packet
        broadcast( shared_ptr<ChunkTilesUpdatePacket>( new ChunkTilesUpdatePacket(pos.x, pos.z, changedTiles, (byte)changes, level) ) );
        for (int i = 0; i < changes; i++)
		{
            int x = pos.x * 16 + ((changedTiles[i] >> 12) & 15);
            int y = ((changedTiles[i]) & 255);
            int z = pos.z * 16 + ((changedTiles[i] >> 8) & 15);

            if (level->isEntityTile(x, y, z))
			{
//                    System.out.println("Sending!");
                broadcast(level->getTileEntity(x, y, z));
            }
        }
    }
    changes = 0;
	prioritised = false;
	return didRegionUpdate;
}

void PlayerChunkMap::PlayerChunk::broadcast(shared_ptr<TileEntity> te)
{
    if (te != NULL)
	{
        shared_ptr<Packet> p = te->getUpdatePacket();
        if (p != NULL)
		{
            broadcast(p);
        }
    }
}

PlayerChunkMap::PlayerChunkMap(ServerLevel *level, int dimension, int radius)
{
	assert(radius <= MAX_VIEW_DISTANCE);
	assert(radius >= MIN_VIEW_DISTANCE);
    this->radius = radius;
    this->level = level;
	this->dimension = dimension;
	lastInhabitedUpdate = 0;
}

PlayerChunkMap::~PlayerChunkMap()
{
	for(auto& chunk : chunks)
	{
		delete chunk.second;
	}
}

ServerLevel *PlayerChunkMap::getLevel()
{
	return level;
}

void PlayerChunkMap::tick()
{
	__int64 time = level->getGameTime();

	if (time - lastInhabitedUpdate > Level::TICKS_PER_DAY / 3)
	{
        lastInhabitedUpdate = time;

        for (int i = 0; i < knownChunks.size(); i++)
		{
            PlayerChunk *chunk = knownChunks.at(i);

			// 4J Stu - Going to let our changeChunks handler below deal with this
            //chunk.broadcastChanges();

            chunk->updateInhabitedTime();
        }
    }

	// 4J - some changes here so that we only send one region update per tick. The chunks themselves also
	// limit their resend rate to once every MIN_TICKS_BETWEEN_REGION_UPDATE ticks
	bool regionUpdateSent = false;
    for (unsigned int i = 0; i < changedChunks.size();)
	{
        regionUpdateSent |= changedChunks[i]->broadcastChanges(!regionUpdateSent);
		// Changes will be 0 if the chunk actually sent something, in which case we can delete it from this array
		if( changedChunks[i]->changes == 0 )
		{
			changedChunks[i] = changedChunks.back();
			changedChunks.pop_back();
		}
		else
		{
			// Limiting of some kind means we didn't send this chunk so move onto the next
			i++;
		}
    }

	for( unsigned int i = 0; i < players.size(); i++ )
	{
		tickAddRequests(players[i]);
	}

	// 4J Stu - Added 1.1 but not relevant to us as we never no 0 players anyway, and don't think we should be dropping stuff
	//if (players.isEmpty()) {
	//	ServerLevel level = server.getLevel(this.dimension);
	//	Dimension dimension = level.dimension;
	//	if (!dimension.mayRespawn()) {
	//		level.cache.dropAll();
	//	}
	//}
}

bool PlayerChunkMap::hasChunk(int x, int z)
{
    __int64 id = (x + 0x7fffffffLL) | ((z + 0x7fffffffLL) << 32);
	return chunks.find(id) != chunks.end();
}

PlayerChunkMap::PlayerChunk *PlayerChunkMap::getChunk(int x, int z, bool create)
{
    __int64 id = (x + 0x7fffffffLL) | ((z + 0x7fffffffLL) << 32);
    auto it = chunks.find(id);

    PlayerChunk *chunk = nullptr;
	if( it != chunks.end() )
	{
		chunk = it->second;
	}
	else if ( create)
	{
        chunk = new PlayerChunk(x, z, this);
        chunks[id] = chunk;
		knownChunks.push_back(chunk);
    }

	return chunk;
}

// 4J - added. If a chunk exists, add a player to it straight away. If it doesn't exist,
// queue a request for it to be created.
void PlayerChunkMap::getChunkAndAddPlayer(int x, int z, shared_ptr<ServerPlayer> player)
{
    __int64 id = (x + 0x7fffffffLL) | ((z + 0x7fffffffLL) << 32);
    auto it = chunks.find(id);

    if( it != chunks.end() )
	{
		it->second->add(player);
	}
	else
	{
		addRequests.push_back(PlayerChunkAddRequest(x,z,player));
	}
}

// 4J - added. If the chunk and player are in the queue to be added, remove from there. Otherwise
// attempt to remove from main chunk map.
void PlayerChunkMap::getChunkAndRemovePlayer(int x, int z, shared_ptr<ServerPlayer> player)
{
    for (auto it = addRequests.begin(); it != addRequests.end(); it++)
    {
		if( ( it->x == x ) &&
			( it->z == z ) &&
			( it->player == player ) )
		{
			addRequests.erase(it);
			return;
		}
	}
    __int64 id = (x + 0x7fffffffLL) | ((z + 0x7fffffffLL) << 32);
    auto it = chunks.find(id);

    if( it != chunks.end() )
	{
		it->second->remove(player);
	}
}

// 4J - added - actually create & add player to a playerchunk, if there is one queued for this player.
void PlayerChunkMap::tickAddRequests(shared_ptr<ServerPlayer> player)
{
	if( addRequests.size() )
	{
		// Find the nearest chunk request to the player
		int px = (int)player->x;
		int pz = (int)player->z;
		int minDistSq = -1;

        auto itNearest = addRequests.end();
        for (auto it = addRequests.begin(); it != addRequests.end(); it++)
        {
			if( it->player == player )
			{
				int xm = ( it->x  * 16 ) + 8;
				int zm = ( it->z  * 16 ) + 8;
				int distSq = (xm - px) * (xm - px) +
							 (zm - pz) * (zm - pz);
				if( ( minDistSq == -1 ) || ( distSq < minDistSq ) )
				{
					minDistSq = distSq;
					itNearest = it;
				}
			}
		}

		// If we found one at all, then do this one
		if( itNearest != addRequests.end() )
		{
			getChunk(itNearest->x, itNearest->z, true)->add(itNearest->player);
			addRequests.erase(itNearest);
		}
	}
}

void PlayerChunkMap::broadcastTileUpdate(shared_ptr<Packet> packet, int x, int y, int z)
{
    int xc = x >> 4;
    int zc = z >> 4;
    PlayerChunk *chunk = getChunk(xc, zc, false);
    if (chunk != NULL)
	{
        chunk->broadcast(packet);
    }
}

void PlayerChunkMap::tileChanged(int x, int y, int z)
{
    int xc = x >> 4;
    int zc = z >> 4;
    PlayerChunk *chunk = getChunk(xc, zc, false);
    if (chunk != NULL)
	{
        chunk->tileChanged(x & 15, y, z & 15);
    }
}

bool PlayerChunkMap::isTrackingTile(int x, int y, int z)
{
    int xc = x >> 4;
    int zc = z >> 4;
    PlayerChunk *chunk = getChunk(xc, zc, false);
	if( chunk ) return true;
	return false;
}

// 4J added - make sure that any tile updates for the chunk at this location get prioritised for sending
void PlayerChunkMap::prioritiseTileChanges(int x, int y, int z)
{
    int xc = x >> 4;
    int zc = z >> 4;
    PlayerChunk *chunk = getChunk(xc, zc, false);
    if (chunk != NULL)
	{
        chunk->prioritiseTileChanges();
    }
}

void PlayerChunkMap::add(shared_ptr<ServerPlayer> player)
{
	static int direction[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, {0, -1} };

	int xc = (int) player->x >> 4;
	int zc = (int) player->z >> 4;

    player->lastMoveX = player->x;
    player->lastMoveZ = player->z;

//        for (int x = xc - radius; x <= xc + radius; x++)
//            for (int z = zc - radius; z <= zc + radius; z++) {
//                getChunk(x, z, true).add(player);
//            }

    // CraftBukkit start
    int facing = 0;
    int size = radius;
    int dx = 0;
    int dz = 0;

    // Origin
    getChunk(xc, zc, true)->add(player, false);

	// 4J Added so we send an area packet rather than one visibility packet per chunk
	int minX, maxX, minZ, maxZ;
	minX = maxX = xc;
	minZ = maxZ = zc;

	// 4J - added so that we don't fully create/send every chunk at this stage. Particularly since moving on to large worlds, where
	// we can be adding 1024 chunks here of which a large % might need to be fully created, this can take a long time. Instead use
	// the getChunkAndAddPlayer for anything but the central region of chunks, which adds them to a queue of chunks which are added
	// one per tick per player.
	const int maxLegSizeToAddNow = 14;

    // All but the last leg
    for (int legSize = 1; legSize <= size * 2; legSize++)
	{
        for (int leg = 0; leg < 2; leg++)
		{
            int *dir = direction[facing++ % 4];

            for (int k = 0; k < legSize; k++)
			{
                dx += dir[0];
                dz += dir[1];

				int targetX, targetZ;
				targetX = xc + dx;
				targetZ = zc + dz;

				if( ( legSize < maxLegSizeToAddNow ) ||
					( ( legSize == maxLegSizeToAddNow ) && ( ( leg == 0 ) || ( k < ( legSize - 1 ) ) ) ) )
				{
					if( targetX > maxX ) maxX = targetX;
					if( targetX < minX ) minX = targetX;
					if( targetZ > maxZ ) maxZ = targetZ;
					if( targetZ < minZ ) minZ = targetZ;

					getChunk(targetX, targetZ, true)->add(player, false);
				}
				else
				{
					getChunkAndAddPlayer(targetX, targetZ, player);
				}
            }
        }
    }

    // Final leg
    facing %= 4;
    for (int k = 0; k < size * 2; k++)
	{
        dx += direction[facing][0];
        dz += direction[facing][1];

		int targetX, targetZ;
		targetX = xc + dx;
		targetZ = zc + dz;
		if( ( size * 2 ) <= maxLegSizeToAddNow )
		{
			if( targetX > maxX ) maxX = targetX;
			if( targetX < minX ) minX = targetX;
			if( targetZ > maxZ ) maxZ = targetZ;
			if( targetZ < minZ ) minZ = targetZ;

			getChunk(targetX, targetZ, true)->add(player, false);
		}
		else
		{
			getChunkAndAddPlayer(targetX, targetZ, player);
		}
    }
    // CraftBukkit end

	player->connection->send( shared_ptr<ChunkVisibilityAreaPacket>( new ChunkVisibilityAreaPacket(minX, maxX, minZ, maxZ) ) );

#ifdef _LARGE_WORLDS
	getLevel()->cache->dontDrop(xc,zc);
#endif

	players.push_back(player);

}

void PlayerChunkMap::remove(shared_ptr<ServerPlayer> player)
{
    int xc = ((int) player->lastMoveX) >> 4;
    int zc = ((int) player->lastMoveZ) >> 4;

    for (int x = xc - radius; x <= xc + radius; x++)
        for (int z = zc - radius; z <= zc + radius; z++)
		{
            PlayerChunk *playerChunk = getChunk(x, z, false);
            if (playerChunk != NULL) playerChunk->remove(player);
        }

    auto it = find(players.begin(), players.end(), player);
    if( players.size() > 0 && it != players.end() )
		players.erase(find(players.begin(),players.end(),player));

	// 4J - added - also remove any queued requests to be added to playerchunks here
    for (auto it = addRequests.begin(); it != addRequests.end();)
    {
		if( it->player == player )
		{
			it = addRequests.erase(it);
		}
		else
		{
			++it;
		}
	}

}

bool PlayerChunkMap::chunkInRange(int x, int z, int xc, int zc)
{
	// If the distance between x and xc
    int xd = x - xc;
    int zd = z - zc;
    if (xd < -radius || xd > radius) return false;
    if (zd < -radius || zd > radius) return false;
    return true;
}

// 4J - have changed this so that we queue requests to add the player to chunks if they
// need to be created, so that we aren't creating potentially 20 chunks per player per tick
void PlayerChunkMap::move(shared_ptr<ServerPlayer> player)
{
    int xc = ((int) player->x) >> 4;
    int zc = ((int) player->z) >> 4;

    double _xd = player->lastMoveX - player->x;
    double _zd = player->lastMoveZ - player->z;
    double dist = _xd * _xd + _zd * _zd;
    if (dist < 8 * 8) return;

    int last_xc = ((int) player->lastMoveX) >> 4;
    int last_zc = ((int) player->lastMoveZ) >> 4;

    int xd = xc - last_xc;
    int zd = zc - last_zc;
    if (xd == 0 && zd == 0) return;

	for (int x = xc - radius; x <= xc + radius; x++)
        for (int z = zc - radius; z <= zc + radius; z++)
		{
			if (!chunkInRange(x, z, last_xc, last_zc))
			{
				// 4J - changed from separate getChunk & add so we can wrap these operations up and queue
				getChunkAndAddPlayer(x, z, player);
			}

			if (!chunkInRange(x - xd, z - zd, xc, zc))
			{
				// 4J - changed from separate getChunk & remove so we can wrap these operations up and queue
				getChunkAndRemovePlayer(x - xd, z - zd, player);
			}
        }

    player->lastMoveX = player->x;
    player->lastMoveZ = player->z;
}

int PlayerChunkMap::getMaxRange()
{
	 return radius * 16 - 16;
}

bool PlayerChunkMap::isPlayerIn(shared_ptr<ServerPlayer> player, int xChunk, int zChunk)
{
	PlayerChunk *chunk = getChunk(xChunk, zChunk, false);

	if(chunk == NULL)
	{
		return false;
	}
	else
	{
        auto it1 = find(chunk->players.begin(), chunk->players.end(), player);
        auto it2 = find(player->chunksToSend.begin(), player->chunksToSend.end(), chunk->pos);
        return it1 != chunk->players.end() && it2 == player->chunksToSend.end();
	}

	//return chunk == NULL ? false : chunk->players->contains(player) && !player->chunksToSend->contains(chunk->pos);
}

int PlayerChunkMap::convertChunkRangeToBlock(int radius)
{
	return radius * 16 - 16;
}

// AP added for Vita so the range can be increased once the level starts
void PlayerChunkMap::setRadius(int newRadius)
{
	if( radius != newRadius )
	{
		PlayerList* players = level->getServer()->getPlayerList();
		for( int i = 0;i < players->players.size();i += 1 )
		{
			shared_ptr<ServerPlayer> player = players->players[i];
			if( player->level == level )
			{
				int xc = ((int) player->x) >> 4;
				int zc = ((int) player->z) >> 4;

				for (int x = xc - newRadius; x <= xc + newRadius; x++)
					for (int z = zc - newRadius; z <= zc + newRadius; z++)
					{
						// check if this chunk is outside the old radius area
						if ( x < xc - radius || x > xc + radius || z < zc - radius || z > zc + radius )
						{
							getChunkAndAddPlayer(x, z, player);
						}
					}
			}
		}

		assert(radius <= MAX_VIEW_DISTANCE);
		assert(radius >= MIN_VIEW_DISTANCE);
		this->radius = newRadius;
	}
}