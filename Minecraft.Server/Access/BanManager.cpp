#include "stdafx.h"

#include "BanManager.h"

#include "..\Common\AccessStorageUtils.h"
#include "..\Common\FileUtils.h"
#include "..\Common\NetworkUtils.h"
#include "..\Common\StringUtils.h"
#include "..\ServerLogger.h"
#include "..\vendor\nlohmann\json.hpp"

#include <algorithm>
#include <stdio.h>

namespace ServerRuntime
{
	namespace Access
	{
		using OrderedJson = nlohmann::ordered_json;

		namespace
		{
			static const char *kBannedPlayersFileName = "banned-players.json";
			static const char *kBannedIpsFileName = "banned-ips.json";

			static bool TryParseUtcTimestamp(const std::string &text, unsigned long long *outFileTime)
			{
				if (outFileTime == nullptr)
				{
					return false;
				}

				std::string trimmed = StringUtils::TrimAscii(text);
				if (trimmed.empty())
				{
					return false;
				}

				unsigned year = 0;
				unsigned month = 0;
				unsigned day = 0;
				unsigned hour = 0;
				unsigned minute = 0;
				unsigned second = 0;
				if (sscanf_s(trimmed.c_str(), "%4u-%2u-%2uT%2u:%2u:%2uZ", &year, &month, &day, &hour, &minute, &second) != 6)
				{
					return false;
				}

				SYSTEMTIME utc = {};
				utc.wYear = (WORD)year;
				utc.wMonth = (WORD)month;
				utc.wDay = (WORD)day;
				utc.wHour = (WORD)hour;
				utc.wMinute = (WORD)minute;
				utc.wSecond = (WORD)second;

				FILETIME fileTime = {};
				if (!SystemTimeToFileTime(&utc, &fileTime))
				{
					return false;
				}

				ULARGE_INTEGER value = {};
				value.LowPart = fileTime.dwLowDateTime;
				value.HighPart = fileTime.dwHighDateTime;
				*outFileTime = value.QuadPart;
				return true;
			}

			static bool IsMetadataExpired(const BanMetadata &metadata, unsigned long long nowFileTime)
			{
				if (metadata.expires.empty())
				{
					return false;
				}

				unsigned long long expiresFileTime = 0;
				if (!TryParseUtcTimestamp(metadata.expires, &expiresFileTime))
				{
					// Keep malformed metadata active instead of silently unbanning a player or address.
					return false;
				}

				return expiresFileTime <= nowFileTime;
			}
		}
		BanManager::BanManager(const std::string &baseDirectory)
			: m_baseDirectory(baseDirectory.empty() ? "." : baseDirectory)
		{
		}

		bool BanManager::EnsureBanFilesExist() const
		{
			const std::string playersPath = GetBannedPlayersFilePath();
			const std::string ipsPath = GetBannedIpsFilePath();

			const bool playersOk = AccessStorageUtils::EnsureJsonListFileExists(playersPath);
			const bool ipsOk = AccessStorageUtils::EnsureJsonListFileExists(ipsPath);
			if (!playersOk)
			{
				LogErrorf("access", "failed to create %s", playersPath.c_str());
			}
			if (!ipsOk)
			{
				LogErrorf("access", "failed to create %s", ipsPath.c_str());
			}
			return playersOk && ipsOk;
		}

		bool BanManager::Reload()
		{
			std::vector<BannedPlayerEntry> players;
			std::vector<BannedIpEntry> ips;

			if (!LoadPlayers(&players))
			{
				return false;
			}
			if (!LoadIps(&ips))
			{
				return false;
			}

			m_bannedPlayers.swap(players);
			m_bannedIps.swap(ips);
			return true;
		}

