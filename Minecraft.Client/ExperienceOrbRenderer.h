#pragma once

#include "EntityRenderer.h"

class ExperienceOrbRenderer : public EntityRenderer
{
private:
	TileRenderer *tileRenderer;

public:
	bool setColor;

	ExperienceOrbRenderer();

	void render(shared_ptr<Entity> _orb, double x, double y, double z, float rot, float a);
	void blit(int x, int y, int sx, int sy, int w, int h);
};