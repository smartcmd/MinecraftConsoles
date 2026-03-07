#pragma once
#include "Dimension.h"

class TheEndDimension : public Dimension
{
public:
	virtual void init();
	virtual ChunkSource *createRandomLevelSource() const;
	virtual float getTimeOfDay(int64_t time, float a) const;
	virtual float *getSunriseColor(float td, float a);
	virtual Vec3 *getFogColor(float td, float a) const;
	virtual bool hasGround();
	virtual bool mayRespawn() const;
	virtual bool isNaturalDimension();
	virtual float getCloudHeight();
	virtual bool isValidSpawn(int x, int z) const;
	virtual Pos *getSpawnPos();
	virtual int getSpawnYPosition();
	virtual bool isFoggyAt(int x, int z);
};
