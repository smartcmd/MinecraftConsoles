#include "stdafx.h"

#include "WhitelistManager.h"

#include "..\Common\AccessStorageUtils.h"
#include "..\Common\FileUtils.h"
#include "..\Common\StringUtils.h"
#include "..\ServerLogger.h"
#include "..\vendor\nlohmann\json.hpp"

#include <algorithm>

namespace ServerRuntime
{
	namespace Access
	{
		using OrderedJson = nlohmann::ordered_json;

		namespace
		{
			static const char *kWhitelistFileName = "whitelist.json";
		}

		WhitelistManager::WhitelistManager(const std::string &baseDirectory)
			: m_baseDirectory(baseDirectory.empty() ? "." : baseDirectory)
		{
		}

		bool WhitelistManager::EnsureWhitelistFileExists() const
		{
			const std::string path = GetWhitelistFilePath();
			if (!AccessStorageUtils::EnsureJsonListFileExists(path))
			{
				LogErrorf("access", "failed to create %s", path.c_str());
				return false;
			}
			return true;
		}

		bool WhitelistManager::Reload()
		{
			std::vector<WhitelistedPlayerEntry> players;
			if (!LoadPlayers(&players))
			{
				return false;
			}

			m_whitelistedPlayers.swap(players);
			return true;
		}

		bool WhitelistManager::Save() const
		{
			std::vector<WhitelistedPlayerEntry> players;
			return SnapshotWhitelistedPlayers(&players) && SavePlayers(players);
		}

		bool WhitelistManager::LoadPlayers(std::vector<WhitelistedPlayerEntry> *outEntries) const
		{
			if (outEntries == nullptr)
			{
				return false;
			}
			outEntries->clear();

			std::string text;
			const std::string path = GetWhitelistFilePath();
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

			for (const auto &object : root)
			{
				if (!object.is_object())
				{
					LogWarnf("access", "skipping whitelist entry that is not an object in %s", path.c_str());
					continue;
				}

				std::string rawXuid;
				if (!AccessStorageUtils::TryGetStringField(object, "xuid", &rawXuid))
				{
					LogWarnf("access", "skipping whitelist entry without xuid in %s", path.c_str());
					continue;
				}

				WhitelistedPlayerEntry entry;
				entry.xuid = AccessStorageUtils::NormalizeXuid(rawXuid);
				if (entry.xuid.empty())
				{
					LogWarnf("access", "skipping whitelist entry with empty xuid in %s", path.c_str());
					continue;
				}

				AccessStorageUtils::TryGetStringField(object, "name", &entry.name);
				AccessStorageUtils::TryGetStringField(object, "created", &entry.metadata.created);
				AccessStorageUtils::TryGetStringField(object, "source", &entry.metadata.source);
				NormalizeMetadata(&entry.metadata);

				outEntries->push_back(entry);
			}

			return true;
		}

		bool WhitelistManager::SavePlayers(const std::vector<WhitelistedPlayerEntry> &entries) const
		{
			OrderedJson root = OrderedJson::array();
			for (const auto &entry : entries)
			{
				OrderedJson object = OrderedJson::object();
				object["xuid"] = AccessStorageUtils::NormalizeXuid(entry.xuid);
				object["name"] = entry.name;
				object["created"] = entry.metadata.created;
				object["source"] = entry.metadata.source;
				root.push_back(object);
			}

			const std::string path = GetWhitelistFilePath();
			const std::string json = root.empty() ? std::string("[]\n") : (root.dump(2) + "\n");
			if (!FileUtils::WriteTextFileAtomic(path, json))
			{
				LogErrorf("access", "failed to write %s", path.c_str());
				return false;
			}
			return true;
		}

		const std::vector<WhitelistedPlayerEntry> &WhitelistManager::GetWhitelistedPlayers() const
		{
			return m_whitelistedPlayers;
		}

		bool WhitelistManager::SnapshotWhitelistedPlayers(std::vector<WhitelistedPlayerEntry> *outEntries) const
		{
			if (outEntries == nullptr)
			{
				return false;
			}

			*outEntries = m_whitelistedPlayers;
			return true;
		}

		bool WhitelistManager::IsPlayerWhitelistedByXuid(const std::string &xuid) const
		{
			const auto normalized = AccessStorageUtils::NormalizeXuid(xuid);
			if (normalized.empty())
			{
				return false;
			}

			return std::any_of(
				m_whitelistedPlayers.begin(),
				m_whitelistedPlayers.end(),
				[&normalized](const WhitelistedPlayerEntry &entry)
				{
					return entry.xuid == normalized;
				});
		}

		bool WhitelistManager::AddPlayer(const WhitelistedPlayerEntry &entry)
		{
			std::vector<WhitelistedPlayerEntry> updatedEntries;
			if (!SnapshotWhitelistedPlayers(&updatedEntries))
			{
				return false;
			}

			auto normalized = entry;
			normalized.xuid = AccessStorageUtils::NormalizeXuid(normalized.xuid);
			NormalizeMetadata(&normalized.metadata);
			if (normalized.xuid.empty())
			{
				return false;
			}

			const auto existing = std::find_if(
				updatedEntries.begin(),
				updatedEntries.end(),
				[&normalized](const WhitelistedPlayerEntry &candidate)
				{
					return candidate.xuid == normalized.xuid;
				});

			if (existing != updatedEntries.end())
			{
				*existing = normalized;
				if (!SavePlayers(updatedEntries))
				{
					return false;
				}

				m_whitelistedPlayers.swap(updatedEntries);
				return true;
			}

			updatedEntries.push_back(normalized);
			if (!SavePlayers(updatedEntries))
			{
				return false;
			}

			m_whitelistedPlayers.swap(updatedEntries);
			return true;
		}

		bool WhitelistManager::RemovePlayerByXuid(const std::string &xuid)
		{
			const auto normalized = AccessStorageUtils::NormalizeXuid(xuid);
			if (normalized.empty())
			{
				return false;
			}

			std::vector<WhitelistedPlayerEntry> updatedEntries;
			if (!SnapshotWhitelistedPlayers(&updatedEntries))
			{
				return false;
			}

			const auto oldSize = updatedEntries.size();
			updatedEntries.erase(
				std::remove_if(
					updatedEntries.begin(),
					updatedEntries.end(),
					[&normalized](const WhitelistedPlayerEntry &entry) { return entry.xuid == normalized; }),
				updatedEntries.end());

			if (updatedEntries.size() == oldSize)
			{
				return false;
			}

			if (!SavePlayers(updatedEntries))
			{
				return false;
			}

			m_whitelistedPlayers.swap(updatedEntries);
			return true;
		}

		std::string WhitelistManager::GetWhitelistFilePath() const
		{
			return BuildPath(kWhitelistFileName);
		}

		WhitelistMetadata WhitelistManager::BuildDefaultMetadata(const char *source)
		{
			WhitelistMetadata metadata;
			metadata.created = StringUtils::GetCurrentUtcTimestampIso8601();
			metadata.source = (source != nullptr) ? source : "Server";
			return metadata;
		}

		void WhitelistManager::NormalizeMetadata(WhitelistMetadata *metadata)
		{
			if (metadata == nullptr)
			{
				return;
			}

			metadata->created = StringUtils::TrimAscii(metadata->created);
			metadata->source = StringUtils::TrimAscii(metadata->source);
		}

		std::string WhitelistManager::BuildPath(const char *fileName) const
		{
			return AccessStorageUtils::BuildPathFromBaseDirectory(m_baseDirectory, fileName);
		}
	}
}
