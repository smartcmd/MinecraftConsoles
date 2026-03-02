#pragma once
#include "Model.h"

class BlazeModel : public Model 
{

private:
	ModelPartArray upperBodyParts;
	ModelPart *head;

public:
	BlazeModel();
	int modelVersion();
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim=0);
};
