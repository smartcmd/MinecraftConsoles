#include "stdafx.h"
#include "EnderCrystalModel.h"



EnderCrystalModel::EnderCrystalModel(float g) 
{
	glass = new ModelPart(this, L"glass");
	glass->texOffs(0, 0)->addBox(-4, -4, -4, 8, 8, 8);

	cube = new ModelPart(this, L"cube");
	cube->texOffs(32, 0)->addBox(-4, -4, -4, 8, 8, 8);

	base = new ModelPart(this, L"base");
	base->texOffs(0, 16)->addBox(-6, 0, -6, 12, 4, 12);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	glass->compile(1.0f/16.0f);
	cube->compile(1.0f/16.0f);
	base->compile(1.0f/16.0f);
}


void EnderCrystalModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	glPushMatrix();
	glScalef(2, 2, 2);
	glTranslatef(0, -0.5f, 0);
	base->render(scale,usecompiled);
	glRotatef(r, 0, 1, 0);
	glTranslatef(0, 0.8f + bob, 0);
	glRotatef(60, 0.7071f, 0, 0.7071f);
	glass->render(scale,usecompiled);
	float ss = 14 / 16.0f;
	glScalef(ss, ss, ss);
	glRotatef(60, 0.7071f, 0, 0.7071f);
	glRotatef(r, 0, 1, 0);
	glass->render(scale,usecompiled);
	glScalef(ss, ss, ss);
	glRotatef(60, 0.7071f, 0, 0.7071f);
	glRotatef(r, 0, 1, 0);
	cube->render(scale,usecompiled);
	glPopMatrix();
}