#include "stdafx.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "OzelotRenderer.h"

OzelotRenderer::OzelotRenderer(Model *model, float shadow) : MobRenderer(model, shadow)
{
}

void OzelotRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	MobRenderer::render(_mob, x, y, z, rot, a);
}

void OzelotRenderer::scale(shared_ptr<Mob> _mob, float a)
{
	// 4J - original version used generics and thus had an input parameter of type Blaze rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
	shared_ptr<Ozelot> mob = dynamic_pointer_cast<Ozelot>(_mob);
	MobRenderer::scale(mob, a);
	if (mob->isTame())
	{
		glScalef(.8f, .8f, .8f);
	}
}