#include "stdafx.h"

#include "BanManager.h"

#include "..\Common\FileUtils.h"
#include "..\Common\StringUtils.h"
#include "..\ServerLogger.h"
#include "..\vendor\nlohmann\json.hpp"

#include <algorithm>
#include <errno.h>
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

			static bool FileExists(const std::string &path)
			{
				const std::wstring widePath = StringUtils::Utf8ToWide(path);
				if (widePath.empty())
				{
					return false;
				}

				DWORD attrs = GetFileAttributesW(widePath.c_str());
				return (attrs != INVALID_FILE_ATTRIBUTES) && ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0);
			}

			/**
			 * Creates an empty array file for access lists that do not exist yet
			 * 未作成のアクセス一覧に空配列ファイルを作る
			 */
			static bool EnsureJsonListFile(const std::string &path)
			{
				if (FileExists(path))
				{
					return true;
				}
				return FileUtils::WriteTextFileAtomic(path, "[]\n");
			}

			static bool TryParseNumericXuid(const std::string &text, unsigned long long *outValue)
			{
				if (outValue == nullptr)
				{
					return false;
				}

				std::string trimmed = StringUtils::TrimAscii(text);
				if (trimmed.empty())
				{
					return false;
				}

				errno = 0;
				char *end = nullptr;
				// Accept both decimal and hexadecimal XUID text so manual edits and tool output stay compatible.
				unsigned long long value = _strtoui64(trimmed.c_str(), &end, 0);
				if (end == trimmed.c_str() || errno != 0)
				{
					return false;
				}

				while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')
				{
					++end;
				}

				if (*end != 0)
				{
					return false;
				}

				*outValue = value;
				return true;
			}

			static bool TryGetStringField(const OrderedJson &object, const char *key, std::string *outValue)
			{
				if (key == nullptr || outValue == nullptr || !object.is_object())
				{
					return false;
				}

				OrderedJson::const_iterator it = object.find(key);
				if (it == object.end() || !it->is_string())
				{
					return false;
				}

				*outValue = it->get<std::string>();
				return true;
			}


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

			bool playersOk = EnsureJsonListFile(playersPath);
			bool ipsOk = EnsureJsonListFile(ipsPath);
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
			for (size_t i = 0; i < root.size(); ++i)
			{
				const OrderedJson &object = root[i];
				if (!object.is_object())
				{
					LogWarnf("access", "skipping banned player entry that is not an object in %s", path.c_str());
					continue;
				}

				std::string rawXuid;
				if (!TryGetStringField(object, "xuid", &rawXuid))
				{
					LogWarnf("access", "skipping banned player entry without xuid in %s", path.c_str());
					continue;
				}

				BannedPlayerEntry entry;
				entry.xuid = NormalizeXuid(rawXuid);
				if (entry.xuid.empty())
				{
					LogWarnf("access", "skipping banned player entry with empty xuid in %s", path.c_str());
					continue;
				}

				TryGetStringField(object, "name", &entry.name);
				TryGetStringField(object, "created", &entry.metadata.created);
				TryGetStringField(object, "source", &entry.metadata.source);
				TryGetStringField(object, "expires", &entry.metadata.expires);
				TryGetStringField(object, "reason", &entry.metadata.reason);
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
			for (size_t i = 0; i < root.size(); ++i)
			{
				const OrderedJson &object = root[i];
				if (!object.is_object())
				{
					LogWarnf("access", "skipping banned ip entry that is not an object in %s", path.c_str());
					continue;
				}

				std::string rawIp;
				if (!TryGetStringField(object, "ip", &rawIp))
				{
					LogWarnf("access", "skipping banned ip entry without ip in %s", path.c_str());
					continue;
				}

				BannedIpEntry entry;
				entry.ip = NormalizeIp(rawIp);
				if (entry.ip.empty())
				{
					LogWarnf("access", "skipping banned ip entry with empty ip in %s", path.c_str());
					continue;
				}

				TryGetStringField(object, "created", &entry.metadata.created);
				TryGetStringField(object, "source", &entry.metadata.source);
				TryGetStringField(object, "expires", &entry.metadata.expires);
				TryGetStringField(object, "reason", &entry.metadata.reason);
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
			for (size_t i = 0; i < entries.size(); ++i)
			{
				OrderedJson object = OrderedJson::object();
				object["xuid"] = NormalizeXuid(entries[i].xuid);
				object["name"] = entries[i].name;
				object["created"] = entries[i].metadata.created;
				object["source"] = entries[i].metadata.source;
				object["expires"] = entries[i].metadata.expires;
				object["reason"] = entries[i].metadata.reason;
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
			for (size_t i = 0; i < entries.size(); ++i)
			{
				OrderedJson object = OrderedJson::object();
				object["ip"] = NormalizeIp(entries[i].ip);
				object["created"] = entries[i].metadata.created;
				object["source"] = entries[i].metadata.source;
				object["expires"] = entries[i].metadata.expires;
				object["reason"] = entries[i].metadata.reason;
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
			for (size_t i = 0; i < m_bannedPlayers.size(); ++i)
			{
				if (!IsMetadataExpired(m_bannedPlayers[i].metadata, nowFileTime))
				{
					outEntries->push_back(m_bannedPlayers[i]);
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
			for (size_t i = 0; i < m_bannedIps.size(); ++i)
			{
				if (!IsMetadataExpired(m_bannedIps[i].metadata, nowFileTime))
				{
					outEntries->push_back(m_bannedIps[i]);
				}
			}
			return true;
		}
		bool BanManager::IsPlayerBannedByXuid(const std::string &xuid) const
		{
			const std::string normalized = NormalizeXuid(xuid);
			if (normalized.empty())
			{
				return false;
			}

			const unsigned long long nowFileTime = FileUtils::GetCurrentUtcFileTime();
			for (size_t i = 0; i < m_bannedPlayers.size(); ++i)
			{
				if (m_bannedPlayers[i].xuid == normalized && !IsMetadataExpired(m_bannedPlayers[i].metadata, nowFileTime))
				{
					return true;
				}
			}
			return false;
		}

		bool BanManager::IsIpBanned(const std::string &ip) const
		{
			const std::string normalized = NormalizeIp(ip);
			if (normalized.empty())
			{
				return false;
			}

			const unsigned long long nowFileTime = FileUtils::GetCurrentUtcFileTime();
			for (size_t i = 0; i < m_bannedIps.size(); ++i)
			{
				if (m_bannedIps[i].ip == normalized && !IsMetadataExpired(m_bannedIps[i].metadata, nowFileTime))
				{
					return true;
				}
			}
			return false;
		}

		bool BanManager::AddPlayerBan(const BannedPlayerEntry &entry)
		{
			std::vector<BannedPlayerEntry> updatedEntries;
			if (!SnapshotBannedPlayers(&updatedEntries))
			{
				return false;
			}

			BannedPlayerEntry normalized = entry;
			normalized.xuid = NormalizeXuid(normalized.xuid);
			NormalizeMetadata(&normalized.metadata);
			if (normalized.xuid.empty())
			{
				return false;
			}

			for (size_t i = 0; i < updatedEntries.size(); ++i)
			{
				// Update the existing entry in place so the stored list remains unique by canonical XUID.
				if (updatedEntries[i].xuid == normalized.xuid)
				{
					updatedEntries[i] = normalized;
					if (!SavePlayers(updatedEntries))
					{
						return false;
					}
					m_bannedPlayers.swap(updatedEntries);
					return true;
				}
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
			normalized.ip = NormalizeIp(normalized.ip);
			NormalizeMetadata(&normalized.metadata);
			if (normalized.ip.empty())
			{
				return false;
			}

			for (size_t i = 0; i < updatedEntries.size(); ++i)
			{
				// Update the existing entry in place so the stored list remains unique by normalized IP.
				if (updatedEntries[i].ip == normalized.ip)
				{
					updatedEntries[i] = normalized;
					if (!SaveIps(updatedEntries))
					{
						return false;
					}
					m_bannedIps.swap(updatedEntries);
					return true;
				}
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
			const std::string normalized = NormalizeXuid(xuid);
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
			const std::string normalized = NormalizeIp(ip);
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

			SYSTEMTIME utc;
			GetSystemTime(&utc);

			char created[64] = {};
			sprintf_s(
				created,
				sizeof(created),
				"%04u-%02u-%02uT%02u:%02u:%02uZ",
				(unsigned)utc.wYear,
				(unsigned)utc.wMonth,
				(unsigned)utc.wDay,
				(unsigned)utc.wHour,
				(unsigned)utc.wMinute,
				(unsigned)utc.wSecond);
			metadata.created = created;
			metadata.source = (source != nullptr) ? source : "Server";
			metadata.expires = "";
			metadata.reason = "";
			return metadata;
		}


		std::string BanManager::NormalizeXuid(const std::string &xuid)
		{
			std::string trimmed = StringUtils::TrimAscii(xuid);
			if (trimmed.empty())
			{
				return "";
			}

			unsigned long long numericXuid = 0;
			// Canonicalize numeric XUID input into a lowercase hexadecimal string so lookups stay stable.
			if (TryParseNumericXuid(trimmed, &numericXuid))
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


		std::string BanManager::NormalizeIp(const std::string &ip)
		{
			return StringUtils::ToLowerAscii(StringUtils::TrimAscii(ip));
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
			if (fileName == nullptr || fileName[0] == 0)
			{
				return "";
			}

			const std::wstring wideFileName = StringUtils::Utf8ToWide(fileName);
			if (wideFileName.empty())
			{
				return "";
			}

			if (m_baseDirectory.empty() || m_baseDirectory == ".")
			{
				return StringUtils::WideToUtf8(wideFileName);
			}

			const std::wstring wideBaseDirectory = StringUtils::Utf8ToWide(m_baseDirectory);
			if (wideBaseDirectory.empty())
			{
				return StringUtils::WideToUtf8(wideFileName);
			}

			// Join Unicode-aware path strings after conversion so non-ASCII dedicated-server directories remain supported.
			const wchar_t last = wideBaseDirectory[wideBaseDirectory.size() - 1];
			if (last == L'\\' || last == L'/')
			{
				return StringUtils::WideToUtf8(wideBaseDirectory + wideFileName);
			}

			return StringUtils::WideToUtf8(wideBaseDirectory + L"\\" + wideFileName);
		}
	}
}
