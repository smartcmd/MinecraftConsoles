#pragma once

#include "..\IServerCliCommand.h"

namespace ServerRuntime
{
	class CliCommandDefaultGamemode : public IServerCliCommand
	{
	public:
		const char *Name() const override;
		const char *Usage() const override;
		const char *Description() const override;
		bool Execute(const ServerCliParsedLine &line, ServerCliEngine *engine) override;
		void Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const override;
	};
}
