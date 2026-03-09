#pragma once

#ifdef _WINDOWS64

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <Windows.h>

namespace Win64Xuid
{
	inline PlayerUID GetLegacyEmbeddedBaseXuid()
	{
		return (PlayerUID)0xe000d45248242f2eULL;
	}

	inline PlayerUID GetLegacyEmbeddedHostXuid()
	{
		// Legacy behavior used "embedded base + smallId"; host was always smallId 0.
		// We intentionally keep this value for host/self compatibility with pre-migration worlds.
		return GetLegacyEmbeddedBaseXuid();
	}

	inline bool IsLegacyEmbeddedRange(PlayerUID xuid)
	{
		// Old Win64 XUIDs were not persistent and always lived in this narrow base+smallId range.
		// Treat them as legacy/non-persistent so uid.dat values never collide with old slot IDs.
		const PlayerUID base = GetLegacyEmbeddedBaseXuid();
		return xuid >= base && xuid < (base + MINECRAFT_NET_MAX_PLAYERS);
	}

	inline bool IsPersistedUidValid(PlayerUID xuid)
	{
		return xuid != INVALID_XUID && !IsLegacyEmbeddedRange(xuid);
	}


	// ./uid.dat
	inline bool BuildUidFilePath(char* outPath, size_t outPathSize)
	{
		if (outPath == NULL || outPathSize == 0)
			return false;

		outPath[0] = 0;

		char exePath[MAX_PATH] = {};
		DWORD len = GetModuleFileNameA(NULL, exePath, MAX_PATH);
		if (len == 0 || len >= MAX_PATH)
			return false;

		char* lastSlash = strrchr(exePath, '\\');
		if (lastSlash != NULL)
		{
			*(lastSlash + 1) = 0;
		}

		if (strcpy_s(outPath, outPathSize, exePath) != 0)
			return false;
		if (strcat_s(outPath, outPathSize, "uid.dat") != 0)
			return false;

		return true;
	}

	inline bool ReadUid(PlayerUID* outXuid)
	{
		if (outXuid == NULL)
			return false;

		char path[MAX_PATH] = {};
		if (!BuildUidFilePath(path, MAX_PATH))
			return false;

		FILE* f = NULL;
		if (fopen_s(&f, path, "rb") != 0 || f == NULL)
			return false;

		char buffer[128] = {};
		size_t readBytes = fread(buffer, 1, sizeof(buffer) - 1, f);
		fclose(f);

		if (readBytes == 0)
			return false;

		// Compatibility: earlier experiments may have written raw 8-byte uid.dat.
		if (readBytes == sizeof(uint64_t))
		{
			uint64_t raw = 0;
			memcpy(&raw, buffer, sizeof(raw));
			PlayerUID parsed = (PlayerUID)raw;
			if (IsPersistedUidValid(parsed))
			{
				*outXuid = parsed;
				return true;
			}
		}

		buffer[readBytes] = 0;
		char* begin = buffer;
		while (*begin == ' ' || *begin == '\t' || *begin == '\r' || *begin == '\n')
		{
			++begin;
		}

		errno = 0;
		char* end = NULL;
		uint64_t raw = _strtoui64(begin, &end, 0);
		if (begin == end || errno != 0)
			return false;

		while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')
		{
			++end;
		}
		if (*end != 0)
			return false;

		PlayerUID parsed = (PlayerUID)raw;
		if (!IsPersistedUidValid(parsed))
			return false;

		*outXuid = parsed;
		return true;
	}

	inline bool WriteUid(PlayerUID xuid)
	{
		char path[MAX_PATH] = {};
		if (!BuildUidFilePath(path, MAX_PATH))
			return false;

		FILE* f = NULL;
		if (fopen_s(&f, path, "wb") != 0 || f == NULL)
			return false;

		int written = fprintf_s(f, "0x%016llX\n", (unsigned long long)xuid);
		fclose(f);
		return written > 0;
	}

	inline uint64_t Mix64(uint64_t x)
	{
		x += 0x9E3779B97F4A7C15ULL;
		x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
		x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
		return x ^ (x >> 31);
	}

	inline PlayerUID GeneratePersistentUid()
	{
		// Avoid rand_s dependency: mix several Win64 runtime values into a 64-bit seed.
		FILETIME ft = {};
		GetSystemTimeAsFileTime(&ft);
		uint64_t t = (((uint64_t)ft.dwHighDateTime) << 32) | ft.dwLowDateTime;

		LARGE_INTEGER qpc = {};
		QueryPerformanceCounter(&qpc);

		uint64_t seed = t;
		seed ^= (uint64_t)qpc.QuadPart;
		seed ^= ((uint64_t)GetCurrentProcessId() << 32);
		seed ^= (uint64_t)GetCurrentThreadId();
		seed ^= (uint64_t)GetTickCount();
		seed ^= (uint64_t)(size_t)&qpc;
		seed ^= (uint64_t)(size_t)GetModuleHandleA(NULL);

		uint64_t raw = Mix64(seed) ^ Mix64(seed + 0xA0761D6478BD642FULL);
		raw ^= 0x8F4B2D6C1A93E705ULL;
		raw |= 0x8000000000000000ULL;

		PlayerUID xuid = (PlayerUID)raw;
		if (!IsPersistedUidValid(xuid))
		{
			raw ^= 0x0100000000000001ULL;
			xuid = (PlayerUID)raw;
		}

		if (!IsPersistedUidValid(xuid))
		{
			// Last-resort deterministic fallback for pathological cases.
			xuid = (PlayerUID)0xD15EA5E000000001ULL;
		}

		return xuid;
	}

	inline PlayerUID DeriveXuidForPad(PlayerUID baseXuid, int iPad)
	{
		if (iPad == 0)
			return baseXuid;

		// Deterministic per-pad XUID: hash the base XUID with the pad number.
		// Produces a fully unique 64-bit value with no risk of overlap.
		// Suggested by rtm516 to avoid adjacent-integer collisions from the old "+ iPad" approach.
		uint64_t raw = Mix64((uint64_t)baseXuid + (uint64_t)iPad);
		raw |= 0x8000000000000000ULL; // keep high bit set like all our XUIDs

		PlayerUID xuid = (PlayerUID)raw;
		if (!IsPersistedUidValid(xuid))
		{
			raw ^= 0x0100000000000001ULL;
			xuid = (PlayerUID)raw;
		}
		if (!IsPersistedUidValid(xuid))
			xuid = (PlayerUID)(0xD15EA5E000000001ULL + iPad);

		return xuid;
	}

	inline PlayerUID ResolvePersistentXuid()
	{
		// Process-local cache: uid.dat is immutable during runtime and this path is hot.
		static bool s_cached = false;
		static PlayerUID s_xuid = INVALID_XUID;

		if (s_cached)
			return s_xuid;

		PlayerUID fileXuid = INVALID_XUID;
		if (ReadUid(&fileXuid))
		{
			s_xuid = fileXuid;
			s_cached = true;
			return s_xuid;
		}

		// First launch on this client: generate once and persist to uid.dat.
		s_xuid = GeneratePersistentUid();
		WriteUid(s_xuid);
		s_cached = true;
		return s_xuid;
	}
}

#endif
