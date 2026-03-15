#include "stdafx.h"

#include "CliCommandHelp.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliRegistry.h"

namespace ServerRuntime
{
	const char *CliCommandHelp::Name() const
	{
		return "help";
	}

	std::vector<std::string> CliCommandHelp::Aliases() const
	{
		return { "?" };
	}

	const char *CliCommandHelp::Usage() const
	{
		return "help";
	}

	const char *CliCommandHelp::Description() const
	{
		return "Show available server console commands.";
	}

	bool CliCommandHelp::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		(void)line;
		const std::vector<std::unique_ptr<IServerCliCommand>> &commands = engine->Registry().Commands();
		engine->LogInfo("Available commands:");
		for (size_t i = 0; i < commands.size(); ++i)
		{
			std::string row = "  ";
			row += commands[i]->Usage();
			row += " - ";
			row += commands[i]->Description();
			engine->LogInfo(row);
		}
		return true;
	}
}

