#pragma once

#include "MobRenderer.h"

class LavaSlimeRenderer : public MobRenderer
{
private:
	int modelVersion;

public:
	LavaSlimeRenderer();

	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);

protected:
	virtual void scale(shared_ptr<Mob> _slime, float a);
};