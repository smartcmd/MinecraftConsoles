#include "stdafx.h"
#include "FourKitNativeInternal.h"

namespace FourKit
{
	std::map<std::string, ServerPlayer*> g_nativePlayerMap;
}

namespace FourKitInternal
{
	bool g_callbacksRegistered = false;

	void AppendUniqueXuid(PlayerUID xuid, std::vector<PlayerUID>* out)
	{
		if (out == nullptr || xuid == INVALID_XUID)
		{
			return;
		}

		if (std::find(out->begin(), out->end(), xuid) == out->end())
		{
			out->push_back(xuid);
		}
	}

	void CollectPlayerBanXuids(ServerPlayer* player, std::vector<PlayerUID>* out)
	{
		if (player == nullptr || out == nullptr)
		{
			return;
		}

		AppendUniqueXuid(player->getXuid(), out);
		AppendUniqueXuid(player->getOnlineXuid(), out);
	}

	std::string BuildBanReason(const char* reason)
	{
		const std::string trimmed = ServerRuntime::StringUtils::TrimAscii((reason != nullptr) ? reason : "");
		return trimmed.empty() ? "Banned by a plugin." : trimmed;
	}

	bool TryGetPlayerRemoteIp(ServerPlayer* player, std::string* outIp)
	{
		if (outIp == nullptr || player == nullptr || player->connection == nullptr || player->connection->connection == nullptr || player->connection->connection->getSocket() == nullptr)
		{
			return false;
		}

		const unsigned char smallId = player->connection->connection->getSocket()->getSmallId();
		if (smallId == 0)
		{
			return false;
		}

		return ServerRuntime::ServerLogManager::TryGetConnectionRemoteIp(smallId, outIp);
	}

	int DisconnectPlayersByRemoteIp(const std::string& ip)
	{
		auto* server = MinecraftServer::getInstance();
		if (server == nullptr || server->getPlayers() == nullptr)
		{
			return 0;
		}

		const std::string normalizedIp = ServerRuntime::NetworkUtils::NormalizeIpToken(ip);
		const std::vector<std::shared_ptr<ServerPlayer>> playerSnapshot = server->getPlayers()->players;
		int disconnectedCount = 0;
		for (const auto& player : playerSnapshot)
		{
			std::string playerIp;
			if (player == nullptr || !TryGetPlayerRemoteIp(player.get(), &playerIp))
			{
				continue;
			}

			if (ServerRuntime::NetworkUtils::NormalizeIpToken(playerIp) == normalizedIp && player->connection != nullptr)
			{
				player->connection->disconnect(DisconnectPacket::eDisconnect_Banned);
				++disconnectedCount;
			}
		}

		return disconnectedCount;
	}
}

