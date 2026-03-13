#include "stdafx.h"

#include "CliCommandEnchant.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\CommandParsing.h"
#include "..\..\..\..\Minecraft.World\GameCommandPacket.h"
#include "..\..\..\..\Minecraft.World\EnchantItemCommand.h"
#include "..\..\..\..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\..\..\..\Minecraft.Client\ServerPlayer.h"

namespace ServerRuntime
{
	namespace
	{
		constexpr const char *kEnchantUsage = "enchant <player> <enchantId> [level]";
	}

	const char *CliCommandEnchant::Name() const
	{
		return "enchant";
	}

	const char *CliCommandEnchant::Usage() const
	{
		return kEnchantUsage;
	}

	const char *CliCommandEnchant::Description() const
	{
		return "Enchant held item via Minecraft.World command dispatcher.";
	}

	bool CliCommandEnchant::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() < 3 || line.tokens.size() > 4)
		{
			engine->LogWarn(std::string("Usage: ") + kEnchantUsage);
			return false;
		}

		std::shared_ptr<ServerPlayer> target = engine->FindPlayerByNameUtf8(line.tokens[1]);
		if (target == nullptr)
		{
			engine->LogWarn("Unknown player: " + line.tokens[1]);
			return false;
		}

		int enchantmentId = 0;
		int enchantmentLevel = 1;
		if (!CommandParsing::TryParseInt(line.tokens[2], &enchantmentId))
		{
			engine->LogWarn("Invalid enchantment id: " + line.tokens[2]);
			return false;
		}
		if (line.tokens.size() >= 4 && !CommandParsing::TryParseInt(line.tokens[3], &enchantmentLevel))
		{
			engine->LogWarn("Invalid enchantment level: " + line.tokens[3]);
			return false;
		}

		std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(target);
		if (player == nullptr)
		{
			engine->LogWarn("Cannot resolve target player entity.");
			return false;
		}

		std::shared_ptr<GameCommandPacket> packet = EnchantItemCommand::preparePacket(player, enchantmentId, enchantmentLevel);
		if (packet == nullptr)
		{
			engine->LogError("Failed to build enchant command packet.");
			return false;
		}

		return engine->DispatchWorldCommand(packet->command, packet->data);
	}

	void CliCommandEnchant::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		if (context.currentTokenIndex == 1)
		{
			engine->SuggestPlayers(context.prefix, context.linePrefix, out);
		}
	}
}
