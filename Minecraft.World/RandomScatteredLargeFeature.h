#pragma once

#include "StructureFeature.h"
#include "StructureStart.h"

class RandomScatteredLargeFeature : public StructureFeature
{
public:
	static const wstring OPTION_SPACING;

	static void staticCtor();
	static vector<Biome *> allowedBiomes;

private:
	vector<Biome::MobSpawnerData *> swamphutEnemies;
	int spacing;
	int minSeparation;

	void _init();

public:
	RandomScatteredLargeFeature();
	RandomScatteredLargeFeature(unordered_map<wstring, wstring> options);

	wstring getFeatureName();

protected:
	virtual bool isFeatureChunk(int x, int z, bool bIsSuperflat=false);
	StructureStart *createStructureStart(int x, int z);

public:
	class ScatteredFeatureStart : public StructureStart
	{
	public:
		static StructureStart *Create() { return new ScatteredFeatureStart(); }
		virtual EStructureStart GetType() { return eStructureStart_ScatteredFeatureStart; }

	public:
		ScatteredFeatureStart();
		ScatteredFeatureStart(Level *level, Random *random, int chunkX, int chunkZ);
	};

public:
	bool isSwamphut(int cellX, int cellY, int cellZ);
	vector<Biome::MobSpawnerData *> *getSwamphutEnemies();
};