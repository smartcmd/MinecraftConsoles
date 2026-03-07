#pragma once
class BiomeSource;
class ChunkSource;
class ChunkStorage;
class Level;
class LevelType;

#include "Material.h"
#include "Vec3.h"
#include "Pos.h"

class Dimension
{
public:
	static const float MOON_BRIGHTNESS_PER_PHASE[8];

	Level *level;
	LevelType *levelType;
	wstring levelTypeOptions;
	BiomeSource *biomeSource;
	bool ultraWarm ;
	bool hasCeiling;
	float *brightnessRamp;
	int id;

	virtual void init(Level *level);

protected:
	virtual void updateLightRamp();
	virtual void init();

public:
	Dimension();
	~Dimension();
	virtual ChunkSource *createRandomLevelSource() const;
	virtual ChunkSource *createFlatLevelSource() const;
	virtual ChunkStorage *createStorage(File dir);

	virtual bool isValidSpawn(int x, int z) const;

	virtual float getTimeOfDay(int64_t time, float a) const;
	virtual int getMoonPhase(int64_t time) const;
	virtual bool isNaturalDimension();
private:
	static const int fogColor = 0xc0d8ff;

	float sunriseCol[4];

public:
	virtual float *getSunriseColor(float td, float a);
	virtual Vec3 *getFogColor(float td, float a) const;
	virtual bool mayRespawn() const;
	static Dimension *getNew(int id);
	virtual float getCloudHeight();
	virtual bool hasGround();
	virtual Pos *getSpawnPos();

	int getSpawnYPosition();
	virtual bool hasBedrockFog();
	double getClearColorScale();
	virtual bool isFoggyAt(int x, int z);

	// 4J Added
	virtual int getXZSize();
};
