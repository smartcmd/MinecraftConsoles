#include "stdafx.h"
#include "PaintingRenderer.h"
#include "entityRenderDispatcher.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\Random.h"
#include "..\Minecraft.World\Mth.h"

ResourceLocation PaintingRenderer::PAINTING_LOCATION(TN_ART_KZ);

PaintingRenderer::PaintingRenderer()
{
	random = new Random();
}

void PaintingRenderer::render(shared_ptr<Entity> _painting, double x, double y, double z, float rot, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Painting> painting = dynamic_pointer_cast<Painting>(_painting);
	
    random->setSeed(187);

    glPushMatrix();
    glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    glRotatef(rot, 0, 1, 0);
    glEnable(GL_RESCALE_NORMAL);
    bindTexture(painting);	// 4J was L"/art/kz.png"

    Painting::Motive *motive = painting->motive;

    float s = 1 / 16.0f;
    glScalef(s, s, s);
    renderPainting(painting, motive->w, motive->h, motive->uo, motive->vo);
    glDisable(GL_RESCALE_NORMAL);
    glPopMatrix();
}

void PaintingRenderer::renderPainting(shared_ptr<Painting> painting, int w, int h, int uo, int vo)
{
	float xx0 = -w / 2.0f;
	float yy0 = -h / 2.0f;

	float edgeWidth = 0.5f;

	// Back
	float bu0 = (12 * 16) / 256.0f;
	float bu1 = (12 * 16 + 16) / 256.0f;
	float bv0 = (0) / 256.0f;
	float bv1 = (0 + 16) / 256.0f;

	// Border
	float uu0 = (12 * 16) / 256.0f;
	float uu1 = (12 * 16 + 16) / 256.0f;
	float uv0 = (0.5f) / 256.0f;
	float uv1 = (0.5f) / 256.0f;

	// Border
	float su0 = (12 * 16 + 0.5f) / 256.0f;
	float su1 = (12 * 16 + 0.5f) / 256.0f;
	float sv0 = (0) / 256.0f;
	float sv1 = (0 + 16) / 256.0f;

	for (int xs = 0; xs < w / 16; xs++)
	{
		for (int ys = 0; ys < h / 16; ys++) {
			float x0 = xx0 + (xs + 1) * 16;
			float x1 = xx0 + (xs) * 16;
			float y0 = yy0 + (ys + 1) * 16;
			float y1 = yy0 + (ys) * 16;

			setBrightness(painting, (x0 + x1) / 2, (y0 + y1) / 2);

			// Painting
			float fu0 = (uo + w - (xs) * 16) / 256.0f;
			float fu1 = (uo + w - (xs + 1) * 16) / 256.0f;
			float fv0 = (vo + h - (ys) * 16) / 256.0f;
			float fv1 = (vo + h - (ys + 1) * 16) / 256.0f;

			Tesselator *t = Tesselator::getInstance();
			t->begin();
			t->normal(0, 0, -1);
			t->vertexUV(x0, y1, -edgeWidth, fu1, fv0);
			t->vertexUV(x1, y1, -edgeWidth, fu0, fv0);
			t->vertexUV(x1, y0, -edgeWidth, fu0, fv1);
			t->vertexUV(x0, y0, -edgeWidth, fu1, fv1);

			t->normal(0, 0, 1);
			t->vertexUV(x0, y0, edgeWidth, bu0, bv0);
			t->vertexUV(x1, y0, edgeWidth, bu1, bv0);
			t->vertexUV(x1, y1, edgeWidth, bu1, bv1);
			t->vertexUV(x0, y1, edgeWidth, bu0, bv1);

			t->normal(0, 1, 0);
			t->vertexUV(x0, y0, -edgeWidth, uu0, uv0);
			t->vertexUV(x1, y0, -edgeWidth, uu1, uv0);
			t->vertexUV(x1, y0, edgeWidth, uu1, uv1);
			t->vertexUV(x0, y0, edgeWidth, uu0, uv1);
			
			t->normal(0, -1, 0);
			t->vertexUV(x0, y1, edgeWidth, uu0, uv0);
			t->vertexUV(x1, y1, edgeWidth, uu1, uv0);
			t->vertexUV(x1, y1, -edgeWidth, uu1, uv1);
			t->vertexUV(x0, y1, -edgeWidth, uu0, uv1);
			
			t->normal(-1, 0, 0);
			t->vertexUV(x0, y0, edgeWidth, su1, sv0);
			t->vertexUV(x0, y1, edgeWidth, su1, sv1);
			t->vertexUV(x0, y1, -edgeWidth, su0, sv1);
			t->vertexUV(x0, y0, -edgeWidth, su0, sv0);
			
			t->normal(1, 0, 0);
			t->vertexUV(x1, y0, -edgeWidth, su1, sv0);
			t->vertexUV(x1, y1, -edgeWidth, su1, sv1);
			t->vertexUV(x1, y1, edgeWidth, su0, sv1);
			t->vertexUV(x1, y0, edgeWidth, su0, sv0);
			t->end();
		}
	}
}

void PaintingRenderer::setBrightness(shared_ptr<Painting> painting, float ss, float ya)
{
    int x = Mth::floor(painting->x);
    int y = Mth::floor(painting->y + ya/16.0f);
    int z = Mth::floor(painting->z);
    if (painting->dir == 0) x = Mth::floor(painting->x + ss/16.0f);
    if (painting->dir == 1) z = Mth::floor(painting->z - ss/16.0f);
    if (painting->dir == 2) x = Mth::floor(painting->x - ss/16.0f);
    if (painting->dir == 3) z = Mth::floor(painting->z + ss/16.0f);

    int col = this->entityRenderDispatcher->level->getLightColor(x, y, z, 0);
    int u = col % 65536;
    int v = col / 65536;
    glMultiTexCoord2f(0, u, v);
    glColor3f(1, 1, 1);
}

ResourceLocation *PaintingRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &PAINTING_LOCATION;
}