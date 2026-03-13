#include "stdafx.h"
#include "ZombieModel.h"
#include "../Minecraft.World/Mth.h"
#include "ModelPart.h"

ZombieModel::ZombieModel() : HumanoidModel(0, 0, 64, 32)
{
}

ZombieModel::ZombieModel(float g, float yOffset, int texWidth, int texHeight) : HumanoidModel(g, yOffset, texWidth, texHeight)
{
}

ZombieModel::ZombieModel(float g, bool isArmor) : HumanoidModel(g, 0, 64, isArmor ? 32 : 64)
{
}

void ZombieModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
    HumanoidModel::setupAnim(time, r, bob, yRot, xRot, scale, entity, uiBitmaskOverrideAnim);
        
    float attack2 = Mth::sin(attackTime*PI);
    float attack = Mth::sin((1-(1-attackTime)*(1-attackTime))*PI);
    arm0->zRot = 0;
    arm1->zRot = 0;
    arm0->yRot = -(0.1f-attack2*0.6f);
    arm1->yRot = +(0.1f-attack2*0.6f);
    arm0->xRot = -(float)PI/2.0f;
    arm1->xRot = -(float)PI/2.0f;
    arm0->xRot -= attack2*1.2f-attack*0.4f;
    arm1->xRot -= attack2*1.2f-attack*0.4f;  
    arm0->zRot += ((Mth::cos(bob * 0.09f)) * 0.05f+0.05f);
    arm1->zRot -= ((Mth::cos(bob * 0.09f)) * 0.05f+0.05f);
    arm0->xRot += ((Mth::sin(bob * 0.067f)) * 0.05f);
    arm1->xRot -= ((Mth::sin(bob * 0.067f)) * 0.05f);
}