#include "stdafx.h"
#include "FourKitNativeInternal.h"

namespace FourKit
{
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

	bool EmitPlayerDropItemEvent(ServerPlayer* nativePlayer, int itemId, int itemCount, int itemData,
		int& outItemId, int& outItemCount, int& outItemData, int& outPickupDelay)
	{
		if (nativePlayer == nullptr)
		{
			return false;
		}

		PlayerDropItemData dropData;
		std::string nameUtf8 = ServerRuntime::StringUtils::WideToUtf8(nativePlayer->name);
		dropData.playerName = nameUtf8.c_str();
		dropData.itemId = itemId;
		dropData.itemCount = itemCount;
		dropData.itemData = itemData;

		bool cancelled = false;
		FourKit_FireOnPlayerDropItem(&dropData, &cancelled);

		outItemId = dropData.itemId;
		outItemCount = dropData.itemCount;
		outItemData = dropData.itemData;
		outPickupDelay = dropData.pickupDelay;

		return cancelled;
	}
}
