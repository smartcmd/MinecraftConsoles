#include "stdafx.h"
#include "..\Minecraft.World\JavaMath.h"
#include "SpellParticle.h"

SpellParticle::SpellParticle(Level *level, double x, double y, double z, double xa, double ya, double za) : Particle(level, x, y, z, xa, ya, za)
{
	yd *= 0.2f;
	if (xa == 0 && za == 0)
	{
		xd *= 0.1f;
		zd *= 0.1f;
	}

	size *= 0.75f;

	lifetime = (int) (8 / (Math::random() * 0.8 + 0.2));
	noPhysics = false;

	baseTex = 8 * 16;
}

void SpellParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
	float l = ((age + a) / lifetime) * 32;
	if (l < 0) l = 0;
	if (l > 1) l = 1;

	Particle::render(t, a, xa, ya, za, xa2, za2);
}

void SpellParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;

	if (age++ >= lifetime) remove();

	setMiscTex(baseTex + (7 - age * 8 / lifetime));

	yd += 0.004;
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

void SpellParticle::setBaseTex(int baseTex)
{
	this->baseTex = baseTex;
}