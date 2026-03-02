#pragma once
#include "MobRenderer.h"

class SheepRenderer : public MobRenderer
{
public:
	SheepRenderer(Model *model, Model *armor, float shadow);

protected:
	virtual int prepareArmor(shared_ptr<Mob> _sheep, int layer, float a);

public:
	virtual void render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a);
};