#pragma once
#include "Model.h"

class SnowManModel : public Model 
{
public:
	ModelPart *piece1, *piece2, *head;
	ModelPart *arm1, *arm2;

	SnowManModel() ;
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim=0);
	void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
};
