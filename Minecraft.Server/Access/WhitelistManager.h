#pragma once

#include <string>
#include <vector>

namespace ServerRuntime
{
	namespace Access
	{
		/**
		 * Information stored with dedicated-server whitelist entries
		 * ホワイトリスト項目に保存する補助情報
		 */
		struct WhitelistMetadata
		{
			std::string created;
			std::string source;
		};

		struct WhitelistedPlayerEntry
		{
			std::string xuid;
			std::string name;
			WhitelistMetadata metadata;
		};

		/**
		 * Dedicated server whitelist file manager.
		 *
		 * Files:
		 * - whitelist.json
		 *
		 * Stores and normalizes XUID-based allow entries.
		 */
		class WhitelistManager
		{
		public:
			explicit WhitelistManager(const std::string &baseDirectory = ".");

			bool EnsureWhitelistFileExists() const;
			bool Reload();
			bool Save() const;

			bool LoadPlayers(std::vector<WhitelistedPlayerEntry> *outEntries) const;
			bool SavePlayers(const std::vector<WhitelistedPlayerEntry> &entries) const;

			const std::vector<WhitelistedPlayerEntry> &GetWhitelistedPlayers() const;
			bool SnapshotWhitelistedPlayers(std::vector<WhitelistedPlayerEntry> *outEntries) const;

			bool IsPlayerWhitelistedByXuid(const std::string &xuid) const;
			bool AddPlayer(const WhitelistedPlayerEntry &entry);
			bool RemovePlayerByXuid(const std::string &xuid);

			std::string GetWhitelistFilePath() const;

			static WhitelistMetadata BuildDefaultMetadata(const char *source = "Server");

		private:
			static std::string NormalizeXuid(const std::string &xuid);
			static void NormalizeMetadata(WhitelistMetadata *metadata);

			std::string BuildPath(const char *fileName) const;

		private:
			std::string m_baseDirectory;
			std::vector<WhitelistedPlayerEntry> m_whitelistedPlayers;
		};
	}
}
