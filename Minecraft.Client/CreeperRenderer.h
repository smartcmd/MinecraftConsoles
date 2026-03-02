#pragma once
#include "MobRenderer.h"

class CreeperRenderer: public MobRenderer
{
private:
	static ResourceLocation POWER_LOCATION;
	static ResourceLocation CREEPER_LOCATION;
	Model *armorModel;	

public:
	CreeperRenderer();

protected:
	virtual void scale(shared_ptr<LivingEntity> _mob, float a);
    virtual int getOverlayColor(shared_ptr<LivingEntity> mob, float br, float a);
    virtual int prepareArmor(shared_ptr<LivingEntity> mob, int layer, float a);
    virtual int prepareArmorOverlay(shared_ptr<LivingEntity> _mob, int layer, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};