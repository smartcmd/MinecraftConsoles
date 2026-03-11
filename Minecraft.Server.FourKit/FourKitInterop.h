#pragma once
// 100% better way of doing this. i just kind adumb
// todo: redo this

#include "FourKitStructs.h"

#define PB_PLAYER_CALLBACK_LIST(X) \
	X(SetFallDistance, void, (const char* playerName, float distance)) \
	X(SetHealth, void, (const char* playerName, float health)) \
	X(SetFood, void, (const char* playerName, int food)) \
	X(SendMessage, void, (const char* playerName, const char* message)) \
	X(TeleportTo, void, (const char* playerName, double x, double y, double z)) \
	X(Kick, void, (const char* playerName, const char* reason)) \
	X(IsSneaking, int, (const char* playerName)) \
	X(SetSneaking, void, (const char* playerName, int sneaking)) \
	X(IsSprinting, int, (const char* playerName)) \
	X(SetSprinting, void, (const char* playerName, int sprinting))

#define PB_BLOCK_CALLBACK_LIST(X) \
	X(BlockBreakNaturally, void, (int x, int y, int z, int dimension)) \
	X(GetBlockType, int, (int x, int y, int z, int dimension)) \
	X(SetBlockType, void, (int x, int y, int z, int dimension, int id)) \
	X(GetBlockData, int, (int x, int y, int z, int dimension)) \
	X(SetBlockData, void, (int x, int y, int z, int dimension, int data))

#define PB_QUERY_CALLBACK_LIST(X) \
	X(GetPlayerSnapshot, bool, (const char* playerName, PlayerJoinData* outData)) \
	X(GetPlayerNetworkAddress, bool, (const char* playerName, PlayerNetworkAddressData* outData))

#define PB_WORLD_CALLBACK_LIST(X) \
	X(GetWorldInfo, bool, (int dimension, WorldInfoData* outData)) \
	X(CreateExplosion, int, (int dimension, double x, double y, double z, float power, int setFire, int breakBlocks)) \
	X(DropItem, bool, (int dimension, double x, double y, double z, int itemId, int count, int data, int naturalOffset, DroppedItemData* outData)) \
	X(GetHighestBlockYAt, int, (int dimension, int x, int z)) \
	X(SetFullTime, void, (int dimension, long long time)) \
	X(SetSpawnLocation, int, (int dimension, int x, int y, int z)) \
	X(SetStorm, void, (int dimension, int hasStorm)) \
	X(SetThunderDuration, void, (int dimension, int duration)) \
	X(SetThundering, void, (int dimension, int thundering)) \
	X(SetTime, void, (int dimension, long long time)) \
	X(SetWeatherDuration, void, (int dimension, int duration)) \
	X(StrikeLightning, int, (int dimension, double x, double y, double z, int effectOnly))

#define PB_NATIVE_CALLBACK_LIST(X) \
	PB_PLAYER_CALLBACK_LIST(X) \
	PB_BLOCK_CALLBACK_LIST(X) \
	PB_QUERY_CALLBACK_LIST(X) \
	PB_WORLD_CALLBACK_LIST(X)

#define PB_SET_NATIVE_CALLBACK_PARAMS \
	FourKitSetFallDistanceCallback SetFallDistance, \
	FourKitSetHealthCallback SetHealth, \
	FourKitSetFoodCallback SetFood, \
	FourKitSendMessageCallback SendMessage, \
	FourKitTeleportToCallback TeleportTo, \
	FourKitKickCallback Kick, \
	FourKitIsSneakingCallback IsSneaking, \
	FourKitSetSneakingCallback SetSneaking, \
	FourKitIsSprintingCallback IsSprinting, \
	FourKitSetSprintingCallback SetSprinting, \
	FourKitBlockBreakNaturallyCallback BlockBreakNaturally, \
	FourKitGetBlockTypeCallback GetBlockType, \
	FourKitSetBlockTypeCallback SetBlockType, \
	FourKitGetBlockDataCallback GetBlockData, \
	FourKitSetBlockDataCallback SetBlockData, \
	FourKitGetPlayerSnapshotCallback GetPlayerSnapshot, \
	FourKitGetPlayerNetworkAddressCallback GetPlayerNetworkAddress, \
	FourKitGetWorldInfoCallback GetWorldInfo, \
	FourKitCreateExplosionCallback CreateExplosion, \
	FourKitDropItemCallback DropItem, \
	FourKitGetHighestBlockYAtCallback GetHighestBlockYAt, \
	FourKitSetFullTimeCallback SetFullTime, \
	FourKitSetSpawnLocationCallback SetSpawnLocation, \
	FourKitSetStormCallback SetStorm, \
	FourKitSetThunderDurationCallback SetThunderDuration, \
	FourKitSetThunderingCallback SetThundering, \
	FourKitSetTimeCallback SetTime, \
	FourKitSetWeatherDurationCallback SetWeatherDuration, \
	FourKitStrikeLightningCallback StrikeLightning

