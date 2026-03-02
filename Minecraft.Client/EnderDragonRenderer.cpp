#include "stdafx.h"
#include "DragonModel.h"
#include "..\Minecraft.World\net.minecraft.world.entity.boss.enderdragon.h"
#include "Tesselator.h"
#include "Lighting.h"
#include "EnderDragonRenderer.h"

shared_ptr<EnderDragon> EnderDragonRenderer::bossInstance;
int EnderDragonRenderer::currentModel;

EnderDragonRenderer::EnderDragonRenderer() : MobRenderer(new DragonModel(0), 0.5f)
{
	currentModel = 0;
	dragonModel = (DragonModel *) model;
	this->setArmor(model);
}

void EnderDragonRenderer::setupRotations(shared_ptr<Mob> _mob, float bob, float bodyRot, float a)
{		
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<EnderDragon> mob = dynamic_pointer_cast<EnderDragon>(_mob);

	// 4J - reorganised a bit so we can free allocations
	double lpComponents[3];
	doubleArray lp = doubleArray(lpComponents, 3);
	mob->getLatencyPos(lp, 7, a);
	float yr = lp[0];
	//mob->getLatencyPos(lp, 5, a);
	//float rot2 = lp[1];
	//mob->getLatencyPos(lp, 10,a);
	//rot2 -= lp[1];
	float rot2 = mob->getTilt(a);

	glRotatef(-yr, 0, 1, 0);
	
	glRotatef(rot2, 1, 0, 0);
	//glRotatef(rot2 * 10, 1, 0, 0);

	glTranslatef(0, 0, 1);
	if (mob->deathTime > 0)
	{
		float fall = (mob->deathTime + a - 1) / 20.0f * 1.6f;
		fall = sqrt(fall);
		if (fall > 1) fall = 1;
		glRotatef(fall * getFlipDegrees(mob), 0, 0, 1);
	}
}

void EnderDragonRenderer::renderModel(shared_ptr<Entity> _mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<EnderDragon> mob = dynamic_pointer_cast<EnderDragon>(_mob);

	if (mob->dragonDeathTime > 0)
	{
		float tt = (mob->dragonDeathTime / 200.0f);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, tt);
		bindTexture(mob->customTextureUrl, TN_MOB_ENDERDRAGON_SHUFFLE); // 4J was "/mob/enderdragon/shuffle.png"
		model->render(mob, wp, ws, bob, headRotMinusBodyRot, headRotx, scale, true);
		glAlphaFunc(GL_GREATER, 0.1f);

		glDepthFunc(GL_EQUAL);
	}


	bindTexture(mob->customTextureUrl, mob->getTexture());
	model->render(mob, wp, ws, bob, headRotMinusBodyRot, headRotx, scale, true);

	if (mob->hurtTime > 0)
	{
		glDepthFunc(GL_EQUAL);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1, 0, 0, 0.5f);
#ifdef __PSVITA__
		// AP - not sure that the usecompiled flag is supposed to be false. This makes it really slow on vita. Making it true still seems to look the same
		model->render(mob, wp, ws, bob, headRotMinusBodyRot, headRotx, scale, true);
#else
		model->render(mob, wp, ws, bob, headRotMinusBodyRot, headRotx, scale, false);
#endif
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LEQUAL);
	}
}

void EnderDragonRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<EnderDragon> mob = dynamic_pointer_cast<EnderDragon>(_mob);
	EnderDragonRenderer::bossInstance = mob;
	if (currentModel != DragonModel::MODEL_ID)
	{
		model = new DragonModel(0);
		currentModel = DragonModel::MODEL_ID;
	}
	MobRenderer::render(mob, x, y, z, rot, a);
	if (mob->nearestCrystal != NULL)
	{
		float tt = mob->nearestCrystal->time + a;
		float hh = sin(tt * 0.2f) / 2 + 0.5f;
		hh = (hh * hh + hh) * 0.2f;

		float xd = (float) (mob->nearestCrystal->x - mob->x - (mob->xo - mob->x) * (1 - a));
		float yd = (float) (hh + mob->nearestCrystal->y - 1 - mob->y - (mob->yo - mob->y) * (1 - a));
		float zd = (float) (mob->nearestCrystal->z - mob->z - (mob->zo - mob->z) * (1 - a));

		float sdd = sqrt(xd * xd + zd * zd);
		float dd = sqrt(xd * xd + yd * yd + zd * zd);

		// this fixes a problem when the dragon is hit and the beam goes black because the diffuse colour isn't being reset in MobRenderer::render
		glColor4f(1, 1, 1, 1);

		glPushMatrix();
		glTranslatef((float) x, (float) y + 2, (float) z);
		glRotatef((float) (-atan2(zd, xd)) * 180.0f / PI - 90.0f, 0, 1, 0);
		glRotatef((float) (-atan2(sdd, yd)) * 180.0f / PI - 90.0f, 1, 0, 0);

		// 4J-PB - Rotating the healing beam too
		static float fRot=0.0f;
		glRotatef(fRot, 0, 0, 1);
		fRot+=0.5f;		// 4J - rate of rotation changed from 5.0 to 0.5 for photosensitivity reasons
		if(fRot>=360.0f)
		{
			fRot=0.0f;
		}

		Tesselator *t = Tesselator::getInstance();
		Lighting::turnOff();
		glDisable(GL_CULL_FACE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);

		bindTexture(TN_MOB_ENDERDRAGON_BEAM); // 4J was "/mob/enderdragon/beam.png"

		glShadeModel(GL_SMOOTH);

		float v0 = 0 - (mob->tickCount + a) * 0.005f;		// 4J - rate of movement changed from 0.01 to 0.005 for photosensitivity reasons
		float v1 = sqrt(xd * xd + yd * yd + zd * zd) / 32.0f - (mob->tickCount + a) * 0.005f;

		t->begin(GL_TRIANGLE_STRIP);

		int steps = 8;
		for (int i = 0; i <= steps; i++)
		{
			double d=i % steps * PI * 2 / steps;
			float s = sin(i % steps * PI * 2 / steps) * 0.75f;
			float c = cos(i % steps * PI * 2 / steps) * 0.75f;
			float u = i % steps * 1.0f / steps;
			//t->color(0x000000);
			t->vertexUV(s * 0.2f, c * 0.2f, 0, u, v1);
			//t->color(0xffffff);
			t->vertexUV(s, c, dd, u, v0);
		}

		t->end();
		glEnable(GL_CULL_FACE);
		glShadeModel(GL_FLAT);
		glDisable(GL_BLEND);

		glPopMatrix();
		Lighting::turnOn();
	}
}

