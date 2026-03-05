#include "stdafx.h"
#include "PlayerList.h"

#include <memory>
#include "PlayerChunkMap.h"
#include "MinecraftServer.h"
#include "Settings.h"
#include "ServerLevel.h"
#include "ServerChunkCache.h"
#include "ServerPlayer.h"
#include "ServerPlayerGameMode.h"
#include "ServerConnection.h"
#include "PendingConnection.h"
#include "PlayerConnection.h"
#include "EntityTracker.h"
#include "..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "..\Minecraft.World\ArrayWithLength.h"
#include "..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\Minecraft.World\net.minecraft.network.h"
#include "..\Minecraft.World\Pos.h"
#include "..\Minecraft.World\ProgressListener.h"
#include "..\Minecraft.World\HellRandomLevelSource.h"
#include "..\Minecraft.World\net.minecraft.world.phys.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "..\Minecraft.World\net.minecraft.world.level.saveddata.h"
#include "..\Minecraft.World\JavaMath.h"
#include "..\Minecraft.World\EntityIO.h"
#if defined(_XBOX) || defined(_WINDOWS64)
#include "Xbox\Network\NetworkPlayerXbox.h"
#elif defined(__PS3__) || defined(__ORBIS__)
#include "Common\Network\Sony\NetworkPlayerSony.h"
#endif

// 4J - this class is fairly substantially altered as there didn't seem any point in porting code for banning, whitelisting, ops etc.

PlayerList::PlayerList(MinecraftServer *server)
{
	playerIo = NULL;

	this->server = server;

	sendAllPlayerInfoIn = 0;
	overrideGameMode = NULL;
	allowCheatsForAllPlayers = false;

#ifdef __PSVITA__
	viewDistance = 3;
#elif defined _LARGE_WORLDS
	viewDistance = 16;
#else
	viewDistance = 10;
#endif

	//int viewDistance = server->settings->getInt(L"view-distance", 10);

#ifdef _WINDOWS64
	maxPlayers = MINECRAFT_NET_MAX_PLAYERS;
#else
	maxPlayers = server->settings->getInt(L"max-players", 20);
#endif
	doWhiteList = false;
	InitializeCriticalSection(&m_kickPlayersCS);
	InitializeCriticalSection(&m_closePlayersCS);
}

PlayerList::~PlayerList()
{
    for (auto& player : players)
    {
		player->connection = nullptr;	// Must remove reference to connection, or else there is a circular dependency
		delete player->gameMode;		// Gamemode also needs deleted as it references back to this player
		player->gameMode = nullptr;
	}

	DeleteCriticalSection(&m_kickPlayersCS);
	DeleteCriticalSection(&m_closePlayersCS);
}

void PlayerList::placeNewPlayer(Connection *connection, shared_ptr<ServerPlayer> player, shared_ptr<LoginPacket> packet)
{
	CompoundTag *playerTag = load(player);

	bool newPlayer = playerTag == NULL;

	player->setLevel(server->getLevel(player->dimension));
	player->gameMode->setLevel((ServerLevel *)player->level);

	// Make sure these privileges are always turned off for the host player
	INetworkPlayer *networkPlayer = connection->getSocket()->getPlayer();
	if(networkPlayer != NULL && networkPlayer->IsHost())
	{
		player->enableAllPlayerPrivileges(true);
		player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_HOST,1);
	}

#if defined(__PS3__) || defined(__ORBIS__)
	// PS3 networking library doesn't automatically assign PlayerUIDs to the network players for anything remote, so need to tell it what to set from the data in this packet now
	if( !g_NetworkManager.IsLocalGame() )
	{
		if( networkPlayer != NULL )
		{
			((NetworkPlayerSony *)networkPlayer)->SetUID( packet->m_onlineXuid );
		}
	}
#endif
#ifdef _WINDOWS64
	if (networkPlayer != NULL && !networkPlayer->IsLocal())
	{
		NetworkPlayerXbox* nxp = (NetworkPlayerXbox*)networkPlayer;
		IQNetPlayer* qnp = nxp->GetQNetPlayer();
		wcsncpy_s(qnp->m_gamertag, 32, player->name.c_str(), _TRUNCATE);
	}
