#pragma once
// is there a better way of doing this?
// i dont want to do this over and over

extern "C" __declspec(dllimport) void PluginBridge_Initialize();
extern "C" __declspec(dllimport) void PluginBridge_Shutdown();
extern "C" __declspec(dllimport) void PluginBridge_FireOnLoad();
extern "C" __declspec(dllimport) void PluginBridge_FireOnExit();
