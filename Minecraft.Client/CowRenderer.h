#pragma once
#include "MobRenderer.h"

class CowRenderer : public MobRenderer
{
public:
	CowRenderer(Model *model, float shadow);
	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);
};