#endif
	// 4J Stu - TU-1 hotfix
	// Fix for #13150 - When a player loads/joins a game after saving/leaving in the nether, sometimes they are spawned on top of the nether and cannot mine down
	validatePlayerSpawnPosition(player);

	//        logger.info(getName() + " logged in with entity id " + playerEntity.entityId + " at (" + playerEntity.x + ", " + playerEntity.y + ", " + playerEntity.z + ")");

	ServerLevel *level = server->getLevel(player->dimension);

	DWORD playerIndex = 0;
	{
		bool usedIndexes[MINECRAFT_NET_MAX_PLAYERS];
		ZeroMemory( &usedIndexes, MINECRAFT_NET_MAX_PLAYERS * sizeof(bool) );
        for (auto& player : players )
        {
			usedIndexes[player->getPlayerIndex()] = true;
		}
		for(unsigned int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i)
		{
			if(!usedIndexes[i])
			{
				playerIndex = i;
				break;
			}
		}
	}
	player->setPlayerIndex( playerIndex );
	player->setCustomSkin( packet->m_playerSkinId );
	player->setCustomCape( packet->m_playerCapeId );

	// 4J-JEV: Moved this here so we can send player-model texture and geometry data.
	shared_ptr<PlayerConnection> playerConnection = shared_ptr<PlayerConnection>(new PlayerConnection(server, connection, player));
	//player->connection = playerConnection;	// Used to be assigned in PlayerConnection ctor but moved out so we can use shared_ptr

	if(newPlayer)
	{
		int mapScale = 3;
#ifdef _LARGE_WORLDS
		int scale = MapItemSavedData::MAP_SIZE * 2 * (1 << mapScale);
		int centreXC = (int) (Math::round(player->x / scale) * scale);
		int centreZC = (int) (Math::round(player->z / scale) * scale);
#else
		// 4J-PB - for Xbox maps, we'll centre them on the origin of the world, since we can fit the whole world in our map
		int centreXC = 0;
		int centreZC = 0;
#endif
		// 4J Added - Give every player a map the first time they join a server
		player->inventory->setItem( 9, shared_ptr<ItemInstance>( new ItemInstance(Item::map_Id, 1, level->getAuxValueForMap(player->getXuid(),0,centreXC, centreZC, mapScale ) ) ) );
		if(app.getGameRuleDefinitions() != NULL)
		{
			app.getGameRuleDefinitions()->postProcessPlayer(player);
		}
	}

	if(!player->customTextureUrl.empty() && player->customTextureUrl.substr(0,3).compare(L"def") != 0 && !app.IsFileInMemoryTextures(player->customTextureUrl))
	{
		if(	server->getConnection()->addPendingTextureRequest(player->customTextureUrl))
		{
#ifndef _CONTENT_PACKAGE
			wprintf(L"Sending texture packet to get custom skin %ls from player %ls\n",player->customTextureUrl.c_str(), player->name.c_str());
#endif
			playerConnection->send(shared_ptr<TextureAndGeometryPacket>( new TextureAndGeometryPacket(player->customTextureUrl,NULL,0) ) );
		}
	}
	else if(!player->customTextureUrl.empty() && app.IsFileInMemoryTextures(player->customTextureUrl))
	{
		// Update the ref count on the memory texture data
		app.AddMemoryTextureFile(player->customTextureUrl,NULL,0);
	}

	if(!player->customTextureUrl2.empty() && player->customTextureUrl2.substr(0,3).compare(L"def") != 0 && !app.IsFileInMemoryTextures(player->customTextureUrl2))
	{
		if(	server->getConnection()->addPendingTextureRequest(player->customTextureUrl2))
		{
#ifndef _CONTENT_PACKAGE
			wprintf(L"Sending texture packet to get custom skin %ls from player %ls\n",player->customTextureUrl2.c_str(), player->name.c_str());
#endif
			playerConnection->send(shared_ptr<TexturePacket>( new TexturePacket(player->customTextureUrl2,NULL,0) ) );
		}
	}
	else if(!player->customTextureUrl2.empty() && app.IsFileInMemoryTextures(player->customTextureUrl2))
	{
		// Update the ref count on the memory texture data
		app.AddMemoryTextureFile(player->customTextureUrl2,NULL,0);
	}

	player->setIsGuest( packet->m_isGuest );

	Pos *spawnPos = level->getSharedSpawnPos();

	updatePlayerGameMode(player, nullptr, level);

	// Update the privileges with the correct game mode
	GameType *gameType = Player::getPlayerGamePrivilege(player->getAllPlayerGamePrivileges(),Player::ePlayerGamePrivilege_CreativeMode) ? GameType::CREATIVE : GameType::SURVIVAL;
	gameType = LevelSettings::validateGameType(gameType->getId());
	if (player->gameMode->getGameModeForPlayer() != gameType)
	{
		player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CreativeMode,player->gameMode->getGameModeForPlayer()->getId() );
	}

	//shared_ptr<PlayerConnection> playerConnection = shared_ptr<PlayerConnection>(new PlayerConnection(server, connection, player));
	player->connection = playerConnection;	// Used to be assigned in PlayerConnection ctor but moved out so we can use shared_ptr

	// 4J Added to store UGC settings
	playerConnection->m_friendsOnlyUGC = packet->m_friendsOnlyUGC;
	playerConnection->m_offlineXUID = packet->m_offlineXuid;
	playerConnection->m_onlineXUID = packet->m_onlineXuid;

	// This player is now added to the list, so incrementing this value invalidates all previous PreLogin packets
	if(packet->m_friendsOnlyUGC) ++server->m_ugcPlayersVersion;

	addPlayerToReceiving( player );

	playerConnection->send( shared_ptr<LoginPacket>( new LoginPacket(L"", player->entityId, level->getLevelData()->getGenerator(), level->getSeed(), player->gameMode->getGameModeForPlayer()->getId(),
		(byte) level->dimension->id, (byte) level->getMaxBuildHeight(), (byte) getMaxPlayers(),
		level->difficulty, TelemetryManager->GetMultiplayerInstanceID(), (BYTE)playerIndex, level->useNewSeaLevel(), player->getAllPlayerGamePrivileges(),
		level->getLevelData()->getXZSize(), level->getLevelData()->getHellScale() ) ) );
	playerConnection->send( shared_ptr<SetSpawnPositionPacket>( new SetSpawnPositionPacket(spawnPos->x, spawnPos->y, spawnPos->z) ) );
	playerConnection->send( shared_ptr<PlayerAbilitiesPacket>( new PlayerAbilitiesPacket(&player->abilities)) );
	playerConnection->send( shared_ptr<SetCarriedItemPacket>( new SetCarriedItemPacket(player->inventory->selected)));
	delete spawnPos;

	updateEntireScoreboard((ServerScoreboard *) level->getScoreboard(), player);

	sendLevelInfo(player, level);

	// 4J-PB - removed, since it needs to be localised in the language the client is in
	//server->players->broadcastAll( shared_ptr<ChatPacket>( new ChatPacket(L"�e" + playerEntity->name + L" joined the game.") ) );
	broadcastAll( shared_ptr<ChatPacket>( new ChatPacket(player->name, ChatPacket::e_ChatPlayerJoinedGame) ) );

	MemSect(14);
	add(player);
	MemSect(0);

	player->doTick(true, true, false);	// 4J - added - force sending of the nearest chunk before the player is teleported, so we have somewhere to arrive on...
	playerConnection->teleport(player->x, player->y, player->z, player->yRot, player->xRot);

	server->getConnection()->addPlayerConnection(playerConnection);
	playerConnection->send( shared_ptr<SetTimePacket>( new SetTimePacket(level->getGameTime(), level->getDayTime(), level->getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT)) ) );

    auto activeEffects = player->getActiveEffects();
    for(MobEffectInstance *effect : *player->getActiveEffects())
	{
		playerConnection->send(shared_ptr<UpdateMobEffectPacket>( new UpdateMobEffectPacket(player->entityId, effect) ) );
	}

	player->initMenu();

	if (playerTag != NULL && playerTag->contains(Entity::RIDING_TAG))
	{
		// this player has been saved with a mount tag
		shared_ptr<Entity> mount = EntityIO::loadStatic(playerTag->getCompound(Entity::RIDING_TAG), level);
		if (mount != NULL)
		{
			mount->forcedLoading = true;
			level->addEntity(mount);
			player->ride(mount);
			mount->forcedLoading = false;
		}
	}

	// If we are joining at the same time as someone in the end on this system is travelling through the win portal,
	// then we should set our wonGame flag to true so that respawning works when the EndPoem is closed
	INetworkPlayer *thisPlayer = player->connection->getNetworkPlayer();
	if( thisPlayer )
	{
		for(auto& servPlayer : players)
		{
			INetworkPlayer* checkPlayer = servPlayer->connection->getNetworkPlayer();
			if(thisPlayer != checkPlayer && checkPlayer && thisPlayer->IsSameSystem( checkPlayer ) && servPlayer->wonGame )
			{
				player->wonGame = true;
				break;
			}
		}
	}
}

void PlayerList::updateEntireScoreboard(ServerScoreboard *scoreboard, shared_ptr<ServerPlayer> player)
{
	//unordered_set<Objective *> objectives;

	//for (PlayerTeam team : scoreboard->getPlayerTeams())
	//{
	//	player->connection->send( shared_ptr<SetPlayerTeamPacket>(new SetPlayerTeamPacket(team, SetPlayerTeamPacket::METHOD_ADD)));
	//}

	//for (int slot = 0; slot < Scoreboard::DISPLAY_SLOTS; slot++)
	//{
	//	Objective objective = scoreboard->getDisplayObjective(slot);

	//	if (objective != NULL && !objectives->contains(objective))
	//	{
	//		vector<shared_ptr<Packet> > *packets = scoreboard->getStartTrackingPackets(objective);

	//		for (Packet packet : packets)
	//		{
	//			player->connection->send(packet);
	//		}

	//		objectives->add(objective);
	//	}
	//}
}

void PlayerList::setLevel(ServerLevelArray levels)
{
	playerIo = levels[0]->getLevelStorage()->getPlayerIO();
}

void PlayerList::changeDimension(shared_ptr<ServerPlayer> player, ServerLevel *from)
{
	ServerLevel *to = player->getLevel();

	if (from != NULL) from->getChunkMap()->remove(player);
	to->getChunkMap()->add(player);

	to->cache->create(((int) player->x) >> 4, ((int) player->z) >> 4);
}

int PlayerList::getMaxRange()
{
	return PlayerChunkMap::convertChunkRangeToBlock(getViewDistance());
}

CompoundTag *PlayerList::load(shared_ptr<ServerPlayer> player)
{
	return playerIo->load(player);
}

void PlayerList::save(shared_ptr<ServerPlayer> player)
{
	playerIo->save(player);
}

