#include "stdafx.h"
#include "SkeletonRenderer.h"
#include "SkeletonModel.h"
#include "../Minecraft.World/Skeleton.h"

ResourceLocation SkeletonRenderer::SKELETON_LOCATION = ResourceLocation(TN_MOB_SKELETON);
ResourceLocation SkeletonRenderer::WITHER_SKELETON_LOCATION = ResourceLocation(TN_MOB_WITHER_SKELETON);

SkeletonRenderer::SkeletonRenderer() : HumanoidMobRenderer(new SkeletonModel(), .5f)
{
}

void SkeletonRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
    if (dynamic_pointer_cast<Skeleton>(mob)->getSkeletonType() == Skeleton::TYPE_WITHER)
	{
        glScalef(1.2f, 1.2f, 1.2f);
    }
}

void SkeletonRenderer::translateWeaponItem()
{
    glTranslatef(1.5f / 16.0f, 3 / 16.0f, 0);
}

ResourceLocation *SkeletonRenderer::getTextureLocation(shared_ptr<Entity> entity)
{
	shared_ptr<Skeleton> skeleton = dynamic_pointer_cast<Skeleton>(entity);

    if (skeleton->getSkeletonType() == Skeleton::TYPE_WITHER)
	{
        return &WITHER_SKELETON_LOCATION;
    }
    return &SKELETON_LOCATION;
}