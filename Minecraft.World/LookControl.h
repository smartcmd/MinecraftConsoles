#pragma once

#include "Control.h"

class Mob;

class LookControl : public Control
{
private:
	Mob *mob;
	float yMax, xMax;
	bool hasWanted;

	double wantedX, wantedY, wantedZ;

public:
	LookControl(Mob *mob);

	void setLookAt(shared_ptr<Entity> target, float yMax, float xMax);
	void setLookAt(double x, double y, double z, float yMax, float xMax);
	virtual void tick();

private:
	float rotlerp(float a, float b, float max);

public:
	bool isHasWanted();
	float getYMax();
	float getXMax();
	double getWantedX();
	double getWantedY();
	double getWantedZ();
};