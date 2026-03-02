#pragma once
#include "..\Minecraft.World\JavaIntHash.h"

class BiomeCache
{
private:
	static const int DECAY_TIME = 1000 * 30;
	static const int ZONE_SIZE_BITS = 4;
	static const int ZONE_SIZE = 1 << ZONE_SIZE_BITS;
	static const int ZONE_SIZE_MASK = ZONE_SIZE - 1;

	const BiomeSource *source;
	__int64 lastUpdateTime;

public:
	class Block
	{
	public:
		// MGH - changed this to just cache biome indices, as we have direct access to the data if we know the index.
// 		floatArray temps;
// 		floatArray downfall;
// 		BiomeArray biomes;
		byteArray biomeIndices;
		int x, z;
		__int64 lastUse;

		Block(int x, int z, BiomeCache *parent);
		~Block();
		Biome *getBiome(int x, int z);
		float getTemperature(int x, int z);
		float getDownfall(int x, int z);
	};

private:
	unordered_map<__int64,Block *,LongKeyHash,LongKeyEq> cached;		// 4J - was LongHashMap
	vector<Block *> all; // was ArrayList<Block>

public:
	BiomeCache(BiomeSource *source);
	~BiomeCache();

	Block *getBlockAt(int x, int z);
	Biome *getBiome(int x, int z);
	float getTemperature(int x, int z);
	float getDownfall(int x, int z);
	void update();
 	BiomeArray getBiomeBlockAt(int x, int z);
	byteArray getBiomeIndexBlockAt(int x, int z);

private:
	CRITICAL_SECTION m_CS;
};