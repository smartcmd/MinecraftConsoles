#pragma once

#include <string>
#include <vector>

namespace ServerRuntime
{
	namespace StringUtils
	{
		std::string WideToUtf8(const std::wstring &value);
		std::wstring Utf8ToWide(const char *value);
		std::wstring Utf8ToWide(const std::string &value);
		std::string StripUtf8Bom(const std::string &value);

		std::string TrimAscii(const std::string &value);
		std::string ToLowerAscii(const std::string &value);
		std::string JoinTokens(const std::vector<std::string> &tokens, size_t startIndex = 0, const char *separator = " ");
		bool StartsWithIgnoreCase(const std::string &value, const std::string &prefix);
		bool TryParseUnsignedLongLong(const std::string &value, unsigned long long *outValue);
		std::string GetCurrentUtcTimestampIso8601();
	}
}

