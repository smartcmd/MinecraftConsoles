#include "stdafx.h"

#include "CliCommandStop.h"

#include "..\..\ServerCliEngine.h"

namespace ServerRuntime
{
	const char *CliCommandStop::Name() const
	{
		return "stop";
	}

	const char *CliCommandStop::Usage() const
	{
		return "stop";
	}

	const char *CliCommandStop::Description() const
	{
		return "Stop the dedicated server.";
	}

	bool CliCommandStop::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		(void)line;
		engine->LogInfo("Stopping server...");
		engine->RequestShutdown();
		return true;
	}
}

