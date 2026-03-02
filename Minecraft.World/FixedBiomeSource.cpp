#include "stdafx.h"
#include "Arrays.h"
#include "FixedBiomeSource.h"

FixedBiomeSource::FixedBiomeSource(Biome *fixed, float temperature, float downfall)
{
	biome = fixed;
	this->temperature = temperature;
	this->downfall = downfall;
}

Biome *FixedBiomeSource::getBiome(ChunkPos *cp)
{
	return biome;
}

Biome *FixedBiomeSource::getBiome(int x, int z)
{
	return biome;
}

float FixedBiomeSource::getTemperature(int x, int z)
{
	return temperature;
}

void FixedBiomeSource::getTemperatureBlock(floatArray& temperatures, int x, int z, int w, int h) const
{
	if (temperatures.data == NULL || temperatures.length < w * h)
	{
		if(temperatures.data != NULL) delete [] temperatures.data;
		temperatures = floatArray(w * h);
	}

	Arrays::fill(temperatures, 0, w * h, temperature);
}

floatArray FixedBiomeSource::getTemperatureBlock(int x, int z, int w, int h) const
{
	floatArray temps(w*h);
	getTemperatureBlock(temps, x, z, w, h);
	return temps;
}

// 4J - note that caller is responsible for deleting returned array. temperatures array is for output only.
void FixedBiomeSource::getTemperatureBlock(doubleArray& temperatures, int x, int z, int w, int h) const
{
	temperatures = doubleArray(w * h);

	Arrays::fill(temperatures, 0, w * h, (double)temperature);
}

void FixedBiomeSource::getDownfallBlock(floatArray &downfalls, int x, int z, int w, int h) const
{
	if (downfalls.data == NULL || downfalls.length < w * h)
	{
		if(downfalls.data != NULL) delete [] downfalls.data;
		downfalls = floatArray(w * h);
	}
	Arrays::fill(downfalls, 0, w * h, downfall);
}

floatArray FixedBiomeSource::getDownfallBlock(int x, int z, int w, int h) const
{
	floatArray downfalls(w*h);
	getDownfallBlock(downfalls, x, z, w, h);
	return downfalls;
}

float FixedBiomeSource::getDownfall(int x, int z) const
{
	return downfall;
}

void FixedBiomeSource::getDownfallBlock(doubleArray downfalls, int x, int z, int w, int h)
{
	if (downfalls.data == NULL || downfalls.length < w * h)
	{
		if(downfalls.data != NULL) delete [] downfalls.data;
		downfalls = doubleArray(w * h);
	}
	Arrays::fill(downfalls, 0, w * h, (double) downfall);
}

// 4J - caller is responsible for deleting biomes array, plus any optional arrays output if pointers are passed in (_temperatures, _downfalls)
void FixedBiomeSource::getBiomeBlock(BiomeArray& biomes, int x, int z, int w, int h, bool useCache) const
{
	MemSect(36);
	biomes = BiomeArray(w * h);
	MemSect(0);

	Arrays::fill(biomes, 0, w * h, biome);
}

// 4J - caller is responsible for deleting biomes array, plus any optional arrays output if pointers are passed in (_temperatures, _downfalls)
void FixedBiomeSource::getBiomeIndexBlock(byteArray& biomeIndices, int x, int z, int w, int h, bool useCache) const
{
	MemSect(36);
	biomeIndices = byteArray(w * h);
	MemSect(0);
	int biomeIndex = biome->id;
	Arrays::fill(biomeIndices, 0, w * h, biomeIndex);
}

// 4J-PB added in from beyond 1.8.2
// 4J - caller is responsible for deleting biomes array, plus any optional arrays output if pointers are passed in (_temperatures, _downfalls)
void FixedBiomeSource::getRawBiomeBlock(BiomeArray& biomes,int x, int z, int w, int h) const
{
	MemSect(36);
	biomes = BiomeArray(w * h);
	MemSect(0);

	Arrays::fill(biomes, 0, w * h, biome);
}

// 4J-PB added in from beyond 1.8.2
// 4J - caller is responsible for deleting biomes array, plus any optional arrays output if pointers are passed in (_temperatures, _downfalls)
BiomeArray FixedBiomeSource::getRawBiomeBlock( int x, int z, int w, int h) const
{
	BiomeArray biomes;
	getRawBiomeBlock(biomes, x, z, w, h);
	return biomes;
}

TilePos *FixedBiomeSource::findBiome(int x, int z, int r, Biome *toFind, Random *random)
{
	if (toFind == biome)
	{
		return new TilePos(x - r + random->nextInt(r * 2 + 1), 0, z - r + random->nextInt(r * 2 + 1));
	}

	return NULL;
}

TilePos *FixedBiomeSource::findBiome(int x, int z, int r, vector<Biome *> allowed, Random *random)
{
	if (find(allowed.begin(), allowed.end(), biome) != allowed.end())
	{
		return new TilePos(x - r + random->nextInt(r * 2 + 1), 0, z - r + random->nextInt(r * 2 + 1));
	}

	return NULL;
}

bool FixedBiomeSource::containsOnly(int x, int z, int r, Biome *allowed)
{
	return allowed == biome;
}

bool FixedBiomeSource::containsOnly(int x, int z, int r, vector<Biome *> allowed)
{
	return find(allowed.begin(), allowed.end(), biome) != allowed.end();
}