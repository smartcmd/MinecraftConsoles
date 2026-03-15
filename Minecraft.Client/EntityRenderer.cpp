#include "stdafx.h"
#include "EntityRenderer.h"
#include "EntityRenderDispatcher.h"
#include "HumanoidModel.h"
#include "LocalPlayer.h"
#include "Options.h"
#include "TextureAtlas.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\Level.h"
#include "..\Minecraft.World\AABB.h"
#include "..\Minecraft.World\Mth.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"

ResourceLocation EntityRenderer::SHADOW_LOCATION = ResourceLocation(TN__CLAMP__MISC_SHADOW);

// 4J - added
EntityRenderer::EntityRenderer()
{
	model = nullptr;
	modelSlim = nullptr;
	newModel = nullptr;
	newModelSlim = nullptr;
	tileRenderer = new TileRenderer();
	shadowRadius = 0;
	shadowStrength = 1.0f;
}

EntityRenderer::~EntityRenderer()
{
	delete tileRenderer;
}

void EntityRenderer::bindTexture(shared_ptr<Entity> entity)
{
	bindTexture(getTextureLocation(entity));
}

void EntityRenderer::bindTexture(ResourceLocation *location)
{
	entityRenderDispatcher->textures->bindTexture(location);
}

bool EntityRenderer::bindTexture(const wstring& urlTexture, int backupTexture)
{
    Textures *t = entityRenderDispatcher->textures;

	// 4J-PB - no http textures on the xbox, mem textures instead
	
	//int id = t->loadHttpTexture(urlTexture, backupTexture);
	int id = t->loadMemTexture(urlTexture, backupTexture);

	if (id >= 0)
	{
		glBindTexture(GL_TEXTURE_2D, id);
		t->clearLastBoundId();
        return true;
    }
	else
	{
        return false;
    }
}

bool EntityRenderer::bindTexture(const wstring& urlTexture, const wstring &backupTexture)
{
	Textures *t = entityRenderDispatcher->textures;

	// 4J-PB - no http textures on the xbox, mem textures instead
	
	//int id = t->loadHttpTexture(urlTexture, backupTexture);
	int id = t->loadMemTexture(urlTexture, backupTexture);

	if (id >= 0)
	{
		glBindTexture(GL_TEXTURE_2D, id);
		t->clearLastBoundId();
		return true;
	}
	else
	{
		return false;
	}
}

