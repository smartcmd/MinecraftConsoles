// 100% better way of doing this. i just kind adumb
// todo: redo this

#include "PluginBridge.h"
#include "PluginBridgeInterop.h"
#include "PluginBridgeStructs.h"
#include "PluginLogger.h"
#include "NativeBlockCallbacks.h"

using namespace System;
using namespace System::Reflection;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;


namespace
{
#define PB_DECLARE_STORAGE(Name, Ret, Sig) PluginBridge##Name##Callback g_##Name = nullptr;
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

bool PluginBridge::Initialize()
{
    String ^ pluginsFolderPath = "plugins";

    try
    {
        if (!Directory::Exists(pluginsFolderPath))
        {
            PluginLogger::LogInfo("pluginbridge", "Plugins folder does not exist, creating...");
            Directory::CreateDirectory(pluginsFolderPath);
        }

        pluginList->Clear();

        cli::array<String ^> ^ dllFiles = Directory::GetFiles(pluginsFolderPath, "*.dll");

        PluginLogger::LogInfo("pluginbridge", String::Format("Found {0} plugin DLL files", dllFiles->Length));

        for each (String ^ dllFile in dllFiles)
        {
            if (!LoadPlugin(dllFile))
            {
                PluginLogger::LogWarn("pluginbridge", String::Format("Failed to load plugin: {0}", dllFile));
            }
        }

        PluginLogger::LogInfo("pluginbridge", String::Format("Loaded {0} plugins successfully", pluginList->Count));

        return true;
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("pluginbridge", String::Format("Error during initialization: {0}", ex->Message));
        return false;
    }
}

void PluginBridge::Shutdown()
{
    try
    {
        FireEventOnExit();
        pluginList->Clear();
        PluginLogger::LogInfo("pluginbridge", "Plugin system shut down");
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("pluginbridge", String::Format("Error during shutdown: {0}", ex->Message));
    }
}

bool PluginBridge::LoadPlugin(String ^ pluginPath)
{
    try
    {
        Assembly ^ pluginAssembly = Assembly::LoadFrom(pluginPath);

        if (pluginAssembly == nullptr)
        {
            PluginLogger::LogError("pluginbridge", String::Format("Failed to load assembly: {0}", pluginPath));
            return false;
        }

        PluginLogger::LogInfo("pluginbridge", String::Format("Loaded assembly: {0}", pluginAssembly->FullName));

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
                        PluginLogger::LogInfo("pluginbridge", 
                            String::Format("Loaded plugin: {0} v{1} by {2}",
                                plugin->GetName(), plugin->GetVersion(), plugin->GetAuthor()));
                        pluginFound = true;
                    }
                }
                catch (Exception ^ ex)
                {
                    PluginLogger::LogError("pluginbridge",
                        String::Format("Failed to instantiate plugin {0}: {1}",
                            type->FullName, ex->Message));
                }
            }
        }

        if (!pluginFound)
        {
            PluginLogger::LogWarn("pluginbridge", 
                String::Format("No ServerPlugin implementations found in assembly: {0}", pluginPath));
        }

        return pluginFound;
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("pluginbridge", String::Format("Error loading plugin: {0}", ex->Message));
        return false;
    }
}

List<ServerPlugin ^> ^ PluginBridge::GetLoadedPlugins()
{
    return pluginList;
}

void PluginBridge::LogPlugin(String ^ pluginName, String ^ message)
{
    PluginLogger::LogPluginInfo(pluginName, message);
}

void PluginBridge::FireEventOnLoad()
{
    try
    {
        ServerLoadEvent ^ event = gcnew ServerLoadEvent();
        event->ServerName = "Minecraft Dedicated Server";
        EventManager::FireEvent(event);

        PluginLogger::LogInfo("pluginbridge", String::Format("Calling OnEnable to {0} plugins", pluginList->Count));
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
                PluginLogger::LogError("pluginbridge",
                    String::Format("Error in OnEnable for plugin {0}: {1}",
                        plugin->GetName(), ex->Message));
                EventManager::ClearCurrentPlugin();
            }
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("pluginbridge", String::Format("Error firing OnLoad event: {0}", ex->Message));
    }
}

