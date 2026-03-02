#pragma once
#include "MobRenderer.h"

class VillagerModel;

class VillagerRenderer : public MobRenderer
{
private:
	static ResourceLocation VILLAGER_LOCATION;
    static ResourceLocation VILLAGER_FARMER_LOCATION;
    static ResourceLocation VILLAGER_LIBRARIAN_LOCATION;
    static ResourceLocation VILLAGER_PRIEST_LOCATION;
    static ResourceLocation VILLAGER_SMITH_LOCATION;
    static ResourceLocation VILLAGER_BUTCHER_LOCATION;

protected:
	VillagerModel *villagerModel;

public:
	VillagerRenderer();
	virtual void render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> _mob);

protected:	
	virtual int prepareArmor(shared_ptr<LivingEntity> villager, int layer, float a);
	virtual void additionalRendering(shared_ptr<LivingEntity> mob, float a);
	virtual void scale(shared_ptr<LivingEntity> player, float a);
};