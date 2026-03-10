// 100% better way of doing this. i just kind adumb
// todo: redo this

#include "FourKit.h"
#include "FourKitInterop.h"
#include "FourKitStructs.h"
#include "PluginLogger.h"
#include "NativeBlockCallbacks.h"

using namespace System;
using namespace System::Reflection;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;


namespace
{
#define PB_DECLARE_STORAGE(Name, Ret, Sig) FourKit##Name##Callback g_##Name = nullptr;
	PB_NATIVE_CALLBACK_LIST(PB_DECLARE_STORAGE)
#undef PB_DECLARE_STORAGE

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
}

bool FourKit::Initialize()
{
    String ^ pluginsFolderPath = "plugins";

    try
    {
        if (!Directory::Exists(pluginsFolderPath))
        {
            PluginLogger::LogInfo("fourkit", "Plugins folder does not exist, creating...");
            Directory::CreateDirectory(pluginsFolderPath);
        }

        pluginList->Clear();

        cli::array<String ^> ^ dllFiles = Directory::GetFiles(pluginsFolderPath, "*.dll");

        PluginLogger::LogInfo("fourkit", String::Format("Found {0} plugin DLL files", dllFiles->Length));

        for each (String ^ dllFile in dllFiles)
        {
            if (!LoadPlugin(dllFile))
            {
                PluginLogger::LogWarn("fourkit", String::Format("Failed to load plugin: {0}", dllFile));
            }
        }

        PluginLogger::LogInfo("fourkit", String::Format("Loaded {0} plugins successfully", pluginList->Count));

        return true;
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error during initialization: {0}", ex->Message));
        return false;
    }
}

void FourKit::Shutdown()
{
    try
    {
        FireEventOnExit();
        pluginList->Clear();
        PluginLogger::LogInfo("fourkit", "Plugin system shut down");
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error during shutdown: {0}", ex->Message));
    }
}

bool FourKit::LoadPlugin(String ^ pluginPath)
{
    try
    {
        Assembly ^ pluginAssembly = Assembly::LoadFrom(pluginPath);

        if (pluginAssembly == nullptr)
        {
            PluginLogger::LogError("fourkit", String::Format("Failed to load assembly: {0}", pluginPath));
            return false;
        }

        PluginLogger::LogInfo("fourkit", String::Format("Loaded assembly: {0}", pluginAssembly->FullName));

        cli::array<Type ^> ^ types = pluginAssembly->GetTypes();

        bool pluginFound = false;

        // should we find it from the interface ServerPlugin or make user define it in some file?

        for each (Type ^ type in types)
        {
            if (type->GetInterface("ServerPlugin") != nullptr && !type->IsInterface)
            {
                try
                {
                    ServerPlugin ^ plugin = safe_cast<ServerPlugin ^>(Activator::CreateInstance(type));

                    if (plugin != nullptr)
                    {
                        pluginList->Add(plugin);
                        PluginLogger::LogInfo("fourkit", 
                            String::Format("Loaded plugin: {0} v{1} by {2}",
                                plugin->GetName(), plugin->GetVersion(), plugin->GetAuthor()));
                        pluginFound = true;
                    }
                }
                catch (Exception ^ ex)
                {
                    PluginLogger::LogError("fourkit",
                        String::Format("Failed to instantiate plugin {0}: {1}",
                            type->FullName, ex->Message));
                }
            }
        }

        if (!pluginFound)
        {
            PluginLogger::LogWarn("fourkit", 
                String::Format("No ServerPlugin implementations found in assembly: {0}", pluginPath));
        }

        return pluginFound;
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error loading plugin: {0}", ex->Message));
        return false;
    }
}

List<ServerPlugin ^> ^ FourKit::GetLoadedPlugins()
{
    return pluginList;
}

void FourKit::LogPlugin(String ^ pluginName, String ^ message)
{
    PluginLogger::LogPluginInfo(pluginName, message);
}

