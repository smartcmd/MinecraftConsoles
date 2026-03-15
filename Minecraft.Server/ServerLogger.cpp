#include "stdafx.h"

#include "ServerLogger.h"
#include "Common\\StringUtils.h"
#include "vendor\\linenoise\\linenoise.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

namespace ServerRuntime
{
static volatile LONG g_minLogLevel = (LONG)eServerLogLevel_Info;

static const char *NormalizeCategory(const char *category)
{
	if (category == NULL || category[0] == 0)
	{
		return "server";
	}
	return category;
}

static const char *LogLevelToString(EServerLogLevel level)
{
	switch (level)
	{
	case eServerLogLevel_Debug:
		return "DEBUG";
	case eServerLogLevel_Info:
		return "INFO";
	case eServerLogLevel_Warn:
		return "WARN";
	case eServerLogLevel_Error:
		return "ERROR";
	default:
		return "INFO";
	}
}

static WORD LogLevelToColor(EServerLogLevel level)
{
	switch (level)
	{
	case eServerLogLevel_Debug:
		return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	case eServerLogLevel_Warn:
		return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	case eServerLogLevel_Error:
		return FOREGROUND_RED | FOREGROUND_INTENSITY;
	case eServerLogLevel_Info:
	default:
		return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	}
}

static void BuildTimestamp(char *buffer, size_t bufferSize)
{
	if (buffer == NULL || bufferSize == 0)
	{
		return;
	}

	SYSTEMTIME localTime;
	GetLocalTime(&localTime);
	sprintf_s(
		buffer,
		bufferSize,
		"%04u-%02u-%02u %02u:%02u:%02u.%03u",
		(unsigned)localTime.wYear,
		(unsigned)localTime.wMonth,
		(unsigned)localTime.wDay,
		(unsigned)localTime.wHour,
		(unsigned)localTime.wMinute,
		(unsigned)localTime.wSecond,
		(unsigned)localTime.wMilliseconds);
}

static bool ShouldLog(EServerLogLevel level)
{
	return ((LONG)level >= g_minLogLevel);
}

static void WriteLogLine(EServerLogLevel level, const char *category, const char *message)
{
	if (!ShouldLog(level))
	{
		return;
	}

	linenoiseExternalWriteBegin();

	const char *safeCategory = NormalizeCategory(category);
	const char *safeMessage = (message != NULL) ? message : "";

	char timestamp[32] = {};
	BuildTimestamp(timestamp, sizeof(timestamp));

	HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO originalInfo;
	bool hasColorConsole = false;
	if (stdoutHandle != INVALID_HANDLE_VALUE && stdoutHandle != NULL)
	{
		if (GetConsoleScreenBufferInfo(stdoutHandle, &originalInfo))
		{
			hasColorConsole = true;
			SetConsoleTextAttribute(stdoutHandle, LogLevelToColor(level));
		}
	}

	printf(
		"[%s][%s][%s] %s\n",
		timestamp,
		LogLevelToString(level),
		safeCategory,
		safeMessage);
	fflush(stdout);

	if (hasColorConsole)
	{
		SetConsoleTextAttribute(stdoutHandle, originalInfo.wAttributes);
	}

	linenoiseExternalWriteEnd();
}

static void WriteLogLineV(EServerLogLevel level, const char *category, const char *format, va_list args)
{
	char messageBuffer[2048] = {};
	if (format == NULL)
	{
		WriteLogLine(level, category, "");
		return;
	}

	vsnprintf_s(messageBuffer, sizeof(messageBuffer), _TRUNCATE, format, args);
	WriteLogLine(level, category, messageBuffer);
}

bool TryParseServerLogLevel(const char *value, EServerLogLevel *outLevel)
{
	if (value == NULL || outLevel == NULL)
	{
		return false;
	}

	if (_stricmp(value, "debug") == 0)
	{
		*outLevel = eServerLogLevel_Debug;
		return true;
	}
	if (_stricmp(value, "info") == 0)
	{
		*outLevel = eServerLogLevel_Info;
		return true;
	}
	if (_stricmp(value, "warn") == 0 || _stricmp(value, "warning") == 0)
	{
		*outLevel = eServerLogLevel_Warn;
		return true;
	}
	if (_stricmp(value, "error") == 0)
	{
		*outLevel = eServerLogLevel_Error;
		return true;
	}

	return false;
}

void SetServerLogLevel(EServerLogLevel level)
{
	if (level < eServerLogLevel_Debug)
	{
		level = eServerLogLevel_Debug;
	}
	else if (level > eServerLogLevel_Error)
	{
		level = eServerLogLevel_Error;
	}

	g_minLogLevel = (LONG)level;
}

EServerLogLevel GetServerLogLevel()
{
	return (EServerLogLevel)g_minLogLevel;
}

void LogDebug(const char *category, const char *message)
{
	WriteLogLine(eServerLogLevel_Debug, category, message);
}

void LogInfo(const char *category, const char *message)
{
	WriteLogLine(eServerLogLevel_Info, category, message);
}

void LogWarn(const char *category, const char *message)
{
	WriteLogLine(eServerLogLevel_Warn, category, message);
}

void LogError(const char *category, const char *message)
{
	WriteLogLine(eServerLogLevel_Error, category, message);
}

void LogDebugf(const char *category, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	WriteLogLineV(eServerLogLevel_Debug, category, format, args);
	va_end(args);
}

void LogInfof(const char *category, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	WriteLogLineV(eServerLogLevel_Info, category, format, args);
	va_end(args);
}

void LogWarnf(const char *category, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	WriteLogLineV(eServerLogLevel_Warn, category, format, args);
	va_end(args);
}

void LogErrorf(const char *category, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	WriteLogLineV(eServerLogLevel_Error, category, format, args);
	va_end(args);
}

void LogStartupStep(const char *message)
{
	LogInfo("startup", message);
}

void LogWorldIO(const char *message)
{
	LogInfo("world-io", message);
}

void LogWorldName(const char *prefix, const std::wstring &name)
{
	std::string utf8 = StringUtils::WideToUtf8(name);
	LogInfof("world-io", "%s: %s", (prefix != NULL) ? prefix : "name", utf8.c_str());
}
}

