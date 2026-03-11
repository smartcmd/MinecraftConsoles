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

bool NativePlayerCallbacks::GetAllowFlight(String^ playerName)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		return NativeCallback_GetAllowFlight(namePtr) != 0;
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SetAllowFlight(String^ playerName, bool flight)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetAllowFlight(namePtr, flight ? 1 : 0);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

float NativePlayerCallbacks::GetExhaustion(String^ playerName)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		return NativeCallback_GetExhaustion(namePtr);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SetExhaustion(String^ playerName, float value)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetExhaustion(namePtr, value);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

float NativePlayerCallbacks::GetSaturation(String^ playerName)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		return NativeCallback_GetSaturation(namePtr);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SetSaturation(String^ playerName, float value)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetSaturation(namePtr, value);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::GiveExp(String^ playerName, int amount)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_GiveExp(namePtr, amount);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::GiveExpLevels(String^ playerName, int amount)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_GiveExpLevels(namePtr, amount);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

int NativePlayerCallbacks::GetTotalExperience(String^ playerName)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		return NativeCallback_GetTotalExperience(namePtr);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

bool NativePlayerCallbacks::IsFlying(String^ playerName)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		return NativeCallback_IsFlying(namePtr) != 0;
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SetFlying(String^ playerName, bool value)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetFlying(namePtr, value ? 1 : 0);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SetExp(String^ playerName, float exp)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetExp(namePtr, exp);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SetPlayerLevel(String^ playerName, int level)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetPlayerLevel(namePtr, level);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SetWalkSpeed(String^ playerName, float value)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetWalkSpeed(namePtr, value);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

float NativePlayerCallbacks::GetWalkSpeed(String^ playerName)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		return NativeCallback_GetWalkSpeed(namePtr);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

bool NativePlayerCallbacks::GetItemInHand(String^ playerName, ItemInHandData* outData)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		return NativeCallback_GetItemInHand(namePtr, outData);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SetItemInHand(String^ playerName, int itemId, int count, int data)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetItemInHand(namePtr, itemId, count, data);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

bool NativePlayerCallbacks::IsSleepingPlayer(String^ playerName)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		return NativeCallback_IsSleepingPlayer(namePtr) != 0;
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

int NativePlayerCallbacks::GetGameMode(String^ playerName)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		return NativeCallback_GetGameMode(namePtr);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}

void NativePlayerCallbacks::SetPlayerGameMode(String^ playerName, int mode)
{
	const char* namePtr = (const char*)(Marshal::StringToHGlobalAnsi(playerName)).ToPointer();
	try
	{
		NativeCallback_SetPlayerGameMode(namePtr, mode);
	}
	finally
	{
		Marshal::FreeHGlobal(IntPtr((void*)namePtr));
	}
}
