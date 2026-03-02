#include "stdafx.h"
#include "..\Minecraft.World\Mth.h"
#include "ChickenRenderer.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"

ChickenRenderer::ChickenRenderer(Model *model, float shadow) : MobRenderer(model,shadow)
{
}

void ChickenRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	MobRenderer::render(_mob, x, y, z, rot, a);
}

float ChickenRenderer::getBob(shared_ptr<Mob> _mob, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Chicken> mob = dynamic_pointer_cast<Chicken>(_mob);

    float flap = mob->oFlap+(mob->flap-mob->oFlap)*a;
    float flapSpeed = mob->oFlapSpeed+(mob->flapSpeed-mob->oFlapSpeed)*a;
        
    return (Mth::sin(flap)+1)*flapSpeed;
}