namespace FourKit
{
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
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second && it->second->connection)
		{
            it->second->connection->disconnect(DisconnectPacket::eDisconnect_Kicked);
		}
	}

	bool NativeCallback_BanPlayer(const char* playerName, const char* reason)
	{
		if (playerName == nullptr || !ServerRuntime::Access::IsInitialized())
		{
			return false;
		}

		auto it = g_nativePlayerMap.find(playerName);
		if (it == g_nativePlayerMap.end() || it->second == nullptr)
		{
			return false;
		}

		ServerPlayer* player = it->second;
		std::vector<PlayerUID> xuids;
		FourKitInternal::CollectPlayerBanXuids(player, &xuids);
		if (xuids.empty())
		{
			return false;
		}

		const bool hasUnbannedIdentity = std::any_of(
			xuids.begin(),
			xuids.end(),
			[](PlayerUID xuid) { return !ServerRuntime::Access::IsPlayerBanned(xuid); });
		if (!hasUnbannedIdentity)
		{
			return false;
		}

		ServerRuntime::Access::BanMetadata metadata = ServerRuntime::Access::BanManager::BuildDefaultMetadata("Plugin");
		metadata.reason = FourKitInternal::BuildBanReason(reason);

		const std::string playerNameUtf8 = ServerRuntime::StringUtils::WideToUtf8(player->name);
		for (const auto xuid : xuids)
		{
			if (ServerRuntime::Access::IsPlayerBanned(xuid))
			{
				continue;
			}

			if (!ServerRuntime::Access::AddPlayerBan(xuid, playerNameUtf8, metadata))
			{
				return false;
			}
		}

		if (player->connection != nullptr)
		{
			player->connection->disconnect(DisconnectPacket::eDisconnect_Banned);
		}

		return true;
	}

	bool NativeCallback_BanPlayerIp(const char* playerName, const char* reason)
	{
		if (playerName == nullptr || !ServerRuntime::Access::IsInitialized())
		{
			return false;
		}

		auto it = g_nativePlayerMap.find(playerName);
		if (it == g_nativePlayerMap.end() || it->second == nullptr)
		{
			return false;
		}

		std::string remoteIp;
		if (!FourKitInternal::TryGetPlayerRemoteIp(it->second, &remoteIp))
		{
			return false;
		}

		if (ServerRuntime::Access::IsIpBanned(remoteIp))
		{
			return false;
		}

		ServerRuntime::Access::BanMetadata metadata = ServerRuntime::Access::BanManager::BuildDefaultMetadata("Plugin");
		metadata.reason = FourKitInternal::BuildBanReason(reason);
		if (!ServerRuntime::Access::AddIpBan(remoteIp, metadata))
		{
			return false;
		}

		FourKitInternal::DisconnectPlayersByRemoteIp(remoteIp);
		return true;
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

	int NativeCallback_GetAllowFlight(const char* playerName)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			return it->second->abilities.mayfly ? 1 : 0;
		}
		return 0;
	}

	void NativeCallback_SetAllowFlight(const char* playerName, int flight)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->abilities.mayfly = (flight != 0);
			it->second->onUpdateAbilities();
		}
	}

	float NativeCallback_GetExhaustion(const char* playerName)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			return it->second->getFoodData()->getExhaustionLevel();
		}
		return 0.0f;
	}

	void NativeCallback_SetExhaustion(const char* playerName, float value)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->getFoodData()->setExhaustion(value);
		}
	}

	float NativeCallback_GetSaturation(const char* playerName)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			return it->second->getFoodData()->getSaturationLevel();
		}
		return 0.0f;
	}

	void NativeCallback_SetSaturation(const char* playerName, float value)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->getFoodData()->setSaturation(value);
		}
	}

	void NativeCallback_GiveExp(const char* playerName, int amount)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->increaseXp(amount);
		}
	}

	void NativeCallback_GiveExpLevels(const char* playerName, int amount)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->giveExperienceLevels(amount);
		}
	}

	int NativeCallback_GetTotalExperience(const char* playerName)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			return it->second->totalExperience;
		}
		return 0;
	}

	int NativeCallback_IsFlying(const char* playerName)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			return it->second->abilities.flying ? 1 : 0;
		}
		return 0;
	}

	void NativeCallback_SetFlying(const char* playerName, int value)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->abilities.flying = (value != 0);
			it->second->onUpdateAbilities();
		}
	}

	void NativeCallback_SetExp(const char* playerName, float exp)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->experienceProgress = exp;
			if (it->second->connection)
			{
				it->second->connection->send(std::make_shared<SetExperiencePacket>(
					it->second->experienceProgress,
					it->second->totalExperience,
					it->second->experienceLevel
				));
			}
		}
	}

	void NativeCallback_SetPlayerLevel(const char* playerName, int level)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->experienceLevel = level;
			if (it->second->connection)
			{
				it->second->connection->send(std::make_shared<SetExperiencePacket>(
					it->second->experienceProgress,
					it->second->totalExperience,
					it->second->experienceLevel
				));
			}
		}
	}

	void NativeCallback_SetWalkSpeed(const char* playerName, float value)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			it->second->abilities.setWalkingSpeed(value);
			it->second->onUpdateAbilities();
		}
	}

	float NativeCallback_GetWalkSpeed(const char* playerName)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			return it->second->abilities.getWalkingSpeed();
		}
		return 0.0f;
	}

	int NativeCallback_IsSleepingPlayer(const char* playerName)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			return it->second->isSleeping() ? 1 : 0;
		}
		return 0;
	}

	int NativeCallback_GetGameMode(const char* playerName)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second && it->second->gameMode)
		{
			GameType* gt = it->second->gameMode->getGameModeForPlayer();
			return gt != NULL ? gt->getId() : -1;
		}
		return -1;
	}

	void NativeCallback_SetPlayerGameMode(const char* playerName, int mode)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second)
		{
			GameType* gt = GameType::byId(mode);
			if (gt != NULL)
			{
				it->second->setGameMode(gt);
			}
		}
	}

	void NativeCallback_SetItemInHand(const char* playerName, int itemId, int count, int data)
	{
		auto it = g_nativePlayerMap.find(playerName);
		if (it != g_nativePlayerMap.end() && it->second && it->second->inventory)
		{
			if (itemId <= 0 || count <= 0)
			{
				it->second->inventory->setItem(it->second->inventory->selected, nullptr);
			}
			else
			{
				it->second->inventory->setItem(it->second->inventory->selected,
					shared_ptr<ItemInstance>(new ItemInstance(itemId, count, data)));
			}
		}
	}

	bool NativeCallback_GetItemInHand(const char* playerName, ItemInHandData* outData)
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

		shared_ptr<ItemInstance> item = it->second->getSelectedItem();
		if (item == nullptr)
		{
			outData->hasItem = false;
			outData->itemId = 0;
			outData->count = 0;
			outData->data = 0;
			return true;
		}

		outData->hasItem = true;
		outData->itemId = item->id;
		outData->count = item->count;
		outData->data = item->getAuxValue();
		return true;
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
}

