#include "stdafx.h"

#include "Access.h"

#include "..\Common\StringUtils.h"
#include "..\ServerLogger.h"

#include <memory>
#include <mutex>

namespace ServerRuntime
{
	namespace Access
	{
		namespace
		{
			/**
			 * **Access State**
			 *
			 * These features are used extensively from various parts of the code, so safe read/write handling is implemented
			 * Stores the published BAN manager snapshot plus a writer gate for clone-and-publish updates
			 * 公開中のBanManagerスナップショットと更新直列化用ロックを保持する
			 */
			struct AccessState
			{
				std::mutex stateLock;
				std::mutex writeLock;
				std::shared_ptr<BanManager> banManager;
				std::shared_ptr<WhitelistManager> whitelistManager;
				bool whitelistEnabled = false;
			};

			AccessState g_accessState;

			/**
			 * Copies the currently published manager pointer so readers can work without holding the publish mutex
			 * 公開中のBanManager共有ポインタを複製取得する
			 */
			static std::shared_ptr<BanManager> GetBanManagerSnapshot()
			{
				std::lock_guard<std::mutex> stateLock(g_accessState.stateLock);
				return g_accessState.banManager;
			}

			/**
			 * Replaces the shared manager pointer with a fully prepared snapshot in one short critical section
			 * 準備完了したBanManagerスナップショットを短いロックで公開する
			 */
			static void PublishBanManagerSnapshot(const std::shared_ptr<BanManager> &banManager)
			{
				std::lock_guard<std::mutex> stateLock(g_accessState.stateLock);
				g_accessState.banManager = banManager;
			}

			static std::shared_ptr<WhitelistManager> GetWhitelistManagerSnapshot()
			{
				std::lock_guard<std::mutex> stateLock(g_accessState.stateLock);
				return g_accessState.whitelistManager;
			}

			static void PublishWhitelistManagerSnapshot(const std::shared_ptr<WhitelistManager> &whitelistManager)
			{
				std::lock_guard<std::mutex> stateLock(g_accessState.stateLock);
				g_accessState.whitelistManager = whitelistManager;
			}
		}

		std::string FormatXuid(PlayerUID xuid)
		{
			if (xuid == INVALID_XUID)
			{
				return "";
			}

			char buffer[32] = {};
			sprintf_s(buffer, sizeof(buffer), "0x%016llx", (unsigned long long)xuid);
			return buffer;
		}

		bool TryParseXuid(const std::string &text, PlayerUID *outXuid)
		{
			if (outXuid == nullptr)
			{
				return false;
			}

			unsigned long long parsed = 0;
			if (!StringUtils::TryParseUnsignedLongLong(text, &parsed) || parsed == 0ULL)
			{
				return false;
			}

			*outXuid = (PlayerUID)parsed;
			return true;
		}

		bool Initialize(const std::string &baseDirectory, bool whitelistEnabled)
		{
			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);

			// Build the replacement manager privately so readers keep using the last published snapshot during disk I/O.
			std::shared_ptr<BanManager> banManager = std::make_shared<BanManager>(baseDirectory);
			std::shared_ptr<WhitelistManager> whitelistManager = std::make_shared<WhitelistManager>(baseDirectory);
			if (!banManager->EnsureBanFilesExist())
			{
				LogError("access", "failed to ensure dedicated server ban files exist");
				return false;
			}
			if (!whitelistManager->EnsureWhitelistFileExists())
			{
				LogError("access", "failed to ensure dedicated server whitelist file exists");
				return false;
			}

			if (!banManager->Reload())
			{
				LogError("access", "failed to load dedicated server ban files");
				return false;
			}
			if (!whitelistManager->Reload())
			{
				LogError("access", "failed to load dedicated server whitelist file");
				return false;
			}

			std::vector<BannedPlayerEntry> playerEntries;
			std::vector<BannedIpEntry> ipEntries;
			std::vector<WhitelistedPlayerEntry> whitelistEntries;
			banManager->SnapshotBannedPlayers(&playerEntries);
			banManager->SnapshotBannedIps(&ipEntries);
			whitelistManager->SnapshotWhitelistedPlayers(&whitelistEntries);
			PublishBanManagerSnapshot(banManager);
			PublishWhitelistManagerSnapshot(whitelistManager);
			{
				std::lock_guard<std::mutex> stateLock(g_accessState.stateLock);
				g_accessState.whitelistEnabled = whitelistEnabled;
			}

