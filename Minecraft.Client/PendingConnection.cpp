#include "stdafx.h"
#include "PendingConnection.h"
#include "PlayerConnection.h"
#include "ServerConnection.h"
#include "ServerPlayer.h"
#include "ServerPlayerGameMode.h"
#include "ServerLevel.h"
#include "PlayerList.h"
#include "MinecraftServer.h"
#include "..\Minecraft.World\net.minecraft.network.h"
#include "..\Minecraft.World\pos.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\SharedConstants.h"
#include "Settings.h"
// #ifdef __PS3__
// #include "PS3\Network\NetworkPlayerSony.h"
// #endif

Random *PendingConnection::random = new Random();

#ifdef _WINDOWS64
bool g_bRejectDuplicateNames = true;
#endif

PendingConnection::PendingConnection(MinecraftServer *server, Socket *socket, const wstring& id)
{
	// 4J - added initialisers
	done = false;
	_tick = 0;
	name = L"";
	acceptedLogin = nullptr;
	loginKey = L"";

	this->server = server;
	connection = new Connection(socket, id, this);
	connection->fakeLag = FAKE_LAG;
}

PendingConnection::~PendingConnection()
{
	delete connection;
}

void PendingConnection::tick()
{
	if (acceptedLogin != nullptr)
	{
		this->handleAcceptedLogin(acceptedLogin);
		acceptedLogin = nullptr;
	}
	if (_tick++ == MAX_TICKS_BEFORE_LOGIN)
	{
		disconnect(DisconnectPacket::eDisconnect_LoginTooLong);
	}
	else
	{
		connection->tick();
	}
}

void PendingConnection::disconnect(DisconnectPacket::eDisconnectReason reason)
{
	//   try {	// 4J - removed try/catch
	//        logger.info("Disconnecting " + getName() + ": " + reason);
	app.DebugPrintf("Pending connection disconnect: %d\n", reason );
	connection->send(std::make_shared<DisconnectPacket>(reason));
	connection->sendAndQuit();
	done = true;
	//    } catch (Exception e) {
	//        e.printStackTrace();
	//    }
}

void PendingConnection::handlePreLogin(shared_ptr<PreLoginPacket> packet)
{
	if (packet->m_netcodeVersion != MINECRAFT_NET_VERSION)
	{
		app.DebugPrintf("Netcode version is %d not equal to %d\n", packet->m_netcodeVersion, MINECRAFT_NET_VERSION);
		if (packet->m_netcodeVersion > MINECRAFT_NET_VERSION)
		{
			disconnect(DisconnectPacket::eDisconnect_OutdatedServer);
		}
		else
		{
			disconnect(DisconnectPacket::eDisconnect_OutdatedClient);
		}
		return;
	}
	//	printf("Server: handlePreLogin\n");
	name = packet->loginKey; // 4J Stu - Change from the login packet as we know better on client end during the pre-login packet
	sendPreLoginResponse();
}

void PendingConnection::sendPreLoginResponse()
{
	// 4J Stu - Calculate the players with UGC privileges set
	PlayerUID *ugcXuids = new PlayerUID[MINECRAFT_NET_MAX_PLAYERS];
	DWORD ugcXuidCount = 0;
	DWORD hostIndex = 0;
	BYTE ugcFriendsOnlyBits = 0;
	char szUniqueMapName[14];

	StorageManager.GetSaveUniqueFilename(szUniqueMapName);

	PlayerList *playerList = MinecraftServer::getInstance()->getPlayers();
	for(auto& player : playerList->players)
	{
		// If the offline Xuid is invalid but the online one is not then that's guest which we should ignore
		// If the online Xuid is invalid but the offline one is not then we are definitely an offline game so dont care about UGC

		// PADDY - this is failing when a local player with chat restrictions joins an online game

		if( player != nullptr && player->connection->m_offlineXUID != INVALID_XUID && player->connection->m_onlineXUID != INVALID_XUID )
		{
			if( player->connection->m_friendsOnlyUGC )
			{
				ugcFriendsOnlyBits |= (1<<ugcXuidCount);
			}
			// Need to use the online XUID otherwise friend checks will fail on the client
			ugcXuids[ugcXuidCount] = player->connection->m_onlineXUID;

			if( player->connection->getNetworkPlayer() != nullptr && player->connection->getNetworkPlayer()->IsHost() ) hostIndex = ugcXuidCount;

			++ugcXuidCount;
		}
	}

#if 0
	if (false)//	server->onlineMode) // 4J - removed
	{
		loginKey = L"TOIMPLEMENT"; // 4J - todo Long.toHexString(random.nextLong());
		connection->send( shared_ptr<PreLoginPacket>( new PreLoginPacket(loginKey, ugcXuids, ugcXuidCount, ugcFriendsOnlyBits, server->m_ugcPlayersVersion, szUniqueMapName,app.GetGameHostOption(eGameHostOption_All),hostIndex) ) );
	}
	else
#endif
	{
		DWORD cappedCount = (ugcXuidCount > 255u) ? 255u : ugcXuidCount;
		BYTE cappedHostIndex = (hostIndex >= 255u) ? 254 : static_cast<BYTE>(hostIndex);
		connection->send(std::make_shared<PreLoginPacket>(L"-", ugcXuids, cappedCount, ugcFriendsOnlyBits, server->m_ugcPlayersVersion, szUniqueMapName, app.GetGameHostOption(eGameHostOption_All), cappedHostIndex, server->m_texturePackId));
	}
}

