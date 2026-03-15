#include "stdafx.h"

#include "StringUtils.h"

#include <cctype>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

namespace ServerRuntime
{
	namespace StringUtils
	{
		std::string WideToUtf8(const std::wstring &value)
		{
			if (value.empty())
			{
				return std::string();
			}

			int charCount = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), (int)value.length(), NULL, 0, NULL, NULL);
			if (charCount <= 0)
			{
				return std::string();
			}

			std::string utf8;
			utf8.resize(charCount);
			WideCharToMultiByte(CP_UTF8, 0, value.c_str(), (int)value.length(), &utf8[0], charCount, NULL, NULL);
			return utf8;
		}

		std::wstring Utf8ToWide(const char *value)
		{
			if (value == NULL || value[0] == 0)
			{
				return std::wstring();
			}

			int wideCount = MultiByteToWideChar(CP_UTF8, 0, value, -1, NULL, 0);
			if (wideCount <= 0)
			{
				// Fall back to the current ANSI code page so legacy non-UTF-8 inputs remain readable.
				wideCount = MultiByteToWideChar(CP_ACP, 0, value, -1, NULL, 0);
				if (wideCount <= 0)
				{
					return std::wstring();
				}

				std::wstring wide;
				wide.resize(wideCount - 1);
				MultiByteToWideChar(CP_ACP, 0, value, -1, &wide[0], wideCount);
				return wide;
			}

			std::wstring wide;
			wide.resize(wideCount - 1);
			MultiByteToWideChar(CP_UTF8, 0, value, -1, &wide[0], wideCount);
			return wide;
		}

		std::wstring Utf8ToWide(const std::string &value)
		{
			return Utf8ToWide(value.c_str());
		}

		std::string StripUtf8Bom(const std::string &value)
		{
			if (value.size() >= 3 &&
				(unsigned char)value[0] == 0xEF &&
				(unsigned char)value[1] == 0xBB &&
				(unsigned char)value[2] == 0xBF)
			{
				return value.substr(3);
			}

			return value;
		}

		std::string TrimAscii(const std::string &value)
		{
			size_t start = 0;
			while (start < value.length() && std::isspace((unsigned char)value[start]))
			{
				++start;
			}

			size_t end = value.length();
			while (end > start && std::isspace((unsigned char)value[end - 1]))
			{
				--end;
			}

			return value.substr(start, end - start);
		}

		std::string ToLowerAscii(const std::string &value)
		{
			std::string lowered = value;
			for (size_t i = 0; i < lowered.length(); ++i)
			{
				lowered[i] = (char)std::tolower((unsigned char)lowered[i]);
			}
			return lowered;
		}

		std::string JoinTokens(const std::vector<std::string> &tokens, size_t startIndex, const char *separator)
		{
			if (startIndex >= tokens.size())
			{
				return std::string();
			}

			const auto joinSeparator = std::string((separator != nullptr) ? separator : " ");
			size_t totalLength = 0;
			for (size_t i = startIndex; i < tokens.size(); ++i)
			{
				totalLength += tokens[i].size();
			}

			totalLength += (tokens.size() - startIndex - 1) * joinSeparator.size();
			std::string joined;
			joined.reserve(totalLength);
			for (size_t i = startIndex; i < tokens.size(); ++i)
			{
				if (!joined.empty())
				{
					joined += joinSeparator;
				}

				joined += tokens[i];
			}

			return joined;
		}

		bool StartsWithIgnoreCase(const std::string &value, const std::string &prefix)
		{
			if (prefix.size() > value.size())
			{
				return false;
			}

			for (size_t i = 0; i < prefix.size(); ++i)
			{
				unsigned char a = (unsigned char)value[i];
				unsigned char b = (unsigned char)prefix[i];
				if (std::tolower(a) != std::tolower(b))
				{
					return false;
				}
			}

			return true;
		}

		bool TryParseUnsignedLongLong(const std::string &value, unsigned long long *outValue)
		{
			if (outValue == nullptr)
			{
				return false;
			}

			const std::string trimmed = TrimAscii(value);
			if (trimmed.empty())
			{
				return false;
			}

			errno = 0;
			char *end = nullptr;
			const unsigned long long parsed = _strtoui64(trimmed.c_str(), &end, 0);
			if (end == trimmed.c_str() || errno != 0)
			{
				return false;
			}

			while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')
			{
				++end;
			}

			if (*end != 0)
			{
				return false;
			}

			*outValue = parsed;
			return true;
		}

		std::string GetCurrentUtcTimestampIso8601()
		{
			SYSTEMTIME utc = {};
			GetSystemTime(&utc);

			char created[64] = {};
			sprintf_s(
				created,
				sizeof(created),
				"%04u-%02u-%02uT%02u:%02u:%02uZ",
				(unsigned)utc.wYear,
				(unsigned)utc.wMonth,
				(unsigned)utc.wDay,
				(unsigned)utc.wHour,
				(unsigned)utc.wMinute,
				(unsigned)utc.wSecond);
			return created;
		}
	}
}