// 4J Stu - TU-1 hotifx
// Add this function to take some of the code from the PlayerList::add function with the fixes
// for checking spawn area, especially in the nether. These needed to be done in a different order from before
// Fix for #13150 - When a player loads/joins a game after saving/leaving in the nether, sometimes they are spawned on top of the nether and cannot mine down
void PlayerList::validatePlayerSpawnPosition(shared_ptr<ServerPlayer> player)
{
	// 4J Stu - Some adjustments to make sure the current players position is correct
	// Make sure that the player is on the ground, and in the centre x/z of the current column
	app.DebugPrintf("Original pos is %f, %f, %f in dimension %d\n", player->x, player->y, player->z, player->dimension);

	bool spawnForced = player->isRespawnForced();

	double targetX = 0;
	if(player->x < 0) targetX = Mth::ceil(player->x) - 0.5;
	else targetX = Mth::floor(player->x) + 0.5;

	double targetY = floor(player->y);

	double targetZ = 0;
	if(player->z < 0) targetZ = Mth::ceil(player->z) - 0.5;
	else targetZ = Mth::floor(player->z) + 0.5;

	player->setPos(targetX, targetY, targetZ);

	app.DebugPrintf("New pos is %f, %f, %f in dimension %d\n", player->x, player->y, player->z, player->dimension);

	ServerLevel *level = server->getLevel(player->dimension);
	while (level->getCubes(player, player->bb)->size() != 0)
	{
		player->setPos(player->x, player->y + 1, player->z);
	}
	app.DebugPrintf("Final pos is %f, %f, %f in dimension %d\n", player->x, player->y, player->z, player->dimension);

	// 4J Stu - If we are in the nether and the above while loop has put us above the nether then we have a problem
	// Finding a valid, safe spawn point is potentially computationally expensive (may have to hunt through a large part
	// of the nether) so move the player to their spawn position in the overworld so that they do not lose their inventory
	// 4J Stu - We also use this mechanism to force a spawn point in the overworld for players who were in the save when the reset nether option was applied
	if(level->dimension->id == -1 && player->y > 125)
	{
		app.DebugPrintf("Player in the nether tried to spawn at y = %f, moving to overworld\n", player->y);
		player->setLevel(server->getLevel(0));
		player->gameMode->setLevel(server->getLevel(0));
		player->dimension = 0;

		level = server->getLevel(player->dimension);

		Pos *levelSpawn = level->getSharedSpawnPos();
		player->setPos(levelSpawn->x, levelSpawn->y, levelSpawn->z);
		delete levelSpawn;

		Pos *bedPosition = player->getRespawnPosition();
		if (bedPosition != NULL)
		{
			Pos *respawnPosition = Player::checkBedValidRespawnPosition(server->getLevel(player->dimension), bedPosition, spawnForced);
			if (respawnPosition != NULL)
			{
				player->moveTo(respawnPosition->x + 0.5f, respawnPosition->y + 0.1f, respawnPosition->z + 0.5f, 0, 0);
				player->setRespawnPosition(bedPosition, spawnForced);
			}
			delete bedPosition;
		}
		while (level->getCubes(player, player->bb)->size() != 0)
		{
			player->setPos(player->x, player->y + 1, player->z);
		}

		app.DebugPrintf("Updated pos is %f, %f, %f in dimension %d\n", player->x, player->y, player->z, player->dimension);
	}
}

void PlayerList::add(shared_ptr<ServerPlayer> player)
{
	//broadcastAll(shared_ptr<PlayerInfoPacket>( new PlayerInfoPacket(player->name, true, 1000) ) );
	if( player->connection->getNetworkPlayer() )
	{
		broadcastAll(shared_ptr<PlayerInfoPacket>( new PlayerInfoPacket( player ) ) );
	}

	players.push_back(player);

	// 4J Added
	addPlayerToReceiving(player);

	// Ensure the area the player is spawning in is loaded!
	ServerLevel *level = server->getLevel(player->dimension);

	// 4J Stu - TU-1 hotfix
	// Fix for #13150 - When a player loads/joins a game after saving/leaving in the nether, sometimes they are spawned on top of the nether and cannot mine down
	// Some code from here has been moved to the above validatePlayerSpawnPosition function

	// 4J Stu - Swapped these lines about so that we get the chunk visiblity packet way ahead of all the add tracked entity packets
	// Fix for #9169 - ART : Sign text is replaced with the words �Awaiting approval�.
	changeDimension(player, NULL);
	level->addEntity(player);

	for (int i = 0; i < players.size(); i++)
	{
		shared_ptr<ServerPlayer> op = players.at(i);
		//player->connection->send(shared_ptr<PlayerInfoPacket>( new PlayerInfoPacket(op->name, true, op->latency) ) );
		if( op->connection->getNetworkPlayer() )
		{
			player->connection->send(shared_ptr<PlayerInfoPacket>( new PlayerInfoPacket( op ) ) );
		}
	}

	if(level->isAtLeastOnePlayerSleeping())
	{
		shared_ptr<ServerPlayer> firstSleepingPlayer = nullptr;
		for (unsigned int i = 0; i < players.size(); i++)
		{
			shared_ptr<ServerPlayer> thisPlayer = players[i];
			if(thisPlayer->isSleeping())
			{
				if(firstSleepingPlayer == NULL) firstSleepingPlayer = thisPlayer;
				thisPlayer->connection->send(shared_ptr<ChatPacket>( new ChatPacket(thisPlayer->name, ChatPacket::e_ChatBedMeSleep)));
			}
		}
		player->connection->send(shared_ptr<ChatPacket>( new ChatPacket(firstSleepingPlayer->name, ChatPacket::e_ChatBedPlayerSleep)));
	}
}

void PlayerList::move(shared_ptr<ServerPlayer> player)
{
	player->getLevel()->getChunkMap()->move(player);
}

void PlayerList::remove(shared_ptr<ServerPlayer> player)
{
	save(player);
	//4J Stu - We don't want to save the map data for guests, so when we are sure that the player is gone delete the map
	if(player->isGuest()) playerIo->deleteMapFilesForPlayer(player);
	ServerLevel *level = player->getLevel();
if (player->riding != NULL)
	{
		level->removeEntityImmediately(player->riding);
		app.DebugPrintf("removing player mount");
	}
	level->getTracker()->removeEntity(player);
	level->removeEntity(player);
	level->getChunkMap()->remove(player);
    auto it = find(players.begin(), players.end(), player);
    if( it != players.end() )
	{
		players.erase(it);
	}
	//broadcastAll(shared_ptr<PlayerInfoPacket>( new PlayerInfoPacket(player->name, false, 9999) ) );

	removePlayerFromReceiving(player);
	player->connection = nullptr;		// Must remove reference to connection, or else there is a circular dependency
	delete player->gameMode;		// Gamemode also needs deleted as it references back to this player
	player->gameMode = NULL;

	// 4J Stu - Save all the players currently in the game, which will also free up unused map id slots if required, and remove old players
	saveAll(NULL,false);
}

shared_ptr<ServerPlayer> PlayerList::getPlayerForLogin(PendingConnection *pendingConnection, const wstring& userName, PlayerUID xuid, PlayerUID onlineXuid)
{
#ifdef _WINDOWS64
	if (players.size() >= (unsigned int)MINECRAFT_NET_MAX_PLAYERS)
#else
	if (players.size() >= (unsigned int)maxPlayers)
#endif
	{
		pendingConnection->disconnect(DisconnectPacket::eDisconnect_ServerFull);
		return shared_ptr<ServerPlayer>();
	}
	shared_ptr<ServerPlayer> player = shared_ptr<ServerPlayer>(new ServerPlayer(server, server->getLevel(0), userName, new ServerPlayerGameMode(server->getLevel(0)) ));
	player->gameMode->player = player; // 4J added as had to remove this assignment from ServerPlayer ctor
	player->setXuid( xuid ); // 4J Added
	player->setOnlineXuid( onlineXuid ); // 4J Added
#ifdef _WINDOWS64
	{
		INetworkPlayer* np = pendingConnection->connection->getSocket()->getPlayer();
		if (np != NULL)
		{
			PlayerUID realXuid = np->GetUID();
			player->setXuid(realXuid);
			player->setOnlineXuid(realXuid);
		}
	}
#endif
	// Work out the base server player settings
	INetworkPlayer *networkPlayer = pendingConnection->connection->getSocket()->getPlayer();
	if(networkPlayer != NULL && !networkPlayer->IsHost())
	{
		player->enableAllPlayerPrivileges( app.GetGameHostOption(eGameHostOption_TrustPlayers)>0 );
	}

	// 4J Added
	LevelRuleset *serverRuleDefs = app.getGameRuleDefinitions();
	if(serverRuleDefs != NULL)
	{
		player->gameMode->setGameRules( GameRuleDefinition::generateNewGameRulesInstance(GameRulesInstance::eGameRulesInstanceType_ServerPlayer, serverRuleDefs, pendingConnection->connection) );
	}

	return player;
}

