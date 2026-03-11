// 100% better way of doing this. i just kind adumb
// todo: partially rewritten but i should split into different files now like a Responsible Human Being Programmer

#include "FourKit.h"
#include "FourKitInterop.h"
#include "FourKitStructs.h"
#include "PluginLogger.h"
#include "NativeBlockCallbacks.h"
#include "PluginCommand.h"
#include "World.h"

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
		//case 1: return InteractAction::LEFT_CLICK_AIR;
		case 2: return InteractAction::LEFT_CLICK_BLOCK;
		//case 3: return InteractAction::RIGHT_CLICK_AIR;
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

	static cli::array<String^>^ ParseCommandArguments(String^ commandLine, String^% outLabel)
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
        onlinePlayerNames->Clear();

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
        onlinePlayerNames->Clear();
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

        //PluginLogger::LogInfo("fourkit", String::Format("Loaded assembly: {0}", pluginAssembly->FullName));

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

List<Player^>^ FourKit::GetPlayersInDimension(int dimension)
{
	System::Collections::Generic::List<Player^>^ players = gcnew System::Collections::Generic::List<Player^>(0);
	for each (String^ name in onlinePlayerNames)
	{
		Player^ player = ResolvePlayerByName(name);
		if (player != nullptr && player->getDimension() == dimension)
		{
			players->Add(player);
		}
	}

	return players;
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

        //PluginLogger::LogInfo("fourkit", String::Format("Calling OnEnable to {0} plugins", pluginList->Count));
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

        //PluginLogger::LogInfo("fourkit", String::Format("Calling OnDisable to {0} plugins", pluginList->Count));
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
        if (!String::IsNullOrEmpty(name))
        {
            onlinePlayerNames->Add(name);
        }

        Player ^ player = gcnew Player(name);
        player->SetPlayerData(playerData.health, playerData.food, playerData.fallDistance,
		                      playerData.yRot, playerData.xRot,
		                      playerData.sneaking, playerData.sprinting,
		                      playerData.x, playerData.y, playerData.z, playerData.dimension);

        PlayerJoinEvent ^ event = gcnew PlayerJoinEvent();
        event->PlayerObject = player;
        EventManager::FireEvent(event);

        //PluginLogger::LogInfo("fourkit", String::Format("Firing OnPlayerJoin event to {0} listeners", pluginList->Count));
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

        if (!String::IsNullOrEmpty(name))
        {
            onlinePlayerNames->Remove(name);
        }

        //PluginLogger::LogInfo("fourkit", String::Format("Firing OnPlayerLeave event for {0}", name));
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerLeave event: {0}", ex->Message));
    }
}

