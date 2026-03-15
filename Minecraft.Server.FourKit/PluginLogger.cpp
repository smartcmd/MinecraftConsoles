#include "PluginLogger.h"

using namespace System;

String^ PluginLogger::GetTimestamp()
{
	DateTime now = DateTime::Now;
	return now.ToString("yyyy-MM-dd HH:mm:ss.fff");
}

String^ PluginLogger::GetLevelString(LogLevel level)
{
	switch (level)
	{
	case LogLevel::Debug:
		return "DEBUG";
	case LogLevel::Info:
		return "INFO";
	case LogLevel::Warn:
		return "WARN";
	case LogLevel::Error:
		return "ERROR";
	default:
		return "INFO";
	}
}

ConsoleColor PluginLogger::GetLevelColor(LogLevel level)
{
	switch (level)
	{
	case LogLevel::Debug:
		return ConsoleColor::Cyan;
	case LogLevel::Info:
		return ConsoleColor::White;
	case LogLevel::Warn:
		return ConsoleColor::Yellow;
	case LogLevel::Error:
		return ConsoleColor::Red;
	default:
		return ConsoleColor::White;
	}
}

void PluginLogger::WriteLog(LogLevel level, String^ category, String^ message)
{
	if (String::IsNullOrEmpty(category))
		category = "plugin";
	if (String::IsNullOrEmpty(message))
		message = "";

	String^ timestamp = GetTimestamp();
	String^ levelStr = GetLevelString(level);
	
	// [2026-03-08 15:53:45.918][INFO][category] message
	String^ logLine = String::Format("[{0}][{1}][{2}] {3}",
		timestamp, levelStr, category, message);

	ConsoleColor originalColor = Console::ForegroundColor;
	try
	{
		Console::ForegroundColor = GetLevelColor(level);
		Console::WriteLine(logLine);
	}
	finally
	{
		Console::ForegroundColor = originalColor;
	}
}

void PluginLogger::LogInfo(String^ category, String^ message)
{
	WriteLog(LogLevel::Info, category, message);
}

void PluginLogger::LogWarn(String^ category, String^ message)
{
	WriteLog(LogLevel::Warn, category, message);
}

void PluginLogger::LogError(String^ category, String^ message)
{
	WriteLog(LogLevel::Error, category, message);
}

void PluginLogger::LogDebug(String^ category, String^ message)
{
	WriteLog(LogLevel::Debug, category, message);
}

void PluginLogger::LogPluginInfo(String^ pluginName, String^ message)
{
	LogInfo("plugin/" + pluginName, message);
}

void PluginLogger::LogPluginWarn(String^ pluginName, String^ message)
{
	LogWarn("plugin/" + pluginName, message);
}

void PluginLogger::LogPluginError(String^ pluginName, String^ message)
{
	LogError("plugin/" + pluginName, message);
}
