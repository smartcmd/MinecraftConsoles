#include "stdafx.h"
#include "FireballRenderer.h"
#include "EntityRenderDispatcher.h"
#include "TextureAtlas.h"
#include "../Minecraft.World/net.minecraft.world.entity.projectile.h"
#include "../Minecraft.World/net.minecraft.world.item.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/net.minecraft.world.phys.h"
#include "../Minecraft.World/net.minecraft.world.h"

FireballRenderer::FireballRenderer(float scale)
{
	this->scale = scale;
}

void FireballRenderer::render(shared_ptr<Entity> _fireball, double x, double y, double z, float rot, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Fireball> fireball = dynamic_pointer_cast<Fireball>(_fireball);

    glPushMatrix();

    glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    glEnable(GL_RESCALE_NORMAL);
    float s = scale;
    glScalef(s / 1.0f, s / 1.0f, s / 1.0f);
	Icon *icon = Item::fireball->getIcon(fireball->GetType()==eTYPE_DRAGON_FIREBALL?1:0);//14 + 2 * 16;
	MemSect(31);
    bindTexture(fireball);
	MemSect(0);
    Tesselator *t = Tesselator::getInstance();

    float u0 = icon->getU0();
    float u1 = icon->getU1();
    float v0 = icon->getV0();
    float v1 = icon->getV1();

    float r = 1.0f;
    float xo = 0.5f;
    float yo = 0.25f;

    glRotatef(180 - entityRenderDispatcher->playerRotY, 0, 1, 0);
    glRotatef(-entityRenderDispatcher->playerRotX, 1, 0, 0);
    t->begin();
    t->normal(0, 1, 0);
    t->vertexUV((float)(0 - xo), (float)( 0 - yo), static_cast<float>(0), (float)( u0), (float)( v1));
    t->vertexUV((float)(r - xo), (float)( 0 - yo), static_cast<float>(0), (float)( u1), (float)( v1));
    t->vertexUV((float)(r - xo), (float)( 1 - yo), static_cast<float>(0), (float)( u1), (float)( v0));
    t->vertexUV((float)(0 - xo), (float)( 1 - yo), static_cast<float>(0), (float)( u0), (float)( v0));
    t->end();

    glDisable(GL_RESCALE_NORMAL);
    glPopMatrix();

}

// 4J Added override. Based on EntityRenderer::renderFlame
void FireballRenderer::renderFlame(shared_ptr<Entity> e, double x, double y, double z, float a)
{
	glDisable(GL_LIGHTING);
	Icon *tex = Tile::fire->getTextureLayer(0);

    glPushMatrix();
    glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));

    float s = e->bbWidth * 1.4f;
    glScalef(s, s, s);
	MemSect(31);
    bindTexture(&TextureAtlas::LOCATION_BLOCKS);
	MemSect(0);
    Tesselator *t = Tesselator::getInstance();

    float r = 1.0f;
    float xo = 0.5f;
//        float yo = 0.0f;

    float h = e->bbHeight / s;
    float yo = static_cast<float>(e->y - e->bb->y0);

    //glRotatef(-entityRenderDispatcher->playerRotY, 0, 1, 0);

	
    glRotatef(180 - entityRenderDispatcher->playerRotY, 0, 1, 0);
    glRotatef(-entityRenderDispatcher->playerRotX, 1, 0, 0);
	glTranslatef(0,0,0.1f);
    //glTranslatef(0, 0, -0.3f + ((int) h) * 0.02f);
    glColor4f(1, 1, 1, 1);
    // glRotatef(-playerRotX, 1, 0, 0);
    float zo = 0;
    t->begin();
	t->normal(0, 1, 0);

	float u0 = tex->getU0();
	float v0 = tex->getV0();
	float u1 = tex->getU1();
	float v1 = tex->getV1();

	float tmp = u1;
	u1 = u0;
	u0 = tmp;

	t->vertexUV((float)(0 - xo), (float)( 0 - yo), static_cast<float>(0), (float)( u1), (float)( v1));
	t->vertexUV((float)(r - xo), (float)( 0 - yo), static_cast<float>(0), (float)( u0), (float)( v1));
	t->vertexUV((float)(r - xo), (float)( 1.4f - yo), static_cast<float>(0), (float)( u0), (float)( v0));
	t->vertexUV((float)(0 - xo), (float)( 1.4f - yo), static_cast<float>(0), (float)( u1), (float)( v0));

    t->end();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

ResourceLocation *FireballRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &TextureAtlas::LOCATION_ITEMS;
}