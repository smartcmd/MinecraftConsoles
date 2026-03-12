#pragma once

#include "FourKit.h"
#include "FourKitInterop.h"
#include "FourKitStructs.h"
#include "PluginLogger.h"
#include "NativeBlockCallbacks.h"
#include "NativePlayerCallbacks.h"
#include "PluginCommand.h"
#include "World.h"

using namespace System;
using namespace System::Reflection;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

namespace FourKitInternal
{
#define PB_DECLARE_STORAGE_EXTERN(Name, Ret, Sig) extern FourKit##Name##Callback g_##Name;
	PB_NATIVE_CALLBACK_LIST(PB_DECLARE_STORAGE_EXTERN)
#undef PB_DECLARE_STORAGE_EXTERN

	BlockFace ToBlockFace(int face);
	InteractAction ToInteractAction(int action);
	Player^ ResolvePlayerByName(String^ name);
	bool TryGetDimensionFromWorldName(String^ name, int% dimension);
	cli::array<String^>^ ParseCommandArguments(String^ commandLine, String^% outLabel);
	void SetNativeCallbacksExport(PB_SET_NATIVE_CALLBACK_PARAMS);
	int DispatchPlayerCommandExport(const char* playerName, const char* commandLine);
}
