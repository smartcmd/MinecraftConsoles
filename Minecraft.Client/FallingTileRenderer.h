#pragma once
#include "EntityRenderer.h"


class FallingTileRenderer : public EntityRenderer
{
private:
	TileRenderer *tileRenderer;

public:
	FallingTileRenderer();

    virtual void render(shared_ptr<Entity> _tile, double x, double y, double z, float rot, float a);
};