void PluginBridge::FireEventOnExit()
{
    try
    {
        ServerShutdownEvent ^ event = gcnew ServerShutdownEvent();
        event->Reason = "Server shutdown";
        EventManager::FireEvent(event);

        PluginLogger::LogInfo("pluginbridge", String::Format("Calling OnDisable to {0} plugins", pluginList->Count));
        for each (ServerPlugin ^ plugin in pluginList)
        {
            try
            {
                plugin->OnDisable();
            }
            catch (Exception ^ ex)
            {
                PluginLogger::LogError("pluginbridge",
                    String::Format("Error in OnDisable for plugin {0}: {1}",
                        plugin->GetName(), ex->Message));
            }
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("pluginbridge", String::Format("Error firing OnExit event: {0}", ex->Message));
    }
}

void PluginBridge::FireEventOnPlayerJoin(const PlayerJoinData &playerData)
{
    try
    {
        String ^ name = gcnew String(playerData.playerName);

        Player ^ player = gcnew Player(name);
        player->SetPlayerData(playerData.health, playerData.food, playerData.fallDistance,
                              playerData.yRot, playerData.xRot,
                              playerData.x, playerData.y, playerData.z, playerData.dimension);

        PlayerJoinEvent ^ event = gcnew PlayerJoinEvent();
        event->PlayerObject = player;
        EventManager::FireEvent(event);

        PluginLogger::LogInfo("pluginbridge", String::Format("Firing OnPlayerJoin event to {0} listeners", pluginList->Count));
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("pluginbridge", String::Format("Error firing OnPlayerJoin event: {0}", ex->Message));
    }
}

void PluginBridge::FireEventOnPlayerLeave(const PlayerLeaveData &playerData)
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

        PluginLogger::LogInfo("pluginbridge", String::Format("Firing OnPlayerLeave event for {0}", name));
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("pluginbridge", String::Format("Error firing OnPlayerLeave event: {0}", ex->Message));
    }
}

void PluginBridge::FireEventOnPlayerChat(const PlayerChatData &chatData, bool* cancelled)
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
        PluginLogger::LogError("pluginbridge", String::Format("Error firing OnPlayerChat event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void PluginBridge::FireEventOnBlockBreak(const BlockBreakData &blockBreakData, bool* cancelled)
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
        PluginLogger::LogError("pluginbridge", String::Format("Error firing OnBlockBreak event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void PluginBridge::FireEventOnBlockPlace(const BlockPlaceData &blockPlaceData, bool* cancelled)
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
        PluginLogger::LogError("pluginbridge", String::Format("Error firing OnBlockPlace event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void PluginBridge::FireEventOnPlayerMove(const PlayerMoveData &moveData, bool* cancelled)
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
        PluginLogger::LogError("pluginbridge", String::Format("Error firing OnPlayerMove event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void PluginBridge::addListener(Listener ^ listener)
{
    EventManager::RegisterListener(listener);
}

Block^ PluginBridge::getBlockAt(double x, double y, double z)
{
    int bx = (int)Math::Floor(x);
    int by = (int)Math::Floor(y);
    int bz = (int)Math::Floor(z);
    int dimension = 0;

    int blockType = NativeBlockCallbacks::GetBlockType(bx, by, bz, dimension);
    int blockData = NativeBlockCallbacks::GetBlockData(bx, by, bz, dimension);

    return gcnew Block(bx, by, bz, dimension, blockType, blockData);
}

Player^ PluginBridge::getPlayer(String^ name)
{
    return ResolvePlayerByName(name);
}


// todo: redo how this export stuff works
// messy

extern "C"
{
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
		PluginBridgeGetPlayerSnapshotCallback GetPlayerSnapshot)
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

    __declspec(dllexport) void PluginBridge_Initialize()
    {
        PluginBridge::Initialize();
    }

    __declspec(dllexport) void PluginBridge_Shutdown()
    {
        PluginBridge::Shutdown();
    }

    __declspec(dllexport) void PluginBridge_FireOnPlayerJoin(const PlayerJoinData &playerData)
    {
        PluginBridge::FireEventOnPlayerJoin(playerData);
    }

    __declspec(dllexport) void PluginBridge_FireOnPlayerLeave(const PlayerLeaveData &playerData)
    {
        PluginBridge::FireEventOnPlayerLeave(playerData);
    }

    __declspec(dllexport) void PluginBridge_FireOnChat(const PlayerChatData &chatData, bool* cancelled)
    {
        PluginBridge::FireEventOnPlayerChat(chatData, cancelled);
    }

    __declspec(dllexport) void PluginBridge_FireOnBlockBreak(const BlockBreakData &blockBreakData, bool* cancelled)
    {
        PluginBridge::FireEventOnBlockBreak(blockBreakData, cancelled);
    }

    __declspec(dllexport) void PluginBridge_FireOnBlockPlace(const BlockPlaceData &blockPlaceData, bool* cancelled)
    {
        PluginBridge::FireEventOnBlockPlace(blockPlaceData, cancelled);
    }

    __declspec(dllexport) void PluginBridge_FireOnPlayerMove(const PlayerMoveData &moveData, bool* cancelled)
    {
        PluginBridge::FireEventOnPlayerMove(moveData, cancelled);
    }

    __declspec(dllexport) void PluginBridge_FireOnLoad()
    {
        PluginBridge::FireEventOnLoad();
    }

    __declspec(dllexport) void PluginBridge_FireOnExit()
    {
        PluginBridge::FireEventOnExit();
    }
}
