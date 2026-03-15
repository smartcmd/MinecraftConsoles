#pragma once
// todo: improve just like nativeplayercallbacks
#include "FourKitInterop.h"

using namespace System;
using namespace System::Runtime::InteropServices;

private ref class NativeBlockCallbacks
{
public:
	static void BreakNaturally(int x, int y, int z, int dimension);
	static int GetBlockType(int x, int y, int z, int dimension);
	static void SetBlockType(int x, int y, int z, int dimension, int id);
	static int GetBlockData(int x, int y, int z, int dimension);
	static void SetBlockData(int x, int y, int z, int dimension, int data);

private:
#define PB_DECLARE_BLOCK_DLLIMPORT(Name, Ret, Sig) \
	[DllImport("Minecraft.Server.FourKit.dll", EntryPoint = "NativeCallback_" #Name, CallingConvention = CallingConvention::Cdecl)] \
	static Ret NativeCallback_##Name Sig;
	PB_BLOCK_CALLBACK_LIST(PB_DECLARE_BLOCK_DLLIMPORT)
#undef PB_DECLARE_BLOCK_DLLIMPORT
};
