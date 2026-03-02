#pragma once

#include "MobRenderer.h"

class BlazeRenderer : public MobRenderer
{
private:
	int modelVersion;

public:
	BlazeRenderer();

	virtual void render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a);
};