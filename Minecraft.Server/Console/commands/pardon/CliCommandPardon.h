#pragma once

#include "..\IServerCliCommand.h"

namespace ServerRuntime
{
	/**
	 * Removes dedicated-server player bans using Java Edition style syntax and Access-backed persistence
	 */
	class CliCommandPardon : public IServerCliCommand
	{
	public:
		virtual const char *Name() const;
		virtual const char *Usage() const;
		virtual const char *Description() const;
		virtual bool Execute(const ServerCliParsedLine &line, ServerCliEngine *engine);
		virtual void Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const;
	};
}