		bool BanManager::Save() const
		{
			std::vector<BannedPlayerEntry> players;
			std::vector<BannedIpEntry> ips;
			return SnapshotBannedPlayers(&players) &&
				SnapshotBannedIps(&ips) &&
				SavePlayers(players) &&
				SaveIps(ips);
		}
		bool BanManager::LoadPlayers(std::vector<BannedPlayerEntry> *outEntries) const
		{
			if (outEntries == nullptr)
			{
				return false;
			}
			outEntries->clear();

			std::string text;
			const std::string path = GetBannedPlayersFilePath();
			if (!FileUtils::ReadTextFile(path, &text))
			{
				LogErrorf("access", "failed to read %s", path.c_str());
				return false;
			}
			if (text.empty())
			{
				text = "[]";
			}

			OrderedJson root;
			try
			{
				// Strip an optional UTF-8 BOM because some editors prepend it when rewriting JSON files.
				root = OrderedJson::parse(StringUtils::StripUtf8Bom(text));
			}
			catch (const nlohmann::json::exception &e)
			{
				LogErrorf("access", "failed to parse %s: %s", path.c_str(), e.what());
				return false;
			}

			if (!root.is_array())
			{
				LogErrorf("access", "failed to parse %s: root json value is not an array", path.c_str());
				return false;
			}

			const unsigned long long nowFileTime = FileUtils::GetCurrentUtcFileTime();
			for (const auto &object : root)
			{
				if (!object.is_object())
				{
					LogWarnf("access", "skipping banned player entry that is not an object in %s", path.c_str());
					continue;
				}

				std::string rawXuid;
				if (!AccessStorageUtils::TryGetStringField(object, "xuid", &rawXuid))
				{
					LogWarnf("access", "skipping banned player entry without xuid in %s", path.c_str());
					continue;
				}

				BannedPlayerEntry entry;
				entry.xuid = AccessStorageUtils::NormalizeXuid(rawXuid);
				if (entry.xuid.empty())
				{
					LogWarnf("access", "skipping banned player entry with empty xuid in %s", path.c_str());
					continue;
				}

				AccessStorageUtils::TryGetStringField(object, "name", &entry.name);
				AccessStorageUtils::TryGetStringField(object, "created", &entry.metadata.created);
				AccessStorageUtils::TryGetStringField(object, "source", &entry.metadata.source);
				AccessStorageUtils::TryGetStringField(object, "expires", &entry.metadata.expires);
				AccessStorageUtils::TryGetStringField(object, "reason", &entry.metadata.reason);
				NormalizeMetadata(&entry.metadata);

				// Ignore entries that already expired before reload so the in-memory cache starts from the active set.
				if (IsMetadataExpired(entry.metadata, nowFileTime))
				{
					continue;
				}

				outEntries->push_back(entry);
			}

			return true;
		}
		bool BanManager::LoadIps(std::vector<BannedIpEntry> *outEntries) const
		{
			if (outEntries == nullptr)
			{
				return false;
			}
			outEntries->clear();

			std::string text;
			const std::string path = GetBannedIpsFilePath();
			if (!FileUtils::ReadTextFile(path, &text))
			{
				LogErrorf("access", "failed to read %s", path.c_str());
				return false;
			}
			if (text.empty())
			{
				text = "[]";
			}

			OrderedJson root;
			try
			{
				// Strip an optional UTF-8 BOM because some editors prepend it when rewriting JSON files.
				root = OrderedJson::parse(StringUtils::StripUtf8Bom(text));
			}
			catch (const nlohmann::json::exception &e)
			{
				LogErrorf("access", "failed to parse %s: %s", path.c_str(), e.what());
				return false;
			}

			if (!root.is_array())
			{
				LogErrorf("access", "failed to parse %s: root json value is not an array", path.c_str());
				return false;
			}

			const unsigned long long nowFileTime = FileUtils::GetCurrentUtcFileTime();
			for (const auto &object : root)
			{
				if (!object.is_object())
				{
					LogWarnf("access", "skipping banned ip entry that is not an object in %s", path.c_str());
					continue;
				}

				std::string rawIp;
				if (!AccessStorageUtils::TryGetStringField(object, "ip", &rawIp))
				{
					LogWarnf("access", "skipping banned ip entry without ip in %s", path.c_str());
					continue;
				}

				BannedIpEntry entry;
				entry.ip = NetworkUtils::NormalizeIpToken(rawIp);
				if (entry.ip.empty())
				{
					LogWarnf("access", "skipping banned ip entry with empty ip in %s", path.c_str());
					continue;
				}

				AccessStorageUtils::TryGetStringField(object, "created", &entry.metadata.created);
				AccessStorageUtils::TryGetStringField(object, "source", &entry.metadata.source);
				AccessStorageUtils::TryGetStringField(object, "expires", &entry.metadata.expires);
				AccessStorageUtils::TryGetStringField(object, "reason", &entry.metadata.reason);
				NormalizeMetadata(&entry.metadata);

				// Ignore entries that already expired before reload so the in-memory cache starts from the active set.
				if (IsMetadataExpired(entry.metadata, nowFileTime))
				{
					continue;
				}

				outEntries->push_back(entry);
			}

			return true;
		}
		bool BanManager::SavePlayers(const std::vector<BannedPlayerEntry> &entries) const
		{
			OrderedJson root = OrderedJson::array();
			for (const auto &entry : entries)
			{
				OrderedJson object = OrderedJson::object();
				object["xuid"] = AccessStorageUtils::NormalizeXuid(entry.xuid);
				object["name"] = entry.name;
				object["created"] = entry.metadata.created;
				object["source"] = entry.metadata.source;
				object["expires"] = entry.metadata.expires;
				object["reason"] = entry.metadata.reason;
				root.push_back(object);
			}

			const std::string path = GetBannedPlayersFilePath();
			const std::string json = root.empty() ? std::string("[]\n") : (root.dump(2) + "\n");
			if (!FileUtils::WriteTextFileAtomic(path, json))
			{
				LogErrorf("access", "failed to write %s", path.c_str());
				return false;
			}
			return true;
		}

