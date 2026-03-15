#include "stdafx.h"

#include "CliCommandExperience.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\CommandParsing.h"
#include "..\..\..\Common\StringUtils.h"
#include "..\..\..\..\Minecraft.Client\MinecraftServer.h"
#include "..\..\..\..\Minecraft.Client\PlayerList.h"
#include "..\..\..\..\Minecraft.Client\ServerPlayer.h"

#include <limits>

namespace ServerRuntime
{
	namespace
	{
		constexpr const char *kExperienceUsage = "xp <amount>[L] [player]";
		constexpr const char *kExperienceUsageWithPlayer = "xp <amount>[L] <player>";

		struct ExperienceAmount
		{
			int amount = 0;
			bool levels = false;
			bool take = false;
		};

		static bool TryParseExperienceAmount(const std::string &token, ExperienceAmount *outValue)
		{
			if (outValue == nullptr || token.empty())
			{
				return false;
			}

			ExperienceAmount parsed;
			std::string numericToken = token;
			const char suffix = token[token.size() - 1];
			if (suffix == 'l' || suffix == 'L')
			{
				parsed.levels = true;
				numericToken = token.substr(0, token.size() - 1);
				if (numericToken.empty())
				{
					return false;
				}
			}

			int signedAmount = 0;
			if (!CommandParsing::TryParseInt(numericToken, &signedAmount))
			{
				return false;
			}
			if (signedAmount == (std::numeric_limits<int>::min)())
			{
				return false;
			}

			parsed.take = signedAmount < 0;
			parsed.amount = parsed.take ? -signedAmount : signedAmount;
			*outValue = parsed;
			return true;
		}

		static std::shared_ptr<ServerPlayer> ResolveTargetPlayer(const ServerCliParsedLine &line, ServerCliEngine *engine)
		{
			if (line.tokens.size() >= 3)
			{
				return engine->FindPlayerByNameUtf8(line.tokens[2]);
			}

			MinecraftServer *server = MinecraftServer::getInstance();
			if (server == nullptr || server->getPlayers() == nullptr)
			{
				return nullptr;
			}

			PlayerList *players = server->getPlayers();
			if (players->players.size() == 1 && players->players[0] != nullptr)
			{
				return players->players[0];
			}

			return nullptr;
		}
	}

	const char *CliCommandExperience::Name() const
	{
		return "xp";
	}

	std::vector<std::string> CliCommandExperience::Aliases() const
	{
		return { "experience" };
	}

	const char *CliCommandExperience::Usage() const
	{
		return kExperienceUsage;
	}

	const char *CliCommandExperience::Description() const
	{
		return "Grant or remove experience (server-side implementation).";
	}

	bool CliCommandExperience::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() < 2 || line.tokens.size() > 3)
		{
			engine->LogWarn(std::string("Usage: ") + kExperienceUsage);
			return false;
		}

		ExperienceAmount amount;
		if (!TryParseExperienceAmount(line.tokens[1], &amount))
		{
			engine->LogWarn(std::string("Usage: ") + kExperienceUsage);
			return false;
		}

		std::shared_ptr<ServerPlayer> target = ResolveTargetPlayer(line, engine);
		if (target == nullptr)
		{
			if (line.tokens.size() >= 3)
			{
				engine->LogWarn("Unknown player: " + line.tokens[2]);
			}
			else
			{
				engine->LogWarn(std::string("Usage: ") + kExperienceUsageWithPlayer);
			}
			return false;
		}

		if (amount.levels)
		{
			target->giveExperienceLevels(amount.take ? -amount.amount : amount.amount);
			if (amount.take)
			{
				engine->LogInfo("Removed " + std::to_string(amount.amount) + " level(s) from " + StringUtils::WideToUtf8(target->getName()) + ".");
			}
			else
			{
				engine->LogInfo("Added " + std::to_string(amount.amount) + " level(s) to " + StringUtils::WideToUtf8(target->getName()) + ".");
			}
			return true;
		}

		if (amount.take)
		{
			engine->LogWarn("Removing raw experience points is not supported. Use negative levels (example: xp -5L <player>).");
			return false;
		}

		target->increaseXp(amount.amount);
		engine->LogInfo("Added " + std::to_string(amount.amount) + " experience points to " + StringUtils::WideToUtf8(target->getName()) + ".");
		return true;
	}

	void CliCommandExperience::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		if (context.currentTokenIndex == 1)
		{
			if (StringUtils::StartsWithIgnoreCase("10", context.prefix))
			{
				out->push_back(context.linePrefix + "10");
			}
			if (StringUtils::StartsWithIgnoreCase("10L", context.prefix))
			{
				out->push_back(context.linePrefix + "10L");
			}
			if (StringUtils::StartsWithIgnoreCase("-5L", context.prefix))
			{
				out->push_back(context.linePrefix + "-5L");
			}
		}
		else if (context.currentTokenIndex == 2)
		{
			engine->SuggestPlayers(context.prefix, context.linePrefix, out);
		}
	}
}
