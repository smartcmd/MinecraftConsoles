#pragma once
#include "MobRenderer.h"

class BlazeRenderer : public MobRenderer
{
private:
	static ResourceLocation BLAZE_LOCATION;
	int modelVersion;	

public:
	BlazeRenderer();

	virtual void render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};