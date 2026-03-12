#pragma once

#include "..\IServerCliCommand.h"

namespace ServerRuntime
{
	class CliCommandTp : public IServerCliCommand
	{
	public:
		virtual const char *Name() const;
		virtual std::vector<std::string> Aliases() const;
		virtual const char *Usage() const;
		virtual const char *Description() const;
		virtual bool Execute(const ServerCliParsedLine &line, ServerCliEngine *engine);
		virtual void Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const;
	};
}

