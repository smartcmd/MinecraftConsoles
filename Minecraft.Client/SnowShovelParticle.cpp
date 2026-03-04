#include "stdafx.h"
#include "..\Minecraft.World\JavaMath.h"
#include "SnowShovelParticle.h"

void SnowShovelParticle::init(Level *level, double x, double y, double z, double xa, double ya, double za, float scale)
{
    xd *= 0.1f;
    yd *= 0.1f;
    zd *= 0.1f;
    xd += xa;
    yd += ya;
    zd += za;

    rCol = gCol = bCol = 1 - (float) (Math::random() * 0.3f);
    size *= 0.75f;
    size *= scale;
    oSize = size;

    lifetime = (int) (8 / (Math::random() * 0.8 + 0.2));
    lifetime = (int) ( lifetime * scale );
    noPhysics = false;
}

SnowShovelParticle::SnowShovelParticle(Level *level, double x, double y, double z, double xa, double ya, double za) : Particle(level, x, y, z, xa, ya, za)
{
	init(level, x, y, z, xa, ya, za, 1);
}

SnowShovelParticle::SnowShovelParticle(Level *level, double x, double y, double z, double xa, double ya, double za, float scale) : Particle(level, x, y, z, xa, ya, za)
{
	init(level, x, y, z, xa, ya, za, scale);
}

void SnowShovelParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
    float l = ((age + a) / lifetime) * 32;
    if (l < 0) l = 0;
    if (l > 1) l = 1;

    size = oSize * l;
    Particle::render(t, a, xa, ya, za, xa2, za2);

}

void SnowShovelParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime) remove();

    setMiscTex(7 - age * 8 / lifetime);

    yd -= 0.03;
    move(xd, yd, zd);
    xd *= 0.99f;
    yd *= 0.99f;
    zd *= 0.99f;

    if (onGround)
	{
        xd *= 0.7f;
        zd *= 0.7f;
    }
}
