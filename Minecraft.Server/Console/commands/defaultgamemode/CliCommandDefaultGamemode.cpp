#include "stdafx.h"

#include "CliCommandDefaultGamemode.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\..\..\..\Minecraft.Client\MinecraftServer.h"
#include "..\..\..\..\Minecraft.Client\PlayerList.h"
#include "..\..\..\..\Minecraft.Client\ServerLevel.h"
#include "..\..\..\..\Minecraft.Client\ServerPlayer.h"
#include "..\..\..\..\Minecraft.World\net.minecraft.world.level.storage.h"

namespace ServerRuntime
{
	namespace
	{
		constexpr const char *kDefaultGamemodeUsage = "defaultgamemode <survival|creative|0|1>";

		static std::string ModeLabel(GameType *mode)
		{
			if (mode == GameType::SURVIVAL)
			{
				return "survival";
			}
			if (mode == GameType::CREATIVE)
			{
				return "creative";
			}
			if (mode == GameType::ADVENTURE)
			{
				return "adventure";
			}

			return std::to_string(mode != nullptr ? mode->getId() : -1);
		}
	}

	const char *CliCommandDefaultGamemode::Name() const
	{
		return "defaultgamemode";
	}

	const char *CliCommandDefaultGamemode::Usage() const
	{
		return kDefaultGamemodeUsage;
	}

	const char *CliCommandDefaultGamemode::Description() const
	{
		return "Set the default game mode (server-side implementation).";
	}

	bool CliCommandDefaultGamemode::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() != 2)
		{
			engine->LogWarn(std::string("Usage: ") + kDefaultGamemodeUsage);
			return false;
		}

		GameType *mode = engine->ParseGamemode(line.tokens[1]);
		if (mode == nullptr)
		{
			engine->LogWarn("Unknown gamemode: " + line.tokens[1]);
			return false;
		}

		MinecraftServer *server = MinecraftServer::getInstance();
		if (server == nullptr)
		{
			engine->LogWarn("MinecraftServer instance is not available.");
			return false;
		}

		PlayerList *players = server->getPlayers();
		if (players == nullptr)
		{
			engine->LogWarn("Player list is not available.");
			return false;
		}

		players->setOverrideGameMode(mode);

		for (unsigned int i = 0; i < server->levels.length; ++i)
		{
			ServerLevel *level = server->levels[i];
			if (level != nullptr && level->getLevelData() != nullptr)
			{
				level->getLevelData()->setGameType(mode);
			}
		}

		if (server->getForceGameType())
		{
			for (size_t i = 0; i < players->players.size(); ++i)
			{
				std::shared_ptr<ServerPlayer> player = players->players[i];
				if (player != nullptr)
				{
					player->setGameMode(mode);
					player->fallDistance = 0.0f;
				}
			}
		}

		engine->LogInfo("Default gamemode set to " + ModeLabel(mode) + ".");
		return true;
	}

	void CliCommandDefaultGamemode::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		if (context.currentTokenIndex == 1)
		{
			engine->SuggestGamemodes(context.prefix, context.linePrefix, out);
		}
	}
}
