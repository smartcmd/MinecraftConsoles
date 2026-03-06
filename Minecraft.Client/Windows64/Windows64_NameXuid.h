#pragma once

#ifdef _WINDOWS64

#include <string>

namespace Win64NameXuid
{
	inline PlayerUID GetLegacyEmbeddedBaseXuid()
	{
		return (PlayerUID)0xe000d45248242f2eULL;
	}

	inline PlayerUID GetLegacyEmbeddedHostXuid()
	{
		// Legacy behavior used "embedded base + smallId"; host was always smallId 0.
		return GetLegacyEmbeddedBaseXuid();
	}

	/**
	 * ## Resolves a persistent 64-bit player ID from the player's username.
	 * 
	 * We keep this deterministic so existing player save/map systems can key off XUID.
	 * 
	 * @param playerName The player's username.
	 * @return The resolved PlayerUID.
	 */
	inline PlayerUID ResolvePersistentXuidFromName(const std::wstring &playerName)
	{
		const unsigned __int64 fnvOffset = 14695981039346656037ULL;
		const unsigned __int64 fnvPrime = 1099511628211ULL;
		unsigned __int64 hash = fnvOffset;

		for (size_t i = 0; i < playerName.length(); ++i)
		{
			unsigned short codeUnit = (unsigned short)playerName[i];
			hash ^= (unsigned __int64)(codeUnit & 0xFF);
			hash *= fnvPrime;
			hash ^= (unsigned __int64)((codeUnit >> 8) & 0xFF);
			hash *= fnvPrime;
		}

		// Namespace the hash away from legacy smallId-based values.
		hash ^= 0x9E3779B97F4A7C15ULL;
		hash |= 0x8000000000000000ULL;

		if (hash == (unsigned __int64)INVALID_XUID)
		{
			hash ^= 0x0100000000000001ULL;
		}

		return (PlayerUID)hash;
	}
}

#endif

