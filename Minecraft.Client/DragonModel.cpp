#include "stdafx.h"
#include "DragonModel.h"
#include "..\Minecraft.World\Mth.h"
#include "..\Minecraft.World\Enderdragon.h"

DragonModel::DragonModel(float g) : Model()
{
	// 4J-PB
	texWidth = 256;
	texHeight = 256;

	setMapTex(L"body.body", 0, 0);
	setMapTex(L"wing.skin", -56, 88);
	setMapTex(L"wingtip.skin", -56, 144);
	setMapTex(L"rearleg.main", 0, 0);
	setMapTex(L"rearfoot.main", 112, 0);
	setMapTex(L"rearlegtip.main", 196, 0);
	setMapTex(L"head.upperhead", 112, 30);
	setMapTex(L"wing.bone", 112, 88);
	setMapTex(L"head.upperlip", 176, 44);
	setMapTex(L"jaw.jaw", 176, 65);
	setMapTex(L"frontleg.main", 112, 104);
	setMapTex(L"wingtip.bone", 112, 136);
	setMapTex(L"frontfoot.main", 144, 104);
	setMapTex(L"neck.box", 192, 104);
	setMapTex(L"frontlegtip.main", 226, 138);
	setMapTex(L"body.scale", 220, 53);
	setMapTex(L"head.scale", 0, 0);
	setMapTex(L"neck.scale", 48, 0);
	setMapTex(L"head.nostril", 112, 0);

	float zo = -16;
	head = new ModelPart(this, L"head");
	head->addBox(L"upperlip", -6, -1, -8 + zo, 12, 5, 16);
	head->addBox(L"upperhead", -8, -8, 6 + zo, 16, 16, 16);
	head->bMirror = true;
	head->addBox(L"scale", -1 - 4, -12, 12 + zo, 2, 4, 6);
	head->addBox(L"nostril", -1 - 4, -3, -6 + zo, 2, 2, 4);
	head->bMirror = false;
	head->addBox(L"scale", -1 + 4, -12, 12 + zo, 2, 4, 6);
	head->addBox(L"nostril", -1 + 4, -3, -6 + zo, 2, 2, 4);

	jaw = new ModelPart(this, L"jaw");
	jaw->setPos(0, 4, 8 + zo);
	jaw->addBox(L"jaw", -6, 0, -16, 12, 4, 16);
	head->addChild(jaw);

	neck = new ModelPart(this, L"neck");
	neck->addBox(L"box", -5, -5, -5, 10, 10, 10);
	neck->addBox(L"scale", -1, -9, -5 + 2, 2, 4, 6);

	body = new ModelPart(this, L"body");
	body->setPos(0, 4, 8);
	body->addBox(L"body", -12, 0, -16, 24, 24, 64);
	body->addBox(L"scale", -1, -6, -10 + 20 * 0, 2, 6, 12);
	body->addBox(L"scale", -1, -6, -10 + 20 * 1, 2, 6, 12);
	body->addBox(L"scale", -1, -6, -10 + 20 * 2, 2, 6, 12);

	wing = new ModelPart(this, L"wing");
	wing->setPos(-12, 5, 2);
	wing->addBox(L"bone", -56, -4, -4, 56, 8, 8);
	wing->addBox(L"skin", -56, 0, +2, 56, 0, 56);
	wingTip = new ModelPart(this, L"wingtip");
	wingTip->setPos(-56, 0, 0);
	wingTip->addBox(L"bone", -56, -2, -2, 56, 4, 4);
	wingTip->addBox(L"skin", -56, 0, +2, 56, 0, 56);
	wing->addChild(wingTip);

	frontLeg = new ModelPart(this, L"frontleg");
	frontLeg->setPos(-12, 20, 2);
	frontLeg->addBox(L"main", -4, -4, -4, 8, 24, 8);
	frontLegTip = new ModelPart(this, L"frontlegtip");
	frontLegTip->setPos(0, 20, -1);
	frontLegTip->addBox(L"main", -3, -1, -3, 6, 24, 6);
	frontLeg->addChild(frontLegTip);
	frontFoot = new ModelPart(this, L"frontfoot");
	frontFoot->setPos(0, 23, 0);
	frontFoot->addBox(L"main", -4, 0, -12, 8, 4, 16);
	frontLegTip->addChild(frontFoot);

	rearLeg = new ModelPart(this, L"rearleg");
	rearLeg->setPos(-12 - 4, 16, 2 + 40);
	rearLeg->addBox(L"main", -8, -4, -8, 16, 32, 16);
	rearLegTip = new ModelPart(this, L"rearlegtip");
	rearLegTip->setPos(0, 32, -4);
	rearLegTip->addBox(L"main", -6, -2, 0, 12, 32, 12);
	rearLeg->addChild(rearLegTip);
	rearFoot = new ModelPart(this, L"rearfoot");
	rearFoot->setPos(0, 31, 4);
	rearFoot->addBox(L"main", -9, 0, -20, 18, 6, 24);
	rearLegTip->addChild(rearFoot);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	// 4J Stu - Not just performance, but alpha+depth tests don't work right unless we compile here
	head->compile(1.0f/16.0f);
	jaw->compile(1.0f/16.0f);
	neck->compile(1.0f/16.0f);
	body->compile(1.0f/16.0f);
	wing->compile(1.0f/16.0f);
	wingTip->compile(1.0f/16.0f);
	frontLeg->compile(1.0f/16.0f);
	frontLegTip->compile(1.0f/16.0f);
	frontFoot->compile(1.0f/16.0f);
	rearLeg->compile(1.0f/16.0f);
	rearLegTip->compile(1.0f/16.0f);
	rearFoot->compile(1.0f/16.0f);
}

