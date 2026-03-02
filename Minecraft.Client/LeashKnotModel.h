#pragma once
#include "Model.h"

class LeashKnotModel : public Model
{
public:
    ModelPart *knot;

	LeashKnotModel();
    LeashKnotModel(int u, int v, int tw, int th);
	void _init(int u, int v, int tw, int th);

    virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
    virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim=0);
};