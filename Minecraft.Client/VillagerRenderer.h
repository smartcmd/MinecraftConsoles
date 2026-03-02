#pragma once

#include "MobRenderer.h"

class VillagerModel;

class VillagerRenderer : public MobRenderer
{
protected:
	VillagerModel *villagerModel;

public:
	VillagerRenderer();

protected:
	virtual int prepareArmor(shared_ptr<Mob> villager, int layer, float a);

public:
	virtual void render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a);

protected:
	virtual void renderName(shared_ptr<Mob> mob, double x, double y, double z);
	virtual void additionalRendering(shared_ptr<Mob> mob, float a);
	virtual void scale(shared_ptr<Mob> player, float a);
};