void FourKit::FireEventOnLoad()
{
    try
    {
        ServerLoadEvent ^ event = gcnew ServerLoadEvent();
        event->ServerName = "Minecraft Dedicated Server";
        EventManager::FireEvent(event);

        PluginLogger::LogInfo("fourkit", String::Format("Calling OnEnable to {0} plugins", pluginList->Count));
        for each (ServerPlugin ^ plugin in pluginList)
        {
            try
            {
                EventManager::SetCurrentPlugin(plugin->GetName());

                plugin->OnEnable();

                EventManager::ClearCurrentPlugin();
            }
            catch (Exception ^ ex)
            {
                PluginLogger::LogError("fourkit",
                    String::Format("Error in OnEnable for plugin {0}: {1}",
                        plugin->GetName(), ex->Message));
                EventManager::ClearCurrentPlugin();
            }
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnLoad event: {0}", ex->Message));
    }
}

void FourKit::FireEventOnExit()
{
    try
    {
        ServerShutdownEvent ^ event = gcnew ServerShutdownEvent();
        event->Reason = "Server shutdown";
        EventManager::FireEvent(event);

        PluginLogger::LogInfo("fourkit", String::Format("Calling OnDisable to {0} plugins", pluginList->Count));
        for each (ServerPlugin ^ plugin in pluginList)
        {
            try
            {
                plugin->OnDisable();
            }
            catch (Exception ^ ex)
            {
                PluginLogger::LogError("fourkit",
                    String::Format("Error in OnDisable for plugin {0}: {1}",
                        plugin->GetName(), ex->Message));
            }
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnExit event: {0}", ex->Message));
    }
}

void FourKit::FireEventOnPlayerJoin(const PlayerJoinData &playerData)
{
    try
    {
        String ^ name = gcnew String(playerData.playerName);

        Player ^ player = gcnew Player(name);
        player->SetPlayerData(playerData.health, playerData.food, playerData.fallDistance,
		                      playerData.yRot, playerData.xRot,
		                      playerData.sneaking, playerData.sprinting,
		                      playerData.x, playerData.y, playerData.z, playerData.dimension);

        PlayerJoinEvent ^ event = gcnew PlayerJoinEvent();
        event->PlayerObject = player;
        EventManager::FireEvent(event);

        PluginLogger::LogInfo("fourkit", String::Format("Firing OnPlayerJoin event to {0} listeners", pluginList->Count));
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerJoin event: {0}", ex->Message));
    }
}

void FourKit::FireEventOnPlayerLeave(const PlayerLeaveData &playerData)
{
    try
    {
        String ^ name = gcnew String((playerData.playerName != nullptr) ? playerData.playerName : "");

        Player ^ player = ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        PlayerLeaveEvent ^ event = gcnew PlayerLeaveEvent();
        event->PlayerObject = player;
        EventManager::FireEvent(event);

        PluginLogger::LogInfo("fourkit", String::Format("Firing OnPlayerLeave event for {0}", name));
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerLeave event: {0}", ex->Message));
    }
}

void FourKit::FireEventOnPlayerChat(const PlayerChatData &chatData, bool *cancelled)
{
    try
    {
        String ^ name = gcnew String((chatData.playerName != nullptr) ? chatData.playerName : "");
        String ^ message = gcnew String((chatData.message != nullptr) ? chatData.message : "");

        Player ^ player = ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        PlayerChatEvent ^ event = gcnew PlayerChatEvent();
        event->PlayerObject = player;
        event->Message = message;
        event->Cancelled = false;

        EventManager::FireEvent(event);

        if (cancelled != nullptr)
        {
            *cancelled = event->Cancelled;
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerChat event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void FourKit::FireEventOnBlockBreak(const BlockBreakData &blockBreakData, bool *cancelled)
{
    try
    {
        String ^ name = gcnew String((blockBreakData.playerName != nullptr) ? blockBreakData.playerName : "");

        Player ^ player = ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        Block ^ block = gcnew Block(
            blockBreakData.x,
            blockBreakData.y,
            blockBreakData.z,
            0,
            blockBreakData.blockId,
            blockBreakData.blockData);

        BlockBreakEvent ^ event = gcnew BlockBreakEvent();
        event->PlayerObject = player;
        event->BlockObject = block;
        event->Cancelled = false;

        EventManager::FireEvent(event);

        if (cancelled != nullptr)
        {
            *cancelled = event->Cancelled;
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnBlockBreak event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void FourKit::FireEventOnBlockPlace(const BlockPlaceData &blockPlaceData, bool *cancelled)
{
    try
    {
        String ^ name = gcnew String((blockPlaceData.playerName != nullptr) ? blockPlaceData.playerName : "");

        Player ^ player = ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        Block ^ block = gcnew Block(
            blockPlaceData.x,
            blockPlaceData.y,
            blockPlaceData.z,
            0,
            blockPlaceData.blockId,
            blockPlaceData.blockData);

        BlockPlaceEvent ^ event = gcnew BlockPlaceEvent();
        event->PlayerObject = player;
        event->BlockObject = block;
        event->Cancelled = false;

        EventManager::FireEvent(event);

        if (cancelled != nullptr)
        {
            *cancelled = event->Cancelled;
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnBlockPlace event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void FourKit::FireEventOnPlayerMove(const PlayerMoveData &moveData, bool *cancelled)
{
    try
    {
        String ^ name = gcnew String((moveData.playerName != nullptr) ? moveData.playerName : "");

        Player ^ player = ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        PlayerMoveEvent^ event = gcnew PlayerMoveEvent();
        event->PlayerObject = player;
        event->From = gcnew Location(moveData.fromX, moveData.fromY, moveData.fromZ);
        event->To = gcnew Location(moveData.toX, moveData.toY, moveData.toZ);
        event->Cancelled = false;

        EventManager::FireEvent(event);

        if (cancelled != nullptr)
        {
            *cancelled = event->Cancelled;
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerMove event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void FourKit::FireEventOnPlayerPortal(PlayerPortalData *portalData, bool *cancelled)
{
    try
    {
        if (portalData == nullptr)
        {
            if (cancelled != nullptr)
            {
                *cancelled = false;
            }
            return;
        }

        String ^ name = gcnew String((portalData->playerName != nullptr) ? portalData->playerName : "");

        Player ^ player = ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        PlayerPortalEvent^ event = gcnew PlayerPortalEvent();
        event->PlayerObject = player;

        TeleportCause cause = TeleportCause::UNKNOWN;
        if (portalData->cause == (int)TeleportCause::END_PORTAL) cause = TeleportCause::END_PORTAL;
        else if (portalData->cause == (int)TeleportCause::ENDER_PEARL) cause = TeleportCause::ENDER_PEARL;
        else if (portalData->cause == (int)TeleportCause::NETHER_PORTAL) cause = TeleportCause::NETHER_PORTAL;
        else if (portalData->cause == (int)TeleportCause::PLUGIN) cause = TeleportCause::PLUGIN;

        event->Cause = cause;
        event->From = gcnew Location(portalData->fromX, portalData->fromY, portalData->fromZ);
        event->To = gcnew Location(portalData->toX, portalData->toY, portalData->toZ);
        event->Cancelled = false;

        EventManager::FireEvent(event);

        if (event->From != nullptr)
        {
            portalData->fromX = event->From->getX();
            portalData->fromY = event->From->getY();
            portalData->fromZ = event->From->getZ();
        }
        if (event->To != nullptr)
        {
            portalData->toX = event->To->getX();
            portalData->toY = event->To->getY();
            portalData->toZ = event->To->getZ();
        }

        if (cancelled != nullptr)
        {
            *cancelled = event->Cancelled;
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerPortal event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void FourKit::addListener(Listener ^ listener)
{
    EventManager::RegisterListener(listener);
}

Block ^ FourKit::getBlockAt(double x, double y, double z)
{
    int bx = (int)Math::Floor(x);
    int by = (int)Math::Floor(y);
    int bz = (int)Math::Floor(z);
    int dimension = 0;

    int blockType = NativeBlockCallbacks::GetBlockType(bx, by, bz, dimension);
    int blockData = NativeBlockCallbacks::GetBlockData(bx, by, bz, dimension);

    return gcnew Block(bx, by, bz, dimension, blockType, blockData);
}

Player ^ FourKit::getPlayer(String ^ name)
{
    return ResolvePlayerByName(name);
}


// todo: redo how this export stuff works
// messy

extern "C"
{
	__declspec(dllexport) void FourKit_SetNativeCallbacks(
		FourKitSetFallDistanceCallback SetFallDistance,
		FourKitSetHealthCallback SetHealth,
		FourKitSetFoodCallback SetFood,
		FourKitSendMessageCallback SendMessage,
		FourKitTeleportToCallback TeleportTo,
		FourKitKickCallback Kick,
		FourKitIsSneakingCallback IsSneaking,
		FourKitSetSneakingCallback SetSneaking,
		FourKitIsSprintingCallback IsSprinting,
		FourKitSetSprintingCallback SetSprinting,
		FourKitBlockBreakNaturallyCallback BlockBreakNaturally,
		FourKitGetBlockTypeCallback GetBlockType,
		FourKitSetBlockTypeCallback SetBlockType,
		FourKitGetBlockDataCallback GetBlockData,
		FourKitSetBlockDataCallback SetBlockData,
		FourKitGetPlayerSnapshotCallback GetPlayerSnapshot,
		FourKitGetPlayerNetworkAddressCallback GetPlayerNetworkAddress)
	{
#define PB_ASSIGN_STORAGE(Name, Ret, Sig) g_##Name = Name;
		PB_NATIVE_CALLBACK_LIST(PB_ASSIGN_STORAGE)
#undef PB_ASSIGN_STORAGE
	}

	__declspec(dllexport) void NativeCallback_SetFallDistance(const char* playerName, float distance)
	{
		if (g_SetFallDistance != nullptr)
		{
			g_SetFallDistance(playerName, distance);
		}
	}

	__declspec(dllexport) void NativeCallback_SetHealth(const char* playerName, float health)
	{
		if (g_SetHealth != nullptr)
		{
			g_SetHealth(playerName, health);
		}
	}

	__declspec(dllexport) void NativeCallback_SetFood(const char* playerName, int food)
	{
		if (g_SetFood != nullptr)
		{
			g_SetFood(playerName, food);
		}
	}

	__declspec(dllexport) void NativeCallback_SendMessage(const char* playerName, const char* message)
	{
		if (g_SendMessage != nullptr)
		{
			g_SendMessage(playerName, message);
		}
	}

	__declspec(dllexport) void NativeCallback_TeleportTo(const char* playerName, double x, double y, double z)
	{
		if (g_TeleportTo != nullptr)
		{
			g_TeleportTo(playerName, x, y, z);
		}
	}

	__declspec(dllexport) void NativeCallback_Kick(const char* playerName, const char* reason)
	{
		if (g_Kick != nullptr)
		{
			g_Kick(playerName, reason);
		}
	}

	__declspec(dllexport) int NativeCallback_IsSneaking(const char* playerName)
	{
		if (g_IsSneaking != nullptr)
		{
			return g_IsSneaking(playerName);
		}
		return 0;
	}

	__declspec(dllexport) void NativeCallback_SetSneaking(const char* playerName, int sneaking)
	{
		if (g_SetSneaking != nullptr)
		{
			g_SetSneaking(playerName, sneaking);
		}
	}

	__declspec(dllexport) int NativeCallback_IsSprinting(const char* playerName)
	{
		if (g_IsSprinting != nullptr)
		{
			return g_IsSprinting(playerName);
		}
		return 0;
	}

	__declspec(dllexport) void NativeCallback_SetSprinting(const char* playerName, int sprinting)
	{
		if (g_SetSprinting != nullptr)
		{
			g_SetSprinting(playerName, sprinting);
		}
	}

	__declspec(dllexport) void NativeCallback_BlockBreakNaturally(int x, int y, int z, int dimension)
	{
		if (g_BlockBreakNaturally != nullptr)
		{
			g_BlockBreakNaturally(x, y, z, dimension);
		}
	}

	__declspec(dllexport) int NativeCallback_GetBlockType(int x, int y, int z, int dimension)
	{
		if (g_GetBlockType != nullptr)
		{
			return g_GetBlockType(x, y, z, dimension);
		}
		return 0;
	}

	__declspec(dllexport) void NativeCallback_SetBlockType(int x, int y, int z, int dimension, int id)
	{
		if (g_SetBlockType != nullptr)
		{
			g_SetBlockType(x, y, z, dimension, id);
		}
	}

	__declspec(dllexport) int NativeCallback_GetBlockData(int x, int y, int z, int dimension)
	{
		if (g_GetBlockData != nullptr)
		{
			return g_GetBlockData(x, y, z, dimension);
		}
		return 0;
	}

	__declspec(dllexport) void NativeCallback_SetBlockData(int x, int y, int z, int dimension, int data)
	{
		if (g_SetBlockData != nullptr)
		{
			g_SetBlockData(x, y, z, dimension, data);
		}
	}

	__declspec(dllexport) bool NativeCallback_GetPlayerSnapshot(const char* playerName, PlayerJoinData* outData)
	{
		if (g_GetPlayerSnapshot != nullptr)
		{
			return g_GetPlayerSnapshot(playerName, outData);
		}
		return false;
	}

	__declspec(dllexport) bool NativeCallback_GetPlayerNetworkAddress(const char* playerName, PlayerNetworkAddressData* outData)
	{
		if (g_GetPlayerNetworkAddress != nullptr)
		{
			return g_GetPlayerNetworkAddress(playerName, outData);
		}
		return false;
	}

    __declspec(dllexport) void FourKit_Initialize()
    {
        FourKit::Initialize();
    }

    __declspec(dllexport) void FourKit_Shutdown()
    {
        FourKit::Shutdown();
    }

    __declspec(dllexport) void FourKit_FireOnPlayerJoin(const PlayerJoinData &playerData)
    {
        FourKit::FireEventOnPlayerJoin(playerData);
    }

    __declspec(dllexport) void FourKit_FireOnPlayerLeave(const PlayerLeaveData &playerData)
    {
        FourKit::FireEventOnPlayerLeave(playerData);
    }

    __declspec(dllexport) void FourKit_FireOnChat(const PlayerChatData &chatData, bool* cancelled)
    {
        FourKit::FireEventOnPlayerChat(chatData, cancelled);
    }

    __declspec(dllexport) void FourKit_FireOnBlockBreak(const BlockBreakData &blockBreakData, bool* cancelled)
    {
        FourKit::FireEventOnBlockBreak(blockBreakData, cancelled);
    }

    __declspec(dllexport) void FourKit_FireOnBlockPlace(const BlockPlaceData &blockPlaceData, bool* cancelled)
    {
        FourKit::FireEventOnBlockPlace(blockPlaceData, cancelled);
    }

    __declspec(dllexport) void FourKit_FireOnPlayerMove(const PlayerMoveData &moveData, bool* cancelled)
    {
        FourKit::FireEventOnPlayerMove(moveData, cancelled);
    }

    __declspec(dllexport) void FourKit_FireOnPlayerPortal(PlayerPortalData* portalData, bool* cancelled)
    {
        FourKit::FireEventOnPlayerPortal(portalData, cancelled);
    }

    __declspec(dllexport) void FourKit_FireOnLoad()
    {
        FourKit::FireEventOnLoad();
    }

    __declspec(dllexport) void FourKit_FireOnExit()
    {
        FourKit::FireEventOnExit();
    }
}
