#include "stdafx.h"

#include "Access.h"

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

		bool Initialize(const std::string &baseDirectory)
		{
			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);

			// Build the replacement manager privately so readers keep using the last published snapshot during disk I/O.
			std::shared_ptr<BanManager> banManager = std::make_shared<BanManager>(baseDirectory);
			if (!banManager->EnsureBanFilesExist())
			{
				LogError("access", "failed to ensure dedicated server ban files exist");
				return false;
			}

			if (!banManager->Reload())
			{
				LogError("access", "failed to load dedicated server ban files");
				return false;
			}

			std::vector<BannedPlayerEntry> playerEntries;
			std::vector<BannedIpEntry> ipEntries;
			banManager->SnapshotBannedPlayers(&playerEntries);
			banManager->SnapshotBannedIps(&ipEntries);
			PublishBanManagerSnapshot(banManager);

			LogInfof(
				"access",
				"loaded %u player bans and %u ip bans",
				(unsigned)playerEntries.size(),
				(unsigned)ipEntries.size());
			return true;
		}

		void Shutdown()
		{
			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);
			PublishBanManagerSnapshot(std::shared_ptr<BanManager>());
		}

		bool Reload()
		{
			std::lock_guard<std::mutex> writeLock(g_accessState.writeLock);
			std::shared_ptr<BanManager> current = GetBanManagerSnapshot();
			if (current == nullptr)
			{
				return false;
			}

			std::shared_ptr<BanManager> banManager = std::make_shared<BanManager>(*current);
			if (!banManager->EnsureBanFilesExist())
			{
				return false;
			}
			if (!banManager->Reload())
			{
				return false;
			}

			PublishBanManagerSnapshot(banManager);
			return true;
		}

		bool IsInitialized()
		{
			return GetBanManagerSnapshot() != nullptr;
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
	}
}