#include "stdafx.h"

#include "CliCommandList.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\..\Common\StringUtils.h"
#include "..\..\..\..\Minecraft.Client\MinecraftServer.h"
#include "..\..\..\..\Minecraft.Client\PlayerList.h"

namespace ServerRuntime
{
	const char *CliCommandList::Name() const
	{
		return "list";
	}

	const char *CliCommandList::Usage() const
	{
		return "list";
	}

	const char *CliCommandList::Description() const
	{
		return "List connected players.";
	}

	bool CliCommandList::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		(void)line;
		MinecraftServer *server = MinecraftServer::getInstance();
		if (server == NULL || server->getPlayers() == NULL)
		{
			engine->LogWarn("Player list is not available.");
			return false;
		}

		PlayerList *players = server->getPlayers();
		std::string names = StringUtils::WideToUtf8(players->getPlayerNames());
		if (names.empty())
		{
			names = "(none)";
		}

		engine->LogInfo("Players (" + std::to_string(players->getPlayerCount()) + "): " + names);
		return true;
	}
}


