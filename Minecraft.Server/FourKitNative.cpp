#include "stdafx.h"

#include "FourKitNative.h"
#include "FourKitNativeCallbacks.h"
#include "..\Minecraft.Client\ServerLevel.h"
#include "..\Minecraft.Client\ServerPlayer.h"
#include "..\Minecraft.Client\PlayerConnection.h"
#include "..\Minecraft.Client\MinecraftServer.h"
#include "..\Minecraft.Client\PlayerList.h"
#include "..\Minecraft.World\Connection.h"
#include "..\Minecraft.World\Socket.h"
#include "..\Minecraft.Server.FourKit\FourKitStructs.h"
#include "..\Minecraft.Server.FourKit\FourKitInterop.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "../Minecraft.World/Dimension.h"
#include "../Minecraft.World/TileUpdatePacket.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "../Minecraft.Server/Common/StringUtils.h"

#if defined(_WINDOWS64)
#include "..\Minecraft.Client\Windows64\Network\WinsockNetLayer.h"
#endif

namespace FourKit
{
	std::map<std::string, ServerPlayer*> g_nativePlayerMap;
	static bool g_callbacksRegistered = false;

	static void BroadcastBlockUpdate(ServerLevel* level, int x, int y, int z)
	{
		if (level == NULL)
		{
			return;
		}

		MinecraftServer* server = MinecraftServer::getInstance();
		if (server == NULL)
		{
			return;
		}

		server->getPlayers()->prioritiseTileChanges(x, y, z, level->dimension->id);

		PlayerList* playerList = server->getPlayerList();
		if (playerList == NULL)
		{
			return;
		}

		for (size_t i = 0; i < playerList->players.size(); ++i)
		{
			if (playerList->players[i] && playerList->players[i]->connection)
			{
				playerList->players[i]->connection->send(shared_ptr<TileUpdatePacket>(new TileUpdatePacket(x, y, z, level)));
			}
		}
	}

	void RegisterNativePlayer(const char* playerName, ServerPlayer* player)
	{
		if (playerName && player)
		{
			g_nativePlayerMap[playerName] = player;
		}
	}
	
	void UnregisterNativePlayer(const char* playerName)
	{
		if (playerName)
		{
			g_nativePlayerMap.erase(playerName);
		}
	}
	
