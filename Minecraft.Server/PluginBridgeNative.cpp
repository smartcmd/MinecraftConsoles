#include "stdafx.h"

#include "PluginBridgeNative.h"
#include "PluginBridgeNativeCallbacks.h"
#include "..\Minecraft.Client\ServerLevel.h"
#include "..\Minecraft.Client\ServerPlayer.h"
#include "..\Minecraft.Client\PlayerConnection.h"
#include "..\Minecraft.Client\MinecraftServer.h"
#include "..\Minecraft.Client\PlayerList.h"
#include "..\Minecraft.Server.PluginBridge\PluginBridgeStructs.h"
#include "..\Minecraft.Server.PluginBridge\PluginBridgeInterop.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "../Minecraft.World/Dimension.h"
#include "../Minecraft.World/TileUpdatePacket.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "../Minecraft.Server/Common/StringUtils.h"

namespace PluginBridge
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
		outData->x = p->x;
		outData->y = p->y;
		outData->z = p->z;
		outData->dimension = p->dimension;
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
		PluginBridge_FireOnChat(chatData, &cancelled);
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
		PluginBridge_FireOnBlockBreak(breakData, &cancelled);
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
		PluginBridge_FireOnBlockPlace(placeData, &cancelled);
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
		PluginBridge_FireOnPlayerMove(moveData, &cancelled);
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

		PluginBridge_FireOnPlayerLeave(leaveData);
		
		UnregisterNativePlayer(nameUtf8.c_str());
	}

	void CreateAndBindManagedPlayer(ServerPlayer* nativePlayer, PlayerConnection* connection)
	{
		if (nativePlayer == nullptr) return;

		try
		{
			if (!g_callbacksRegistered)
			{
				PluginBridge_SetNativeCallbacks(
					&NativeCallback_SetFallDistance,
					&NativeCallback_SetHealth,
					&NativeCallback_SetFood,
					&NativeCallback_SendMessage,
					&NativeCallback_TeleportTo,
					&NativeCallback_Kick,
					&NativeCallback_BlockBreakNaturally,
					&NativeCallback_GetBlockType,
					&NativeCallback_SetBlockType,
					&NativeCallback_GetBlockData,
					&NativeCallback_SetBlockData,
					&NativeCallback_GetPlayerSnapshot);
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
			
			playerData.x = nativePlayer->x;
			playerData.y = nativePlayer->y;
			playerData.z = nativePlayer->z;
			playerData.dimension = nativePlayer->dimension;
			
			PluginBridge_FireOnPlayerJoin(playerData);
		}
		catch (...)
		{
			app.DebugPrintf("[PluginBridge] Error creating managed player binding\n");
		}
	}
}