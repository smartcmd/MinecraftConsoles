#include "stdafx.h"
#include "PlayerCloudParticle.h"
#include "..\Minecraft.World\JavaMath.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.phys.h"

PlayerCloudParticle::PlayerCloudParticle(Level *level, double x, double y, double z, double xa, double ya, double za) : Particle(level,x,y,z,0,0,0)
{
	float scale = 2.5f;
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

	lifetime = (int) (8 / (Math::random() * 0.8 + 0.3));
	lifetime *= scale;
	noPhysics = false;
}

void PlayerCloudParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
	float l = ((age + a) / lifetime) * 32;
	if (l < 0) l = 0;
	if (l > 1) l = 1;

	size = oSize * l;
	Particle::render(t, a, xa, ya, za, xa2, za2);
}

void PlayerCloudParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;

	if (age++ >= lifetime) remove();

	setMiscTex(7 - age * 8 / lifetime);

	move(xd, yd, zd);
	xd *= 0.96f;
	yd *= 0.96f;
	zd *= 0.96f;
	shared_ptr<Player> p = level->getNearestPlayer(shared_from_this(), 2);
	if (p != NULL)
	{
		if (y > p->bb->y0)
		{
			y+=(p->bb->y0-y)*0.2;
			yd += (p->yd-yd)*0.2;
			setPos(x, y, z);
		}
	}

	if (onGround)
	{
		xd *= 0.7f;
		zd *= 0.7f;
	}
}