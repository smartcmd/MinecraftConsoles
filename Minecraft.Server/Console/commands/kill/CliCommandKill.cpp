#include "stdafx.h"

#include "CliCommandKill.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\..\..\..\Minecraft.World\CommandSender.h"
#include "..\..\..\..\Minecraft.Client\ServerPlayer.h"

namespace ServerRuntime
{
	namespace
	{
		constexpr const char *kKillUsage = "kill <player>";
	}

	const char *CliCommandKill::Name() const
	{
		return "kill";
	}

	const char *CliCommandKill::Usage() const
	{
		return kKillUsage;
	}

	const char *CliCommandKill::Description() const
	{
		return "Kill a player via Minecraft.World command dispatcher.";
	}

	bool CliCommandKill::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() != 2)
		{
			engine->LogWarn(std::string("Usage: ") + kKillUsage);
			return false;
		}

		std::shared_ptr<ServerPlayer> target = engine->FindPlayerByNameUtf8(line.tokens[1]);
		if (target == nullptr)
		{
			engine->LogWarn("Unknown player: " + line.tokens[1]);
			return false;
		}

		std::shared_ptr<CommandSender> sender = std::dynamic_pointer_cast<CommandSender>(target);
		if (sender == nullptr)
		{
			engine->LogWarn("Cannot resolve target command sender.");
			return false;
		}

		return engine->DispatchWorldCommand(eGameCommand_Kill, byteArray(), sender);
	}

	void CliCommandKill::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		if (context.currentTokenIndex == 1)
		{
			engine->SuggestPlayers(context.prefix, context.linePrefix, out);
		}
	}
}
