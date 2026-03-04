#include "stdafx.h"
#include "SuspendedTownParticle.h"
#include "..\Minecraft.World\Random.h"
#include "..\Minecraft.World\JavaMath.h"

SuspendedTownParticle::SuspendedTownParticle(Level *level, double x, double y, double z, double xa, double ya, double za) : Particle(level, x, y, z, xa, ya, za)
{
	float br = random->nextFloat() * 0.1f + 0.2f;
	rCol = br;
	gCol = br;
	bCol = br;
	setMiscTex(0);
	this->setSize(0.02f, 0.02f);

	size = size * (random->nextFloat() * 0.6f + 0.5f);

	xd *= 0.02f;
	yd *= 0.02f;
	zd *= 0.02f;

	lifetime = (int) (20 / (Math::random() * 0.8 + 0.2));
	this->noPhysics = true;
}

void SuspendedTownParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;

	move(xd, yd, zd);
	xd *= 0.99;
	yd *= 0.99;
	zd *= 0.99;

	if (lifetime-- <= 0) remove();
}