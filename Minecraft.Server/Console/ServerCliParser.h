#pragma once

#include <string>
#include <vector>

namespace ServerRuntime
{
	/**
	 * **Parsed command line**
	 */
	struct ServerCliParsedLine
	{
		std::string raw;
		std::vector<std::string> tokens;
		bool trailingSpace;

		ServerCliParsedLine()
			: trailingSpace(false)
		{
		}
	};

	/**
	 * **Completion context for one input line**
	 *
	 * Indicates current token index, token prefix, and the fixed line prefix.
	 */
	struct ServerCliCompletionContext
	{
		ServerCliParsedLine parsed;
		size_t currentTokenIndex;
		std::string prefix;
		std::string linePrefix;

		ServerCliCompletionContext()
			: currentTokenIndex(0)
		{
		}
	};

	/**
	 * **CLI parser helpers**
	 *
	 * Converts raw input text into tokenized data used by execution and completion.
	 */
	class ServerCliParser
	{
	public:
		/**
		 * **Tokenize one command line**
		 *
		 * Supports quoted segments and escaped characters.
		 */
		static ServerCliParsedLine Parse(const std::string &line);

		/**
		 * **Build completion metadata**
		 *
		 * Determines active token position and reusable prefix parts.
		 */
		static ServerCliCompletionContext BuildCompletionContext(const std::string &line);
	};
}
