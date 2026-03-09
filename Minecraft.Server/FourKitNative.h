#pragma once

#if defined(_WINDOWS64) && defined(MINECRAFT_SERVER_BUILD)

#include <string>

class ServerPlayer;
class PlayerConnection;

namespace FourKit
{
	void CreateAndBindManagedPlayer(ServerPlayer* nativePlayer, PlayerConnection* connection);
	bool EmitPlayerChatEvent(ServerPlayer* nativePlayer, const std::wstring& message);
	bool EmitBlockBreakEvent(ServerPlayer* nativePlayer, int x, int y, int z, int blockId, int blockData);
	bool EmitBlockPlaceEvent(ServerPlayer* nativePlayer, int x, int y, int z, int blockId, int blockData);
	bool EmitPlayerMoveEvent(ServerPlayer* nativePlayer, double fromX, double fromY, double fromZ, double toX, double toY, double toZ);
	void EmitPlayerLeaveEvent(ServerPlayer* nativePlayer);
}

#endif
