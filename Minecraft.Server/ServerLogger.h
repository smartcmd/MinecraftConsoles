#pragma once

#include <string>

namespace ServerRuntime
{
	enum EServerLogLevel
	{
		eServerLogLevel_Debug = 0,
		eServerLogLevel_Info = 1,
		eServerLogLevel_Warn = 2,
		eServerLogLevel_Error = 3
	};

	/**
	 * **Parse Log Level String**
	 *
	 * Converts a string value into log level (`debug`/`info`/`warn`/`error`)
	 * ログレベル文字列の変換処理
	 *
	 * @param value Source string
	 * @param outLevel Output location for parsed level
	 * @return `true` when conversion succeeds
	 */
	bool TryParseServerLogLevel(const char *value, EServerLogLevel *outLevel);

	void SetServerLogLevel(EServerLogLevel level);
	EServerLogLevel GetServerLogLevel();

	void LogDebug(const char *category, const char *message);
	void LogInfo(const char *category, const char *message);
	void LogWarn(const char *category, const char *message);
	void LogError(const char *category, const char *message);

	/** Emit formatted log output with the specified level and category */
	void LogDebugf(const char *category, const char *format, ...);
	void LogInfof(const char *category, const char *format, ...);
	void LogWarnf(const char *category, const char *format, ...);
	void LogErrorf(const char *category, const char *format, ...);

	void LogStartupStep(const char *message);
	void LogWorldIO(const char *message);
	void LogWorldName(const char *prefix, const std::wstring &name);
}