void FourKit::FireEventOnPlayerDeath(PlayerDeathData* deathData)
{
    try
    {
        if (deathData == nullptr) return;

        String^ name = gcnew String((deathData->playerName != nullptr) ? deathData->playerName : "");

        Player^ player = ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        PlayerDeathEvent^ event = gcnew PlayerDeathEvent();
        event->PlayerObject = player;
        event->DeathMessage = gcnew String(deathData->deathMessage);
        event->KeepInventory = deathData->keepInventory;
        event->KeepLevel = deathData->keepLevel;
        event->NewExp = deathData->newExp;
        event->NewLevel = deathData->newLevel;
        event->NewTotalExp = deathData->newTotalExp;

        EventManager::FireEvent(event);

        deathData->keepInventory = event->KeepInventory;
        deathData->keepLevel = event->KeepLevel;
        deathData->newExp = event->NewExp;
        deathData->newLevel = event->NewLevel;
        deathData->newTotalExp = event->NewTotalExp;

        if (event->DeathMessage != nullptr)
        {
            IntPtr ptr = Marshal::StringToHGlobalAnsi(event->DeathMessage);
            try
            {
                strncpy_s(deathData->deathMessage, sizeof(deathData->deathMessage),
                    (const char*)ptr.ToPointer(), _TRUNCATE);
            }
            finally
            {
                Marshal::FreeHGlobal(ptr);
            }
        }
    }
    catch (Exception^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerDeath event: {0}", ex->Message));
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

void FourKit::FireEventOnSignChange(SignChangeData *signData, bool *cancelled)
{
    try
    {
        if (signData == nullptr)
        {
            if (cancelled != nullptr)
            {
                *cancelled = false;
            }
            return;
        }

        String ^ name = gcnew String((signData->playerName != nullptr) ? signData->playerName : "");

        Player ^ player = ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        SignChangeEvent^ event = gcnew SignChangeEvent();
        event->PlayerObject = player;
        event->BlockObject = gcnew Block(
            signData->x,
            signData->y,
            signData->z,
            signData->dimension,
            NativeBlockCallbacks::GetBlockType(signData->x, signData->y, signData->z, signData->dimension),
            NativeBlockCallbacks::GetBlockData(signData->x, signData->y, signData->z, signData->dimension));

        event->Lines = gcnew cli::array<String^>(4);
        for (int i = 0; i < 4; ++i)
        {
            event->Lines[i] = gcnew String(signData->lines[i]);
        }
        event->Cancelled = false;

        EventManager::FireEvent(event);

        if (event->Lines != nullptr)
        {
            for (int i = 0; i < 4; ++i)
            {
                String^ managedLine = (i < event->Lines->Length && event->Lines[i] != nullptr) ? event->Lines[i] : String::Empty;
                IntPtr linePtr = Marshal::StringToHGlobalAnsi(managedLine);
                try
                {
                    const char* lineChars = (const char*)linePtr.ToPointer();
                    strncpy_s(signData->lines[i], sizeof(signData->lines[i]), lineChars, _TRUNCATE);
                }
                finally
                {
                    Marshal::FreeHGlobal(linePtr);
                }
            }
        }

        if (cancelled != nullptr)
        {
            *cancelled = event->Cancelled;
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnSignChange event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void FourKit::FireEventOnPlayerInteract(PlayerInteractData *interactData, bool *cancelled)
{
	try
	{
		if (interactData == nullptr)
		{
			if (cancelled != nullptr)
			{
				*cancelled = false;
			}
			return;
		}

		String ^ name = gcnew String((interactData->playerName != nullptr) ? interactData->playerName : "");
		Player ^ player = ResolvePlayerByName(name);
		if (player == nullptr)
		{
			player = gcnew Player(name);
		}

		PlayerInteractEvent^ event = gcnew PlayerInteractEvent();
		event->PlayerObject = player;
		event->Action = ToInteractAction(interactData->action);
        event->Face = ToBlockFace(interactData->blockFace);
        event->HasBlock = interactData->hasBlock;
        event->HasItem = interactData->hasItem;
		event->Cancelled = false;

		if (interactData->hasBlock)
		{
			event->ClickedBlock = gcnew Block(
				interactData->x,
				interactData->y,
				interactData->z,
				interactData->dimension,
				interactData->blockId,
				interactData->blockData);
		}
		else
		{
			event->ClickedBlock = nullptr;
		}

		EventManager::FireEvent(event);

		if (cancelled != nullptr)
		{
			*cancelled = event->Cancelled;
		}
	}
	catch (Exception ^ ex)
	{
		PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerInteract event: {0}", ex->Message));
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

World^ FourKit::getWorld(int dimension)
{
	switch (dimension)
	{
	case -1:
	case 0:
	case 1:
		return gcnew World(dimension);
	default:
		return nullptr;
	}
}

World^ FourKit::getWorld(String^ name)
{
	int dimension = 0;
	return TryGetDimensionFromWorldName(name, dimension) ? getWorld(dimension) : nullptr;
}

PluginCommand^ FourKit::getCommand(String^ name)
{
	if (String::IsNullOrWhiteSpace(name))
	{
		return nullptr;
	}

	String^ key = name->Trim()->ToLowerInvariant();
	PluginCommand^ command = nullptr;
	if (!commandMap->TryGetValue(key, command))
	{
		command = gcnew PluginCommand(key);
		commandMap->Add(key, command);
	}

	return command;
}

bool FourKit::DispatchPlayerCommand(String^ playerName, String^ commandLine)
{
	String^ label = String::Empty;
	cli::array<String^>^ args = ParseCommandArguments(commandLine, label);
	if (String::IsNullOrWhiteSpace(label))
	{
		return false;
	}

	PluginCommand^ command = nullptr;
	if (!commandMap->TryGetValue(label, command) || command == nullptr || command->getExecutor() == nullptr)
	{
		command = nullptr;
		for each (KeyValuePair<String^, PluginCommand^> entry in commandMap)
		{
			PluginCommand^ registeredCommand = entry.Value;
			if (registeredCommand == nullptr || registeredCommand->getExecutor() == nullptr)
			{
				continue;
			}

			if (String::Equals(registeredCommand->getName(), label, StringComparison::OrdinalIgnoreCase))
			{
				command = registeredCommand;
				break;
			}

			List<String^>^ aliases = registeredCommand->getAliases();
			if (aliases == nullptr)
			{
				continue;
			}

			for each (String^ alias in aliases)
			{
				if (!String::IsNullOrWhiteSpace(alias) && String::Equals(alias->Trim(), label, StringComparison::OrdinalIgnoreCase))
				{
					command = registeredCommand;
					break;
				}
			}

			if (command != nullptr)
			{
				break;
			}
		}

		if (command == nullptr)
		{
			return false;
		}
	}

	CommandSender^ sender = ResolvePlayerByName(playerName);
	if (sender == nullptr)
	{
		sender = gcnew Player(playerName);
	}

	return command->execute(sender, label, args);
}

// AAAAH
// AJHHHHH
static void SetNativeCallbacksExport(PB_SET_NATIVE_CALLBACK_PARAMS)
{
#define PB_ASSIGN_STORAGE(Name, Ret, Sig) g_##Name = Name;
    PB_NATIVE_CALLBACK_LIST(PB_ASSIGN_STORAGE)
#undef PB_ASSIGN_STORAGE
}

static int DispatchPlayerCommandExport(const char *playerName, const char *commandLine)
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

// stolen from the other callbacks
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
    X(FourKit_SetNativeCallbacks, (PB_SET_NATIVE_CALLBACK_PARAMS), (SetNativeCallbacksExport(PB_SET_NATIVE_CALLBACK_ARGS)))                              \
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
    X(FourKit_FireOnLoad, (), (FourKit::FireEventOnLoad()))                                                                                              \
    X(FourKit_FireOnExit, (), (FourKit::FireEventOnExit()))

#define PB_FOURKIT_VALUE_EXPORT_LIST(X) \
    X(FourKit_DispatchPlayerCommand, int, (const char *playerName, const char *commandLine), (DispatchPlayerCommandExport(playerName, commandLine)))

extern "C"
{
#define PB_DECLARE_NATIVE_VOID_EXPORT(Name, Sig, Args)   \
    __declspec(dllexport) void NativeCallback_##Name Sig \
    {                                                    \
        if (g_##Name != nullptr)                         \
        {                                                \
            g_##Name Args;                               \
        }                                                \
    }

#define PB_DECLARE_NATIVE_VALUE_EXPORT(Name, Ret, Sig, DefaultValue, Args) \
    __declspec(dllexport) Ret NativeCallback_##Name Sig                    \
    {                                                                      \
        return g_##Name != nullptr ? g_##Name Args : DefaultValue;         \
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
