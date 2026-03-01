#include "stdafx.h"
#include "SkiModel.h"

SkiModel::SkiModel()
{
	_init(false);
}

SkiModel::SkiModel(bool leftSki)
{
	_init(leftSki);
}

void SkiModel::_init(bool leftSki)
{
	this->leftSki = leftSki;
    texWidth = 32;
    texHeight = 64;
    int xOffTex = 0;
    if (!leftSki) {
        xOffTex = 14;
    }

	cubes = ModelPartArray(2);
    cubes[0] = new ModelPart(this, xOffTex, 0);
    cubes[1] = new ModelPart(this, xOffTex, 5);

    cubes[0]->addBox(0.f, 0.f, 0.f, 3, 1, 4, 0);
    cubes[0]->setPos(0, 0, 0);
			
    cubes[1]->addBox(0.f, 0.f, 0.f, 3, 52, 1, 0);
    cubes[1]->setPos(0, 0, 0);
}

void SkiModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
    for (int i = 0; i < cubes.length; i++)
	{
        cubes[i]->render(scale, usecompiled);
    }
}

void SkiModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity)
{
    cubes[0]->y = 24.2f;
    cubes[0]->xRot = PI * .5f;
			
    cubes[1]->y = 24.2f;
    cubes[1]->xRot = PI * .5f;

    if (leftSki)
	{
        cubes[0]->z = -26 - 12 * (cos(time * 0.6662f) * 0.7f) * r;
        cubes[1]->z = -26 - 12 * (cos(time * 0.6662f) * 0.7f) * r;
        cubes[0]->x = .5f;
        cubes[1]->x = .5f;
    }			
	else		
	{			
        cubes[0]->z = -26 + 12 * (cos(time * 0.6662f) * 0.7f) * r;
        cubes[1]->z = -26 + 12 * (cos(time * 0.6662f) * 0.7f) * r;
        cubes[0]->x = -3.5f;
        cubes[1]->x = -3.5f;
    }
}