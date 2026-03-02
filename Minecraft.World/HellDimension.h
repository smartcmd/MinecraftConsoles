#pragma once
#include "Dimension.h"

class HellDimension : public Dimension
{
public:
	virtual void init();
	virtual Vec3 *getFogColor(float td, float a) const;

protected:
	virtual void updateLightRamp();

public:
	virtual ChunkSource *createRandomLevelSource() const;
	virtual bool isNaturalDimension();
	virtual bool isValidSpawn(int x, int y) const;
	virtual float getTimeOfDay(__int64 time, float a) const;
	virtual bool mayRespawn() const;
	virtual bool isFoggyAt(int x, int z);

	// 4J Added
	virtual int getXZSize();
};
