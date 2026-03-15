#include "stdafx.h"

#include "ServerProperties.h"

#include "ServerLogger.h"
#include "Common\\StringUtils.h"
#include "Common\\FileUtils.h"
#include "..\\Minecraft.World\\ChunkSource.h"

#include <cctype>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>

namespace ServerRuntime
{
using StringUtils::ToLowerAscii;
using StringUtils::TrimAscii;
using StringUtils::StripUtf8Bom;
using StringUtils::Utf8ToWide;
using StringUtils::WideToUtf8;

struct ServerPropertyDefault
{
	const char *key;
	const char *value;
};

static const char *kServerPropertiesPath = "server.properties";
static const size_t kMaxSaveIdLength = 31;

static const int kDefaultServerPort = 25565;
static const int kDefaultMaxPlayers = 16;
static const int kMaxDedicatedPlayers = 256;
static const int kDefaultAutosaveIntervalSeconds = 60;
static const char *kLanAdvertisePropertyKey = "lan-advertise";

static const ServerPropertyDefault kServerPropertyDefaults[] =
{
	{ "allow-flight", "true" },
	{ "allow-nether", "true" },
	{ "autosave-interval", "60" },
	{ "bedrock-fog", "true" },
	{ "bonus-chest", "false" },
	{ "difficulty", "1" },
	{ "disable-saving", "false" },
	{ "do-daylight-cycle", "true" },
	{ "do-mob-loot", "true" },
	{ "do-mob-spawning", "true" },
	{ "do-tile-drops", "true" },
	{ "fire-spreads", "true" },
	{ "friends-of-friends", "false" },
	{ "gamemode", "0" },
	{ "gamertags", "true" },
	{ "generate-structures", "true" },
	{ "host-can-be-invisible", "true" },
	{ "host-can-change-hunger", "true" },
	{ "host-can-fly", "true" },
	{ "keep-inventory", "false" },
	{ "level-id", "world" },
	{ "level-name", "world" },
	{ "level-seed", "" },
	{ "level-type", "default" },
	{ "world-size", "classic" },
	{ "spawn-protection", "0" },
	{ "log-level", "info" },
	{ "max-build-height", "256" },
	{ "max-players", "16" },
	{ "mob-griefing", "true" },
	{ "motd", "A Minecraft Server" },
	{ "natural-regeneration", "true" },
	{ "pvp", "true" },
	{ "server-ip", "0.0.0.0" },
	{ "server-name", "DedicatedServer" },
	{ "server-port", "25565" },
	{ "white-list", "false" },
	{ "lan-advertise", "false" },
	{ "spawn-animals", "true" },
	{ "spawn-monsters", "true" },
	{ "spawn-npcs", "true" },
	{ "tnt", "true" },
	{ "trust-players", "true" }
};

static std::string BoolToString(bool value)
{
	return value ? "true" : "false";
}

static std::string IntToString(int value)
{
	char buffer[32] = {};
	sprintf_s(buffer, sizeof(buffer), "%d", value);
	return std::string(buffer);
}

static std::string Int64ToString(__int64 value)
{
	char buffer[64] = {};
	_i64toa_s(value, buffer, sizeof(buffer), 10);
	return std::string(buffer);
}

static int ClampInt(int value, int minValue, int maxValue)
{
	if (value < minValue)
	{
		return minValue;
	}
	if (value > maxValue)
	{
		return maxValue;
	}
	return value;
}

static bool TryParseBool(const std::string &value, bool *outValue)
{
	if (outValue == NULL)
	{
		return false;
	}

	std::string lowered = ToLowerAscii(TrimAscii(value));
	if (lowered == "true" || lowered == "1" || lowered == "yes" || lowered == "on")
	{
		*outValue = true;
		return true;
	}
	if (lowered == "false" || lowered == "0" || lowered == "no" || lowered == "off")
	{
		*outValue = false;
		return true;
	}
	return false;
}

static bool TryParseInt(const std::string &value, int *outValue)
{
	if (outValue == NULL)
	{
		return false;
	}

	std::string trimmed = TrimAscii(value);
	if (trimmed.empty())
	{
		return false;
	}

	char *end = NULL;
	long parsed = strtol(trimmed.c_str(), &end, 10);
	if (end == trimmed.c_str() || *end != 0)
	{
		return false;
	}

	*outValue = (int)parsed;
	return true;
}

static bool TryParseInt64(const std::string &value, __int64 *outValue)
{
	if (outValue == NULL)
	{
		return false;
	}

	std::string trimmed = TrimAscii(value);
	if (trimmed.empty())
	{
		return false;
	}

	char *end = NULL;
	__int64 parsed = _strtoi64(trimmed.c_str(), &end, 10);
	if (end == trimmed.c_str() || *end != 0)
	{
		return false;
	}

	*outValue = parsed;
	return true;
}

static std::string LogLevelToPropertyValue(EServerLogLevel level)
{
	switch (level)
	{
	case eServerLogLevel_Debug:
		return "debug";
	case eServerLogLevel_Warn:
		return "warn";
	case eServerLogLevel_Error:
		return "error";
	case eServerLogLevel_Info:
	default:
		return "info";
	}
}

/**
 * **Normalize Save ID**
 *
 * Normalizes an arbitrary string into a safe save destination ID
 * Conversion rules:
 * - Lowercase alphabetic characters
 * - Keep only `[a-z0-9_.-]`
 * - Replace spaces and unsupported characters with `_`
 * - Fallback to `world` when empty
 * - Enforce max length to match storage constraints
 * 保存先IDの正規化処理
 */
static std::string NormalizeSaveId(const std::string &source)
{
	std::string out;
	out.reserve(source.length());

	// Normalize into a character set that is safe for storage save IDs
	// Replace invalid characters with '_' and fold letter case to reduce collisions
	for (size_t i = 0; i < source.length(); ++i)
	{
		unsigned char ch = (unsigned char)source[i];
		if (ch >= 'A' && ch <= 'Z')
		{
			ch = (unsigned char)(ch - 'A' + 'a');
		}

		const bool alnum = (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9');
		const bool passthrough = (ch == '_') || (ch == '-') || (ch == '.');
		if (alnum || passthrough)
		{
			out.push_back((char)ch);
		}
		else if (std::isspace(ch))
		{
			out.push_back('_');
		}
		else if (ch < 0x80)
		{
			out.push_back('_');
		}
	}

	if (out.empty())
	{
		out = "world";
	}

	// Add a prefix when needed to avoid awkward leading characters
	if (!((out[0] >= 'a' && out[0] <= 'z') || (out[0] >= '0' && out[0] <= '9')))
	{
		out = std::string("w_") + out;
	}

	// Clamp length to the 4J-side filename buffer constraint
	if (out.length() > kMaxSaveIdLength)
	{
		out.resize(kMaxSaveIdLength);
	}

	return out;
}

static void ApplyDefaultServerProperties(std::unordered_map<std::string, std::string> *properties)
{
	if (properties == NULL)
	{
		return;
	}

	const size_t defaultCount = sizeof(kServerPropertyDefaults) / sizeof(kServerPropertyDefaults[0]);
	for (size_t i = 0; i < defaultCount; ++i)
	{
		(*properties)[kServerPropertyDefaults[i].key] = kServerPropertyDefaults[i].value;
	}
}

/**
 * **Parse server.properties Text**
 *
 * Extracts key/value pairs from `server.properties` format text
 * - Ignores lines starting with `#` or `!` as comments
 * - Accepts `=` or `:` as separators
 * - Skips invalid lines and continues
 * server.propertiesのパース処理
 */
static bool ReadServerPropertiesFile(const char *filePath, std::unordered_map<std::string, std::string> *properties, int *outParsedCount)
{
	if (properties == NULL)
	{
		return false;
	}

	std::string text;
	if (filePath == NULL || !FileUtils::ReadTextFile(filePath, &text))
	{
		return false;
	}

	text = StripUtf8Bom(text);

	int parsedCount = 0;
	for (size_t start = 0; start <= text.length();)
	{
		size_t end = text.find_first_of("\r\n", start);
		size_t nextStart = text.length() + 1;
		if (end != std::string::npos)
		{
			nextStart = end + 1;
			if (text[end] == '\r' && nextStart < text.length() && text[nextStart] == '\n')
			{
				++nextStart;
			}
		}

		std::string line;
		if (end == std::string::npos)
		{
			line = text.substr(start);
		}
		else
		{
			line = text.substr(start, end - start);
		}

		std::string trimmedLine = TrimAscii(line);
		if (trimmedLine.empty())
		{
			start = nextStart;
			continue;
		}

		if (trimmedLine[0] == '#' || trimmedLine[0] == '!')
		{
			start = nextStart;
			continue;
		}

		size_t eqPos = trimmedLine.find('=');
		size_t colonPos = trimmedLine.find(':');
		size_t sepPos = std::string::npos;
		if (eqPos == std::string::npos)
		{
			sepPos = colonPos;
		}
		else if (colonPos == std::string::npos)
		{
			sepPos = eqPos;
		}
		else
		{
			sepPos = (eqPos < colonPos) ? eqPos : colonPos;
		}

		if (sepPos == std::string::npos)
		{
			start = nextStart;
			continue;
		}

		std::string key = TrimAscii(trimmedLine.substr(0, sepPos));
		if (key.empty())
		{
			start = nextStart;
			continue;
		}

		std::string value = TrimAscii(trimmedLine.substr(sepPos + 1));
		(*properties)[key] = value;
		++parsedCount;
		start = nextStart;
	}

	if (outParsedCount != NULL)
	{
		*outParsedCount = parsedCount;
	}

	return true;
}

/**
 * **Write server.properties Text**
 *
 * Writes key/value data back as `server.properties`
 * Sorts keys before writing to keep output order stable
 * server.propertiesの書き戻し処理
 */
static bool WriteServerPropertiesFile(const char *filePath, const std::unordered_map<std::string, std::string> &properties)
{
	if (filePath == NULL)
	{
		return false;
	}

	std::string text;
	text += "# Minecraft server properties\n";
	text += "# Auto-generated and normalized when missing\n";

	std::map<std::string, std::string> sortedProperties(properties.begin(), properties.end());
	for (std::map<std::string, std::string>::const_iterator it = sortedProperties.begin(); it != sortedProperties.end(); ++it)
	{
		text += it->first;
		text += "=";
		text += it->second;
		text += "\n";
	}

	return FileUtils::WriteTextFileAtomic(filePath, text);
}

static bool ReadNormalizedBoolProperty(
	std::unordered_map<std::string, std::string> *properties,
	const char *key,
	bool defaultValue,
	bool *shouldWrite)
{
	std::string raw = TrimAscii((*properties)[key]);
	bool value = defaultValue;
	if (!TryParseBool(raw, &value))
	{
		value = defaultValue;
	}

	std::string normalized = BoolToString(value);
	if (raw != normalized)
	{
		(*properties)[key] = normalized;
		if (shouldWrite != NULL)
		{
			*shouldWrite = true;
		}
	}

	return value;
}

static int ReadNormalizedIntProperty(
	std::unordered_map<std::string, std::string> *properties,
	const char *key,
	int defaultValue,
	int minValue,
	int maxValue,
	bool *shouldWrite)
{
	std::string raw = TrimAscii((*properties)[key]);
	int value = defaultValue;
	if (!TryParseInt(raw, &value))
	{
		value = defaultValue;
	}
	value = ClampInt(value, minValue, maxValue);

	std::string normalized = IntToString(value);
	if (raw != normalized)
	{
		(*properties)[key] = normalized;
		if (shouldWrite != NULL)
		{
			*shouldWrite = true;
		}
	}

	return value;
}

static std::string ReadNormalizedStringProperty(
	std::unordered_map<std::string, std::string> *properties,
	const char *key,
	const std::string &defaultValue,
	size_t maxLength,
	bool *shouldWrite)
{
	std::string value = TrimAscii((*properties)[key]);
	if (value.empty())
	{
		value = defaultValue;
	}
	if (maxLength > 0 && value.length() > maxLength)
	{
		value.resize(maxLength);
	}

	if (value != (*properties)[key])
	{
		(*properties)[key] = value;
		if (shouldWrite != NULL)
		{
			*shouldWrite = true;
		}
	}

	return value;
}

static bool ReadNormalizedOptionalInt64Property(
	std::unordered_map<std::string, std::string> *properties,
	const char *key,
	__int64 *outValue,
	bool *shouldWrite)
{
	std::string raw = TrimAscii((*properties)[key]);
	if (raw.empty())
	{
		if ((*properties)[key] != "")
		{
			(*properties)[key] = "";
			if (shouldWrite != NULL)
			{
				*shouldWrite = true;
			}
		}
		return false;
	}

	__int64 parsed = 0;
	if (!TryParseInt64(raw, &parsed))
	{
		(*properties)[key] = "";
		if (shouldWrite != NULL)
		{
			*shouldWrite = true;
		}
		return false;
	}

	std::string normalized = Int64ToString(parsed);
	if (raw != normalized)
	{
		(*properties)[key] = normalized;
		if (shouldWrite != NULL)
		{
			*shouldWrite = true;
		}
	}

	if (outValue != NULL)
	{
		*outValue = parsed;
	}
	return true;
}

static EServerLogLevel ReadNormalizedLogLevelProperty(
	std::unordered_map<std::string, std::string> *properties,
	const char *key,
	EServerLogLevel defaultValue,
	bool *shouldWrite)
{
	std::string raw = TrimAscii((*properties)[key]);
	EServerLogLevel value = defaultValue;
	if (!TryParseServerLogLevel(raw.c_str(), &value))
	{
		value = defaultValue;
	}

	std::string normalized = LogLevelToPropertyValue(value);
	if (raw != normalized)
	{
		(*properties)[key] = normalized;
		if (shouldWrite != NULL)
		{
			*shouldWrite = true;
		}
	}

	return value;
}

static std::string ReadNormalizedLevelTypeProperty(
	std::unordered_map<std::string, std::string> *properties,
	const char *key,
	bool *outIsFlat,
	bool *shouldWrite)
{
	std::string raw = TrimAscii((*properties)[key]);
	std::string lowered = ToLowerAscii(raw);

	bool isFlat = false;
	std::string normalized = "default";
	if (lowered == "flat" || lowered == "superflat" || lowered == "1")
	{
		isFlat = true;
		normalized = "flat";
	}
	else if (lowered == "default" || lowered == "normal" || lowered == "0")
	{
		isFlat = false;
		normalized = "default";
	}

	if (raw != normalized)
	{
		(*properties)[key] = normalized;
		if (shouldWrite != NULL)
		{
			*shouldWrite = true;
		}
	}

	if (outIsFlat != NULL)
	{
		*outIsFlat = isFlat;
	}

	return normalized;
}

static std::string WorldSizeToPropertyValue(int worldSize)
{
	switch (worldSize)
	{
	case e_worldSize_Small:
		return "small";
	case e_worldSize_Medium:
		return "medium";
	case e_worldSize_Large:
		return "large";
	case e_worldSize_Classic:
	default:
		return "classic";
	}
}

static int WorldSizeToXzChunks(int worldSize)
{
	switch (worldSize)
	{
	case e_worldSize_Small:
		return LEVEL_WIDTH_SMALL;
	case e_worldSize_Medium:
		return LEVEL_WIDTH_MEDIUM;
	case e_worldSize_Large:
		return LEVEL_WIDTH_LARGE;
	case e_worldSize_Classic:
	default:
		return LEVEL_WIDTH_CLASSIC;
	}
}

static int WorldSizeToHellScale(int worldSize)
{
	switch (worldSize)
	{
	case e_worldSize_Small:
		return HELL_LEVEL_SCALE_SMALL;
	case e_worldSize_Medium:
		return HELL_LEVEL_SCALE_MEDIUM;
	case e_worldSize_Large:
		return HELL_LEVEL_SCALE_LARGE;
	case e_worldSize_Classic:
	default:
		return HELL_LEVEL_SCALE_CLASSIC;
	}
}

static bool TryParseWorldSize(const std::string &lowered, int *outWorldSize)
{
	if (outWorldSize == NULL)
	{
		return false;
	}

	if (lowered == "classic" || lowered == "54" || lowered == "1")
	{
		*outWorldSize = e_worldSize_Classic;
		return true;
	}
	if (lowered == "small" || lowered == "64" || lowered == "2")
	{
		*outWorldSize = e_worldSize_Small;
		return true;
	}
	if (lowered == "medium" || lowered == "192" || lowered == "3")
	{
		*outWorldSize = e_worldSize_Medium;
		return true;
	}
	if (lowered == "large" || lowered == "320" || lowered == "4")
	{
		*outWorldSize = e_worldSize_Large;
		return true;
	}

	return false;
}

static int ReadNormalizedWorldSizeProperty(
	std::unordered_map<std::string, std::string> *properties,
	const char *key,
	int defaultWorldSize,
	int *outXzChunks,
	int *outHellScale,
	bool *shouldWrite)
{
	std::string raw = TrimAscii((*properties)[key]);
	std::string lowered = ToLowerAscii(raw);

	int worldSize = defaultWorldSize;
	if (!raw.empty())
	{
		int parsedWorldSize = defaultWorldSize;
		if (TryParseWorldSize(lowered, &parsedWorldSize))
		{
			worldSize = parsedWorldSize;
		}
	}

	std::string normalized = WorldSizeToPropertyValue(worldSize);
	if (raw != normalized)
	{
		(*properties)[key] = normalized;
		if (shouldWrite != NULL)
		{
			*shouldWrite = true;
		}
	}

	if (outXzChunks != NULL)
	{
		*outXzChunks = WorldSizeToXzChunks(worldSize);
	}
	if (outHellScale != NULL)
	{
		*outHellScale = WorldSizeToHellScale(worldSize);
	}

	return worldSize;
}

/**
 * **Load Effective Server Properties Config**
 *
 * Loads effective world settings, repairs missing or invalid values, and returns normalized config
 * - Creates defaults when file is missing
 * - Fills required keys when absent
 * - Normalizes `level-id` to a safe format
 * - Auto-saves when any fix is applied
 * 実効設定の読み込みと補正処理
 */
ServerPropertiesConfig LoadServerPropertiesConfig()
{
	ServerPropertiesConfig config;

	std::unordered_map<std::string, std::string> defaults;
	std::unordered_map<std::string, std::string> loaded;
	ApplyDefaultServerProperties(&defaults);

	int parsedCount = 0;
	bool readSuccess = ReadServerPropertiesFile(kServerPropertiesPath, &loaded, &parsedCount);
	std::unordered_map<std::string, std::string> merged = defaults;
	bool shouldWrite = false;

	if (!readSuccess)
	{
		LogWorldIO("server.properties not found or unreadable; creating defaults");
		shouldWrite = true;
	}
	else
	{
		if (parsedCount == 0)
		{
			LogWorldIO("server.properties has no properties; applying defaults");
			shouldWrite = true;
		}

		const size_t defaultCount = sizeof(kServerPropertyDefaults) / sizeof(kServerPropertyDefaults[0]);
		for (size_t i = 0; i < defaultCount; ++i)
		{
			if (loaded.find(kServerPropertyDefaults[i].key) == loaded.end())
			{
				shouldWrite = true;
				break;
			}
		}
	}

	for (std::unordered_map<std::string, std::string>::const_iterator it = loaded.begin(); it != loaded.end(); ++it)
	{
		// Merge loaded values over defaults and keep unknown keys whenever possible
		merged[it->first] = it->second;
	}

	std::string worldName = TrimAscii(merged["level-name"]);
	if (worldName.empty())
	{
		worldName = "world";
		shouldWrite = true;
	}

	std::string worldSaveId = TrimAscii(merged["level-id"]);
	if (worldSaveId.empty())
	{
		// If level-id is missing, derive it from level-name to lock save destination
		worldSaveId = NormalizeSaveId(worldName);
		shouldWrite = true;
	}
	else
	{
		// Normalize existing level-id as well to avoid future inconsistencies
		std::string normalized = NormalizeSaveId(worldSaveId);
		if (normalized != worldSaveId)
		{
			worldSaveId = normalized;
			shouldWrite = true;
		}
	}

	merged["level-name"] = worldName;
	merged["level-id"] = worldSaveId;

	config.worldName = Utf8ToWide(worldName.c_str());
	config.worldSaveId = worldSaveId;

	config.serverPort = ReadNormalizedIntProperty(&merged, "server-port", kDefaultServerPort, 1, 65535, &shouldWrite);
	config.serverIp = ReadNormalizedStringProperty(&merged, "server-ip", "0.0.0.0", 255, &shouldWrite);
	config.lanAdvertise = ReadNormalizedBoolProperty(&merged, kLanAdvertisePropertyKey, false, &shouldWrite);
	config.whiteListEnabled = ReadNormalizedBoolProperty(&merged, "white-list", false, &shouldWrite);
	config.serverName = ReadNormalizedStringProperty(&merged, "server-name", "DedicatedServer", 16, &shouldWrite);
	config.maxPlayers = ReadNormalizedIntProperty(&merged, "max-players", kDefaultMaxPlayers, 1, kMaxDedicatedPlayers, &shouldWrite);
	config.seed = 0;
	config.hasSeed = ReadNormalizedOptionalInt64Property(&merged, "level-seed", &config.seed, &shouldWrite);
	config.logLevel = ReadNormalizedLogLevelProperty(&merged, "log-level", eServerLogLevel_Info, &shouldWrite);
	config.autosaveIntervalSeconds = ReadNormalizedIntProperty(&merged, "autosave-interval", kDefaultAutosaveIntervalSeconds, 5, 3600, &shouldWrite);

	config.difficulty = ReadNormalizedIntProperty(&merged, "difficulty", 1, 0, 3, &shouldWrite);
	config.gameMode = ReadNormalizedIntProperty(&merged, "gamemode", 0, 0, 1, &shouldWrite);
	config.worldSize = ReadNormalizedWorldSizeProperty(
		&merged,
		"world-size",
		e_worldSize_Classic,
		&config.worldSizeChunks,
		&config.worldHellScale,
		&shouldWrite);
	config.levelType = ReadNormalizedLevelTypeProperty(&merged, "level-type", &config.levelTypeFlat, &shouldWrite);
	config.spawnProtectionRadius = ReadNormalizedIntProperty(&merged, "spawn-protection", 0, 0, 256, &shouldWrite);
	config.generateStructures = ReadNormalizedBoolProperty(&merged, "generate-structures", true, &shouldWrite);
	config.bonusChest = ReadNormalizedBoolProperty(&merged, "bonus-chest", false, &shouldWrite);
	config.pvp = ReadNormalizedBoolProperty(&merged, "pvp", true, &shouldWrite);
	config.trustPlayers = ReadNormalizedBoolProperty(&merged, "trust-players", true, &shouldWrite);
	config.fireSpreads = ReadNormalizedBoolProperty(&merged, "fire-spreads", true, &shouldWrite);
	config.tnt = ReadNormalizedBoolProperty(&merged, "tnt", true, &shouldWrite);
	config.spawnAnimals = ReadNormalizedBoolProperty(&merged, "spawn-animals", true, &shouldWrite);
	config.spawnNpcs = ReadNormalizedBoolProperty(&merged, "spawn-npcs", true, &shouldWrite);
	config.spawnMonsters = ReadNormalizedBoolProperty(&merged, "spawn-monsters", true, &shouldWrite);
	config.allowFlight = ReadNormalizedBoolProperty(&merged, "allow-flight", true, &shouldWrite);
	config.allowNether = ReadNormalizedBoolProperty(&merged, "allow-nether", true, &shouldWrite);
	config.friendsOfFriends = ReadNormalizedBoolProperty(&merged, "friends-of-friends", false, &shouldWrite);
	config.gamertags = ReadNormalizedBoolProperty(&merged, "gamertags", true, &shouldWrite);
	config.bedrockFog = ReadNormalizedBoolProperty(&merged, "bedrock-fog", true, &shouldWrite);
	config.hostCanFly = ReadNormalizedBoolProperty(&merged, "host-can-fly", true, &shouldWrite);
	config.hostCanChangeHunger = ReadNormalizedBoolProperty(&merged, "host-can-change-hunger", true, &shouldWrite);
	config.hostCanBeInvisible = ReadNormalizedBoolProperty(&merged, "host-can-be-invisible", true, &shouldWrite);
	config.disableSaving = ReadNormalizedBoolProperty(&merged, "disable-saving", false, &shouldWrite);
	config.mobGriefing = ReadNormalizedBoolProperty(&merged, "mob-griefing", true, &shouldWrite);
	config.keepInventory = ReadNormalizedBoolProperty(&merged, "keep-inventory", false, &shouldWrite);
	config.doMobSpawning = ReadNormalizedBoolProperty(&merged, "do-mob-spawning", true, &shouldWrite);
	config.doMobLoot = ReadNormalizedBoolProperty(&merged, "do-mob-loot", true, &shouldWrite);
	config.doTileDrops = ReadNormalizedBoolProperty(&merged, "do-tile-drops", true, &shouldWrite);
	config.naturalRegeneration = ReadNormalizedBoolProperty(&merged, "natural-regeneration", true, &shouldWrite);
	config.doDaylightCycle = ReadNormalizedBoolProperty(&merged, "do-daylight-cycle", true, &shouldWrite);

	config.maxBuildHeight = ReadNormalizedIntProperty(&merged, "max-build-height", 256, 64, 256, &shouldWrite);
	config.motd = ReadNormalizedStringProperty(&merged, "motd", "A Minecraft Server", 255, &shouldWrite);

	if (shouldWrite)
	{
		if (WriteServerPropertiesFile(kServerPropertiesPath, merged))
		{
			LogWorldIO("wrote server.properties");
		}
		else
		{
			LogWorldIO("failed to write server.properties");
		}
	}

	return config;
}

/**
 * **Save World Identity While Preserving Other Keys**
 *
 * Saves world identity fields while preserving as many other settings as possible
 * - Reads existing file and merges including unknown keys
 * - Updates `level-name`, `level-id`, and `white-list` before writing back
 * ワールド識別情報の保存処理
 */
bool SaveServerPropertiesConfig(const ServerPropertiesConfig &config)
{
	std::unordered_map<std::string, std::string> merged;
	ApplyDefaultServerProperties(&merged);

	std::unordered_map<std::string, std::string> loaded;
	int parsedCount = 0;
	if (ReadServerPropertiesFile(kServerPropertiesPath, &loaded, &parsedCount))
	{
		for (std::unordered_map<std::string, std::string>::const_iterator it = loaded.begin(); it != loaded.end(); ++it)
		{
			// Keep existing content so keys untouched by caller are not dropped
			merged[it->first] = it->second;
		}
	}

	std::string worldName = TrimAscii(WideToUtf8(config.worldName));
	if (worldName.empty())
	{
		worldName = "world"; // Default world name
	}

	std::string worldSaveId = TrimAscii(config.worldSaveId);
	if (worldSaveId.empty())
	{
		worldSaveId = NormalizeSaveId(worldName);
	}
	else
	{
		worldSaveId = NormalizeSaveId(worldSaveId);
	}

	merged["level-name"] = worldName;
	merged["level-id"] = worldSaveId;
	merged["white-list"] = BoolToString(config.whiteListEnabled);

	return WriteServerPropertiesFile(kServerPropertiesPath, merged);
}
}

