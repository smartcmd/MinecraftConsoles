#include "stdafx.h"

#include "CliCommandBanList.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\..\..\Access\Access.h"
#include "..\..\..\Common\StringUtils.h"

#include <algorithm>

namespace ServerRuntime
{
	namespace
	{
		static void AppendUniqueText(const std::string &text, std::vector<std::string> *out)
		{
			if (out == nullptr || text.empty())
			{
				return;
			}

			if (std::find(out->begin(), out->end(), text) == out->end())
			{
				out->push_back(text);
			}
		}

		static bool CompareLowerAscii(const std::string &left, const std::string &right)
		{
			return StringUtils::ToLowerAscii(left) < StringUtils::ToLowerAscii(right);
		}

		static bool LogBannedPlayers(ServerCliEngine *engine)
		{
			std::vector<ServerRuntime::Access::BannedPlayerEntry> entries;
			if (!ServerRuntime::Access::SnapshotBannedPlayers(&entries))
			{
				engine->LogError("Failed to read banned players.");
				return false;
			}

			std::vector<std::string> names;
			for (const auto &entry : entries)
			{
				AppendUniqueText(entry.name, &names);
			}
			std::sort(names.begin(), names.end(), CompareLowerAscii);

			engine->LogInfo("There are " + std::to_string(names.size()) + " banned player(s).");
			for (const auto &name : names)
			{
				engine->LogInfo("  " + name);
			}
			return true;
		}

		static bool LogBannedIps(ServerCliEngine *engine)
		{
			std::vector<ServerRuntime::Access::BannedIpEntry> entries;
			if (!ServerRuntime::Access::SnapshotBannedIps(&entries))
			{
				engine->LogError("Failed to read banned IPs.");
				return false;
			}

			std::vector<std::string> ips;
			for (const auto &entry : entries)
			{
				AppendUniqueText(entry.ip, &ips);
			}
			std::sort(ips.begin(), ips.end(), CompareLowerAscii);

			engine->LogInfo("There are " + std::to_string(ips.size()) + " banned IP(s).");
			for (const auto &ip : ips)
			{
				engine->LogInfo("  " + ip);
			}
			return true;
		}

		static bool LogAllBans(ServerCliEngine *engine)
		{
			if (!LogBannedPlayers(engine))
			{
				return false;
			}

			// Always print the IP snapshot as well so ban-ip entries are visible from the same command output.
			return LogBannedIps(engine);
		}
	}

	const char *CliCommandBanList::Name() const
	{
		return "banlist";
	}

	const char *CliCommandBanList::Usage() const
	{
		return "banlist";
	}

	const char *CliCommandBanList::Description() const
	{
		return "List all banned players and IPs.";
	}

	/**
	 * Reads the current Access snapshots and always prints both banned players and banned IPs
	 * Access の一覧を読みプレイヤーBANとIP BANをまとめて表示する
	 */
	bool CliCommandBanList::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() > 1)
		{
			engine->LogWarn("Usage: banlist");
			return false;
		}
		if (!ServerRuntime::Access::IsInitialized())
		{
			engine->LogWarn("Access manager is not initialized.");
			return false;
		}

		return LogAllBans(engine);
	}

	void CliCommandBanList::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		(void)context;
		(void)engine;
		(void)out;
	}
}

