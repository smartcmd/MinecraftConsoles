#pragma once

#include <string>
#include "ServerLogger.h"

namespace ServerRuntime
{
	/**
	 * `server.properties`
	 */
	struct ServerPropertiesConfig
	{
		/** world name `level-name` */
		std::wstring worldName;
		/** world save id `level-id` */
		std::string worldSaveId;

		/** `server-port` */
		int serverPort;
		/** `server-ip` */
		std::string serverIp;
		/** `lan-advertise` */
		bool lanAdvertise;
		/** `white-list` */
		bool whiteListEnabled;
		/** `server-name` (max 16 chars at runtime) */
		std::string serverName;
		/** `max-players` */
		int maxPlayers;
		/** `level-seed` is explicitly set */
		bool hasSeed;
		/** `level-seed` */
		__int64 seed;
		/** `log-level` */
		EServerLogLevel logLevel;
		/** `autosave-interval` (seconds) */
		int autosaveIntervalSeconds;

		/** host options / game settings */
		int difficulty;
		int gameMode;
		/** `world-size` preset (`classic` / `small` / `medium` / `large`) */
		int worldSize;
		/** Overworld chunk width derived from `world-size` */
		int worldSizeChunks;
		/** Nether scale derived from `world-size` */
		int worldHellScale;
		bool levelTypeFlat;
		/** `spawn-protection` radius in blocks (0 disables protection) */
		int spawnProtectionRadius;
		bool generateStructures;
		bool bonusChest;
		bool pvp;
		bool trustPlayers;
		bool fireSpreads;
		bool tnt;
		bool spawnAnimals;
		bool spawnNpcs;
		bool spawnMonsters;
		bool allowFlight;
		bool allowNether;
		bool friendsOfFriends;
		bool gamertags;
		bool bedrockFog;
		bool hostCanFly;
		bool hostCanChangeHunger;
		bool hostCanBeInvisible;
		bool disableSaving;
		bool mobGriefing;
		bool keepInventory;
		bool doMobSpawning;
		bool doMobLoot;
		bool doTileDrops;
		bool naturalRegeneration;
		bool doDaylightCycle;

		/** other MinecraftServer runtime settings */
		int maxBuildHeight;
		std::string levelType;
		std::string motd;
	};

	/**
	 * server.properties loader
	 *
	 * - ファイル欠損時はデフォルト値で新規作成
	 * - 必須キー不足時は補完して再保存
	 * - `level-id` は保存先として安全な形式へ正規化
	 *
	 * @return `WorldManager` が利用するワールド設定
	 */
	ServerPropertiesConfig LoadServerPropertiesConfig();

	/**
	 * server.properties saver
	 *
	 * - `level-name` と `level-id` を更新
	 * - `white-list` を更新
	 * - それ以外の既存キーは極力保持
	 *
	 * @param config 保存するワールド識別情報と永続化対象設定
	 * @return 書き込み成功時 `true`
	 */
	bool SaveServerPropertiesConfig(const ServerPropertiesConfig &config);
}
