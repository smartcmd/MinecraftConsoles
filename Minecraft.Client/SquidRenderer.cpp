#include "stdafx.h"
#include "SquidRenderer.h"
#include "../Minecraft.World/net.minecraft.world.entity.animal.h"

ResourceLocation SquidRenderer::SQUID_LOCATION = ResourceLocation(TN_MOB_SQUID);

SquidRenderer::SquidRenderer(Model *model, float shadow) : MobRenderer(model, shadow)
{
}

void SquidRenderer::render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a)
{
	MobRenderer::render(mob, x, y, z, rot, a);
}

void SquidRenderer::setupRotations(shared_ptr<LivingEntity> _mob, float bob, float bodyRot, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Squid> mob = dynamic_pointer_cast<Squid>(_mob);

    float bodyXRot = (mob->xBodyRotO + (mob->xBodyRot - mob->xBodyRotO) * a);
    float bodyZRot = (mob->zBodyRotO + (mob->zBodyRot - mob->zBodyRotO) * a);

    glTranslatef(0, 0.5f, 0);
    glRotatef(180 - bodyRot, 0, 1, 0);
    glRotatef(bodyXRot, 1, 0, 0);
    glRotatef(bodyZRot, 0, 1, 0);
    glTranslatef(0, -1.2f, 0);
}

float SquidRenderer::getBob(shared_ptr<LivingEntity> _mob, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Squid> mob = dynamic_pointer_cast<Squid>(_mob);

    return mob->oldTentacleAngle + (mob->tentacleAngle - mob->oldTentacleAngle) * a;
}

ResourceLocation *SquidRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &SQUID_LOCATION;
}