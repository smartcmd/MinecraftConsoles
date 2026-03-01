#pragma once
#include "VillagerModel.h"

class WitchModel : public VillagerModel
{
public:
	bool holdingItem;

private:
	ModelPart *mole;
	ModelPart *hat;

public:
	WitchModel(float g);
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim=0);
	int getModelVersion();
};