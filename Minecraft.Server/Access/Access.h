#pragma once

#include "BanManager.h"

namespace ServerRuntime
{
	/**
	 * A frontend that will be general-purpose, assuming the implementation of whitelists and ops in the future.
	 */
	namespace Access
	{
		bool Initialize(const std::string &baseDirectory = ".");
		void Shutdown();
		bool Reload();
		bool IsInitialized();

		bool IsPlayerBanned(PlayerUID xuid);
		bool IsIpBanned(const std::string &ip);

		bool AddPlayerBan(PlayerUID xuid, const std::string &name, const BanMetadata &metadata);
		bool AddIpBan(const std::string &ip, const BanMetadata &metadata);
		bool RemovePlayerBan(PlayerUID xuid);
		bool RemoveIpBan(const std::string &ip);

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

		std::string FormatXuid(PlayerUID xuid);
	}
}
