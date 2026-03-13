#include "stdafx.h"

#include "CliCommandTp.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\..\..\..\Minecraft.Client\PlayerConnection.h"
#include "..\..\..\..\Minecraft.Client\TeleportCommand.h"
#include "..\..\..\..\Minecraft.Client\ServerPlayer.h"
#include "..\..\..\..\Minecraft.World\GameCommandPacket.h"

namespace ServerRuntime
{
	namespace
	{
		constexpr const char *kTpUsage = "tp <player> <target>";
	}

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
		return kTpUsage;
	}

	const char *CliCommandTp::Description() const
	{
		return "Teleport one player to another via Minecraft.World command dispatcher.";
	}

	bool CliCommandTp::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() != 3)
		{
			engine->LogWarn(std::string("Usage: ") + kTpUsage);
			return false;
		}

		std::shared_ptr<ServerPlayer> subject = engine->FindPlayerByNameUtf8(line.tokens[1]);
		std::shared_ptr<ServerPlayer> destination = engine->FindPlayerByNameUtf8(line.tokens[2]);
		if (subject == nullptr)
		{
			engine->LogWarn("Unknown player: " + line.tokens[1]);
			return false;
		}
		if (destination == nullptr)
		{
			engine->LogWarn("Unknown player: " + line.tokens[2]);
			return false;
		}
		if (subject->connection == nullptr)
		{
			engine->LogWarn("Cannot teleport because source player connection is inactive.");
			return false;
		}
		std::shared_ptr<GameCommandPacket> packet = TeleportCommand::preparePacket(subject->getXuid(), destination->getXuid());
		if (packet == nullptr)
		{
			engine->LogError("Failed to build teleport command packet.");
			return false;
		}

		return engine->DispatchWorldCommand(packet->command, packet->data);
	}

	void CliCommandTp::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		if (context.currentTokenIndex == 1 || context.currentTokenIndex == 2)
		{
			engine->SuggestPlayers(context.prefix, context.linePrefix, out);
		}
	}
}

