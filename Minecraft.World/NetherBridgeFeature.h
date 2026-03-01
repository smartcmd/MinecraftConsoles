#pragma once
#include "StructureFeature.h"
#include "StructureStart.h"
#include "biome.h"
class Random;

class NetherBridgeFeature : public StructureFeature
{

private:
	vector<Biome::MobSpawnerData *> bridgeEnemies;
	bool isSpotSelected;
	ChunkPos *netherFortressPos;

public:
	NetherBridgeFeature();
	~NetherBridgeFeature();
	wstring getFeatureName();
	vector<Biome::MobSpawnerData *> *getBridgeEnemies();
protected:
	virtual bool isFeatureChunk(int x, int z, bool bIsSuperflat);
	virtual StructureStart *createStructureStart(int x, int z);
public:
	void clearCachedBuildings();

	class NetherBridgeStart : public StructureStart
	{
public:
	static StructureStart *Create() { return new NetherBridgeStart(); }
	virtual EStructureStart GetType() { return eStructureStart_NetherBridgeStart; }

	public:
		NetherBridgeStart();
		NetherBridgeStart(Level *level, Random *random, int chunkX, int chunkZ);
	};
};
