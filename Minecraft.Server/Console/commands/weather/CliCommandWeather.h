#pragma once

#include "..\IServerCliCommand.h"

namespace ServerRuntime
{
	class CliCommandWeather : public IServerCliCommand
	{
	public:
		const char *Name() const override;
		const char *Usage() const override;
		const char *Description() const override;
		bool Execute(const ServerCliParsedLine &line, ServerCliEngine *engine) override;
	};
}