		bool BanManager::SaveIps(const std::vector<BannedIpEntry> &entries) const
		{
			OrderedJson root = OrderedJson::array();
			for (const auto &entry : entries)
			{
				OrderedJson object = OrderedJson::object();
				object["ip"] = NetworkUtils::NormalizeIpToken(entry.ip);
				object["created"] = entry.metadata.created;
				object["source"] = entry.metadata.source;
				object["expires"] = entry.metadata.expires;
				object["reason"] = entry.metadata.reason;
				root.push_back(object);
			}

			const std::string path = GetBannedIpsFilePath();
			const std::string json = root.empty() ? std::string("[]\n") : (root.dump(2) + "\n");
			if (!FileUtils::WriteTextFileAtomic(path, json))
			{
				LogErrorf("access", "failed to write %s", path.c_str());
				return false;
			}
			return true;
		}

		const std::vector<BannedPlayerEntry> &BanManager::GetBannedPlayers() const
		{
			return m_bannedPlayers;
		}

		const std::vector<BannedIpEntry> &BanManager::GetBannedIps() const
		{
			return m_bannedIps;
		}

		bool BanManager::SnapshotBannedPlayers(std::vector<BannedPlayerEntry> *outEntries) const
		{
			if (outEntries == nullptr)
			{
				return false;
			}

			outEntries->clear();
			outEntries->reserve(m_bannedPlayers.size());

			const unsigned long long nowFileTime = FileUtils::GetCurrentUtcFileTime();
			for (const auto &entry : m_bannedPlayers)
			{
				if (!IsMetadataExpired(entry.metadata, nowFileTime))
				{
					outEntries->push_back(entry);
				}
			}
			return true;
		}

		bool BanManager::SnapshotBannedIps(std::vector<BannedIpEntry> *outEntries) const
		{
			if (outEntries == nullptr)
			{
				return false;
			}

			outEntries->clear();
			outEntries->reserve(m_bannedIps.size());

			const unsigned long long nowFileTime = FileUtils::GetCurrentUtcFileTime();
			for (const auto &entry : m_bannedIps)
			{
				if (!IsMetadataExpired(entry.metadata, nowFileTime))
				{
					outEntries->push_back(entry);
				}
			}
			return true;
		}
		bool BanManager::IsPlayerBannedByXuid(const std::string &xuid) const
		{
			const std::string normalized = AccessStorageUtils::NormalizeXuid(xuid);
			if (normalized.empty())
			{
				return false;
			}

			const unsigned long long nowFileTime = FileUtils::GetCurrentUtcFileTime();
			return std::any_of(
				m_bannedPlayers.begin(),
				m_bannedPlayers.end(),
				[&normalized, nowFileTime](const BannedPlayerEntry &entry)
				{
					return entry.xuid == normalized && !IsMetadataExpired(entry.metadata, nowFileTime);
				});
		}

		bool BanManager::IsIpBanned(const std::string &ip) const
		{
			const std::string normalized = NetworkUtils::NormalizeIpToken(ip);
			if (normalized.empty())
			{
				return false;
			}

			const unsigned long long nowFileTime = FileUtils::GetCurrentUtcFileTime();
			return std::any_of(
				m_bannedIps.begin(),
				m_bannedIps.end(),
				[&normalized, nowFileTime](const BannedIpEntry &entry)
				{
					return entry.ip == normalized && !IsMetadataExpired(entry.metadata, nowFileTime);
				});
		}

		bool BanManager::AddPlayerBan(const BannedPlayerEntry &entry)
		{
			std::vector<BannedPlayerEntry> updatedEntries;
			if (!SnapshotBannedPlayers(&updatedEntries))
			{
				return false;
			}

			BannedPlayerEntry normalized = entry;
			normalized.xuid = AccessStorageUtils::NormalizeXuid(normalized.xuid);
			NormalizeMetadata(&normalized.metadata);
			if (normalized.xuid.empty())
			{
				return false;
			}

			const auto existing = std::find_if(
				updatedEntries.begin(),
				updatedEntries.end(),
				[&normalized](const BannedPlayerEntry &candidate)
				{
					return candidate.xuid == normalized.xuid;
				});
			if (existing != updatedEntries.end())
			{
				// Update the existing entry in place so the stored list remains unique by canonical XUID.
				*existing = normalized;
				if (!SavePlayers(updatedEntries))
				{
					return false;
				}
				m_bannedPlayers.swap(updatedEntries);
				return true;
			}

			updatedEntries.push_back(normalized);
			if (!SavePlayers(updatedEntries))
			{
				return false;
			}
			m_bannedPlayers.swap(updatedEntries);
			return true;
		}

