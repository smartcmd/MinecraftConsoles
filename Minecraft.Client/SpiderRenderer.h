#pragma once
#include "MobRenderer.h"

class SpiderRenderer : public MobRenderer
{
public:
	SpiderRenderer();
protected:
	virtual float getFlipDegrees(shared_ptr<Mob> spider);
    virtual int prepareArmor(shared_ptr<Mob> _spider, int layer, float a);
	void scale(shared_ptr<Mob> _mob, float a);
};