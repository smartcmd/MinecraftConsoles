
#pragma once
#include "Model.h"

class VillagerModel : public Model 
{
public:
	ModelPart *head, *body, *arms, *leg0, *leg1;

	void _init(float g, float yOffset); // 4J added
    VillagerModel(float g, float yOffset);
    VillagerModel(float g);
    virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled) ;
    virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim=0);
};
