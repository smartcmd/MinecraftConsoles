#include "stdafx.h"

#include "CliCommandPardonIp.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\..\..\Access\Access.h"
#include "..\..\..\Common\NetworkUtils.h"
#include "..\..\..\Common\StringUtils.h"

namespace ServerRuntime
{
	const char *CliCommandPardonIp::Name() const
	{
		return "pardon-ip";
	}

	const char *CliCommandPardonIp::Usage() const
	{
		return "pardon-ip <address>";
	}

	const char *CliCommandPardonIp::Description() const
	{
		return "Remove an IP ban.";
	}

	/**
	 * Validates the literal IP argument and removes the matching Access IP ban entry
	 * リテラルIPを検証して一致するIP BANを解除する
	 */
	bool CliCommandPardonIp::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() != 2)
		{
			engine->LogWarn("Usage: pardon-ip <address>");
			return false;
		}
		if (!ServerRuntime::Access::IsInitialized())
		{
			engine->LogWarn("Access manager is not initialized.");
			return false;
		}

		// Java Edition pardon-ip only operates on a literal address, so do not resolve player names here.
		const std::string ip = StringUtils::TrimAscii(line.tokens[1]);
		if (!NetworkUtils::IsIpLiteral(ip))
		{
			engine->LogWarn("Invalid IP address: " + line.tokens[1]);
			return false;
		}
		// Distinguish invalid input from a valid but currently unbanned address for clearer operator feedback.
		if (!ServerRuntime::Access::IsIpBanned(ip))
		{
			engine->LogWarn("That IP address is not banned.");
			return false;
		}
		if (!ServerRuntime::Access::RemoveIpBan(ip))
		{
			engine->LogError("Failed to remove IP ban.");
			return false;
		}

		engine->LogInfo("Unbanned IP address " + ip + ".");
		return true;
	}

	/**
	 * Suggests currently banned IP addresses for the Java Edition literal-IP argument
	 * BAN済みIPの補完候補を返す
	 */
	void CliCommandPardonIp::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		(void)engine;
		// Complete from the persisted IP-ban snapshot because this command only accepts already-banned literals.
		if (context.currentTokenIndex != 1 || out == nullptr)
		{
			return;
		}

		std::vector<ServerRuntime::Access::BannedIpEntry> entries;
		if (!ServerRuntime::Access::SnapshotBannedIps(&entries))
		{
			return;
		}

		// Reuse the normalized prefix match used by other commands so completion stays case-insensitive.
		for (const auto &entry : entries)
		{
			const std::string &candidate = entry.ip;
			if (StringUtils::StartsWithIgnoreCase(candidate, context.prefix))
			{
				out->push_back(context.linePrefix + candidate);
			}
		}
	}
}

