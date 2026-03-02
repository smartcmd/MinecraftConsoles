#pragma once
#include "Model.h"
#include "ModelPart.h"

class WitherBossModel : public Model
{
private:
	ModelPartArray upperBodyParts;
	ModelPartArray heads;

public:
	WitherBossModel();

	int modelVersion();
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim=0);
	virtual void prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a);
};