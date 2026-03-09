#pragma once
// 100% better way of doing this. i just kind adumb
// todo: redo this

#include "PluginBridgeStructs.h"

#define PB_PLAYER_CALLBACK_LIST(X) \
	X(SetFallDistance, void, (const char* playerName, float distance)) \
	X(SetHealth, void, (const char* playerName, float health)) \
	X(SetFood, void, (const char* playerName, int food)) \
	X(SendMessage, void, (const char* playerName, const char* message)) \
	X(TeleportTo, void, (const char* playerName, double x, double y, double z)) \
	X(Kick, void, (const char* playerName, const char* reason))

#define PB_BLOCK_CALLBACK_LIST(X) \
	X(BlockBreakNaturally, void, (int x, int y, int z, int dimension)) \
	X(GetBlockType, int, (int x, int y, int z, int dimension)) \
	X(SetBlockType, void, (int x, int y, int z, int dimension, int id)) \
	X(GetBlockData, int, (int x, int y, int z, int dimension)) \
	X(SetBlockData, void, (int x, int y, int z, int dimension, int data))

#define PB_QUERY_CALLBACK_LIST(X) \
	X(GetPlayerSnapshot, bool, (const char* playerName, PlayerJoinData* outData))

#define PB_NATIVE_CALLBACK_LIST(X) \
	PB_PLAYER_CALLBACK_LIST(X) \
	PB_BLOCK_CALLBACK_LIST(X) \
	PB_QUERY_CALLBACK_LIST(X)

extern "C"
{
#define PB_DECLARE_CALLBACK_TYPE(Name, Ret, Sig) typedef Ret(*PluginBridge##Name##Callback) Sig;
	PB_NATIVE_CALLBACK_LIST(PB_DECLARE_CALLBACK_TYPE)
#undef PB_DECLARE_CALLBACK_TYPE

	__declspec(dllexport) void PluginBridge_Initialize();
	__declspec(dllexport) void PluginBridge_Shutdown();
	__declspec(dllexport) void PluginBridge_FireOnPlayerJoin(const PlayerJoinData& playerData);
	__declspec(dllexport) void PluginBridge_FireOnPlayerLeave(const PlayerLeaveData& playerData);
	__declspec(dllexport) void PluginBridge_FireOnChat(const PlayerChatData& chatData, bool* cancelled);
	__declspec(dllexport) void PluginBridge_FireOnBlockBreak(const BlockBreakData& blockBreakData, bool* cancelled);
	__declspec(dllexport) void PluginBridge_FireOnBlockPlace(const BlockPlaceData& blockPlaceData, bool* cancelled);
	__declspec(dllexport) void PluginBridge_FireOnPlayerMove(const PlayerMoveData& moveData, bool* cancelled);
	__declspec(dllexport) void PluginBridge_FireOnLoad();
	__declspec(dllexport) void PluginBridge_FireOnExit();

	__declspec(dllexport) void PluginBridge_SetNativeCallbacks(
		PluginBridgeSetFallDistanceCallback SetFallDistance,
		PluginBridgeSetHealthCallback SetHealth,
		PluginBridgeSetFoodCallback SetFood,
		PluginBridgeSendMessageCallback SendMessage,
		PluginBridgeTeleportToCallback TeleportTo,
		PluginBridgeKickCallback Kick,
		PluginBridgeBlockBreakNaturallyCallback BlockBreakNaturally,
		PluginBridgeGetBlockTypeCallback GetBlockType,
		PluginBridgeSetBlockTypeCallback SetBlockType,
		PluginBridgeGetBlockDataCallback GetBlockData,
		PluginBridgeSetBlockDataCallback SetBlockData,
		PluginBridgeGetPlayerSnapshotCallback GetPlayerSnapshot);

#define PB_DECLARE_NATIVE_EXPORT(Name, Ret, Sig) __declspec(dllexport) Ret NativeCallback_##Name Sig;
	PB_NATIVE_CALLBACK_LIST(PB_DECLARE_NATIVE_EXPORT)
#undef PB_DECLARE_NATIVE_EXPORT
}
