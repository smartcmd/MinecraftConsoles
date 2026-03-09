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
#include "FourKitStructs.h"

using namespace System;
using namespace System::Reflection;
using namespace System::IO;
using namespace System::Collections::Generic;

enum class PluginEventType
{
	OnLoad,
	OnExit,
	OnPlayerJoin
};

public ref class PlayerJoinEventArgs
{
public:
	String^ PlayerName;
};

public ref class FourKit
{
public:
	static void addListener(Listener^ listener);
	static Block^ getBlockAt(double x, double y, double z);
	static Player^ getPlayer(String^ name);
private:
	static List<ServerPlugin^>^ pluginList;

	static FourKit()
	{
		// Visual Studio Pls
		pluginList = gcnew List<ServerPlugin^>();
	}
internal:
	static bool Initialize();

    static void Shutdown();

    static bool LoadPlugin(String ^ pluginPath);

    static List<ServerPlugin ^> ^ GetLoadedPlugins();

    static void LogPlugin(String ^ pluginName, String ^ message);
    static void FireEventOnLoad();
    static void FireEventOnExit();
    static void FireEventOnPlayerJoin(const PlayerJoinData &playerData);
    static void FireEventOnPlayerLeave(const PlayerLeaveData &playerData);
    static void FireEventOnPlayerChat(const PlayerChatData &chatData, bool *cancelled);
    static void FireEventOnBlockBreak(const BlockBreakData &blockBreakData, bool *cancelled);
    static void FireEventOnBlockPlace(const BlockPlaceData &blockPlaceData, bool *cancelled);
    static void FireEventOnPlayerMove(const PlayerMoveData &moveData, bool *cancelled);
};