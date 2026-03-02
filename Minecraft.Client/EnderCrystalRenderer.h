#pragma once

#include "EntityRenderer.h"

class Model;

class EnderCrystalRenderer : public EntityRenderer
{
private:
	int currentModel;
	Model *model;

public:
	EnderCrystalRenderer();

	virtual void render(shared_ptr<Entity> _crystal, double x, double y, double z, float rot, float a);
};