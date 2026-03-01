#pragma once
#include "MobRenderer.h"

class OcelotRenderer : public MobRenderer
{
private:
	static ResourceLocation CAT_BLACK_LOCATION; 
    static ResourceLocation CAT_OCELOT_LOCATION;
    static ResourceLocation CAT_RED_LOCATION;
    static ResourceLocation CAT_SIAMESE_LOCATION;

public:
	OcelotRenderer(Model *model, float shadow);
	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);

protected:
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> entity);
	virtual void scale(shared_ptr<LivingEntity> _mob, float a);	
};