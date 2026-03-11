#pragma once
// todo: this sucks
#include "FourKitInterop.h"

using namespace System;
using namespace System::Runtime::InteropServices;

private ref class NativePlayerCallbacks
{
public:
	static void SetFallDistance(String^ playerName, float distance);
	static void SetHealth(String^ playerName, float health);
	static void SetFood(String^ playerName, int food);
	static void SendMessage(String^ playerName, String^ message);
	static void TeleportTo(String^ playerName, double x, double y, double z);
	static void Kick(String^ playerName, String^ reason);
	static bool IsSneaking(String^ playerName);
	static void SetSneaking(String^ playerName, bool sneaking);
	static bool IsSprinting(String^ playerName);
	static void SetSprinting(String^ playerName, bool sprinting);
	static bool GetPlayerNetworkAddress(String^ playerName, PlayerNetworkAddressData* outData);
	static bool GetAllowFlight(String^ playerName);
	static void SetAllowFlight(String^ playerName, bool flight);
	static float GetExhaustion(String^ playerName);
	static void SetExhaustion(String^ playerName, float value);
	static float GetSaturation(String^ playerName);
	static void SetSaturation(String^ playerName, float value);
	static void GiveExp(String^ playerName, int amount);
	static void GiveExpLevels(String^ playerName, int amount);
	static int GetTotalExperience(String^ playerName);
	static bool IsFlying(String^ playerName);
	static void SetFlying(String^ playerName, bool value);
	static void SetExp(String^ playerName, float exp);
	static void SetPlayerLevel(String^ playerName, int level);
	static void SetWalkSpeed(String^ playerName, float value);
	static float GetWalkSpeed(String^ playerName);
	static bool GetItemInHand(String^ playerName, ItemInHandData* outData);
	static void SetItemInHand(String^ playerName, int itemId, int count, int data);
	static bool IsSleepingPlayer(String^ playerName);
	static int GetGameMode(String^ playerName);
	static void SetPlayerGameMode(String^ playerName, int mode);

private:
#define PB_DECLARE_PLAYER_DLLIMPORT(Name, Ret, Sig) \
	[DllImport("Minecraft.Server.FourKit.dll", EntryPoint = "NativeCallback_" #Name, CallingConvention = CallingConvention::Cdecl)] \
	static Ret NativeCallback_##Name Sig;
	PB_PLAYER_CALLBACK_LIST(PB_DECLARE_PLAYER_DLLIMPORT)
#undef PB_DECLARE_PLAYER_DLLIMPORT

	[DllImport("Minecraft.Server.FourKit.dll", EntryPoint = "NativeCallback_GetPlayerNetworkAddress", CallingConvention = CallingConvention::Cdecl)]
	static bool NativeCallback_GetPlayerNetworkAddress(const char* playerName, PlayerNetworkAddressData* outData);

	[DllImport("Minecraft.Server.FourKit.dll", EntryPoint = "NativeCallback_GetItemInHand", CallingConvention = CallingConvention::Cdecl)]
	static bool NativeCallback_GetItemInHand(const char* playerName, ItemInHandData* outData);
};
