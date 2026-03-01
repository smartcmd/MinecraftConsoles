#include "stdafx.h"
#include "ModelPart.h"
#include "SkeletonHeadModel.h"

void SkeletonHeadModel::_init(int u, int v, int tw, int th)
{
	texWidth = tw;
	texHeight = th;
	head = new ModelPart(this, u, v);

	// 4J Stu - Set "g" param to 0.1 to fix z-fighting issues (hair has this set to 0.5, so need to be less that that)
	// Fix for #101501 - TU12: Content: Art: Z-Fighting occurs on the bottom side of a character's head when a Mob Head is equipped.
	head->addBox(-4, -8, -4, 8, 8, 8, 0.1); // Head
	head->setPos(0, 0, 0);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	head->compile(1.0f/16.0f);
}

SkeletonHeadModel::SkeletonHeadModel()
{
	_init(0, 35, 64, 64);
}

SkeletonHeadModel::SkeletonHeadModel(int u, int v, int tw, int th)
{
	_init(u,v,tw,th);
}

void SkeletonHeadModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	setupAnim(time, r, bob, yRot, xRot, scale, entity);

	head->render(scale,usecompiled);
}

void SkeletonHeadModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	Model::setupAnim(time, r, bob, yRot, xRot, scale, entity, uiBitmaskOverrideAnim);

	head->yRot = yRot / (180 / PI);
	head->xRot = xRot / (180 / PI);
}