shared_ptr<ServerPlayer> PlayerList::respawn(shared_ptr<ServerPlayer> serverPlayer, int targetDimension, bool keepAllPlayerData)
{
	// How we handle the entity tracker depends on whether we are the primary player currently, and whether there will be any player in the same system in the same dimension once we finish respawning.
	bool isPrimary = canReceiveAllPackets(serverPlayer);	// Is this the primary player in its current dimension?
	int oldDimension = serverPlayer->dimension;
	bool isEmptying = ( targetDimension != oldDimension);		// We're not emptying this dimension on this machine if this player is going back into the same dimension

	// Also consider if there is another player on this machine which is in the same dimension and can take over as primary player
	if( isEmptying )
	{
		INetworkPlayer *thisPlayer = serverPlayer->connection->getNetworkPlayer();

		for( unsigned int i = 0; i < players.size(); i++ )
		{
			shared_ptr<ServerPlayer> ep = players[i];
			if( ep == serverPlayer ) continue;
			if( ep->dimension !=  oldDimension ) continue;

			INetworkPlayer * otherPlayer = ep->connection->getNetworkPlayer();
			if( otherPlayer != NULL && thisPlayer->IsSameSystem(otherPlayer) )
			{
				// There's another player here in the same dimension - we're not the last one out
				isEmptying = false;
			}
		}
	}

	// Now we know where we stand, the actions to take are as follows:
	// (1) if this isn't the primary player, then we just need to remove it from the entity tracker
	// (2) if this Is the primary player then:
	//		(a) if isEmptying is true, then remove the player from the tracker, and send "remove entity" packets for anything seen (this is the original behaviour of the code)
	//		(b) if isEmptying is false, then we'll be transferring control of entity tracking to another player

	if( isPrimary )
	{
		if( isEmptying )
		{
			app.DebugPrintf("Emptying this dimension\n");
			serverPlayer->getLevel()->getTracker()->clear(serverPlayer);
		}
		else
		{
			app.DebugPrintf("Transferring... storing flags\n");
			serverPlayer->getLevel()->getTracker()->removeEntity(serverPlayer);
		}
	}
	else
	{
		app.DebugPrintf("Not primary player\n");
		serverPlayer->getLevel()->getTracker()->removeEntity(serverPlayer);
	}

	serverPlayer->getLevel()->getChunkMap()->remove(serverPlayer);
    auto it = find(players.begin(), players.end(), serverPlayer);
    if( it != players.end() )
	{
		players.erase(it);
	}
	server->getLevel(serverPlayer->dimension)->removeEntityImmediately(serverPlayer);

	Pos *bedPosition = serverPlayer->getRespawnPosition();
	bool spawnForced = serverPlayer->isRespawnForced();

	removePlayerFromReceiving(serverPlayer);
	serverPlayer->dimension = targetDimension;

	EDefaultSkins skin = serverPlayer->getPlayerDefaultSkin();
	DWORD playerIndex = serverPlayer->getPlayerIndex();

	PlayerUID playerXuid = serverPlayer->getXuid();
	PlayerUID playerOnlineXuid = serverPlayer->getOnlineXuid();

	shared_ptr<ServerPlayer> player = shared_ptr<ServerPlayer>(new ServerPlayer(server, server->getLevel(serverPlayer->dimension), serverPlayer->getName(), new ServerPlayerGameMode(server->getLevel(serverPlayer->dimension))));
	player->connection = serverPlayer->connection;
	player->restoreFrom(serverPlayer, keepAllPlayerData);
	if (keepAllPlayerData)
	{
		// Fix for #81759 - TU9: Content: Gameplay: Entering The End Exit Portal replaces the Player's currently held item with the first one from the Quickbar
		player->inventory->selected = serverPlayer->inventory->selected;
	}
	player->gameMode->player = player; // 4J added as had to remove this assignment from ServerPlayer ctor
	player->setXuid( playerXuid ); // 4J Added
	player->setOnlineXuid( playerOnlineXuid ); // 4J Added

	// 4J Stu - Don't reuse the id. If we do, then the player can be re-added after being removed, but the add packet gets sent before the remove packet
	//player->entityId = serverPlayer->entityId;

	player->setPlayerDefaultSkin( skin );
	player->setIsGuest( serverPlayer->isGuest() );
	player->setPlayerIndex( playerIndex );
	player->setCustomSkin( serverPlayer->getCustomSkin() );
	player->setCustomCape( serverPlayer->getCustomCape() );
	player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All, serverPlayer->getAllPlayerGamePrivileges());
	player->gameMode->setGameRules( serverPlayer->gameMode->getGameRules() );
	player->dimension = targetDimension;

	// 4J Stu - Added this as we need to know earlier if the player is the player for this connection so that
	// we can work out if they are the primary for the system and can receive all packets
	player->connection->setPlayer( player );

	addPlayerToReceiving(player);

	ServerLevel *level = server->getLevel(serverPlayer->dimension);

	// reset the player's game mode (first pick from old, then copy level if
	// necessary)
	updatePlayerGameMode(player, serverPlayer, level);

	if(serverPlayer->wonGame && targetDimension == oldDimension && serverPlayer->getHealth() > 0)
	{
		// If the player is still alive and respawning to the same dimension, they are just being added back from someone else viewing the Win screen
		player->moveTo(serverPlayer->x, serverPlayer->y, serverPlayer->z, serverPlayer->yRot, serverPlayer->xRot);
		if(bedPosition != NULL)
		{
			player->setRespawnPosition(bedPosition, spawnForced);
			delete bedPosition;
		}
		// Fix for #81759 - TU9: Content: Gameplay: Entering The End Exit Portal replaces the Player's currently held item with the first one from the Quickbar
		player->inventory->selected = serverPlayer->inventory->selected;
	}
	else if (bedPosition != NULL)
	{
		Pos *respawnPosition = Player::checkBedValidRespawnPosition(server->getLevel(serverPlayer->dimension), bedPosition, spawnForced);
		if (respawnPosition != NULL)
		{
			player->moveTo(respawnPosition->x + 0.5f, respawnPosition->y + 0.1f, respawnPosition->z + 0.5f, 0, 0);
			player->setRespawnPosition(bedPosition, spawnForced);
		}
		else
		{
			player->connection->send( shared_ptr<GameEventPacket>( new GameEventPacket(GameEventPacket::NO_RESPAWN_BED_AVAILABLE, 0) ) );
		}
		delete bedPosition;
	}

	// Ensure the area the player is spawning in is loaded!
	level->cache->create(((int) player->x) >> 4, ((int) player->z) >> 4);

	while (!level->getCubes(player, player->bb)->empty())
	{
		player->setPos(player->x, player->y + 1, player->z);
	}

	player->connection->send( std::make_shared<RespawnPacket>( (char) player->dimension, player->level->getSeed(), player->level->getMaxBuildHeight(),
		player->gameMode->getGameModeForPlayer(), level->difficulty, level->getLevelData()->getGenerator(),
		player->level->useNewSeaLevel(), player->entityId, level->getLevelData()->getXZSize(), level->getLevelData()->getHellScale() ) );
	player->connection->teleport(player->x, player->y, player->z, player->yRot, player->xRot);
	player->connection->send( std::make_shared<SetExperiencePacket>( player->experienceProgress, player->totalExperience, player->experienceLevel) );

	if(keepAllPlayerData)
	{
		vector<MobEffectInstance *> *activeEffects = player->getActiveEffects();
		for(MobEffectInstance *effect : *activeEffects)
		{
			player->connection->send(std::make_shared<UpdateMobEffectPacket>( player->entityId, effect ) );
		}
		delete activeEffects;
		player->getEntityData()->markDirty(Mob::DATA_EFFECT_COLOR_ID);
	}

	sendLevelInfo(player, level);

	level->getChunkMap()->add(player);
	level->addEntity(player);
	players.push_back(player);

	player->initMenu();
	player->setHealth(player->getHealth());

	// 4J-JEV - Dying before this point in the tutorial is pretty annoying,
	// making sure to remove health/hunger and give you back your meat.
	if( Minecraft::GetInstance()->isTutorial()
		&& (!Minecraft::GetInstance()->gameMode->getTutorial()->isStateCompleted(e_Tutorial_State_Food_Bar)) )
	{
		app.getGameRuleDefinitions()->postProcessPlayer(player);
	}

	if( oldDimension == 1 && player->dimension != 1 )
	{
		player->displayClientMessage(IDS_PLAYER_LEFT_END);
	}

	return player;

}

