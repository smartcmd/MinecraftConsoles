#include "stdafx.h"

#include "CliCommandGamemode.h"

#include "..\ServerCliEngine.h"
#include "..\ServerCliParser.h"
#include "..\..\Common\StringUtils.h"
#include "..\..\..\Minecraft.Client\MinecraftServer.h"
#include "..\..\..\Minecraft.Client\PlayerList.h"
#include "..\..\..\Minecraft.Client\ServerPlayer.h"
#include "..\..\..\Minecraft.World\LevelSettings.h"

namespace ServerRuntime
{
	const char *CliCommandGamemode::Name() const
	{
		return "gamemode";
	}

	std::vector<std::string> CliCommandGamemode::Aliases() const
	{
		return { "gm" };
	}

	const char *CliCommandGamemode::Usage() const
	{
		return "gamemode <survival|creative|0|1> [player]";
	}

	const char *CliCommandGamemode::Description() const
	{
		return "Set a player's game mode.";
	}

	bool CliCommandGamemode::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() < 2)
		{
			engine->LogWarn("Usage: gamemode <survival|creative|0|1> [player]");
			return false;
		}

		GameType *mode = engine->ParseGamemode(line.tokens[1]);
		if (mode == NULL)
		{
			engine->LogWarn("Unknown gamemode: " + line.tokens[1]);
			return false;
		}

		std::shared_ptr<ServerPlayer> target = nullptr;
		if (line.tokens.size() >= 3)
		{
			target = engine->FindPlayerByNameUtf8(line.tokens[2]);
			if (target == NULL)
			{
				engine->LogWarn("Unknown player: " + line.tokens[2]);
				return false;
			}
		}
		else
		{
			MinecraftServer *server = MinecraftServer::getInstance();
			if (server == NULL || server->getPlayers() == NULL)
			{
				engine->LogWarn("Player list is not available.");
				return false;
			}

			PlayerList *players = server->getPlayers();
			if (players->players.size() != 1 || players->players[0] == NULL)
			{
				engine->LogWarn("Usage: gamemode <survival|creative|0|1> <player>");
				return false;
			}
			target = players->players[0];
		}

		target->setGameMode(mode);
		target->fallDistance = 0.0f;
		engine->LogInfo(
			"Set " + StringUtils::WideToUtf8(target->getName()) + " gamemode to " +
			StringUtils::WideToUtf8(mode->getName()) + ".");
		return true;
	}

	void CliCommandGamemode::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		if (context.currentTokenIndex == 1)
		{
			engine->SuggestGamemodes(context.prefix, context.linePrefix, out);
		}
		else if (context.currentTokenIndex == 2)
		{
			engine->SuggestPlayers(context.prefix, context.linePrefix, out);
		}
	}
}

