#include "stdafx.h"
#include "LavaParticle.h"
#include "..\Minecraft.World\JavaMath.h"
#include "..\Minecraft.World\Random.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"

LavaParticle::LavaParticle(Level *level, double x, double y, double z) : Particle(level, x, y, z, 0, 0, 0)
{
    xd *= 0.8f;
    yd *= 0.8f;
    zd *= 0.8f;
    yd = random->nextFloat() * 0.4f + 0.05f;

    rCol = gCol = bCol = 1;
    size *= (random->nextFloat() * 2 + 0.2f);
    oSize = size;

    lifetime = (int) (16 / (Math::random() * 0.8 + 0.2));
    noPhysics = false;
    setMiscTex(49);
}

// 4J - brought forward from 1.8.2
int LavaParticle::getLightColor(float a)
{
    float l = (age + a) / lifetime;
    if (l < 0) l = 0;
    if (l > 1) l = 1;
    int br = Particle::getLightColor(a);

    int br1 = 15 * 16;
    int br2 = (br >> 16) & 0xff;
    return br1 | br2 << 16;
}

float LavaParticle::getBrightness(float a)
{
	return 1;
}

void LavaParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
    float s = (age + a) / (float) lifetime;
    size = oSize * (1 - s*s);
    Particle::render(t, a, xa, ya, za, xa2, za2);
}

void LavaParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime) remove();
    float odds = age / (float) lifetime;
    if (random->nextFloat() > odds) level->addParticle(eParticleType_smoke, x, y, z, xd, yd, zd);

    yd -= 0.03;
    move(xd, yd, zd);
    xd *= 0.999f;
    yd *= 0.999f;
    zd *= 0.999f;

    if (onGround)
	{
        xd *= 0.7f;
        zd *= 0.7f;
    }
}
