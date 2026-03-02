#pragma once

#include "BasePressurePlateTile.h"

class WeightedPressurePlateTile : public BasePressurePlateTile
{
private:
	int maxWeight;

public:
	WeightedPressurePlateTile(int id, const wstring &tex, Material *material, int maxWeight);

protected:
	virtual int getSignalStrength(Level *level, int x, int y, int z);
	virtual int getSignalForData(int data);
	virtual int getDataForSignal(int signal);
	virtual int getTickDelay(Level *level);
};