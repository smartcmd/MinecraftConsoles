#pragma once

#include "FileUtils.h"
#include "StringUtils.h"

#include "..\vendor\nlohmann\json.hpp"

#include <stdio.h>

namespace ServerRuntime
{
	namespace AccessStorageUtils
	{
		inline bool IsRegularFile(const std::string &path)
		{
			const std::wstring widePath = StringUtils::Utf8ToWide(path);
			if (widePath.empty())
			{
				return false;
			}

			const DWORD attributes = GetFileAttributesW(widePath.c_str());
			return (attributes != INVALID_FILE_ATTRIBUTES) && ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0);
		}

		inline bool EnsureJsonListFileExists(const std::string &path)
		{
			return IsRegularFile(path) || FileUtils::WriteTextFileAtomic(path, "[]\n");
		}

		inline bool TryGetStringField(const nlohmann::ordered_json &object, const char *key, std::string *outValue)
		{
			if (key == nullptr || outValue == nullptr || !object.is_object())
			{
				return false;
			}

			const auto it = object.find(key);
			if (it == object.end() || !it->is_string())
			{
				return false;
			}

			*outValue = it->get<std::string>();
			return true;
		}

		inline std::string NormalizeXuid(const std::string &xuid)
		{
			const std::string trimmed = StringUtils::TrimAscii(xuid);
			if (trimmed.empty())
			{
				return "";
			}

			unsigned long long numericXuid = 0;
			if (StringUtils::TryParseUnsignedLongLong(trimmed, &numericXuid))
			{
				if (numericXuid == 0ULL)
				{
					return "";
				}

				char buffer[32] = {};
				sprintf_s(buffer, sizeof(buffer), "0x%016llx", numericXuid);
				return buffer;
			}

			return StringUtils::ToLowerAscii(trimmed);
		}

		inline std::string BuildPathFromBaseDirectory(const std::string &baseDirectory, const char *fileName)
		{
			if (fileName == nullptr || fileName[0] == 0)
			{
				return "";
			}

			const std::wstring wideFileName = StringUtils::Utf8ToWide(fileName);
			if (wideFileName.empty())
			{
				return "";
			}

			if (baseDirectory.empty() || baseDirectory == ".")
			{
				return StringUtils::WideToUtf8(wideFileName);
			}

			const std::wstring wideBaseDirectory = StringUtils::Utf8ToWide(baseDirectory);
			if (wideBaseDirectory.empty())
			{
				return StringUtils::WideToUtf8(wideFileName);
			}

			const wchar_t last = wideBaseDirectory[wideBaseDirectory.size() - 1];
			if (last == L'\\' || last == L'/')
			{
				return StringUtils::WideToUtf8(wideBaseDirectory + wideFileName);
			}

			return StringUtils::WideToUtf8(wideBaseDirectory + L"\\" + wideFileName);
		}
	}
}
