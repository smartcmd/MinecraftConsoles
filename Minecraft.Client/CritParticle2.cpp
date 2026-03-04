#include "stdafx.h"
#include "CritParticle2.h"
#include "..\Minecraft.World\JavaMath.h"

void CritParticle2::_init(double xa, double ya, double za, float scale)
{
	xd *= 0.1f;
	yd *= 0.1f;
	zd *= 0.1f;
	xd += xa * 0.4;
	yd += ya * 0.4;
	zd += za * 0.4;

	rCol = gCol = bCol = (float) (Math::random() * 0.3f + 0.6f);
	size *= 0.75f;
	size *= scale;
	oSize = size;

	lifetime = (int) (6 / (Math::random() * 0.8 + 0.6));
	lifetime *= scale;
	noPhysics = false;

	setMiscTex(16 * 4 + 1);
	// 4J-PB - can't use a shared_from_this in the constructor
	//tick();
	m_bAgeUniformly=false; // 4J added
}

CritParticle2::CritParticle2(Level *level, double x, double y, double z, double xa, double ya, double za) : Particle(level, x, y, z, 0, 0, 0)
{
	_init(xa,ya,za,1);
}

CritParticle2::CritParticle2(Level *level, double x, double y, double z, double xa, double ya, double za, float scale) : Particle(level, x, y, z, 0, 0, 0)
{
	_init(xa,ya,za,scale);
}

void CritParticle2::CritParticle2PostConstructor(void)
{
	tick();
}

void CritParticle2::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
	float l = ((age + a) / lifetime) * 32;
	if (l < 0) l = 0;
	if (l > 1) l = 1;

	size = oSize * l;
	Particle::render(t, a, xa, ya, za, xa2, za2);
}

void CritParticle2::SetAgeUniformly()
{
	m_bAgeUniformly=true;
}

void CritParticle2::tick()
{
	xo = x;
	yo = y;
	zo = z;

	if (age++ >= lifetime) remove();

	move(xd, yd, zd);
	gCol *= 0.96;
	bCol *= 0.9;

	if(m_bAgeUniformly)
	{
		rCol *= 0.99;
		gCol *= 0.99;
		bCol *= 0.99;
	}
	else
	{
		gCol *= 0.96;
		bCol *= 0.9;
	}

	xd *= 0.70f;
	yd *= 0.70f;
	zd *= 0.70f;
	yd-=0.02f;

	if (onGround)
	{
		xd *= 0.7f;
		zd *= 0.7f;
	}
}