			LogInfof(
				"access",
				"loaded %u player bans, %u ip bans, and %u whitelist entries (whitelist=%s)",
				(unsigned)playerEntries.size(),
				(unsigned)ipEntries.size(),
				(unsigned)whitelistEntries.size(),
				whitelistEnabled ? "enabled" : "disabled");
			return true;
		}

		void Shutdown()
		{
			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);
			PublishBanManagerSnapshot(std::shared_ptr<BanManager>{});
			PublishWhitelistManagerSnapshot(std::shared_ptr<WhitelistManager>{});
			std::lock_guard<std::mutex> stateLock(g_accessState.stateLock);
			g_accessState.whitelistEnabled = false;
		}

		bool Reload()
		{
			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);
			std::shared_ptr<BanManager> current = GetBanManagerSnapshot();
			std::shared_ptr<WhitelistManager> currentWhitelist = GetWhitelistManagerSnapshot();
			if (current == nullptr || currentWhitelist == nullptr)
			{
				return false;
			}

			std::shared_ptr<BanManager> banManager = std::make_shared<BanManager>(*current);
			std::shared_ptr<WhitelistManager> whitelistManager = std::make_shared<WhitelistManager>(*currentWhitelist);
			if (!banManager->EnsureBanFilesExist())
			{
				return false;
			}
			if (!whitelistManager->EnsureWhitelistFileExists())
			{
				return false;
			}
			if (!banManager->Reload())
			{
				return false;
			}
			if (!whitelistManager->Reload())
			{
				return false;
			}

			PublishBanManagerSnapshot(banManager);
			PublishWhitelistManagerSnapshot(whitelistManager);
			return true;
		}

		bool ReloadWhitelist()
		{
			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);
			const auto current = GetWhitelistManagerSnapshot();
			if (current == nullptr)
			{
				return false;
			}

			auto whitelistManager = std::make_shared<WhitelistManager>(*current);
			if (!whitelistManager->EnsureWhitelistFileExists())
			{
				return false;
			}
			if (!whitelistManager->Reload())
			{
				return false;
			}

			PublishWhitelistManagerSnapshot(whitelistManager);
			return true;
		}

		bool IsInitialized()
		{
			return GetBanManagerSnapshot() != nullptr && GetWhitelistManagerSnapshot() != nullptr;
		}

		bool IsWhitelistEnabled()
		{
			std::lock_guard<std::mutex> stateLock(g_accessState.stateLock);
			return g_accessState.whitelistEnabled;
		}

		void SetWhitelistEnabled(bool enabled)
		{
			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);
			std::lock_guard<std::mutex> stateLock(g_accessState.stateLock);
			g_accessState.whitelistEnabled = enabled;
		}

		bool IsPlayerBanned(PlayerUID xuid)
		{
			const std::string formatted = FormatXuid(xuid);
			if (formatted.empty())
			{
				return false;
			}

			std::shared_ptr<BanManager> banManager = GetBanManagerSnapshot();
			return (banManager != nullptr) ? banManager->IsPlayerBannedByXuid(formatted) : false;
		}

		bool IsIpBanned(const std::string &ip)
		{
			std::shared_ptr<BanManager> banManager = GetBanManagerSnapshot();
			return (banManager != nullptr) ? banManager->IsIpBanned(ip) : false;
		}

		bool IsPlayerWhitelisted(PlayerUID xuid)
		{
			const std::string formatted = FormatXuid(xuid);
			if (formatted.empty())
			{
				return false;
			}

			std::shared_ptr<WhitelistManager> whitelistManager = GetWhitelistManagerSnapshot();
			return (whitelistManager != nullptr) ? whitelistManager->IsPlayerWhitelistedByXuid(formatted) : false;
		}

		bool AddPlayerBan(PlayerUID xuid, const std::string &name, const BanMetadata &metadata)
		{
			const std::string formatted = FormatXuid(xuid);
			if (formatted.empty())
			{
				return false;
			}

			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);
			std::shared_ptr<BanManager> current = GetBanManagerSnapshot();
			if (current == nullptr)
			{
				return false;
			}

			std::shared_ptr<BanManager> banManager = std::make_shared<BanManager>(*current);
			BannedPlayerEntry entry;
			entry.xuid = formatted;
			entry.name = name;
			entry.metadata = metadata;
			if (!banManager->AddPlayerBan(entry))
			{
				return false;
			}

			PublishBanManagerSnapshot(banManager);
			return true;
		}

		bool AddIpBan(const std::string &ip, const BanMetadata &metadata)
		{
			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);
			std::shared_ptr<BanManager> current = GetBanManagerSnapshot();
			if (current == nullptr)
			{
				return false;
			}

			std::shared_ptr<BanManager> banManager = std::make_shared<BanManager>(*current);
			BannedIpEntry entry;
			entry.ip = ip;
			entry.metadata = metadata;
			if (!banManager->AddIpBan(entry))
			{
				return false;
			}

			PublishBanManagerSnapshot(banManager);
			return true;
		}

		bool RemovePlayerBan(PlayerUID xuid)
		{
			const std::string formatted = FormatXuid(xuid);
			if (formatted.empty())
			{
				return false;
			}

			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);
			std::shared_ptr<BanManager> current = GetBanManagerSnapshot();
			if (current == nullptr)
			{
				return false;
			}

			std::shared_ptr<BanManager> banManager = std::make_shared<BanManager>(*current);
			if (!banManager->RemovePlayerBanByXuid(formatted))
			{
				return false;
			}

			PublishBanManagerSnapshot(banManager);
			return true;
		}

		bool RemoveIpBan(const std::string &ip)
		{
			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);
			std::shared_ptr<BanManager> current = GetBanManagerSnapshot();
			if (current == nullptr)
			{
				return false;
			}

			std::shared_ptr<BanManager> banManager = std::make_shared<BanManager>(*current);
			if (!banManager->RemoveIpBan(ip))
			{
				return false;
			}

			PublishBanManagerSnapshot(banManager);
			return true;
		}

		bool AddWhitelistedPlayer(PlayerUID xuid, const std::string &name, const WhitelistMetadata &metadata)
		{
			const auto formatted = FormatXuid(xuid);
			if (formatted.empty())
			{
				return false;
			}

			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);
			const auto current = GetWhitelistManagerSnapshot();
			if (current == nullptr)
			{
				return false;
			}

			auto whitelistManager = std::make_shared<WhitelistManager>(*current);
			const WhitelistedPlayerEntry entry = { formatted, name, metadata };
			if (!whitelistManager->AddPlayer(entry))
			{
				return false;
			}

			PublishWhitelistManagerSnapshot(whitelistManager);
			return true;
		}

		bool RemoveWhitelistedPlayer(PlayerUID xuid)
		{
			const auto formatted = FormatXuid(xuid);
			if (formatted.empty())
			{
				return false;
			}

			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);
			const auto current = GetWhitelistManagerSnapshot();
			if (current == nullptr)
			{
				return false;
			}

			auto whitelistManager = std::make_shared<WhitelistManager>(*current);
			if (!whitelistManager->RemovePlayerByXuid(formatted))
			{
				return false;
			}

			PublishWhitelistManagerSnapshot(whitelistManager);
			return true;
		}

		bool SnapshotBannedPlayers(std::vector<BannedPlayerEntry> *outEntries)
		{
			if (outEntries == nullptr)
			{
				return false;
			}

			std::shared_ptr<BanManager> banManager = GetBanManagerSnapshot();
			if (banManager == nullptr)
			{
				outEntries->clear();
				return false;
			}

			return banManager->SnapshotBannedPlayers(outEntries);
		}

		bool SnapshotBannedIps(std::vector<BannedIpEntry> *outEntries)
		{
			if (outEntries == nullptr)
			{
				return false;
			}

			std::shared_ptr<BanManager> banManager = GetBanManagerSnapshot();
			if (banManager == nullptr)
			{
				outEntries->clear();
				return false;
			}

			return banManager->SnapshotBannedIps(outEntries);
		}

		bool SnapshotWhitelistedPlayers(std::vector<WhitelistedPlayerEntry> *outEntries)
		{
			if (outEntries == nullptr)
			{
				return false;
			}

			const auto whitelistManager = GetWhitelistManagerSnapshot();
			if (whitelistManager == nullptr)
			{
				outEntries->clear();
				return false;
			}

			return whitelistManager->SnapshotWhitelistedPlayers(outEntries);
		}
	}
}
