#pragma once
#include "Feature.h"


class HellSpringFeature : public Feature
{
private:
	int tile;
	bool insideRock;

public:
	HellSpringFeature(int tile, bool insideRock);

	virtual bool place(Level *level, Random *random, int x, int y, int z);
};