#pragma once

#if defined(_WINDOWS64) && defined(MINECRAFT_SERVER_BUILD)

#include <string>

class ServerPlayer;
class PlayerConnection;

namespace FourKit
{
	enum EPortalTeleportCause
	{
		ePortalCause_EndPortal = 0,
		ePortalCause_EnderPearl = 1,
		ePortalCause_NetherPortal = 2,
		ePortalCause_Plugin = 3,
		ePortalCause_Unknown = 4
	};

	enum EInteractAction
	{
		eInteract_RightClickBlock = 0,
		//eInteract_LeftClickAir = 1,
		eInteract_LeftClickBlock = 2,
		//eInteract_RightClickAir = 3,
		eInteract_Physical = 4
	};

	void CreateAndBindManagedPlayer(ServerPlayer* nativePlayer, PlayerConnection* connection);
	bool EmitPlayerChatEvent(ServerPlayer* nativePlayer, const std::wstring& message);
	bool EmitBlockBreakEvent(ServerPlayer* nativePlayer, int x, int y, int z, int blockId, int blockData);
	bool EmitBlockPlaceEvent(ServerPlayer* nativePlayer, int x, int y, int z, int blockId, int blockData);
	bool EmitPlayerMoveEvent(ServerPlayer* nativePlayer, double fromX, double fromY, double fromZ, double toX, double toY, double toZ);
	bool EmitPlayerPortalEvent(ServerPlayer* nativePlayer, EPortalTeleportCause cause,
		double& fromX, double& fromY, double& fromZ,
		double& toX, double& toY, double& toZ);
	bool EmitSignChangeEvent(ServerPlayer* nativePlayer, int x, int y, int z, int dimension, std::wstring lines[4]);
	bool EmitPlayerInteractEvent(ServerPlayer* nativePlayer, EInteractAction action, int blockFace,
		bool hasBlock, int x, int y, int z, int dimension, int blockId, int blockData, bool hasItem);
	void EmitPlayerLeaveEvent(ServerPlayer* nativePlayer);
}

#endif
