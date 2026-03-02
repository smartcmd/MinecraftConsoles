#pragma once

#include "Model.h"

class SkeletonHeadModel : public Model
{
public:
	ModelPart *head;

private:
	void _init(int u, int v, int tw, int th);

public:
	SkeletonHeadModel();
	SkeletonHeadModel(int u, int v, int tw, int th);

	void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim=0);
};