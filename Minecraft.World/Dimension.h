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
    Level *level;
	LevelType *levelType;
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

    virtual float getTimeOfDay(__int64 time, float a) const;
	virtual int getMoonPhase(__int64 time, float a) const;
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
