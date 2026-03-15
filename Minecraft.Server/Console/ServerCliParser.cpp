#include "stdafx.h"

#include "ServerCliParser.h"

namespace ServerRuntime
{
	static void TokenizeLine(const std::string &line, std::vector<std::string> *tokens, bool *trailingSpace)
	{
		std::string current;
		bool inQuotes = false;
		bool escaped = false;

		tokens->clear();
		*trailingSpace = false;

		for (size_t i = 0; i < line.size(); ++i)
		{
			char ch = line[i];
			if (escaped)
			{
				// Keep escaped character literally (e.g. \" or \ ).
				current.push_back(ch);
				escaped = false;
				continue;
			}

			if (ch == '\\')
			{
				escaped = true;
				continue;
			}

			if (ch == '"')
			{
				// Double quotes group spaces into one token.
				inQuotes = !inQuotes;
				continue;
			}

			if (!inQuotes && (ch == ' ' || ch == '\t'))
			{
				if (!current.empty())
				{
					tokens->push_back(current);
					current.clear();
				}
				continue;
			}

			current.push_back(ch);
		}

		if (!current.empty())
		{
			tokens->push_back(current);
		}

		if (!line.empty())
		{
			char tail = line[line.size() - 1];
			// Trailing space means completion targets the next token slot.
			*trailingSpace = (!inQuotes && (tail == ' ' || tail == '\t'));
		}
	}

	ServerCliParsedLine ServerCliParser::Parse(const std::string &line)
	{
		ServerCliParsedLine parsed;
		parsed.raw = line;
		TokenizeLine(line, &parsed.tokens, &parsed.trailingSpace);
		return parsed;
	}

	ServerCliCompletionContext ServerCliParser::BuildCompletionContext(const std::string &line)
	{
		ServerCliCompletionContext context;
		context.parsed = Parse(line);

		if (context.parsed.tokens.empty())
		{
			context.currentTokenIndex = 0;
			context.prefix.clear();
			context.linePrefix.clear();
			return context;
		}

		if (context.parsed.trailingSpace)
		{
			// Cursor is after a separator, so complete a new token.
			context.currentTokenIndex = context.parsed.tokens.size();
			context.prefix.clear();
		}
		else
		{
			// Cursor is inside current token, so complete by its prefix.
			context.currentTokenIndex = context.parsed.tokens.size() - 1;
			context.prefix = context.parsed.tokens.back();
		}

		for (size_t i = 0; i < context.currentTokenIndex; ++i)
		{
			// linePrefix is the immutable left side reused by completion output.
			if (!context.linePrefix.empty())
			{
				context.linePrefix.push_back(' ');
			}
			context.linePrefix += context.parsed.tokens[i];
		}
		if (!context.linePrefix.empty())
		{
			context.linePrefix.push_back(' ');
		}

		return context;
	}
}
