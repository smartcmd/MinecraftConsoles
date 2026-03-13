#include "stdafx.h"

#include "CliCommandGive.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\CommandParsing.h"
#include "..\..\..\..\Minecraft.World\GameCommandPacket.h"
#include "..\..\..\..\Minecraft.World\GiveItemCommand.h"
#include "..\..\..\..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\..\..\..\Minecraft.Client\ServerPlayer.h"

namespace ServerRuntime
{
	namespace
	{
		constexpr const char *kGiveUsage = "give <player> <itemId> [amount] [aux]";
	}

	const char *CliCommandGive::Name() const
	{
		return "give";
	}

	const char *CliCommandGive::Usage() const
	{
		return kGiveUsage;
	}

	const char *CliCommandGive::Description() const
	{
		return "Give an item via Minecraft.World command dispatcher.";
	}

	bool CliCommandGive::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() < 3 || line.tokens.size() > 5)
		{
			engine->LogWarn(std::string("Usage: ") + kGiveUsage);
			return false;
		}

		std::shared_ptr<ServerPlayer> target = engine->FindPlayerByNameUtf8(line.tokens[1]);
		if (target == nullptr)
		{
			engine->LogWarn("Unknown player: " + line.tokens[1]);
			return false;
		}

		int itemId = 0;
		int amount = 1;
		int aux = 0;
		if (!CommandParsing::TryParseInt(line.tokens[2], &itemId))
		{
			engine->LogWarn("Invalid item id: " + line.tokens[2]);
			return false;
		}
		if (itemId <= 0)
		{
			engine->LogWarn("Item id must be greater than 0.");
			return false;
		}
		if (line.tokens.size() >= 4 && !CommandParsing::TryParseInt(line.tokens[3], &amount))
		{
			engine->LogWarn("Invalid amount: " + line.tokens[3]);
			return false;
		}
		if (line.tokens.size() >= 5 && !CommandParsing::TryParseInt(line.tokens[4], &aux))
		{
			engine->LogWarn("Invalid aux value: " + line.tokens[4]);
			return false;
		}
		if (amount <= 0)
		{
			engine->LogWarn("Amount must be greater than 0.");
			return false;
		}

		std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(target);
		if (player == nullptr)
		{
			engine->LogWarn("Cannot resolve target player entity.");
			return false;
		}

		std::shared_ptr<GameCommandPacket> packet = GiveItemCommand::preparePacket(player, itemId, amount, aux);
		if (packet == nullptr)
		{
			engine->LogError("Failed to build give command packet.");
			return false;
		}

		return engine->DispatchWorldCommand(packet->command, packet->data);
	}

	void CliCommandGive::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		if (context.currentTokenIndex == 1)
		{
			engine->SuggestPlayers(context.prefix, context.linePrefix, out);
		}
	}
}
