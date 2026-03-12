#include "FourKitInternal.h"

namespace FourKitInternal
{
#define PB_DECLARE_STORAGE(Name, Ret, Sig) FourKit##Name##Callback g_##Name = nullptr;
	PB_NATIVE_CALLBACK_LIST(PB_DECLARE_STORAGE)
#undef PB_DECLARE_STORAGE

	BlockFace ToBlockFace(int face)
	{
		switch (face)
		{
		case 0: return BlockFace::DOWN;
		case 1: return BlockFace::UP;
		case 2: return BlockFace::NORTH;
		case 3: return BlockFace::SOUTH;
		case 4: return BlockFace::WEST;
		case 5: return BlockFace::EAST;
		default: return BlockFace::SELF;
		}
	}

	InteractAction ToInteractAction(int action)
	{
		switch (action)
		{
		case 0: return InteractAction::RIGHT_CLICK_BLOCK;
		case 2: return InteractAction::LEFT_CLICK_BLOCK;
		case 4: return InteractAction::PHYSICAL;
		default: return InteractAction::RIGHT_CLICK_BLOCK;
		}
	}

	Player^ ResolvePlayerByName(String^ name)
	{
		if (String::IsNullOrEmpty(name))
		{
			return nullptr;
		}

		Player^ player = gcnew Player(name);
		IntPtr namePtr = Marshal::StringToHGlobalAnsi(name);
		try
		{
			PlayerJoinData snapshot;
			if (NativeCallback_GetPlayerSnapshot((const char*)namePtr.ToPointer(), &snapshot))
			{
				player->SetPlayerData(snapshot.health, snapshot.food, snapshot.fallDistance,
				                      snapshot.yRot, snapshot.xRot,
				                      snapshot.sneaking, snapshot.sprinting,
				                      snapshot.x, snapshot.y, snapshot.z, snapshot.dimension);
			}
		}
		finally
		{
			Marshal::FreeHGlobal(namePtr);
		}

		return player;
	}

	bool TryGetDimensionFromWorldName(String^ name, int% dimension)
	{
		if (String::IsNullOrWhiteSpace(name))
		{
			return false;
		}

		String^ normalized = name->Trim();
		int parsedDimension = 0;
		if (Int32::TryParse(normalized, parsedDimension))
		{
			dimension = parsedDimension;
			return true;
		}

		if (String::Equals(normalized, "world", StringComparison::OrdinalIgnoreCase))
		{
			dimension = 0;
			return true;
		}

		if (String::Equals(normalized, "world_nether", StringComparison::OrdinalIgnoreCase))
		{
			dimension = -1;
			return true;
		}

		if (String::Equals(normalized, "world_the_end", StringComparison::OrdinalIgnoreCase))
		{
			dimension = 1;
			return true;
		}

		return false;
	}

	cli::array<String^>^ ParseCommandArguments(String^ commandLine, String^% outLabel)
	{
		outLabel = String::Empty;
		if (String::IsNullOrWhiteSpace(commandLine))
		{
			return gcnew cli::array<String^>(0);
		}

		String^ trimmed = commandLine->Trim();
		if (trimmed->StartsWith("/"))
		{
			trimmed = trimmed->Substring(1);
		}

		if (String::IsNullOrWhiteSpace(trimmed))
		{
			return gcnew cli::array<String^>(0);
		}

		cli::array<String^>^ parts = trimmed->Split(gcnew cli::array<wchar_t>{ ' ' }, StringSplitOptions::RemoveEmptyEntries);
		if (parts == nullptr || parts->Length == 0)
		{
			return gcnew cli::array<String^>(0);
		}

		outLabel = parts[0]->ToLowerInvariant();
		int argCount = parts->Length - 1;
		if (argCount <= 0)
		{
			return gcnew cli::array<String^>(0);
		}

		cli::array<String^>^ args = gcnew cli::array<String^>(argCount);
		for (int i = 0; i < argCount; ++i)
		{
			args[i] = parts[i + 1];
		}
		return args;
	}

