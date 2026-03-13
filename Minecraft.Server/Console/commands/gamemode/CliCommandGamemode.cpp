#include "stdafx.h"

#include "CliCommandGamemode.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\..\..\..\Minecraft.Client\MinecraftServer.h"
#include "..\..\..\..\Minecraft.Client\PlayerList.h"
#include "..\..\..\..\Minecraft.Client\ServerPlayer.h"

namespace ServerRuntime
{
	namespace
	{
		constexpr const char *kGamemodeUsage = "gamemode <survival|creative|0|1> [player]";
		constexpr const char *kGamemodeUsageWithPlayer = "gamemode <survival|creative|0|1> <player>";
	}

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
		return kGamemodeUsage;
	}

	const char *CliCommandGamemode::Description() const
	{
		return "Set a player's game mode.";
	}

	bool CliCommandGamemode::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() < 2 || line.tokens.size() > 3)
		{
			engine->LogWarn(std::string("Usage: ") + kGamemodeUsage);
			return false;
		}

		GameType *mode = engine->ParseGamemode(line.tokens[1]);
		if (mode == nullptr)
		{
			engine->LogWarn("Unknown gamemode: " + line.tokens[1]);
			return false;
		}

		std::shared_ptr<ServerPlayer> target = nullptr;
		if (line.tokens.size() >= 3)
		{
			target = engine->FindPlayerByNameUtf8(line.tokens[2]);
			if (target == nullptr)
			{
				engine->LogWarn("Unknown player: " + line.tokens[2]);
				return false;
			}
		}
		else
		{
			MinecraftServer *server = MinecraftServer::getInstance();
			if (server == nullptr || server->getPlayers() == nullptr)
			{
				engine->LogWarn("Player list is not available.");
				return false;
			}

			PlayerList *players = server->getPlayers();
			if (players->players.size() != 1 || players->players[0] == nullptr)
			{
				engine->LogWarn(std::string("Usage: ") + kGamemodeUsageWithPlayer);
				return false;
			}
			target = players->players[0];
		}

		target->setGameMode(mode);
		target->fallDistance = 0.0f;

		if (line.tokens.size() >= 3)
		{
			engine->LogInfo("Set " + line.tokens[2] + " gamemode to " + line.tokens[1] + ".");
		}
		else
		{
			engine->LogInfo("Set gamemode to " + line.tokens[1] + ".");
		}
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