void PlayerList::toggleDimension(shared_ptr<ServerPlayer> player, int targetDimension)
{
	int lastDimension = player->dimension;
	// How we handle the entity tracker depends on whether we are the primary player currently, and whether there will be any player in the same system in the same dimension once we finish respawning.
	bool isPrimary = canReceiveAllPackets(player);	// Is this the primary player in its current dimension?
	bool isEmptying = true;

	// Also consider if there is another player on this machine which is in the same dimension and can take over as primary player
	INetworkPlayer *thisPlayer = player->connection->getNetworkPlayer();

	for( unsigned int i = 0; i < players.size(); i++ )
	{
		shared_ptr<ServerPlayer> ep = players[i];
		if( ep == player ) continue;
		if( ep->dimension !=  lastDimension ) continue;

		INetworkPlayer * otherPlayer = ep->connection->getNetworkPlayer();
		if( otherPlayer != NULL && thisPlayer->IsSameSystem(otherPlayer) )
		{
			// There's another player here in the same dimension - we're not the last one out
			isEmptying = false;
		}
	}


	// Now we know where we stand, the actions to take are as follows:
	// (1) if this isn't the primary player, then we just need to remove it from the entity tracker
	// (2) if this Is the primary player then:
	//		(a) if isEmptying is true, then remove the player from the tracker, and send "remove entity" packets for anything seen (this is the original behaviour of the code)
	//		(b) if isEmptying is false, then we'll be transferring control of entity tracking to another player

	if( isPrimary )
	{
		if( isEmptying )
		{
			app.DebugPrintf("Toggle... Emptying this dimension\n");
			player->getLevel()->getTracker()->clear(player);
		}
		else
		{
			app.DebugPrintf("Toggle...  transferring\n");
			player->getLevel()->getTracker()->removeEntity(player);
		}
	}
	else
	{
		app.DebugPrintf("Toggle...  Not primary player\n");
		player->getLevel()->getTracker()->removeEntity(player);
	}

	ServerLevel *oldLevel = server->getLevel(player->dimension);

	// 4J Stu - Do this much earlier so we don't end up unloading chunks in the wrong dimension
	player->getLevel()->getChunkMap()->remove(player);

	if(player->dimension != 1 && targetDimension == 1)
	{
		player->displayClientMessage(IDS_PLAYER_ENTERED_END);
	}
	else if( player->dimension == 1 )
	{
		player->displayClientMessage(IDS_PLAYER_LEFT_END);
	}

	player->dimension = targetDimension;

	ServerLevel *newLevel = server->getLevel(player->dimension);

	// 4J Stu - Fix for #46423 - TU5: Art: Code: No burning animation visible after entering The Nether while burning
	player->clearFire(); // Stop burning if travelling through a portal

	// 4J Stu Added so that we remove entities from the correct level, after the respawn packet we will be in the wrong level
	player->flushEntitiesToRemove();

	player->connection->send( shared_ptr<RespawnPacket>( new RespawnPacket((char) player->dimension, newLevel->getSeed(), newLevel->getMaxBuildHeight(),
		player->gameMode->getGameModeForPlayer(), newLevel->difficulty, newLevel->getLevelData()->getGenerator(),
		newLevel->useNewSeaLevel(), player->entityId, newLevel->getLevelData()->getXZSize(), newLevel->getLevelData()->getHellScale()) ) );

	oldLevel->removeEntityImmediately(player);
	player->removed = false;

	repositionAcrossDimension(player, lastDimension, oldLevel, newLevel);
	changeDimension(player, oldLevel);

	player->gameMode->setLevel(newLevel);

	// Resend the teleport if we haven't yet sent the chunk they will land on
	if( !g_NetworkManager.SystemFlagGet(player->connection->getNetworkPlayer(),ServerPlayer::getFlagIndexForChunk( ChunkPos(player->xChunk,player->zChunk), player->level->dimension->id ) ) )
	{
		player->connection->teleport(player->x, player->y, player->z, player->yRot, player->xRot, false);
		// Force sending of the current chunk
		player->doTick(true, true, true);
	}

	player->connection->teleport(player->x, player->y, player->z, player->yRot, player->xRot);

	// 4J Stu - Fix for #64683 - Customer Encountered: TU7: Content: Gameplay: Potion effects are removed after using the Nether Portal
	vector<MobEffectInstance *> *activeEffects = player->getActiveEffects();
	for(MobEffectInstance *effect : *activeEffects)
	{
		player->connection->send(std::make_shared<UpdateMobEffectPacket>( player->entityId, effect ) );
	}
	delete activeEffects;
	player->getEntityData()->markDirty(Mob::DATA_EFFECT_COLOR_ID);

	sendLevelInfo(player, newLevel);
	sendAllPlayerInfo(player);
}

void PlayerList::repositionAcrossDimension(shared_ptr<Entity> entity, int lastDimension, ServerLevel *oldLevel, ServerLevel *newLevel)
{
	double xt = entity->x;
	double zt = entity->z;
	double xOriginal = entity->x;
	double yOriginal = entity->y;
	double zOriginal = entity->z;
	float yRotOriginal = entity->yRot;
	double scale = newLevel->getLevelData()->getHellScale(); // 4J Scale was 8 but this is all we can fit in
	if (entity->dimension == -1)
	{
		xt /= scale;
		zt /= scale;
		entity->moveTo(xt, entity->y, zt, entity->yRot, entity->xRot);
		if (entity->isAlive())
		{
			oldLevel->tick(entity, false);
		}
	}
	else if (entity->dimension == 0)
	{
		xt *= scale;
		zt *= scale;
		entity->moveTo(xt, entity->y, zt, entity->yRot, entity->xRot);
		if (entity->isAlive())
		{
			oldLevel->tick(entity, false);
		}
	}
	else
	{
		Pos *p;

		if (lastDimension == 1)
		{
			// Coming from the end
			p = newLevel->getSharedSpawnPos();
		}
		else
		{
			// Going to the end
			p = newLevel->getDimensionSpecificSpawn();
		}

		xt = p->x;
		entity->y = p->y;
		zt = p->z;
		delete p;
		entity->moveTo(xt, entity->y, zt, 90, 0);
		if (entity->isAlive())
		{
			oldLevel->tick(entity, false);
		}
	}

	if(entity->GetType() == eTYPE_SERVERPLAYER)
	{
		shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(entity);
		removePlayerFromReceiving(player, false, lastDimension);
		addPlayerToReceiving(player);
	}

	if (lastDimension != 1)
	{
		xt = (double) Mth::clamp((int) xt, -Level::MAX_LEVEL_SIZE + 128, Level::MAX_LEVEL_SIZE - 128);
		zt = (double) Mth::clamp((int) zt, -Level::MAX_LEVEL_SIZE + 128, Level::MAX_LEVEL_SIZE - 128);
		if (entity->isAlive())
		{
			newLevel->addEntity(entity);
			entity->moveTo(xt, entity->y, zt, entity->yRot, entity->xRot);
			newLevel->tick(entity, false);
			newLevel->cache->autoCreate = true;
			newLevel->getPortalForcer()->force(entity, xOriginal, yOriginal, zOriginal, yRotOriginal);
			newLevel->cache->autoCreate = false;
		}
	}

	entity->setLevel(newLevel);
}

