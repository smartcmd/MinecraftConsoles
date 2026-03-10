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

private:
#define PB_DECLARE_PLAYER_DLLIMPORT(Name, Ret, Sig) \
	[DllImport("Minecraft.Server.FourKit.dll", EntryPoint = "NativeCallback_" #Name, CallingConvention = CallingConvention::Cdecl)] \
	static Ret NativeCallback_##Name Sig;
	PB_PLAYER_CALLBACK_LIST(PB_DECLARE_PLAYER_DLLIMPORT)
#undef PB_DECLARE_PLAYER_DLLIMPORT

	[DllImport("Minecraft.Server.FourKit.dll", EntryPoint = "NativeCallback_GetPlayerNetworkAddress", CallingConvention = CallingConvention::Cdecl)]
	static bool NativeCallback_GetPlayerNetworkAddress(const char* playerName, PlayerNetworkAddressData* outData);
};
