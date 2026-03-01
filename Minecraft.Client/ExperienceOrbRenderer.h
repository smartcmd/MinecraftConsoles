#pragma once
#include "EntityRenderer.h"

class ExperienceOrbRenderer : public EntityRenderer
{
private:
	static ResourceLocation XP_ORB_LOCATION;

public:
	ExperienceOrbRenderer();

	virtual void render(shared_ptr<Entity> _orb, double x, double y, double z, float rot, float a);
	void blit(int x, int y, int sx, int sy, int w, int h);

	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};