#pragma once

#include <string>

namespace ServerRuntime
{
	namespace FileUtils
	{
		/**
		 * Reads the full UTF-8 path target into memory without interpreting JSON or line endings
		 * UTF-8パスのテキストファイル全体をそのまま読み込む
		 */
		bool ReadTextFile(const std::string &filePath, std::string *outText);
		/**
		 * Writes text through a same-directory temporary file and publishes it with a single replacement step
		 * 同一ディレクトリの一時ファイル経由で安全に書き換える
		 */
		bool WriteTextFileAtomic(const std::string &filePath, const std::string &text);
		/**
		 * Returns the current UTC timestamp encoded in Windows FILETIME units for expiry comparisons
		 * 期限判定用に現在UTC時刻をWindows FILETIME単位で返す
		 */
		unsigned long long GetCurrentUtcFileTime();
	}
}