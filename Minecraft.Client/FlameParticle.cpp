#include "stdafx.h"
#include "..\Minecraft.World\JavaMath.h"
#include "..\Minecraft.World\Random.h"
#include "FlameParticle.h"

FlameParticle::FlameParticle(Level *level, double x, double y, double z, double xd, double yd, double zd) : Particle(level, x, y, z, xd, yd, zd)
{
    this->xd=this->xd*0.01f+xd;
    this->yd=this->yd*0.01f+yd;
    this->zd=this->zd*0.01f+zd;
    x+=(random->nextFloat()-random->nextFloat())*0.05f;
    y+=(random->nextFloat()-random->nextFloat())*0.05f;
    z+=(random->nextFloat()-random->nextFloat())*0.05f;
        
    oSize = size;
    rCol = gCol = bCol = 1.0f;
        
    lifetime = (int)(8/(Math::random()*0.8+0.2))+4;
    noPhysics = true;
    setMiscTex(48);
}

void FlameParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
    float s = (age + a) / (float) lifetime;
    size = oSize * (1 - s*s*0.5f);
    Particle::render(t, a, xa, ya, za, xa2, za2);
}

// 4J - brought forward from 1.8.2
int FlameParticle::getLightColor(float a)
{
    float l = (age + a) / lifetime;
    if (l < 0) l = 0;
    if (l > 1) l = 1;
    int br = Particle::getLightColor(a);

    int br1 = (br) & 0xff;
    int br2 = (br >> 16) & 0xff;
    br1 += (int) (l * 15 * 16);
    if (br1 > 15 * 16) br1 = 15 * 16;
    return br1 | br2 << 16;
}

float FlameParticle::getBrightness(float a)
{
	float l = (age+a)/lifetime;
	if (l<0) l = 0;
	if (l>1) l = 1;
	float br = Particle::getBrightness(a);
        
	return br*l+(1-l);
}

void FlameParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime) remove();

    move(xd, yd, zd);
    xd *= 0.96f;
    yd *= 0.96f;
    zd *= 0.96f;

    if (onGround)
    {
        xd *= 0.7f;
        zd *= 0.7f;
    }
}