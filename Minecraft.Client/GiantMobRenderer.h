#pragma once
#include "MobRenderer.h"

class GiantMobRenderer : public MobRenderer
{
private:
	float _scale;

public:
	GiantMobRenderer(Model *model, float shadow, float scale);

protected:
	virtual void scale(shared_ptr<Mob> mob, float a);
};