void DragonModel::prepareMobModel(shared_ptr<LivingEntity> mob, float time, float r, float a) 
{
	this->a = a;
}

void DragonModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	glPushMatrix();
	shared_ptr<EnderDragon> dragon = dynamic_pointer_cast<EnderDragon>(entity);

	float ttt = dragon->oFlapTime + (dragon->flapTime - dragon->oFlapTime) * a;
	jaw->xRot = (float) (Mth::sin(ttt * PI * 2) + 1) * 0.2f;

	float yo = (float) (Mth::sin(ttt * PI * 2 - 1) + 1);
	yo = (yo * yo * 1 + yo * 2) * 0.05f;

	glTranslatef(0, yo - 2.0f, -3);
	glRotatef(yo * 2, 1, 0, 0);

	float yy = -30.0f;
	float zz = 22.0f;
	float xx = 0.0f;

	float rotScale = 1.5f;


	double startComponents[3];
	doubleArray start = doubleArray(startComponents,3);
	dragon->getLatencyPos(start, 6, a);

	double latencyPosAComponents[3], latencyPosBComponents[3];
	doubleArray latencyPosA = doubleArray( latencyPosAComponents, 3 );
	doubleArray latencyPosB = doubleArray( latencyPosBComponents, 3 );
	dragon->getLatencyPos(latencyPosA, 5, a);
	dragon->getLatencyPos(latencyPosB, 10, a);
	float rot2 = rotWrap(latencyPosA[0] - latencyPosB[0]);
	float rot = rotWrap(latencyPosA[0] + rot2 / 2);

	yy += 2.0f;

	float rr = 0;
	float roff = ttt * PI * 2.0f;
	yy = 20.0f;
	zz = -12.0f;
	double pComponents[3];
	doubleArray p = doubleArray(pComponents,3);

	for (int i = 0; i < 5; i++) 
	{
		dragon->getLatencyPos(p, 5 - i, a);

		rr = (float) Mth::cos(i * 0.45f + roff) * 0.15f;
		neck->yRot = rotWrap(dragon->getHeadPartYRotDiff(i, start, p)) * PI / 180.0f * rotScale; // 4J replaced "p[0] - start[0] with call to getHeadPartYRotDiff
		neck->xRot = rr + (float) (dragon->getHeadPartYOffset(i, start, p)) * PI / 180.0f * rotScale * 5.0f; // 4J replaced "p[1] - start[1]" with call to getHeadPartYOffset
		neck->zRot = -rotWrap(p[0] - rot) * PI / 180.0f * rotScale;

		neck->y = yy;
		neck->z = zz;
		neck->x = xx;
		yy += Mth::sin(neck->xRot) * 10.0f;
		zz -= Mth::cos(neck->yRot) * Mth::cos(neck->xRot) * 10.0f;
		xx -= Mth::sin(neck->yRot) * Mth::cos(neck->xRot) * 10.0f;
		neck->render(scale,usecompiled);
	}

	head->y = yy;
	head->z = zz;
	head->x = xx;
	dragon->getLatencyPos(p, 0, a);
	head->yRot = rotWrap(dragon->getHeadPartYRotDiff(6, start, p)) * PI / 180.0f * 1; // 4J replaced "p[0] - start[0] with call to getHeadPartYRotDiff
	head->xRot = (float) (dragon->getHeadPartYOffset(6, start, p)) * PI / 180.0f * rotScale * 5.0f; // 4J Added
	head->zRot = -rotWrap(p[0] - rot) * PI / 180 * 1;
	head->render(scale,usecompiled);
	glPushMatrix();
	glTranslatef(0, 1, 0);
	glRotatef(-(float) (rot2) * rotScale * 1, 0, 0, 1);
	glTranslatef(0, -1, 0);
	body->zRot = 0;
	body->render(scale,usecompiled);
	
	glEnable(GL_CULL_FACE);
	for (int i = 0; i < 2; i++) 
	{
		float flapTime = ttt * PI * 2;
		wing->xRot = 0.125f - (float) (Mth::cos(flapTime)) * 0.2f;
		wing->yRot = 0.25f;
		wing->zRot = (float) (Mth::sin(flapTime) + 0.125f) * 0.8f;
		wingTip->zRot = -(float) (Mth::sin(flapTime + 2.0f) + 0.5f) * 0.75f;

		rearLeg->xRot = 1.0f + yo * 0.1f;
		rearLegTip->xRot = 0.5f + yo * 0.1f;
		rearFoot->xRot = 0.75f + yo * 0.1f;

		frontLeg->xRot = 1.3f + yo * 0.1f;
		frontLegTip->xRot = -0.5f - yo * 0.1f;
		frontFoot->xRot = 0.75f + yo * 0.1f;
		wing->render(scale,usecompiled);
		frontLeg->render(scale,usecompiled);
		rearLeg->render(scale,usecompiled);
		glScalef(-1, 1, 1);
		if (i == 0) 
		{
			glCullFace(GL_FRONT);
		}
	}
	glPopMatrix();
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);

	rr = -(float) Mth::sin(ttt * PI * 2) * 0.0f;
	roff = ttt * PI * 2;
	yy = 10;
	zz = 60;
	xx = 0;
	dragon->getLatencyPos(start, 11, a);
	for (int i = 0; i < 12; i++) 
	{
		dragon->getLatencyPos(p, 12 + i, a);
		rr += Mth::sin(i * 0.45f + roff) * 0.05f;
		neck->yRot = (rotWrap(p[0] - start[0]) * rotScale + 180) * PI / 180;
		neck->xRot = rr + (float) (p[1] - start[1]) * PI / 180 * rotScale * 5;
		neck->zRot = rotWrap(p[0] - rot) * PI / 180 * rotScale;
		neck->y = yy;
		neck->z = zz;
		neck->x = xx;
		yy += Mth::sin(neck->xRot) * 10;
		zz -= Mth::cos(neck->yRot) * Mth::cos(neck->xRot) * 10;
		xx -= Mth::sin(neck->yRot) * Mth::cos(neck->xRot) * 10;
		neck->render(scale,usecompiled);
	}
	glPopMatrix();
}
float DragonModel::rotWrap(double d) 
{
	while (d >= 180)
		d -= 360;
	while (d < -180)
		d += 360;
	return (float) d;
}