void PendingConnection::handleLogin(shared_ptr<LoginPacket> packet)
{
	//	printf("Server: handleLogin\n");
	//name = packet->userName;
	if (packet->clientVersion != SharedConstants::NETWORK_PROTOCOL_VERSION)
	{
		app.DebugPrintf("Client version is %d not equal to %d\n", packet->clientVersion, SharedConstants::NETWORK_PROTOCOL_VERSION);
		if (packet->clientVersion > SharedConstants::NETWORK_PROTOCOL_VERSION)
		{
			disconnect(DisconnectPacket::eDisconnect_OutdatedServer);
		}
		else
		{
			disconnect(DisconnectPacket::eDisconnect_OutdatedClient);
		}
		return;
	}

	//if (true)// 4J removed !server->onlineMode)
	bool sentDisconnect = false;

	// Use the same Xuid choice as handleAcceptedLogin (offline first, online fallback).
	// 
	PlayerUID loginXuid = packet->m_offlineXuid;
	if (loginXuid == INVALID_XUID) loginXuid = packet->m_onlineXuid;

	bool duplicateXuid = false;
	if (loginXuid != INVALID_XUID && server->getPlayers()->getPlayer(loginXuid) != nullptr)
	{
		duplicateXuid = true;
	}
	else if (packet->m_onlineXuid != INVALID_XUID &&
		packet->m_onlineXuid != loginXuid &&
		server->getPlayers()->getPlayer(packet->m_onlineXuid) != nullptr)
	{
		duplicateXuid = true;
	}

	if( sentDisconnect )
	{
		// Do nothing
	}
	else if( server->getPlayers()->isXuidBanned( packet->m_onlineXuid ) )
	{
		disconnect(DisconnectPacket::eDisconnect_Banned);
	}
	else if (duplicateXuid)
	{
		// Reject the incoming connection — a player with this UID is already
		// on the server.  Allowing duplicates causes invisible players and
		// other undefined behaviour.
		app.DebugPrintf("LOGIN: Rejecting duplicate xuid for name: %ls\n", name.c_str());
		disconnect(DisconnectPacket::eDisconnect_Banned);
	}
#ifdef _WINDOWS64
	else if (g_bRejectDuplicateNames)
	{
		bool nameTaken = false;
		vector<shared_ptr<ServerPlayer> >& pl = server->getPlayers()->players;
		for (const auto& i : pl)
		{
			if (i != nullptr && i->name == name)
			{
				nameTaken = true;
				break;
			}
		}
		if (nameTaken)
		{
			app.DebugPrintf("Rejecting duplicate name: %ls\n", name.c_str());
			disconnect(DisconnectPacket::eDisconnect_Banned);
		}
		else
		{
			handleAcceptedLogin(packet);
		}
	}
#endif
	else
	{
		handleAcceptedLogin(packet);
	}
	//else
	{
		//4J - removed
#if 0
		new Thread() {
			public void run() {
				try {
					String key = loginKey;
					URL url = new URL("http://www.minecraft.net/game/checkserver.jsp?user=" + URLEncoder.encode(packet.userName, "UTF-8") + "&serverId=" + URLEncoder.encode(key, "UTF-8"));
					BufferedReader br = new BufferedReader(new InputStreamReader(url.openStream()));
					String msg = br.readLine();
					br.close();
					if (msg.equals("YES")) {
						acceptedLogin = packet;
					} else {
						disconnect("Failed to verify username!");
					}
				} catch (Exception e) {
					disconnect("Failed to verify username! [internal error " + e + "]");
					e.printStackTrace();
				}
			}
		}.start();
#endif
	}

}

void PendingConnection::handleAcceptedLogin(shared_ptr<LoginPacket> packet)
{
	if(packet->m_ugcPlayersVersion != server->m_ugcPlayersVersion)
	{
		// Send the pre-login packet again with the new list of players
		sendPreLoginResponse();
		return;
	}

	// Guests use the online xuid, everyone else uses the offline one
	PlayerUID playerXuid = packet->m_offlineXuid;
	if(playerXuid == INVALID_XUID) playerXuid = packet->m_onlineXuid;

	shared_ptr<ServerPlayer> playerEntity = server->getPlayers()->getPlayerForLogin(this, name, playerXuid,packet->m_onlineXuid);
	if (playerEntity != nullptr)
	{
		server->getPlayers()->placeNewPlayer(connection, playerEntity, packet);
		connection = nullptr;	// We've moved responsibility for this over to the new PlayerConnection, nullptr so we don't delete our reference to it here in our dtor
	}
	done = true;

}

void PendingConnection::onDisconnect(DisconnectPacket::eDisconnectReason reason, void *reasonObjects)
{
	//    logger.info(getName() + " lost connection");
	done = true;
}

void PendingConnection::handleGetInfo(shared_ptr<GetInfoPacket> packet)
{
	//try {
	//String message = server->motd + "�" + server->players->getPlayerCount() + "�" + server->players->getMaxPlayers();
	//connection->send(new DisconnectPacket(message));
	connection->send(std::make_shared<DisconnectPacket>(DisconnectPacket::eDisconnect_ServerFull));
	connection->sendAndQuit();
	server->connection->removeSpamProtection(connection->getSocket());
	done = true;
	//} catch (Exception e) {
	//	e.printStackTrace();
	//}
}

void PendingConnection::handleKeepAlive(shared_ptr<KeepAlivePacket> packet)
{
	// Ignore
}

void PendingConnection::onUnhandledPacket(shared_ptr<Packet> packet)
{
	disconnect(DisconnectPacket::eDisconnect_UnexpectedPacket);
}

void PendingConnection::send(shared_ptr<Packet> packet)
{
	connection->send(packet);
}

wstring PendingConnection::getName()
{
	return L"Unimplemented";
	//        if (name != null) return name + " [" + connection.getRemoteAddress().toString() + "]";
	//        return connection.getRemoteAddress().toString();
}

bool PendingConnection::isServerPacketListener()
{
	return true;
}

bool PendingConnection::isDisconnected()
{
	return done;
}