#pragma once
#include "MobRenderer.h"

class CreeperRenderer: public MobRenderer
{
private:
	Model *armorModel;

public:
	CreeperRenderer();
protected:
	virtual void scale(shared_ptr<Mob> _mob, float a);
    virtual int getOverlayColor(shared_ptr<Mob> mob, float br, float a);
    virtual int prepareArmor(shared_ptr<Mob> mob, int layer, float a);
    virtual int prepareArmorOverlay(shared_ptr<Mob> _mob, int layer, float a);
};