void PlayerList::tick()
{
	// 4J - brought changes to how often this is sent forward from 1.2.3
	if (++sendAllPlayerInfoIn > SEND_PLAYER_INFO_INTERVAL)
	{
		sendAllPlayerInfoIn = 0;
	}

	if (sendAllPlayerInfoIn < players.size())
	{
		shared_ptr<ServerPlayer> op = players[sendAllPlayerInfoIn];
		//broadcastAll(shared_ptr<PlayerInfoPacket>( new PlayerInfoPacket(op->name, true, op->latency) ) );
		if( op->connection->getNetworkPlayer() )
		{
			broadcastAll(shared_ptr<PlayerInfoPacket>( new PlayerInfoPacket( op ) ) );
		}
	}

	EnterCriticalSection(&m_closePlayersCS);
	while(!m_smallIdsToClose.empty())
	{
		BYTE smallId = m_smallIdsToClose.front();
		m_smallIdsToClose.pop_front();

		shared_ptr<ServerPlayer> player = nullptr;

		for(unsigned int i = 0; i < players.size(); i++)
		{
			shared_ptr<ServerPlayer> p = players.at(i);
			// 4J Stu - May be being a bit overprotective with all the NULL checks, but adding late in TU7 so want to be safe
			if (p != NULL && p->connection != NULL && p->connection->connection != NULL && p->connection->connection->getSocket() != NULL && p->connection->connection->getSocket()->getSmallId() == smallId )
			{
				player = p;
				break;
			}
		}

		if (player != NULL)
		{
			player->connection->disconnect( DisconnectPacket::eDisconnect_Closed );
		}
	}
	LeaveCriticalSection(&m_closePlayersCS);

	EnterCriticalSection(&m_kickPlayersCS);
	while(!m_smallIdsToKick.empty())
	{
		BYTE smallId = m_smallIdsToKick.front();
		m_smallIdsToKick.pop_front();
		INetworkPlayer *selectedPlayer = g_NetworkManager.GetPlayerBySmallId(smallId);
		if( selectedPlayer != NULL )
		{
			if( selectedPlayer->IsLocal() != TRUE )
			{
				//#ifdef _XBOX
				PlayerUID xuid = selectedPlayer->GetUID();
				// Kick this player from the game
				shared_ptr<ServerPlayer> player = nullptr;

				for(unsigned int i = 0; i < players.size(); i++)
				{
					shared_ptr<ServerPlayer> p = players.at(i);
					PlayerUID playersXuid = p->getOnlineXuid();
					if (p != NULL && ProfileManager.AreXUIDSEqual(playersXuid, xuid ) )
					{
						player = p;
						break;
					}
				}

				if (player != NULL)
				{
					m_bannedXuids.push_back( player->getOnlineXuid() );
					// 4J Stu - If we have kicked a player, make sure that they have no privileges if they later try to join the world when trust players is off
					player->enableAllPlayerPrivileges( false );
					player->connection->setWasKicked();
					player->connection->send( shared_ptr<DisconnectPacket>( new DisconnectPacket(DisconnectPacket::eDisconnect_Kicked) ));
				}
				//#endif
			}
		}
	}
	LeaveCriticalSection(&m_kickPlayersCS);

	// Check our receiving players, and if they are dead see if we can replace them
	for(unsigned int dim = 0; dim < 2; ++dim)
	{
		for(unsigned int i = 0; i < receiveAllPlayers[dim].size(); ++i)
		{
			shared_ptr<ServerPlayer> currentPlayer  = receiveAllPlayers[dim][i];
			if(currentPlayer->removed)
			{
				shared_ptr<ServerPlayer> newPlayer = findAlivePlayerOnSystem(currentPlayer);
				if(newPlayer != NULL)
				{
					receiveAllPlayers[dim][i] = newPlayer;
					app.DebugPrintf("Replacing primary player %ls with %ls in dimension %d\n", currentPlayer->name.c_str(), newPlayer->name.c_str(), dim);
				}
			}
		}
	}
}

bool PlayerList::isTrackingTile(int x, int y, int z, int dimension)
{
	return server->getLevel(dimension)->getChunkMap()->isTrackingTile(x, y, z);
}

// 4J added - make sure that any tile updates for the chunk at this location get prioritised for sending
void PlayerList::prioritiseTileChanges(int x, int y, int z, int dimension)
{
	server->getLevel(dimension)->getChunkMap()->prioritiseTileChanges(x, y, z);
}

void PlayerList::broadcastAll(shared_ptr<Packet> packet)
{
	for (unsigned int i = 0; i < players.size(); i++)
	{
		shared_ptr<ServerPlayer> player = players[i];
		player->connection->send(packet);
	}
}

void PlayerList::broadcastAll(shared_ptr<Packet> packet, int dimension)
{
	for (unsigned int i = 0; i < players.size(); i++)
	{
		shared_ptr<ServerPlayer> player = players[i];
		if (player->dimension == dimension) player->connection->send(packet);
	}
}

wstring PlayerList::getPlayerNames()
{
	wstring msg;
	for (unsigned int i = 0; i < players.size(); i++)
	{
		if (i > 0) msg += L", ";
		msg += players[i]->name;
	}
	return msg;
}

bool PlayerList::isWhiteListed(const wstring& name)
{
	return true;
}

bool PlayerList::isOp(const wstring& name)
{
	return false;
}

bool PlayerList::isOp(shared_ptr<ServerPlayer> player)
{
	bool cheatsEnabled = app.GetGameHostOption(eGameHostOption_CheatsEnabled);
#ifdef _DEBUG_MENUS_ENABLED
	cheatsEnabled = cheatsEnabled || app.GetUseDPadForDebug();
#endif
	INetworkPlayer *networkPlayer = player->connection->getNetworkPlayer();
	bool isOp = cheatsEnabled && (player->isModerator() || (networkPlayer != NULL && networkPlayer->IsHost()));
	return isOp;
}

shared_ptr<ServerPlayer> PlayerList::getPlayer(const wstring& name)
{
	for (unsigned int i = 0; i < players.size(); i++)
	{
		shared_ptr<ServerPlayer> p = players[i];
		if (p->name == name)	// 4J - used to be case insensitive (using equalsIgnoreCase) - imagine we'll be shifting to XUIDs anyway
		{
			return p;
		}
	}
	return nullptr;
}

// 4J Added
shared_ptr<ServerPlayer> PlayerList::getPlayer(PlayerUID uid)
{
	for (unsigned int i = 0; i < players.size(); i++)
	{
		shared_ptr<ServerPlayer> p = players[i];
		if (p->getXuid() == uid || p->getOnlineXuid() == uid)	// 4J - used to be case insensitive (using equalsIgnoreCase) - imagine we'll be shifting to XUIDs anyway
		{
			return p;
		}
	}
	return nullptr;
}

shared_ptr<ServerPlayer> PlayerList::getNearestPlayer(Pos *position, int range)
{
	if (players.empty()) return nullptr;
	if (position == NULL) return players.at(0);
	shared_ptr<ServerPlayer> current = nullptr;
	double dist = -1;
	int rangeSqr = range * range;

	for (int i = 0; i < players.size(); i++)
	{
		shared_ptr<ServerPlayer> next = players.at(i);
		double newDist = position->distSqr(next->getCommandSenderWorldPosition());

		if ((dist == -1 || newDist < dist) && (range <= 0 || newDist <= rangeSqr))
		{
			dist = newDist;
			current = next;
		}
	}

	return current;
}

