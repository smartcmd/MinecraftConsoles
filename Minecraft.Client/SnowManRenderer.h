#pragma once

#include "MobRenderer.h"

class SnowManModel;

class SnowManRenderer : public MobRenderer
{
private:
	SnowManModel *model;

public:
	SnowManRenderer();

protected:
	virtual void additionalRendering(shared_ptr<Mob> _mob, float a);
};