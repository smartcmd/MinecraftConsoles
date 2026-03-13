#include "stdafx.h"

#include "CliCommandTime.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\..\..\Common\StringUtils.h"
#include "..\..\..\..\Minecraft.World\GameCommandPacket.h"
#include "..\..\..\..\Minecraft.World\TimeCommand.h"

namespace ServerRuntime
{
	namespace
	{
		constexpr const char *kTimeUsage = "time <day|night|set day|set night>";

		static bool TryResolveNightFlag(const std::vector<std::string> &tokens, bool *outNight)
		{
			if (outNight == nullptr)
			{
				return false;
			}

			std::string value;
			if (tokens.size() == 2)
			{
				value = StringUtils::ToLowerAscii(tokens[1]);
			}
			else if (tokens.size() == 3 && StringUtils::ToLowerAscii(tokens[1]) == "set")
			{
				value = StringUtils::ToLowerAscii(tokens[2]);
			}
			else
			{
				return false;
			}

			if (value == "day")
			{
				*outNight = false;
				return true;
			}
			if (value == "night")
			{
				*outNight = true;
				return true;
			}

			return false;
		}

		static void SuggestLiteral(const char *candidate, const ServerCliCompletionContext &context, std::vector<std::string> *out)
		{
			if (candidate == nullptr || out == nullptr)
			{
				return;
			}

			const std::string text(candidate);
			if (StringUtils::StartsWithIgnoreCase(text, context.prefix))
			{
				out->push_back(context.linePrefix + text);
			}
		}
	}

	const char *CliCommandTime::Name() const
	{
		return "time";
	}

	const char *CliCommandTime::Usage() const
	{
		return kTimeUsage;
	}

	const char *CliCommandTime::Description() const
	{
		return "Set day or night via Minecraft.World command dispatcher.";
	}

	bool CliCommandTime::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		bool night = false;
		if (!TryResolveNightFlag(line.tokens, &night))
		{
			engine->LogWarn(std::string("Usage: ") + kTimeUsage);
			return false;
		}

		std::shared_ptr<GameCommandPacket> packet = TimeCommand::preparePacket(night);
		if (packet == nullptr)
		{
			engine->LogError("Failed to build time command packet.");
			return false;
		}

		return engine->DispatchWorldCommand(packet->command, packet->data);
	}

	void CliCommandTime::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		(void)engine;
		if (context.currentTokenIndex == 1)
		{
			SuggestLiteral("day", context, out);
			SuggestLiteral("night", context, out);
			SuggestLiteral("set", context, out);
		}
		else if (context.currentTokenIndex == 2 &&
			context.parsed.tokens.size() >= 2 &&
			StringUtils::ToLowerAscii(context.parsed.tokens[1]) == "set")
		{
			SuggestLiteral("day", context, out);
			SuggestLiteral("night", context, out);
		}
	}
}
