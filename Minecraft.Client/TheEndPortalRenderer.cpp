#include "stdafx.h"
#include "..\Minecraft.World\Random.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "Tesselator.h"
#include "TileEntityRenderDispatcher.h"
#include "Camera.h"
#include "..\Minecraft.World\FloatBuffer.h"
#include "TheEndPortalRenderer.h"

ResourceLocation TheEndPortalRenderer::END_SKY_LOCATION = ResourceLocation(TN_MISC_TUNNEL);
ResourceLocation TheEndPortalRenderer::END_PORTAL_LOCATION = ResourceLocation(TN_MISC_PARTICLEFIELD);
int TheEndPortalRenderer::RANDOM_SEED = 31100;
Random TheEndPortalRenderer::RANDOM = Random(RANDOM_SEED);

void TheEndPortalRenderer::render(shared_ptr<TileEntity> _table, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	// 4J Convert as we aren't using a templated class
	shared_ptr<TheEndPortalTileEntity> table = dynamic_pointer_cast<TheEndPortalTileEntity>(_table);
	float xx = static_cast<float>(tileEntityRenderDispatcher->xPlayer);
	float yy = static_cast<float>(tileEntityRenderDispatcher->yPlayer);
	float zz = static_cast<float>(tileEntityRenderDispatcher->zPlayer);

	glDisable(GL_LIGHTING);

	RANDOM.setSeed(RANDOM_SEED);

	float hoff = 12 / 16.0f;
	for (int i = 0; i < 16; i++)
	{
		glPushMatrix();

		float dist = (16 - (i));
		float sscale = 1 / 16.0f;

		float br = 1.0f / (dist + 1);
		if (i == 0)
		{
			this->bindTexture(&END_SKY_LOCATION);
			br = 0.1f;
			dist = 65;
			sscale = 1 / 8.0f;
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		if (i == 1)
		{
			this->bindTexture(&END_PORTAL_LOCATION);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			sscale = 1 / 2.0f;
		}

		float dd = static_cast<float>(-(y + hoff));
		{
			float ss1 = (float) (dd + Camera::yPlayerOffs);
			float ss2 = (float) (dd + dist + Camera::yPlayerOffs);
			float s = ss1 / ss2;
			s = static_cast<float>(y + hoff) + s;

			glTranslatef(xx, s, zz);
		}
		// 4J - note that the glTexGeni/glEnable calls don't actually do anything in our opengl wrapper version, everything is currently just inferred from the glTexGen calls.

		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

		glTexGen(GL_S, GL_OBJECT_PLANE, getBuffer(1, 0, 0, 0));
		glTexGen(GL_T, GL_OBJECT_PLANE, getBuffer(0, 0, 1, 0));
		glTexGen(GL_R, GL_OBJECT_PLANE, getBuffer(0, 0, 0, 1));
		glTexGen(GL_Q, GL_EYE_PLANE, getBuffer(0, 1, 0, 0));

		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_TEXTURE_GEN_Q);


		glPopMatrix();
		glMatrixMode(GL_TEXTURE);

		glPushMatrix();
		glLoadIdentity();

		glTranslatef(0, System::currentTimeMillis() % 700000 / 700000.0f, 0);
		glScalef(sscale, sscale, sscale);
		glTranslatef(0.5f, 0.5f, 0);
		glRotatef((i * i * 4321 + i * 9) * 2.0f, 0, 0, 1);
		glTranslatef(-0.5f, -0.5f, 0);
		glTranslatef(-xx, -zz, -yy);
		float ss1 = (float) (dd + Camera::yPlayerOffs);
		glTranslatef(Camera::xPlayerOffs * dist / ss1, Camera::zPlayerOffs * dist / ss1, -yy);

		Tesselator *t = Tesselator::getInstance();
		t->useProjectedTexture(true);				// 4J added - turns on both the generation of texture coordinates in the vertex shader & perspective divide of the texture coord in the pixel shader
		t->begin();

		float r = RANDOM.nextFloat() * 0.5f + 0.1f;
		float g = RANDOM.nextFloat() * 0.5f + 0.4f;
		float b = RANDOM.nextFloat() * 0.5f + 0.5f;
		if (i == 0) r = g = b = 1;
		t->color(r * br, g * br, b * br, 1.0f);
		t->vertex(x, y + hoff, z);
		t->vertex(x, y + hoff, z + 1);
		t->vertex(x + 1, y + hoff, z + 1);
		t->vertex(x + 1, y + hoff, z);
		t->end();

		t->useProjectedTexture(false);			// 4J added
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
	glDisable(GL_BLEND);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
	glEnable(GL_LIGHTING);
}

TheEndPortalRenderer::TheEndPortalRenderer()
{
	lb = MemoryTracker::createFloatBuffer(16);
}

FloatBuffer *TheEndPortalRenderer::getBuffer(float a, float b, float c, float d)
{
	lb->clear();
	lb->put(a)->put(b)->put(c)->put(d);
	lb->flip();
	return lb;
}
