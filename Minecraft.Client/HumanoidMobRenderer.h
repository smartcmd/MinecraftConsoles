#pragma once
#include "MobRenderer.h"
class HumanoidModel;
class Giant;

class HumanoidMobRenderer : public MobRenderer
{
protected:
	HumanoidModel *humanoidModel;
	float _scale;	
	HumanoidModel *armorParts1;
	HumanoidModel *armorParts2;

	void _init(HumanoidModel *humanoidModel, float scale);
public:
	HumanoidMobRenderer(HumanoidModel *humanoidModel, float shadow);
	HumanoidMobRenderer(HumanoidModel *humanoidModel, float shadow, float scale);
protected:
	virtual void createArmorParts();
	virtual void additionalRendering(shared_ptr<Mob> mob, float a);
	void scale(shared_ptr<Mob> mob, float a);
};