namespace FourKitInternal
{
	void RegisterNativeCallbacks()
	{
		if (g_callbacksRegistered)
		{
			return;
		}

		FourKit_SetNativeCallbacks(
			&FourKit::NativeCallback_SetFallDistance,
			&FourKit::NativeCallback_SetHealth,
			&FourKit::NativeCallback_SetFood,
			&FourKit::NativeCallback_SendMessage,
			&FourKit::NativeCallback_TeleportTo,
			&FourKit::NativeCallback_Kick,
			&FourKit::NativeCallback_BanPlayer,
			&FourKit::NativeCallback_BanPlayerIp,
			&FourKit::NativeCallback_IsSneaking,
			&FourKit::NativeCallback_SetSneaking,
			&FourKit::NativeCallback_IsSprinting,
			&FourKit::NativeCallback_SetSprinting,
			&FourKit::NativeCallback_GetAllowFlight,
			&FourKit::NativeCallback_SetAllowFlight,
			&FourKit::NativeCallback_GetExhaustion,
			&FourKit::NativeCallback_SetExhaustion,
			&FourKit::NativeCallback_GetSaturation,
			&FourKit::NativeCallback_SetSaturation,
			&FourKit::NativeCallback_GiveExp,
			&FourKit::NativeCallback_GiveExpLevels,
			&FourKit::NativeCallback_GetTotalExperience,
			&FourKit::NativeCallback_IsFlying,
			&FourKit::NativeCallback_SetFlying,
			&FourKit::NativeCallback_SetExp,
			&FourKit::NativeCallback_SetPlayerLevel,
			&FourKit::NativeCallback_SetWalkSpeed,
			&FourKit::NativeCallback_GetWalkSpeed,
			&FourKit::NativeCallback_IsSleepingPlayer,
			&FourKit::NativeCallback_GetGameMode,
			&FourKit::NativeCallback_SetPlayerGameMode,
			&FourKit::NativeCallback_SetItemInHand,
			&FourKit::NativeCallback_BlockBreakNaturally,
			&FourKit::NativeCallback_GetBlockType,
			&FourKit::NativeCallback_SetBlockType,
			&FourKit::NativeCallback_GetBlockData,
			&FourKit::NativeCallback_SetBlockData,
			&FourKit::NativeCallback_GetPlayerSnapshot,
			&FourKit::NativeCallback_GetPlayerNetworkAddress,
			&FourKit::NativeCallback_GetItemInHand,
			&FourKit::NativeCallback_GetWorldInfo,
			&FourKit::NativeCallback_CreateExplosion,
			&FourKit::NativeCallback_DropItem,
			&FourKit::NativeCallback_GetHighestBlockYAt,
			&FourKit::NativeCallback_SetFullTime,
			&FourKit::NativeCallback_SetSpawnLocation,
			&FourKit::NativeCallback_SetStorm,
			&FourKit::NativeCallback_SetThunderDuration,
			&FourKit::NativeCallback_SetThundering,
			&FourKit::NativeCallback_SetTime,
			&FourKit::NativeCallback_SetWeatherDuration,
			&FourKit::NativeCallback_StrikeLightning);
		g_callbacksRegistered = true;
	}
}

namespace FourKit
{
	void CreateAndBindManagedPlayer(ServerPlayer* nativePlayer, PlayerConnection* connection)
	{
		if (nativePlayer == nullptr) return;

		try
		{
			FourKitInternal::RegisterNativeCallbacks();

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
