#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

private ref class PluginLogger
{
public:
	enum class LogLevel
	{
		Debug,
		Info,
		Warn,
		Error
	};

	static void LogInfo(String^ category, String^ message);
	static void LogWarn(String^ category, String^ message);
	static void LogError(String^ category, String^ message);
	static void LogDebug(String^ category, String^ message);
	
	static void LogPluginInfo(String^ pluginName, String^ message);
	static void LogPluginWarn(String^ pluginName, String^ message);
	static void LogPluginError(String^ pluginName, String^ message);

private:
	static void WriteLog(LogLevel level, String^ category, String^ message);
	static String^ GetTimestamp();
	static String^ GetLevelString(LogLevel level);
	static ConsoleColor GetLevelColor(LogLevel level);
};