#define PB_SET_NATIVE_CALLBACK_ARGS \
	SetFallDistance, \
	SetHealth, \
	SetFood, \
	SendMessage, \
	TeleportTo, \
	Kick, \
	IsSneaking, \
	SetSneaking, \
	IsSprinting, \
	SetSprinting, \
	BlockBreakNaturally, \
	GetBlockType, \
	SetBlockType, \
	GetBlockData, \
	SetBlockData, \
	GetPlayerSnapshot, \
	GetPlayerNetworkAddress, \
	GetWorldInfo, \
	CreateExplosion, \
	DropItem, \
	GetHighestBlockYAt, \
	SetFullTime, \
	SetSpawnLocation, \
	SetStorm, \
	SetThunderDuration, \
	SetThundering, \
	SetTime, \
	SetWeatherDuration, \
	StrikeLightning

extern "C"
{
#define PB_DECLARE_CALLBACK_TYPE(Name, Ret, Sig) typedef Ret(*FourKit##Name##Callback) Sig;
	PB_NATIVE_CALLBACK_LIST(PB_DECLARE_CALLBACK_TYPE)
#undef PB_DECLARE_CALLBACK_TYPE

	__declspec(dllexport) void FourKit_Initialize();
	__declspec(dllexport) void FourKit_Shutdown();
	__declspec(dllexport) void FourKit_FireOnPlayerJoin(const PlayerJoinData& playerData);
	__declspec(dllexport) void FourKit_FireOnPlayerLeave(const PlayerLeaveData& playerData);
	__declspec(dllexport) void FourKit_FireOnChat(const PlayerChatData& chatData, bool* cancelled);
	__declspec(dllexport) void FourKit_FireOnBlockBreak(const BlockBreakData& blockBreakData, bool* cancelled);
	__declspec(dllexport) void FourKit_FireOnBlockPlace(const BlockPlaceData& blockPlaceData, bool* cancelled);
	__declspec(dllexport) void FourKit_FireOnPlayerMove(const PlayerMoveData& moveData, bool* cancelled);
	__declspec(dllexport) void FourKit_FireOnPlayerPortal(PlayerPortalData* portalData, bool* cancelled);
	__declspec(dllexport) void FourKit_FireOnSignChange(SignChangeData* signData, bool* cancelled);
	__declspec(dllexport) void FourKit_FireOnPlayerInteract(PlayerInteractData* interactData, bool* cancelled);
	__declspec(dllexport) int FourKit_DispatchPlayerCommand(const char* playerName, const char* commandLine);
	__declspec(dllexport) void FourKit_FireOnLoad();
	__declspec(dllexport) void FourKit_FireOnExit();
	__declspec(dllexport) void FourKit_FireOnPlayerDeath(PlayerDeathData* deathData);

	__declspec(dllexport) void FourKit_SetNativeCallbacks(PB_SET_NATIVE_CALLBACK_PARAMS);

#define PB_DECLARE_NATIVE_EXPORT(Name, Ret, Sig) __declspec(dllexport) Ret NativeCallback_##Name Sig;
	PB_NATIVE_CALLBACK_LIST(PB_DECLARE_NATIVE_EXPORT)
#undef PB_DECLARE_NATIVE_EXPORT
}