vector<ServerPlayer> *PlayerList::getPlayers(Pos *position, int rangeMin, int rangeMax, int count, int mode, int levelMin, int levelMax, unordered_map<wstring, int> *scoreRequirements, const wstring &playerName,	const wstring &teamName, Level *level)
{
	app.DebugPrintf("getPlayers NOT IMPLEMENTED!");
	return NULL;

	/*if (players.empty()) return NULL;
	vector<shared_ptr<ServerPlayer> > result = new vector<shared_ptr<ServerPlayer> >();
	bool reverse = count < 0;
	bool playerNameNot = !playerName.empty() && playerName.startsWith("!");
	bool teamNameNot = !teamName.empty() && teamName.startsWith("!");
	int rangeMinSqr = rangeMin * rangeMin;
	int rangeMaxSqr = rangeMax * rangeMax;
	count = Mth.abs(count);

	if (playerNameNot) playerName = playerName.substring(1);
	if (teamNameNot) teamName = teamName.substring(1);

	for (int i = 0; i < players.size(); i++) {
	ServerPlayer player = players.get(i);

	if (level != null && player.level != level) continue;
	if (playerName != null) {
	if (playerNameNot == playerName.equalsIgnoreCase(player.getAName())) continue;
	}
	if (teamName != null) {
	Team team = player.getTeam();
	String actualName = team == null ? "" : team.getName();
	if (teamNameNot == teamName.equalsIgnoreCase(actualName)) continue;
	}

	if (position != null && (rangeMin > 0 || rangeMax > 0)) {
	float distance = position.distSqr(player.getCommandSenderWorldPosition());
	if (rangeMin > 0 && distance < rangeMinSqr) continue;
	if (rangeMax > 0 && distance > rangeMaxSqr) continue;
	}

	if (!meetsScoreRequirements(player, scoreRequirements)) continue;

	if (mode != GameType.NOT_SET.getId() && mode != player.gameMode.getGameModeForPlayer().getId()) continue;
	if (levelMin > 0 && player.experienceLevel < levelMin) continue;
	if (player.experienceLevel > levelMax) continue;

	result.add(player);
	}

	if (position != null) Collections.sort(result, new PlayerDistanceComparator(position));
	if (reverse) Collections.reverse(result);
	if (count > 0) result = result.subList(0, Math.min(count, result.size()));

	return result;*/
}

bool PlayerList::meetsScoreRequirements(shared_ptr<Player> player, unordered_map<wstring, int> scoreRequirements)
{
	app.DebugPrintf("meetsScoreRequirements NOT IMPLEMENTED!");
	return false;

	//if (scoreRequirements == null || scoreRequirements.size() == 0) return true;

	//for (Map.Entry<String, Integer> requirement : scoreRequirements.entrySet()) {
	//	String name = requirement.getKey();
	//	boolean min = false;

	//	if (name.endsWith("_min") && name.length() > 4) {
	//		min = true;
	//		name = name.substring(0, name.length() - 4);
	//	}

	//	Scoreboard scoreboard = player.getScoreboard();
	//	Objective objective = scoreboard.getObjective(name);
	//	if (objective == null) return false;
	//	Score score = player.getScoreboard().getPlayerScore(player.getAName(), objective);
	//	int value = score.getScore();

	//	if (value < requirement.getValue() && min) {
	//		return false;
	//	} else if (value > requirement.getValue() && !min) {
	//		return false;
	//	}
	//}

	//return true;
}

void PlayerList::sendMessage(const wstring& name, const wstring& message)
{
	shared_ptr<ServerPlayer> player = getPlayer(name);
	if (player != NULL)
	{
		player->connection->send( shared_ptr<ChatPacket>( new ChatPacket(message) ) );
	}
}

void PlayerList::broadcast(double x, double y, double z, double range, int dimension, shared_ptr<Packet> packet)
{
	broadcast(nullptr, x, y, z, range, dimension, packet);
}

