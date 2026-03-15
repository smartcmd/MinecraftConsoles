#pragma once

#include <cerrno>
#include <cstdlib>
#include <limits>
#include <string>

namespace ServerRuntime
{
	namespace CommandParsing
	{
		inline bool TryParseInt(const std::string &text, int *outValue)
		{
			if (outValue == nullptr || text.empty())
			{
				return false;
			}

			char *end = nullptr;
			errno = 0;
			const long parsedValue = std::strtol(text.c_str(), &end, 10);
			if (end == text.c_str() || *end != '\0')
			{
				return false;
			}
			if (errno == ERANGE)
			{
				return false;
			}
			if (parsedValue < (std::numeric_limits<int>::min)() || parsedValue > (std::numeric_limits<int>::max)())
			{
				return false;
			}

			*outValue = static_cast<int>(parsedValue);
			return true;
		}
	}
}
