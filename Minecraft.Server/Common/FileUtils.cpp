#include "stdafx.h"

#include "FileUtils.h"

#include "StringUtils.h"

#include <io.h>
#include <stdio.h>

namespace ServerRuntime
{
	namespace FileUtils
	{
		namespace
		{
			static std::wstring ToWidePath(const std::string &filePath)
			{
				return StringUtils::Utf8ToWide(filePath);
			}
		}

		unsigned long long GetCurrentUtcFileTime()
		{
			FILETIME now = {};
			GetSystemTimeAsFileTime(&now);

			ULARGE_INTEGER value = {};
			value.LowPart = now.dwLowDateTime;
			value.HighPart = now.dwHighDateTime;
			return value.QuadPart;
		}

		bool ReadTextFile(const std::string &filePath, std::string *outText)
		{
			if (outText == nullptr)
			{
				return false;
			}

			outText->clear();

			const std::wstring widePath = ToWidePath(filePath);
			if (widePath.empty())
			{
				return false;
			}

			FILE *inFile = nullptr;
			if (_wfopen_s(&inFile, widePath.c_str(), L"rb") != 0 || inFile == nullptr)
			{
				return false;
			}

			if (fseek(inFile, 0, SEEK_END) != 0)
			{
				fclose(inFile);
				return false;
			}

			long fileSize = ftell(inFile);
			if (fileSize < 0)
			{
				fclose(inFile);
				return false;
			}

			if (fseek(inFile, 0, SEEK_SET) != 0)
			{
				fclose(inFile);
				return false;
			}

			if (fileSize == 0)
			{
				fclose(inFile);
				return true;
			}

			outText->resize((size_t)fileSize);
			size_t bytesRead = fread(&(*outText)[0], 1, (size_t)fileSize, inFile);
			fclose(inFile);

			if (bytesRead != (size_t)fileSize)
			{
				outText->clear();
				return false;
			}

			return true;
		}

		bool WriteTextFileAtomic(const std::string &filePath, const std::string &text)
		{
			const std::wstring widePath = ToWidePath(filePath);
			if (widePath.empty())
			{
				return false;
			}

			const std::wstring tmpPath = widePath + L".tmp";

			FILE *outFile = nullptr;
			if (_wfopen_s(&outFile, tmpPath.c_str(), L"wb") != 0 || outFile == nullptr)
			{
				return false;
			}

			if (!text.empty())
			{
				size_t bytesWritten = fwrite(text.data(), 1, text.size(), outFile);
				if (bytesWritten != text.size())
				{
					fclose(outFile);
					DeleteFileW(tmpPath.c_str());
					return false;
				}
			}

			if (fflush(outFile) != 0 || _commit(_fileno(outFile)) != 0)
			{
				fclose(outFile);
				DeleteFileW(tmpPath.c_str());
				return false;
			}
			fclose(outFile);

			DWORD attrs = GetFileAttributesW(widePath.c_str());
			if (attrs != INVALID_FILE_ATTRIBUTES && ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0))
			{
				// Replace the destination without deleting the last known-good file first.
				if (ReplaceFileW(widePath.c_str(), tmpPath.c_str(), nullptr, REPLACEFILE_IGNORE_MERGE_ERRORS, nullptr, nullptr))
				{
					return true;
				}
			}

			if (MoveFileExW(tmpPath.c_str(), widePath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
			{
				return true;
			}

			// Keep the temp file on failure so the original file remains recoverable and the caller can inspect the write result.
			return false;
		}
	}
}