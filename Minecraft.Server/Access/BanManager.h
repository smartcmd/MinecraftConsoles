#pragma once

#include <string>
#include <vector>

namespace ServerRuntime
{
	namespace Access
	{
		/**
		 * Information shared with player bans and IP bans
		 * プレイヤーBANとIP BANで共有する情報
		 */
		struct BanMetadata
		{
			std::string created;
			std::string source;
			std::string expires;
			std::string reason;
		};

		struct BannedPlayerEntry
		{
			std::string xuid;
			std::string name;
			BanMetadata metadata;
		};

		struct BannedIpEntry
		{
			std::string ip;
			BanMetadata metadata;
		};

		/**
		 * Dedicated server BAN file manager.
		 *
		 * Files:
		 * - banned-players.json
		 * - banned-ips.json
		 *
		 * This class only handles storage/caching.
		 * Connection-time hooks are wired separately.
		 */
		class BanManager
		{
		public:
			/**
			 * **Create Ban Manager**
			 *
			 * Binds the manager to the directory that stores the dedicated-server access files
			 * Dedicated Server のアクセスファイル配置先を設定する
			 */
			explicit BanManager(const std::string &baseDirectory = ".");

			/**
			 * Creates empty JSON array files when the dedicated server starts without persisted access data
			 * BANファイルが無い初回起動時に空JSONを用意する
			 */
			bool EnsureBanFilesExist() const;
			bool Reload();
			bool Save() const;

			bool LoadPlayers(std::vector<BannedPlayerEntry> *outEntries) const;
			bool LoadIps(std::vector<BannedIpEntry> *outEntries) const;
			bool SavePlayers(const std::vector<BannedPlayerEntry> &entries) const;
			bool SaveIps(const std::vector<BannedIpEntry> &entries) const;

			const std::vector<BannedPlayerEntry> &GetBannedPlayers() const;
			const std::vector<BannedIpEntry> &GetBannedIps() const;
			/**
			 * Copies only currently active player BAN entries so expired metadata does not leak into command output
			 * 期限切れを除いた有効なプレイヤーBAN一覧を複製取得する
			 */
			bool SnapshotBannedPlayers(std::vector<BannedPlayerEntry> *outEntries) const;
			/**
			 * Copies only currently active IP BAN entries so expired metadata does not leak into command output
			 * 期限切れを除いた有効なIP BAN一覧を複製取得する
			 */
			bool SnapshotBannedIps(std::vector<BannedIpEntry> *outEntries) const;

			bool IsPlayerBannedByXuid(const std::string &xuid) const;
			bool IsIpBanned(const std::string &ip) const;

			bool AddPlayerBan(const BannedPlayerEntry &entry);
			bool AddIpBan(const BannedIpEntry &entry);
			bool RemovePlayerBanByXuid(const std::string &xuid);
			bool RemoveIpBan(const std::string &ip);

			std::string GetBannedPlayersFilePath() const;
			std::string GetBannedIpsFilePath() const;

			static BanMetadata BuildDefaultMetadata(const char *source = "Server");

		private:
			static void NormalizeMetadata(BanMetadata *metadata);

			std::string BuildPath(const char *fileName) const;

		private:
			std::string m_baseDirectory;
			std::vector<BannedPlayerEntry> m_bannedPlayers;
			std::vector<BannedIpEntry> m_bannedIps;
		};
	}
}
