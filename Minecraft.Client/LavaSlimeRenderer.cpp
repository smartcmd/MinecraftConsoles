#include "stdafx.h"
#include "..\Minecraft.World\net.minecraft.world.entity.monster.h"
#include "LavaSlimeModel.h"
#include "LavaSlimeRenderer.h"

LavaSlimeRenderer::LavaSlimeRenderer() : MobRenderer(new LavaSlimeModel(), .25f)
{
	this->modelVersion = ((LavaSlimeModel *) model)->getModelVersion();
}

void LavaSlimeRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{		
	// 4J - original version used generics and thus had an input parameter of type LavaSlime rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
	shared_ptr<LavaSlime> mob = dynamic_pointer_cast<LavaSlime>(_mob);
	int modelVersion = ((LavaSlimeModel *) model)->getModelVersion();
	if (modelVersion != this->modelVersion)
	{
		this->modelVersion = modelVersion;
		model = new LavaSlimeModel();
		app.DebugPrintf("new lava slime model\n");
	}
	MobRenderer::render(mob, x, y, z, rot, a);
}

void LavaSlimeRenderer::scale(shared_ptr<Mob> _slime, float a)
{
	// 4J - original version used generics and thus had an input parameter of type LavaSlime rather than shared_ptr<Mob>  we have here - 
	// do some casting around instead
	shared_ptr<LavaSlime> slime = dynamic_pointer_cast<LavaSlime>(_slime);
	int size = slime->getSize();
	float ss = (slime->oSquish + (slime->squish - slime->oSquish) * a) / (size * 0.5f + 1);
	float w = 1 / (ss + 1);
	float s = size;
	glScalef(w * s, 1 / w * s, w * s);
}