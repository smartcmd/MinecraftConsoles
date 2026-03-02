#pragma once
#include "MobRenderer.h"

class WitchModel;

class WitchRenderer : public MobRenderer
{
private:
	static ResourceLocation WITCH_LOCATION;
	WitchModel *witchModel;

public:	
	WitchRenderer();
	virtual void render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a);

protected:
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> entity);
	virtual void additionalRendering(shared_ptr<LivingEntity> mob, float a);
	virtual void translateWeaponItem();
	virtual void scale(shared_ptr<LivingEntity> mob, float a);
};