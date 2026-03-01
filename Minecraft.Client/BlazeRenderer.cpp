#include "stdafx.h"
#include "BlazeModel.h"
#include "..\Minecraft.World\net.minecraft.world.entity.monster.h"
#include "BlazeRenderer.h"

ResourceLocation BlazeRenderer::BLAZE_LOCATION = ResourceLocation(TN_MOB_BLAZE);

BlazeRenderer::BlazeRenderer() : MobRenderer(new BlazeModel(), 0.5f)
{
	modelVersion = ((BlazeModel *) model)->modelVersion();
}

void BlazeRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	// 4J - original version used generics and thus had an input parameter of type Blaze rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
	shared_ptr<Blaze> mob = dynamic_pointer_cast<Blaze>(_mob);

	int modelVersion = ((BlazeModel *) model)->modelVersion();
	if (modelVersion != this->modelVersion)
	{
		this->modelVersion = modelVersion;
		model = new BlazeModel();
	}
	MobRenderer::render(mob, x, y, z, rot, a);
}

ResourceLocation *BlazeRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &BLAZE_LOCATION;
}