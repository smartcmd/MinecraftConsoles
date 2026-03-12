#pragma once

#include "..\IServerCliCommand.h"

namespace ServerRuntime
{
	class CliCommandStop : public IServerCliCommand
	{
	public:
		virtual const char *Name() const;
		virtual const char *Usage() const;
		virtual const char *Description() const;
		virtual bool Execute(const ServerCliParsedLine &line, ServerCliEngine *engine);
	};
}

