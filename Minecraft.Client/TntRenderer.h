#pragma once
#include "EntityRenderer.h"

class TntRenderer : public EntityRenderer
{
private:
	TileRenderer *tileRenderer;

public:
	TntRenderer();
	virtual void render(shared_ptr<Entity> _tnt, double x, double y, double z, float rot, float a);
};