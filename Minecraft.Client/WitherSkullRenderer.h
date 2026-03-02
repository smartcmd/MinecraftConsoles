#pragma once
#include "EntityRenderer.h"

class SkeletonHeadModel;

class WitherSkullRenderer : public EntityRenderer
{
private:
	static ResourceLocation WITHER_ARMOR_LOCATION;
	static ResourceLocation WITHER_LOCATION;

	SkeletonHeadModel *model;

public:
	WitherSkullRenderer();
	void render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a);
	ResourceLocation *getTextureLocation(shared_ptr<Entity> entity);

private:
	float rotlerp(float from, float to, float a);	
};