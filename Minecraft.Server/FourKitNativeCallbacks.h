#pragma once
// todo: UUUUGH

#include <map>
#include <string>
#include "..\Minecraft.Server.FourKit\FourKitInterop.h"

class ServerPlayer;

namespace FourKit
{
	extern std::map<std::string, ServerPlayer*> g_nativePlayerMap;

#define PB_DECLARE_NATIVE_CALLBACK(Name, Ret, Sig) Ret NativeCallback_##Name Sig;
	PB_NATIVE_CALLBACK_LIST(PB_DECLARE_NATIVE_CALLBACK)
#undef PB_DECLARE_NATIVE_CALLBACK
	
	void RegisterNativePlayer(const char* playerName, ServerPlayer* player);
	void UnregisterNativePlayer(const char* playerName);
}
