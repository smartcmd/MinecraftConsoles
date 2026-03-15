#pragma once

// header hell
#include "FourKitNative.h"
#include "FourKitNativeCallbacks.h"
#include "Access\Access.h"
#include "Common\NetworkUtils.h"
#include "ServerLogManager.h"
#include "..\Minecraft.Client\ServerLevel.h"
#include "..\Minecraft.Client\ServerPlayer.h"
#include "..\Minecraft.Client\PlayerConnection.h"
#include "..\Minecraft.Client\MinecraftServer.h"
#include "..\Minecraft.Client\PlayerList.h"
#include "..\Minecraft.World\Connection.h"
#include "..\Minecraft.World\Socket.h"
#include "..\Minecraft.World\DisconnectPacket.h"
#include "..\Minecraft.Server.FourKit\FourKitStructs.h"
#include "..\Minecraft.Server.FourKit\FourKitInterop.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "../Minecraft.World/Dimension.h"
#include "../Minecraft.World/TileUpdatePacket.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "../Minecraft.Server/Common/StringUtils.h"
#include "..\Minecraft.World\LevelData.h"
#include "..\Minecraft.World\ItemEntity.h"
#include "..\Minecraft.World\ItemInstance.h"
#include "..\Minecraft.World\LightningBolt.h"
#include "..\Minecraft.World\AddGlobalEntityPacket.h"
#include "..\Minecraft.World\SetExperiencePacket.h"
#include "..\Minecraft.Client\ServerPlayerGameMode.h"
#include "..\Minecraft.World\LevelSettings.h"
#include "..\Minecraft.Client\Windows64\Network\WinsockNetLayer.h"
#include <algorithm>

namespace FourKit
{
	extern std::map<std::string, ServerPlayer*> g_nativePlayerMap;
}

namespace FourKitInternal
{
	extern bool g_callbacksRegistered;

	void AppendUniqueXuid(PlayerUID xuid, std::vector<PlayerUID>* out);
	void CollectPlayerBanXuids(ServerPlayer* player, std::vector<PlayerUID>* out);
	std::string BuildBanReason(const char* reason);
	bool TryGetPlayerRemoteIp(ServerPlayer* player, std::string* outIp);
	int DisconnectPlayersByRemoteIp(const std::string& ip);
	ServerLevel* GetServerLevel(int dimension);
	LevelData* GetLevelData(ServerLevel* level);
	void BroadcastBlockUpdate(ServerLevel* level, int x, int y, int z);
	void RegisterNativeCallbacks();
}
