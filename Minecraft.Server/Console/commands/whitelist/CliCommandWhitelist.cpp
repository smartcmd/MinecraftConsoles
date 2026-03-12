#include "stdafx.h"

#include "CliCommandWhitelist.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\..\..\Access\Access.h"
#include "..\..\..\Common\StringUtils.h"
#include "..\..\..\ServerProperties.h"

#include <algorithm>
#include <array>

namespace ServerRuntime
{
	namespace
	{
		static const char *kWhitelistUsage = "whitelist <on|off|list|add|remove|reload> [...]";

		static bool CompareWhitelistEntries(const ServerRuntime::Access::WhitelistedPlayerEntry &left, const ServerRuntime::Access::WhitelistedPlayerEntry &right)
		{
			const auto leftName = StringUtils::ToLowerAscii(left.name);
			const auto rightName = StringUtils::ToLowerAscii(right.name);
			if (leftName != rightName)
			{
				return leftName < rightName;
			}

			return StringUtils::ToLowerAscii(left.xuid) < StringUtils::ToLowerAscii(right.xuid);
		}

		static bool PersistWhitelistToggle(bool enabled)
		{
			auto config = LoadServerPropertiesConfig();
			config.whiteListEnabled = enabled;
			return SaveServerPropertiesConfig(config);
		}

		static std::string BuildWhitelistEntryRow(const ServerRuntime::Access::WhitelistedPlayerEntry &entry)
		{
			std::string row = "  ";
			row += entry.xuid;
			if (!entry.name.empty())
			{
				row += " - ";
				row += entry.name;
			}
			return row;
		}

		static void LogWhitelistMode(ServerCliEngine *engine)
		{
			engine->LogInfo(std::string("Whitelist is ") + (ServerRuntime::Access::IsWhitelistEnabled() ? "enabled." : "disabled."));
		}

		static bool LogWhitelistEntries(ServerCliEngine *engine)
		{
			std::vector<ServerRuntime::Access::WhitelistedPlayerEntry> entries;
			if (!ServerRuntime::Access::SnapshotWhitelistedPlayers(&entries))
			{
				engine->LogError("Failed to read whitelist entries.");
				return false;
			}

			std::sort(entries.begin(), entries.end(), CompareWhitelistEntries);
			LogWhitelistMode(engine);
			engine->LogInfo("There are " + std::to_string(entries.size()) + " whitelisted player(s).");
			for (const auto &entry : entries)
			{
				engine->LogInfo(BuildWhitelistEntryRow(entry));
			}
			return true;
		}

		static bool TryParseWhitelistXuid(const std::string &text, ServerCliEngine *engine, PlayerUID *outXuid)
		{
			if (ServerRuntime::Access::TryParseXuid(text, outXuid))
			{
				return true;
			}

			engine->LogWarn("Invalid XUID: " + text);
			return false;
		}

		static void SuggestLiteral(const std::string &candidate, const ServerCliCompletionContext &context, std::vector<std::string> *out)
		{
			if (out == nullptr)
			{
				return;
			}

			if (StringUtils::StartsWithIgnoreCase(candidate, context.prefix))
			{
				out->push_back(context.linePrefix + candidate);
			}
		}
	}

	const char *CliCommandWhitelist::Name() const
	{
		return "whitelist";
	}

	const char *CliCommandWhitelist::Usage() const
	{
		return kWhitelistUsage;
	}

	const char *CliCommandWhitelist::Description() const
	{
		return "Manage the dedicated-server XUID whitelist.";
	}