	void SetNativeCallbacksExport(PB_SET_NATIVE_CALLBACK_PARAMS)
	{
#define PB_ASSIGN_STORAGE(Name, Ret, Sig) g_##Name = Name;
		PB_NATIVE_CALLBACK_LIST(PB_ASSIGN_STORAGE)
#undef PB_ASSIGN_STORAGE
	}

	int DispatchPlayerCommandExport(const char *playerName, const char *commandLine)
	{
		String ^ managedPlayerName = gcnew String((playerName != nullptr) ? playerName : "");
		String ^ managedCommandLine = gcnew String((commandLine != nullptr) ? commandLine : "");

		try
		{
			return FourKit::DispatchPlayerCommand(managedPlayerName, managedCommandLine) ? 1 : 0;
		}
		catch (Exception ^ ex)
		{
			PluginLogger::LogError("fourkit", String::Format("Error dispatching command: {0}", ex->Message));
			return 0;
		}
	}
}

#define PB_NATIVE_VOID_EXPORT_LIST(X)                                                            \
    X(SetFallDistance, (const char *playerName, float distance), (playerName, distance))         \
    X(SetHealth, (const char *playerName, float health), (playerName, health))                   \
    X(SetFood, (const char *playerName, int food), (playerName, food))                           \
    X(SendMessage, (const char *playerName, const char *message), (playerName, message))         \
    X(TeleportTo, (const char *playerName, double x, double y, double z), (playerName, x, y, z)) \
    X(Kick, (const char *playerName, const char *reason), (playerName, reason))                  \
    X(SetSneaking, (const char *playerName, int sneaking), (playerName, sneaking))               \
    X(SetSprinting, (const char *playerName, int sprinting), (playerName, sprinting))            \
    X(SetAllowFlight, (const char *playerName, int flight), (playerName, flight))                \
    X(SetExhaustion, (const char *playerName, float value), (playerName, value))                 \
    X(SetSaturation, (const char *playerName, float value), (playerName, value))                 \
    X(GiveExp, (const char *playerName, int amount), (playerName, amount))                       \
    X(GiveExpLevels, (const char *playerName, int amount), (playerName, amount))                 \
    X(SetFlying, (const char *playerName, int value), (playerName, value))                       \
    X(SetExp, (const char *playerName, float exp), (playerName, exp))                            \
    X(SetPlayerLevel, (const char *playerName, int level), (playerName, level))                  \
    X(SetWalkSpeed, (const char *playerName, float value), (playerName, value))                  \
    X(SetItemInHand, (const char *playerName, int itemId, int count, int data), (playerName, itemId, count, data)) \
    X(SetPlayerGameMode, (const char *playerName, int mode), (playerName, mode))                 \
    X(BlockBreakNaturally, (int x, int y, int z, int dimension), (x, y, z, dimension))           \
    X(SetBlockType, (int x, int y, int z, int dimension, int id), (x, y, z, dimension, id))      \
    X(SetBlockData, (int x, int y, int z, int dimension, int data), (x, y, z, dimension, data))  \
    X(SetFullTime, (int dimension, long long time), (dimension, time))                           \
    X(SetStorm, (int dimension, int hasStorm), (dimension, hasStorm))                            \
    X(SetThunderDuration, (int dimension, int duration), (dimension, duration))                  \
    X(SetThundering, (int dimension, int thundering), (dimension, thundering))                   \
    X(SetTime, (int dimension, long long time), (dimension, time))                               \
    X(SetWeatherDuration, (int dimension, int duration), (dimension, duration))

