#pragma once
#include "MobRenderer.h"
#include "ResourceLocation.h"

class EntityHorse;
class PathfinderMob;

class HorseRenderer : public MobRenderer
{
private:
	static std::map<wstring, ResourceLocation *> LAYERED_LOCATION_CACHE;

	static ResourceLocation HORSE_LOCATION;
	static ResourceLocation HORSE_MULE_LOCATION;
	static ResourceLocation HORSE_DONKEY_LOCATION;
	static ResourceLocation HORSE_ZOMBIE_LOCATION;
	static ResourceLocation HORSE_SKELETON_LOCATION;

public:
	HorseRenderer(Model *model, float f);

protected:
	void adjustHeight(shared_ptr<PathfinderMob> mob, float FHeight);
	virtual void scale(shared_ptr<LivingEntity> entityliving, float f);
	virtual void renderModel(shared_ptr<LivingEntity> mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale);
	virtual void bindTexture(ResourceLocation *location);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> entity);

private:
	ResourceLocation *getOrCreateLayeredTextureLocation(shared_ptr<EntityHorse> horse);
};