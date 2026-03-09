#include "NativePlayerCallbacks.h"

using namespace System;
using namespace System::Runtime::InteropServices;

void NativePlayerCallbacks::SetFallDistance(String^ playerName, float distance)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetFallDistance(namePtr, distance);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SetHealth(String^ playerName, float health)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetHealth(namePtr, health);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SetFood(String^ playerName, int food)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetFood(namePtr, food);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SendMessage(String^ playerName, String^ message)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	const char* msgPtr = (const char*)(Marshal::StringToHGlobalAnsi(message)).ToPointer();
	try
	{
		NativeCallback_SendMessage(namePtr, msgPtr);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
		Marshal::FreeHGlobal(IntPtr((void*)msgPtr));
	}
}

void NativePlayerCallbacks::TeleportTo(String^ playerName, double x, double y, double z)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_TeleportTo(namePtr, x, y, z);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::Kick(String^ playerName, String^ reason)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	const char* reasonPtr = (const char*)(Marshal::StringToHGlobalAnsi(reason)).ToPointer();
	try
	{
		NativeCallback_Kick(namePtr, reasonPtr);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
		Marshal::FreeHGlobal(IntPtr((void*)reasonPtr));
	}
}

bool NativePlayerCallbacks::IsSneaking(String^ playerName)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		return NativeCallback_IsSneaking(namePtr) != 0;
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SetSneaking(String^ playerName, bool sneaking)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetSneaking(namePtr, sneaking ? 1 : 0);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

bool NativePlayerCallbacks::IsSprinting(String^ playerName)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		return NativeCallback_IsSprinting(namePtr) != 0;
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SetSprinting(String^ playerName, bool sprinting)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetSprinting(namePtr, sprinting ? 1 : 0);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

bool NativePlayerCallbacks::GetPlayerNetworkAddress(String^ playerName, PlayerNetworkAddressData* outData)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		return NativeCallback_GetPlayerNetworkAddress(namePtr, outData);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}
