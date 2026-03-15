#pragma once

#include <string>
#include <vector>

namespace ServerRuntime
{
	class ServerCliEngine;
	struct ServerCliParsedLine;
	struct ServerCliCompletionContext;

	/**
	 * **Command interface for server CLI**
	 *
	 * Implement this contract to add new commands without changing engine internals.
	 */
	class IServerCliCommand
	{
	public:
		virtual ~IServerCliCommand() = default;

		/** Primary command name */
		virtual const char *Name() const = 0;
		/** Optional aliases */
		virtual std::vector<std::string> Aliases() const { return {}; }
		/** Usage text for help */
		virtual const char *Usage() const = 0;
		/** Short command description*/
		virtual const char *Description() const = 0;

		/**
		 * **Execute command logic**
		 *
		 * Called after tokenization and command lookup.
		 */
		virtual bool Execute(const ServerCliParsedLine &line, ServerCliEngine *engine) = 0;

		/**
		 * **Provide argument completion candidates**
		 *
		 * Override when command-specific completion is needed.
		 */
		virtual void Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
		{
			(void)context;
			(void)engine;
			(void)out;
		}
	};
}
