#pragma once
#include "MobRenderer.h"

class WitherBoss;
class LivingEntity;

class WitherBossRenderer : public MobRenderer
{
private:
	static ResourceLocation WITHER_INVULERABLE_LOCATION;
    static ResourceLocation WITHER_ARMOR_LOCATION;
    static ResourceLocation WITHER_LOCATION;
    int modelVersion;

public:
	WitherBossRenderer();
    virtual void render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a);
    virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> entity);

protected:
    virtual void scale(shared_ptr<LivingEntity> mob, float a);
    virtual int prepareArmor(shared_ptr<LivingEntity> entity, int layer, float a);
    virtual int prepareArmorOverlay(shared_ptr<LivingEntity> entity, int layer, float a);
};