		bool BanManager::AddIpBan(const BannedIpEntry &entry)
		{
			std::vector<BannedIpEntry> updatedEntries;
			if (!SnapshotBannedIps(&updatedEntries))
			{
				return false;
			}

			BannedIpEntry normalized = entry;
			normalized.ip = NetworkUtils::NormalizeIpToken(normalized.ip);
			NormalizeMetadata(&normalized.metadata);
			if (normalized.ip.empty())
			{
				return false;
			}

			const auto existing = std::find_if(
				updatedEntries.begin(),
				updatedEntries.end(),
				[&normalized](const BannedIpEntry &candidate)
				{
					return candidate.ip == normalized.ip;
				});
			if (existing != updatedEntries.end())
			{
				// Update the existing entry in place so the stored list remains unique by normalized IP.
				*existing = normalized;
				if (!SaveIps(updatedEntries))
				{
					return false;
				}
				m_bannedIps.swap(updatedEntries);
				return true;
			}

			updatedEntries.push_back(normalized);
			if (!SaveIps(updatedEntries))
			{
				return false;
			}
			m_bannedIps.swap(updatedEntries);
			return true;
		}

		bool BanManager::RemovePlayerBanByXuid(const std::string &xuid)
		{
			const std::string normalized = AccessStorageUtils::NormalizeXuid(xuid);
			if (normalized.empty())
			{
				return false;
			}

			std::vector<BannedPlayerEntry> updatedEntries;
			if (!SnapshotBannedPlayers(&updatedEntries))
			{
				return false;
			}

			size_t oldSize = updatedEntries.size();
			updatedEntries.erase(
				std::remove_if(
					updatedEntries.begin(),
					updatedEntries.end(),
					[&normalized](const BannedPlayerEntry &entry) { return entry.xuid == normalized; }),
				updatedEntries.end());

			if (updatedEntries.size() == oldSize)
			{
				return false;
			}
			if (!SavePlayers(updatedEntries))
			{
				return false;
			}
			m_bannedPlayers.swap(updatedEntries);
			return true;
		}


		bool BanManager::RemoveIpBan(const std::string &ip)
		{
			const std::string normalized = NetworkUtils::NormalizeIpToken(ip);
			if (normalized.empty())
			{
				return false;
			}

			std::vector<BannedIpEntry> updatedEntries;
			if (!SnapshotBannedIps(&updatedEntries))
			{
				return false;
			}

			size_t oldSize = updatedEntries.size();
			updatedEntries.erase(
				std::remove_if(
					updatedEntries.begin(),
					updatedEntries.end(),
					[&normalized](const BannedIpEntry &entry) { return entry.ip == normalized; }),
				updatedEntries.end());

			if (updatedEntries.size() == oldSize)
			{
				return false;
			}
			if (!SaveIps(updatedEntries))
			{
				return false;
			}
			m_bannedIps.swap(updatedEntries);
			return true;
		}
		std::string BanManager::GetBannedPlayersFilePath() const
		{
			return BuildPath(kBannedPlayersFileName);
		}

		std::string BanManager::GetBannedIpsFilePath() const
		{
			return BuildPath(kBannedIpsFileName);
		}


		BanMetadata BanManager::BuildDefaultMetadata(const char *source)
		{
			BanMetadata metadata;
			metadata.created = StringUtils::GetCurrentUtcTimestampIso8601();
			metadata.source = (source != nullptr) ? source : "Server";
			metadata.expires = "";
			metadata.reason = "";
			return metadata;
		}


		void BanManager::NormalizeMetadata(BanMetadata *metadata)
		{
			if (metadata == nullptr)
			{
				return;
			}

			metadata->created = StringUtils::TrimAscii(metadata->created);
			metadata->source = StringUtils::TrimAscii(metadata->source);
			metadata->expires = StringUtils::TrimAscii(metadata->expires);
			metadata->reason = StringUtils::TrimAscii(metadata->reason);
		}


		std::string BanManager::BuildPath(const char *fileName) const
		{
			return AccessStorageUtils::BuildPathFromBaseDirectory(m_baseDirectory, fileName);
		}
	}
}