void EnderDragonRenderer::additionalRendering(shared_ptr<Mob> _mob, float a)
{		
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<EnderDragon> mob = dynamic_pointer_cast<EnderDragon>(_mob);
	MobRenderer::additionalRendering(mob, a);
	Tesselator *t = Tesselator::getInstance();

	if (mob->dragonDeathTime > 0)
	{
		Lighting::turnOff();
		float tt = ((mob->dragonDeathTime + a) / 200.0f);
		float overDrive = 0;
		if (tt > 0.8f)
		{
			overDrive = (tt - 0.8f) / 0.2f;
		}

		Random random(432);
		glDisable(GL_TEXTURE_2D);
		glShadeModel(GL_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_CULL_FACE);
		glDepthMask(false);
		glPushMatrix();
		glTranslatef(0, -1, -2);
		for (int i = 0; i < (tt + tt * tt) / 2 * 60; i++)
		{
			glRotatef(random.nextFloat() * 360, 1, 0, 0);
			glRotatef(random.nextFloat() * 360, 0, 1, 0);
			glRotatef(random.nextFloat() * 360, 0, 0, 1);
			glRotatef(random.nextFloat() * 360, 1, 0, 0);
			glRotatef(random.nextFloat() * 360, 0, 1, 0);
			glRotatef(random.nextFloat() * 360 + tt * 90, 0, 0, 1);
			t->begin(GL_TRIANGLE_FAN);
			float dist = random.nextFloat() * 20 + 5 + overDrive * 10;
			float w = random.nextFloat() * 2 + 1 + overDrive * 2;
			t->color(0xffffff, (int) (255 * (1 - overDrive)));
			t->vertex(0, 0, 0);
			t->color(0xff00ff, 0);
			t->vertex(-0.866 * w, dist, -0.5f * w);
			t->vertex(+0.866 * w, dist, -0.5f * w);
			t->vertex(0, dist, 1 * w);
			t->vertex(-0.866 * w, dist, -0.5f * w);
			t->end();
		}
		glPopMatrix();
		glDepthMask(true);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glShadeModel(GL_FLAT);
		glColor4f(1, 1, 1, 1);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
		Lighting::turnOn();
	}

}

int EnderDragonRenderer::prepareArmor(shared_ptr<Mob> _mob, int layer, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<EnderDragon> mob = dynamic_pointer_cast<EnderDragon>(_mob);

	if (layer == 1)
	{
		glDepthFunc(GL_LEQUAL);
	}
	if (layer != 0) return -1;

	bindTexture(TN_MOB_ENDERDRAGON_ENDEREYES); // 4J was "/mob/enderdragon/ender_eyes.png"
	float br = 1;
	glEnable(GL_BLEND);
	// 4J Stu - We probably don't need to do this on 360 either (as we force it back on the renderer)
	// However we do want it off for other platforms that don't force it on in the render lib CBuff handling
	// Several texture packs have fully transparent bits that break if this is off
#ifdef _XBOX
    glDisable(GL_ALPHA_TEST);
#endif
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_LIGHTING);
	glDepthFunc(GL_EQUAL);

	if (SharedConstants::TEXTURE_LIGHTING)
	{
		int col = 0xf0f0;
		int u = col % 65536;
		int v = col / 65536;

		glMultiTexCoord2f(GL_TEXTURE1, u / 1.0f, v / 1.0f);
		glColor4f(1, 1, 1, 1);
	}

	glEnable(GL_LIGHTING);
	glColor4f(1, 1, 1, br);
	return 1;
}