void EntityRenderer::renderFlame(shared_ptr<Entity> e, double x, double y, double z, float a)
{
	glDisable(GL_LIGHTING);

	Icon *fire1 = Tile::fire->getTextureLayer(0);
	Icon *fire2 = Tile::fire->getTextureLayer(1);

	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));

	float s = e->bbWidth * 1.4f;
	glScalef(s, s, s);
	MemSect(31);
	bindTexture(&TextureAtlas::LOCATION_BLOCKS);
	MemSect(0);
	Tesselator *t = Tesselator::getInstance();

	float r = 0.5f;
	float xo = 0.0f;

	float h = e->bbHeight / s;
	float yo = static_cast<float>(e->y - e->bb->y0);

	glRotatef(-entityRenderDispatcher->playerRotY, 0, 1, 0);

	glTranslatef(0, 0, -0.3f + static_cast<int>(h) * 0.02f);
	glColor4f(1, 1, 1, 1);
	float zo = 0;
	int ss = 0;
	t->begin();
	while (h > 0)
	{
		Icon *tex = nullptr;
		if (ss % 2 == 0)
		{
			tex = fire1;
		}
		else
		{
			tex = fire2;
		}

		float u0 = tex->getU0();
		float v0 = tex->getV0();
		float u1 = tex->getU1();
		float v1 = tex->getV1();

		if (ss / 2 % 2 == 0)
		{
			float tmp = u1;
			u1 = u0;
			u0 = tmp;
		}
		t->vertexUV((float)(r - xo), (float)( 0 - yo), (float)( zo), (float)( u1), (float)( v1));
		t->vertexUV((float)(-r - xo), (float)( 0 - yo), (float)( zo), (float)( u0), (float)( v1));
		t->vertexUV((float)(-r - xo), (float)( 1.4f - yo), (float)( zo), (float)( u0), (float)( v0));
		t->vertexUV((float)(r - xo), (float)( 1.4f - yo), (float)( zo), (float)( u1), (float)( v0));
		h -= 0.45f;
		yo -= 0.45f;
		r *= 0.9f;
		zo += 0.03f;
		ss++;
	}
	t->end();
	glPopMatrix();
	glEnable(GL_LIGHTING);

}
void EntityRenderer::renderShadow(shared_ptr<Entity> e, double x, double y, double z, float pow, float a)
{
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	MemSect(31);
	entityRenderDispatcher->textures->bindTexture(&SHADOW_LOCATION);
	MemSect(0);

	Level *level = getLevel();

	glDepthMask(false);
	float r = shadowRadius;
	float fYLocalPlayerShadowOffset=0.0f;

	if (e->instanceof(eTYPE_MOB))
	{
		shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(e);
		r *= mob->getSizeScale();

		
		if (mob->instanceof(eTYPE_ANIMAL))
		{
			if (dynamic_pointer_cast<Animal>(mob)->isBaby())
			{
				r *= 0.5f;
			}
		}
	}

	double ex = e->xOld + (e->x - e->xOld) * a;
	double ey = e->yOld + (e->y - e->yOld) * a + e->getShadowHeightOffs();

	// 4J-PB - local players seem to have a position at their head, and remote players have a foot position.
	// get the shadow to render by changing the check here depending on the player type
	if(e->instanceof(eTYPE_LOCALPLAYER))
	{
		ey-=1.62;
		fYLocalPlayerShadowOffset=-1.62f;
	}
	double ez = e->zOld + (e->z - e->zOld) * a;

	int x0 = Mth::floor(ex - r);
	int x1 = Mth::floor(ex + r);
	int y0 = Mth::floor(ey - r);
	int y1 = Mth::floor(ey);
	int z0 = Mth::floor(ez - r);
	int z1 = Mth::floor(ez + r);

	double xo = x - ex;
	double yo = y - ey;
	double zo = z - ez;

	Tesselator *tt = Tesselator::getInstance();
	tt->begin();
	for (int xt = x0; xt <= x1; xt++)
		for (int yt = y0; yt <= y1; yt++)
			for (int zt = z0; zt <= z1; zt++)
			{				
				int t = level->getTile(xt, yt - 1, zt);
				if (t > 0 && level->getRawBrightness(xt, yt, zt) > 3)
				{
					renderTileShadow(Tile::tiles[t], x, y + e->getShadowHeightOffs() + fYLocalPlayerShadowOffset, z, xt, yt , zt, pow, r, xo, yo + e->getShadowHeightOffs() + fYLocalPlayerShadowOffset, zo);
				}			
			}
	tt->end();

	glColor4f(1, 1, 1, 1);
	glDisable(GL_BLEND);
	glDepthMask(true);
	glEnable(GL_LIGHTING);

}

Level *EntityRenderer::getLevel()
{
	return entityRenderDispatcher->level;
}

void EntityRenderer::renderTileShadow(Tile *tt, double x, double y, double z, int xt, int yt, int zt, float pow, float r, double xo, double yo, double zo)
{
	Tesselator *t = Tesselator::getInstance();
	if (!tt->isCubeShaped()) return;

	double a = ((pow - (y - (yt + yo)) / 2) * 0.5f) * getLevel()->getBrightness(xt, yt, zt);
	if (a < 0) return;
	if (a > 1) a = 1;
	
	t->color(1.0f, 1.0f, 1.0f, static_cast<float>(a));
	// glColor4f(1, 1, 1, (float) a);

	double x0 = xt + tt->getShapeX0() + xo;
	double x1 = xt + tt->getShapeX1() + xo;
	double y0 = yt + tt->getShapeY0() + yo + 1.0 / 64.0f;
	double z0 = zt + tt->getShapeZ0() + zo;
	double z1 = zt + tt->getShapeZ1() + zo;

	float u0 = static_cast<float>((x - (x0)) / 2 / r + 0.5f);
	float u1 = static_cast<float>((x - (x1)) / 2 / r + 0.5f);
	float v0 = static_cast<float>((z - (z0)) / 2 / r + 0.5f);
	float v1 = static_cast<float>((z - (z1)) / 2 / r + 0.5f);

	// u0 = 0;
	// v0 = 0;
	// u1 = 1;
	// v1 = 1;

	t->vertexUV(static_cast<float>(x0), static_cast<float>(y0), static_cast<float>(z0), (float)( u0), (float)( v0));
	t->vertexUV(static_cast<float>(x0), static_cast<float>(y0), static_cast<float>(z1), (float)( u0), (float)( v1));
	t->vertexUV(static_cast<float>(x1), static_cast<float>(y0), static_cast<float>(z1), (float)( u1), (float)( v1));
	t->vertexUV(static_cast<float>(x1), static_cast<float>(y0), static_cast<float>(z0), (float)( u1), (float)( v0));
}

