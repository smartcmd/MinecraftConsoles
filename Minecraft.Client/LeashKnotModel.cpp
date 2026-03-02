#include "stdafx.h";
#include "LeashKnotModel.h"
#include "ModelPart.h"

LeashKnotModel::LeashKnotModel() 
{
	_init(0, 0, 32, 32);
}

LeashKnotModel::LeashKnotModel(int u, int v, int tw, int th)
{
    _init(u, v, tw, th);
}

void LeashKnotModel::_init(int u, int v, int tw, int th)
{
	texWidth = tw;
    texHeight = th;
    knot = new ModelPart(this, u, v);
    knot->addBox(-3, -6, -3, 6, 8, 6, 0);
    knot->setPos(0, 0, 0);
}

void LeashKnotModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
    setupAnim(time, r, bob, yRot, xRot, scale, entity);

    knot->render(scale, usecompiled);
}

void LeashKnotModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
    Model::setupAnim(time, r, bob, yRot, xRot, scale, entity);

    knot->yRot = yRot / (180 / PI);
    knot->xRot = xRot / (180 / PI);
}