#pragma once

#include "Goal.h"

class Ocelot;

class OcelotSitOnTileGoal : public Goal
{
private:
	static const int GIVE_UP_TICKS;
	static const int SIT_TICKS;
	static const int SEARCH_RANGE;
	static const double SIT_CHANCE;

private:
	Ocelot *ocelot; // Owner of this goal
	double speedModifier;
	int _tick;
	int tryTicks;
	int maxTicks;
	int tileX;
	int tileY;
	int tileZ;

public:
	OcelotSitOnTileGoal(Ocelot *ocelot, double speedModifier);

	bool canUse();
	bool canContinueToUse();
	void start();
	void stop();
	void tick();

private:
	bool findNearestTile();
	bool isValidTarget(Level *level, int x, int y, int z);
};