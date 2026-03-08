#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace ServerRuntime
{
	class IServerCliCommand;

	/**
	 * **CLI command registry**
	 */
	class ServerCliRegistry
	{
	public:
		/**
		 * **Register a command object**
		 *
		 * Validates name/aliases and adds lookup entries.
		 * コマンドの追加
		 */
		bool Register(std::unique_ptr<IServerCliCommand> command);

		/**
		 * **Find command by name or alias (const)**
		 *
		 * Returns null when no match exists.
		 */
		const IServerCliCommand *Find(const std::string &name) const;

		/**
		 * **Find mutable command by name or alias**
		 *
		 * Used by runtime dispatch path.
		 */
		IServerCliCommand *FindMutable(const std::string &name);

		/**
		 * **Suggest top-level command names**
		 *
		 * Adds matching command names and aliases to the output list.
		 */
		void SuggestCommandNames(const std::string &prefix, const std::string &linePrefix, std::vector<std::string> *out) const;

		/**
		 * **Get registered command list**
		 *
		 * Intended for help output and inspection.
		 */
		const std::vector<std::unique_ptr<IServerCliCommand>> &Commands() const;

	private:
		std::vector<std::unique_ptr<IServerCliCommand>> m_commands;
		std::unordered_map<std::string, IServerCliCommand *> m_lookup;
	};
}
