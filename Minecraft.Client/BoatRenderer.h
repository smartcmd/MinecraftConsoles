#pragma once
#include "EntityRenderer.h"

class BoatRenderer : public EntityRenderer
{
private:
	static ResourceLocation BOAT_LOCATION;

protected:
	Model *model;
public:
	BoatRenderer();

    virtual void render(shared_ptr<Entity> boat, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};