void EntityRenderer::render(AABB *bb, double xo, double yo, double zo)
{
	glDisable(GL_TEXTURE_2D);
	Tesselator *t = Tesselator::getInstance();
	glColor4f(1, 1, 1, 1);
	t->begin();
	t->offset(static_cast<float>(xo), static_cast<float>(yo), static_cast<float>(zo));
	t->normal(0, 0, -1);
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y1), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y1), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y0), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y0), static_cast<float>(bb->z0));

	t->normal(0, 0, 1);
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y0), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y0), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y1), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y1), static_cast<float>(bb->z1));

	t->normal(0, -1, 0);
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y0), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y0), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y0), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y0), static_cast<float>(bb->z1));

	t->normal(0, 1, 0);
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y1), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y1), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y1), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y1), static_cast<float>(bb->z0));

	t->normal(-1, 0, 0);
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y0), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y1), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y1), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y0), static_cast<float>(bb->z0));

	t->normal(1, 0, 0);
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y0), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y1), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y1), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y0), static_cast<float>(bb->z1));
	t->offset(0, 0, 0);
	t->end();
	glEnable(GL_TEXTURE_2D);
	// model.render(0, 1)
}

void EntityRenderer::renderFlat(AABB *bb)
{
	Tesselator *t = Tesselator::getInstance();
	t->begin();
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y1), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y1), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y0), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y0), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y0), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y0), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y1), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y1), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y0), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y0), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y0), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y0), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y1), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y1), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y1), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y1), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y0), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y1), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y1), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x0), static_cast<float>(bb->y0), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y0), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y1), static_cast<float>(bb->z0));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y1), static_cast<float>(bb->z1));
	t->vertex(static_cast<float>(bb->x1), static_cast<float>(bb->y0), static_cast<float>(bb->z1));
	t->end();
}

void EntityRenderer::renderFlat(float x0, float y0, float z0, float x1, float y1, float z1)
{
	Tesselator *t = Tesselator::getInstance();
	t->begin();
	t->vertex(x0, y1, z0);
	t->vertex(x1, y1, z0);
	t->vertex(x1, y0, z0);
	t->vertex(x0, y0, z0);
	t->vertex(x0, y0, z1);
	t->vertex(x1, y0, z1);
	t->vertex(x1, y1, z1);
	t->vertex(x0, y1, z1);
	t->vertex(x0, y0, z0);
	t->vertex(x1, y0, z0);
	t->vertex(x1, y0, z1);
	t->vertex(x0, y0, z1);
	t->vertex(x0, y1, z1);
	t->vertex(x1, y1, z1);
	t->vertex(x1, y1, z0);
	t->vertex(x0, y1, z0);
	t->vertex(x0, y0, z1);
	t->vertex(x0, y1, z1);
	t->vertex(x0, y1, z0);
	t->vertex(x0, y0, z0);
	t->vertex(x1, y0, z0);
	t->vertex(x1, y1, z0);
	t->vertex(x1, y1, z1);
	t->vertex(x1, y0, z1);
	t->end();
}

void EntityRenderer::init(EntityRenderDispatcher *entityRenderDispatcher)
{
	this->entityRenderDispatcher = entityRenderDispatcher;
}

void EntityRenderer::postRender(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a, bool bRenderPlayerShadow)
{
	if( !entityRenderDispatcher->isGuiRender )		// 4J - added, don't render shadow in gui as it uses its own blending, and we have globally enabled blending for interface opacity
	{
		if (bRenderPlayerShadow && entityRenderDispatcher->options->fancyGraphics && shadowRadius > 0 && !entity->isInvisible())
		{
			double dist = entityRenderDispatcher->distanceToSqr(entity->x, entity->y, entity->z);
			float pow = static_cast<float>((1 - dist / (16.0f * 16.0f)) * shadowStrength);
			if (pow > 0)
			{
				renderShadow(entity, x, y, z, pow, a);
			}
		}
	}
	if (entity->isOnFire()) renderFlame(entity, x, y, z, a);
}

Font *EntityRenderer::getFont()
{
	return entityRenderDispatcher->getFont();
}

void EntityRenderer::registerTerrainTextures(IconRegister *iconRegister)
{
}

ResourceLocation *EntityRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	return nullptr;
}