#define PB_NATIVE_VALUE_EXPORT_LIST(X)                                                                                                                                                                                       \
    X(BanPlayer, bool, (const char *playerName, const char *reason), false, (playerName, reason))                                                                                                                          \
    X(BanPlayerIp, bool, (const char *playerName, const char *reason), false, (playerName, reason))                                                                                                                        \
    X(IsSneaking, int, (const char *playerName), 0, (playerName))                                                                                                                                                            \
    X(IsSprinting, int, (const char *playerName), 0, (playerName))                                                                                                                                                           \
    X(GetAllowFlight, int, (const char *playerName), 0, (playerName))                                                                                                                                                        \
    X(GetExhaustion, float, (const char *playerName), 0.0f, (playerName))                                                                                                                                                    \
    X(GetSaturation, float, (const char *playerName), 0.0f, (playerName))                                                                                                                                                    \
    X(GetTotalExperience, int, (const char *playerName), 0, (playerName))                                                                                                                                                    \
    X(IsFlying, int, (const char *playerName), 0, (playerName))                                                                                                                                                              \
    X(GetWalkSpeed, float, (const char *playerName), 0.0f, (playerName))                                                                                                                                                     \
    X(IsSleepingPlayer, int, (const char *playerName), 0, (playerName))                                                                                                                                                      \
    X(GetGameMode, int, (const char *playerName), -1, (playerName))                                                                                                                                                          \
    X(GetBlockType, int, (int x, int y, int z, int dimension), 0, (x, y, z, dimension))                                                                                                                                      \
    X(GetBlockData, int, (int x, int y, int z, int dimension), 0, (x, y, z, dimension))                                                                                                                                      \
    X(GetPlayerSnapshot, bool, (const char *playerName, PlayerJoinData *outData), false, (playerName, outData))                                                                                                              \
    X(GetPlayerNetworkAddress, bool, (const char *playerName, PlayerNetworkAddressData *outData), false, (playerName, outData))                                                                                              \
    X(GetItemInHand, bool, (const char *playerName, ItemInHandData *outData), false, (playerName, outData))                                                                                                                  \
    X(GetWorldInfo, bool, (int dimension, WorldInfoData *outData), false, (dimension, outData))                                                                                                                              \
    X(CreateExplosion, int, (int dimension, double x, double y, double z, float power, int setFire, int breakBlocks), 0, (dimension, x, y, z, power, setFire, breakBlocks))                                                  \
    X(DropItem, bool, (int dimension, double x, double y, double z, int itemId, int count, int data, int naturalOffset, DroppedItemData *outData), false, (dimension, x, y, z, itemId, count, data, naturalOffset, outData)) \
    X(GetHighestBlockYAt, int, (int dimension, int x, int z), -1, (dimension, x, z))                                                                                                                                         \
    X(SetSpawnLocation, int, (int dimension, int x, int y, int z), 0, (dimension, x, y, z))                                                                                                                                  \
    X(StrikeLightning, int, (int dimension, double x, double y, double z, int effectOnly), 0, (dimension, x, y, z, effectOnly))

