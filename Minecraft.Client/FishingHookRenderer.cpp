#include "stdafx.h"
#include "FishingHookRenderer.h"
#include "EntityRenderDispatcher.h"
#include "Options.h"
#include "..\Minecraft.World\net.minecraft.world.entity.projectile.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\Vec3.h"
#include "..\Minecraft.World\Mth.h"
#include "MultiPlayerLocalPlayer.h"

ResourceLocation FishingHookRenderer::PARTICLE_LOCATION = ResourceLocation(TN_PARTICLES);

void FishingHookRenderer::render(shared_ptr<Entity> _hook, double x, double y, double z, float rot, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<FishingHook> hook = dynamic_pointer_cast<FishingHook>(_hook);

    glPushMatrix();

    glTranslatef((float) x, (float) y, (float) z);
    glEnable(GL_RESCALE_NORMAL);
    glScalef(1 / 2.0f, 1 / 2.0f, 1 / 2.0f);
    int xi = 1;
    int yi = 2;
    bindTexture(hook);		// 4J was L"/particles.png"
    Tesselator *t = Tesselator::getInstance();

    float u0 = (xi * 8 + 0) / 128.0f;
    float u1 = (xi * 8 + 8) / 128.0f;
    float v0 = (yi * 8 + 0) / 128.0f;
    float v1 = (yi * 8 + 8) / 128.0f;


    float r = 1.0f;
    float xo = 0.5f;
    float yo = 0.5f;

    glRotatef(180 - entityRenderDispatcher->playerRotY, 0, 1, 0);
    glRotatef(-entityRenderDispatcher->playerRotX, 1, 0, 0);
    t->begin();
    t->normal(0, 1, 0);
    t->vertexUV((float)(0 - xo), (float)( 0 - yo), (float)( 0), (float)( u0), (float)( v1));
    t->vertexUV((float)(r - xo), (float)( 0 - yo), (float)( 0), (float)( u1), (float)( v1));
    t->vertexUV((float)(r - xo), (float)( 1 - yo), (float)( 0), (float)( u1), (float)( v0));
    t->vertexUV((float)(0 - xo), (float)( 1 - yo), (float)( 0), (float)( u0), (float)( v0));
    t->end();

    glDisable(GL_RESCALE_NORMAL);
    glPopMatrix();


    if (hook->owner != NULL)
	{
        float swing = hook->owner->getAttackAnim(a);
        float swing2 = (float) Mth::sin(sqrt(swing) * PI);


        Vec3 *vv = Vec3::newTemp(-0.5, 0.03, 0.8);
        vv->xRot(-(hook->owner->xRotO + (hook->owner->xRot - hook->owner->xRotO) * a) * PI / 180);
        vv->yRot(-(hook->owner->yRotO + (hook->owner->yRot - hook->owner->yRotO) * a) * PI / 180);
        vv->yRot(swing2 * 0.5f);
        vv->xRot(-swing2 * 0.7f);

        double xp = hook->owner->xo + (hook->owner->x - hook->owner->xo) * a + vv->x;
        double yp = hook->owner->yo + (hook->owner->y - hook->owner->yo) * a + vv->y;
        double zp = hook->owner->zo + (hook->owner->z - hook->owner->zo) * a + vv->z;
		double yOffset = hook->owner == dynamic_pointer_cast<Player>(Minecraft::GetInstance()->player) ? 0 : hook->owner->getHeadHeight();

		// 4J-PB - changing this to be per player
		//if (this->entityRenderDispatcher->options->thirdPersonView)
		if (hook->owner->ThirdPersonView() > 0)
 		{
            float rr = (float) (hook->owner->yBodyRotO + (hook->owner->yBodyRot - hook->owner->yBodyRotO) * a) * PI / 180;
            double ss = Mth::sin((float) rr);
            double cc = Mth::cos((float) rr);
            xp = hook->owner->xo + (hook->owner->x - hook->owner->xo) * a - cc * 0.35 - ss * 0.85;
            yp = hook->owner->yo + yOffset + (hook->owner->y - hook->owner->yo) * a - 0.45;
            zp = hook->owner->zo + (hook->owner->z - hook->owner->zo) * a - ss * 0.35 + cc * 0.85;
        }

        double xh = hook->xo + (hook->x - hook->xo) * a;
        double yh = hook->yo + (hook->y - hook->yo) * a + 4 / 16.0f;
        double zh = hook->zo + (hook->z - hook->zo) * a;

        double xa = (float) (xp - xh);
        double ya = (float) (yp - yh);
        double za = (float) (zp - zh);

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        t->begin(GL_LINE_STRIP);
        t->color(0x000000);
        int steps = 16;
        for (int i = 0; i <= steps; i++)
		{
            float aa = i / (float) steps;
            t->vertex((float)(x + xa * aa), (float)( y + ya * (aa * aa + aa) * 0.5 + 4 / 16.0f), (float)( z + za * aa));
        }
        t->end();
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
    }
}

ResourceLocation *FishingHookRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &PARTICLE_LOCATION;
}
