#pragma once

#include "MobRenderer.h"

class MushroomCowRenderer : public MobRenderer
{
public:
	MushroomCowRenderer(Model *model, float shadow);

	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);

protected:
	virtual void additionalRendering(shared_ptr<Mob> _mob, float a);
};