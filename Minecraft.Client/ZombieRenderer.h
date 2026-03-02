#pragma once
#include "HumanoidMobRenderer.h"

class VillagerZombieModel;
class Zombie;

class ZombieRenderer : public HumanoidMobRenderer
{
private:
	static ResourceLocation ZOMBIE_PIGMAN_LOCATION;
    static ResourceLocation ZOMBIE_LOCATION;
    static ResourceLocation ZOMBIE_VILLAGER_LOCATION;

	HumanoidModel *defaultModel;
	VillagerZombieModel *villagerModel;

protected:
	HumanoidModel *defaultArmorParts1;
	HumanoidModel *defaultArmorParts2;
	HumanoidModel *villagerArmorParts1;
	HumanoidModel *villagerArmorParts2;

private:
	int modelVersion;

public:
	ZombieRenderer();

protected:
	virtual void createArmorParts();
	virtual int prepareArmor(shared_ptr<LivingEntity> _mob, int layer, float a);

public:
	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> entity);

protected:
	virtual void additionalRendering(shared_ptr<LivingEntity> _mob, float a);

private:
	virtual void swapArmor(shared_ptr<Zombie> mob);

protected:
	virtual void setupRotations(shared_ptr<LivingEntity> _mob, float bob, float bodyRot, float a);
};