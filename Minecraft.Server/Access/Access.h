#pragma once

#include "BanManager.h"
#include "WhitelistManager.h"

namespace ServerRuntime
{
	/**
	 * A frontend that will be general-purpose, assuming the implementation of whitelists and ops in the future.
	 */
	namespace Access
	{
		bool Initialize(const std::string &baseDirectory = ".", bool whitelistEnabled = false);
		void Shutdown();
		bool Reload();
		bool ReloadWhitelist();
		bool IsInitialized();
		bool IsWhitelistEnabled();
		void SetWhitelistEnabled(bool enabled);

		bool IsPlayerBanned(PlayerUID xuid);
		bool IsIpBanned(const std::string &ip);
		bool IsPlayerWhitelisted(PlayerUID xuid);

		bool AddPlayerBan(PlayerUID xuid, const std::string &name, const BanMetadata &metadata);
		bool AddIpBan(const std::string &ip, const BanMetadata &metadata);
		bool RemovePlayerBan(PlayerUID xuid);
		bool RemoveIpBan(const std::string &ip);
		bool AddWhitelistedPlayer(PlayerUID xuid, const std::string &name, const WhitelistMetadata &metadata);
		bool RemoveWhitelistedPlayer(PlayerUID xuid);

		/**
		 * Copies the current cached player bans for inspection or command output
		 * 現在のプレイヤーBAN一覧を複製取得
		 */
		bool SnapshotBannedPlayers(std::vector<BannedPlayerEntry> *outEntries);
		/**
		 * Copies the current cached IP bans for inspection or command output
		 * 現在のIP BAN一覧を複製取得
		 */
		bool SnapshotBannedIps(std::vector<BannedIpEntry> *outEntries);
		bool SnapshotWhitelistedPlayers(std::vector<WhitelistedPlayerEntry> *outEntries);

		std::string FormatXuid(PlayerUID xuid);
		bool TryParseXuid(const std::string &text, PlayerUID *outXuid);
	}
}
