#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include "EventManager.h"
#include "Events.h"
#include "Listener.h"
#include "Player.h"
#include "Block.h"
#include "PluginCommand.h"
#include "FourKitStructs.h"

ref class World;

public ref class FourKit
{
public:
	static void addListener(Listener^ listener);
	static Block^ getBlockAt(double x, double y, double z);
	static Player^ getPlayer(String^ name);
	static World^ getWorld(int dimension);
	static World^ getWorld(String^ name);
	static PluginCommand^ getCommand(String^ name);
	static void broadcastMessage(String^ message);
private:
	static List<ServerPlugin^>^ pluginList;
	static Dictionary<String^, PluginCommand^>^ commandMap;
	static System::Collections::Generic::HashSet<String^>^ onlinePlayerNames;

	static FourKit()
	{
		// Visual Studio Pls
		pluginList = gcnew List<ServerPlugin^>();
		commandMap = gcnew Dictionary<String^, PluginCommand^>(StringComparer::OrdinalIgnoreCase);
		onlinePlayerNames = gcnew System::Collections::Generic::HashSet<String^>(StringComparer::OrdinalIgnoreCase);
	}
internal:
	static bool Initialize();

    static void Shutdown();

    static bool LoadPlugin(String ^ pluginPath);

    static List<ServerPlugin ^> ^ GetLoadedPlugins();
    static List<Player^>^ GetPlayersInDimension(int dimension);

    static void LogPlugin(String ^ pluginName, String ^ message);
    static void FireEventOnLoad();
    static void FireEventOnExit();
    static void FireEventOnPlayerJoin(const PlayerJoinData &playerData);
    static void FireEventOnPlayerLeave(const PlayerLeaveData &playerData);
    static void FireEventOnPlayerChat(const PlayerChatData &chatData, bool *cancelled);
    static void FireEventOnBlockBreak(const BlockBreakData &blockBreakData, bool *cancelled);
    static void FireEventOnBlockPlace(const BlockPlaceData &blockPlaceData, bool *cancelled);
    static void FireEventOnPlayerMove(const PlayerMoveData &moveData, bool *cancelled);
    static void FireEventOnPlayerPortal(PlayerPortalData *portalData, bool *cancelled);
    static void FireEventOnSignChange(SignChangeData *signData, bool *cancelled);
    static void FireEventOnPlayerInteract(PlayerInteractData *interactData, bool *cancelled);
	static bool DispatchPlayerCommand(String^ playerName, String^ commandLine);
	static void FireEventOnPlayerDropItem(PlayerDropItemData* dropData, bool* cancelled);
	static void FireEventOnPlayerDeath(PlayerDeathData* deathData);
	static void FireEventOnEntityDamage(EntityDamageData* damageData, bool* cancelled);
	static void FireEventOnEntityDamageByEntity(EntityDamageData* damageData, bool* cancelled);
};