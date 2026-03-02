#pragma once
#include "MobRenderer.h"

class SlimeRenderer : public MobRenderer
{
private:
	Model *armor;
public:
	SlimeRenderer(Model *model, Model *armor, float shadow);
protected:
	virtual int prepareArmor(shared_ptr<Mob> _slime, int layer, float a);
    virtual void scale(shared_ptr<Mob> _slime, float a);
};