void PlayerList::broadcast(shared_ptr<Player> except, double x, double y, double z, double range, int dimension, shared_ptr<Packet> packet)
{
	// 4J - altered so that we don't send to the same machine more than once. Add the source player to the machines we have "sent" to as it doesn't need to go to that
	// machine either
	vector< shared_ptr<ServerPlayer> > sentTo;
	if( except != NULL )
	{
		sentTo.push_back(dynamic_pointer_cast<ServerPlayer>(except));
	}

	for (unsigned int i = 0; i < players.size(); i++)
	{
		shared_ptr<ServerPlayer> p = players[i];
		if (p == except) continue;
		if (p->dimension != dimension) continue;

		// 4J - don't send to the same machine more than once
		bool dontSend = false;
		if( sentTo.size() )
		{
			INetworkPlayer *thisPlayer = p->connection->getNetworkPlayer();
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


		double xd = x - p->x;
		double yd = y - p->y;
		double zd = z - p->z;
		if (xd * xd + yd * yd + zd * zd < range * range)
		{
#if 0 // _DEBUG
			shared_ptr<LevelSoundPacket> SoundPacket= dynamic_pointer_cast<LevelSoundPacket>(packet);

			if(SoundPacket)
			{

				app.DebugPrintf("---broadcast - eSoundType_[%d] ",SoundPacket->getSound());
				OutputDebugStringW(ConsoleSoundEngine::wchSoundNames[SoundPacket->getSound()]);
				app.DebugPrintf("\n");
			}
#endif
			p->connection->send(packet);
			sentTo.push_back( p );
		}
	}

}

void PlayerList::saveAll(ProgressListener *progressListener, bool bDeleteGuestMaps /*= false*/)
{
	if(progressListener != NULL) progressListener->progressStart(IDS_PROGRESS_SAVING_PLAYERS);
	// 4J - playerIo can be NULL if we have have to exit a game really early on due to network failure
	if(playerIo)
	{
		playerIo->saveAllCachedData();
		for (unsigned int i = 0; i < players.size(); i++)
		{
			playerIo->save(players[i]);

			//4J Stu - We don't want to save the map data for guests, so when we are sure that the player is gone delete the map
			if(bDeleteGuestMaps && players[i]->isGuest()) playerIo->deleteMapFilesForPlayer(players[i]);

			if(progressListener != NULL) progressListener->progressStagePercentage((i * 100)/ ((int)players.size()));
		}
		playerIo->clearOldPlayerFiles();
		playerIo->saveMapIdLookup();
	}
}

void PlayerList::whiteList(const wstring& playerName)
{
}

void PlayerList::blackList(const wstring& playerName)
{
}

void PlayerList::reloadWhitelist()
{
}

void PlayerList::sendLevelInfo(shared_ptr<ServerPlayer> player, ServerLevel *level)
{
	player->connection->send( shared_ptr<SetTimePacket>( new SetTimePacket(level->getGameTime(), level->getDayTime(), level->getGameRules()->getBoolean(GameRules::RULE_DAYLIGHT)) ) );
	if (level->isRaining())
	{
		player->connection->send( shared_ptr<GameEventPacket>( new GameEventPacket(GameEventPacket::START_RAINING, 0) ) );
	}
	else
	{
		// 4J Stu - Fix for #44836 - Customer Encountered: Out of Sync Weather [A-10]
		// If it was raining when the player left the level, and is now not raining we need to make sure that state is updated
		player->connection->send( shared_ptr<GameEventPacket>( new GameEventPacket(GameEventPacket::STOP_RAINING, 0) ) );
	}

	// send the stronghold position if there is one
	if((level->dimension->id==0) && level->getLevelData()->getHasStronghold())
	{
		player->connection->send( shared_ptr<XZPacket>( new XZPacket(XZPacket::STRONGHOLD,level->getLevelData()->getXStronghold(),level->getLevelData()->getZStronghold()) ) );
	}
}

void PlayerList::sendAllPlayerInfo(shared_ptr<ServerPlayer> player)
{
	player->refreshContainer(player->inventoryMenu);
	player->resetSentInfo();
	player->connection->send( shared_ptr<SetCarriedItemPacket>( new SetCarriedItemPacket(player->inventory->selected)) );
}

int PlayerList::getPlayerCount()
{
	return (int)players.size();
}

int PlayerList::getPlayerCount(ServerLevel *level)
{
	int count = 0;

	for(auto& player : players)
	{
		if( player->level == level ) ++count;
	}

	return count;
}

int PlayerList::getMaxPlayers()
{
	return maxPlayers;
}

MinecraftServer *PlayerList::getServer()
{
	return server;
}

int PlayerList::getViewDistance()
{
	return viewDistance;
}

void PlayerList::setOverrideGameMode(GameType *gameMode)
{
	overrideGameMode = gameMode;
}

void PlayerList::updatePlayerGameMode(shared_ptr<ServerPlayer> newPlayer, shared_ptr<ServerPlayer> oldPlayer, Level *level)
{

	// reset the player's game mode (first pick from old, then copy level if
	// necessary)
	if (oldPlayer != NULL)
	{
		newPlayer->gameMode->setGameModeForPlayer(oldPlayer->gameMode->getGameModeForPlayer());
	}
	else if (overrideGameMode != NULL)
	{
		newPlayer->gameMode->setGameModeForPlayer(overrideGameMode);
	}
	newPlayer->gameMode->updateGameMode(level->getLevelData()->getGameType());
}

void PlayerList::setAllowCheatsForAllPlayers(bool allowCommands)
{
	this->allowCheatsForAllPlayers = allowCommands;
}

shared_ptr<ServerPlayer> PlayerList::findAlivePlayerOnSystem(shared_ptr<ServerPlayer> player)
{
	int dimIndex, playerDim;
	dimIndex = playerDim = player->dimension;
	if( dimIndex == -1 ) dimIndex = 1;
	else if( dimIndex == 1) dimIndex = 2;

	INetworkPlayer *thisPlayer = player->connection->getNetworkPlayer();
	if( thisPlayer )
	{
		for(auto& newPlayer : players)
		{
			INetworkPlayer *otherPlayer = newPlayer->connection->getNetworkPlayer();

			if( !newPlayer->removed &&
				newPlayer != player &&
				newPlayer->dimension == playerDim &&
				otherPlayer &&
				otherPlayer->IsSameSystem( thisPlayer )
				)
			{
				return newPlayer;
			}
		}
	}

	return nullptr;
}

void PlayerList::removePlayerFromReceiving(shared_ptr<ServerPlayer> player, bool usePlayerDimension /*= true*/, int dimension /*= 0*/)
{
	int dimIndex, playerDim;
	dimIndex = playerDim = usePlayerDimension ? player->dimension : dimension;
	if( dimIndex == -1 ) dimIndex = 1;
	else if( dimIndex == 1) dimIndex = 2;

#ifndef _CONTENT_PACKAGE
	app.DebugPrintf("Requesting remove player %ls as primary in dimension %d\n", player->name.c_str(), dimIndex);
#endif
	bool playerRemoved = false;

    auto it = find(receiveAllPlayers[dimIndex].begin(), receiveAllPlayers[dimIndex].end(), player);
    if( it != receiveAllPlayers[dimIndex].end() )
	{
#ifndef _CONTENT_PACKAGE
		app.DebugPrintf("Remove: Removing player %ls as primary in dimension %d\n", player->name.c_str(), dimIndex);
#endif
		receiveAllPlayers[dimIndex].erase(it);
		playerRemoved = true;
	}

	INetworkPlayer *thisPlayer = player->connection->getNetworkPlayer();
	if( thisPlayer && playerRemoved )
	{
		for(auto& newPlayer : players)
		{
			INetworkPlayer *otherPlayer = newPlayer->connection->getNetworkPlayer();

			if( newPlayer != player &&
				newPlayer->dimension == playerDim &&
				otherPlayer &&
				otherPlayer->IsSameSystem( thisPlayer )
				)
			{
#ifndef _CONTENT_PACKAGE
				app.DebugPrintf("Remove: Adding player %ls as primary in dimension %d\n", newPlayer->name.c_str(), dimIndex);
#endif
				receiveAllPlayers[dimIndex].push_back( newPlayer );
				break;
			}
		}
	}
	else if( thisPlayer == NULL )
	{
#ifndef _CONTENT_PACKAGE
		app.DebugPrintf("Remove: Qnet player for %ls was NULL so re-checking all players\n", player->name.c_str() );
#endif
		// 4J Stu - Something went wrong, or possibly the QNet player left before we got here.
		// Re-check all active players and make sure they have someone on their system to receive all packets
		for(auto& newPlayer : players)
		{
			INetworkPlayer *checkingPlayer = newPlayer->connection->getNetworkPlayer();

			if( checkingPlayer )
			{
				int newPlayerDim = 0;
				if( newPlayer->dimension == -1 ) newPlayerDim = 1;
				else if( newPlayer->dimension == 1) newPlayerDim = 2;
				bool foundPrimary = false;
				for(auto& primaryPlayer : receiveAllPlayers[newPlayerDim])
				{
					INetworkPlayer *primPlayer = primaryPlayer->connection->getNetworkPlayer();
					if(primPlayer != NULL && checkingPlayer->IsSameSystem( primPlayer ) )
					{
						foundPrimary = true;
						break;
					}
				}
				if(!foundPrimary)
				{
#ifndef _CONTENT_PACKAGE
					app.DebugPrintf("Remove: Adding player %ls as primary in dimension %d\n", newPlayer->name.c_str(), newPlayerDim);
#endif
					receiveAllPlayers[newPlayerDim].push_back( newPlayer );
				}
			}
		}
	}
}

void PlayerList::addPlayerToReceiving(shared_ptr<ServerPlayer> player)
{
	int playerDim = 0;
	if( player->dimension == -1 ) playerDim = 1;
	else if( player->dimension == 1) playerDim = 2;

#ifndef _CONTENT_PACKAGE
	app.DebugPrintf("Requesting add player %ls as primary in dimension %d\n", player->name.c_str(), playerDim);
#endif

	bool shouldAddPlayer = true;

	INetworkPlayer *thisPlayer = player->connection->getNetworkPlayer();

	if( thisPlayer == NULL )
	{
#ifndef _CONTENT_PACKAGE
		app.DebugPrintf("Add: Qnet player for player %ls is NULL so not adding them\n", player->name.c_str() );
#endif
		shouldAddPlayer = false;
	}
	else
	{
		for(auto& oldPlayer : receiveAllPlayers[playerDim])
		{
			INetworkPlayer *checkingPlayer = oldPlayer->connection->getNetworkPlayer();
			if(checkingPlayer != NULL && checkingPlayer->IsSameSystem( thisPlayer ) )
			{
				shouldAddPlayer = false;
				break;
			}
		}
	}

	if( shouldAddPlayer )
	{
#ifndef _CONTENT_PACKAGE
		app.DebugPrintf("Add: Adding player %ls as primary in dimension %d\n", player->name.c_str(), playerDim);
#endif
		receiveAllPlayers[playerDim].push_back( player );
	}
}

bool PlayerList::canReceiveAllPackets(shared_ptr<ServerPlayer> player)
{
	int playerDim = 0;
	if( player->dimension == -1 ) playerDim = 1;
	else if( player->dimension == 1) playerDim = 2;
	for(const auto& newPlayer : receiveAllPlayers[playerDim])
	{
		if(newPlayer == player)
		{
			return true;
		}
	}
	return false;
}

void PlayerList::kickPlayerByShortId(BYTE networkSmallId)
{
	EnterCriticalSection(&m_kickPlayersCS);
	m_smallIdsToKick.push_back(networkSmallId);
	LeaveCriticalSection(&m_kickPlayersCS);
}

void  PlayerList::closePlayerConnectionBySmallId(BYTE networkSmallId)
{
	EnterCriticalSection(&m_closePlayersCS);
	m_smallIdsToClose.push_back(networkSmallId);
	LeaveCriticalSection(&m_closePlayersCS);
}

bool PlayerList::isXuidBanned(PlayerUID xuid)
{
	if( xuid == INVALID_XUID ) return false;

	bool banned = false;

	for(PlayerUID it : m_bannedXuids)
	{
		if( ProfileManager.AreXUIDSEqual( xuid, it ) )
		{
			banned = true;
			break;
		}
	}

	return banned;
}

// AP added for Vita so the range can be increased once the level starts
void PlayerList::setViewDistance(int newViewDistance)
{
	viewDistance = newViewDistance;
}