	bool CliCommandWhitelist::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() < 2)
		{
			engine->LogWarn(std::string("Usage: ") + kWhitelistUsage);
			return false;
		}
		if (!ServerRuntime::Access::IsInitialized())
		{
			engine->LogWarn("Access manager is not initialized.");
			return false;
		}

		const auto subcommand = StringUtils::ToLowerAscii(line.tokens[1]);
		if (subcommand == "on" || subcommand == "off")
		{
			if (line.tokens.size() != 2)
			{
				engine->LogWarn("Usage: whitelist <on|off>");
				return false;
			}

			const bool enabled = (subcommand == "on");
			if (!PersistWhitelistToggle(enabled))
			{
				engine->LogError("Failed to persist whitelist mode to server.properties.");
				return false;
			}

			ServerRuntime::Access::SetWhitelistEnabled(enabled);
			engine->LogInfo(std::string("Whitelist ") + (enabled ? "enabled." : "disabled."));
			return true;
		}

		if (subcommand == "list")
		{
			if (line.tokens.size() != 2)
			{
				engine->LogWarn("Usage: whitelist list");
				return false;
			}

			return LogWhitelistEntries(engine);
		}

		if (subcommand == "reload")
		{
			if (line.tokens.size() != 2)
			{
				engine->LogWarn("Usage: whitelist reload");
				return false;
			}
			if (!ServerRuntime::Access::ReloadWhitelist())
			{
				engine->LogError("Failed to reload whitelist.");
				return false;
			}

			const auto config = LoadServerPropertiesConfig();
			ServerRuntime::Access::SetWhitelistEnabled(config.whiteListEnabled);
			engine->LogInfo("Reloaded whitelist from disk.");
			LogWhitelistMode(engine);
			return true;
		}

		if (subcommand == "add")
		{
			if (line.tokens.size() < 3)
			{
				engine->LogWarn("Usage: whitelist add <xuid> [name ...]");
				return false;
			}

			PlayerUID xuid = INVALID_XUID;
			if (!TryParseWhitelistXuid(line.tokens[2], engine, &xuid))
			{
				return false;
			}

			if (ServerRuntime::Access::IsPlayerWhitelisted(xuid))
			{
				engine->LogWarn("That XUID is already whitelisted.");
				return false;
			}

			const auto metadata = ServerRuntime::Access::WhitelistManager::BuildDefaultMetadata("Console");
			const auto name = StringUtils::JoinTokens(line.tokens, 3);
			if (!ServerRuntime::Access::AddWhitelistedPlayer(xuid, name, metadata))
			{
				engine->LogError("Failed to write whitelist entry.");
				return false;
			}

			std::string message = "Whitelisted XUID " + ServerRuntime::Access::FormatXuid(xuid) + ".";
			if (!name.empty())
			{
				message += " Name: " + name;
			}
			engine->LogInfo(message);
			return true;
		}

		if (subcommand == "remove")
		{
			if (line.tokens.size() != 3)
			{
				engine->LogWarn("Usage: whitelist remove <xuid>");
				return false;
			}

			PlayerUID xuid = INVALID_XUID;
			if (!TryParseWhitelistXuid(line.tokens[2], engine, &xuid))
			{
				return false;
			}

			if (!ServerRuntime::Access::IsPlayerWhitelisted(xuid))
			{
				engine->LogWarn("That XUID is not whitelisted.");
				return false;
			}

			if (!ServerRuntime::Access::RemoveWhitelistedPlayer(xuid))
			{
				engine->LogError("Failed to remove whitelist entry.");
				return false;
			}

			engine->LogInfo("Removed XUID " + ServerRuntime::Access::FormatXuid(xuid) + " from the whitelist.");
			return true;
		}

		engine->LogWarn(std::string("Usage: ") + kWhitelistUsage);
		return false;
	}

	void CliCommandWhitelist::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		(void)engine;
		if (out == nullptr)
		{
			return;
		}

		if (context.currentTokenIndex == 1)
		{
			SuggestLiteral("on", context, out);
			SuggestLiteral("off", context, out);
			SuggestLiteral("list", context, out);
			SuggestLiteral("add", context, out);
			SuggestLiteral("remove", context, out);
			SuggestLiteral("reload", context, out);
			return;
		}

		if (context.currentTokenIndex == 2 && context.parsed.tokens.size() >= 2 && StringUtils::ToLowerAscii(context.parsed.tokens[1]) == "remove")
		{
			std::vector<ServerRuntime::Access::WhitelistedPlayerEntry> entries;
			if (!ServerRuntime::Access::SnapshotWhitelistedPlayers(&entries))
			{
				return;
			}

			for (const auto &entry : entries)
			{
				SuggestLiteral(entry.xuid, context, out);
			}
		}
	}
}

