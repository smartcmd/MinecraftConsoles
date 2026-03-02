#pragma once

class Biome;
#include "StructureFeature.h"

#include "StructureStart.h"


#include "ChunkPos.h"

// 4J Stu Added
// We can get away with a few more attempts on new-gen consoles
#ifdef _LARGE_WORLDS
#define MAX_STRONGHOLD_ATTEMPTS 30
#else
#define MAX_STRONGHOLD_ATTEMPTS 10
#endif

class StrongholdFeature : public StructureFeature
{
public:
	static const wstring OPTION_DISTANCE;
	static const wstring OPTION_COUNT;
	static const wstring OPTION_SPREAD;

public:
	static void staticCtor();
private:
	static vector<Biome *> allowedBiomes;

	bool isSpotSelected;
	static const int strongholdPos_length = 1;// Java game has 3, but xbox game only has 1 because of the world size;	// 4J added
	ChunkPos *strongholdPos[strongholdPos_length];
	double distance;
	int spread;

	void _init();

public:
	StrongholdFeature();
	StrongholdFeature(unordered_map<wstring, wstring> options);
	~StrongholdFeature();

	wstring getFeatureName();

protected:
	virtual bool isFeatureChunk(int x, int z, bool bIsSuperflat=false);
	vector<TilePos> *getGuesstimatedFeaturePositions();
	virtual StructureStart *createStructureStart(int x, int z);

public:
	class StrongholdStart : public StructureStart
	{
	public:
		static StructureStart *Create() { return new StrongholdStart(); }
	virtual EStructureStart GetType() { return eStructureStart_StrongholdStart; }

	public:
		StrongholdStart();
		StrongholdStart(Level *level, Random *random, int chunkX, int chunkZ);
	};
};
