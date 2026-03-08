#include "stdafx.h"
#include "ArrowRenderer.h"
#include "../Minecraft.World/net.minecraft.world.entity.projectile.h"
#include "../Minecraft.World/Mth.h"

ResourceLocation ArrowRenderer::ARROW_LOCATION = ResourceLocation(TN_ITEM_ARROWS);

void ArrowRenderer::render(shared_ptr<Entity> _arrow, double x, double y, double z, float rot, float a)
{
	// 4J - original version used generics and thus had an input parameter of type Arrow rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
	shared_ptr<Arrow> arrow = dynamic_pointer_cast<Arrow>(_arrow);
    bindTexture(_arrow); // 4J - was L"/item/arrows.png"

    glPushMatrix();

	float yRot = arrow->yRot;
	float xRot = arrow->xRot;
	float yRotO = arrow->yRotO;
	float xRotO = arrow->xRotO;
	if( ( yRot - yRotO ) > 180.0f ) yRot -= 360.0f;
	else if( ( yRot - yRotO ) < -180.0f ) yRot += 360.0f;
	if( ( xRot - xRotO ) > 180.0f ) xRot -= 360.0f;
	else if( ( xRot - xRotO ) < -180.0f ) xRot += 360.0f;

    glTranslatef((float)x, (float)y, (float)z);
    glRotatef(yRotO + (yRot - yRotO) * a - 90, 0, 1, 0);
    glRotatef(xRotO + (xRot - xRotO) * a, 0, 0, 1);

    Tesselator *t = Tesselator::getInstance();
    int type = 0;

    float u0 = 0 / 32.0f;
    float u1 = 16 / 32.0f;
    float v0 = (0 + type * 10) / 32.0f;
    float v1 = (5 + type * 10) / 32.0f;

    float u02 = 0 / 32.0f;
    float u12 = 5 / 32.0f;
    float v02 = (5 + type * 10) / 32.0f;
    float v12 = (10 + type * 10) / 32.0f;
    float ss = 0.9f / 16.0f;
    glEnable(GL_RESCALE_NORMAL);
    float shake = arrow->shakeTime-a;
    if (shake>0)
    {
        float pow = -Mth::sin(shake*3)*shake;
        glRotatef(pow, 0, 0, 1);
    }
    glRotatef(45, 1, 0, 0);
    glScalef(ss, ss, ss);

    glTranslatef(-4, 0, 0);

//    glNormal3f(ss, 0, 0);		// 4J - changed to use tesselator
    t->begin();
	t->normal(1,0,0);
    t->vertexUV((float)(-7), (float)( -2), (float)( -2), (float)( u02), (float)( v02));
    t->vertexUV((float)(-7), (float)( -2), (float)( +2), (float)( u12), (float)( v02));
    t->vertexUV((float)(-7), (float)( +2), (float)( +2), (float)( u12), (float)( v12));
    t->vertexUV((float)(-7), (float)( +2), (float)( -2), (float)( u02), (float)( v12));
    t->end();

//    glNormal3f(-ss, 0, 0);	// 4J - changed to use tesselator
    t->begin();
	t->normal(-1,0,0);
    t->vertexUV((float)(-7), (float)( +2), (float)( -2), (float)( u02), (float)( v02));
    t->vertexUV((float)(-7), (float)( +2), (float)( +2), (float)( u12), (float)( v02));
    t->vertexUV((float)(-7), (float)( -2), (float)( +2), (float)( u12), (float)( v12));
    t->vertexUV((float)(-7), (float)( -2), (float)( -2), (float)( u02), (float)( v12));
    t->end();

    for (int i = 0; i < 4; i++)
	{

        glRotatef(90, 1, 0, 0);
//        glNormal3f(0, 0, ss);		// 4J - changed to use tesselator
        t->begin();
		t->normal(0,0,1);
        t->vertexUV((float)(-8), (float)( -2), (float)( 0), (float)( u0), (float)( v0));
        t->vertexUV((float)(+8), (float)( -2), (float)( 0), (float)( u1), (float)( v0));
        t->vertexUV((float)(+8), (float)( +2), (float)( 0), (float)( u1), (float)( v1));
        t->vertexUV((float)(-8), (float)( +2), (float)( 0), (float)( u0), (float)( v1));
        t->end();
    }
    glDisable(GL_RESCALE_NORMAL);
    glPopMatrix();
}

ResourceLocation *ArrowRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &ARROW_LOCATION;
}