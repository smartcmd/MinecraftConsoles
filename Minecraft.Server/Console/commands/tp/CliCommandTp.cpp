#include "stdafx.h"

#include "CliCommandTp.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\..\..\..\Minecraft.Client\PlayerConnection.h"
#include "..\..\..\..\Minecraft.Client\ServerPlayer.h"
#include "..\..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\..\Minecraft.World\net.minecraft.world.level.dimension.h"

namespace ServerRuntime
{
	const char *CliCommandTp::Name() const
	{
		return "tp";
	}

	std::vector<std::string> CliCommandTp::Aliases() const
	{
		return { "teleport" };
	}

	const char *CliCommandTp::Usage() const
	{
		return "tp <player> <target>";
	}

	const char *CliCommandTp::Description() const
	{
		return "Teleport one player to another player.";
	}

	bool CliCommandTp::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() < 3)
		{
			engine->LogWarn("Usage: tp <player> <target>");
			return false;
		}

		std::shared_ptr<ServerPlayer> subject = engine->FindPlayerByNameUtf8(line.tokens[1]);
		std::shared_ptr<ServerPlayer> destination = engine->FindPlayerByNameUtf8(line.tokens[2]);
		if (subject == NULL)
		{
			engine->LogWarn("Unknown player: " + line.tokens[1]);
			return false;
		}
		if (destination == NULL)
		{
			engine->LogWarn("Unknown player: " + line.tokens[2]);
			return false;
		}
		if (subject->connection == NULL)
		{
			engine->LogWarn("Cannot teleport because source player connection is inactive.");
			return false;
		}
		if (subject->level == NULL || destination->level == NULL || subject->level->dimension->id != destination->level->dimension->id || !subject->isAlive())
		{
			engine->LogWarn("Teleport failed because players are in different dimensions or source player is dead.");
			return false;
		}

		subject->ride(nullptr);
		subject->connection->teleport(destination->x, destination->y, destination->z, destination->yRot, destination->xRot);
		engine->LogInfo("Teleported " + line.tokens[1] + " to " + line.tokens[2] + ".");
		return true;
	}

	void CliCommandTp::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		if (context.currentTokenIndex == 1 || context.currentTokenIndex == 2)
		{
			engine->SuggestPlayers(context.prefix, context.linePrefix, out);
		}
	}
}

