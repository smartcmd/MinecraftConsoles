#include "stdafx.h"

#include "CliCommandWeather.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\..\..\..\Minecraft.World\GameCommandPacket.h"
#include "..\..\..\..\Minecraft.World\ToggleDownfallCommand.h"

namespace ServerRuntime
{
	namespace
	{
		constexpr const char *kWeatherUsage = "weather";
	}

	const char *CliCommandWeather::Name() const
	{
		return "weather";
	}

	const char *CliCommandWeather::Usage() const
	{
		return kWeatherUsage;
	}

	const char *CliCommandWeather::Description() const
	{
		return "Toggle weather via Minecraft.World command dispatcher.";
	}

	bool CliCommandWeather::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() != 1)
		{
			engine->LogWarn(std::string("Usage: ") + kWeatherUsage);
			return false;
		}

		std::shared_ptr<GameCommandPacket> packet = ToggleDownfallCommand::preparePacket();
		if (packet == nullptr)
		{
			engine->LogError("Failed to build weather command packet.");
			return false;
		}

		return engine->DispatchWorldCommand(packet->command, packet->data);
	}
}
