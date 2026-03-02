#pragma once

#include "MobRenderer.h"

class OzelotRenderer : public MobRenderer
{
public:
	OzelotRenderer(Model *model, float shadow);

	void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);

protected:
	void scale(shared_ptr<Mob> _mob, float a);
};