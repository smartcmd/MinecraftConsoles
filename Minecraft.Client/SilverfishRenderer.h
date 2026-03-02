#pragma once

#include "MobRenderer.h"

class Silverfish;

class SilverfishRenderer : public MobRenderer
{
private:
	//int modelVersion;

public:
	SilverfishRenderer();

protected:
	float getFlipDegrees(shared_ptr<Silverfish> spider);

public:
	void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);

protected:
	int prepareArmor(shared_ptr<Mob> _silverfish, int layer, float a);
};