	void NativeCallback_SetFallDistance(const char* playerName, float distance)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->fallDistance = distance;
		}
	}
	
	void NativeCallback_SetHealth(const char* playerName, float health)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->setHealth(health);
		}
	}
	
	void NativeCallback_SetFood(const char* playerName, int food)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->getFoodData()->setFoodLevel(food);
		}
	}

	void NativeCallback_SendMessage(const char* playerName, const char* message)
	{
		if (playerName == nullptr || message == nullptr)
		{
			return;
		}

		PlayerList *playerList = MinecraftServer::getPlayerList();
		if (playerList == NULL)
		{
			return;
		}

		std::wstring widePlayerName = ServerRuntime::StringUtils::Utf8ToWide(playerName);
		std::wstring wideMessage = ServerRuntime::StringUtils::Utf8ToWide(message);
		playerList->sendMessage(widePlayerName, wideMessage);
	}
	
	void NativeCallback_TeleportTo(const char* playerName, double x, double y, double z)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second && it->second->connection)
		{
			it->second->connection->teleport(x, y, z, it->second->yRot, it->second->xRot);
		}
	}
	
	void NativeCallback_Kick(const char* playerName, const char* reason)
	{
		// todo: actually handle reason
		// would require messing with the actual disconnect packet

		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second && it->second->connection)
		{
            it->second->connection->disconnect(DisconnectPacket::eDisconnect_Kicked);
		}
	}

	int NativeCallback_IsSneaking(const char* playerName)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			return it->second->isSneaking() ? 1 : 0;
		}
		return 0;
	}

	void NativeCallback_SetSneaking(const char* playerName, int sneaking)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->setSneaking(sneaking != 0);
		}
	}

	int NativeCallback_IsSprinting(const char* playerName)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			return it->second->isSprinting() ? 1 : 0;
		}
		return 0;
	}

	void NativeCallback_SetSprinting(const char* playerName, int sprinting)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->setSprinting(sprinting != 0);
		}
	}

	void NativeCallback_BlockBreakNaturally(int x, int y, int z, int dimension)
	{
		ServerLevel* level = MinecraftServer::getInstance()->getLevel(dimension);
		if (level != NULL)
		{
			level->destroyTile(x, y, z, true);
		}
	}

	int NativeCallback_GetBlockType(int x, int y, int z, int dimension)
	{
		ServerLevel* level = MinecraftServer::getInstance()->getLevel(dimension);
		if (level != NULL)
		{
			return level->getTile(x, y, z);
		}
		return 0;
	}

	void NativeCallback_SetBlockType(int x, int y, int z, int dimension, int id)
	{
		ServerLevel* level = MinecraftServer::getInstance()->getLevel(dimension);
		if (level != NULL)
		{
			int data = level->getData(x, y, z);
			bool result = level->setTileAndData(x, y, z, id, data, 3);
			if (result)
			{
				BroadcastBlockUpdate(level, x, y, z);
			}
		}
	}

	int NativeCallback_GetBlockData(int x, int y, int z, int dimension)
	{
		ServerLevel* level = MinecraftServer::getInstance()->getLevel(dimension);
		if (level != NULL)
		{
			return level->getData(x, y, z);
		}
		return 0;
	}

	void NativeCallback_SetBlockData(int x, int y, int z, int dimension, int data)
	{
		ServerLevel* level = MinecraftServer::getInstance()->getLevel(dimension);
		if (level != NULL)
		{
			bool result = level->setData(x, y, z, data, 3);
			if (result)
			{
				BroadcastBlockUpdate(level, x, y, z);
			}
		}
	}

	bool NativeCallback_GetPlayerSnapshot(const char* playerName, PlayerJoinData* outData)
	{
		if (playerName == nullptr || outData == nullptr)
		{
			return false;
		}

		auto it = g_nativePlayerMap.find(playerName);
		if (it == g_nativePlayerMap.end() || it->second == nullptr)
		{
			return false;
		}

		ServerPlayer* p = it->second;
		outData->playerName = playerName;
		outData->health = p->getHealth();
		outData->food = p->getFoodData()->getFoodLevel();
		outData->fallDistance = p->fallDistance;
		outData->yRot = p->yRot;
		outData->xRot = p->xRot;
		outData->sneaking = p->isSneaking();
		outData->sprinting = p->isSprinting();
		outData->x = p->x;
		outData->y = p->y;
		outData->z = p->z;
		outData->dimension = p->dimension;
		return true;
	}

	bool NativeCallback_GetPlayerNetworkAddress(const char* playerName, PlayerNetworkAddressData* outData)
	{
		if (playerName == nullptr || outData == nullptr)
		{
			return false;
		}

		auto it = g_nativePlayerMap.find(playerName);
		if (it == g_nativePlayerMap.end() || it->second == nullptr || it->second->connection == nullptr || it->second->connection->connection == nullptr)
		{
			return false;
		}

		Socket* socket = it->second->connection->connection->getSocket();
		if (socket == NULL)
		{
			return false;
		}

		BYTE smallId = socket->getSmallId();
		char ip[64] = {};
		int port = 0;
		if (!WinsockNetLayer::GetRemoteEndpointForSmallId(smallId, ip, (int)sizeof(ip), &port))
		{
			return false;
		}

		strcpy_s(outData->hostAddress, sizeof(outData->hostAddress), ip);
		strcpy_s(outData->hostString, sizeof(outData->hostString), ip);
		strcpy_s(outData->hostName, sizeof(outData->hostName), ip);
		outData->port = port;
		outData->unresolved = false;
		return true;
	}

	bool EmitPlayerChatEvent(ServerPlayer* nativePlayer, const std::wstring& message)
	{
		if (nativePlayer == nullptr)
		{
			return false;
		}

		PlayerChatData chatData;
		std::string nameUtf8 = ServerRuntime::StringUtils::WideToUtf8(nativePlayer->name);
		std::string messageUtf8 = ServerRuntime::StringUtils::WideToUtf8(message);
		chatData.playerName = nameUtf8.c_str();
		chatData.message = messageUtf8.c_str();

		bool cancelled = false;
		FourKit_FireOnChat(chatData, &cancelled);
		return cancelled;
	}

	bool EmitBlockBreakEvent(ServerPlayer* nativePlayer, int x, int y, int z, int blockId, int blockData)
	{
		if (nativePlayer == nullptr)
		{
			return false;
		}

		BlockBreakData breakData;
		std::string nameUtf8 = ServerRuntime::StringUtils::WideToUtf8(nativePlayer->name);
		breakData.playerName = nameUtf8.c_str();
		breakData.x = x;
		breakData.y = y;
		breakData.z = z;
		breakData.blockId = blockId;
		breakData.blockData = blockData;

		bool cancelled = false;
		FourKit_FireOnBlockBreak(breakData, &cancelled);
		return cancelled;
	}

	bool EmitBlockPlaceEvent(ServerPlayer* nativePlayer, int x, int y, int z, int blockId, int blockData)
	{
		if (nativePlayer == nullptr)
		{
			return false;
		}

		BlockPlaceData placeData;
		std::string nameUtf8 = ServerRuntime::StringUtils::WideToUtf8(nativePlayer->name);
		placeData.playerName = nameUtf8.c_str();
		placeData.x = x;
		placeData.y = y;
		placeData.z = z;
		placeData.blockId = blockId;
		placeData.blockData = blockData;

		bool cancelled = false;
		FourKit_FireOnBlockPlace(placeData, &cancelled);
		return cancelled;
	}

	bool EmitPlayerMoveEvent(ServerPlayer* nativePlayer, double fromX, double fromY, double fromZ, double toX, double toY, double toZ)
	{
		if (nativePlayer == nullptr)
		{
			return false;
		}

		PlayerMoveData moveData;
		std::string nameUtf8 = ServerRuntime::StringUtils::WideToUtf8(nativePlayer->name);
		moveData.playerName = nameUtf8.c_str();
		moveData.fromX = fromX;
		moveData.fromY = fromY;
		moveData.fromZ = fromZ;
		moveData.toX = toX;
		moveData.toY = toY;
		moveData.toZ = toZ;

		bool cancelled = false;
		FourKit_FireOnPlayerMove(moveData, &cancelled);
		return cancelled;
	}

	bool EmitPlayerPortalEvent(ServerPlayer* nativePlayer, EPortalTeleportCause cause,
		double& fromX, double& fromY, double& fromZ,
		double& toX, double& toY, double& toZ)
	{
		if (nativePlayer == nullptr)
		{
			return false;
		}

		PlayerPortalData portalData;
		std::string nameUtf8 = ServerRuntime::StringUtils::WideToUtf8(nativePlayer->name);
		portalData.playerName = nameUtf8.c_str();
		portalData.cause = (int)cause;
		portalData.fromX = fromX;
		portalData.fromY = fromY;
		portalData.fromZ = fromZ;
		portalData.toX = toX;
		portalData.toY = toY;
		portalData.toZ = toZ;

		bool cancelled = false;
		FourKit_FireOnPlayerPortal(&portalData, &cancelled);

		fromX = portalData.fromX;
		fromY = portalData.fromY;
		fromZ = portalData.fromZ;
		toX = portalData.toX;
		toY = portalData.toY;
		toZ = portalData.toZ;

		return cancelled;
	}

	bool EmitSignChangeEvent(ServerPlayer* nativePlayer, int x, int y, int z, int dimension, std::wstring lines[4])
	{
		if (nativePlayer == nullptr)
		{
			return false;
		}

		SignChangeData signData;
		std::string nameUtf8 = ServerRuntime::StringUtils::WideToUtf8(nativePlayer->name);
		signData.playerName = nameUtf8.c_str();
		signData.x = x;
		signData.y = y;
		signData.z = z;
		signData.dimension = dimension;

		for (int i = 0; i < 4; ++i)
		{
			std::string lineUtf8 = ServerRuntime::StringUtils::WideToUtf8(lines[i]);
			strncpy_s(signData.lines[i], sizeof(signData.lines[i]), lineUtf8.c_str(), _TRUNCATE);
		}

		bool cancelled = false;
		FourKit_FireOnSignChange(&signData, &cancelled);

		for (int i = 0; i < 4; ++i)
		{
			lines[i] = ServerRuntime::StringUtils::Utf8ToWide(signData.lines[i]);
		}

		return cancelled;
	}

	bool EmitPlayerInteractEvent(ServerPlayer* nativePlayer, EInteractAction action, int blockFace,
		bool hasBlock, int x, int y, int z, int dimension, int blockId, int blockData, bool hasItem)
	{
		if (nativePlayer == nullptr)
		{
			return false;
		}

		PlayerInteractData interactData;
		std::string nameUtf8 = ServerRuntime::StringUtils::WideToUtf8(nativePlayer->name);
		interactData.playerName = nameUtf8.c_str();
		interactData.action = (int)action;
		interactData.blockFace = blockFace;
		interactData.hasBlock = hasBlock;
        interactData.hasItem = hasItem;
		interactData.x = x;
		interactData.y = y;
		interactData.z = z;
		interactData.dimension = dimension;
		interactData.blockId = blockId;
		interactData.blockData = blockData;

		bool cancelled = false;
		FourKit_FireOnPlayerInteract(&interactData, &cancelled);

		return cancelled;
	}

	void EmitPlayerLeaveEvent(ServerPlayer* nativePlayer)
	{
		if (nativePlayer == nullptr)
		{
			return;
		}

		PlayerLeaveData leaveData;
		std::string nameUtf8 = ServerRuntime::StringUtils::WideToUtf8(nativePlayer->name);
		leaveData.playerName = nameUtf8.c_str();

		FourKit_FireOnPlayerLeave(leaveData);
		
		UnregisterNativePlayer(nameUtf8.c_str());
	}

	bool DispatchPlayerCommand(ServerPlayer* nativePlayer, const std::wstring& commandLine)
	{
		if (nativePlayer == nullptr)
		{
			return false;
		}

		std::string nameUtf8 = ServerRuntime::StringUtils::WideToUtf8(nativePlayer->name);
		std::string commandUtf8 = ServerRuntime::StringUtils::WideToUtf8(commandLine);
		return FourKit_DispatchPlayerCommand(nameUtf8.c_str(), commandUtf8.c_str()) != 0;
	}

	void EmitPlayerDeathEvent(ServerPlayer* nativePlayer, PlayerDeathData* deathData)
	{
		if (nativePlayer == nullptr || deathData == nullptr)
		{
			return;
		}

		FourKit_FireOnPlayerDeath(deathData);
	}

	void CreateAndBindManagedPlayer(ServerPlayer* nativePlayer, PlayerConnection* connection)
	{
		if (nativePlayer == nullptr) return;

		try
		{
			if (!g_callbacksRegistered)
			{
				FourKit_SetNativeCallbacks(
					&NativeCallback_SetFallDistance,
					&NativeCallback_SetHealth,
					&NativeCallback_SetFood,
					&NativeCallback_SendMessage,
					&NativeCallback_TeleportTo,
					&NativeCallback_Kick,
					&NativeCallback_IsSneaking,
					&NativeCallback_SetSneaking,
					&NativeCallback_IsSprinting,
					&NativeCallback_SetSprinting,
					&NativeCallback_BlockBreakNaturally,
					&NativeCallback_GetBlockType,
					&NativeCallback_SetBlockType,
					&NativeCallback_GetBlockData,
					&NativeCallback_SetBlockData,
					&NativeCallback_GetPlayerSnapshot,
					&NativeCallback_GetPlayerNetworkAddress);
				g_callbacksRegistered = true;
			}

			PlayerJoinData playerData;
			
			std::string nameUtf8 = ServerRuntime::StringUtils::WideToUtf8(nativePlayer->name);
            playerData.playerName = nameUtf8.c_str();
			
			RegisterNativePlayer(playerData.playerName, nativePlayer);
			
			playerData.health = nativePlayer->getHealth();
			playerData.food = nativePlayer->getFoodData()->getFoodLevel();
			playerData.fallDistance = nativePlayer->fallDistance;
			playerData.yRot = nativePlayer->yRot;
			playerData.xRot = nativePlayer->xRot;
			playerData.sneaking = nativePlayer->isSneaking();
			playerData.sprinting = nativePlayer->isSprinting();
			
			playerData.x = nativePlayer->x;
			playerData.y = nativePlayer->y;
			playerData.z = nativePlayer->z;
			playerData.dimension = nativePlayer->dimension;
			
			FourKit_FireOnPlayerJoin(playerData);
		}
		catch (...)
		{
			app.DebugPrintf("[FourKit] Error creating managed player binding\n");
		}
	}
}