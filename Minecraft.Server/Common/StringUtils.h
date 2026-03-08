#pragma once

#include <string>

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
		bool StartsWithIgnoreCase(const std::string &value, const std::string &prefix);
	}
}

