#pragma once
#include "MobRenderer.h"

#ifdef _XBOX
class EnderDragon;
#endif
class DragonModel;

class EnderDragonRenderer : public MobRenderer
{
private:
	static ResourceLocation DRAGON_EXPLODING_LOCATION;
	static ResourceLocation CRYSTAL_BEAM_LOCATION;
	static ResourceLocation DRAGON_EYES_LOCATION;
	static ResourceLocation DRAGON_LOCATION;

protected:
	DragonModel *dragonModel;

public:
	EnderDragonRenderer();

protected:
	virtual void setupRotations(shared_ptr<LivingEntity> _mob, float bob, float bodyRot, float a);
	virtual void renderModel(shared_ptr<LivingEntity> _mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale);

public:
	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);

protected:
	virtual void additionalRendering(shared_ptr<LivingEntity> _mob, float a);
	virtual int prepareArmor(shared_ptr<LivingEntity> _mob, int layer, float a);
};