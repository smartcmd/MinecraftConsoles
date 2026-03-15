#pragma once
#include "FourKitInterop.h"

using namespace System::Runtime::InteropServices;

private ref class NativeWorldCallbacks
{
public:
	static bool GetWorldInfo(int dimension, WorldInfoData* outData)
	{
		return NativeCallback_GetWorldInfo(dimension, outData);
	}

	static bool CreateExplosion(int dimension, double x, double y, double z, float power, bool setFire, bool breakBlocks)
	{
		return NativeCallback_CreateExplosion(dimension, x, y, z, power, setFire ? 1 : 0, breakBlocks ? 1 : 0) != 0;
	}

	static bool DropItem(int dimension, double x, double y, double z, int itemId, int count, int data, bool naturalOffset, DroppedItemData* outData)
	{
		return NativeCallback_DropItem(dimension, x, y, z, itemId, count, data, naturalOffset ? 1 : 0, outData);
	}

	static int GetHighestBlockYAt(int dimension, int x, int z)
	{
		return NativeCallback_GetHighestBlockYAt(dimension, x, z);
	}

	static void SetFullTime(int dimension, long long time)
	{
		NativeCallback_SetFullTime(dimension, time);
	}

	static bool SetSpawnLocation(int dimension, int x, int y, int z)
	{
		return NativeCallback_SetSpawnLocation(dimension, x, y, z) != 0;
	}

	static void SetStorm(int dimension, bool hasStorm)
	{
		NativeCallback_SetStorm(dimension, hasStorm ? 1 : 0);
	}

	static void SetThunderDuration(int dimension, int duration)
	{
		NativeCallback_SetThunderDuration(dimension, duration);
	}

	static void SetThundering(int dimension, bool thundering)
	{
		NativeCallback_SetThundering(dimension, thundering ? 1 : 0);
	}

	static void SetTime(int dimension, long long time)
	{
		NativeCallback_SetTime(dimension, time);
	}

	static void SetWeatherDuration(int dimension, int duration)
	{
		NativeCallback_SetWeatherDuration(dimension, duration);
	}

	static bool StrikeLightning(int dimension, double x, double y, double z, bool effectOnly)
	{
		return NativeCallback_StrikeLightning(dimension, x, y, z, effectOnly ? 1 : 0) != 0;
	}

private:
#define PB_DECLARE_WORLD_DLLIMPORT(Name, Ret, Sig) \
	[DllImport("Minecraft.Server.FourKit.dll", EntryPoint = "NativeCallback_" #Name, CallingConvention = CallingConvention::Cdecl)] \
	static Ret NativeCallback_##Name Sig;
	PB_WORLD_CALLBACK_LIST(PB_DECLARE_WORLD_DLLIMPORT)
#undef PB_DECLARE_WORLD_DLLIMPORT
};