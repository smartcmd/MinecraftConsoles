#include "stdafx.h"
#include "CowRenderer.h"

CowRenderer::CowRenderer(Model *model, float shadow) : MobRenderer(model, shadow)
{
}

void CowRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	MobRenderer::render(_mob, x, y, z, rot, a);
}