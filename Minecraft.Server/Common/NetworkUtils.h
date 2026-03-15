#pragma once

#include "StringUtils.h"

#include <WS2tcpip.h>

namespace ServerRuntime
{
	namespace NetworkUtils
	{
		inline std::string NormalizeIpToken(const std::string &ip)
		{
			return StringUtils::ToLowerAscii(StringUtils::TrimAscii(ip));
		}

		inline bool IsIpLiteral(const std::string &text)
		{
			const std::string trimmed = StringUtils::TrimAscii(text);
			if (trimmed.empty())
			{
				return false;
			}

			IN_ADDR ipv4 = {};
			IN6_ADDR ipv6 = {};
			return InetPtonA(AF_INET, trimmed.c_str(), &ipv4) == 1 || InetPtonA(AF_INET6, trimmed.c_str(), &ipv6) == 1;
		}
	}
}
