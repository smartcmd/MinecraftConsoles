#include "stdafx.h"
#include "PigRenderer.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"

PigRenderer::PigRenderer(Model *model, Model *armor, float shadow) : MobRenderer(model,shadow)
{
	setArmor(armor);
}

int PigRenderer::prepareArmor(shared_ptr<Mob> _pig, int layer, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Pig> pig = dynamic_pointer_cast<Pig>(_pig);

	MemSect(31);
    bindTexture(TN_MOB_SADDLE);	// 4J was L"/mob/saddle.png"
	MemSect(0);
    return (layer == 0 && pig->hasSaddle()) ? 1 : -1;
}

void PigRenderer::render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a)
{
	MobRenderer::render(mob, x, y, z, rot, a);
} 