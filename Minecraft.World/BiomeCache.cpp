#include "stdafx.h"

#include "Biome.h"
#include "BiomeSource.h"
#include "BiomeCache.h"
#include "System.h"

BiomeCache::Block::Block(int x, int z, BiomeCache *parent)
{
// 	temps = floatArray(ZONE_SIZE * ZONE_SIZE, false);		// MGH - added "no clear" flag to arrayWithLength
// 	downfall = floatArray(ZONE_SIZE * ZONE_SIZE, false);
// 	biomes = BiomeArray(ZONE_SIZE * ZONE_SIZE, false);
	biomeIndices = byteArray(ZONE_SIZE * ZONE_SIZE, false);

	lastUse = 0;
	this->x = x;
	this->z = z;
// 	parent->source->getTemperatureBlock(temps, x << ZONE_SIZE_BITS, z << ZONE_SIZE_BITS, ZONE_SIZE, ZONE_SIZE);
// 	parent->source->getDownfallBlock(downfall, x << ZONE_SIZE_BITS, z << ZONE_SIZE_BITS, ZONE_SIZE, ZONE_SIZE);
// 	parent->source->getBiomeBlock(biomes, x << ZONE_SIZE_BITS, z << ZONE_SIZE_BITS, ZONE_SIZE, ZONE_SIZE, false);
	parent->source->getBiomeIndexBlock(biomeIndices, x << ZONE_SIZE_BITS, z << ZONE_SIZE_BITS, ZONE_SIZE, ZONE_SIZE, false);

}

BiomeCache::Block::~Block()
{
// 	delete [] temps.data;
// 	delete [] downfall.data;
// 	delete [] biomes.data;
	delete [] biomeIndices.data;
}

Biome *BiomeCache::Block::getBiome(int x, int z)
{
//	return biomes[(x & ZONE_SIZE_MASK) | ((z & ZONE_SIZE_MASK) << ZONE_SIZE_BITS)];

	int biomeIndex = biomeIndices[(x & ZONE_SIZE_MASK) | ((z & ZONE_SIZE_MASK) << ZONE_SIZE_BITS)];
	return Biome::biomes[biomeIndex];
}

float BiomeCache::Block::getTemperature(int x, int z)
{
//	return temps[(x & ZONE_SIZE_MASK) | ((z & ZONE_SIZE_MASK) << ZONE_SIZE_BITS)];

	int biomeIndex = biomeIndices[(x & ZONE_SIZE_MASK) | ((z & ZONE_SIZE_MASK) << ZONE_SIZE_BITS)];
	return Biome::biomes[biomeIndex]->getTemperature();

}

float BiomeCache::Block::getDownfall(int x, int z)
{
// 	return downfall[(x & ZONE_SIZE_MASK) | ((z & ZONE_SIZE_MASK) << ZONE_SIZE_BITS)];

	int biomeIndex = biomeIndices[(x & ZONE_SIZE_MASK) | ((z & ZONE_SIZE_MASK) << ZONE_SIZE_BITS)];
	return Biome::biomes[biomeIndex]->getDownfall();

}

BiomeCache::BiomeCache(BiomeSource *source)
{
	// 4J Initialisors
	lastUpdateTime = 0;

	this->source = source;

	InitializeCriticalSection(&m_CS);

}

BiomeCache::~BiomeCache()
{
	// 4J Stu - Delete source?
	// delete source;

	for( auto& it : all )
	{
		delete it;
	}
	DeleteCriticalSection(&m_CS);
}


BiomeCache::Block *BiomeCache::getBlockAt(int x, int z)
{
	EnterCriticalSection(&m_CS);
	x >>= ZONE_SIZE_BITS;
	z >>= ZONE_SIZE_BITS;
	__int64 slot = (((__int64) x) & 0xffffffffl) | ((((__int64) z) & 0xffffffffl) << 32l);
	auto it = cached.find(slot);
	Block *block = NULL;
	if (it == cached.end())
	{
		MemSect(48);
		block = new Block(x, z, this);
		cached[slot] = block;
		all.push_back(block);
		MemSect(0);
	}
	else
	{
		block = it->second;
	}
	block->lastUse = app.getAppTime();
	LeaveCriticalSection(&m_CS);
	return block;
}


Biome *BiomeCache::getBiome(int x, int z)
{
	return getBlockAt(x, z)->getBiome(x, z);
}

float BiomeCache::getTemperature(int x, int z)
{
	return getBlockAt(x, z)->getTemperature(x, z);
}

float BiomeCache::getDownfall(int x, int z)
{
	return getBlockAt(x, z)->getDownfall(x, z);
}

void BiomeCache::update()
{
	EnterCriticalSection(&m_CS);
	__int64 now = app.getAppTime();
	__int64 utime = now - lastUpdateTime;
	if (utime > DECAY_TIME / 4 || utime < 0)
	{
		lastUpdateTime = now;

		for (auto it = all.begin(); it != all.end();)
		{
			Block *block = *it;
			__int64 time = now - block->lastUse;
			if (time > DECAY_TIME || time < 0)
			{
				it = all.erase(it);
				__int64 slot = (((__int64) block->x) & 0xffffffffl) | ((((__int64) block->z) & 0xffffffffl) << 32l);
				cached.erase(slot);
				delete block;
			}
			else
			{
				++it;
			}
		}
	}
	LeaveCriticalSection(&m_CS);
}

BiomeArray BiomeCache::getBiomeBlockAt(int x, int z)
{
	byteArray indices = getBlockAt(x, z)->biomeIndices;
	BiomeArray biomes(indices.length);
	for(int i=0;i<indices.length;i++)
		biomes[i] = Biome::biomes[indices[i]];
	return biomes;
}

byteArray BiomeCache::getBiomeIndexBlockAt(int x, int z)
{
	return getBlockAt(x, z)->biomeIndices;
}