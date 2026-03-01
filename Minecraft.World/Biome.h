#pragma once
using namespace std;

#include "LevelSource.h"
#include "Mob.h"
#include "WeighedRandom.h"

class Feature;
class MobCategory;
class BiomeDecorator;
class TreeFeature;
class BasicTree;
class BirchFeature;
class SwampTreeFeature;
class ChunkRebuildData;

class Biome
{
	friend class ChunkRebuildData;
public:
	// 4J JEV, replaces the static blocks.
	static void staticCtor();

	static Biome *biomes[256];

	static Biome *ocean;
	static Biome *plains;
	static Biome *desert;
	static Biome *extremeHills;
	static Biome *forest;
	static Biome *taiga;
	static Biome *swampland;
	static Biome *river;
	static Biome *hell;
	static Biome *sky;
	static Biome *frozenOcean;
	static Biome *frozenRiver;
	static Biome *iceFlats;
	static Biome *iceMountains;
	static Biome *mushroomIsland;
	static Biome *mushroomIslandShore ;
	static Biome *beaches;
	static Biome *desertHills;
	static Biome *forestHills;
	static Biome *taigaHills;
	static Biome *smallerExtremeHills;
	static Biome *jungle;
	static Biome *jungleHills;

	static const int BIOME_COUNT = 23; // 4J Stu added

public:
	wstring m_name;
	int color;
	byte topMaterial;
	byte material;
	int leafColor;
	float depth;
	float scale;
	float temperature;
	float downfall;
	//int waterColor; // 4J Stu removed

	BiomeDecorator *decorator;

	const int id;

	class MobSpawnerData : public WeighedRandomItem
	{
	public:
		eINSTANCEOF mobClass;
		int minCount;
		int maxCount;

		MobSpawnerData(eINSTANCEOF mobClass, int probabilityWeight, int minCount, int maxCount) : WeighedRandomItem(probabilityWeight)
		{
			this->mobClass = mobClass;
			this->minCount = minCount;
			this->maxCount = maxCount;
		}
	};

protected:
	vector<MobSpawnerData *> enemies;
	vector<MobSpawnerData *> friendlies;
	vector<MobSpawnerData *> waterFriendlies;
	vector<MobSpawnerData *> friendlies_chicken;
	vector<MobSpawnerData *> friendlies_wolf;
	vector<MobSpawnerData *> friendlies_mushroomcow;
	vector<MobSpawnerData *> ambientFriendlies;

	Biome(int id);
	~Biome();

	BiomeDecorator *createDecorator();

private:
	Biome *setTemperatureAndDownfall(float temp, float downfall);
	Biome *setDepthAndScale(float depth, float scale);

	bool snowCovered;
	bool _hasRain;

	// 4J Added
	eMinecraftColour m_grassColor;
	eMinecraftColour m_foliageColor;
	eMinecraftColour m_waterColor;
	eMinecraftColour m_skyColor;

	Biome *setNoRain();

protected:
	/* removing these so that we can consistently return newly created trees via getTreeFeature, and let the calling function be resposible for deleting the returned tree
	TreeFeature *normalTree;
	BasicTree *fancyTree;
	BirchFeature *birchTree;
	SwampTreeFeature *swampTree;
	*/

public:
	virtual Feature *getTreeFeature(Random *random);
	virtual Feature *getGrassFeature(Random *random);

protected:
	Biome *setSnowCovered();
	Biome *setName(const wstring &name);
	Biome *setLeafColor(int leafColor);
	Biome *setColor(int color);

	// 4J Added
	Biome *setLeafFoliageWaterSkyColor(eMinecraftColour grassColor, eMinecraftColour foliageColor, eMinecraftColour waterColour, eMinecraftColour skyColour);

public:
	virtual int getSkyColor(float temp);

	vector<MobSpawnerData *> *getMobs(MobCategory *category);

	virtual bool hasSnow();
	virtual bool hasRain();
	virtual bool isHumid();

	virtual float getCreatureProbability();
	virtual int getDownfallInt();
	virtual int getTemperatureInt();
	virtual float getDownfall();			// 4J - brought forward from 1.2.3
	virtual float getTemperature();			// 4J - brought forward from 1.2.3

	virtual void decorate(Level *level, Random *random, int xo, int zo);

	virtual int getGrassColor();
	virtual int getFolageColor();
	virtual int getWaterColor(); // 4J Added
};