#pragma once
#include "HumanoidMobRenderer.h"

class SkeletonRenderer : public HumanoidMobRenderer
{
private:
    static ResourceLocation SKELETON_LOCATION;
    static ResourceLocation WITHER_SKELETON_LOCATION;

public:
	SkeletonRenderer();

protected:
    virtual void scale(shared_ptr<LivingEntity> mob, float a);
    void translateWeaponItem();
    virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> entity);
};