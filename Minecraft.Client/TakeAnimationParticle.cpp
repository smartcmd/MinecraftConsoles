#include "stdafx.h"
#include "TakeAnimationParticle.h"
#include "EntityRenderDispatcher.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\Mth.h"

TakeAnimationParticle::TakeAnimationParticle(Level *level, shared_ptr<Entity> item, shared_ptr<Entity> target, float yOffs) : Particle(level, item->x, item->y, item->z, item->xd, item->yd, item->zd)
{
	// 4J - added initialisers
	life = 0;
	lifeTime = 0;

    this->item = item;

    this->target = target;
    lifeTime = 3;
    this->yOffs = yOffs;

}

TakeAnimationParticle::~TakeAnimationParticle()
{
}

void TakeAnimationParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
    float time = (life + a) / lifeTime;
    time = time*time;

    double xo = item->x;
    double yo = item->y;
    double zo = item->z;

    double xt = target->xOld + (target->x - target->xOld) * a;
    double yt = target->yOld + (target->y - target->yOld) * a+yOffs;
    double zt = target->zOld + (target->z - target->zOld) * a;

    double xx = xo + (xt - xo) * time;
    double yy = yo + (yt - yo) * time;
    double zz = zo + (zt - zo) * time;
        
    int xTile = Mth::floor(xx);
    int yTile = Mth::floor(yy + heightOffset / 2.0f);
    int zTile = Mth::floor(zz);

	// 4J - change brought forward from 1.8.2
    if (SharedConstants::TEXTURE_LIGHTING)
	{
        int col = getLightColor(a);
        int u = col%65536;
        int v = col/65536;
        glMultiTexCoord2f(GL_TEXTURE1, u/1.0f, v/1.0f);
        glColor4f(1, 1, 1, 1);
    }
	else
	{
	    float br = level->getBrightness(xTile, yTile, zTile);
		glColor4f(br, br, br, 1);
	}
        
    xx-=xOff;
    yy-=yOff;
    zz-=zOff;
        
    
    EntityRenderDispatcher::instance->render(item, static_cast<float>(xx), static_cast<float>(yy), static_cast<float>(zz), item->yRot, a);

}

void TakeAnimationParticle::tick()
{
    life++;
    if (life == lifeTime) remove();
}

int TakeAnimationParticle::getParticleTexture()
{
	return ParticleEngine::ENTITY_PARTICLE_TEXTURE;
}
