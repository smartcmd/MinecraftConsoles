#pragma once

#include "Mob.h"

#ifdef __PSVITA__
#include "..\Minecraft.Client\PSVita\PSVitaExtras\CustomMap.h"
#endif

class Player;
class Level;

class MobSpawner
{
private:
	// For some reason, this constant is used in Java as the dividend for the max count of a mob category in the level, thus this is done to keep parity.
    static const int CHUNK_HORIZONTAL_SCALE = 289;
	static const int MIN_SPAWN_DISTANCE;

protected:
	static TilePos getRandomPosWithin(Level *level, int cx, int cz);

private:
#ifdef __PSVITA__
	// AP - See CustomMap.h for an explanation of this
	static CustomMap chunksToPoll;
#else
	static unordered_map<ChunkPos,bool,ChunkPosKeyHash,ChunkPosKeyEq> chunksToPoll;
#endif

public:
	static const int tick(ServerLevel *level, bool spawnEnemies, bool spawnFriendlies, bool spawnPersistent);
	static bool isSpawnPositionOk(MobCategory *category, Level *level, int x, int y, int z);

public:
	static bool attackSleepingPlayers(Level *level, vector<shared_ptr<Player> > *players);

	static void postProcessSpawnMobs(Level *level, Biome *biome, int xo, int zo, int cellWidth, int cellHeight, Random *random);
};