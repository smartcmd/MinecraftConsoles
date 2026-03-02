#pragma once
#include "ZombieModel.h"

class SkeletonModel : public ZombieModel
{
private:
	void _init(float g);

public:
	SkeletonModel();
	SkeletonModel(float g);
	virtual void prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a);
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim=0);
};