#define PB_FOURKIT_VOID_EXPORT_LIST(X)                                                                                                                   \
    X(FourKit_SetNativeCallbacks, (PB_SET_NATIVE_CALLBACK_PARAMS), (FourKitInternal::SetNativeCallbacksExport(PB_SET_NATIVE_CALLBACK_ARGS)))              \
    X(FourKit_Initialize, (), (FourKit::Initialize()))                                                                                                   \
    X(FourKit_Shutdown, (), (FourKit::Shutdown()))                                                                                                       \
    X(FourKit_FireOnPlayerJoin, (const PlayerJoinData &playerData), (FourKit::FireEventOnPlayerJoin(playerData)))                                        \
    X(FourKit_FireOnPlayerLeave, (const PlayerLeaveData &playerData), (FourKit::FireEventOnPlayerLeave(playerData)))                                     \
    X(FourKit_FireOnPlayerDeath, (PlayerDeathData * deathData), (FourKit::FireEventOnPlayerDeath(deathData)))                                            \
    X(FourKit_FireOnChat, (const PlayerChatData &chatData, bool *cancelled), (FourKit::FireEventOnPlayerChat(chatData, cancelled)))                      \
    X(FourKit_FireOnBlockBreak, (const BlockBreakData &blockBreakData, bool *cancelled), (FourKit::FireEventOnBlockBreak(blockBreakData, cancelled)))    \
    X(FourKit_FireOnBlockPlace, (const BlockPlaceData &blockPlaceData, bool *cancelled), (FourKit::FireEventOnBlockPlace(blockPlaceData, cancelled)))    \
    X(FourKit_FireOnPlayerMove, (const PlayerMoveData &moveData, bool *cancelled), (FourKit::FireEventOnPlayerMove(moveData, cancelled)))                \
    X(FourKit_FireOnPlayerPortal, (PlayerPortalData * portalData, bool *cancelled), (FourKit::FireEventOnPlayerPortal(portalData, cancelled)))           \
    X(FourKit_FireOnSignChange, (SignChangeData * signData, bool *cancelled), (FourKit::FireEventOnSignChange(signData, cancelled)))                     \
    X(FourKit_FireOnPlayerInteract, (PlayerInteractData * interactData, bool *cancelled), (FourKit::FireEventOnPlayerInteract(interactData, cancelled))) \
	X(FourKit_FireOnPlayerDropItem, (PlayerDropItemData * dropData, bool *cancelled), (FourKit::FireEventOnPlayerDropItem(dropData, cancelled)))         \
	X(FourKit_FireOnLoad, (), (FourKit::FireEventOnLoad()))                                                                                              \
	X(FourKit_FireOnExit, (), (FourKit::FireEventOnExit()))

#define PB_FOURKIT_VALUE_EXPORT_LIST(X) \
    X(FourKit_DispatchPlayerCommand, int, (const char *playerName, const char *commandLine), (FourKitInternal::DispatchPlayerCommandExport(playerName, commandLine)))

extern "C"
{
#define PB_DECLARE_NATIVE_VOID_EXPORT(Name, Sig, Args)                      \
    __declspec(dllexport) void NativeCallback_##Name Sig                   \
    {                                                                       \
        if (FourKitInternal::g_##Name != nullptr)                           \
        {                                                                   \
            FourKitInternal::g_##Name Args;                                 \
        }                                                                   \
    }

#define PB_DECLARE_NATIVE_VALUE_EXPORT(Name, Ret, Sig, DefaultValue, Args) \
    __declspec(dllexport) Ret NativeCallback_##Name Sig                    \
    {                                                                      \
        return FourKitInternal::g_##Name != nullptr ? FourKitInternal::g_##Name Args : DefaultValue; \
    }

#define PB_DECLARE_FOURKIT_VOID_EXPORT(Name, Sig, Call) \
    __declspec(dllexport) void Name Sig                 \
    {                                                   \
        Call;                                           \
    }

#define PB_DECLARE_FOURKIT_VALUE_EXPORT(Name, Ret, Sig, Call) \
    __declspec(dllexport) Ret Name Sig                        \
    {                                                         \
        return Call;                                          \
    }

    PB_NATIVE_VOID_EXPORT_LIST(PB_DECLARE_NATIVE_VOID_EXPORT)
    PB_NATIVE_VALUE_EXPORT_LIST(PB_DECLARE_NATIVE_VALUE_EXPORT)
    PB_FOURKIT_VOID_EXPORT_LIST(PB_DECLARE_FOURKIT_VOID_EXPORT)
    PB_FOURKIT_VALUE_EXPORT_LIST(PB_DECLARE_FOURKIT_VALUE_EXPORT)

#undef PB_DECLARE_NATIVE_VOID_EXPORT
#undef PB_DECLARE_NATIVE_VALUE_EXPORT
#undef PB_DECLARE_FOURKIT_VOID_EXPORT
#undef PB_DECLARE_FOURKIT_VALUE_EXPORT
}
