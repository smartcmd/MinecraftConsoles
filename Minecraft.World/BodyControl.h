#pragma once

#include "Control.h"

class BodyControl : public Control
{
private:
	LivingEntity *mob;
	static const float maxClampAngle;
	int timeStill;
	float lastHeadY;

public:
	BodyControl(LivingEntity *mob);

	void clientTick();

private:
	float clamp(float clampTo, float clampFrom, float clampAngle);
};