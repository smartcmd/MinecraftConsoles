#pragma once
#include "MobRenderer.h"

class EnderMan;
class EndermanModel;

class EndermanRenderer : public MobRenderer
{
private:
	EndermanModel *model;
	Random random;
	static ResourceLocation ENDERMAN_EYES_LOCATION;
	static ResourceLocation ENDERMAN_LOCATION;

public:
	EndermanRenderer();

	void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);
	ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
	void additionalRendering(shared_ptr<LivingEntity> _mob, float a);

protected:
	int prepareArmor(shared_ptr<LivingEntity> _mob, int layer, float a);
};