#include "stdafx.h"
#include "..\Minecraft.World\JavaMath.h"
#include "RedDustParticle.h"

void RedDustParticle::init(Level *level, double x, double y, double z, float scale, float rCol, float gCol, float bCol)
{
    xd *= 0.1f;
    yd *= 0.1f;
    zd *= 0.1f;

	// 4J Stu - If they are all 0 then this particle has been created differently
	// If just red is 0 it could be because we have made redstone a completely different colour (eg blue)
    if (rCol == 0 && gCol == 0 && bCol == 0)
	{
        rCol = 1;
    }
    float brr = static_cast<float>(Math::random()) * 0.4f + 0.6f;
    this->rCol = (static_cast<float>(Math::random() * 0.2f) + 0.8f) * rCol * brr;
    this->gCol = (static_cast<float>(Math::random() * 0.2f) + 0.8f) * gCol * brr;
    this->bCol = (static_cast<float>(Math::random() * 0.2f) + 0.8f) * bCol * brr;
    size *= 0.75f;
    size *= scale;
    oSize = size;

    lifetime = static_cast<int>(8 / (Math::random() * 0.8 + 0.2));
    lifetime = static_cast<int>(lifetime * scale);
    noPhysics = false;
}

RedDustParticle::RedDustParticle(Level *level, double x, double y, double z, float rCol, float gCol, float bCol) : Particle(level, x, y, z, 0, 0, 0)
{
	init(level, x, y, z, 1, rCol, gCol, bCol);
}

RedDustParticle::RedDustParticle(Level *level, double x, double y, double z, float scale, float rCol, float gCol, float bCol) : Particle(level, x, y, z, 0, 0, 0)
{
	init(level, x, y, z, scale, rCol, gCol, bCol);
}

void RedDustParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
    float l = ((age + a) / lifetime) * 32;
    if (l < 0) l = 0;
    if (l > 1) l = 1;

    size = oSize * l;
    Particle::render(t, a, xa, ya, za, xa2, za2);
}

void RedDustParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime) remove();

    setMiscTex(7 - age * 8 / lifetime);

    move(xd, yd, zd);
    if (y == yo)
	{
        xd *= 1.1;
        zd *= 1.1;
    }
    xd *= 0.96f;
    yd *= 0.96f;
    zd *= 0.96f;

    if (onGround)
	{
        xd *= 0.7f;
        zd *= 0.7f;
    }
}
