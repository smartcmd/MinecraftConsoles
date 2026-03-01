#pragma once
#include "MobRenderer.h"

class Silverfish;

class SilverfishRenderer : public MobRenderer
{
private:
	//int modelVersion;
	static ResourceLocation SILVERFISH_LOCATION;

public:
	SilverfishRenderer();

protected:
	float getFlipDegrees(shared_ptr<LivingEntity> spider);

public:
	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);

protected:
	virtual int prepareArmor(shared_ptr<LivingEntity> _silverfish, int layer, float a);
};
